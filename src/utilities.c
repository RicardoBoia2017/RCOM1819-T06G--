#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>

#define FLAG 0x7E
#define AD 0x03
#define UA 0x07
#define emissor 1
#define recetor 0

int verifier(int step,unsigned char mess,int id){
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
				step=0;
		
