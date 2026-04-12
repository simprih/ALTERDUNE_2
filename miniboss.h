#ifndef MINIBOSS_H
#define MINIBOSS_H

#include "monstre.h"

class MiniBoss : public Monstre {
public:
    MiniBoss(string nom, int hp, int atk, int def, int mercyGoal, vector<string> actions);

    void afficher() const override;
    Monstre* cloner() const override;
};

#endif