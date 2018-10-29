#include "linkLayer.h"
#include "appLayer.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

void printStats (LinkLayer * linkLayer)
{
	printf("File name = %s\n", linkLayer->fileName);
	printf("File size = %d\n", linkLayer->fileSize);	
	printf("#RR = %d \n#REJ = %d\n", linkLayer->nRR, linkLayer->nREJ);
	printf("Time: %f s\n", linkLayer->totalTime);
}

int main(int argc, char *argv[]) {

	srand ( time(NULL) );

	LinkLayer * linkLayer = malloc(sizeof(LinkLayer));
	setupLinkLayer (linkLayer,38400);

	if ( (argc != 3) ||
	     ((strcmp("/dev/ttyS0", argv[1])!=0) &&
	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
	   printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
	   exit(1);
	 }

	ApplicationLayer * appLayer = malloc(sizeof(ApplicationLayer));

	if ((strcmp("0", argv[2]) == 0))
		appLayer->status = TRANSMITTER;
	else if ((strcmp("1", argv[2]) == 0))
		appLayer->status = RECEIVER;
	else
		return -1;	
	
	startAppLayer(linkLayer, appLayer);		
	printStats(linkLayer);
	return 0;
}
