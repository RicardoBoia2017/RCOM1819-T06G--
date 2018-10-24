#include "linkLayer.h"
#include "appLayer.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*void setupLinkLayer (LinkLayer * linkLayer)
{

}
*/
int main(int argc, char *argv[]) {

	LinkLayer * linkLayer = malloc(sizeof(LinkLayer));
	setupLinkLayer (linkLayer);

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

	return 0;
}
