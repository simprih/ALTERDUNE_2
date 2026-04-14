#include "monstre.h"
#include <iostream>
using namespace std;

Monstre::Monstre(string nom, string categorie, int hp, int atk, int def, int mercyGoal, vector<string> actions) {
    this->nom = nom;
    this->categorie = categorie;
    this->hp = hp;
    this->hpMax = hp;
    this->atk = atk;
    this->def = def;
    this->mercy = 0;
    this->mercyGoal = mercyGoal;
    this->nbActionsMax = 0;
    this->actionsDisponibles = actions;
}

string Monstre::getNom() const { return nom; }
string Monstre::getCategorie() const { return categorie; }
int Monstre::getHp() const { return hp; }
int Monstre::getHpMax() const { return hpMax; }
int Monstre::getAtk() const { return atk; }
int Monstre::getDef() const { return def; }
int Monstre::getMercy() const { return mercy; }
int Monstre::getMercyGoal() const { return mercyGoal; }
vector<string> Monstre::getActions() const { return actionsDisponibles; }
int Monstre::getNbActionsMax() const { return nbActionsMax; }

bool Monstre::estVivant() const {
    return hp > 0;
}

bool Monstre::peutEtreEpargne() const {
    return mercy >= mercyGoal;
}

void Monstre::modifierMercy(int valeur) {
    mercy += valeur;
    if (mercy < 0) mercy = 0;
    if (mercy > mercyGoal) mercy = mercyGoal;
}

void Monstre::subirDegats(int degats) {
    hp -= degats;
    if (hp < 0) hp = 0;
}

void Monstre::afficherStats() const {
    cout << "[ " << categorie << " ] " << nom << endl;
    cout << "HP : " << hp << "/" << hpMax << endl;
    cout << "ATK : " << atk << " | DEF : " << def << endl;
    cout << "Mercy : " << mercy << "/" << mercyGoal << endl;
}

void Monstre::afficherResume() const {
    cout << "[ " << categorie << " ] " << nom << endl;
    cout << "HP max : " << hpMax << " | ATK : " << atk << " | DEF : " << def << endl;
    cout << "Actions : ";
    for (int i = 0; i < (int)actionsDisponibles.size(); i++) {
        if (i > 0) cout << ", ";
        cout << actionsDisponibles[i];
    }
    cout << endl;
}