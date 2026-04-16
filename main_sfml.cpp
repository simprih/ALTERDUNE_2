// main_sfml.cpp
// ALTERDUNE — point d'entrée version SFML 3.0
//
// IMPORTANT : lancer depuis la racine du projet pour que les CSV soient trouvés.
//
// Compilation (macOS ARM64) :
//   clang++ -std=c++17 -arch arm64 -DSFML_STATIC -I./include \
//     main_sfml.cpp jeu_sfml.cpp \
//     joueur.cpp monstre.cpp monstreNormal.cpp miniboss.cpp boss.cpp \
//     item.cpp actionAct.cpp bestiaire.cpp chargeurCSV.cpp \
//     -L./lib \
//     -lsfml-graphics-s -lsfml-window-s -lsfml-system-s -lfreetype \
//     -framework OpenGL -framework AppKit -framework Foundation \
//     -framework IOKit -framework CoreFoundation -framework CoreGraphics \
//     -framework CoreVideo -framework Carbon \
//     -o alterdune_sfml

#include "jeu_sfml.h"

int main() {
    JeuSFML jeu;
    jeu.demarrer();
    return 0;
}
