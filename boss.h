#ifndef BOSS_H
#define BOSS_H

#include "monstre.h"

class Boss : public Monstre {
public:
    Boss(string nom, int hp, int atk, int def, int mercyGoal, vector<string> actions);

    void afficher() const override;
    Monstre* cloner() const override;
};

#endif