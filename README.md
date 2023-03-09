# Donkey Kong Jr

![image](https://user-images.githubusercontent.com/113984975/224064857-1f13434d-f341-4154-8396-70dd33edf6de.png)

## Aperçu général du jeu

Donkey Kong Jr (DK Jr) doit libérer Donkey Kong (DK) qui a été capturé par Mario. Pour libérer
DK, DK Jr doit atteindre, puis attraper la clé à 4 reprises pour ouvrir chaque partie de la cage. Sur son chemin, DK Jr doit éviter des ennemis (crocos et corbeaux). Chaque fois que DK Jr ouvre une partie de la cage, on gagne 10 poids. Quand les 4 parties de la cage sont ouvertes et que DK est libéré, on gagne 10 points additionnels. Ensuite, la cage se referme. Quand DK Jr tombe dans le buisson après avoir tenté en vain d’attraper la clé, ou quand il entre en collision avec un ennemi (corbeau ou croco), il perd une vie. À ce moment, une tête de DK Jr apparaît à droite dans la zone Echecs, puis on peut jouer à nouveau. Lorsque 3 vies ont été perdues, la partie est terminée.

## Architecture générale de l’application

Cette application repose sur une architecture en 2 couches :

- La couche présentation, pour le compte de la couche des traitements, crée la fenêtre du jeu, intercepte les événements produits par le joueur (frappe sur une touche du curseur, clic sur la croix de la fenêtre), et réalise les affichages des sprites. 
- La couche des traitements gère tous les traitements relatifs au jeu : gestion de l’état de chaque sprite dans le jeu, prise en compte des collisions/interactions entre les sprites, et gestion du score.

Choisir une architecture en couches a un avantage important : quand on modifie l’implémentation
interne d’une couche, mais que l’on conserve son interface, alors toute autre couche de l’application qui utilise les services de cette couche ne doit pas être modifiée.
La couche présentation est donnée aux étudiants. Le travail va consister à implémenter la couche des traitements.

## La couche présentation

Les fichiers de code relatifs à la couche présentation se trouvent dans le dossier /presentation. Le
fichier presentation.c contient les différentes fonctions, tandis que le fichier presentation.h donné cidessous contient le prototype de chacune de ces fonctions.

```c
#ifndef PRESENTATION_H
#define PRESENTATION_H
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <SDL/SDL.h>
void ouvrirFenetreGraphique();
void afficherCle(int num);
void afficherCorbeau(int colonne, int num);
void afficherCroco(int colonne, int num);
void afficherDKJr(int ligne, int colonne, int num);
void afficherCage(int num);
void afficherRireDK();
void afficherEchec(int nbEchecs);
void afficherScore(int score);
void afficherChiffre(int ligne, int colonne, int chiffre);
void effacerCarres(int ligne, int colonne, int nbLignes = 1, int nbColonnes = 1);
void effacerPoints(int x, int y, int nbX = 1, int nbY = 1);
int lireEvenement();
#endif
```

## La couche des traitements

Le tableau global grilleJeu est un tableau à 2 dimensions constitué de 4 lignes et 8 colonnes qui va
permettre de détecter les collisions pouvant survenir entre différents sprites dans le jeu : la clé, DK
Jr, les crocos et les corbeaux.

```c
S_CASE grilleJeu[4][8];
typedef struct
{
 int type;
 pthread_t tid;
} S_CASE;

```


Chaque cellule de grilleJeu est du type S_CASE :

Le champ type contient le type du sprite. Il peut avoir comme valeur :
```c
#define VIDE 0
#define DKJR 1
#define CROCO 2
#define CORBEAU 3
#define CLE 4
```

 Le champ tid contient l’identifiant du thread qui gère le sprite.
