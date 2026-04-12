#include "boss.h"
#include <iostream>
using namespace std;

Boss::Boss(string nom, int hp, int atk, int def, int mercyGoal, vector<string> actions)
    : Monstre(nom, "BOSS", hp, atk, def, mercyGoal, actions) {}

Monstre* Boss::cloner() const {
    return new Boss(*this);
}

void Boss::afficher() const {
    cout << "[BOSS] " << nom << endl;
    cout << "HP : " << hp << "/" << hpMax << " | Mercy : " << mercy << "/" << mercyGoal << endl;
    cout << "Actions disponibles : " << actionsDisponibles[0] << ", " << actionsDisponibles[1] << ", " << actionsDisponibles[2] << ", " << actionsDisponibles[3] << endl;
}