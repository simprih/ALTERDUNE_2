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
    // mettre un autre atttibut nbactionmax qui sera initialisé dans les constructeurs des classes filles
    vector<string> actionsDisponibles; // mettre un vecteur d'action 

    public:
    // Constructeurs : 
    Monstre(string nom, string categorie, int hp, int atk,int def, int mercyGoal, vector<string> actions);

    //Getters : 
    string getNom() const;
    int getHp() const;
    int getHpMax() const;
    int getMercy() const;
    int getMercyGoal() const;
    string getCategorie() const;
    vector<string> getActions() const;

    // Méthodes communes : 
    bool estVivant() const;
    bool peutEtreEpargne() const;    // mercy >= mercyGoal
    void modifierMercy(int valeur);  // modifie + borne entre 0 et mercyGoal
    void subirDegats(int degats);
    void afficherStats() const;

    // Méthode virtuelle pour que la méthode du bon type de monstre soit appelée
    virtual void afficher() const = 0;

    // Crée une copie fraîche du monstre (HP et Mercy réinitialisés)
    virtual Monstre* cloner() const = 0;

    // Destructeur virtuel car sinon le destructeur du monstre peut ne pas être appelé correctement
    virtual ~Monstre() {}

};

#endif