#ifndef LINKLAYER_H
#define LINKLAYER_H

#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <strings.h>
#include <signal.h>
#include <time.h>



#define BAUDRATE B38400

typedef struct 
{
	int fd; //Descritor de ficheiro
	char *port; /*Dispositivo /dev/ttySx, x = 0, 1*/
	int baudRate; /*Velocidade de transmissão*/
	unsigned int sequenceNumber; /*Número de sequência da trama: 0, 1*/  
	unsigned int timeout; /*Valor do temporizador: 1 s*/
	unsigned int numTransmissions;  /*Número de tentativas em caso de falha*/
	char * frame; //trama
	
	char * fileName; //nome do ficheiro

	unsigned int nRR;
	unsigned int nREJ; 
	double totalTime;
} LinkLayer;

struct termios oldtio, newtio; 

void setupLinkLayer (LinkLayer *linkLayer);
void openPort (LinkLayer * linkLayer);
void setTermiosStructure (LinkLayer * linkLayer);
void llopenT (LinkLayer * linkLayer); // Tem que retornar inteiro
void llopenR (LinkLayer * linkLayer); // Tem que retornar inteiro
int llwrite (LinkLayer *linkLayer, char * buffer, int lenght);
int llread (LinkLayer * linkLayer);
void llcloseT (LinkLayer * linkLayer); // Tem que retornar inteiro
void llcloseR (LinkLayer * linkLayer); // Tem que retornar inteiro

#endif
