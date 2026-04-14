#ifndef JEU_H
#define JEU_H

#include <vector>
#include <random>
#include <string>
#include "joueur.h"
#include "bestiaire.h"
#include "monstre.h"
using namespace std;

class Jeu {
private:
    Joueur joueur;
    vector<Monstre*> monstres;
    Bestiaire bestiaire;
    mt19937 rng;

    // Helpers internes
    int lireEntier();
    int calculerDegats(int hpMax);
    void attendreEntree() const;
    void effacerEcran() const;
    string barreHP(int hp, int hpMax, int largeur = 20) const;

    // Phases de jeu
    void menuItems();
    void menuItemsCombat(bool& tourConsomme, Monstre* monstre);
    void lancerCombat();
    void afficherEtatCombat(const Monstre* monstre) const;
    void appliquerDrop(const Monstre* monstre);
    void afficherFin() const;

public:
    Jeu();
    ~Jeu();
    void demarrer();
};

#endif
