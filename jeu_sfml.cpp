#include "jeu_sfml.h"
#include "chargeurCSV.h"
#include "actionAct.h"
#include "item.h"
#include <algorithm>
#include <sstream>

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
      frameCount(0),
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
            if (!monstres.empty()) {
                std::uniform_int_distribution<int> dist(0, (int)monstres.size() - 1);
                if (monstreCourant) { delete monstreCourant; }
                monstreCourant = monstres[dist(rng)]->cloner();
                tourConsomme = false;
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
    float cy = 200.f;
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
                etat = GameState::COMBAT_RESULT;
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

        prochainEtat = GameState::COMBAT_MAIN;
        etat = GameState::COMBAT_RESULT;
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
