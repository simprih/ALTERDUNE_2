#include "joueur.h"
#include <iostream>
#include <vector>
using namespace std;

Joueur::Joueur()
{
    nom = "Inconnu";
    hpMax = 100;
    hp = hpMax;
    nbVictoires = 0;
    nbTues = 0;
    nbEpargnes = 0;
}

Joueur::Joueur(string n, int hmax)
{
    nom = n;
    hpMax = hmax;
    hp = hpMax;
    nbVictoires = 0;
    nbTues = 0;
    nbEpargnes = 0;
}

// Getters : 
string Joueur::getNom() const {return nom;}
int Joueur::getHp() const { return hp;}
int Joueur::getHpMax() const{return hpMax;}
int Joueur::getNbVictoires() const{return nbVictoires;}
int Joueur::getNbTues() const {return nbTues;}
int Joueur::getNbEpargnes() const{return nbEpargnes;}
vector<Item>& Joueur::getInventaire(){return inventaire;}

// Méthodes :
bool Joueur::estVivant() const
{
    return hp>0;
}

void Joueur::subirDegats(int degats)
{
    hp -= degats;
    if(hp < 0){hp = 0;}
}

void Joueur::soigner(int valeur)
{
    hp += valeur;
    if(hp>hpMax){hp = hpMax;}
}

void Joueur::ajouterItem(Item& item) 
{
    inventaire.push_back(item);
}

void Joueur::utiliserItem(int index)
{
    if(index < 0 || index >= (int)inventaire.size())
    {
        cout << "Item invalide." << endl;
    }
    else if(!inventaire[index].estDisponible())
    {
        cout << "Plus de " << inventaire[index].getNom() << " disponible !" << endl;
    }
    else 
    {
        if (inventaire[index].getType() == "HEAL") 
        {
            soigner(inventaire[index].getValeur());
            cout << "Vous utilisez " << inventaire[index].getNom() << " et recuperez " << inventaire[index].getValeur() << " HP !" << endl;
        }
        inventaire[index].reduireQuantite();
    }
}

void Joueur::ajouterVictoire()
{
    nbVictoires++;
}
void Joueur::ajouterTue()
{
    nbTues++;
}
void Joueur::ajouterEpargne()
{
    nbEpargnes++;
}

void Joueur::afficherStats() const
{
    cout << "=== Stats de " << nom << " ===" << endl;
    cout << "HP : " << hp << "/" << hpMax << endl;
    cout << "Victoires : " << nbVictoires << "/10" << endl;
    cout << "Monstres tues : " << nbTues << endl;
    cout << "Monstres epargnes : " << nbEpargnes << endl;
}

void Joueur::afficherInventaire() const
{
    cout << "=== Inventaire ===" << endl;
    if(inventaire.empty())
    {
        cout << "Aucun item disponible" << endl;
    }
    else
    {
        for (int i = 0; i < (int)inventaire.size(); i++)
        {
            cout << i+1 << ". ";
            inventaire[i].afficher();
        }
    }
}


