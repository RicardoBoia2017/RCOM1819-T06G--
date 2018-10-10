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
#define A 0x03
#define C_SET 0x03
#define C_DISC 0x0B
#define C_UA 0x07
#define C_RR 0x05
#define C_REJ 0x01
#define BYTE_TO_SEND 5

unsigned char SETUP[5];

volatile int STOP=FALSE;

void prepareMessage(){ 
  SETUP[0]=FLAG;
	SETUP[1]=A;
	SETUP[2]=C_SET;
	SETUP[3]=SETUP[1]^SETUP[2];
	SETUP[4]=FLAG;

}


void sendMessage(int fd){
  int byteChar=0;

  while (byteChar!=BYTE_TO_SEND) {
    byteChar=write(fd,SETUP,BYTE_TO_SEND);
    if(byteChar==-1){
      perror("write");
      exit(-1);
    }
    printf("%d bytes\n",byteChar);
  }

}

int stateValidMessage(int fd,char res[], unsigned char end){
  int state=0,aux;
  char readed;

  while(1){
    aux=read(fd,&readed,1);
    if(aux==-1){
      perror("state_read");
      exit(-1);

    }



    printf("readed=%x state=%d\n",readed,state );

    switch (state) {
      case 0:
              if(readed==FLAG){
                state=1;
              }else state=0;
      break;
      case 1:
              res[0]=readed;
              if(readed==A){
                state=2;
              }else state=0;
      break;
      case 2:
              res[1]=readed;
              if(readed==end){
                state=3;
              }else state=0;
      break;
      case 3:
              if((res[0]^res[1])==readed){
                state=4;
              }else state=0;
      break;
      case 4:
              if(readed==FLAG){
                state=5;
                return 0;
              }else state=0;
      break;


    }


  }

  return 0;

}




/*int verifier(int step,unsigned char mess,int id){
	switch(step)
	{
		case 0:
			if(mess==FLAG)
				step++;
		case 1:
			if(mess==AD)
				step++;
			else if(mess!=FLAG)
						step=0;
		case 2:
			if(id==emissor && mess==UA)
				step++;
			else if(mess==AD && id==recetor)
					step++;
			else if(mess==FLAG)
					step=1;
			else
				step=0;
		case 3:
			if(id==emissor && mess==(AD^UA))
				step++;
			else if(mess==(AD^AD) && id==recetor)
					step++;
			else if(mess==FLAG)
					step=1;
			else
				step=0;
		case 4:
			if(mess==FLAG)
				step++;
			else
				step=0;*/
