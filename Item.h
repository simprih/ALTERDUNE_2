#ifndef ITEM_H
#define ITEM_H

#include <string>
using namespace std;
class Item
{
    private:
    string nom;
    string type; // que HEAL pour l'instant
    int valeur; // HP rendus 
    int quantite;

    public:
    Item(string nom, string type, int valeur, int quantite);

    //Getters :
    string getNom()const;
    string getType()const;
    int getValeur()const;
    int getQuantite()const;

    // Méthodes :
    void reduireQuantite();          // enleve 1 à quantite
    void augmenterQuantite(int n);   // ajoute n à quantite
    bool estDisponible() const;      // retourne true si quantite > 0
    void afficher() const;
};

#endif