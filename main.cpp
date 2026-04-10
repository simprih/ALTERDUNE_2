#include <iostream>
#include "ChargeurCSV.h"
#include "Joueur.h"

using namespace std;

int main() {
    
   
    cout << "=== Bienvenue dans ALTERDUNE ===" << endl;

    // Chargement des fichiers
    vector<Item> items = ChargeurCSV::chargerItems("items.csv");
<<<<<<< HEAD
    vector<Monstre*> monstres = ChargeurCSV::chargerMonstres("monsters.csv");
=======
    //vector<Monstre*> monstres = ChargeurCSV::chargerMonstres("monsters.csv");
>>>>>>> 338dbe3 (5_fichier main et correction, le code compile)

    // Création du joueur
    string nom;
    cout << "Entrez le nom de votre personnage : ";
    cin >> nom;

    Joueur joueur(nom, 100);

    // On donne les items au joueur
    for (Item& item : items) {
        joueur.ajouterItem(item);
    }

    // Affichage du récap
    cout << "\n=== Recap de demarrage ===" << endl;
    joueur.afficherStats();
    joueur.afficherInventaire();

<<<<<<< HEAD
    cout << "\n=== Monstres disponibles ===" << endl;
    for (Monstre* m : monstres) {
        m->afficher();
        cout << endl;
    }

    // Libération mémoire
    for (Monstre* m : monstres) {
        delete m;
    }
=======
    // cout << "\n=== Monstres disponibles ===" << endl;
    // for (Monstre* m : monstres) {
    //     m->afficher();
    //     cout << endl;
    // }

    // // Libération mémoire
    // for (Monstre* m : monstres) {
    //     delete m;
    // }
>>>>>>> 338dbe3 (5_fichier main et correction, le code compile)

    return 0;
}