#ifndef APPLAYER_H
#define APPLAYER_H

//#include "utilities.h"
#include "linkLayer.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>

typedef enum {TRANSMITTER, RECEIVER} Status;

typedef struct {
	Status status;
} ApplicationLayer;

typedef struct {

	unsigned int type;
	unsigned char lenght;
	char * value;

} TLV;

typedef struct {

	unsigned int controlField;
	TLV* parameters;
	
} ControlPacket;

typedef struct {

	unsigned int controlField;
	unsigned int sequenceNumber;
	unsigned int nOctets;
	char * data;
} DataPacket; 

//void setupAppLayer (ApplicationLayer *appLayer);
void startAppLayer (LinkLayer *linkLayer, ApplicationLayer * appLayer);
unsigned int getFileSize (char * fileName);
void send (LinkLayer * linkLayer);
void receive (LinkLayer * linkLayer);
int sendControl(LinkLayer * linkLayer, ControlPacket * controlPacket,int nParameters);

#endif
