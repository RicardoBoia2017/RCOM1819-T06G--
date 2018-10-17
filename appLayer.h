#ifndef APPLAYER_H
#define APPLAYER_H

//#include "utilities.h"
#include "linkLayer.h"

typedef enum {TRANSMITTER, RECEIVER} Status;

typedef struct {
	Status status;
} ApplicationLayer;

//void setupAppLayer (ApplicationLayer *appLayer);
void startAppLayer (LinkLayer *linkLayer, ApplicationLayer * appLayer);

#endif
