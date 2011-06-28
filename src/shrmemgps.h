/* ---------------------------------------------------------------------- */
/* structures for the GPS  */
/* ---------------------------------------------------------------------- */

#include "shrmemgps-struct.h"

/* - Address of the structures - */
struct shmgps *shmgps;
int sizeshmgps=sizeof(struct shmgps);
char gpslogfile[128]="gps.log";
char gpslinelog[256];
