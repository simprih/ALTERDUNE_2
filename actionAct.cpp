#include "actionAct.h"
#include <iostream>
using namespace std;

ActionACT::ActionACT(string id, string texte, int impactMercy) {
    this->id = id;
    this->texte = texte;
    this->impactMercy = impactMercy;
}

string ActionACT::getId() const { return id; }
string ActionACT::getTexte() const { return texte; }
int ActionACT::getImpactMercy() const { return impactMercy; }

void ActionACT::afficher() const {
    cout << "[" << id << "] " << texte << endl;
}

const map<string, ActionACT>& ActionACT::getCatalogue() {
    // static local : la map n'est construite qu'une seule fois au premier appel,
    // puis réutilisée — c'est le pattern "singleton paresseux"
    static const map<string, ActionACT> catalogue = {
        {"COMPLIMENT",  ActionACT("COMPLIMENT",  "Vous faites un compliment sincere. Le monstre ne sait plus ou se mettre.", 20)},
        {"JOKE",        ActionACT("JOKE",        "Vous racontez une blague. Le monstre ricane malgre lui.", 15)},
        {"DISCUSS",     ActionACT("DISCUSS",     "Vous engagez une discussion philosophique. Le monstre semble perplexe.", 10)},
        {"OBSERVE",     ActionACT("OBSERVE",     "Vous l'observez attentivement. Il se sent important.", 5)},
        {"PET",         ActionACT("PET",         "Vous tendez la main pour le caresser. Risque, mais ca marche.", 25)},
        {"OFFER_SNACK", ActionACT("OFFER_SNACK", "Vous sortez un en-cas. Qui peut resister a la nourriture ?", 20)},
        {"DANCE",       ActionACT("DANCE",       "Vous dansez. Le monstre ne sait absolument pas comment reagir.", 10)},
        {"REASON",      ActionACT("REASON",      "Vous essayez de raisonner le monstre. Etonnamment efficace.", 15)},
        {"INSULT",      ActionACT("INSULT",      "Vous l'insultez copieusement. C'etait une tres mauvaise idee.", -20)},
        {"TAUNT",       ActionACT("TAUNT",       "Vous le provoquez. Il ne semble vraiment pas apprecier.", -15)},
    };
    return catalogue;
}