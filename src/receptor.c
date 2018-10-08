/*Non-Canonical Input Processing*/
#include "utilities.c"

int main(int argc, char** argv)
{
    int fd;//res;
    struct termios oldtio,newtio;
    //char buf[5];
    char result_A_C[2];

    if ( (argc < 2) ||
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) &&
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }


  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */


    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */



  /*
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
    leitura do(s) pr�ximo(s) caracter(es)
  */



    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set( Noncanonical mode)\n\n");
    printf("Ready to receve messsage\n");

    stateValidMessage(fd,result_A_C);
    int i;
    for(i=0;i<2;i++){
        printf("%x\n",result_A_C[i]);
    }


    /*for(int i=0;i<BYTE_TO_SEND;i++){
      res=read(fd,buf,1);
      printf("%d\n",buf[i]);

      if(res==-1){
        perror("read");
        exit(-1);
      }
    }*/





  /*
    O ciclo WHILE deve ser alterado de modo a respeitar o indicado no gui�o
  */

    sleep(2);

    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}
