#include "utilities.h"

volatile int STOP = FALSE;
int CANCEL = FALSE;

int timeOut = TRUE;
int tries = 0;

void alrmHanler(int sig)
{
	timeOut = TRUE;
	tries++;
}

void resetTries() {tries = 0;}

int outOfTries (int maxTries) { return tries >= maxTries; }

void setTimeOut(int value)
{
	timeOut = value;
}

int getTimeOut() { return timeOut; }

FILE *openFile(int type, char *filePath)
{
	FILE *result;

	if (type == 0)
		result = fopen(filePath, "wb");

	else
		result = fopen(filePath, "rb");

	if (result == NULL)
	{
		perror("error to open the file ");
		exit (-1);
	}
	return result;
}

void closeFile(FILE *file)
{
	if (fclose (file) != 0)
	{
		perror("closeFile");
		exit(-1);
	}
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

void sendMessage(int fd, const unsigned char cmd[])
{
	int byteChar = 0;

	while (byteChar != BYTE_TO_SEND)
	{

		byteChar = write(fd, cmd, BYTE_TO_SEND);

		if (byteChar == -1)
		{
			perror("sendMessage");
			exit(-1);
		}

	}

}

int stateValidMessage(int fd, const unsigned char cmd[])
{
	int state = 0, aux;
	unsigned char reader;

	while (state != FINALSTATE && timeOut == FALSE)
	{

		aux = read(fd, &reader, 1);

		if (aux == -1)
		{
			perror("stateValidMessage");
			exit(-1);
		}

		//printf("reader=%x state=%d\n", reader, state);

		switch (state)
		{
		case 0:
			if (reader == cmd[0])
			{
				state = 1;
			}
			else
				state = 0;
			break;
		case 1:
			if (reader == cmd[1])
				state = 2;

			else if (reader != cmd[0])
				state = 0;
			break;
		case 2:
			if (reader == cmd[2])
				state = 3;

			else if (reader != cmd[0])
				state = 0;
			break;
		case 3:

			if ((cmd[3]) == reader)
				state = 4;
			else
				state = 0;
			break;
		case 4:
			if (reader == cmd[4])
				state = 5;

			else
				state = 0;
			break;
		}
	}
	return 0;
}

int validateFrame(int fd, char *frame)
{

	int state = 0, bytesRead, dataSize = 0;
	unsigned char reader;
	while (state != FINALSTATE && timeOut == FALSE)
	{
		bytesRead = read(fd, &reader, 1);

		if (bytesRead == -1)
		{
			perror("validateFrame");
			exit(-1);
		}
		//printf("reader=%x state=%d\n", reader, state);

		switch (state)
		{

		case 0: //start
			if (reader == FLAG)
			{
				frame[0] = reader;
				state = 1;
			}
			break;

		case 1: //flag
			if (reader == A)
			{
				frame[1] = reader;
				state = 2;
			}

			else if (reader != FLAG)
				state = 0;
			break;

		case 2: //A

			if (reader == 0 || reader == (1 << 6))
			{
				frame[2] = reader;
				state = 3;
			}

			else if (reader == FLAG)
				state = 1;

			else
				state = 0;
			break;

		case 3: //C
			if ((frame[1] ^ frame[2]) == reader)
			{
				frame[3] = reader;
				state = 4;
			}

			else if (reader == FLAG)
				state = 1;

			else
				state = 0;
			break;

		case 4:	 //BCC
			frame[4 + dataSize] = reader; //Vai colocando dados até encontrar flag
			dataSize++;
			if (reader == FLAG)
				state = 5;

			break;
		}
	}

	return 4 + dataSize; // F + A + C + BCC1 + dataSize (Dados + BCC2 +)
}

int stuffing(char *frame, int size)
{
	char *result = malloc(20000);
	int resultSize = size;
	int i;

	for (i = 1; i < (size - 1); i++)
	{
		if (frame[i] == FLAG || frame[i] == ESC)
		{
			resultSize++;
		}
	}
	result[0] = frame[0];
	int j = 1;

	for (i = 1; i < (size - 1); i++)
	{
		if (frame[i] == FLAG || frame[i] == ESC)
		{
			result[j] = ESC;
			result[++j] = frame[i] ^ 0X20;
		}
		else
		{
			result[j] = frame[i];
		}
		j++;
	}

	result[j] = frame[i];
	frame = realloc(frame, size + resultSize - size);
	memcpy(frame, result, resultSize);
	return resultSize;
}

int destuffing(char *frame, int size)
{
	int i, j = 0;
	char *result = malloc(20000);
	int resultSize = size;

	for (i = 1; i < (size - 1); i++)
	{
		if (frame[i] == ESC)
		{
			resultSize--;
		}
	}

	for (i = 0; i < size; i++)
	{
		if (frame[i] == ESC)
		{
			result[j] = frame[++i] ^ 0X20;
		}
		else
		{
			result[j] = frame[i];
		}
		j++;
	}
	memcpy(frame, result, resultSize);

	return resultSize;
}

int isValidBcc2(char * packet,int packetSize,unsigned char received){
    unsigned char expected = 0;

    unsigned int i = 4;
    for(;i<packetSize - 2;i++){
        expected ^= packet[i];
    }

    return(expected==received);
}
