#include "jeu.h"
#include "chargeurCSV.h"
#include "actionAct.h"
#include <iostream>
#include <limits>
#include <algorithm>
#ifdef _WIN32
#include <windows.h>
#endif
using namespace std;

// ─── Couleurs ANSI ────────────────────────────────────────────────────────────
// Definies localement pour ne pas polluer les autres fichiers

static const string C_RESET   = "\033[0m";
static const string C_ROUGE   = "\033[31m";
static const string C_VERT    = "\033[32m";
static const string C_JAUNE   = "\033[33m";
static const string C_CYAN    = "\033[36m";
static const string C_MAGENTA = "\033[35m";
static const string C_GRAS    = "\033[1m";

// Retourne la couleur correspondant au pourcentage de HP restants
static string couleurHP(int hp, int hpMax) {
    if (hpMax <= 0) return C_VERT;
    float ratio = (float)hp / (float)hpMax;
    if (ratio > 0.6f) return C_VERT;
    if (ratio > 0.3f) return C_JAUNE;
    return C_ROUGE;
}

// ─── Constructeur / Destructeur ───────────────────────────────────────────────

Jeu::Jeu() : rng(random_device{}()) {
#ifdef _WIN32
    // Active le support des codes ANSI sur Windows 10+
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    SetConsoleMode(hOut, dwMode | 0x0004); // ENABLE_VIRTUAL_TERMINAL_PROCESSING
#endif
}

Jeu::~Jeu() {
    for (Monstre* m : monstres) delete m;
}

// ─── Helpers ──────────────────────────────────────────────────────────────────

int Jeu::lireEntier() {
    int val;
    while (!(cin >> val)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "  Entree invalide, reessayez : ";
    }
    return val;
}

int Jeu::calculerDegats(int hpMax) {
    uniform_int_distribution<int> dist(0, hpMax);
    return dist(rng);
}

// Efface le terminal (cross-platform)
void Jeu::effacerEcran() const {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// Attend que l'utilisateur appuie sur Entree
// (consomme le '\n' laisse par le cin >> precedent, puis attend un nouveau)
void Jeu::attendreEntree() const {
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}

// Genere une barre de progression de type [######....]
string Jeu::barreHP(int hp, int hpMax, int largeur) const {
    if (hpMax <= 0) return "[" + string(largeur, '.') + "]";
    int rempli = (hp * largeur) / hpMax;
    string barre = "[";
    for (int i = 0; i < largeur; i++) {
        barre += (i < rempli) ? '#' : '.';
    }
    barre += "]";
    return barre;
}

// ─── Affichage etat de combat ─────────────────────────────────────────────────

void Jeu::afficherEtatCombat(const Monstre* monstre) const {
    effacerEcran();
    cout << C_GRAS << string(50, '-') << C_RESET << endl;

    // Joueur
    string coulJ = couleurHP(joueur.getHp(), joueur.getHpMax());
    cout << "  " << C_CYAN << C_GRAS << joueur.getNom() << C_RESET << endl;
    cout << "  HP    " << coulJ
         << barreHP(joueur.getHp(), joueur.getHpMax())
         << "  " << joueur.getHp() << "/" << joueur.getHpMax()
         << C_RESET << endl;

    cout << endl;

    // Monstre
    string coulM = couleurHP(monstre->getHp(), monstre->getHpMax());
    cout << "  " << C_MAGENTA << C_GRAS
         << monstre->getNom() << "  [" << monstre->getCategorie() << "]"
         << C_RESET << endl;
    cout << "  HP    " << coulM
         << barreHP(monstre->getHp(), monstre->getHpMax())
         << "  " << monstre->getHp() << "/" << monstre->getHpMax()
         << C_RESET << endl;
    cout << "  Mercy " << C_JAUNE
         << barreHP(monstre->getMercy(), monstre->getMercyGoal())
         << "  " << monstre->getMercy() << "/" << monstre->getMercyGoal()
         << C_RESET << endl;

    cout << C_GRAS << string(50, '-') << C_RESET << endl << endl;
}

// ─── Menu Items (hors combat) ─────────────────────────────────────────────────

void Jeu::menuItems() {
    effacerEcran();
    cout << C_GRAS << "=== INVENTAIRE ===" << C_RESET << endl << endl;
    joueur.afficherInventaire();
    cout << endl;
    cout << "  Utiliser un item (numero) ou " << C_JAUNE << "0" << C_RESET
         << " pour revenir : ";
    int idx = lireEntier();
    if (idx > 0) joueur.utiliserItem(idx - 1);
}

// ─── Menu Items (en combat) ───────────────────────────────────────────────────
// Si l'utilisateur entre 0, le tour n'est pas consomme.
// Les items MERCY_BOOST sont geres ici directement (necessitent le monstre courant).

void Jeu::menuItemsCombat(bool& tourConsomme, Monstre* monstre) {
    tourConsomme = false; // utiliser un item ne consomme jamais le tour du monstre
    cout << endl;
    joueur.afficherInventaire();
    cout << "  Numero de l'item (" << C_JAUNE << "0" << C_RESET << " = annuler) : ";
    int idx = lireEntier();
    if (idx <= 0) {
        cout << C_JAUNE << "  Annulation. Le tour n'est pas consomme." << C_RESET << endl;
        tourConsomme = false;
        return;
    }
    int realIdx = idx - 1;
    vector<Item>& inv = joueur.getInventaire();
    if (realIdx >= (int)inv.size()) {
        cout << "  Item invalide." << endl;
        return;
    }
    if (!inv[realIdx].estDisponible()) {
        cout << "  Plus de " << inv[realIdx].getNom() << " disponible !" << endl;
        return;
    }
    if (inv[realIdx].getType() == "MERCY_BOOST") {
        int gain = inv[realIdx].getValeur();
        monstre->modifierMercy(gain);
        inv[realIdx].reduireQuantite();
        cout << C_JAUNE << "  Vous utilisez " << inv[realIdx].getNom()
             << " : Mercy +" << gain << "  ->  "
             << monstre->getMercy() << "/" << monstre->getMercyGoal()
             << C_RESET << endl;
    } else {
        joueur.utiliserItem(realIdx);
    }
}

// ─── Drop apres combat ────────────────────────────────────────────────────────
// NORMAL  : 40% chance → Snack (HEAL)
// MINIBOSS: 60% chance → Cristal de Mercy (MERCY_BOOST)
// BOSS    : 80% chance → SuperPotion (HEAL)

void Jeu::appliquerDrop(const Monstre* monstre) {
    uniform_int_distribution<int> roll(1, 100);
    int r = roll(rng);

    const string& cat = monstre->getCategorie();
    string nom, type;
    int valeur;

    if (cat == "NORMAL") {
        if (r > 40) return;
        nom = "Snack"; type = "HEAL"; valeur = 8;
    } else if (cat == "MINIBOSS") {
        if (r > 60) return;
        nom = "Cristal de Mercy"; type = "MERCY_BOOST"; valeur = 25;
    } else { // BOSS
        if (r > 80) return;
        nom = "SuperPotion"; type = "HEAL"; valeur = 30;
    }

    Item drop(nom, type, valeur, 1);
    joueur.ajouterItem(drop);
    cout << C_VERT << "  " << monstre->getNom() << " a laisse tomber : "
         << nom << " !" << C_RESET << endl;
}

// ─── Combat ───────────────────────────────────────────────────────────────────

void Jeu::lancerCombat() {
    if (monstres.empty()) {
        cout << "  Aucun monstre disponible." << endl;
        return;
    }

    // Tirage aleatoire et clone pour ne pas modifier le modele CSV
    uniform_int_distribution<int> dist(0, (int)monstres.size() - 1);
    Monstre* monstre = monstres[dist(rng)]->cloner();
    const map<string, ActionACT>& catalogue = ActionACT::getCatalogue();

    effacerEcran();
    cout << C_ROUGE << C_GRAS << string(50, '=') << C_RESET << endl;
    cout << C_ROUGE << C_GRAS
         << "  COMBAT : " << joueur.getNom() << " vs " << monstre->getNom()
         << C_RESET << endl;
    cout << C_ROUGE << C_GRAS << string(50, '=') << C_RESET << endl;
    cout << endl << "  (appuyez sur Entree)";
    attendreEntree();

    bool combatTermine = false;

    while (!combatTermine) {
        afficherEtatCombat(monstre);

        bool tourConsomme = true;
        int choix = 0;
        while (choix < 1 || choix > 4) {
            cout << "  " << C_GRAS
                 << "[1] FIGHT   [2] ACT   [3] ITEM   [4] MERCY"
                 << C_RESET << endl;
            cout << "  Action : ";
            choix = lireEntier();
        }
        cout << endl;

        // ---- FIGHT ----
        if (choix == 1) {
            // La DEF du monstre absorbe une partie des degats
            int base = calculerDegats(monstre->getHpMax());
            int degats = base - monstre->getDef();
            if (degats < 0) degats = 0;

            if (degats == 0) {
                cout << C_JAUNE << "  Votre attaque est absorbee par la DEF de "
                     << monstre->getNom() << " !" << C_RESET << endl;
            } else {
                monstre->subirDegats(degats);
                cout << C_VERT << "  Vous infligez " << degats
                     << " degats a " << monstre->getNom()
                     << " (DEF -" << monstre->getDef() << ")"
                     << C_RESET << endl;
            }
            if (!monstre->estVivant()) {
                cout << C_ROUGE << C_GRAS << endl
                     << "  " << monstre->getNom() << " est vaincu !"
                     << C_RESET << endl;
                appliquerDrop(monstre);
                joueur.ajouterVictoire();
                joueur.ajouterTue();
                bestiaire.ajouterMonstre(monstre, "Tue");
                monstre = nullptr;
                combatTermine = true;
            }

        // ---- ACT ----
        } else if (choix == 2) {
            const vector<string>& actions = monstre->getActions();
            int nbActions = min(monstre->getNbActionsMax(), (int)actions.size());
            cout << C_CYAN << "  === Actions disponibles ===" << C_RESET << endl;
            for (int i = 0; i < nbActions; i++) {
                cout << "    " << i + 1 << ". " << actions[i] << endl;
            }
            int actChoix = 0;
            while (actChoix < 1 || actChoix > nbActions) {
                cout << "  Choisissez : ";
                actChoix = lireEntier();
            }
            const string& actionId = actions[actChoix - 1];
            auto it = catalogue.find(actionId);
            if (it != catalogue.end()) {
                cout << endl;
                it->second.afficher();
                int impact = it->second.getImpactMercy();
                monstre->modifierMercy(impact);
                if (impact > 0) {
                    cout << C_VERT << "  Mercy +" << impact << "  ->  "
                         << monstre->getMercy() << "/" << monstre->getMercyGoal()
                         << C_RESET << endl;
                    cout << C_VERT << "  " << monstre->getNom()
                         << " semble s'apaiser..." << C_RESET << endl;
                } else if (impact < 0) {
                    cout << C_ROUGE << "  Mercy " << impact << "  ->  "
                         << monstre->getMercy() << "/" << monstre->getMercyGoal()
                         << C_RESET << endl;
                    cout << C_ROUGE << "  " << monstre->getNom()
                         << " se met en colere !" << C_RESET << endl;
                } else {
                    cout << C_JAUNE << "  " << monstre->getNom()
                         << " reste indifferent." << C_RESET << endl;
                }
            } else {
                cout << "  Action inconnue dans le catalogue." << endl;
            }

        // ---- ITEM ----
        } else if (choix == 3) {
            menuItemsCombat(tourConsomme, monstre);

        // ---- MERCY ----
        } else {
            if (monstre->peutEtreEpargne()) {
                cout << C_VERT << C_GRAS << endl
                     << "  " << monstre->getNom() << " est epargne ! Vous gagnez !"
                     << C_RESET << endl;
                appliquerDrop(monstre);
                joueur.ajouterVictoire();
                joueur.ajouterEpargne();
                bestiaire.ajouterMonstre(monstre, "Epargne");
                monstre = nullptr;
                combatTermine = true;
            } else {
                cout << C_ROUGE << "  Mercy insuffisante. ("
                     << monstre->getMercy() << "/" << monstre->getMercyGoal()
                     << ")  Le tour n'est pas consomme."
                     << C_RESET << endl;
                tourConsomme = false;
            }
        }

        // ---- Tour du monstre ----
        if (!combatTermine && tourConsomme) {
            cout << endl;

            // Hesitation du monstre selon son niveau de Mercy
            bool hesitation = false;
            if (monstre->getMercyGoal() > 0) {
                float ratio = (float)monstre->getMercy() / (float)monstre->getMercyGoal();
                int seuil = 0;
                if (ratio >= 1.0f)       seuil = 75;
                else if (ratio >= 0.66f) seuil = 50;
                else if (ratio >= 0.33f) seuil = 25;
                if (seuil > 0) {
                    uniform_int_distribution<int> roll(1, 100);
                    hesitation = roll(rng) <= seuil;
                }
            }
            if (hesitation) {
                cout << C_JAUNE << "  " << monstre->getNom()
                     << " hesite et ne vous attaque pas ce tour !"
                     << C_RESET << endl;
            } else {
            // L'ATK du monstre s'ajoute comme bonus aux degats de base
            int degats = calculerDegats(joueur.getHpMax()) + monstre->getAtk() / 3;
            if (degats == 0) {
                cout << C_JAUNE << "  " << monstre->getNom()
                     << " rate son attaque !" << C_RESET << endl;
            } else {
                joueur.subirDegats(degats);
                cout << C_ROUGE << "  " << monstre->getNom()
                     << " vous inflige " << degats << " degats !"
                     << " (ATK +" << monstre->getAtk() / 3 << ")"
                     << C_RESET << endl;
            }
            }
            if (!joueur.estVivant()) {
                cout << C_ROUGE << C_GRAS << endl
                     << "  " << joueur.getNom() << " tombe au combat. Game Over..."
                     << C_RESET << endl;
                combatTermine = true;
            }
        }

        if (!combatTermine) {
            cout << endl << "  (Entree pour continuer)";
            attendreEntree();
        }
    }

    if (joueur.estVivant()) {
        cout << C_VERT << "\n  Victoires : " << joueur.getNbVictoires()
             << "/10" << C_RESET << endl;
    }

    cout << endl << "  (Entree pour continuer)";
    attendreEntree();

    if (monstre != nullptr) delete monstre;
}

// ─── Fin de partie ────────────────────────────────────────────────────────────

void Jeu::afficherFin() const {
    effacerEcran();
    cout << C_GRAS << string(42, '=') << C_RESET << endl;
    cout << C_GRAS << "           FIN DE PARTIE" << C_RESET << endl;
    cout << C_GRAS << string(42, '=') << C_RESET << endl << endl;
    joueur.afficherStats();
    cout << endl;

    if (!joueur.estVivant()) {
        cout << C_ROUGE << C_GRAS << "Vous avez ete vaincu." << C_RESET << endl;
        cout << "Le monde d'ALTERDUNE vous a eu..." << endl;
    } else if (joueur.getNbTues() == 0) {
        cout << C_VERT << C_GRAS << ">> FIN PACIFISTE <<" << C_RESET << endl;
        cout << "Vous avez epargne chaque ame. La paix regne sur ALTERDUNE." << endl;
    } else if (joueur.getNbEpargnes() == 0) {
        cout << C_ROUGE << C_GRAS << ">> FIN GENOCIDAIRE <<" << C_RESET << endl;
        cout << "Vous n'avez montre aucune pitie. ALTERDUNE tremble a votre nom." << endl;
    } else {
        cout << C_JAUNE << C_GRAS << ">> FIN NEUTRE <<" << C_RESET << endl;
        cout << "Tue et epargne. Votre legende reste ambigue." << endl;
    }

    cout << C_GRAS << string(42, '=') << C_RESET << endl;
}

// ─── Demarrer ─────────────────────────────────────────────────────────────────

void Jeu::demarrer() {
    // Ecran titre
    effacerEcran();
    cout << C_CYAN << C_GRAS << endl;
    cout << "  +----------------------------------+" << endl;
    cout << "  |   BIENVENUE DANS ALTERDUNE       |" << endl;
    cout << "  |   Un RPG par console en C++      |" << endl;
    cout << "  +----------------------------------+" << endl;
    cout << C_RESET << endl;

    // Chargement des donnees
    monstres = ChargeurCSV::chargerMonstres("monsters.csv");
    vector<Item> items = ChargeurCSV::chargerItems("items.csv");

    // Saisie du nom
    cout << endl << "  Entrez le nom de votre personnage : ";
    string nom;
    cin >> nom;

    joueur = Joueur(nom, 100);
    for (Item& item : items) joueur.ajouterItem(item);

    // Recap de depart
    effacerEcran();
    cout << C_GRAS << "=== Recapitulatif de depart ===" << C_RESET << endl << endl;
    joueur.afficherStats();
    joueur.afficherInventaire();
    cout << endl << "  (appuyez sur Entree pour commencer)";
    attendreEntree();

    // Boucle principale
    bool quitter = false;
    while (joueur.getNbVictoires() < 10 && joueur.estVivant() && !quitter) {
        effacerEcran();

        // En-tete du menu avec etat du joueur
        cout << C_CYAN << C_GRAS << "=== MENU PRINCIPAL ===" << C_RESET << endl;
        cout << "  " << C_GRAS << joueur.getNom() << C_RESET
             << "   HP : " << couleurHP(joueur.getHp(), joueur.getHpMax())
             << joueur.getHp() << "/" << joueur.getHpMax() << C_RESET
             << "   Victoires : " << C_VERT << joueur.getNbVictoires()
             << "/10" << C_RESET << endl;
        cout << string(42, '-') << endl << endl;

        cout << "  1. Bestiaire" << endl;
        cout << "  2. Demarrer un combat" << endl;
        cout << "  3. Statistiques" << endl;
        cout << "  4. Items" << endl;
        cout << "  5. Quitter" << endl;
        cout << endl << "  Choix : ";

        int choix = lireEntier();

        switch (choix) {
            case 1:
                effacerEcran();
                bestiaire.afficher();
                cout << endl << "  (Entree pour revenir)";
                attendreEntree();
                break;
            case 2:
                lancerCombat();
                break;
            case 3:
                effacerEcran();
                joueur.afficherStats();
                cout << endl << "  (Entree pour revenir)";
                attendreEntree();
                break;
            case 4:
                menuItems();
                break;
            case 5:
                quitter = true;
                break;
            default:
                cout << C_ROUGE << "  Choix invalide." << C_RESET << endl;
                break;
        }
    }

    if (!quitter) {
        afficherFin();
    } else {
        effacerEcran();
        cout << C_CYAN << "\n  Au revoir !" << C_RESET << endl << endl;
    }
}
