#include "jeu.h"
#ifdef _WIN32
#include <windows.h>
#endif

int main() {
#ifdef _WIN32
    // UTF-8 pour l'affichage des caracteres speciaux sur Windows
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
#endif

    Jeu jeu;
    jeu.demarrer();

    return 0;
}
