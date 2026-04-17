# Makefile — ALTERDUNE (macOS ARM64)
# Usage:
#   make          -> compile les deux versions (console + SFML)
#   make console  -> version console uniquement
#   make sfml     -> version SFML uniquement
#   make clean    -> supprime les binaires compilés

CXX      = clang++
CXXFLAGS = -std=c++17 -arch arm64 -Wall

COMMON_SRC = joueur.cpp monstre.cpp monstreNormal.cpp miniboss.cpp boss.cpp \
             item.cpp actionAct.cpp bestiaire.cpp chargeurCSV.cpp

.PHONY: all console sfml clean

all: console sfml

console: main.cpp jeu.cpp $(COMMON_SRC)
	$(CXX) $(CXXFLAGS) -o alterdune \
		main.cpp jeu.cpp $(COMMON_SRC)

SFML_PREFIX = /opt/homebrew/opt/sfml

sfml: main_sfml.cpp jeu_sfml.cpp $(COMMON_SRC)
	$(CXX) $(CXXFLAGS) -I$(SFML_PREFIX)/include \
		-o alterdune_sfml \
		main_sfml.cpp jeu_sfml.cpp $(COMMON_SRC) \
		-L$(SFML_PREFIX)/lib \
		-lsfml-graphics -lsfml-window -lsfml-system

clean:
	rm -f alterdune alterdune_sfml
