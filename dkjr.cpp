#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <SDL/SDL.h>
#include "./presentation/presentation.h"

#define VIDE        		0
#define DKJR       		1
#define CROCO       		2
#define CORBEAU     		3
#define CLE 			4

#define AUCUN_EVENEMENT    	0

#define LIBRE_BAS		1
#define LIANE_BAS		2
#define DOUBLE_LIANE_BAS	3
#define LIBRE_HAUT		4
#define LIANE_HAUT		5

void* FctThreadEvenements(void *);
void* FctThreadCle(void *);
void* FctThreadDK(void *);
void* FctThreadDKJr(void *);
void* FctThreadScore(void *);
void* FctThreadEnnemis(void *);
void* FctThreadCorbeau(void *);
void* FctThreadCroco(void *);

void initGrilleJeu();
void setGrilleJeu(int l, int c, int type = VIDE, pthread_t tid = 0);
void afficherGrilleJeu();

void HandlerSIGUSR1(int);
void HandlerSIGUSR2(int);
void HandlerSIGALRM(int);
void HandlerSIGINT(int);
void HandlerSIGQUIT(int);
void HandlerSIGCHLD(int);
void HandlerSIGHUP(int);

void DestructeurVS(void *p);

pthread_t threadCle;
pthread_t threadDK;
pthread_t threadDKJr;
pthread_t threadEvenements;
pthread_t threadScore;
pthread_t threadEnnemis;

pthread_cond_t condDK;
pthread_cond_t condScore;

pthread_mutex_t mutexGrilleJeu;
pthread_mutex_t mutexDK;
pthread_mutex_t mutexEvenement;
pthread_mutex_t mutexScore;

pthread_key_t keySpec;

bool MAJDK = false;
int  score = 0;
bool MAJScore = true;
int  delaiEnnemis = 4000;
int  positionDKJr = 1;
int  evenement = AUCUN_EVENEMENT;
int etatDKJr;
int etatCage;
int echec = 0;

struct sigaction sigAct;

typedef struct
{
  int type;
  pthread_t tid;
} S_CASE;

S_CASE grilleJeu[4][8];

typedef struct
{
  bool haut;
  int position;
} S_CROCO;

// ------------------------------------------------------------------------

int main(int argc, char* argv[])
{
	sigset_t mask;

	// armement des signaux

	// armement du SIGQUIT
	sigemptyset(&sigAct.sa_mask);
	sigAct.sa_handler = HandlerSIGQUIT;
	sigAct.sa_flags = 0;
	sigaction(SIGQUIT, &sigAct, NULL);

	// armement de ALARM
	sigemptyset(&sigAct.sa_mask);
	sigAct.sa_handler = HandlerSIGALRM;
	sigAct.sa_flags = 0;
	sigaction(SIGALRM, &sigAct, NULL);

	// armement de SIGUSR1
	sigemptyset(&sigAct.sa_mask);
	sigAct.sa_handler = HandlerSIGUSR1;
	sigAct.sa_flags = 0;
	sigaction(SIGUSR1, &sigAct, NULL);

	// armement de SIGINT
	sigemptyset(&sigAct.sa_mask);
	sigAct.sa_handler = HandlerSIGINT;
	sigAct.sa_flags = 0;
	sigaction(SIGINT, &sigAct, NULL);

	// armement de SIGUSRS2
	sigemptyset(&sigAct.sa_mask);
	sigAct.sa_handler = HandlerSIGUSR2;
	sigAct.sa_flags = 0;
	sigaction(SIGUSR2, &sigAct, NULL);

	// armement de SIGHUP
	sigemptyset(&sigAct.sa_mask);
	sigAct.sa_handler = HandlerSIGHUP;
	sigAct.sa_flags = 0;
	sigaction(SIGHUP, &sigAct, NULL);

	// armement de SIGCHLD
	sigemptyset(&sigAct.sa_mask);
	sigAct.sa_handler = HandlerSIGCHLD;
	sigAct.sa_flags = 0;
	sigaction(SIGCHLD, &sigAct, NULL);

	// masquage des signaux ("pour tout les threads")
	sigemptyset(&mask);
	sigaddset(&mask, SIGQUIT);
	sigaddset(&mask, SIGALRM);
	sigaddset(&mask, SIGUSR1);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGUSR2);
	sigaddset(&mask, SIGHUP);
	sigaddset(&mask, SIGCHLD);

	sigprocmask(SIG_BLOCK, &mask, NULL);

	// Creation de la cle specifique pour les ennemis
	pthread_key_create(&keySpec, DestructeurVS);



	// Creation de la fenetre Graphique
	ouvrirFenetreGraphique();

	//Creation des différents threads
	pthread_create(&threadCle, NULL, FctThreadCle, NULL); // Cle

	//evenement
	pthread_mutex_init(&mutexEvenement, NULL); 
	pthread_create(&threadEvenements, NULL, FctThreadEvenements, NULL);

	// DK
	pthread_mutex_init(&mutexDK, NULL);
	pthread_cond_init(&condDK, NULL);
	pthread_create(&threadDK, NULL, FctThreadDK, NULL);

	// Score
	pthread_mutex_init(&mutexScore, NULL);
	pthread_cond_init(&condScore, NULL);
	pthread_create(&threadScore, NULL, FctThreadScore, NULL);

	// Ennemis
	pthread_create(&threadEnnemis, NULL, FctThreadEnnemis, NULL);
	
	afficherScore(0);

	//DKjr
	pthread_mutex_init(&mutexGrilleJeu, NULL);

	while(echec < 3)
	{
		pthread_create(&threadDKJr, NULL, FctThreadDKJr, NULL);
		pthread_join(threadDKJr, NULL);
		echec++;
		afficherEchec(echec);
	}

	pause();

	
}

// -------------------------------------

void initGrilleJeu()
{
  int i, j;   
  
  pthread_mutex_lock(&mutexGrilleJeu);

  for(i = 0; i < 4; i++)
    for(j = 0; j < 7; j++)
      setGrilleJeu(i, j);

  pthread_mutex_unlock(&mutexGrilleJeu);
}

// -------------------------------------

void setGrilleJeu(int l, int c, int type, pthread_t tid)
{
  grilleJeu[l][c].type = type;
  grilleJeu[l][c].tid = tid;
}

// -------------------------------------

void afficherGrilleJeu()
{   
   for(int j = 0; j < 4; j++)
   {
       for(int k = 0; k < 8; k++)
          printf("%d  ", grilleJeu[j][k].type);
       printf("\n");
   }

   printf("\n");   
}

void* FctThreadCle(void *)
{
	struct timespec temps = { 0, 700000000 };

	while(1)
	{
		for (int i = 1; i < 5; ++i)
		{
			afficherCle(i);

			if (i == 1)
			{
				setGrilleJeu(0,1,CLE, pthread_self());
			}
			else
			{
				setGrilleJeu(0,1,VIDE, pthread_self());
			}

			nanosleep(&temps, NULL);
			effacerCarres(3,12,2,3);
			
		}
		for (int i = 3; i > 1; --i)
		{
			afficherCle(i);

			if (i == 1)
			{
				setGrilleJeu(0,1,CLE, pthread_self());
			}
			else
			{
				setGrilleJeu(0,1,VIDE, pthread_self());
			}

			nanosleep(&temps, NULL);
			effacerCarres(3,12,2,3);


		}
	}
}

void* FctThreadEvenements(void *)
{
	int evt;
	struct timespec temps = {0, 100000000};
	while (1)
	{
    evt = lireEvenement();

    pthread_mutex_lock(&mutexEvenement);

    evenement = evt;

    switch (evt)
    {
		case SDL_QUIT:
			exit(0);
		case SDLK_UP:
			printf("KEY_UP\n");
			break;
		case SDLK_DOWN:
			printf("KEY_DOWN\n");
			break;
		case SDLK_LEFT:
			printf("KEY_LEFT\n");
			break;
		case SDLK_RIGHT:
			printf("KEY_RIGHT\n");
			
	   }

	   pthread_mutex_unlock(&mutexEvenement);

	   kill(getpid(), SIGQUIT);

	   nanosleep(&temps, NULL);

	   evenement = AUCUN_EVENEMENT;
	}
}

// --- THREAD POUR LES HOMIES ---

void* FctThreadDKJr(void* p)
{
	// printf("dk : %d\n", pthread_self());

	sigset_t mask;


	struct timespec dureeJump = {1 , 400000000}; // duree du saut de dkjr
	struct timespec dureeFinalJump = {0, 500000000};

	bool on = true; 

	// on enleve le masque sur SIGQUIT et SIGINT
	sigemptyset(&mask);
	sigaddset(&mask, SIGQUIT);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGCHLD);
	sigaddset(&mask, SIGHUP);
	sigprocmask(SIG_UNBLOCK, &mask, NULL);

	pthread_mutex_lock(&mutexGrilleJeu);

	setGrilleJeu(3, 1, DKJR); 
	afficherDKJr(11, 9, 1); 

	pthread_mutex_unlock(&mutexGrilleJeu);

	etatDKJr = LIBRE_BAS; 
	positionDKJr = 1;

	// anti-spawnKill
	if(grilleJeu[2][0].tid != 0)
		pthread_kill(grilleJeu[2][0].tid, SIGUSR1);
	if(grilleJeu[2][1].tid != 0)
		pthread_kill(grilleJeu[2][1].tid, SIGUSR1);
	if(grilleJeu[2][2].tid != 0)
		pthread_kill(grilleJeu[2][2].tid, SIGUSR1);

	if(grilleJeu[3][1].tid != 0)
		pthread_kill(grilleJeu[3][1].tid, SIGUSR2);
	if(grilleJeu[3][2].tid != 0)
		pthread_kill(grilleJeu[3][2].tid, SIGUSR2);
	if(grilleJeu[3][3].tid != 0)
		pthread_kill(grilleJeu[3][3].tid, SIGUSR2);

	while (on)
	{
		pause();
		pthread_mutex_lock(&mutexEvenement);
		pthread_mutex_lock(&mutexGrilleJeu);
		switch (etatDKJr)
		{
			case LIBRE_BAS:

			switch (evenement)
			{
				case SDLK_LEFT:
					if (positionDKJr > 1)
					{
						
						// On fixe à VIDE le champ type de la cellule grilleJeu[3][positionDKJr]
						setGrilleJeu(3, positionDKJr);
						// on efface l’image de
						// DK Jr à l’écran avec effacerCarre(). Les paramètres passés à cette fonction sont : 11 est le numéro
						// de la ligne dans la fenêtre, (positionDKJr * 2) + 7 transforme la position horizontale courante de
						// DK Jr en un numéro de colonne à l’écran, et les valeurs 2 indiquent le nombre de carrés par
						// colonne et par ligne à effacer
						effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
						// Après, on déplace DK Jr vers la gauche d’une position horizontale en
						// décrémentant positionDKJr
						positionDKJr--;
						// Puis, on fixe à DKJR le champ type de la cellule grilleJeu[3]
						// [positionDKJr]. Enfin, on affiche DK Jr à sa nouvelle position
						setGrilleJeu(3, positionDKJr, DKJR);
						// Au niveau des paramètres transmis
						// à afficherDKJr(), on trouve ((positionDKJr - 1) % 4) + 1 qui sélectionne l’image de DK Jr à
						// afficher parmi les 4 images disponibles dans images/dkjr
						afficherDKJr(11, (positionDKJr * 2) + 7, ((positionDKJr - 1) % 4) + 1);
					}
					break;
				case SDLK_RIGHT:
						
					// pour verifier que le joueur ne dépasse pas le cadre de la fenetre
					if (positionDKJr < 7)
					{
						// meme logique du case SDLK_LEFT mais en incrémentant postitionDKJr pour aller a droite
						setGrilleJeu(3, positionDKJr);
						effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
						positionDKJr++;
						setGrilleJeu(3, positionDKJr, DKJR);
						afficherDKJr(11, (positionDKJr * 2) + 7, ((positionDKJr - 1) % 4) + 1);
					}
					break;
				case SDLK_UP:
					
					if (grilleJeu[2][positionDKJr].type == 3)
					{
						pthread_kill(grilleJeu[2][positionDKJr].tid, SIGUSR1);
						effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
						setGrilleJeu(3, positionDKJr);
						pthread_mutex_unlock(&mutexEvenement);
						pthread_mutex_unlock(&mutexGrilleJeu);

						pthread_exit(0);
					}
					// meme logique du case SDLK_LEFT ou Right sauf que on monte juste la ligne sans le changer de pos
					setGrilleJeu(3, positionDKJr);
					effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
					setGrilleJeu(2, positionDKJr, DKJR);
					

					if (positionDKJr == 1 || positionDKJr == 5)
					{
						afficherDKJr(10, (positionDKJr * 2) + 7, 7);
						etatDKJr = LIANE_BAS;
					}
					else
					{
						if (positionDKJr == 7)
						{
							afficherDKJr(10, (positionDKJr * 2) + 7, 8);
							etatDKJr = DOUBLE_LIANE_BAS;
						}
						else
						{
							afficherDKJr(10, (positionDKJr * 2) + 7, ((positionDKJr - 1) % 4) + 1);
							// Lorsque DK Jr saute sans s’accrocher à une liane
							// DK Jr reste simplement en l’air pendant 1,4 seconde après quoi il revient sur le sol.
							// Durant cette attente de 1,4 seconde, ThreadDKJr libère mutexGrilleJeu pour permettre
							// aux autres threads de continuer à accéder au tableau grilleJeu, mais il conserve mutexEvenement.
							pthread_mutex_unlock(&mutexGrilleJeu);
							nanosleep(&dureeJump, NULL); // 1,4 sec
							pthread_mutex_lock(&mutexGrilleJeu);

							if (grilleJeu[3][positionDKJr].type == CROCO)
							{
								pthread_kill(grilleJeu[3][positionDKJr].tid, SIGUSR2);
								effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);
								setGrilleJeu(2, positionDKJr);
								pthread_mutex_unlock(&mutexEvenement);
								pthread_mutex_unlock(&mutexGrilleJeu);

								pthread_exit(0);
							}

							setGrilleJeu(2, positionDKJr);
							effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);
							setGrilleJeu(3, positionDKJr, DKJR);
							afficherDKJr(11, (positionDKJr * 2) + 7, ((positionDKJr - 1) % 4) + 1);
						}
					}
					
					
					break;
			}
			break;

			case LIANE_BAS:
				switch(evenement)
				{
					case SDLK_DOWN:
						if (grilleJeu[3][positionDKJr].type == CROCO)
						{
							pthread_kill(grilleJeu[3][positionDKJr].tid, SIGUSR2);
							effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);
							setGrilleJeu(2, positionDKJr);
							pthread_mutex_unlock(&mutexEvenement);
							pthread_mutex_unlock(&mutexGrilleJeu);

							pthread_exit(0);
						}
						// pour faire descendre dkJr de sa lianes
						setGrilleJeu(2, positionDKJr);
						effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);
						setGrilleJeu(3, positionDKJr, DKJR);
						afficherDKJr(11, (positionDKJr * 2) + 7, ((positionDKJr - 1) % 4) + 1);
						etatDKJr = LIBRE_BAS;
						break;
				}
				break;
			case DOUBLE_LIANE_BAS:
				switch(evenement)
				{
					case SDLK_DOWN:
						// pour faire descendre dkJr de sa lianes
						setGrilleJeu(2, positionDKJr);
						effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);
						setGrilleJeu(3, positionDKJr, DKJR);
						afficherDKJr(11, (positionDKJr * 2) + 7, ((positionDKJr - 1) % 4) + 1);
						etatDKJr = LIBRE_BAS;
						break;
					case SDLK_UP:
						// pour faire monter dkjr dans LIBRES_HAUT
						setGrilleJeu(2, positionDKJr);
						effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);
						setGrilleJeu(1, positionDKJr, DKJR);
						afficherDKJr(7, (positionDKJr * 2) + 7, ((positionDKJr - 1) % 4) + 1);
						etatDKJr = LIBRE_HAUT;
						break;
				}
				break;
			case LIBRE_HAUT:
				switch (evenement)
				{
					case SDLK_LEFT:
						if (positionDKJr >= 3)
						{
							setGrilleJeu(1, positionDKJr);
							effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);
							positionDKJr--;

							// partie récupération de cle (a retravailler)
							// peut etre faire positionDKJr-- apres le if
							if (positionDKJr == 2)
							{
								setGrilleJeu(1, positionDKJr, DKJR);
								if (grilleJeu[0][1].type == CLE)
								{
									// si dkjr prend la cle
									pthread_mutex_lock(&mutexDK);
									MAJDK = true;
									pthread_mutex_unlock(&mutexDK);
									
									pthread_mutex_lock(&mutexDK);
									etatCage++;
									pthread_mutex_unlock(&mutexDK);
									pthread_cond_signal(&condDK);

									afficherDKJr(7, (positionDKJr * 2) + 7, 10);
									setGrilleJeu(7,positionDKJr); // pour ne pas se faire avoir par la hitbox

									// saut ou dkjr recupere la cle
									nanosleep(&dureeFinalJump, NULL); // 0,5 sec
									etatDKJr = LIBRE_BAS;
									setGrilleJeu(1, positionDKJr);
									effacerCarres(3, 10, 3, 3);

									// anti-spawnKill
									if(grilleJeu[2][0].tid != 0)
										pthread_kill(grilleJeu[2][0].tid, SIGUSR1);
									if(grilleJeu[2][1].tid != 0)
										pthread_kill(grilleJeu[2][1].tid, SIGUSR1);
									if(grilleJeu[2][2].tid != 0)
										pthread_kill(grilleJeu[2][2].tid, SIGUSR1);

									if(grilleJeu[3][1].tid != 0)
										pthread_kill(grilleJeu[3][1].tid, SIGUSR2);
									if(grilleJeu[3][2].tid != 0)
										pthread_kill(grilleJeu[3][2].tid, SIGUSR2);
									if(grilleJeu[3][3].tid != 0)
										pthread_kill(grilleJeu[3][3].tid, SIGUSR2);

									// animation dkjr ds le buisson
									// afficherDKJr(11, 7, 13);
									// nanosleep(&dureeJump, NULL);
									// effacerCarres(11,7,2,2);

									positionDKJr = 1;
									setGrilleJeu(3, positionDKJr, DKJR);
									afficherDKJr(11, (positionDKJr * 2) + 7, ((positionDKJr - 1) % 4) + 1);
								}
								else
								{
									// si dkjr rate la cle
									afficherDKJr(7, (positionDKJr * 2) + 7, 9); // 9 = dk rate la key
									setGrilleJeu(7,positionDKJr); // pour ne pas se faire avoir par la hitbox
									nanosleep(&dureeJump, NULL);
									effacerCarres(5,12,3,2);
									etatDKJr = LIBRE_BAS;
									positionDKJr = 1;

									// animation dkjr ds le buisson
									afficherDKJr(11, 7, 13);
									nanosleep(&dureeJump, NULL);
									effacerCarres(11,7,2,2);
									pthread_mutex_unlock(&mutexGrilleJeu);
									pthread_mutex_unlock(&mutexEvenement);
									pthread_exit(0);
								}
							}
							else
							{
								setGrilleJeu(1, positionDKJr, DKJR);
								afficherDKJr(7, (positionDKJr * 2) + 7, ((positionDKJr - 1) % 4) + 1);
							}
						}
						break;
					case SDLK_RIGHT:
						if (positionDKJr < 7)
						{
							setGrilleJeu(1, positionDKJr);
							effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);
							positionDKJr++;
							setGrilleJeu(1, positionDKJr, DKJR);
							afficherDKJr(7, (positionDKJr * 2) + 7, ((positionDKJr - 1) % 4) + 1);
						}
						break;

					case SDLK_UP:
						
						if (positionDKJr == 3 || positionDKJr == 4)
						{
							setGrilleJeu(1, positionDKJr);
							effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);
							setGrilleJeu(0, positionDKJr, DKJR);
							afficherDKJr(5, (positionDKJr * 2) + 7, 8);

							pthread_mutex_unlock(&mutexGrilleJeu);
							nanosleep(&dureeJump, NULL); // 1,4 sec
							pthread_mutex_lock(&mutexGrilleJeu);
							if (grilleJeu[1][positionDKJr].type == CROCO)
							{
								pthread_kill(grilleJeu[1][positionDKJr].tid, SIGUSR2);
								effacerCarres(5, (positionDKJr * 2) + 7, 2, 2);
								setGrilleJeu(0, positionDKJr);
								pthread_mutex_unlock(&mutexEvenement);
								pthread_mutex_unlock(&mutexGrilleJeu);

								pthread_exit(0);
							}

							setGrilleJeu(0, positionDKJr);
							effacerCarres(5, (positionDKJr * 2) + 7, 2, 2);
							setGrilleJeu(1, positionDKJr, DKJR);
							afficherDKJr(7, (positionDKJr * 2) + 7, ((positionDKJr - 1) % 4) + 1);
						}
						else
						{
							if (positionDKJr == 6)
							{
								setGrilleJeu(1, positionDKJr);
								effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);
								setGrilleJeu(0, positionDKJr, DKJR);
								afficherDKJr(5, (positionDKJr * 2) + 7, 7);
								etatDKJr = LIANE_HAUT;
							}
						}
						
						
						break;
					case SDLK_DOWN:
						if (positionDKJr == 7)
						{
							setGrilleJeu(1, positionDKJr);
							effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);
							setGrilleJeu(2, positionDKJr, DKJR);
							afficherDKJr(10, (positionDKJr * 2) + 7, 8);
							etatDKJr = DOUBLE_LIANE_BAS;
						}
						break;
					}
				break;
			case LIANE_HAUT:
				switch(evenement)
				{
					case SDLK_DOWN:
						if (grilleJeu[1][positionDKJr].type == CROCO)
						{
							pthread_kill(grilleJeu[1][positionDKJr].tid, SIGUSR2);
							effacerCarres(5, (positionDKJr * 2) + 7, 2, 2);
							setGrilleJeu(0, positionDKJr);
							pthread_mutex_unlock(&mutexEvenement);
							pthread_mutex_unlock(&mutexGrilleJeu);

							pthread_exit(0);
						}
						// pour faire descendre dkJr de sa lianes
						setGrilleJeu(0, positionDKJr);
						effacerCarres(5, (positionDKJr * 2) + 7, 2, 2);
						setGrilleJeu(1, positionDKJr, DKJR);
						afficherDKJr(7, (positionDKJr * 2) + 7, ((positionDKJr - 1) % 4) + 1);
						etatDKJr = LIBRE_HAUT;
						break;
				}
				break;
		}
		pthread_mutex_unlock(&mutexGrilleJeu);
		pthread_mutex_unlock(&mutexEvenement);
		afficherGrilleJeu();
	}
	pthread_exit(0);
}

void* FctThreadDK(void*)
{
	struct timespec dureeSourire = {0, 700000000}; // duree du sourire lorsque la cage est libere

	while(1)
	{
		afficherCage(1);
		afficherCage(2);
		afficherCage(3);
		afficherCage(4);
		etatCage = 0;

		pthread_mutex_lock(&mutexDK); 
		//verifier la varialble majdk
	 	while (etatCage < 4 && MAJDK == false) 
	 	{
	 		pthread_cond_wait(&condDK, &mutexDK); 
	 		pthread_mutex_lock(&mutexGrilleJeu);
	 		// on enleve la partie de la cage
	 		switch(etatCage)
	 		{
	 			case 1:
	 				effacerCarres(2,7,2,2);
	 				break;
	 			case 2:
	 				effacerCarres(4,7,2,2);
	 				break;
	 			case 3:
	 				effacerCarres(4,9,2,2);
	 				break;
	 			
	 		}

	 		// Lorsque DK Jr ouvre une partie de la grille de la cage, ThreadDKJr augmente le contenu de
			// la variable score de 10 et assigne à la variable MAJScore la valeur true, puis il réveille
			// ThreadScore qui était en attente sur la variable de condition condScore.
			pthread_mutex_lock(&mutexScore);
	 		score = score + 10;
	 		MAJDK = false;
	 		pthread_mutex_unlock(&mutexScore);
	 		pthread_cond_signal(&condScore);

	 		pthread_mutex_unlock(&mutexGrilleJeu);
	 		
	 	}



		pthread_mutex_unlock(&mutexDK);
		// Durant l’ouverture d’une partie de la cage ou la chute de DK Jr dans le buisson, ThreadDKJr
		// conserve mutexGrille. Ceci a pour effet de faire attendre les autres sprites du jeu.

		pthread_mutex_lock(&mutexGrilleJeu);
		effacerCarres(2,9,2,2);

		afficherRireDK();

		// Lorsque la grille est totalement ouverte et que DK rigole, ThreadDK augmente le contenu
		// de la variable score de 10 et assigne à la variable MAJScore la valeur true. puis il réveille
		// ThreadScore qui était en attente sur la variable de condition condScore.
		pthread_mutex_lock(&mutexScore);
		score = score + 10;
		MAJDK = false;
		pthread_mutex_unlock(&mutexScore);
		pthread_cond_signal(&condScore);


		nanosleep(&dureeSourire, NULL);

		effacerCarres(3,8,2,2);

		pthread_mutex_unlock(&mutexGrilleJeu);

	}

	pthread_exit(0);

}

void* FctThreadScore(void*)
{
	while(1)
	{
		pthread_mutex_lock(&mutexScore); 
	 	while (MAJScore == false) 
	 	{
	 		pthread_cond_wait(&condScore, &mutexScore); 
			afficherScore(score);
	 	}
	 	MAJScore = false;
		pthread_mutex_unlock(&mutexScore);
	}

	pthread_exit(0);
}

// --- THREAD SUR LES OPS ---

void* FctThreadEnnemis(void*)
{
	sigset_t mask;

	// on enleve le masque sur SIGALRM
	sigemptyset(&mask);
	sigaddset(&mask, SIGALRM);
	sigprocmask(SIG_UNBLOCK, &mask, NULL);

	alarm(15); // On arme l'alarme ici la premiere fois car après c'est l'alarm qui le fait

	// pour avoir une rng différente entre les sessions
	srand(time(NULL));

	pthread_t thread;

	struct timespec delaySpawnEnnemis = {delaiEnnemis / 1000, delaiEnnemis * 100000};

	while(1)
	{
		nanosleep(&delaySpawnEnnemis, NULL);

		// on met a jour le delay d'apparition des ennemis
		delaySpawnEnnemis = {delaiEnnemis / 1000, delaiEnnemis * 100000};

		if ((rand() % 2) == 0)
		{
			pthread_create(&thread,NULL,FctThreadCroco, NULL);

		}
		else
		{
			pthread_create(&thread,NULL,FctThreadCorbeau, NULL);
		}

	}

	pthread_exit(0);
}

void* FctThreadCroco(void*)
{
	// on a rajouter -fpermissive dans le makefile pour pouvoir compiler 
	S_CROCO *pos = malloc(sizeof(S_CROCO));
  *pos = {true, 2};
	pthread_setspecific(keySpec, (void*)pos);

	sigset_t mask;

	// on enleve le masque sur 3 snignaux
	sigemptyset(&mask);
	sigaddset(&mask, SIGUSR2);
	sigprocmask(SIG_UNBLOCK, &mask, NULL);

	struct timespec delayAvance = {0, 700000000};

	while(pos->position > 1)
	{
		if (pos->position == 8)
		{

			afficherCroco(pos->position * 2 + 7, 3);
			nanosleep(&delayAvance, NULL);
			effacerCarres(9, pos->position * 2 + 7, 1, 1);
			pos->position--;
			pos->haut = false;
			continue;
		}
		if (pos->haut == true)
		{
			pthread_mutex_lock(&mutexGrilleJeu);
			if (grilleJeu[1][pos->position].type == 1)
			{
				printf("COLLISION croco\n");
				kill(getpid(), SIGHUP);
				pthread_mutex_unlock(&mutexGrilleJeu);
				pthread_exit(0);
			}
			
			setGrilleJeu(1, pos->position, CROCO, pthread_self());
			pthread_mutex_unlock(&mutexGrilleJeu);
			afficherCroco(pos->position * 2 + 7, (pos->position - 1) % 2 + 1);

			nanosleep(&delayAvance, NULL);

			pthread_mutex_lock(&mutexGrilleJeu);
			setGrilleJeu(1, pos->position);
			pthread_mutex_unlock(&mutexGrilleJeu);
			effacerCarres(8, pos->position * 2 + 7, 1, 1);

			pos->position = pos->position + 1;
		}
		else // si c'est pas en haut c'est forcement en bas
		{
			pthread_mutex_lock(&mutexGrilleJeu);
			if (grilleJeu[3][pos->position].type == 1)
			{
				printf("COLLISION\n");
				kill(getpid(), SIGCHLD);
				pthread_mutex_unlock(&mutexGrilleJeu);
				pthread_exit(0);
			}
			setGrilleJeu(3, pos->position, CROCO, pthread_self());
			afficherCroco(pos->position * 2 + 7, (pos->position - 1) % 2 + 4);	
			pthread_mutex_unlock(&mutexGrilleJeu);

			nanosleep(&delayAvance, NULL);

			pthread_mutex_lock(&mutexGrilleJeu);
			setGrilleJeu(3, pos->position);
			effacerCarres(12, pos->position * 2 + 7, 1, 1);
			pthread_mutex_unlock(&mutexGrilleJeu);

			pos->position = pos->position - 1;
		}
		
	}
	// *** Version sans le pos.haut ***
	// // croco qui marche de gauche a droite en haut
	// while(pos.position < 8)
	// {
	// 	setGrilleJeu(1, pos.position, CROCO, pthread_self());
	// 	afficherCroco(pos.position * 2 + 7, (pos.position - 1) % 2 + 1);
	// 	nanosleep(&delayAvance, NULL);
	// 	setGrilleJeu(1, pos.position);
	// 	effacerCarres(8, pos.position * 2 + 7, 1, 1);
	// 	pos.position++;
	// }

	// // croco qui tombe
	// // pas besoin de le mettre dans le grille de jeu car il en ai en dehors, on ne fait que l'affiche tomber
	// afficherCroco(pos.position * 2 + 7, 3);
	// nanosleep(&delayAvance, NULL);
	// effacerCarres(9, pos.position * 2 + 7, 1, 1);
	// pos.position--;
	
	// // croco qui marche de droite a gauche en bas
	// while(pos.position > 1)
	// {

	// 	setGrilleJeu(3, pos.position, CROCO, pthread_self());
	// 	afficherCroco(pos.position * 2 + 7, (pos.position - 1) % 2 + 4);
	// 	nanosleep(&delayAvance, NULL);
	// 	setGrilleJeu(3, pos.position);
	// 	effacerCarres(12, pos.position * 2 + 7, 1, 1);
	// 	pos.position--;
	// }

	pthread_exit(0);
}

void* FctThreadCorbeau(void*)
{

	// on a rajouter -fpermissive dans le makefile pour pouvoir compiler 
	int *pos = malloc(sizeof(int));
  *pos = 0;
	pthread_setspecific(keySpec, (void*)pos);

	sigset_t mask;

	// on enleve le masque sur 3 signals
	sigemptyset(&mask);
	sigaddset(&mask, SIGUSR1);
	sigprocmask(SIG_UNBLOCK, &mask, NULL);

	struct timespec delayAvance = {0, 700000000};

	while(*pos < 8)
	{
		pthread_mutex_lock(&mutexGrilleJeu);
		if (grilleJeu[2][*pos].type == 1)
		{
			printf("COLLISION CORBEAU\n");
			pthread_mutex_unlock(&mutexGrilleJeu);
			kill(getpid(), SIGINT);
			pthread_exit(0);
		}
		
		setGrilleJeu(2, *pos, CORBEAU, pthread_self());
		pthread_mutex_unlock(&mutexGrilleJeu);
		afficherCorbeau(*pos * 2 + 8, (*pos - 1) % 2 + 1);

		nanosleep(&delayAvance, NULL);

		pthread_mutex_lock(&mutexGrilleJeu);
		setGrilleJeu(2, *pos);
		pthread_mutex_unlock(&mutexGrilleJeu);
		effacerCarres(9, *pos * 2 + 8, 2, 1);

		*pos = *pos + 1;
		

	}

	pthread_exit(0);
}

void DestructeurVS(void* p)
{
	#ifdef DEBUG
	int val = *((int*)p);
	printf("Destruction de la variable spec pour le thread %d id = %u\n", val, (unsigned int)pthread_self());
	#endif
	fflush(stdout);
	free(p);
}


// --- SIGNALS ---
void HandlerSIGQUIT(int sig)
{
	#ifdef DEBUG
	printf("SIGQUIT pour le thread (%d)\n", pthread_self());
	#endif
}


// La difficulté du jeu augmente avec le temps. Toutes les 15 secondes, le signal
// SIGALRM est envoyé à ThreadEnnemis. Cette alarme est initialisée au départ dans
// ThreadEnnemis. Dans le handler de SIGALRM, on diminue de 0,25 seconde le délai stocké dans la
// variable delaiEnnemis, puis on réinitialise l’alarme à 15 secondes. On cesse de réinitialiser l’alarme
// dans le handler de SIGALRM quand la variable delaiEnnemis stocke un délai de 2,5 secondes qui
// est le délai minimum.
void HandlerSIGALRM(int sig)
{
	if (delaiEnnemis > 2500)
	{
		delaiEnnemis = delaiEnnemis - 250;
		alarm(15);
	
	}
	#ifdef DEBUG
	else
	{
		printf("(DEBUG)Delay maximum atteint (%d)\n", delaiEnnemis);
	}
	printf("(DEBUG)Réduction du delay de 0,25 sec : %d\n", delaiEnnemis);

	#endif
}

// si DK Jr se
// trouve à la position horizontale dans laquelle doit être placé ce corbeau, corbeau envoie SIGINT a dkjr
void HandlerSIGINT(int sig)
{
	setGrilleJeu(2, positionDKJr);
	effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);
	if (etatDKJr == LIBRE_BAS)
	{
		pthread_mutex_unlock(&mutexEvenement);
	}
	pthread_exit(0);
}

void HandlerSIGHUP(int sig)
{
	setGrilleJeu(1, positionDKJr);
	effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);
	if (etatDKJr == LIBRE_BAS)
	{
		pthread_mutex_unlock(&mutexEvenement);
	}
	pthread_exit(0);
}

void HandlerSIGCHLD(int sig)
{
	setGrilleJeu(3, positionDKJr);
	effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
	if (etatDKJr == LIBRE_BAS)
	{
		pthread_mutex_unlock(&mutexEvenement);
	}
	pthread_exit(0);
}

// ThreadDKJr vérifie dans grilleJeu si
// aucun corbeau n’est présent dans la ligne 2 à cette position horizontale. Si c’est le cas,
// ThreadDKJr envoie le signal SIGUSR1 au ThreadCorbeau qui gère ce corbeau
void HandlerSIGUSR1(int sig)
{
	pthread_mutex_lock(&mutexGrilleJeu);

	int pos = *(int*)pthread_getspecific(keySpec);
	effacerCarres(9, (int)pos * 2 + 8, 2, 1);
	setGrilleJeu(2, (int)pos);

	pthread_mutex_unlock(&mutexGrilleJeu);
	
	pthread_exit(0);
}

// ThreadDKJr vérifie dans
// grilleJeu si aucun croco n’est présent à sa position horizontale. Si c’est le cas, ThreadDKJr
// envoie le signal SIGUSR2 au ThreadCroco qui gère ce croco
void HandlerSIGUSR2(int sig)
{
	S_CROCO pos = *(S_CROCO*)pthread_getspecific(keySpec);

	pthread_mutex_lock(&mutexGrilleJeu);

	if (pos.haut == true)
	{
		effacerCarres(8, pos.position * 2 + 7, 1, 1);
		setGrilleJeu(1, pos.position);
	}
	else
	{
		effacerCarres(12, pos.position * 2 + 7, 1, 1);
		setGrilleJeu(3, pos.position);
	}

	pthread_mutex_unlock(&mutexGrilleJeu);
	
	
	pthread_exit(0);
}