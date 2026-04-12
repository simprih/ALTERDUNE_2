#ifndef JOUEUR_H
#define JOUEUR_H

#include <string>
#include <vector>
#include "item.h"
using namespace std;

class Joueur
{
    private:
    string nom;
    int hp;
    int hpMax;
    int nbVictoires;
    int nbTues;
    int nbEpargnes;
    vector<Item> inventaire;

    public:
    // Constructeurs
    Joueur(); // constructeur par défaut
    Joueur(string nom, int hpMax);

    // Getters
    string getNom() const;
    int getHp() const;
    int getHpMax() const;
    int getNbVictoires() const;
    int getNbTues() const;
    int getNbEpargnes() const;
    vector<Item>& getInventaire();

    // Méthodes
    bool estVivant()const;
    void subirDegats(int degats);
    void soigner(int valeur);
    void ajouterItem(Item& item);
    void utiliserItem(int index);
    void ajouterVictoire();
    void ajouterTue();
    void ajouterEpargne();
    void afficherStats() const;
    void afficherInventaire() const;

};

#endif
