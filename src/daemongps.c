#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "tsip_ifc.h"
#include "tsipincl.h"
#include "dateheure.h"
#include "shrmemgps.h"

char fileserial[60];
int fdserial;
#define ERROR -1

/* --------------------------------------------------------------------- */
int main(int argc, char **argv)
{
int i;
char prgid[64],date[10],texte[64],heure[10],ts[10];
pid_t pidp;
struct termios tbuf,tbufsave;
unsigned short cu;
static TSIPPKT rpt;

pidp=getpid();
strcpy(prgid,argv[0]);
sprintf(fileserial,"/dev/ttyS0");

/* -- Open and init the serial line -- */
if((fdserial=open(fileserial,O_RDWR,0)) == -1){
  perror("*** erreur open serial ***");
  return(ERROR);
}
sprintf(gpslinelog,"%s> Start with pid=%d fileserial=%s",prgid,pidp,fileserial); gpslog(gpslinelog);
if(tcgetattr(fdserial,&tbuf) == -1){
  perror("*** erreur tcgetattr ***");
  return(ERROR);
}

tbufsave=tbuf;
tbuf.c_lflag = 0;
tbuf.c_iflag = 0x202;
 tbuf.c_cflag |= B9600;
 tbuf.c_cflag |= CS8;
 tbuf.c_cflag &= ~PARENB;
tbuf.c_cc[VMIN]=9;
tbuf.c_cc[VTIME]=4;
tbuf.c_cc[VINTR]=128;
tbuf.c_cc[VQUIT]=128;
tbuf.c_cc[VERASE]=128;
tbuf.c_cc[VKILL]=128;
tbuf.c_cc[VEOF]=128;
tbuf.c_cc[VEOL]=128;
tbuf.c_cc[VEOL2]=128;
tbuf.c_cc[VSTART]=128;
tbuf.c_cc[VSUSP]=128;
tbuf.c_cc[VSTOP]=128;
tbuf.c_cc[VREPRINT]=128;
tbuf.c_cc[VDISCARD]=128;
tbuf.c_cc[VWERASE]=128;
tbuf.c_cc[VLNEXT]=128;
if(cfsetospeed(&tbuf,B9600) == -1){
  perror("*** erreur cfsetospeed ***");
  return(ERROR);
}
if(cfsetispeed(&tbuf,B9600) == -1){
  perror("*** erreur cfsetispeed ***");
  return(ERROR);
}
if(tcsetattr(fdserial,TCSANOW,&tbuf) == -1){
  perror("*** erreur tcsetattr ***");
  return(ERROR);
}
sprintf(gpslinelog,"%s> Serial line %s set to 9600 bauds",prgid,fileserial);
gpslog(gpslinelog);

/* - install the shared memory gps - */
if((i=initshrmemgps(prgid)) < 0){
  printf(" *** Error initialization of shared memory segment gps  code=%d ***\n",i);
  exit(-2);
} else {
  printf("%s> Attach to the shared memory segment \"gps\" successfull (ShmId=%d)",prgid,shmgps->shmId);
  sprintf(gpslinelog,"%s> Attach to the shared memory segment \"gps\" successfull (ShmId=%d)",prgid,shmgps->shmId);
  gpslog(gpslinelog);
	  /*  printf(" -- Initialization of shared memory segment OK --\n"); */
}

while(1) {
  if(read(fdserial,&cu,1)<0){
    printf("ERR sndthira:%s",strerror(errno));
    return(-1);
  }
  cu=cu & 0xff;
  /* printf(" <%02x>",(cu & 0xff)); */
  tsip_input_proc (&rpt, (short)cu);
  if (rpt.status == TSIP_PARSED_FULL) {
    if((rpt.code == 0x8f) && (rpt.buf[0] == 0xab) ) {
      dateheurec(date,heure);
      printf("\nReport Packet 0x8F-AB Primary Timing Packet received\nCOMPUTER DATE/TIME  %s %s\n",date,heure);
    }
    rpt_packet (&rpt);
    printf("\n");
  }

  /* -- Test if there is something to send -- */
  if(shmgps->nbchar2send > 0) {
    sprintf(gpslinelog,"%s> will send <0x",prgid);
    for(i=0;i<shmgps->nbchar2send;i++) { sprintf(texte,"%02x",(shmgps->char2send[i] & 0xff)); strcat(gpslinelog,texte); }
    strcat(gpslinelog,">");
    gpslog(gpslinelog);
    for(i=0;i<shmgps->nbchar2send;i++) write(fdserial,&shmgps->char2send[i],1);
    shmgps->nbchar2send=0;
  }
}
close(fdserial);

return(0);
}
