#include "appLayer.h"
//#include "utilities.c"

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
//		llopenT (linkLayer);
		send (linkLayer);
//		llcloseT (linkLayer);
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
	char sizeString[16];	
	
	//Start control packet
	ControlPacket startCP;	
	startCP.controlField = 2;

	//control packet name
 	TLV startTLVName;
	
	startTLVName.type = 1;
	startTLVName.lenght = strlen(linkLayer->fileName);
	startTLVName.value = linkLayer->fileName;	
	printf("%d %s\n", startTLVName.lenght, startTLVName.value);	
	//control packet size
 	TLV startTLVSize;

	unsigned int fileSize = getFileSize(linkLayer->fileName);

	sprintf(sizeString,"%d", fileSize);

	startTLVSize.type = 0;
	startTLVSize.lenght =  strlen(sizeString);
	startTLVSize.value = sizeString;
	
	TLV listParameters [2] = {startTLVSize, startTLVName};
	startCP.parameters = listParameters;
 	
	clock_t startTime = clock();

	sendControl(linkLayer,&startCP,2);

	//Data packet

	FILE* file = fopen(linkLayer->fileName,"rb");; //Mudar isto e colocar função das utilities

	DataPacket dataPacket;
	dataPacket.data = malloc(fileSize);
	int nBytesRead = 0;
	int sequenceNumber = 0;
	while((nBytesRead = fread(dataPacket.data, sizeof(char), MAX_SIZE, file)) > 0)
	{
//		printf("%d\n", nBytesRead);
		sendData(linkLayer, dataPacket.data, nBytesRead, sequenceNumber++ % 255);

		memset (dataPacket.data, 0, 255);
		if (linkLayer->sequenceNumber)
			linkLayer->sequenceNumber = 0;
		else
			linkLayer->sequenceNumber = 1;  
	}	

	//close file

	//End control packet
	ControlPacket endCP;
	endCP = startCP;
	endCP.controlField = 3;	

 	tcflush(linkLayer->fd, TCIOFLUSH);

	sendControl(linkLayer,&endCP,2);

	clock_t endTime = clock();

	linkLayer->totalTime = (double) (endTime - startTime)/CLOCKS_PER_SEC;
}

unsigned int getFileSize (char * fileName)
{
	struct stat st;
	if (stat(fileName, &st) < 0)
	{
		perror("getFileSize");
		exit(-1);
	}
	return st.st_size;
}

int sendControl(LinkLayer * linkLayer, ControlPacket * controlPacket,int nParameters)
{
	unsigned int i = 0, packetSize = 1, index = 1;


	for(i;i<nParameters;i++)
		packetSize += 2 + controlPacket->parameters[i].lenght;

	unsigned char frame[packetSize];
	frame[0] = controlPacket->controlField; //conversão de inteiro para char (pode estar errado)
	

	for (i = 0; i < nParameters; i++)
	{
		frame[index++] = controlPacket->parameters[i].type;
  		frame[index++] = controlPacket->parameters[i].lenght;
		memcpy (&frame[index], controlPacket->parameters[i].value, controlPacket->parameters[i].lenght);
 		index += controlPacket->parameters[i].lenght;
	}

	if (llwrite(linkLayer, frame, packetSize) < 0)
	{
		perror("llwrite");
		exit (-1);
	}		

	return 0;
}

int sendData(LinkLayer * linkLayer, char * buffer, int size, int sequenceNumber)
{
	unsigned char L1, L2, packetSize;

	//usando teorema do resto
	L1 = size / 256;
	L2 = size % 256;

	packetSize = 4 + size;
	unsigned char frame[packetSize];

	frame[0] = 1;
	frame[1] = sequenceNumber;
	frame[2] = L2;
	frame[3] = L1;

	memcpy (&frame[4], buffer, size);

	if (llwrite(linkLayer, frame, packetSize) < 0)
	{
		perror("llwrite");
		exit (-1);
	}	
}


void receive (LinkLayer * linkLayer)
{
	//Start control packet
	
	char* buffer = malloc(MAX_SIZE);
	int size = llread(linkLayer, buffer);

	int index = 0;

	TLV startParameters[2];
	
	int j = 1;
	for(j; j < 2; j++)
	{
		startParameters[j].type = frame[index++];
		startParameters[j].lenght = frame[index++];	

		memcpy(startParameters[j].value, &linkLayer->frame[index], startParameters[j].length);	

		if (startParameters.type == 0) //size
			fileSize = startParameters[j].value;

	}	
}
