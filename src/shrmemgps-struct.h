/* ---------------------------------------------------------------------- */
/* structures for the GPS Thunderbolt status  */
/* ---------------------------------------------------------------------- */

#define NbAnswerString 15

struct shmgps {
  int shmId;
  int timing_flag;
  int week_number;
  int time_of_week;
  int UTC_offset;
  int year;
  int month;
  int day_of_month;
  int hours;
  int minutes;
  int seconds;
  long timeGPS,timeCPU;
  double diffCPU_GPS;
  char s0x8FAB_1[128];
  char s0x8FAB_2[128];
  char s0x8FAB_3[128];
  char s0x8FAC_1[128];
  char s0x8FAC_2[128];
  char s0x8FAC_3[128];
  char s0x8FAC_4[128];
  char s0x8FAC_5[128];
  char s0x8FAC_6[128];
  char s0x8FAC_7[128];
  char s0x8FAC_8[128];
  char s0x8FAC_9[128];
  char s0x8FAC_10[128];
  char s0x8FAC_11[128];
  char answer[NbAnswerString][128];
  int idxAnswerString;
  int nbchar2send;
  char char2send[16];
};
