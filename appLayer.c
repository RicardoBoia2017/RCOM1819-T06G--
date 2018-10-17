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
		break;
	case RECEIVER: 
		llopenR (linkLayer);
		break;
	}

	sleep(3);

	switch (appLayer->status)
	{
	case TRANSMITTER:
		llcloseT (linkLayer);
		break;
	case RECEIVER: 
		llcloseR (linkLayer);
		break;
	}
}	
