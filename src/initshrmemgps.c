#include <stdio.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "dateheure.h"
#include "shrmemgps-extern.h"

/* To Show Shared Memory Segment available : ipcs -m (-t, ...) */
/* To delete a shared memory segment : ipcrm shm ShmId          */

/* ------------------------------------------------------------------------ */
int gpslog(char msg[])
{char buf[256]; char date[10],heure[10];
FILE *pfmsg;

dateheurec(date,heure);
sprintf(buf,"%s-%s %s\n",date,heure,msg);
if((pfmsg=fopen(gpslogfile,"a")) != NULL) {
  fputs(buf,pfmsg);
  fclose(pfmsg);
} else {
  printf("Unable to open file <%s>...\n",gpslogfile);
}
return(0);
}

/* ---------------------------------------------------------------------- */
int mysleep(float tsec)
/* ---------------------------------------------------------------------- */
{struct timespec req,rem;
int tseci; float tsecf;

tseci=(int)tsec; tsecf=tsec-(float)tseci;
req.tv_sec=(time_t)tseci; req.tv_nsec=(long)(tsecf*1.0e9);
nanosleep(&req,&rem);
return(0);
}

/* ---------------------------------------------------------------------- */
/*  Function to create and attach the shared memory GPS                   */
/* ---------------------------------------------------------------------- */
int initshrmemgps(char name[])
{int shmi,verbose;  key_t cle;

verbose=0;
/* -- Creation of the key to define the shared memory segment -- */
if((cle=ftok("/bin/awk",'e')) < 0){
  perror("*** error creation cle ftok ***");
  printf("initshrmemgps> Error ftok with (/bin/awk e)\n");
  return(-1);
} else {
  if(verbose) printf("initshrmemgps> Creation of key with ftok OK\n");
}
if(verbose)printf("initshrmemgps> cle=%d   (sizeshmgps=%d)\n",cle,sizeshmgps);

/* -- Creation of the shared memory segment defined by the key and the size -- */
if((shmi=shmget(cle,sizeshmgps,IPC_CREAT|0666)) < 0){
  perror("*** error shmget ***");
  printf("initshrmemgps> Error shmget with cle=%d size=%d par=IPC_CREAT|0666\n",cle,sizeshmgps);
  printf("initshrmemgps> Error shmget with cle=%d size=%d(%.2fMB) par=IPC_CREAT|0666\n",cle,sizeshmgps,(float)sizeshmgps/1048576.0);
  return(-2);
} else {
  if(verbose) printf("initshrmemgps> Creation of new shared memory segment with shmget OK\n");
}

/* -- Attachment of the segment to the structure pointer -- */
if((shmgps=(struct shrmemgps *)shmat(shmi,0,0)) == (struct shrmemgps *)-1){
  perror("*** error shmat ***");
  printf("initshrmemgps> Error shmat with result of shmget shmi=%d\n",shmi);
  return(-3);
} else {
  if(verbose) printf("initshrmemgps> Attachement of shared memory segment with shmat OK\n");
}
shmgps->shmId=shmi;
if(verbose) printf("initshrmemed> shmgps.shmId=%d\n",shmgps->shmId);
printf("initshrmemed> shmgps.shmId=%d\n",shmgps->shmId);
return(shmi);
}
