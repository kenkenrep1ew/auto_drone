#include <unistd.h> //for open() close() read() 
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <strings.h>
#include <signal.h> //to handle ctrl-c
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>

#define BAUDRATE B9600
#define MODEMDEVICE "/dev/ttyPA0"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define PORT 5556
#define ADDR "192.168.1.1"

void makeATCommand(char *at_command,int seq,char serialRead);
void interrupt(int sig);
int serialOpen(void);
void serialClose(void);
unsigned char serialReadChar(void);



int fd;
struct termios oldtio, newtio;
int isrunning = TRUE;
char at_command[50];

int  main() {
	int c, res ,seq = 0 ,len;
	unsigned char result, serialRead;
	struct sockaddr_in receiver_addr;
	int sock_fd;

	/* Prepare Serial Port */
	if(serialOpen() != 0) return -1;
	signal(SIGINT, interrupt); //set ctrl-c handler

	/* socket */
	sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	receiver_addr.sin_family = AF_INET;
	if( inet_aton( ADDR,  &receiver_addr.sin_addr )== 0) {
		printf("Crap!, Init failed\n");
		close(sock_fd);
		return;
	}
	receiver_addr.sin_port = htons( PORT );

	while (isrunning) {   /*endless loop until ctrl-c is typed */
		serialRead = serialReadChar();
		printf("value is %c\n", serialRead);
		makeATCommand(at_command,seq,serialRead);
		len = strlen(at_command);
		sendto(sock_fd, at_command, len, 0,(struct sockaddr*)&receiver_addr,sizeof(receiver_addr));
		seq ++;
		
	}
 	
 	close(sock_fd);
	serialClose();

	return 0;
}

void makeATCommand(char *at_command,int seq,char serialRead){
	strcpy(at_command,"");
	char at[5] = "AT*" ,command[9],seri_num[6];
	strcpy(at_command,at);
	sprintf(seri_num,"%d",seq);

	if(serialRead == 'k'){
		//takeoff
		strcat(at_command,"REF=");
		strcat(at_command,seri_num);
		strcat(at_command,",");
		strcat(at_command,"290718208\r");
		return;

	}else if(serialRead == 'j'){
		//landing
		strcat(at_command,"REF=");
		strcat(at_command,seri_num);
		strcat(at_command,",");
		strcat(at_command,"290717696\r");
		return;

	}else if(serialRead == 'w'){
		//move forward
		strcat(at_command,"PCMD=");
		strcat(at_command,seri_num);
		strcat(at_command,",");
		strcat(at_command,"1,0,-1102263091,0,0\r");
		return;

	}else if(serialRead == 's'){
		//move backward
		strcat(at_command,"PCMD=");
		strcat(at_command,seri_num);
		strcat(at_command,",");
		strcat(at_command,"1,0,1045220577,0,0\r");
		return;


	}else if(serialRead == 'd'){
		//move to right
		strcat(at_command,"PCMD=");
		strcat(at_command,seri_num);
		strcat(at_command,",");
		strcat(at_command,"1,1045220557,0,0,0\r");
		return;
		
	}else if(serialRead == 'a'){
		//move left
		strcat(at_command,"PCMD=");
		strcat(at_command,seri_num);
		strcat(at_command,",");
		strcat(at_command,"1,-1102263091,0,0,0\r");
		return;

	}else if(serialRead == '1'){
		//rise upward
		strcat(at_command,"PCMD=");
		strcat(at_command,seri_num);
		strcat(at_command,",");
		strcat(at_command,"1,0,0,1102263091,0\r");
		return;
		
	}else if(serialRead == '3'){
		//rise downward
		strcat(at_command,"PCMD=");
		strcat(at_command,seri_num);
		strcat(at_command,",");
		strcat(at_command,"1,0,0,-1102263091,0\r");
		return;
		
	}
	else if(serialRead == 'e'){
		//rotate right
		strcat(at_command,"PCMD=");
		strcat(at_command,seri_num);
		strcat(at_command,",");
		strcat(at_command,"1,0,0,0,1102263091\r");
		return;
		
	}else if(serialRead == 'q'){
		//rotate left
		strcat(at_command,"PCMD=");
		strcat(at_command,seri_num);
		strcat(at_command,",");
		strcat(at_command,"1,0,0,0,-1102263091\r");
		return;
		
	}else if(serialRead == 'x' | serialRead == '2'){
		//hover
		strcat(at_command,"PCMD=");
		strcat(at_command,seri_num);
		strcat(at_command,",");
		strcat(at_command,"0,0,0,0,0\r");
		return;
		
	}else {
		//watchdog
		serialReadChar();
		strcat(at_command,"COMWDG=");
		strcat(at_command,seri_num);
		strcat(at_command,"\r");
		return;
		
	}

}

int serialOpen() {
	fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY ); 
	if (fd <0) {
		printf("open error: %s\n",MODEMDEVICE); 
		return -1; 
	}
	tcgetattr(fd,&oldtio); /* save current port settings */	
	bzero(&newtio,  sizeof(newtio));
	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;
	/* set input mode (non-canonical, no echo,...) */
	newtio.c_lflag = 0;
	newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
	newtio.c_cc[VMIN]     = 1;   /* blocking read until 1 chars received */
	tcflush(fd, TCIFLUSH);
	tcsetattr(fd,TCSANOW,&newtio);
	return 0;
}

void serialClose() {
	tcsetattr(fd,TCSANOW,&oldtio); //restore previous port setting
	close(fd);
}

unsigned char serialReadChar() {
	unsigned char result;
	int length=0;
	tcflush(fd, TCIFLUSH); //clear the receiving buffer
	length = read(fd,&result,1); //blocked until reading 1 byte.
	return result;
}

void interrupt(int sig) { isrunning = FALSE; } //ctrl-c handler
