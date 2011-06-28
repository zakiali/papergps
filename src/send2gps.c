#include <stdio.h>
#include "dateheure.h"
#include "shrmemgps.h"

/* --------------------------------------------------------------------- */
int main(int argc, char **argv)
{int pid,i,byte;
char prgid[32];

pid=getpid();
strcpy(prgid,argv[0]);

if(argc<2) {
  printf(" Use: %s <HexaCode_to_send>\n",prgid);
  printf("  will send command to the Trimble ThunderBolt GPS receiver connected to the serial line\n");
  printf("  <HexaCode_to_send> : code like 1f, 27, 8e ac 07, ...\n");
  printf("    1f = Request Software Version\n");
  printf("    27 = Request Signal Levels\n");
  printf("    29 = Request Almanach Health Page\n");
  printf("    8e ac 07 = Save Segments to EEPROM (Accurate Position)\n");
  printf("  Warning! All the answers from the GPS are not displayed yet on statgps/gps.log\n");
  exit(2);
}

/* - install the shared memory gps - */
if((i=initshrmemgps(prgid)) < 0){
  printf(" *** Error initialization of shared memory segment gps  code=%d ***\n",i);
  exit(-2);
} else {
  printf("%s> Attach to the shared memory segment \"gps\" successfull (ShmId=%d)\n",prgid,shmgps->shmId);
  /*  printf(" -- Initialization of shared memory segment OK --\n"); */
}

sprintf(shmgps->char2send,"%c",0x10);
shmgps->nbchar2send=1;
for(i=1;i<argc;i++) {
  sscanf(argv[i],"%x",&byte);
  sprintf(&shmgps->char2send[i],"%c",byte);
  shmgps->nbchar2send++;
}

sprintf(&shmgps->char2send[argc],"%c%c",0x10,0x03);
shmgps->nbchar2send+=2;

}
