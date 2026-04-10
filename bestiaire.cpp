#include "Bestiare.h"
#include <iostream>
using namespace std;
#include <windows.h>

void Bestiaire::ajouterMonstre(Monstre* monstre, string resultat)
{
    monstresVaincus.push_back(make_pair(monstre, resultat));
}

void Bestiare::afficher() const 
{
    if(estVide())
    {
        cout << "Aucun monstre vaincu pour l'instant." << endl;
    }
    else
    {
        cout << "=== Bestiaire ===" << endl;
        for (int i = 0; i < monstresVaincus.size(); i++)
        {
            Monstre* m = monstresVaincus[i].first;
            string resultat = monstresVaincus[i].second;
            cout << "--- Monstre " << i+1 << " ---" << endl;
            m -> afficherStats();
            cout << "Resultat : " << resultat << endl;
        }
    }
}

bool Bestiaire::estVide() const 
{
    return monstresVaincus.empty();
}