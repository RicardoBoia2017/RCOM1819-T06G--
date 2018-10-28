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

void openPort(LinkLayer *linkLayer)
{
    linkLayer->fd = open(linkLayer->port, O_RDWR | O_NOCTTY); //port = /dev/ttyS0

    if (linkLayer->fd == -1)
        perror("openPort");
}

void setTermiosStructure(LinkLayer *linkLayer)
{
    if (tcgetattr(linkLayer->fd, &oldtio) == -1)
    { /* save current port settings */
        perror("setTermiosStructure");
        exit(-1);
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
        exit(-1);
    }
}

void llopenT(LinkLayer *linkLayer)
{

    (void)signal(SIGALRM, alrmHanler);

    char result_A_C[2]; //Tirar isto depois

    while (!outOfTries(linkLayer->numTransmissions)  && getTimeOut() == TRUE)
    {
        setTimeOut(FALSE);

        alarm(3);

        sendMessage(linkLayer->fd, SETUP);
        //	receiveResponse(linkLayer->fd);
        stateValidMessage(linkLayer->fd, result_A_C, UA);

        alarm(0);
    }

    if (outOfTries(linkLayer->numTransmissions) )
    {
        printf("%s\n", "Failed to send the message (3 attemps)");
        exit(-1);
    }
    else
    {
        printf("%s\n", "UA was received");
    }
}

void llopenR(LinkLayer *linkLayer)
{
    char result_A_C[2];
    setTimeOut(FALSE);

    stateValidMessage(linkLayer->fd, result_A_C, SETUP);
    printf("SETUP receives\n");
    sendMessage(linkLayer->fd, UA);
    printf("UA sent\n");
}

int llwrite(LinkLayer *linkLayer, char *buffer, int lenght)
{

    (void)signal(SIGALRM, alrmHanler); //TIRAR DEPOIS

    char *packet = malloc(6 + lenght);
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

    //stuffing(packet, lenght + 6);

    while (tries < 3 && getTimeOut() == TRUE)
    {
        //printf("Entrou\n");
        setTimeOut(FALSE);
        alarm(3);

        if (write(linkLayer->fd, packet, 255) < 0)
        {
            perror("write");
            exit(-1);
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
        printf("%s\n", "Failed to send the message (3 attemps)");
        exit(-1);
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

   // size = destuffing(linkLayer->frame, size);
    //TODO BBC2 verification (não sei se antes ou depois do stuffing)

    return size;
}

void llcloseT(LinkLayer *linkLayer)
{
    (void)signal(SIGALRM, alrmHanler); //TODO tirar isto, isto só serve para testes com o llopen comentado

    resetTries();
    setTimeOut (TRUE);
    char result_A_C[2];

    while (!outOfTries(linkLayer->numTransmissions)  && getTimeOut() == TRUE)
    {

        setTimeOut (FALSE);
        alarm(3);

        sendMessage(linkLayer->fd, DISC);

        //	receiveResponse(linkLayer->fd);
        stateValidMessage(linkLayer->fd, result_A_C, DISC);

        alarm(0);
    }

    if (outOfTries(linkLayer->numTransmissions) )
    {
        printf("%s\n", "Failed to send the message (3 attemps)");
        exit(-1);
    }
    else
    {
        printf("%s\n", "UA was received");
    }

    sendMessage(linkLayer->fd, UA);
}

void llcloseR(LinkLayer *linkLayer)
{
    resetTries();
    setTimeOut (TRUE);
    char result_A_C[2];

    (void)signal(SIGALRM, alrmHanler);

   // stateValidMessage(linkLayer->fd, result_A_C, DISC);
    printf("DISC received\n");
    while (!outOfTries(linkLayer->numTransmissions) && getTimeOut() == TRUE)
    {

        setTimeOut (FALSE);
        alarm(3);

        sendMessage(linkLayer->fd, DISC);
        printf("DISC sent\n");
        //	receiveResponse(linkLayer->fd);
        stateValidMessage(linkLayer->fd, result_A_C, UA);
        printf("UA received\n");
        alarm(0);
    }

    if (outOfTries(linkLayer->numTransmissions) )
    {
        printf("%s\n", "Failed to send the message (3 attemps)");
        exit(-1);
    }
    else
    {
        printf("%s\n", "UA was received");
    }
}
