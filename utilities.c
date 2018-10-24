#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define FLAG 0x7E
#define FLAG_S_1 0x7D
#define FLAG_S_2 0x5E
#define FLAG_S_3 0x5D

#define C_REJ0 0X01
#define C_REJ1 0X81

#define C_RR0 0X05
#define C_RR1 0X85

#define A 0x03
#define C_SET 0x03
#define C_DISC 0x0B
#define C_UA 0x07
#define BYTE_TO_SEND 5
#define FINALSTATE 5



static const unsigned char SETUP[5] = { FLAG, A, C_SET, A ^ C_SET, FLAG };
static const unsigned char UA[5] = { FLAG, A, C_UA, A ^ C_UA, FLAG };
static const unsigned char DISC[5] = { FLAG, A, C_DISC, A ^ C_DISC, FLAG };

int tries = 0;

volatile int STOP = FALSE;
int CANCEL = FALSE;
int timOut = TRUE;

void alrmHanler(int sig) {
	timOut = TRUE;
	tries++;
}


FILE* openFile(int type ,char* filePath){

    FILE* result;

    if(type==RECEIVER){
     result=fopen(filePath,"rb");
    }else{
      result=fopen(filePath,"wb");
    }

    if(result==NULL){
       perror("error to open the file ");

     }
   return result;
}






void sendMessage(int fd, const unsigned char cmd[]) {
	int byteChar = 0;


	while (byteChar != BYTE_TO_SEND) {

		byteChar = write(fd, cmd, BYTE_TO_SEND);
		printf("%d\n", byteChar);
		if (byteChar == -1) {
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

int stateValidMessage(int fd, char res[], const unsigned char cmd[]) {

	int state = 0, aux;
	unsigned char reader;

	while (state != FINALSTATE && timOut == FALSE) {

		aux = read(fd, &reader, 1);

		if (aux == -1) {
			perror("stateValidMessage");
			exit(-1);
		}

		printf("reader=%x state=%d\n", reader, state);

		switch (state) {
		case 0:
			if (reader == cmd[0]) {
				state = 1;
			} else
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

			printf("Command 2: %0x\n", cmd[2]);

			if (reader == cmd[2])
				state = 3;

			else if (reader != cmd[0])
				state = 0;
			break;
		case 3:


			if (cmd[3] == reader)

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

int validateFrame(LinkLayer * linkLayer) {

	int state = 0, bytesRead, index=0;
	unsigned char reader;

	while (state != FINALSTATE && timOut == FALSE) {

		bytesRead = read(linkLayer->fd, &reader, 1);

		if (bytesRead == -1) {
			perror("validateFrame");
			exit(-1);
		}

		switch (state) {

		case 0: //start
			if (reader == FLAG)
			{
				linkLayer->frame[0] = reader;
				state = 1;
			}
			break;

		case 1: //flag
			if (reader == A)
			{
				linkLayer->frame[1] = reader;
				state = 2;
			}

			else if (reader != FLAG)
				state = 0;
			break;

		case 2: //A

			if (reader == 0 || reader == (1 << 6) )
			{
				linkLayer->frame[2] == reader;
				state = 3;
			}

			else if (reader == FLAG)
				state = 1;

			else
				state = 0;
			break;

		case 3: //C
			if ((linkLayer->frame[1] ^ linkLayer->frame[2]) == reader)
			{
				linkLayer->frame[3] == reader;
				state = 4;
			}

			else if (reader == FLAG)
				state = 1;

			else
				state = 0;
			break;

		case 4: //BCC
			linkLayer->frame[4 + index] == reader; //Vai colocando dados até encontrar flag
			index++;
			if (reader == FLAG)
				state = 5;

			break;

		}
	}

	return 0;
}

void bStuffing(char* messageData, char byte, int* countB, int size) {

	if (byte == FLAG) {
		messageData[4 + size + (*countB)] = FLAG_S_1;
		messageData[4 + size + (++(*countB))] = FLAG_S_2;
	} else if (byte == FLAG_S_1) {
		messageData[4 + size + (*countB)] = FLAG_S_1;
		messageData[4 + size + (++(*countB))] = FLAG_S_3;
	} else {
		messageData[4 + size + (*countB)] = byte;
	}
}
