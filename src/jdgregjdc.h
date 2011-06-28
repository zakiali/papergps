double gregjd(int jy, int jm, int jd, double rh, double rm, double rs);
void jdgreg(double tjd, int *jy, int *jm, int *jd, double *rh, double *rm, double *rs);
int tuts(int jy, int jm, int jd, int tuh, int tum, int tus, int *tsh, int *tsm, int *tss);
int tstu(int jy, int jm, int jd, int tsh, int tsm, int tss, int *tuh, int *tum, int *tus);
int mjd2str(double mjdi, double mjdf, char string[]);
int mjd2strms(double mjdi, double mjdf, char string[]);
int weekday(double jd);
int easter(int jy, int *jm, int *jd);
