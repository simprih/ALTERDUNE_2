#ifndef MONSTRE_H
#define MONSTRE_H

#include <string>
#include <vector>
using namespace std;

class Monstre
{
    protected:
    string nom;
    int hp;
    int hpMax;
    int atk;
    int def;
    int mercy;
    int mercyGoal;
    string categorie;
    int nbActionsMax;                  // initialise dans chaque classe fille (2, 3, ou 4)
    vector<string> actionsDisponibles;

    public:
    // Constructeurs : 
    Monstre(string nom, string categorie, int hp, int atk,int def, int mercyGoal, vector<string> actions);

    //Getters :
    string getNom() const;
    int getHp() const;
    int getHpMax() const;
    int getAtk() const;
    int getDef() const;
    int getMercy() const;
    int getMercyGoal() const;
    string getCategorie() const;
    vector<string> getActions() const;
    int getNbActionsMax() const;

    // Méthodes communes :
    bool estVivant() const;
    bool peutEtreEpargne() const;    // mercy >= mercyGoal
    void modifierMercy(int valeur);  // modifie + borne entre 0 et mercyGoal
    void subirDegats(int degats);
    void afficherStats() const;
    void afficherResume() const;     // stats originales pour le bestiaire

    // = 0 rend la méthode purement virtuelle : Monstre ne peut pas être instancié directement
    virtual void afficher() const = 0;

    // cloner() retourne une copie via new — chaque combat repart sur un monstre à HP/Mercy pleins
    // sans ça, un monstre déjà blessé conserverait ses dégâts d'un combat à l'autre
    virtual Monstre* cloner() const = 0;

    // Sans ce destructeur virtuel, supprimer un Monstre* qui pointe vers un MonstreNormal
    // n'appellerait que ~Monstre et fuiterait la mémoire de la sous-classe
    virtual ~Monstre() {}

};

#endif