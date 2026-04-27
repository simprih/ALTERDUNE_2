#ifndef JEU_SFML_H
#define JEU_SFML_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <random>
#include "joueur.h"
#include "bestiaire.h"
#include "monstre.h"

enum class GameState {
    TITLE,
    NAME_INPUT,
    RECAP,
    MAIN_MENU,
    BESTIARY,
    STATS,
    INVENTORY,
    COMBAT_START,
    COMBAT_MAIN,
    COMBAT_ACT,
    COMBAT_ITEM,
    COMBAT_RESULT,
    GAME_OVER,
    END_SCREEN
};

class JeuSFML {
private:
    // ── SFML ────────────────────────────────────────────────────────────────────
    sf::RenderWindow window;
    sf::Font         font;

    // ── Logique de jeu (identique à Jeu) ────────────────────────────────────────
    Joueur                joueur;
    std::vector<Monstre*> monstres;
    Bestiaire             bestiaire;
    std::mt19937          rng;

    // ── Machine à états ─────────────────────────────────────────────────────────
    GameState etat;
    Monstre*  monstreCourant;
    bool      tourConsomme;

    // ── Saisie texte (nom joueur) ────────────────────────────────────────────────
    std::string inputBuffer;

    // ── Affichage défilant (bestiaire, ACT) ──────────────────────────────────────
    std::vector<std::string> lignesAffichage;
    int scrollOffset;

    // ── Log des événements du tour courant ───────────────────────────────────────
    std::string messageLog;

    // ── Sous-menu ACT ────────────────────────────────────────────────────────────
    std::vector<std::string> actionsDisponibles;
    int nbActionsMax;

    // ── Bestiaire SFML (contournement du getter privé de Bestiaire) ──────────────
    // Chaque entrée : ligne de texte décrivant un monstre vaincu
    std::vector<std::string> bestiaireLog;

    // ── Pool de monstres disponibles (indices dans monstres[]) ───────────────────
    std::vector<int> monstresDisponibles;
    int monstreCourantIdx;

    // ── Compteur de frames (clignotement) ────────────────────────────────────────
    int frameCount;

    // ── Animation de coup ────────────────────────────────────────────────────────
    int hitAnimStart;   // frameCount au moment du coup (-1 = inactif)
    int hitDamage;      // dégâts du coup (chiffre flottant)

    // ── État prochain après COMBAT_RESULT ────────────────────────────────────────
    GameState prochainEtat;

    // ── Helpers ──────────────────────────────────────────────────────────────────
    bool chargerPolice();
    void chargerDonnees();
    void render();
    void dispatchEvent(const sf::Event& e);

    int       keyToInt(const sf::Event::KeyPressed& kp);
    int       calculerDegats(int hpMax);
    void      appliquerDrop(const Monstre* monstre);
    sf::Color couleurHP(int hp, int hpMax);

    void dessinerTexte(const std::string& s, unsigned int taille,
                       sf::Color couleur, float x, float y);
    void dessinerLignes(const std::vector<std::string>& lignes,
                        float x, float y, unsigned int taille,
                        sf::Color couleur, int offset, int maxVisible);
    void dessinerBarre(float x, float y, float w, float h,
                       int val, int max, sf::Color fill,
                       sf::Color empty = sf::Color(50, 50, 50));
    void dessinerSeparateur(float y, sf::Color couleur = sf::Color::White);
    void dessinerMonstre(const Monstre* m, float cx, float cy);

    // ── Renderers ────────────────────────────────────────────────────────────────
    void renderTitre();
    void renderNomInput();
    void renderRecap();
    void renderMenuPrincipal();
    void renderBestiaire();
    void renderStats();
    void renderInventaire();
    void renderCombatStart();
    void renderCombatPrincipal();
    void renderCombatACT();
    void renderCombatItem();
    void renderCombatResultat();
    void renderFin();

    // ── Handlers d'événements ────────────────────────────────────────────────────
    void handleEventTitre(const sf::Event& e);
    void handleEventNomInput(const sf::Event& e);
    void handleEventRecap(const sf::Event& e);
    void handleEventMenuPrincipal(const sf::Event& e);
    void handleEventBestiaire(const sf::Event& e);
    void handleEventStats(const sf::Event& e);
    void handleEventInventaire(const sf::Event& e);
    void handleEventCombatStart(const sf::Event& e);
    void handleEventCombatPrincipal(const sf::Event& e);
    void handleEventCombatACT(const sf::Event& e);
    void handleEventCombatItem(const sf::Event& e);
    void handleEventCombatResultat(const sf::Event& e);
    void handleEventFin(const sf::Event& e);

public:
    JeuSFML();
    ~JeuSFML();
    void demarrer();
};

#endif
