#ifndef ACTIONACT_H
#define ACTIONACT_H

#include <string>
#include <map>
using namespace std;

class ActionACT {
private:
    string id;           // ex: "JOKE", "COMPLIMENT"
    string texte;        // texte drôle affiché
    int impactMercy;     // positif, négatif ou 0

public:
    // Constructeur
    ActionACT(string id, string texte, int impactMercy);

    // Getters
    string getId() const;
    string getTexte() const;
    int getImpactMercy() const;

    // Méthodes
    void afficher() const;

    // Catalogue prédéfini de toutes les actions ACT du jeu
    static const map<string, ActionACT>& getCatalogue();
};

#endif