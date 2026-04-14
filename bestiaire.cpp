#include "bestiaire.h"
#include <iostream>
using namespace std;

Bestiaire::~Bestiaire() {
    for (auto& p : monstresVaincus) {
        delete p.first;
    }
}

void Bestiaire::ajouterMonstre(Monstre* monstre, string resultat)
{
    monstresVaincus.push_back(make_pair(monstre, resultat));
}

void Bestiaire::afficher() const 
{
    if(estVide())
    {
        cout << "Aucun monstre vaincu pour l'instant." << endl;
    }
    else
    {
        cout << "=== Bestiaire ===" << endl;
        for (int i = 0; i < (int)monstresVaincus.size(); i++)
        {
            Monstre* m = monstresVaincus[i].first;
            string resultat = monstresVaincus[i].second;
            cout << "--- Monstre " << i+1 << " ---" << endl;
            m->afficherResume();
            cout << "Resultat : " << resultat << endl;
        }
    }
}

bool Bestiaire::estVide() const 
{
    return monstresVaincus.empty();
}