#ifndef CHARGEURCSV_H
#define CHARGEURCSV_H

#include <vector>
#include "Item.h"

using namespace std;

class ChargeurCSV {
public:
    static vector<Item> chargerItems(const string& fichier);

};

#endif