#include "linkLayer.h"
#include <stdio.h>
//#include "utilities.c"

//int tries1 = 0;
///int timOut1 = TRUE;

void setupLinkLayer(LinkLayer *linkLayer)
{
    linkLayer->fd = -1;
    linkLayer->port = "/dev/ttyS0";
    linkLayer->baudRate =
    linkLayer->sequenceNumber = 0;
    linkLayer->timeout = 3;
    linkLayer->numTransmissions = 3;
    linkLayer->frame = malloc(MAX_SIZE);

    linkLayer->fileName = "pinguim.gif"; //Considerar passar o nome do ficheiro por terminal

    linkLayer->nRR = 0;
    linkLayer->nREJ = 0;
    linkLayer->totalTime = 0;
}

int openPort(LinkLayer *linkLayer)
{
    linkLayer->fd = open(linkLayer->port, O_RDWR | O_NOCTTY); //port = /dev/ttyS0

    if (linkLayer->fd == -1)
	{
        perror("openPort");
		return -1;
	}

	return 0;
}

int setTermiosStructure(LinkLayer *linkLayer)
{
    if (tcgetattr(linkLayer->fd, &oldtio) == -1)
    { /* save current port settings */
        perror("setTermiosStructure");
		return -1;
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    newtio.c_lflag = 0;

    newtio.c_cc[VTIME] = 1; /* inter-character timer unused */
    newtio.c_cc[VMIN] = 0;  /* blocking read until 5 chars received */

    tcflush(linkLayer->fd, TCIOFLUSH);

    if (tcsetattr(linkLayer->fd, TCSANOW, &newtio) == -1)
    {
        perror("tcsetattr");
		return -1;
    }
	
	return 0;
}

int llopenT(LinkLayer *linkLayer)
{

    (void)signal(SIGALRM, alrmHanler);

    while (!outOfTries(linkLayer->numTransmissions)  && getTimeOut() == TRUE)
    {
        setTimeOut(FALSE);

        alarm(linkLayer->timeout);

        sendMessage(linkLayer->fd, SETUP);
        stateValidMessage(linkLayer->fd, UA);

        alarm(0);
    }

    if (outOfTries(linkLayer->numTransmissions) )
    {
        printf("Failed to send the message (%d attemps)\n", linkLayer->numTransmissions);
        return -1;
    }
    else
    {
        printf("%s\n", "UA was received");
    }

	return 0;
}

int llopenR(LinkLayer *linkLayer)
{
    setTimeOut(FALSE);

    stateValidMessage(linkLayer->fd, SETUP);
    printf("SETUP receives\n");
    sendMessage(linkLayer->fd, UA);
    printf("UA sent\n");

	return 0;
}

int llwrite(LinkLayer *linkLayer, char *buffer, int lenght)
{

    char *packet = malloc(12000);
    int tries = 0;
    setTimeOut(TRUE);

    packet[0] = FLAG;
    packet[1] = A;
    packet[2] = linkLayer->sequenceNumber << 6;
    packet[3] = packet[1] ^ packet[2];

    memcpy(&packet[4], buffer, lenght);

    unsigned char BCC2 = 0;
    int i = 0;

    for (; i < lenght; i++)
        BCC2 ^= buffer[i];

    packet[lenght + 4] = BCC2;
    packet[lenght + 5] = FLAG;

    int newLenght = stuffing(packet, lenght + 6);

    while (tries < 3 && getTimeOut() == TRUE)
    {
        //printf("Entrou\n");
        setTimeOut(FALSE);
        alarm(linkLayer->timeout);

        if (write(linkLayer->fd, packet, 6 + newLenght) < 0)
        {
            perror("write");
            return -1;
        }

        char response[5];
        int r = 0;

       //TODO TIRAR ISTO QUANDO MAQUINA DE ESTADOS DE RR E REJ ESTIVER FEITA
        while (1)
        {
         //   printf("%d\n", r);
            if ((r = read(linkLayer->fd, response, 5)) > 0)
            {
                printf("Break\n");
                setTimeOut(FALSE);
                break;
            }
        }

        if (linkLayer->sequenceNumber == 1)
        {
            if (response[2] == C_RR1)
                linkLayer->nRR++;

            else if (response[2] == C_REJ1)
                linkLayer->nREJ++;
        }

        else if (linkLayer->sequenceNumber == 0)
        {
            if (response[2] == C_RR0)
                linkLayer->nRR++;

            else if (response[2] == C_REJ0)
                linkLayer->nREJ++;
        }

        alarm(0);
    }

    if (tries == 3)
    {
        printf("Failed to send the message (%d attemps)\n", linkLayer->numTransmissions);
       	return -1;
    }
    else
    {
        printf("%s\n", "UA was received");
    }
    return 0;
}

int llread(LinkLayer *linkLayer)
{
    setTimeOut(FALSE);

    int size = validateFrame(linkLayer->fd, linkLayer->frame);

    size = destuffing(linkLayer->frame, size);
//TODO BBC2 verification (é depois do bytedestuffing)
    /* se frame[frame_lenght-3] for igual ao bytesuff
        faz-se destuff
        se nao BccReceived=frame[frame_lenght-2]*/

    return size;
}

int llcloseT(LinkLayer *linkLayer)
{
    resetTries();
    setTimeOut (TRUE);

    while (!outOfTries(linkLayer->numTransmissions)  && getTimeOut() == TRUE)
    {

        setTimeOut (FALSE);
        alarm(linkLayer->timeout);

        sendMessage(linkLayer->fd, DISC);

        //	receiveResponse(linkLayer->fd);
        stateValidMessage(linkLayer->fd, DISC);

        alarm(0);
    }

    if (outOfTries(linkLayer->numTransmissions) )
    {
        printf("Failed to send the message (%d attemps)\n", linkLayer->numTransmissions);
        return -1;
    }
    else
    {
        printf("%s\n", "UA was received");
    }

    sendMessage(linkLayer->fd, UA);

	return 0;
}

int llcloseR(LinkLayer *linkLayer)
{
    resetTries();
    setTimeOut (TRUE);

    (void)signal(SIGALRM, alrmHanler);

   // stateValidMessage(linkLayer->fd, DISC);
    printf("DISC received\n");
    while (!outOfTries(linkLayer->numTransmissions) && getTimeOut() == TRUE)
    {

        setTimeOut (FALSE);
        alarm(linkLayer->timeout);

        sendMessage(linkLayer->fd, DISC);
        printf("DISC sent\n");
        //	receiveResponse(linkLayer->fd);
        stateValidMessage(linkLayer->fd, UA);
        printf("UA received\n");
        alarm(0);
    }

    if (outOfTries(linkLayer->numTransmissions) )
    {
        printf("Failed to send the message (%d attemps)\n", linkLayer->numTransmissions);
       	return  -1;
    }
    else
    {
        printf("%s\n", "UA was received");
    }

	return 0;
}
