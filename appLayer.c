#include "appLayer.h"
//#include "utilities.c"

void startAppLayer(LinkLayer *linkLayer, ApplicationLayer *appLayer)
{
	if (openPort(linkLayer) < 0)
		exit(-1);	

	else if (setTermiosStructure(linkLayer) < 0)
		exit(-1);	

	switch (appLayer->status)
	{
	case TRANSMITTER:
		transmitter(linkLayer);
		break;
	case RECEIVER:
		receiver(linkLayer);
		break;
	}
}

void transmitter (LinkLayer * linkLayer)
{
		if (llopenT(linkLayer) < 0)
			exit(-1);

		send (linkLayer);

		if (llcloseT(linkLayer) < 0)
			exit(-1);
}

void receiver (LinkLayer * linkLayer)
{
		if (llopenR(linkLayer) < 0)
			exit(-1);

		receive (linkLayer);

		if (llcloseR(linkLayer) < 0)
			exit(-1);
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

	FILE *file = openFile(1, linkLayer->fileName);
	
	
	//DataPacket dataPacket;
	//dataPacket.data = malloc(fileSize);
	char* fileData = (char*) malloc(fileSize);
	int nBytesRead = 0, sequenceNumber = 0;

	while ((nBytesRead = fread(fileData, sizeof(unsigned char),fileSize , file)) > 0) //TODO verificar tamanho de cada fread (fileSize ou 255)
	{
		sendData(linkLayer, fileData, fileSize, sequenceNumber++ % 255);

		memset(fileData, 0, 255);
	}
	
	free(fileData);
	closeFile(file);
	
	//End control packet
	ControlPacket endCP;
	endCP = startCP;
	endCP.controlField = 3;

	tcflush(linkLayer->fd, TCIOFLUSH);

	sendControl(linkLayer, &endCP, 2);

	clock_t endTime = clock();

	linkLayer->totalTime = (double)(endTime - startTime) / CLOCKS_PER_SEC;
}

int sendControl(LinkLayer *linkLayer, ControlPacket *controlPacket, int nParameters)
{
	unsigned int i, packetSize = 1, index = 1;

	for (i = 0; i < nParameters; i++)
		packetSize += 2 + controlPacket->parameters[i].lenght;

	char frame[packetSize];
	frame[0] = controlPacket->controlField; //conversão de inteiro para char (pode estar errado)

	for (i = 0; i < nParameters; i++)
	{
		frame[index++] = controlPacket->parameters[i].type;
		frame[index++] = controlPacket->parameters[i].lenght;
		memcpy(&frame[index], controlPacket->parameters[i].value, controlPacket->parameters[i].lenght);
		index += controlPacket->parameters[i].lenght;
	}

	if (llwrite(linkLayer, frame, packetSize) < 0)
		exit(-1);
	
	linkLayer->sequenceNumber = !linkLayer->sequenceNumber;

	return 0;
}

int sendData(LinkLayer *linkLayer, char *buffer, int size, int sequenceNumber)
{
	unsigned char L1, L2;
	unsigned int packetSize;

	//usando teorema do resto
	L1 = size % 256;
	L2 = size / 256;

	packetSize = 4 + size;
	char * frame = malloc (packetSize);

	frame[0] = 1;
	frame[1] = sequenceNumber;
	frame[2] = L2;
	frame[3] = L1;

	printf("%d\n", frame[3]);
	printf("256 * %d + %d = %d\n", L2, L1, 256 * L2 + L1);

	memcpy(&frame[4], buffer, size);

	if (llwrite(linkLayer, frame, packetSize) < 0)
		exit(-1);
	

	linkLayer->sequenceNumber = !linkLayer->sequenceNumber;

	return 0;
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

	while (index < size)
	{
		unsigned int type = linkLayer->frame[index++]; //0 = size, 1 = name
		unsigned char lenght = linkLayer->frame[index++]; //size of file
		char *value = malloc(lenght); // either size or name, according to type

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

		index += lenght;
	}

	printf("Received start control packet.\n");
	sendMessage(linkLayer->fd, RR0); //TODO esta mensagem varia
	linkLayer->nRR++;	
	linkLayer->sequenceNumber = !linkLayer->sequenceNumber;

	//DAQUI PARA BAIXO LÊ OS DADOS ATÉ RECEBER CONTROL PACKET A INDICAR FIM
	
	printf("File size: %d File Name: %s\n",fileSize,  fileName);
	FILE *file = openFile(0, "e.gif"); //Mudar isto e colocar fileNAme

	while (1)
	{
		char * data;
		int C_data, C_packet;//, N;
		unsigned int L1, L2, lenght;

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
		{
			printf("Received end control packet.\n");

			linkLayer->nRR++;

			sendMessage(linkLayer->fd, RR0); //Só para testes
			break;
		}

		else if (C_data != 1) 
		{
			printf ("receive: packet received not expected\n");
			exit(-1);
		}

		//N = linkLayer->frame[5];
		L2 = linkLayer->frame[6];
		L1 = 216;
		//L1 = linkLayer->frame[7];

		lenght = 256 * L2 + L1;

		C_packet = linkLayer->frame[2]; //que contém sequence number
		//printf("C_packet = %d  Sequence number = %d\n", C_packet >> 6, linkLayer->sequenceNumber);
		//printf("C_packet = %d\n", C_packet);
		if (linkLayer->sequenceNumber != C_packet >> 6 || 
			!isValidBcc2(linkLayer->frame, size, linkLayer->frame[size - 2])) // se o sequence number não for o esperado TODO pôr validação do BCC2
		{
			linkLayer->nREJ++;
			if(linkLayer->sequenceNumber)
				sendMessage(linkLayer->fd, REJ1);
			else
				sendMessage(linkLayer->fd, REJ0);
			continue;
		}
		
		data = malloc(lenght);

		memcpy (data, &linkLayer->frame[8], lenght);

		fwrite (data, sizeof(char), lenght, file);
		free(data);

		linkLayer->nRR++;
		if(linkLayer->sequenceNumber)
		{
			sendMessage(linkLayer->fd, RR1);
			printf("RR\n");
		}
		else
		{
			sendMessage(linkLayer->fd, RR0);
			printf("RR\n");
		}

		linkLayer->sequenceNumber = !linkLayer->sequenceNumber;
	}

	closeFile(file);
	
	clock_t endTime = clock();

	linkLayer->totalTime = (double)(endTime - startTime) / CLOCKS_PER_SEC;
}
