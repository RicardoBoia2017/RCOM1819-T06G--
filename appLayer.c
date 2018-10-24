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
		send (linkLayer);
		llcloseT (linkLayer);
		break;
	case RECEIVER: 
		llopenR (linkLayer);
		receive (linkLayer);
		llcloseR (linkLayer);
		break;
	}

}	

void send (LinkLayer * linkLayer)
{
	
}

void receive (LinkLayer * linkLayer)
{

}
