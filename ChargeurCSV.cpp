#include "chargeurCSV.h"

#include <fstream>
#include <sstream>
using namespace std;

vector<Item> ChargeurCSV::chargerItems(const string& fichier) {
    vector<Item> items;
    ifstream file(fichier);
    string ligne;

    getline(file, ligne); // on saute l'en-tête

    while (getline(file, ligne)) // tant qu'on lit une ligne
    { 
        stringstream ss(ligne); // tranformer la ligne en flux pour la lire morceaux par morceaux
        string nom, type, valeurStr, quantiteStr;

        getline(ss, nom, ';'); // attribut ce qui est lu de ss à ; à nom 
        getline(ss, type, ';'); // attribut ce qui est lu jusqu'au prochain ;
        getline(ss, valeurStr, ';'); //...
        getline(ss, quantiteStr, ';'); //...

        items.push_back(Item(nom, type, stoi(valeurStr), stoi(quantiteStr))); // ajoute l'objet dans le vecteur items ; stoi = string to int 
    }

    file.close();

    cout << items.size() << " items charges depuis " << fichier << endl;

    return items;
}