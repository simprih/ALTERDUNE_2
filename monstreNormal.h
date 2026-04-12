#ifndef MONSTRENORMAL_H
#define MONSTRENORMAL_H

#include "monstre.h"

class MonstreNormal : public Monstre {
public:
    MonstreNormal(string nom, int hp, int atk, int def, int mercyGoal, vector<string> actions);

    void afficher() const override;
    Monstre* cloner() const override;
};

#endif