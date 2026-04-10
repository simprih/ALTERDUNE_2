#ifndef BESTIARE_H
#define BESTIARE_H

#include <vector>
#include <string>
#include "monstre.h"
using namespace std;

class Bestiaire 
{
    private:
    vector<pair<Monstre*, string>> monstresVaincus; // vecteur sous la forme : Monstre + "Tué" ou "Épargné"

    public:
    //Méthodes
    void ajouterMonstre(Monstre* monstre, string resultat);
    void afficher() const;
    bool estVide() const;

};

#endif
