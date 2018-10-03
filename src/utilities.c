#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define FLAG 0x7E
#define AD 0x03

volatile int STOP=FALSE;


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
		
