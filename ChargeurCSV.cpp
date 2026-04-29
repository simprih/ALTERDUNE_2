#include "chargeurCSV.h"
#include "monstreNormal.h"
#include "miniboss.h"
#include "boss.h"

#include <fstream>
#include <sstream>
#include <iostream>
using namespace std;

vector<Item> ChargeurCSV::chargerItems(const string& fichier) {
    vector<Item> items;
    ifstream file(fichier);

    // Erreur fichier introuvable
    if (!file.is_open()) {
        cout << "Erreur : impossible d'ouvrir " << fichier << endl;
        exit(1);
    }

    string ligne;
    getline(file, ligne); // on saute la première ligne (en-tête CSV)

    while (getline(file, ligne)) {
        // On enveloppe la ligne dans un flux pour pouvoir la découper champ par champ
        stringstream ss(ligne);
        string nom, type, valeurStr, quantiteStr;

        // getline avec délimiteur ';' extrait un champ à la fois — si un champ manque, ligne ignorée
        if (!getline(ss, nom, ';') ||
            !getline(ss, type, ';') ||
            !getline(ss, valeurStr, ';') ||
            !getline(ss, quantiteStr, ';')) {
            cout << "Ligne mal formée ignorée : " << ligne << endl;
            continue;
        }

        // stoi convertit la string en int — plantera si le CSV contient une valeur non numérique
        int valeur = stoi(valeurStr);
        int quantite = stoi(quantiteStr);
        items.push_back(Item(nom, type, valeur, quantite));
    }

    file.close();
    cout << items.size() << " items chargés depuis " << fichier << endl;
    return items;
}

vector<Monstre*> ChargeurCSV::chargerMonstres(const string& fichier) {
    vector<Monstre*> monstres;
    ifstream file(fichier);

    // Erreur fichier introuvable
    if (!file.is_open()) {
        cout << "Erreur : impossible d'ouvrir " << fichier << endl;
        exit(1);
    }

    string ligne;
    getline(file, ligne); // on saute la première ligne (en-tête)

    while (getline(file, ligne)) {
        stringstream ss(ligne);
        string categorie, nom, hpStr, atkStr, defStr, mercyGoalStr;
        string act1, act2, act3, act4;

        // On découpe la ligne
        if (!getline(ss, categorie, ';') ||
            !getline(ss, nom, ';') ||
            !getline(ss, hpStr, ';') ||
            !getline(ss, atkStr, ';') ||
            !getline(ss, defStr, ';') ||
            !getline(ss, mercyGoalStr, ';') ||
            !getline(ss, act1, ';') ||
            !getline(ss, act2, ';') ||
            !getline(ss, act3, ';') ||
            !getline(ss, act4, ';')) {
            cout << "Ligne mal formée ignorée : " << ligne << endl;
            continue;
        }

        int hp = stoi(hpStr);
        int atk = stoi(atkStr);
        int def = stoi(defStr);
        int mercyGoal = stoi(mercyGoalStr);

        vector<string> actions;

        // On instancie la bonne sous-classe selon la catégorie lue dans le CSV
        // new alloue sur le tas — on stocke un pointeur de base Monstre* pour le polymorphisme
        if (categorie == "NORMAL") {
            actions = {act1, act2};
            monstres.push_back(new MonstreNormal(nom, hp, atk, def, mercyGoal, actions));
        } else if (categorie == "MINIBOSS") {
            actions = {act1, act2, act3};
            monstres.push_back(new MiniBoss(nom, hp, atk, def, mercyGoal, actions));
        } else if (categorie == "BOSS") {
            actions = {act1, act2, act3, act4};
            monstres.push_back(new Boss(nom, hp, atk, def, mercyGoal, actions));
        } else {
            cout << "Catégorie inconnue ignorée : " << categorie << endl;
        }
    }

    file.close();
    cout << monstres.size() << " monstres chargés depuis " << fichier << endl;
    return monstres;
}