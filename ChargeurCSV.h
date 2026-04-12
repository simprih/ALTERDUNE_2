#ifndef CHARGEURCSV_H
#define CHARGEURCSV_H

#include <vector>
#include "item.h"
#include "monstre.h"
using namespace std;

class ChargeurCSV {
public:
    static vector<Item> chargerItems(const string& fichier);
    static vector<Monstre*> chargerMonstres(const string& fichier);
};

#endif