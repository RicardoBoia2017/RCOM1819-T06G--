#include "utilities.h"

volatile int STOP = FALSE;
int CANCEL = FALSE;

int timeOut = TRUE;

void alrmHanler(int sig)
{
	timeOut = TRUE;
}

void setTimeOut(int value)
{
	timeOut = value;
}

int getTimeOut() { return timeOut; }

FILE *openFile(int type, char *filePath)
{

	FILE *result;

	if (type == 0)
	{
		result = fopen(filePath, "rb");
	}
	else
	{
		result = fopen(filePath, "wb");
	}

	if (result == NULL)
	{
		perror("error to open the file ");
	}

	return result;
}

void sendMessage(int fd, const unsigned char cmd[])
{
	int byteChar = 0;

	while (byteChar != BYTE_TO_SEND)
	{

		byteChar = write(fd, cmd, BYTE_TO_SEND);
		printf("%d\n", byteChar);
		if (byteChar == -1)
		{
			perror("write");
			exit(-1);
		}
		printf("%d bytes\n", byteChar);
	}

	/*if(CANCEL==FALSE){
	 alarm(3);
	 printf("alarm ");
	 }*/
}

int stateValidMessage(int fd, char res[], const unsigned char cmd[])
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

		printf("reader=%x state=%d\n", reader, state);

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
			res[0] = reader;
			if (reader == cmd[1])
				state = 2;

			else if (reader != cmd[0])
				state = 0;
			break;
		case 2:
			res[1] = reader;

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

	int state = 0, bytesRead, index = 0;
	unsigned char reader;
	while (state != FINALSTATE && timeOut == FALSE)
	{
		bytesRead = read(fd, &reader, 1);

		if (bytesRead == -1)
		{
			perror("validateFrame");
			exit(-1);
		}
		printf("reader=%x state=%d\n", reader, state);

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

		case 4:						   //BCC
			frame[4 + index] = reader; //Vai colocando dados até encontrar flag
			index++;
			if (reader == FLAG)
				state = 5;

			break;
		}
	}

	return 5 + index; //2 * F + A + C + BCC1 + index (Dados + BCC2)
}

unsigned char *stuffing(char *frame, int size)
{
	unsigned char *result = malloc(MAX_SIZE);
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

	return result;
}

unsigned char *destuffing(char *frame, int size)
{
	int i, j = 0;
	unsigned char *result = malloc(MAX_SIZE);

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

	return result;
}
