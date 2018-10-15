#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include<signal.h>


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
#define FINALSTATE 5

unsigned char SETUP[5];
unsigned char UA[5];
int tries=0;

volatile int STOP=FALSE;
int CANCEL=FALSE;


void prepare_SET_UA(){
  SETUP[0]=FLAG;
	SETUP[1]=A;
	SETUP[2]=C_SET;
	SETUP[3]=SETUP[1]^SETUP[2];
	SETUP[4]=FLAG;

  UA[0]=FLAG;
  UA[1]=A;
  UA[2]=C_UA;
  UA[3]=UA[1]^UA[2];
  UA[4]=FLAG;

}



void sendMessage(int fd){
  int byteChar=0;
  if(tries==3){
    printf("%s\n","Fail to send the message (3 attemps)" );
    exit(-1);
  }

  while (byteChar!=BYTE_TO_SEND) {
    byteChar=write(fd,SETUP,BYTE_TO_SEND);
    if(byteChar==-1){
      perror("write");
      exit(-1);
    }
    printf("%d bytes\n",byteChar);
  }
  tries++;

  if(CANCEL==FALSE){
    alarm(3);
  }



}

int stateValidMessage(int fd,char res[]){

  int state=0,aux;
  unsigned char reader;

  while(state!=FINALSTATE){

    aux=read(fd,&reader,1);

    if(aux==-1){
      perror("state_read");
      exit(-1);

    }

    printf("reader=%x state=%d\n",reader,state );

    switch (state) {
      case 0:
              if(reader==SETUP[0]){
                state=1;
              }else state=0;
      break;
      case 1:
              res[0]=reader;
              if(reader==SETUP[1]){
                state=2;
              }else state=0;
      break;
      case 2:
              res[1]=reader;
              if(reader==SETUP[2]){
                state=3;
              }else state=0;
      break;
      case 3:
              if((SETUP[3])==reader){
                state=4;
              }else state=0;
      break;
      case 4:
              if(reader==SETUP[4]){
                state=5;
              }else state=0;
      break;

    }

  }
  return 0;

}


void receiveResponse(int fd){
  int state=0,aux;


  unsigned char reader;

  while(state!=FINALSTATE){

    aux=read(fd,&reader,1);
    if(aux==-1){
      perror("state_read");
      exit(-1);

    }

    printf("reader=%x state=%d\n",reader,state );

    switch (state) {
      case 0:
      if(reader==UA[0]){
        state=1;
      }else state=0;
      break;
      case 1:
      if(reader==UA[1]){
        state=2;
      }else if(reader!=UA[0]){
        state=0;}
      break;

      case 2:
      if(reader==UA[2]){
        state=3;
      }else if(reader==UA[0]){
        state=1;
      }else state=0;
      break;

      case 3:
      if(reader==UA[3]){
        state=4;
      }else state=0;
      break;

      case 4:
      if(reader==UA[4]){
        state=5;
      }else state=0;
      break;
    }

  }
  printf("%s\n","UA was received"  );
  CANCEL=TRUE;

}
