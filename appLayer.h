#ifndef APPLAYER_H
#define APPLAYER_H

#include "linkLayer.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>

typedef enum
{
	TRANSMITTER,
	RECEIVER
} Status;

typedef struct
{
	Status status;
} ApplicationLayer;

typedef struct
{
	unsigned int type;
	unsigned char lenght;
	char *value;
} TLV;

typedef struct
{
	unsigned int controlField;
	TLV *parameters;
} ControlPacket;

void startAppLayer(LinkLayer *linkLayer, ApplicationLayer *appLayer);
void transmitter(LinkLayer *linkLayer);
void receiver(LinkLayer *linkLayer);
void send(LinkLayer *linkLayer);
int sendControl(LinkLayer *linkLayer, ControlPacket *controlPacket, int nParameters);
int sendData(LinkLayer *linkLayer, char *buffer, int size, int sequenceNumber);
void receive(LinkLayer *linkLayer);
#endif
