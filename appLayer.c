#include "appLayer.h"
//#include "utilities.c"

/*void setupAppLayer (ApplicationLayer * appLayer) //TODO deve ser para acrescentar mais
{
	appLayer->fd = -1;
}*/

void startAppLayer(LinkLayer *linkLayer, ApplicationLayer *appLayer)
{
	openPort(linkLayer); //abre serial port /dev/ttyS0 para leitura e escrita

	setTermiosStructure(linkLayer);

	switch (appLayer->status)
	{
	case TRANSMITTER:
		llopenT(linkLayer);
		//		send (linkLayer);
		llcloseT(linkLayer);
		break;
	case RECEIVER:
		llopenR(linkLayer);
		//		receive (linkLayer);
		llcloseR(linkLayer);
		break;
	}
}

void send(LinkLayer *linkLayer)
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

	//control packet size
	TLV startTLVSize;

	unsigned int fileSize = getFileSize(linkLayer->fileName);

	sprintf(sizeString, "%d", fileSize);

	startTLVSize.type = 0;
	startTLVSize.lenght = strlen(sizeString);
	startTLVSize.value = sizeString;

	TLV listParameters[2] = {startTLVSize, startTLVName};
	startCP.parameters = listParameters;

	clock_t startTime = clock();

	sendControl(linkLayer, &startCP, 2);

	//Data packet

	FILE *file = fopen(linkLayer->fileName, "rb");
	; //Mudar isto e colocar função das utilities

	DataPacket dataPacket;
	dataPacket.data = malloc(fileSize);
	int nBytesRead = 0;
	int sequenceNumber = 0;
	while ((nBytesRead = fread(dataPacket.data, sizeof(char), MAX_SIZE, file)) > 0)
	{
		//		printf("%d\n", nBytesRead);
		sendData(linkLayer, dataPacket.data, nBytesRead, sequenceNumber++ % 255);

		memset(dataPacket.data, 0, 255);
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

	sendControl(linkLayer, &endCP, 2);

	clock_t endTime = clock();

	linkLayer->totalTime = (double)(endTime - startTime) / CLOCKS_PER_SEC;
}

unsigned int getFileSize(char *fileName)
{
	struct stat st;
	if (stat(fileName, &st) < 0)
	{
		perror("getFileSize");
		exit(-1);
	}
	return st.st_size;
}

int sendControl(LinkLayer *linkLayer, ControlPacket *controlPacket, int nParameters)
{
	unsigned int i = 0, packetSize = 1, index = 1;

	for (i; i < nParameters; i++)
		packetSize += 2 + controlPacket->parameters[i].lenght;

	unsigned char frame[packetSize];
	frame[0] = controlPacket->controlField; //conversão de inteiro para char (pode estar errado)

	for (i = 0; i < nParameters; i++)
	{
		frame[index++] = controlPacket->parameters[i].type;
		frame[index++] = controlPacket->parameters[i].lenght;
		memcpy(&frame[index], controlPacket->parameters[i].value, controlPacket->parameters[i].lenght);
		index += controlPacket->parameters[i].lenght;
	}

	if (llwrite(linkLayer, frame, packetSize) < 0)
	{
		perror("llwrite");
		exit(-1);
	}

	return 0;
}

int sendData(LinkLayer *linkLayer, char *buffer, int size, int sequenceNumber)
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

	memcpy(&frame[4], buffer, size);

	if (llwrite(linkLayer, frame, packetSize) < 0)
	{
		perror("llwrite");
		exit(-1);
	}
}

void receive(LinkLayer *linkLayer)
{
	int size;
	unsigned int fileSize, index = 0;
	char *fileName;
	clock_t startTime = clock();
	//Start control packet
	//	char* buffer = malloc(MAX_SIZE);
	size = llread(linkLayer);

	// receives start control packet
	while (index < size)
	{
		unsigned int type = linkLayer->frame[index++]; //0 = size, 1 = name
		unsigned char lenght = linkLayer->frame[index++]; //size of file
		char *value; // either size or name, according to type
		memcpy(value, &linkLayer->frame[index], lenght);

		if (type == 0) //stores size of file in fileSize
		{
			fileSize = atoi(value);
		}

		else if (type == 1) //stores name of file in fileNAme
		{
			fileName = malloc(lenght);
			memcpy(fileName, value, lenght);
		}
	}

	FILE *file = fopen(fileName, "wb"); //Mudar isto e colocar função das utilities

	while (1)
	{
		char * data;
		int C_packet, C_data, N, L1, L2, lenght;

		size = llread (linkLayer);

		if (size < 0)
		{
			printf ("llread error\n");
			exit(-1);
		}
		// frame[0] = FLAG
		// frame[1] = A
		// frame[2] = C da trama, não dos dados 
		// frame[3] = BBC1
		// A partir do 4 começa dos dados
		C_data = linkLayer->frame[4];


		if (C_data == 3) //receives end control packet
			break;

		else if (C_data != 1) 
		{
			printf ("receive: packet received not expected\n");
			exit(-1);
		}

		N = linkLayer->frame[5];
		L2 = linkLayer->frame[6];
		L1 = linkLayer->frame[7];

		lenght = 256 * L2 + L1;

		C_packet = linkLayer->frame[2]; 

		//TODO verificar sequence e BCC2  


		memcpy (data, &linkLayer->frame[8], lenght);

	}
}

int receivePacket (LinkLayer *linkLayer, int * lenght, char ** data)
{
	





}