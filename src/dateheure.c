#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <stdio.h>

double gregjd(int jy, int jm, int jd, double rh, double rm, double rs);
int tuts(int jy, int jm, int jd, int tuh, int tum, int tus, int *tsh, int *tsm, int *tss);

/* ------------------------------------------------------------------------ */
/* -- provide date/hour as a formated string (with ms) and double        -- */
/* ------------------------------------------------------------------------ */
double dateheurec(char *dateymd, char *heurehms)
{ int status;
struct timeval tv; struct tm *datetime;
double d;

status=gettimeofday(&tv,NULL);
if(status != 0)return(status);
datetime=gmtime(&tv.tv_sec);
sprintf(dateymd,"%04d%02d%02d",datetime->tm_year+1900,datetime->tm_mon+1,datetime->tm_mday);
sprintf(heurehms,"%02d%02d%02d.%03d",datetime->tm_hour,datetime->tm_min,datetime->tm_sec,
  (int)(tv.tv_usec/1.0e3+0.5));
status=(int)tv.tv_sec;
d=(double)status+(double)tv.tv_usec*1.0e-6;
#if DEBUG
printf("dateheure>  date=<%s>  heure=<%s>\n",dateymd,heurehms);
#endif
return(d);
}

/* ------------------------------------------------------------------------ */
/* -- provide date/hour as a compact formated string and double          -- */
/* ------------------------------------------------------------------------ */
double dateheure(char *dateymd, char *heurehms)
{ int status;
struct timeval tv; struct tm *datetime;
double d;

status=gettimeofday(&tv,NULL);
if(status != 0)return(status);
datetime=gmtime(&tv.tv_sec);
sprintf(dateymd,"%04d%02d%02d",datetime->tm_year+1900,datetime->tm_mon+1,datetime->tm_mday);
sprintf(heurehms,"%02d%02d%02d",datetime->tm_hour,datetime->tm_min,datetime->tm_sec);
status=(int)tv.tv_sec;
d=(double)status+(double)tv.tv_usec*1.0e-6;
#if DEBUG
printf("dateheure>  date=<%s>  heure=<%s>\n",dateymd,heurehms);
#endif
return(d);
}

/* ------------------------------------------------------------------------ */
/* -- provide date/hour as formated (suitable to men) and double         -- */
/* ------------------------------------------------------------------------ */
double dateheureimpr(char *dateymd, char *heurehms, char *tuhms)
{int status,timezone;
struct timeval tv; 
struct tm dt, tudt;
time_t utc_time,local_time;
static char *month[12]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
double d;

status=gettimeofday(&tv,NULL);
if(status != 0)return(status);

gmtime_r(&tv.tv_sec,&dt);
localtime_r(&tv.tv_sec,&tudt);

sprintf(dateymd,"%s %02d, %04d",month[dt.tm_mon],dt.tm_mday,dt.tm_year+1900);
sprintf(heurehms,"%02d:%02d:%02d",dt.tm_hour,dt.tm_min,dt.tm_sec);
sprintf(tuhms,"%02d:%02d:%02d",tudt.tm_hour,tudt.tm_min,tudt.tm_sec);

status=(int)tv.tv_sec;
d=(double)status+(double)tv.tv_usec*1.0e-6;
#if DEBUG
printf("dateheureimpr>  date=<%s>  heure=<%s>\n",dateymd,heurehms);
#endif
return(d);
}

/* ------------------------------------------------------------------------ */
/* -- provide date/hour as integers                                      -- */
/* ------------------------------------------------------------------------ */
double dateheureint(int *jy, int *jm, int *jd, int *jhr, int *jmn, int *jsc)
{ int status;
struct timeval tv; struct tm *datetime;
double d;

status=gettimeofday(&tv,NULL);
if(status != 0)return(status);
datetime=gmtime(&tv.tv_sec);
*jy=datetime->tm_year+1900;
*jm=datetime->tm_mon+1;
*jd=datetime->tm_mday;
*jhr=datetime->tm_hour;
*jmn=datetime->tm_min;
*jsc=datetime->tm_sec;
status=(int)tv.tv_sec;
d=(double)status+(double)tv.tv_usec*1.0e-6;
#if DEBUG
printf("dateheureint>  jy=%04d jm=%02d jd=%02d  jhr=%02d jmn=%02d jsc=%02d\n",*jy,*jm,*jd,*jhr,*jmn,*jsc);
#endif
return(d);
}

/* ------------------------------------------------------------------------ */
/* -- provide formated date (in one string) and fraction of day (us)     -- */
/* ------------------------------------------------------------------------ */
long datehour(char *datehour)
{int status;
struct timeval tv; struct tm *datetime;
static char *month[12]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

status=gettimeofday(&tv,NULL);
if(status != 0)return(-1L);
datetime=gmtime(&tv.tv_sec);
sprintf(datehour,"UT %s %02d, %04d %02d:%02d:%02d.%03d",
  month[datetime->tm_mon],datetime->tm_mday,datetime->tm_year+1900,
  datetime->tm_hour,datetime->tm_min,datetime->tm_sec,
  (int)(tv.tv_usec/1.0e3+0.5));
return(tv.tv_usec);
}

/* ------------------------------------------------------------------------ */
/* -- provide current Julian Date                                        -- */
/* ------------------------------------------------------------------------ */
double jdcur(void)
{int status;
struct timeval tv; struct tm *datetime;
double d;

status=gettimeofday(&tv,NULL);
if(status != 0)return(-1L);
datetime=gmtime(&tv.tv_sec);
d=gregjd(datetime->tm_year+1900,datetime->tm_mon+1,datetime->tm_mday,
  (double)datetime->tm_hour,(double)datetime->tm_min,(double)(datetime->tm_sec+tv.tv_usec/1.0e6));
return(d);
}
