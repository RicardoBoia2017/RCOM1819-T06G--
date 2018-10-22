#include "appLayer.h"

/*void setupAppLayer (ApplicationLayer * appLayer) //TODO deve ser para acrescentar mais
{
	appLayer->fd = -1;
}*/

void startAppLayer (LinkLayer *linkLayer, ApplicationLayer * appLayer)
{
	openPort (linkLayer); //abre serial port /dev/ttyS0 para leitura e escrita

	setTermiosStructure (linkLayer);

	switch (appLayer->status)
	{
	case TRANSMITTER:
		llopenT (linkLayer);
		sleep(3);
		llcloseT (linkLayer);
		break;
	case RECEIVER: 
		llopenR (linkLayer);
		llcloseR (linkLayer);
		break;
	}

	sleep(3);

}	
