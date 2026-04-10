#include "ChargeurCSV.h"

#include <fstream>
#include <sstream>
using namespace std;

vector<Item> ChargeurCSV::chargerItems(const string& fichier) {
    vector<Item> items;
    ifstream file(fichier);
    string ligne;

    getline(file, ligne); // on saute l'en-tête

    while (getline(file, ligne)) {
        stringstream ss(ligne);
        string nom, type, valeurStr, quantiteStr;

        getline(ss, nom, ';');
        getline(ss, type, ';');
        getline(ss, valeurStr, ';');
        getline(ss, quantiteStr, ';');

        items.push_back(Item(nom, type, stoi(valeurStr), stoi(quantiteStr)));
    }

    file.close();
    return items;
}