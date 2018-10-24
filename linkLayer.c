#include "linkLayer.h"
#include <stdio.h>
#include "utilities.c"
void setupLinkLayer (LinkLayer * linkLayer)
{
	linkLayer->fd = -1;
	linkLayer->port = "/dev/ttyS0";
	linkLayer->baudRate = 
	linkLayer->sequenceNumber = 0;
	linkLayer->timeout = 3;
	linkLayer->numTransmissions = 3;
	linkLayer->frame = malloc(MAX_SIZE);

	linkLayer->nRR = 0;
	linkLayer->nREJ = 0;
}	

void openPort (LinkLayer * linkLayer)
{
	linkLayer->fd = open (linkLayer->port, O_RDWR | O_NOCTTY); //port = /dev/ttyS0

	if (linkLayer->fd == -1)
		perror ("openPort");

}

void setTermiosStructure (LinkLayer * linkLayer)
{
    if ( tcgetattr(linkLayer->fd,&oldtio) == -1) { /* save current port settings */
      perror("setTermiosStructure");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 1;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 0;   /* blocking read until 5 chars received */

    tcflush(linkLayer->fd, TCIOFLUSH);

    if ( tcsetattr(linkLayer->fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

}

void llopenT (LinkLayer * linkLayer)
{
    (void) signal(SIGALRM,alrmHanler);

    char result_A_C[2]; //Tirar isto depois

    while(tries<3 && timOut==TRUE){
	
        timOut=FALSE;
	alarm(3);
    
	sendMessage(linkLayer->fd, SETUP);
//	receiveResponse(linkLayer->fd);
	stateValidMessage(linkLayer->fd, result_A_C, UA);

  	alarm(0);
    }
		
    if(tries==3){
    	printf("%s\n","Failed to send the message (3 attemps)" );
    	exit(-1);
    }
    else{ 
	printf("%s\n","UA was received"  );
   }
}

void llopenR (LinkLayer * linkLayer)
{
    char result_A_C[2];
    timOut=FALSE;

    stateValidMessage(linkLayer->fd,result_A_C, SETUP);

    sendMessage (linkLayer->fd, UA);
 /*	write(linkLayer->fd,UA,BYTE_TO_SEND);

    if(write(linkLayer->fd,UA,BYTE_TO_SEND)==-1){
      perror("write_E");
    }else
	 printf("Response read");*/
}

int llwrite (LinkLayer *linkLayer, char * buffer, int lenght)
{
   unsigned char * packet = malloc(5 + lenght);

   packet[0] = FLAG;
   packet[1] = A;
   packet[2] = linkLayer->sequenceNumber << 6;
   packet[3] = packet[1] ^ packet[2];
 
   memcpy(&packet[4], buffer, lenght);

   unsigned char BCC2 = 0;
   int i = 0;

   for(	; i < lenght; i++)
	BCC2 ^= buffer[i];

   packet[lenght+4] = BCC2;	  	
   packet[lenght+5] = FLAG;

   //byteStuffing

   while(tries<3 && timOut==TRUE){
	
        timOut=FALSE;
	alarm(3);
    
	if (write (linkLayer->fd, packet, lenght + 6) < 0)
	{
   	   perror("write");
   	   exit(-1);
	}	

//	receiveResponse(linkLayer->fd);
//	stateValidMessage(linkLayer->fd, result_A_C, UA);

	char response [5];
	read (linkLayer->fd, response, 5);
	
	if (linkLayer->sequenceNumber == 1)
	{
		if (response[2] == C_RR1)
			linkLayer->nRR++;

		else if(response[2] == C_REJ1)
			linkLayer->nREJ++;
	}

	else if (linkLayer->sequenceNumber == 0)
	{
		if (response[2] == C_RR0)
			linkLayer->nRR++;

		else if(response[2] == C_REJ0)
			linkLayer->nREJ++;
	}

  	alarm(0);
	
    }
		
    if(tries==3){
    	printf("%s\n","Failed to send the message (3 attemps)" );
    	exit(-1);
    }
    else{ 
	printf("%s\n","UA was received"  );
   }	
   return 0;
}

int llread (LinkLayer * linkLayer, char * buffer)
{

	validateFrame (linkLayer);

	//byteDestuffing


}
	
void llcloseT (LinkLayer * linkLayer)
{
    tries = 0;
    timOut = TRUE;
    char result_A_C[2];

    while(tries<3 && timOut==TRUE){
	
        timOut=FALSE;
	alarm(3);
    
	sendMessage(linkLayer->fd, DISC);

//	receiveResponse(linkLayer->fd);
<<<<<<< HEAD
	stateValidMessage(linkLayer->fd, result_A_C, DISC);
=======
	stateValidMessage(linkLayer->fd, result_A_C, UA);
>>>>>>> 159aa070f7b17a7db5aeddb8cbd78b9f2a6c743f

  	alarm(0);
    }
		
    if(tries==3){
    	printf("%s\n","Failed to send the message (3 attemps)" );
    	exit(-1);
    }
    else{ 
	printf("%s\n","UA was received"  );
   }	

<<<<<<< HEAD
   sendMessage(linkLayer->fd, UA);
=======
   sendMessage(linkLayer->fd, DISC);
>>>>>>> 159aa070f7b17a7db5aeddb8cbd78b9f2a6c743f
}

void llcloseR (LinkLayer * linkLayer)
{
    tries = 0;
    timOut = TRUE;
    char result_A_C[2];

    (void) signal(SIGALRM,alrmHanler);

    stateValidMessage(linkLayer->fd, result_A_C, DISC);

    while(tries<3 && timOut==TRUE){
	
        timOut=FALSE;
	alarm(3);
    
	sendMessage(linkLayer->fd, DISC);

//	receiveResponse(linkLayer->fd);
	stateValidMessage(linkLayer->fd, result_A_C, UA);

  	alarm(0);
    }
		
    if(tries==3){
    	printf("%s\n","Failed to send the message (3 attemps)" );
    	exit(-1);
    }
    else{ 
	printf("%s\n","UA was received"  );
   }
}
