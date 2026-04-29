#include "monstreNormal.h"
#include <iostream>
using namespace std;

MonstreNormal::MonstreNormal(string nom, int hp, int atk, int def, int mercyGoal, vector<string> actions)
    : Monstre(nom, "NORMAL", hp, atk, def, mercyGoal, actions) {
    nbActionsMax = 2;
    if (actionsDisponibles.size() > 2) actionsDisponibles.resize(2); // max 2 actions pour un NORMAL
}

Monstre* MonstreNormal::cloner() const {
    return new MonstreNormal(*this);
}

void MonstreNormal::afficher() const {
    cout << "[NORMAL] " << nom << endl;
    cout << "HP : " << hp << "/" << hpMax << " | Mercy : " << mercy << "/" << mercyGoal << endl;
    cout << "Actions disponibles : " << actionsDisponibles[0] << ", " << actionsDisponibles[1] << endl;
}