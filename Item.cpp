#include "Item.h"
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
bool Item::estDisponible() const
{
    return quantite > 0;
}

void Item::afficher() const
{

    cout << nom << " (" << type << ") - Soigne " << valeur << " HP - Quantite : " << quantite << endl;
}
