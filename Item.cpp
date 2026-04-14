#include "item.h"
#include <iostream>
using namespace std;

Item::Item(string nom, string type, int valeur, int quantite)
{
    this->nom = nom;
    this->type = type;
    this->valeur = valeur;
    this->quantite = quantite;
}

//Getters :
string Item::getNom()const{return nom;}
string Item::getType()const{return type;}
int Item::getValeur()const{return valeur;}
int Item::getQuantite()const{return quantite;}

// Méthodes :
void Item::reduireQuantite()
{
    if(quantite > 0)
    {
        quantite --;
    }
}

void Item::augmenterQuantite(int n)
{
    quantite += n;
} 
bool Item::estDisponible() const
{
    return quantite > 0;
}

void Item::afficher() const
{
    if (type == "HEAL") {
        cout << nom << " (HEAL) - Soigne " << valeur << " HP - Quantite : " << quantite << endl;
    } else if (type == "MERCY_BOOST") {
        cout << nom << " (MERCY_BOOST) - Mercy +" << valeur << " - Quantite : " << quantite << endl;
    } else {
        cout << nom << " (" << type << ") - Valeur : " << valeur << " - Quantite : " << quantite << endl;
    }
}
