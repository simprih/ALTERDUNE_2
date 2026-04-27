#include "jeu_sfml.h"
#include "chargeurCSV.h"
#include "actionAct.h"
#include "item.h"
#include <algorithm>
#include <sstream>
#include <cmath>

// ─── Constantes de mise en page ───────────────────────────────────────────────
static constexpr unsigned int WIN_W   = 900;
static constexpr unsigned int WIN_H   = 650;
static constexpr unsigned int FONT_SM = 16;
static constexpr unsigned int FONT_MD = 20;
static constexpr unsigned int FONT_LG = 28;
static constexpr unsigned int FONT_XL = 52;
static constexpr float        MARGIN  = 40.f;
static constexpr float        LINE_SM = 22.f;
static constexpr float        LINE_MD = 28.f;
static constexpr int          MAX_VIS = 22;

// ─── Constructeur / Destructeur ───────────────────────────────────────────────

JeuSFML::JeuSFML()
    : rng(std::random_device{}()),
      etat(GameState::TITLE),
      monstreCourant(nullptr),
      tourConsomme(false),
      scrollOffset(0),
      nbActionsMax(0),
      monstreCourantIdx(-1),
      frameCount(0),
      hitAnimStart(-1),
      hitDamage(0),
      prochainEtat(GameState::MAIN_MENU)
{}

JeuSFML::~JeuSFML() {
    for (Monstre* m : monstres) delete m;
    if (monstreCourant) { delete monstreCourant; monstreCourant = nullptr; }
}

// ─── Chargement police ────────────────────────────────────────────────────────

bool JeuSFML::chargerPolice() {
    const std::vector<std::string> chemins = {
        "/System/Library/Fonts/Monaco.ttf",
        "/Library/Fonts/Arial.ttf",
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/System/Library/Fonts/Helvetica.ttc"
    };
    for (const auto& p : chemins)
        if (font.openFromFile(p)) return true;
    return false;
}

// ─── Chargement données CSV ───────────────────────────────────────────────────

void JeuSFML::chargerDonnees() {
    monstres = ChargeurCSV::chargerMonstres("monsters.csv");
    monstresDisponibles.clear();
    for (int i = 0; i < (int)monstres.size(); ++i)
        monstresDisponibles.push_back(i);
}

// ─── Boucle principale ────────────────────────────────────────────────────────

void JeuSFML::demarrer() {
    window.create(sf::VideoMode({WIN_W, WIN_H}), "ALTERDUNE");
    window.setFramerateLimit(60);

    if (!chargerPolice()) {
        window.close();
        return;
    }
    chargerDonnees();
    etat = GameState::TITLE;

    while (window.isOpen()) {
        ++frameCount;
        while (const auto ev = window.pollEvent()) {
            if (ev->is<sf::Event::Closed>()) {
                window.close();
                return;
            }
            dispatchEvent(*ev);
        }
        window.clear(sf::Color::Black);
        render();
        window.display();
    }
}

// ─── Dispatch événements ──────────────────────────────────────────────────────

void JeuSFML::dispatchEvent(const sf::Event& e) {
    switch (etat) {
        case GameState::TITLE:         handleEventTitre(e);           break;
        case GameState::NAME_INPUT:    handleEventNomInput(e);        break;
        case GameState::RECAP:         handleEventRecap(e);           break;
        case GameState::MAIN_MENU:     handleEventMenuPrincipal(e);   break;
        case GameState::BESTIARY:      handleEventBestiaire(e);       break;
        case GameState::STATS:         handleEventStats(e);           break;
        case GameState::INVENTORY:     handleEventInventaire(e);      break;
        case GameState::COMBAT_START:  handleEventCombatStart(e);     break;
        case GameState::COMBAT_MAIN:   handleEventCombatPrincipal(e); break;
        case GameState::COMBAT_ACT:    handleEventCombatACT(e);       break;
        case GameState::COMBAT_ITEM:   handleEventCombatItem(e);      break;
        case GameState::COMBAT_RESULT: handleEventCombatResultat(e);  break;
        case GameState::GAME_OVER:     handleEventFin(e);             break;
        case GameState::END_SCREEN:    handleEventFin(e);             break;
    }
}

// ─── Dispatch rendu ───────────────────────────────────────────────────────────

void JeuSFML::render() {
    switch (etat) {
        case GameState::TITLE:         renderTitre();           break;
        case GameState::NAME_INPUT:    renderNomInput();        break;
        case GameState::RECAP:         renderRecap();           break;
        case GameState::MAIN_MENU:     renderMenuPrincipal();   break;
        case GameState::BESTIARY:      renderBestiaire();       break;
        case GameState::STATS:         renderStats();           break;
        case GameState::INVENTORY:     renderInventaire();      break;
        case GameState::COMBAT_START:  renderCombatStart();     break;
        case GameState::COMBAT_MAIN:   renderCombatPrincipal(); break;
        case GameState::COMBAT_ACT:    renderCombatACT();       break;
        case GameState::COMBAT_ITEM:   renderCombatItem();      break;
        case GameState::COMBAT_RESULT: renderCombatResultat();  break;
        case GameState::GAME_OVER:     renderFin();             break;
        case GameState::END_SCREEN:    renderFin();             break;
    }
}

// ─── Primitives de rendu ──────────────────────────────────────────────────────

void JeuSFML::dessinerTexte(const std::string& s, unsigned int taille,
                             sf::Color couleur, float x, float y) {
    sf::Text txt(font, s, taille);
    txt.setFillColor(couleur);
    txt.setPosition({x, y});
    window.draw(txt);
}

void JeuSFML::dessinerLignes(const std::vector<std::string>& lignes,
                              float x, float y, unsigned int taille,
                              sf::Color couleur, int offset, int maxVisible) {
    int n = (int)lignes.size();
    float lineH = (taille <= 16) ? LINE_SM : LINE_MD;
    for (int i = offset; i < n && i < offset + maxVisible; ++i) {
        dessinerTexte(lignes[i], taille, couleur,
                      x, y + (float)(i - offset) * lineH);
    }
}

void JeuSFML::dessinerBarre(float x, float y, float w, float h,
                             int val, int max, sf::Color fill, sf::Color empty) {
    // Fond
    sf::RectangleShape fond({w, h});
    fond.setPosition({x, y});
    fond.setFillColor(empty);
    fond.setOutlineColor(sf::Color(180, 180, 180));
    fond.setOutlineThickness(1.f);
    window.draw(fond);

    // Remplissage
    if (max > 0 && val > 0) {
        float ratio = std::min(1.f, (float)val / (float)max);
        sf::RectangleShape barre({w * ratio, h});
        barre.setPosition({x, y});
        barre.setFillColor(fill);
        window.draw(barre);
    }
}

void JeuSFML::dessinerSeparateur(float y, sf::Color couleur) {
    sf::RectangleShape ligne({(float)WIN_W - 2.f * MARGIN, 1.f});
    ligne.setPosition({MARGIN, y});
    ligne.setFillColor(couleur);
    window.draw(ligne);
}

// ─── Rendu visuel des monstres ────────────────────────────────────────────────
//
// Chaque monstre est dessiné avec des formes SFML (aucun asset externe).
// Les coordonnées dx/dy des helpers R() et C() sont relatives au centre (cx, cy).
//
// Animations :
//   NORMAL   — flottement vertical doux  (sinf * 5 px)
//   MINIBOSS — flottement plus ample     (sinf * 8 px)
//   BOSS     — pulsation lumineuse + léger tangage (sinf * 3 px)
//
// L'ombre reste au sol (cy0) pendant que le monstre flotte.

void JeuSFML::dessinerMonstre(const Monstre* m, float cx, float cy) {
    const std::string& cat = m->getCategorie();
    const std::string& nom = m->getNom();
    float t   = static_cast<float>(frameCount) * 0.05f;
    float cy0 = cy;

    // Taille proportionnelle aux HP
    float scale = 0.55f + static_cast<float>(m->getHpMax()) / 100.f * 0.85f;
    scale = std::max(0.55f, std::min(scale, 1.75f));

    // Animation par catégorie
    if      (cat == "NORMAL")   cy += sinf(t)        *  6.f;
    else if (cat == "MINIBOSS") cy += sinf(t * 1.3f) * 10.f;
    else                        cy += sinf(t * 0.6f) *  5.f;

    // Ombre au sol
    {
        float sw = ((cat == "BOSS") ? 110.f : (cat == "MINIBOSS") ? 75.f : 55.f) * scale;
        float so = ((cat == "BOSS") ?  85.f : (cat == "MINIBOSS") ? 62.f : 48.f) * scale;
        sf::RectangleShape shad({sw, 7.f});
        shad.setPosition({cx - sw * .5f, cy0 + so});
        shad.setFillColor(sf::Color(5, 0, 10, 110));
        window.draw(shad);
    }

    // Helpers : rectangle et cercle centrés, scale appliqué
    auto R = [&](float dx, float dy, float w, float h, sf::Color c) {
        sf::RectangleShape s({w * scale, h * scale});
        s.setPosition({cx + dx * scale - w * scale * .5f,
                       cy + dy * scale - h * scale * .5f});
        s.setFillColor(c);
        window.draw(s);
    };
    auto C = [&](float dx, float dy, float r, sf::Color c) {
        float rs = r * scale;
        sf::CircleShape s(rs);
        s.setPosition({cx + dx * scale - rs, cy + dy * scale - rs});
        s.setFillColor(c);
        window.draw(s);
    };
    auto spike = [&](float dx, float dy, float w, float h, sf::Color c) {
        sf::ConvexShape tri(3);
        float bx = cx + dx * scale, by = cy + dy * scale;
        float hw = w * scale * .5f, sh = h * scale;
        tri.setPoint(0, {bx - hw, by + sh});
        tri.setPoint(1, {bx + hw, by + sh});
        tri.setPoint(2, {bx,      by     });
        tri.setFillColor(c);
        window.draw(tri);
    };

    // ─── FROGGIT — crapaud des marais (NORMAL) ────────────────────────────────
    if (nom == "Froggit") {
        float blink = (sinf(t * 4.f) > 0.85f) ? 0.f : 1.f;
        sf::Color body(25, 90, 20), belly(40, 125, 35), dark(10, 45, 8);
        sf::Color eyeC((int)(220 * blink), (int)(30 * (1.f - blink)), 10);
        sf::Color tooth(215, 205, 180);
        C(-18, -36, 13, dark);    C(18, -36, 13, dark);
        C(-18, -35, 11, body);    C(18, -35, 11, body);
        C(-18, -34,  9, eyeC);    C(18, -34,  9, eyeC);
        C(-18, -34,  4, sf::Color(10, 0, 0)); C(18, -34, 4, sf::Color(10, 0, 0));
        C(0, -8, 34, body);
        R(0, 14, 44, 16, belly);
        R(0, 22, 40,  8, dark);
        for (int i = -2; i <= 2; ++i) R(i * 8.f, 20, 5, 10, tooth);
        R(-42, 2, 18, 10, body); R(42, 2, 18, 10, body);
        R(-26, 44, 20, 10, body); R(26, 44, 20, 10, body);
        R(-34, 50, 28,  8, body); R(34, 50, 28,  8, body);
    }

    // ─── WHIMSUN — spectre ailé (NORMAL) ─────────────────────────────────────
    else if (nom == "Whimsun") {
        float g = 0.4f + sinf(t * 2.5f) * 0.4f;
        sf::Color bdy(110, 70, 150, 210), wing(70, 45, 125, 130);
        sf::Color sock(8, 4, 20);
        sf::Color glow((int)(165 * g), (int)(70 * g), (int)(210 * g));
        C(-40, -20, 26, wing); C(40, -20, 26, wing);
        R(-46, -10, 10, 20, wing); R(46, -10, 10, 20, wing);
        R(-52,  -2,  8, 12, wing); R(52,  -2,  8, 12, wing);
        C(0, -12, 24, bdy);
        C(-10, -16, 7, sock); C(10, -16, 7, sock);
        C(-10, -16, 4, glow); C(10, -16, 4, glow);
        R(0, 12, 34, 40, bdy);
        C(-12, 38, 10, sf::Color(90, 58, 130, 150));
        C(  0, 42,  8, sf::Color(90, 58, 130, 120));
        C( 12, 38, 10, sf::Color(90, 58, 130, 150));
        R(-8, -4, 4, 5, sf::Color(35, 18, 55));
        R( 8, -4, 4, 5, sf::Color(35, 18, 55));
        R( 0, -2, 10, 4, sf::Color(35, 18, 55));
    }

    // ─── SLIMETTE — blob acide (NORMAL) ──────────────────────────────────────
    else if (nom == "Slimette") {
        float drip = std::abs(sinf(t * 1.8f));
        sf::Color slime(28, 168, 12, 225), dslime(14, 95, 6, 215);
        sf::Color eyeC(235, 28, 8);
        C(0, 0, 32, slime); C(-14, -8, 18, slime); C(14, -8, 18, slime); C(0, -14, 20, slime);
        C(-12, -8, 7, eyeC); C(12, -8, 7, eyeC);
        C(-12, -8, 3, sf::Color(10, 0, 0)); C(12, -8, 3, sf::Color(10, 0, 0));
        R(-12, -16, 14, 4, dslime); R(12, -16, 14, 4, dslime);
        R(0, 8, 28, 6, dslime);
        for (int i = -2; i <= 2; ++i) R(i * 6.f, 7, 4, 8, sf::Color(175, 210, 10));
        R( 0, 32 + drip * 12, 6, 14 + drip * 8, slime);
        R(-16, 28 + drip *  6, 5, 10 + drip * 6, slime);
        R( 16, 28 + drip *  6, 5, 10 + drip * 6, slime);
    }

    // ─── SKELOX — squelette guerrier (NORMAL) ────────────────────────────────
    else if (nom == "Skelox") {
        float flicker = (sinf(t * 5.f) > 0.7f) ? 1.f : 0.f;
        sf::Color bone(190, 182, 162), crack(125, 115, 100);
        sf::Color eyeC((int)(210 * flicker + 90), (int)(18 * flicker), (int)(170 * flicker));
        sf::Color tooth(210, 202, 180);
        C(0, -24, 28, bone);
        R(-8, -32, 3, 18, crack); R(8, -28, 2, 12, crack);
        C(-12, -28, 9, sf::Color(10, 10, 12)); C(12, -28, 9, sf::Color(10, 10, 12));
        C(-12, -28, 6, eyeC); C(12, -28, 6, eyeC);
        R(0, -8, 36, 12, bone);
        for (int i = -2; i <= 2; ++i) R(i * 7.f, -5, 5, 10, tooth);
        R(0, -4, 32, 4, sf::Color(20, 15, 15));
        R(0, 14, 38, 6, bone);
        for (int i = 0; i < 4; ++i) R(0, 20.f + i * 10.f, 28, 4, bone);
        R(0, 30, 6, 30, bone);
        R(-28, 18, 8, 30, bone); R(28, 18, 8, 30, bone);
        R(-28, 42, 10, 8, bone); R(28, 42, 10,  8, bone);
    }

    // ─── MIMICBOX — coffre-mimic (MINIBOSS) ──────────────────────────────────
    else if (nom == "MimicBox") {
        float eg  = 0.5f + sinf(t * 2.8f) * 0.5f;
        float jaw = sinf(t * 3.f) * 6.f;
        sf::Color wood(85, 48, 12), lid(55, 28, 6), metal(135, 115, 48);
        sf::Color eyeC((int)(255 * eg), (int)(28 * eg), 8);
        sf::Color tooth(220, 210, 190), spike_c(50, 26, 4);
        for (int i = -3; i <= 3; ++i) spike(i * 12.f, -38, 10, 14, spike_c);
        R(0, -18 + jaw * .5f, 88, 30, lid);
        R(0, -28 + jaw * .5f, 88,  6, metal);
        R(0, -22 + jaw * .5f, 20, 16, metal);
        R(0, -20 + jaw * .5f, 10, 10, sf::Color(22, 12, 4));
        R(0, 20, 88, 58, wood);
        R(0,  4, 88,  6, metal);
        C(-22, 18, 12, sf::Color(28, 8, 4)); C(22, 18, 12, sf::Color(28, 8, 4));
        C(-22, 18,  9, eyeC); C(22, 18,  9, eyeC);
        C(-22, 18,  4, sf::Color(255, std::min(255, (int)(110 + 145 * eg)), 55));
        C( 22, 18,  4, sf::Color(255, std::min(255, (int)(110 + 145 * eg)), 55));
        for (int i = -4; i <= 4; ++i) R(i * 9.f, 4 + jaw * .3f, 5, 14, tooth);
        R(-34, 52, 20, 12, lid); R(34, 52, 20, 12, lid);
    }

    // ─── SHYREN — abomination abyssale (MINIBOSS) ────────────────────────────
    else if (nom == "Shyren") {
        float swx = sinf(t * 2.f) * 5.f;
        float g   = 0.5f + sinf(t * 2.8f) * 0.5f;
        sf::Color bdy(12, 75, 148), fin(6, 40, 105), dark(4, 18, 55);
        sf::Color eyeC((int)(18 * g), (int)(175 * g), (int)(250 * g));
        sf::Color tooth(200, 195, 178);
        R(swx, 55, 60, 16, fin);
        R(swx - 22, 40, 26, 22, bdy); R(swx + 22, 40, 26, 22, bdy);
        R(-52, 0, 22, 8, fin); R(52, 0, 22, 8, fin);
        spike(-58, -8, 12, 20, fin); spike(58, -8, 12, 20, fin);
        C(0, 5, 40, bdy);
        spike( 0, -48, 18, 32, fin);
        spike(-10, -40, 12, 22, fin); spike(10, -40, 12, 22, fin);
        C(-14, 0, 12, dark); C(14, 0, 12, dark);
        C(-14, 0,  9, eyeC); C(14, 0,  9, eyeC);
        C(-14, 0,  4, sf::Color(195, 250, 255)); C(14, 0, 4, sf::Color(195, 250, 255));
        R(0, 16, 42, 8, dark);
        for (int i = -2; i <= 2; ++i) R(i * 8.f, 14, 5, 12, tooth);
    }

    // ─── PYROC — golem de lave (MINIBOSS) ────────────────────────────────────
    else if (nom == "Pyroc") {
        float flame = sinf(t * 3.5f);
        float g = 0.5f + flame * 0.5f;
        sf::Color rock(38, 26, 20), lava(195, 75, 8);
        sf::Color fire_in((int)(250 * g), (int)(135 * g), 8);
        sf::Color fire_out(250, (int)(55 + 78 * g), 0);
        sf::Color eye_c((int)(250 * g), (int)(175 * g), 0);
        spike(-18, -72 + flame * 4, 24, 32, fire_out);
        spike(  0, -80 + flame * 4, 28, 40, fire_in);
        spike( 18, -72 + flame * 4, 24, 32, fire_out);
        spike(-32, -62 + flame * 3, 18, 24, sf::Color(250, 95, 0, 165));
        spike( 32, -62 + flame * 3, 18, 24, sf::Color(250, 95, 0, 165));
        C(0, -30, 30, rock);
        R( 0, -36, 4, 20, lava); R(-12, -30, 3, 14, lava); R(12, -30, 3, 14, lava);
        C(-12, -34, 8, sf::Color(18, 8, 4)); C(12, -34, 8, sf::Color(18, 8, 4));
        C(-12, -34, 6, eye_c); C(12, -34, 6, eye_c);
        R(0, 8, 80, 68, rock);
        R(0,  4,  5, 68, lava); R(-20, 10, 4, 50, lava); R(20, 10, 4, 50, lava);
        R(0, 20, 60, 4, lava);  R(0, 38, 55, 3, lava);
        R(-52, 0, 22, 52, rock); R(52, 0, 22, 52, rock);
        R(-52, 34, 28, 26, rock); R(52, 34, 28, 26, rock);
        R(-52, 14,  3, 36, lava); R(52, 14,  3, 36, lava);
        R(-20, 54, 24, 20, rock); R(20, 54, 24, 20, rock);
    }

    // ─── THORNBEAST — prédateur végétal (MINIBOSS) ───────────────────────────
    else if (nom == "Thornbeast") {
        float pulse = 0.5f + sinf(t * 2.f) * 0.5f;
        float sw    = sinf(t * 1.5f) * 4.f;
        sf::Color bark(32, 52, 18), leaf(18, 88, 12), thorn_c(175, 145, 8);
        sf::Color eyeC((int)(28 * pulse), (int)(215 * pulse), 18);
        sf::Color toxic(48, 175, 18, 185);
        R(sw - 38, -10, 12, 60, bark); R(sw + 38, -10, 12, 60, bark);
        for (int i = -2; i <= 2; ++i) {
            spike(sw - 44.f, -8.f + i * 12.f, 8, 14, thorn_c);
            spike(sw + 44.f, -8.f + i * 12.f, 8, 14, thorn_c);
        }
        C(sw, -10, 38, bark);
        for (int i = 0; i < 6; ++i) {
            float angle = i * 3.14159f / 3.f + t * 0.3f;
            C(sw + cosf(angle) * 44.f, sinf(angle) * 44.f - 10.f, 12, leaf);
        }
        C(sw, -10, 30, sf::Color(12, 38, 8));
        C(sw - 12, -16, 8, sf::Color(4, 14, 4)); C(sw + 12, -16, 8, sf::Color(4, 14, 4));
        C(sw - 12, -16, 6, eyeC); C(sw + 12, -16, 6, eyeC);
        for (int i = -2; i <= 2; ++i) {
            spike(sw + i * 8.f, -24, 6, 14, thorn_c);
            R(sw + i * 8.f, 0, 5, 10, thorn_c);
        }
        R(sw - 16, 38, 8, 24, bark); R(sw + 16, 38, 8, 24, bark);
        R(sw, 40, 10, 22, bark);
        C(sw - 5, 20 + pulse * 8, 5, toxic); C(sw + 5, 18 + pulse * 6, 4, toxic);
    }

    // ─── QUEENBYTE — reine des circuits (BOSS) ───────────────────────────────
    else if (nom == "QueenByte") {
        float g = 0.55f + sinf(t * 1.8f) * 0.45f;
        sf::Color bdy(35, 0, 58), lit(78, 16, 115), crown(195, 135, 0);
        sf::Color eyeC((int)(255 * g), (int)(180 * g), 0);
        sf::Color cir(95, 35, 155, 175), danger(215, 28, 28);
        {
            sf::CircleShape a1(80.f * scale);
            a1.setPosition({cx - 80.f * scale, cy - 80.f * scale});
            a1.setFillColor(sf::Color(75, 0, 125, (int)(35 * g)));
            window.draw(a1);
            sf::CircleShape a2(58.f * scale);
            a2.setPosition({cx - 58.f * scale, cy - 58.f * scale});
            a2.setFillColor(sf::Color(115, 0, 175, (int)(22 * g)));
            window.draw(a2);
        }
        R(-46,-102, 10, 44, crown); R(-26,-110, 10, 44, crown);
        R(  0,-118, 14, 52, crown); R( 26,-110, 10, 44, crown); R(46,-102, 10, 44, crown);
        R(-10,-106, 10, 44, crown); R( 10,-106, 10, 44, crown);
        C(-42,-82, 7, sf::Color(255, 38, 38)); C(-22,-90, 7, sf::Color(38, 195, 255));
        C(  0,-96, 8, sf::Color(255, 215, 0)); C( 22,-90, 7, sf::Color(38, 250, 98));
        C( 42,-82, 7, sf::Color(205, 38, 255)); C(-62,-72, 5, danger); C(62,-72, 5, danger);
        C(0, -42, 35, lit);
        R(0, -48, 50, 3, cir); R(0, -42, 50, 2, cir);
        C(-16,-46, 11, sf::Color(18, 4, 32)); C(16,-46, 11, sf::Color(18, 4, 32));
        C(-16,-46,  9, eyeC); C(16,-46, 9, eyeC);
        C(-16,-46,  4, sf::Color(255, 255, 195)); C(16,-46, 4, sf::Color(255, 255, 195));
        R(0, -4, 98, 76, bdy); R(0, -14, 78, 18, lit);
        R(0, 5, 68, 3, cir); R(0, 20, 68, 3, cir); R(0, 35, 68, 3, cir);
        R(-4,-4, 3, 76, cir); R(4,-4, 3, 76, cir);
        R(-22,-4, 3, 50, cir); R(22,-4, 3, 50, cir);
        C(-22, 5, 4, eyeC);  C(22, 5, 4, eyeC);
        C(-22,20, 4, danger); C(22,20, 4, danger);
        C(-22,35, 4, eyeC);  C(22,35, 4, eyeC);
        R(-60,-7, 24, 40, lit); R(60,-7, 24, 40, lit);
        spike(-72,-18, 14, 22, crown); spike(72,-18, 14, 22, crown);
        R(-76, 16, 16, 56, bdy); R(76, 16, 16, 56, bdy);
        R(-84,55, 10,20,crown); R(-72,60, 10,18,crown); R(-60,62, 8,14,crown);
        R( 60,62,  8,14,crown); R( 72,60, 10,18,crown); R( 84,55,10,20,crown);
    }

    // ─── VOIDLORD — entité du néant (BOSS) ───────────────────────────────────
    else if (nom == "VoidLord") {
        float g     = 0.5f + sinf(t * 1.5f) * 0.5f;
        float g2    = 0.5f + sinf(t * 2.3f + 1.f) * 0.5f;
        float swirl = t * 0.8f;
        sf::Color void_c(6, 0, 18), outer(32, 0, 52);
        sf::Color eye_c((int)(195 * g2), 0, (int)(250 * g));
        sf::Color tentacle(22, 0, 42);
        for (int i = 3; i >= 1; --i) {
            float r = (48.f + i * 26.f) * scale;
            sf::CircleShape aura(r);
            aura.setPosition({cx - r, cy - r});
            aura.setFillColor(sf::Color(28, 0, 58, (int)(18 * g * (4 - i))));
            window.draw(aura);
        }
        for (int i = 0; i < 6; ++i) {
            float angle = swirl + i * 3.14159f / 3.f;
            float px = cosf(angle) * 62.f, py = sinf(angle) * 35.f;
            float len = 28.f + sinf(angle * 2.f + t) * 10.f;
            R(px, py + 20, 10, len, tentacle);
            C(px, py + 20.f + len * .5f, 6, outer);
        }
        C(0, 0, 52, outer); C(0, 0, 44, void_c);
        C(0, -5, 14, sf::Color(14, 0, 28));
        C(0, -5, 11, eye_c);
        C(0, -5,  5, sf::Color(255, 195, 255));
        R(0, -5, 2, 22, sf::Color(0, 0, 0, 185));
        R(0, -5, 22, 2, sf::Color(0, 0, 0, 185));
        for (int i = 0; i < 5; ++i) {
            float a = swirl * 0.7f + i * 3.14159f * 2.f / 5.f;
            float ex = cosf(a) * 34.f, ey = sinf(a) * 22.f;
            C(ex, ey, 7, sf::Color(8, 0, 18));
            C(ex, ey, 5, sf::Color((int)(175 * g2), 0, (int)(215 * g)));
            C(ex, ey, 2, sf::Color(255, 175, 255));
        }
        for (int i = 0; i < 8; ++i) {
            float a = i * 3.14159f / 4.f + t * 0.2f;
            float mx = (cosf(a) * 44.f + cosf(a) * 58.f) * .5f;
            float my = (sinf(a) * 44.f + sinf(a) * 58.f) * .5f;
            C(mx, my, 3, sf::Color((int)(155 * g), 0, (int)(195 * g), 185));
        }
        R(0, 22, 48, 16, sf::Color(4, 0, 10));
        for (int i = -3; i <= 3; ++i) {
            spike(i * 7.f, 14, 6, 12, outer);
            R(i * 7.f, 30, 5, 10, outer);
        }
    }

    // ─── Générique ───────────────────────────────────────────────────────────
    else {
        sf::Color c = (cat == "BOSS")     ? sf::Color(130, 18, 18) :
                      (cat == "MINIBOSS") ? sf::Color(155, 95, 12) :
                                            sf::Color( 36, 115, 36);
        float r  = (cat == "BOSS") ? 55.f : (cat == "MINIBOSS") ? 40.f : 26.f;
        float eg = 0.5f + sinf(t * 3.f) * 0.5f;
        C(0, 0, r, c);
        C(-r*.35f, -r*.3f, r*.22f, sf::Color::White);
        C( r*.35f, -r*.3f, r*.22f, sf::Color::White);
        C(-r*.35f, -r*.3f, r*.13f, sf::Color((int)(215 * eg), (int)(28 * eg), 8));
        C( r*.35f, -r*.3f, r*.13f, sf::Color((int)(215 * eg), (int)(28 * eg), 8));
        for (int i = -2; i <= 2; ++i)
            R(i * r * .18f, r * .42f, r * .1f, r * .2f, sf::Color(218, 210, 192));
    }
}

sf::Color JeuSFML::couleurHP(int hp, int hpMax) {
    if (hpMax <= 0) return sf::Color::Green;
    float ratio = (float)hp / (float)hpMax;
    if (ratio > 0.6f) return sf::Color::Green;
    if (ratio > 0.3f) return sf::Color::Yellow;
    return sf::Color::Red;
}

int JeuSFML::keyToInt(const sf::Event::KeyPressed& kp) {
    using K = sf::Keyboard::Key;
    if (kp.code >= K::Num1 && kp.code <= K::Num9)
        return (int)kp.code - (int)K::Num0;
    if (kp.code >= K::Numpad1 && kp.code <= K::Numpad9)
        return (int)kp.code - (int)K::Numpad0;
    if (kp.code == K::Num0 || kp.code == K::Numpad0) return 0;
    return -1;
}

int JeuSFML::calculerDegats(int hpMax) {
    std::uniform_int_distribution<int> dist(0, hpMax);
    return dist(rng);
}

void JeuSFML::appliquerDrop(const Monstre* monstre) {
    std::uniform_int_distribution<int> roll(1, 100);
    int r = roll(rng);
    const std::string& cat = monstre->getCategorie();
    std::string nom, type;
    int valeur;

    if (cat == "NORMAL") {
        if (r > 40) return;
        nom = "Snack"; type = "HEAL"; valeur = 8;
    } else if (cat == "MINIBOSS") {
        if (r > 60) return;
        nom = "Cristal de Mercy"; type = "MERCY_BOOST"; valeur = 25;
    } else {
        if (r > 80) return;
        nom = "SuperPotion"; type = "HEAL"; valeur = 30;
    }
    Item drop(nom, type, valeur, 1);
    joueur.ajouterItem(drop);
    messageLog += "\n  " + monstre->getNom() + " a laisse tomber : " + nom + " !";
}

// ─── TITLE ────────────────────────────────────────────────────────────────────

void JeuSFML::renderTitre() {
    // Boîte centrale
    float bw = 600.f, bh = 180.f;
    float bx = (WIN_W - bw) / 2.f, by = 160.f;
    sf::RectangleShape boite({bw, bh});
    boite.setPosition({bx, by});
    boite.setFillColor(sf::Color(10, 10, 30));
    boite.setOutlineColor(sf::Color::Cyan);
    boite.setOutlineThickness(2.f);
    window.draw(boite);

    // Titre
    sf::Text titre(font, "ALTERDUNE", FONT_XL);
    titre.setFillColor(sf::Color::Cyan);
    auto tb = titre.getLocalBounds();
    titre.setOrigin({tb.size.x / 2.f, 0.f});
    titre.setPosition({WIN_W / 2.f, by + 20.f});
    window.draw(titre);

    // Sous-titre
    sf::Text sub(font, "Un RPG en C++", FONT_MD);
    sub.setFillColor(sf::Color(180, 180, 180));
    auto sb = sub.getLocalBounds();
    sub.setOrigin({sb.size.x / 2.f, 0.f});
    sub.setPosition({WIN_W / 2.f, by + 110.f});
    window.draw(sub);

    // Texte clignotant
    if ((frameCount / 30) % 2 == 0) {
        sf::Text hint(font, "Appuyez sur Entree pour commencer", FONT_SM);
        hint.setFillColor(sf::Color(150, 150, 150));
        auto hb = hint.getLocalBounds();
        hint.setOrigin({hb.size.x / 2.f, 0.f});
        hint.setPosition({WIN_W / 2.f, by + bh + 40.f});
        window.draw(hint);
    }
}

void JeuSFML::handleEventTitre(const sf::Event& e) {
    if (auto* kp = e.getIf<sf::Event::KeyPressed>())
        if (kp->code == sf::Keyboard::Key::Enter)
            etat = GameState::NAME_INPUT;
}

// ─── NAME INPUT ───────────────────────────────────────────────────────────────

void JeuSFML::renderNomInput() {
    dessinerTexte("Entrez le nom de votre personnage :", FONT_MD,
                  sf::Color::White, MARGIN, 200.f);

    // Boîte de saisie
    float bw = 420.f, bh = 42.f;
    float bx = MARGIN, by = 250.f;
    sf::RectangleShape boite({bw, bh});
    boite.setPosition({bx, by});
    boite.setFillColor(sf::Color(20, 20, 40));
    boite.setOutlineColor(sf::Color::Cyan);
    boite.setOutlineThickness(1.5f);
    window.draw(boite);

    // Texte saisi + curseur clignotant
    std::string affiche = inputBuffer;
    if ((frameCount / 20) % 2 == 0) affiche += "_";
    dessinerTexte(affiche, FONT_MD, sf::Color::Cyan, bx + 10.f, by + 8.f);

    dessinerTexte("Entree pour valider", FONT_SM,
                  sf::Color(120, 120, 120), MARGIN, 330.f);
}

void JeuSFML::handleEventNomInput(const sf::Event& e) {
    if (auto* te = e.getIf<sf::Event::TextEntered>()) {
        char32_t c = te->unicode;
        if (c == 8) {
            if (!inputBuffer.empty()) inputBuffer.pop_back();
        } else if (c >= 32 && c < 127 && inputBuffer.size() < 20) {
            inputBuffer += static_cast<char>(c);
        }
    }
    if (auto* kp = e.getIf<sf::Event::KeyPressed>()) {
        if (kp->code == sf::Keyboard::Key::Enter && !inputBuffer.empty()) {
            joueur = Joueur(inputBuffer, 100);
            std::vector<Item> items = ChargeurCSV::chargerItems("items.csv");
            for (Item& item : items) joueur.ajouterItem(item);
            inputBuffer.clear();
            etat = GameState::RECAP;
        }
    }
}

// ─── RECAP ────────────────────────────────────────────────────────────────────

void JeuSFML::renderRecap() {
    float y = 40.f;
    dessinerTexte("=== Recapitulatif de depart ===", FONT_LG,
                  sf::Color::White, MARGIN, y);
    y += 50.f;
    dessinerSeparateur(y);
    y += 15.f;

    dessinerTexte("Personnage : " + joueur.getNom(), FONT_MD,
                  sf::Color::Cyan, MARGIN, y); y += LINE_MD;
    dessinerTexte("HP : " + std::to_string(joueur.getHp()) + " / " +
                  std::to_string(joueur.getHpMax()), FONT_MD,
                  sf::Color::Green, MARGIN, y); y += LINE_MD;
    dessinerTexte("Victoires : 0/10", FONT_MD, sf::Color::White, MARGIN, y);
    y += LINE_MD + 10.f;

    dessinerTexte("Inventaire de depart :", FONT_MD,
                  sf::Color(180, 180, 180), MARGIN, y); y += LINE_MD;

    const std::vector<Item>& inv = const_cast<Joueur&>(joueur).getInventaire();
    for (int i = 0; i < (int)inv.size(); ++i) {
        std::string ligne = "  " + std::to_string(i + 1) + ". " +
                            inv[i].getNom() +
                            "  (" + inv[i].getType() + "  +" +
                            std::to_string(inv[i].getValeur()) + ")" +
                            "  x" + std::to_string(inv[i].getQuantite());
        dessinerTexte(ligne, FONT_SM, sf::Color::White, MARGIN, y);
        y += LINE_SM;
    }

    y += 20.f;
    dessinerSeparateur(y);

    if ((frameCount / 30) % 2 == 0)
        dessinerTexte("Appuyez sur Entree pour commencer", FONT_SM,
                      sf::Color(150, 150, 150), MARGIN, y + 20.f);
}

void JeuSFML::handleEventRecap(const sf::Event& e) {
    if (auto* kp = e.getIf<sf::Event::KeyPressed>())
        if (kp->code == sf::Keyboard::Key::Enter)
            etat = GameState::MAIN_MENU;
}

// ─── MENU PRINCIPAL ───────────────────────────────────────────────────────────

void JeuSFML::renderMenuPrincipal() {
    float y = 30.f;
    dessinerTexte("=== MENU PRINCIPAL ===", FONT_LG,
                  sf::Color::Cyan, MARGIN, y); y += 45.f;

    // Barre d'état joueur
    std::string etatJoueur = joueur.getNom() +
        "   HP : " + std::to_string(joueur.getHp()) + "/" +
        std::to_string(joueur.getHpMax()) +
        "   Victoires : " + std::to_string(joueur.getNbVictoires()) + "/10";
    dessinerTexte(etatJoueur, FONT_SM, sf::Color::White, MARGIN, y);
    y += 24.f;

    // Mini-barre HP
    dessinerBarre(MARGIN, y, 250.f, 10.f,
                  joueur.getHp(), joueur.getHpMax(),
                  couleurHP(joueur.getHp(), joueur.getHpMax()));
    y += 24.f;

    dessinerSeparateur(y); y += 20.f;

    const std::vector<std::string> options = {
        "  1. Bestiaire",
        "  2. Demarrer un combat",
        "  3. Statistiques",
        "  4. Items",
        "  5. Quitter"
    };
    for (const auto& opt : options) {
        dessinerTexte(opt, FONT_MD, sf::Color::White, MARGIN, y);
        y += LINE_MD + 4.f;
    }

    dessinerTexte("Appuyez sur 1-5", FONT_SM,
                  sf::Color(100, 100, 100), MARGIN, (float)WIN_H - 40.f);
}

void JeuSFML::handleEventMenuPrincipal(const sf::Event& e) {
    // Lecture du choix depuis KeyPressed (pavé principal + numérique)
    // OU depuis TextEntered (fallback macOS où Num1-Num5 ne génèrent pas de KeyPressed)
    int choix = -1;
    if (auto* kp = e.getIf<sf::Event::KeyPressed>()) {
        using K = sf::Keyboard::Key;
        if (kp->code >= K::Num1 && kp->code <= K::Num5)
            choix = (int)kp->code - (int)K::Num0;
        else if (kp->code >= K::Numpad1 && kp->code <= K::Numpad5)
            choix = (int)kp->code - (int)K::Numpad0;
    }
    if (auto* te = e.getIf<sf::Event::TextEntered>()) {
        char32_t c = te->unicode;
        if (c >= U'1' && c <= U'5') choix = (int)(c - U'0');
    }

    switch (choix) {
        case 1:
            scrollOffset = 0;
            etat = GameState::BESTIARY;
            break;
        case 2:
            if (!monstresDisponibles.empty()) {
                std::uniform_int_distribution<int> dist(0, (int)monstresDisponibles.size() - 1);
                monstreCourantIdx = monstresDisponibles[dist(rng)];
                if (monstreCourant) { delete monstreCourant; }
                monstreCourant = monstres[monstreCourantIdx]->cloner();
                tourConsomme = false;
                hitAnimStart = -1;
                messageLog.clear();
                etat = GameState::COMBAT_START;
            }
            break;
        case 3:
            etat = GameState::STATS;
            break;
        case 4:
            etat = GameState::INVENTORY;
            break;
        case 5:
            window.close();
            break;
        default: break;
    }
}

// ─── BESTIAIRE ────────────────────────────────────────────────────────────────

void JeuSFML::renderBestiaire() {
    float y = 30.f;
    dessinerTexte("=== Bestiaire ===", FONT_LG,
                  sf::Color::White, MARGIN, y); y += 45.f;
    dessinerSeparateur(y); y += 15.f;

    if (bestiaireLog.empty()) {
        dessinerTexte("Aucun monstre vaincu pour l'instant.", FONT_MD,
                      sf::Color(180, 180, 180), MARGIN, y);
    } else {
        dessinerLignes(bestiaireLog, MARGIN, y, FONT_SM,
                       sf::Color::White, scrollOffset, MAX_VIS);
    }

    dessinerSeparateur((float)WIN_H - 50.f);
    dessinerTexte("[Haut/Bas] Defiler     [Entree] Retour", FONT_SM,
                  sf::Color(100, 100, 100), MARGIN, (float)WIN_H - 38.f);
}

void JeuSFML::handleEventBestiaire(const sf::Event& e) {
    if (auto* kp = e.getIf<sf::Event::KeyPressed>()) {
        if (kp->code == sf::Keyboard::Key::Enter)
            etat = GameState::MAIN_MENU;
        if (kp->code == sf::Keyboard::Key::Up)
            scrollOffset = std::max(0, scrollOffset - 1);
        if (kp->code == sf::Keyboard::Key::Down) {
            int maxOff = std::max(0, (int)bestiaireLog.size() - MAX_VIS);
            scrollOffset = std::min(maxOff, scrollOffset + 1);
        }
    }
}

// ─── STATS ────────────────────────────────────────────────────────────────────

void JeuSFML::renderStats() {
    float y = 30.f;
    dessinerTexte("=== Stats de " + joueur.getNom() + " ===", FONT_LG,
                  sf::Color::Cyan, MARGIN, y); y += 45.f;
    dessinerSeparateur(y); y += 20.f;

    auto ligne = [&](const std::string& label, const std::string& val, sf::Color c) {
        dessinerTexte(label + val, FONT_MD, c, MARGIN, y);
        y += LINE_MD;
    };

    ligne("HP         : ", std::to_string(joueur.getHp()) + " / " +
          std::to_string(joueur.getHpMax()),
          couleurHP(joueur.getHp(), joueur.getHpMax()));
    ligne("Victoires  : ", std::to_string(joueur.getNbVictoires()) + "/10",
          sf::Color::White);
    ligne("Monstres tues    : ", std::to_string(joueur.getNbTues()),
          sf::Color::Red);
    ligne("Monstres epargnes: ", std::to_string(joueur.getNbEpargnes()),
          sf::Color::Green);

    dessinerSeparateur((float)WIN_H - 50.f);
    dessinerTexte("[Entree] Retour", FONT_SM,
                  sf::Color(100, 100, 100), MARGIN, (float)WIN_H - 38.f);
}

void JeuSFML::handleEventStats(const sf::Event& e) {
    if (auto* kp = e.getIf<sf::Event::KeyPressed>())
        if (kp->code == sf::Keyboard::Key::Enter)
            etat = GameState::MAIN_MENU;
}

// ─── INVENTAIRE (hors combat) ─────────────────────────────────────────────────

void JeuSFML::renderInventaire() {
    float y = 30.f;
    dessinerTexte("=== INVENTAIRE ===", FONT_LG,
                  sf::Color::White, MARGIN, y); y += 45.f;
    dessinerSeparateur(y); y += 15.f;

    const std::vector<Item>& inv = const_cast<Joueur&>(joueur).getInventaire();
    if (inv.empty()) {
        dessinerTexte("Inventaire vide.", FONT_MD,
                      sf::Color(180, 180, 180), MARGIN, y);
    } else {
        for (int i = 0; i < (int)inv.size(); ++i) {
            sf::Color c = inv[i].estDisponible() ? sf::Color::White
                                                 : sf::Color(100, 100, 100);
            std::string ligne = "  " + std::to_string(i + 1) + ". " +
                                inv[i].getNom() +
                                "  [" + inv[i].getType() + " +" +
                                std::to_string(inv[i].getValeur()) + "]" +
                                "  x" + std::to_string(inv[i].getQuantite());
            dessinerTexte(ligne, FONT_MD, c, MARGIN, y);
            y += LINE_MD;
        }
    }

    dessinerSeparateur((float)WIN_H - 50.f);
    dessinerTexte("Touche 1-" + std::to_string((int)inv.size()) +
                  " pour utiliser  |  [0] ou [Entree] Retour",
                  FONT_SM, sf::Color(100, 100, 100),
                  MARGIN, (float)WIN_H - 38.f);
}

void JeuSFML::handleEventInventaire(const sf::Event& e) {
    if (auto* kp = e.getIf<sf::Event::KeyPressed>()) {
        if (kp->code == sf::Keyboard::Key::Enter ||
            kp->code == sf::Keyboard::Key::Num0  ||
            kp->code == sf::Keyboard::Key::Numpad0) {
            etat = GameState::MAIN_MENU;
            return;
        }
        int idx = keyToInt(*kp);
        if (idx > 0) joueur.utiliserItem(idx - 1);
    }
    if (auto* te = e.getIf<sf::Event::TextEntered>()) {
        char32_t c = te->unicode;
        if (c == U'0') { etat = GameState::MAIN_MENU; return; }
        if (c >= U'1' && c <= U'9') joueur.utiliserItem((int)(c - U'1'));
    }
}

// ─── COMBAT START ─────────────────────────────────────────────────────────────

void JeuSFML::renderCombatStart() {
    // Aperçu du monstre centré — position ajustée selon la taille
    float cyM = (monstreCourant->getCategorie() == "BOSS") ? 200.f :
                (monstreCourant->getCategorie() == "MINIBOSS") ? 175.f : 155.f;
    dessinerMonstre(monstreCourant, WIN_W * 0.5f, cyM);

    float cy = 220.f;
    dessinerSeparateur(cy, sf::Color::Red); cy += 20.f;

    std::string titre = "COMBAT : " + joueur.getNom() +
                        " vs " + monstreCourant->getNom();
    sf::Text t(font, titre, FONT_LG);
    t.setFillColor(sf::Color::Red);
    auto tb = t.getLocalBounds();
    t.setOrigin({tb.size.x / 2.f, 0.f});
    t.setPosition({WIN_W / 2.f, cy});
    window.draw(t);
    cy += 50.f;

    std::string cat = "[" + monstreCourant->getCategorie() + "]";
    sf::Text tc(font, cat, FONT_MD);
    tc.setFillColor(sf::Color(180, 100, 100));
    auto tcb = tc.getLocalBounds();
    tc.setOrigin({tcb.size.x / 2.f, 0.f});
    tc.setPosition({WIN_W / 2.f, cy});
    window.draw(tc);
    cy += 40.f;

    dessinerSeparateur(cy, sf::Color::Red);

    if ((frameCount / 30) % 2 == 0)
        dessinerTexte("Appuyez sur Entree pour commencer", FONT_SM,
                      sf::Color(150, 150, 150), MARGIN, (float)WIN_H - 60.f);
}

void JeuSFML::handleEventCombatStart(const sf::Event& e) {
    if (auto* kp = e.getIf<sf::Event::KeyPressed>())
        if (kp->code == sf::Keyboard::Key::Enter)
            etat = GameState::COMBAT_MAIN;
}

// ─── COMBAT PRINCIPAL ─────────────────────────────────────────────────────────

void JeuSFML::renderCombatPrincipal() {
    float y = 30.f;
    dessinerSeparateur(y); y += 15.f;

    // ── Joueur ──────────────────────────────────────────────────────────────────
    dessinerTexte("JOUEUR : " + joueur.getNom(), FONT_MD,
                  sf::Color::Cyan, MARGIN, y); y += LINE_MD;
    dessinerTexte("HP", FONT_SM, sf::Color::White, MARGIN, y + 2.f);
    dessinerBarre(MARGIN + 55.f, y, 250.f, 16.f,
                  joueur.getHp(), joueur.getHpMax(),
                  couleurHP(joueur.getHp(), joueur.getHpMax()));
    dessinerTexte(std::to_string(joueur.getHp()) + "/" +
                  std::to_string(joueur.getHpMax()),
                  FONT_SM, sf::Color::White, MARGIN + 315.f, y + 2.f);
    y += 30.f;

    // ── Monstre ─────────────────────────────────────────────────────────────────
    dessinerTexte("MONSTRE : " + monstreCourant->getNom() +
                  "  [" + monstreCourant->getCategorie() + "]",
                  FONT_MD, sf::Color(180, 100, 220), MARGIN, y); y += LINE_MD;

    dessinerTexte("HP", FONT_SM, sf::Color::White, MARGIN, y + 2.f);
    dessinerBarre(MARGIN + 55.f, y, 250.f, 16.f,
                  monstreCourant->getHp(), monstreCourant->getHpMax(),
                  couleurHP(monstreCourant->getHp(), monstreCourant->getHpMax()));
    dessinerTexte(std::to_string(monstreCourant->getHp()) + "/" +
                  std::to_string(monstreCourant->getHpMax()),
                  FONT_SM, sf::Color::White, MARGIN + 315.f, y + 2.f);
    y += 25.f;

    dessinerTexte("Mercy", FONT_SM, sf::Color::Yellow, MARGIN, y + 2.f);
    dessinerBarre(MARGIN + 55.f, y, 250.f, 16.f,
                  monstreCourant->getMercy(), monstreCourant->getMercyGoal(),
                  sf::Color::Yellow);
    dessinerTexte(std::to_string(monstreCourant->getMercy()) + "/" +
                  std::to_string(monstreCourant->getMercyGoal()),
                  FONT_SM, sf::Color::Yellow, MARGIN + 315.f, y + 2.f);
    y += 30.f;

    dessinerSeparateur(y); y += 20.f;

    // ── Menu d'actions ──────────────────────────────────────────────────────────
    float cx = MARGIN;
    float actionY = y;
    struct Action { std::string label; sf::Color col; };
    const Action actions[4] = {
        {"[1] FIGHT",  sf::Color::Red},
        {"[2] ACT",    sf::Color::Green},
        {"[3] ITEM",   sf::Color::Yellow},
        {"[4] MERCY",  sf::Color::Cyan}
    };
    for (const auto& a : actions) {
        dessinerTexte(a.label, FONT_MD, a.col, cx, actionY);
        cx += 200.f;
    }
    y = actionY + LINE_MD + 15.f;

    // ── Zone log ────────────────────────────────────────────────────────────────
    float logH = (float)WIN_H - y - 20.f;
    sf::RectangleShape fond({(float)WIN_W - 2.f * MARGIN, logH});
    fond.setPosition({MARGIN, y});
    fond.setFillColor(sf::Color(20, 20, 20));
    fond.setOutlineColor(sf::Color(60, 60, 60));
    fond.setOutlineThickness(1.f);
    window.draw(fond);

    // Monstre centré horizontalement dans la zone grise
    if (monstreCourant) {
        float mcx = WIN_W * 0.5f;
        float mcy = y + logH * 0.58f;

        int hitElapsed = (hitAnimStart >= 0) ? (frameCount - hitAnimStart) : 999;
        if (hitElapsed >= 45) hitAnimStart = -1;

        // Tremblement pendant les 15 premières frames
        float shake = 0.f;
        if (hitElapsed < 15)
            shake = sinf(hitElapsed * 1.8f) * (15 - hitElapsed) * 0.85f;

        dessinerMonstre(monstreCourant, mcx + shake, mcy);

        // Flash rouge sur le monstre (fondu rapide)
        if (hitElapsed < 18) {
            float alpha = 190.f * (1.f - hitElapsed / 18.f);
            float sc = std::max(0.55f, std::min(0.55f + (float)monstreCourant->getHpMax() / 100.f * 0.85f, 1.75f));
            float r = 58.f * sc;
            sf::RectangleShape flash({r * 2.2f, r * 2.4f});
            flash.setPosition({mcx + shake - r * 1.1f, mcy - r * 1.5f});
            flash.setFillColor(sf::Color(255, 35, 10, (int)alpha));
            window.draw(flash);
        }

        // Chiffre de dégâts flottant (monte et disparaît)
        if (hitElapsed < 45 && hitDamage > 0) {
            uint8_t alpha = (uint8_t)std::max(0, 255 - hitElapsed * 6);
            float floatY = mcy - 70.f - hitElapsed * 2.8f;
            dessinerTexte("-" + std::to_string(hitDamage),
                         FONT_XL, sf::Color(255, 70, 40, alpha),
                         mcx - 22.f, floatY);
        }
    }

    // Affiche les derniers messages du tour précédent
    if (!messageLog.empty()) {
        std::istringstream ss(messageLog);
        std::string line;
        float ly = y + 8.f;
        while (std::getline(ss, line) && ly < y + logH - 10.f) {
            dessinerTexte(line, FONT_SM, sf::Color(200, 200, 200), MARGIN + 10.f, ly);
            ly += LINE_SM;
        }
    }
}

void JeuSFML::handleEventCombatPrincipal(const sf::Event& e) {
    // Bloquer l'input pendant l'animation de coup
    if (hitAnimStart >= 0 && frameCount - hitAnimStart < 45) return;

    int choix = -1;
    if (auto* kp = e.getIf<sf::Event::KeyPressed>())
        choix = keyToInt(*kp);
    else if (auto* te = e.getIf<sf::Event::TextEntered>()) {
        char32_t c = te->unicode;
        if (c >= U'0' && c <= U'9') choix = (int)(c - U'0');
    }
    if (choix >= 1 && choix <= 4) {

        messageLog.clear();
        tourConsomme = true;

        // ── FIGHT ──────────────────────────────────────────────────────────────
        if (choix == 1) {
            int degats = calculerDegats(monstreCourant->getHpMax());
            hitAnimStart = frameCount;
            hitDamage    = degats;
            if (degats == 0) {
                messageLog += "  Votre attaque rate !";
            } else {
                monstreCourant->subirDegats(degats);
                messageLog += "  Vous infligez " + std::to_string(degats) +
                              " degats a " + monstreCourant->getNom() + ".";
            }
            if (!monstreCourant->estVivant()) {
                appliquerDrop(monstreCourant);
                joueur.ajouterVictoire();
                joueur.ajouterTue();

                // Enregistrement bestiaire
                bestiaireLog.push_back("--- " + monstreCourant->getNom() + " ---");
                bestiaireLog.push_back("  [" + monstreCourant->getCategorie() + "]  Resultat : TUE");
                bestiaireLog.push_back("  HP max : " + std::to_string(monstreCourant->getHpMax()) +
                                       "  ATK : " + std::to_string(monstreCourant->getAtk()));
                bestiaireLog.push_back("");
                bestiaire.ajouterMonstre(monstreCourant, "Tue");
                monstresDisponibles.erase(
                    std::remove(monstresDisponibles.begin(), monstresDisponibles.end(), monstreCourantIdx),
                    monstresDisponibles.end());
                monstreCourant = nullptr;

                messageLog += "\n  Monstre vaincu !";
                messageLog += "\n  Victoires : " + std::to_string(joueur.getNbVictoires()) + "/10";
                tourConsomme = false; // pas de contre-attaque
                prochainEtat = (joueur.getNbVictoires() >= 10)
                               ? GameState::END_SCREEN : GameState::MAIN_MENU;
                etat = GameState::COMBAT_RESULT;
                return;
            }

        // ── ACT ────────────────────────────────────────────────────────────────
        } else if (choix == 2) {
            actionsDisponibles = monstreCourant->getActions();
            nbActionsMax = std::min(monstreCourant->getNbActionsMax(),
                                    (int)actionsDisponibles.size());
            actionsDisponibles.resize(nbActionsMax);
            etat = GameState::COMBAT_ACT;
            return;

        // ── ITEM ───────────────────────────────────────────────────────────────
        } else if (choix == 3) {
            etat = GameState::COMBAT_ITEM;
            return;

        // ── MERCY ──────────────────────────────────────────────────────────────
        } else {
            if (monstreCourant->peutEtreEpargne()) {
                appliquerDrop(monstreCourant);
                joueur.ajouterVictoire();
                joueur.ajouterEpargne();

                bestiaireLog.push_back("--- " + monstreCourant->getNom() + " ---");
                bestiaireLog.push_back("  [" + monstreCourant->getCategorie() + "]  Resultat : EPARGNE");
                bestiaireLog.push_back("  HP max : " + std::to_string(monstreCourant->getHpMax()) +
                                       "  ATK : " + std::to_string(monstreCourant->getAtk()));
                bestiaireLog.push_back("");
                bestiaire.ajouterMonstre(monstreCourant, "Epargne");
                monstresDisponibles.erase(
                    std::remove(monstresDisponibles.begin(), monstresDisponibles.end(), monstreCourantIdx),
                    monstresDisponibles.end());
                monstreCourant = nullptr;

                messageLog += "  Monstre epargne ! Vous gagnez !";
                messageLog += "\n  Victoires : " + std::to_string(joueur.getNbVictoires()) + "/10";
                tourConsomme = false;
                prochainEtat = (joueur.getNbVictoires() >= 10)
                               ? GameState::END_SCREEN : GameState::MAIN_MENU;
                etat = GameState::COMBAT_RESULT;
                return;
            } else {
                messageLog += "  Mercy insuffisante. (" +
                              std::to_string(monstreCourant->getMercy()) + "/" +
                              std::to_string(monstreCourant->getMercyGoal()) + ")";
                tourConsomme = false;
                // Rester en COMBAT_MAIN — message affiché dans le log
                return;
            }
        }

        // ── Tour du monstre ────────────────────────────────────────────────────
        if (tourConsomme && monstreCourant) {
            bool hesitation = false;
            if (monstreCourant->getMercyGoal() > 0) {
                float ratio = (float)monstreCourant->getMercy() /
                              (float)monstreCourant->getMercyGoal();
                int seuil = 0;
                if (ratio >= 1.0f)       seuil = 75;
                else if (ratio >= 0.66f) seuil = 50;
                else if (ratio >= 0.33f) seuil = 25;
                if (seuil > 0) {
                    std::uniform_int_distribution<int> roll(1, 100);
                    hesitation = roll(rng) <= seuil;
                }
            }
            if (hesitation) {
                messageLog += "\n  " + monstreCourant->getNom() +
                              " hesite et ne vous attaque pas !";
            } else {
                int degats = monstreCourant->getAtk();
                joueur.subirDegats(degats);
                messageLog += "\n  " + monstreCourant->getNom() +
                              " vous inflige " + std::to_string(degats) + " degats !";
            }
            if (!joueur.estVivant()) {
                messageLog += "\n\n  " + joueur.getNom() +
                              " tombe au combat. Game Over...";
                prochainEtat = GameState::GAME_OVER;
                etat = GameState::COMBAT_RESULT;
                return;
            }
        }

        // Rester en COMBAT_MAIN — les HP et le log sont mis à jour sur place
    }
}

// ─── COMBAT ACT ───────────────────────────────────────────────────────────────

void JeuSFML::renderCombatACT() {
    float y = 30.f;
    dessinerTexte("=== Actions disponibles ===", FONT_LG,
                  sf::Color::Green, MARGIN, y); y += 45.f;
    dessinerSeparateur(y); y += 15.f;

    const auto& catalogue = ActionACT::getCatalogue();
    for (int i = 0; i < nbActionsMax; ++i) {
        const std::string& id = actionsDisponibles[i];
        std::string ligne = "  " + std::to_string(i + 1) + ". " + id;
        auto it = catalogue.find(id);
        if (it != catalogue.end()) {
            int impact = it->second.getImpactMercy();
            ligne += "  (Mercy " + (impact >= 0 ? std::string("+") : std::string("")) +
                     std::to_string(impact) + ")";
        }
        dessinerTexte(ligne, FONT_MD, sf::Color::White, MARGIN, y);
        y += LINE_MD;
    }

    dessinerSeparateur((float)WIN_H - 50.f);
    dessinerTexte("Appuyez sur 1-" + std::to_string(nbActionsMax) + " pour choisir",
                  FONT_SM, sf::Color(100, 100, 100), MARGIN, (float)WIN_H - 38.f);
}

void JeuSFML::handleEventCombatACT(const sf::Event& e) {
    int idx = -1;
    if (auto* kp = e.getIf<sf::Event::KeyPressed>())
        idx = keyToInt(*kp);
    else if (auto* te = e.getIf<sf::Event::TextEntered>()) {
        char32_t c = te->unicode;
        if (c >= U'0' && c <= U'9') idx = (int)(c - U'0');
    }
    if (idx >= 1 && idx <= nbActionsMax) {
        const std::string& actionId = actionsDisponibles[idx - 1];
        const auto& catalogue = ActionACT::getCatalogue();
        auto it = catalogue.find(actionId);
        if (it != catalogue.end()) {
            int impact = it->second.getImpactMercy();
            monstreCourant->modifierMercy(impact);
            messageLog += "  Action : " + actionId;
            if (impact > 0)
                messageLog += "\n  Mercy +" + std::to_string(impact) +
                              "  ->  " + std::to_string(monstreCourant->getMercy()) +
                              "/" + std::to_string(monstreCourant->getMercyGoal());
            else if (impact < 0)
                messageLog += "\n  Mercy " + std::to_string(impact) +
                              "  ->  " + std::to_string(monstreCourant->getMercy()) +
                              "/" + std::to_string(monstreCourant->getMercyGoal());
            else
                messageLog += "\n  " + monstreCourant->getNom() + " reste indifferent.";
        }

        // Tour du monstre
        bool hesitation = false;
        if (monstreCourant->getMercyGoal() > 0) {
            float ratio = (float)monstreCourant->getMercy() /
                          (float)monstreCourant->getMercyGoal();
            int seuil = 0;
            if (ratio >= 1.0f)       seuil = 75;
            else if (ratio >= 0.66f) seuil = 50;
            else if (ratio >= 0.33f) seuil = 25;
            if (seuil > 0) {
                std::uniform_int_distribution<int> roll(1, 100);
                hesitation = roll(rng) <= seuil;
            }
        }
        if (hesitation) {
            messageLog += "\n  " + monstreCourant->getNom() +
                          " hesite et ne vous attaque pas !";
        } else {
            int degats = monstreCourant->getAtk();
            joueur.subirDegats(degats);
            messageLog += "\n  " + monstreCourant->getNom() +
                          " vous inflige " + std::to_string(degats) + " degats !";
        }
        if (!joueur.estVivant()) {
            messageLog += "\n\n  Game Over...";
            prochainEtat = GameState::GAME_OVER;
        } else {
            prochainEtat = GameState::COMBAT_MAIN;
        }
        etat = GameState::COMBAT_RESULT;
    }
}

// ─── COMBAT ITEM ──────────────────────────────────────────────────────────────

void JeuSFML::renderCombatItem() {
    float y = 30.f;
    dessinerTexte("=== ITEM (Combat) ===", FONT_LG,
                  sf::Color::Yellow, MARGIN, y); y += 45.f;
    dessinerSeparateur(y); y += 15.f;

    const std::vector<Item>& inv = const_cast<Joueur&>(joueur).getInventaire();
    if (inv.empty()) {
        dessinerTexte("Inventaire vide.", FONT_MD,
                      sf::Color(180, 180, 180), MARGIN, y);
    } else {
        for (int i = 0; i < (int)inv.size(); ++i) {
            sf::Color c = inv[i].estDisponible() ? sf::Color::White
                                                 : sf::Color(100, 100, 100);
            std::string ligne = "  " + std::to_string(i + 1) + ". " +
                                inv[i].getNom() +
                                "  [" + inv[i].getType() + " +" +
                                std::to_string(inv[i].getValeur()) + "]" +
                                "  x" + std::to_string(inv[i].getQuantite());
            dessinerTexte(ligne, FONT_MD, c, MARGIN, y);
            y += LINE_MD;
        }
    }

    dessinerSeparateur((float)WIN_H - 50.f);
    dessinerTexte("[0] ou [Entree] Annuler  |  Touche 1-N Utiliser", FONT_SM,
                  sf::Color(100, 100, 100), MARGIN, (float)WIN_H - 38.f);
}

void JeuSFML::handleEventCombatItem(const sf::Event& e) {
    // Annulation : Enter ou touche 0
    if (auto* kp = e.getIf<sf::Event::KeyPressed>()) {
        if (kp->code == sf::Keyboard::Key::Enter ||
            kp->code == sf::Keyboard::Key::Num0  ||
            kp->code == sf::Keyboard::Key::Numpad0) {
            etat = GameState::COMBAT_MAIN;
            return;
        }
    }
    if (auto* te = e.getIf<sf::Event::TextEntered>()) {
        if (te->unicode == U'0') { etat = GameState::COMBAT_MAIN; return; }
    }

    // Sélection d'item depuis KeyPressed OU TextEntered
    int idx = -1;
    if (auto* kp = e.getIf<sf::Event::KeyPressed>())
        idx = keyToInt(*kp);
    else if (auto* te = e.getIf<sf::Event::TextEntered>()) {
        char32_t c = te->unicode;
        if (c >= U'1' && c <= U'9') idx = (int)(c - U'0');
    }
    if (idx >= 1) {
        int realIdx = idx - 1;
        std::vector<Item>& inv = joueur.getInventaire();
        if (realIdx >= (int)inv.size()) return;
        if (!inv[realIdx].estDisponible()) {
            messageLog = "  Plus de " + inv[realIdx].getNom() + " disponible !";
            prochainEtat = GameState::COMBAT_MAIN;
            etat = GameState::COMBAT_RESULT;
            return;
        }
        if (inv[realIdx].getType() == "MERCY_BOOST") {
            int gain = inv[realIdx].getValeur();
            monstreCourant->modifierMercy(gain);
            inv[realIdx].reduireQuantite();
            messageLog = "  Vous utilisez " + inv[realIdx].getNom() +
                         " : Mercy +" + std::to_string(gain) +
                         "  ->  " + std::to_string(monstreCourant->getMercy()) +
                         "/" + std::to_string(monstreCourant->getMercyGoal());
        } else {
            joueur.utiliserItem(realIdx);
            messageLog = "  Item utilise.";
        }
        // Tour du monstre
        int degats = monstreCourant->getAtk();
        joueur.subirDegats(degats);
        messageLog += "\n  " + monstreCourant->getNom() +
                      " vous inflige " + std::to_string(degats) + " degats !";
        if (!joueur.estVivant()) {
            messageLog += "\n\n  Game Over...";
            prochainEtat = GameState::GAME_OVER;
        } else {
            prochainEtat = GameState::COMBAT_MAIN;
        }
        etat = GameState::COMBAT_RESULT;
    }
}

// ─── COMBAT RÉSULTAT ──────────────────────────────────────────────────────────

void JeuSFML::renderCombatResultat() {
    float y = 80.f;
    dessinerSeparateur(y); y += 20.f;

    // Affiche le log du tour
    std::istringstream ss(messageLog);
    std::string line;
    while (std::getline(ss, line)) {
        sf::Color c = sf::Color::White;
        if (line.find("degats") != std::string::npos)  c = sf::Color::Red;
        if (line.find("Mercy")  != std::string::npos)  c = sf::Color::Yellow;
        if (line.find("laisse") != std::string::npos)  c = sf::Color::Green;
        if (line.find("vaincu") != std::string::npos ||
            line.find("epargne") != std::string::npos) c = sf::Color::Green;
        dessinerTexte(line, FONT_MD, c, MARGIN, y);
        y += LINE_MD;
    }

    y += 20.f;
    dessinerSeparateur(y);

    if ((frameCount / 30) % 2 == 0)
        dessinerTexte("Appuyez sur Entree pour continuer", FONT_SM,
                      sf::Color(150, 150, 150), MARGIN, (float)WIN_H - 60.f);
}

void JeuSFML::handleEventCombatResultat(const sf::Event& e) {
    if (auto* kp = e.getIf<sf::Event::KeyPressed>()) {
        if (kp->code == sf::Keyboard::Key::Enter) {
            messageLog.clear();
            etat = prochainEtat;
        }
    }
}

// ─── FIN DE PARTIE ────────────────────────────────────────────────────────────

void JeuSFML::renderFin() {
    float y = 40.f;
    dessinerSeparateur(y, sf::Color::White); y += 20.f;

    dessinerTexte("FIN DE PARTIE", FONT_XL, sf::Color::White, MARGIN, y); y += 70.f;
    dessinerSeparateur(y); y += 20.f;

    // Stats
    dessinerTexte("Personnage  : " + joueur.getNom(), FONT_MD,
                  sf::Color::Cyan, MARGIN, y); y += LINE_MD;
    dessinerTexte("HP          : " + std::to_string(joueur.getHp()) + "/" +
                  std::to_string(joueur.getHpMax()), FONT_MD,
                  couleurHP(joueur.getHp(), joueur.getHpMax()), MARGIN, y); y += LINE_MD;
    dessinerTexte("Victoires   : " + std::to_string(joueur.getNbVictoires()) + "/10",
                  FONT_MD, sf::Color::White, MARGIN, y); y += LINE_MD;
    dessinerTexte("Tues        : " + std::to_string(joueur.getNbTues()),
                  FONT_MD, sf::Color::Red, MARGIN, y); y += LINE_MD;
    dessinerTexte("Epargnes    : " + std::to_string(joueur.getNbEpargnes()),
                  FONT_MD, sf::Color::Green, MARGIN, y); y += LINE_MD + 15.f;

    dessinerSeparateur(y); y += 20.f;

    // Type de fin
    if (!joueur.estVivant()) {
        dessinerTexte("Vous avez ete vaincu.", FONT_LG,
                      sf::Color::Red, MARGIN, y); y += 40.f;
        dessinerTexte("Le monde d'ALTERDUNE vous a eu...", FONT_MD,
                      sf::Color(200, 100, 100), MARGIN, y);
    } else if (joueur.getNbTues() == 0) {
        dessinerTexte(">> FIN PACIFISTE <<", FONT_LG,
                      sf::Color::Green, MARGIN, y); y += 40.f;
        dessinerTexte("Vous avez epargne chaque ame.", FONT_MD,
                      sf::Color(100, 220, 100), MARGIN, y);
    } else if (joueur.getNbEpargnes() == 0) {
        dessinerTexte(">> FIN GENOCIDAIRE <<", FONT_LG,
                      sf::Color::Red, MARGIN, y); y += 40.f;
        dessinerTexte("ALTERDUNE tremble a votre nom.", FONT_MD,
                      sf::Color(200, 100, 100), MARGIN, y);
    } else {
        dessinerTexte(">> FIN NEUTRE <<", FONT_LG,
                      sf::Color::Yellow, MARGIN, y); y += 40.f;
        dessinerTexte("Tue et epargne. Votre legende reste ambigue.", FONT_MD,
                      sf::Color(220, 220, 100), MARGIN, y);
    }

    dessinerSeparateur((float)WIN_H - 50.f);
    dessinerTexte("[Entree] Fermer", FONT_SM,
                  sf::Color(100, 100, 100), MARGIN, (float)WIN_H - 38.f);
}

void JeuSFML::handleEventFin(const sf::Event& e) {
    if (auto* kp = e.getIf<sf::Event::KeyPressed>())
        if (kp->code == sf::Keyboard::Key::Enter)
            window.close();
}
