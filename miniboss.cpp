#include "miniboss.h"
#include <iostream>
using namespace std;

MiniBoss::MiniBoss(string nom, int hp, int atk, int def, int mercyGoal, vector<string> actions)
    : Monstre(nom, "MINIBOSS", hp, atk, def, mercyGoal, actions) {
    nbActionsMax = 3;
}

Monstre* MiniBoss::cloner() const {
    return new MiniBoss(*this);
}

void MiniBoss::afficher() const {
    cout << "[MINIBOSS] " << nom << endl;
    cout << "HP : " << hp << "/" << hpMax << " | Mercy : " << mercy << "/" << mercyGoal << endl;
    cout << "Actions disponibles : " << actionsDisponibles[0] << ", " << actionsDisponibles[1] << ", " << actionsDisponibles[2] << endl;
}