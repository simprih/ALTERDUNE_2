#include "boss.h"
#include <iostream>
using namespace std;

Boss::Boss(string nom, int hp, int atk, int def, int mercyGoal, vector<string> actions)
    : Monstre(nom, "BOSS", hp, atk, def, mercyGoal, actions) {
    nbActionsMax = 4;
    if (actionsDisponibles.size() > 4) actionsDisponibles.resize(4); // max 4 actions pour un BOSS
}

Monstre* Boss::cloner() const {
    return new Boss(*this);
}

void Boss::afficher() const {
    cout << "[BOSS] " << nom << endl;
    cout << "HP : " << hp << "/" << hpMax << " | Mercy : " << mercy << "/" << mercyGoal << endl;
    cout << "Actions disponibles : " << actionsDisponibles[0] << ", " << actionsDisponibles[1] << ", " << actionsDisponibles[2] << ", " << actionsDisponibles[3] << endl;
}