#include <curses.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <stdarg.h>
#include <math.h>
#include "dateheure.h"
#include "shrmemgps.h"

#define ERROR -1
#define MINROWS         35
#define MINCOLS         80
#define MARGIN          1
#define LSTRLEN 256

char date[13],heure[10];
char prgid[80];
int pid;

/* -- Prototypes -- */
void print(int row, char *format, ...);
int mysleep(float tsec);

/* --------------------------------------------------------------------- */
/* --------------------------------------------------------------------- */
int main(int argc, char **argv)
{int i,j,il,ISATTY,heigh,jh,jm,js,jcur,jdiff;
WINDOW *window;
char date[13],heure[10],tu[10],host[80],str1[80],str2[80],dateobserv[50],sign[2],*adr;
int elh,elm,els;
double d;

pid=getpid();
strcpy(prgid,argv[0]);

gethostname(host,80);
if((adr=(char *)strstr(host,".")) != NULL) sprintf(adr,"\0");
/* - install the shared memory gps - */
if((i=initshrmemgps(prgid)) < 0){
  printf(" *** Error initialization of shared memory segment gps  code=%d ***\n",i);
  exit(-2);
} else {
  printf("%s> Attach to the shared memory segment \"gps\" successfull (ShmId=%d)",prgid,shmgps->shmId);
/*  printf(" -- Initialization of shared memory segment OK --\n"); */
}

ISATTY = isatty(STDOUT_FILENO);
if (ISATTY) {
   printf("Initialization of screen by ncurses...\n");
   window=initscr();
   printf("Initialization of screen by ncurses... OK!\n");
   cbreak(); noecho();
   nonl(); intrflush(window, FALSE); keypad(window, TRUE); leaveok( window, TRUE );
   printf("Initialization of screen by ncurses... OK!\n");
} else {
   printf("It's not a tty... I don't know what to do... Exit!\n");
   exit(2);
}

while(1){
   d=dateheureimpr(date,heure,tu);
   sscanf(heure,"%02d:%02d:%02d",&jh,&jm,&js); jcur=jh*3600+jm*60+js;
   print(1," ------------------------------------------ ");
   print(2," |  GPS shared memory                     | ");
   print(3," |   host=%7s      shm_id= %10d |",host,shmgps->shmId);
   print(4," ------------------------------------------ ");
   print(6,"  %s  %s UTC\n",date,heure);
   print(7,"                %s LST\n",tu);

   print(9," %s",shmgps->s0x8FAB_1);
   print(10," %s",shmgps->s0x8FAB_2);
   print(11," %s",shmgps->s0x8FAB_3);

   print(13," diff_GPS_CPU= %.6lf s",shmgps->diffCPU_GPS);
   
   print(15," %s",shmgps->s0x8FAC_1);
   print(16," %s",shmgps->s0x8FAC_2);
   print(17," %s",shmgps->s0x8FAC_3);
   print(18," %s",shmgps->s0x8FAC_4);
   print(19," %s",shmgps->s0x8FAC_5);
   print(20," %s",shmgps->s0x8FAC_6);
   print(21," %s",shmgps->s0x8FAC_7);
   print(22," %s",shmgps->s0x8FAC_8);
   print(23," %s",shmgps->s0x8FAC_9);
   print(24," %s",shmgps->s0x8FAC_10);
   print(25," %s",shmgps->s0x8FAC_11);

   il=27;
   for(i=0;i<shmgps->idxAnswerString;i++) {
     print(il," %s",shmgps->answer[i]);
     il++;
   }
   for(i=il;i<40;i++) print(i,"                                                            ");

   move(MINROWS-1, 1);
   refresh();

   mysleep(0.5);
}   /* while(1) */

}


/* --------------------------------------------------------------------- */
void print(int row, char *format, ...)
{int nrow = MINROWS , ncol = MINCOLS;

char out[LSTRLEN];
va_list ap;
    
va_start(ap,format);
move(row, MARGIN);
vsprintf(out, format, ap);
out[ncol-MARGIN] = '\0';
printw(out);
clrtoeol();
va_end(ap);
}

