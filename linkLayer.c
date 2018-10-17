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

    while(tries<3 && timOut==TRUE){
	
        timOut=FALSE;
	alarm(3);
    
	sendMessage(linkLayer->fd, SETUP);
	receiveResponse(linkLayer->fd);
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

    stateValidMessage(linkLayer->fd,result_A_C, SETUP);

    sendMessage (linkLayer->fd, UA);
 /*	write(linkLayer->fd,UA,BYTE_TO_SEND);

    if(write(linkLayer->fd,UA,BYTE_TO_SEND)==-1){
      perror("write_E");
    }else
	 printf("Response read");*/
}

void llcloseT (LinkLayer * linkLayer)
{
    tries = 0;
    timOut = TRUE;

    while(tries<3 && timOut==TRUE){
	
        timOut=FALSE;
	alarm(3);
    
	sendMessage(linkLayer->fd, DISC);

	receiveResponse(linkLayer->fd);
    }
		
    if(tries==3){
    	printf("%s\n","Failed to send the message (3 attemps)" );
    	exit(-1);
    }
    else{ 
	printf("%s\n","UA was received"  );
   }	
}

void llcloseR (LinkLayer * linkLayer)
{
	char result_A_C[2];

	stateValidMessage(linkLayer->fd,result_A_C, DISC);

	sendMessage (linkLayer->fd, UA);
}
