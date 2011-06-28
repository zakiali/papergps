#include <math.h>
#include <stdio.h>
#include <string.h>

/* ----------------------------------------------------------------- */
 double gregjd(int jy, int jm, int jd, double rh, double rm, double rs)
/* ----------------------------------------------------------------- */
 {
 double rj,ra,rb,rg,tjd,y,m;
	rj=jd+rh/24.0+rm/1440.0+rs/86400.0;
	if(jm<=2) {
		y=jy-1;
		m=jm+12;
		}
		else {
			y=jy;
			m=jm;
			};
	rg=y+m/100+rj/10000;
	ra=0.0;
	rb=0.0;
	if (rg>=1582.1015) {
		ra=floor(y/100.0);
		rb=2-ra+floor(ra/4.0);
		}
	tjd=floor(365.25*y) + floor(30.6001*(m+1)) + rj +1720994.5 + rb;
	return tjd;
 }
/* ----------------------------------------------------------------- */

/* ----------------------------------------------------------------- */
 void jdgreg(double tjd, int *jy, int *jm, int *jd, double *rh,
				double *rm, double *rs)
/* ----------------------------------------------------------------- */
  {
  double z,f,a,alpha,b,c,d,e,g;
		f=modf(tjd+0.5,&z);
		if (z<2299161.0)  {
			a=z;
			};
		if (z>=2299161.0) {
			alpha=floor((z-1867216.25e0)/36524.25e0);
			a=z+1.0e0+alpha-floor(alpha/4.0e0);
			};
		b=a+1524.0e0;
		c=floor((b-122.1e0)/365.25e0);
		d=floor(365.25e0*c);
		e=floor((b-d)/30.6001e0);
		g=b-d-floor(30.6001e0*e)+f;
		*jd=floor(g);
		*rh=floor(24.0*modf(g,&a));
		*rm=floor(60.0*modf(24.0*modf(g,&a),&b));
		*rs=60.0*modf(60.0*modf(24.0*modf(g,&a),&b),&d);
		if (e<13.5) {
			*jm=floor(e-1.0);
			}
			else {
			*jm=floor(e-13.0);
			};
		if (*jm>2.5) {
			*jy=floor(c-4716.0);
			}
			else {
			*jy=floor(c-4715.0);
			};
  }
/* ----------------------------------------------------------------- */

int tuts(int jy, int jm, int jd, int tuh, int tum, int tus, int *tsh, int *tsm, int *tss)
{double tu,rh,rm,rs,tj,t,TSo,TS;

tu=(tuh+tum/60.0e0+tus/3600.0e0)/24.0e0;
rh=0.0e0; rm=0.0e0; rs=0.0e0;
tj=gregjd(jy,jm,jd,rh,rm,rs);
t=(tj-2451545.0e0)/36525.0e0;
TSo=24110.54841e0+8640184.812866e0*t+0.093104e0*t*t-6.2e-6*t*t*t;
TSo=TSo/86400.0e0;
while (TSo < 0.0e0) TSo=TSo+1.0e0;
tu=tu*1.0027379093e0;
TS=TSo + tu - (-6.104729938e-3); /* Nancay */
/* TS=TSo + tu - (-6.493055556e-3); */ /* Paris */
while (TS < 0.0e0) TS=TS+1.0e0;
while (TS > 1.0e0) TS=TS-1.0e0;
TS=24.0e0*TS;
*tsh=TS;
*tsm=(TS-(double)(*tsh))*60.0e0;
*tss=(TS-(double)(*tsh)-(double)(*tsm)/60.0e0)*3600.0e0;
return(0);
}

/* ----------------------------------------------------------------- */
int tstu(int jy, int jm, int jd, int tsh, int tsm, int tss, int *tuh, int *tum, int *tus)
{double tsday,jd0,t,TSo,tu,rh,rm,rs;

tsday=(tsh+tsm/60.0+tss/3600.0)/24.0;
/* printf("tstu> IN jy=%04d jm=%02d jd=%02d  tsh=%02d tsm=%02d tss=%02d\n",jy,jm,jd,tsh,tsm,tss); */
rh=rm=rs=0.0;
jd0=gregjd(jy,jm,jd,rh,rm,rs);
/* printf("tstu> jd0=%lf\n",jd0); */
t=(jd0-2451545.0e0)/36525.0e0;
TSo=24110.54841e0+8640184.812866e0*t+0.093104e0*t*t-6.2e-6*t*t*t;
TSo=TSo/86400.0e0;
/* printf("tstu> TSo=%lf\n",TSo); */
while(TSo < 0.0) TSo=TSo+1.0e0;
/* TSo=TSo - (-6.493055556e-3); */ /* Paris */
TSo=TSo - (-6.104729938e-3); /* Nancay */
tu=((tsh+tsm/60.0+tss/3600.0)/24.0 - TSo);
while(tu < 0.0e0) tu=tu+1.0e0;
while(tu > 1.0e0) tu=tu-1.0e0;
tu=24.0e0*tu/1.0027379093e0;
*tuh=tu;
*tum=(tu-(double)(*tuh))*60.0e0;
*tus=(tu-(double)(*tuh)-(double)(*tum)/60.0e0)*3600.0e0;
/* printf("tstu> OUT tuh=%02d tum=%02d tus=%02d\n",*tuh,*tum,*tus); */
return(0);
}

/* ----------------------------------------------------------------- */

int mjd2str(double mjdi, double mjdf, char string[])
{double d,ds,rh,rm,rs;
int jy,jm,jd,jhr,jmn,jsc;
static char *month[12]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

d=mjdi+mjdf;
if(mjdi<2500000.0) d+=2400000.5;
jdgreg(d,&jy,&jm,&jd,&rh,&rm,&rs);
d=mjdf-0.5; if(mjdi<2500000.0) d=mjdf;
if(d<0.0) d+=1.0;
d*=86400.0;
jhr=(int)(d/3600.0);
jmn=(int)((d-jhr*3600.0)/60.0);
ds=d-(double)jhr*3600.0-(double)jmn*60.0;
jsc=(int)(d-(double)jhr*3600.0-(double)jmn*60.0 + 0.5);
/* printf("mjd2str> %lf %d %d %lf  rs=%lf\n",d,jhr,jmn,ds,rs); */
/* sprintf(string,"%s %02d, %04d  %02d:%02d:%6.3lf",month[jm-1],jd,jy,jhr,jmn,ds); */
sprintf(string,"%s %02d, %04d  %02d:%02d:%02d",month[jm-1],jd,jy,jhr,jmn,jsc);
return(0);
}

/* ----------------------------------------------------------------- */
int mjd2strms(double mjdi, double mjdf, char string[])
{double d,ds,rh,rm,rs;
int jy,jm,jd,jhr,jmn;
static char *month[12]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

d=mjdi+mjdf;
if(mjdi<2500000.0) d+=2400000.5;
jdgreg(d,&jy,&jm,&jd,&rh,&rm,&rs);
d=mjdf-0.5; if(mjdi<2500000.0) d=mjdf;
if(d<0.0) d+=1.0;
d*=86400.0;
jhr=(int)(d/3600.0);
jmn=(int)((d-jhr*3600.0)/60.0);
ds=d-(double)jhr*3600.0-(double)jmn*60.0;
rs=(d-(double)jhr*3600.0-(double)jmn*60.0);
/* printf("mjd2str> %lf %d %d %lf  rs=%lf\n",d,jhr,jmn,ds,rs); */
/* sprintf(string,"%s %02d, %04d  %02d:%02d:%6.3lf",month[jm-1],jd,jy,jhr,jmn,ds); */
sprintf(string,"%s %02d, %04d  %02d:%02d:%06.3lf",month[jm-1],jd,jy,jhr,jmn,rs);
return(0);
}

/* ----------------------------------------------------------------- */
/* -- calculates week day (i) from julian date (jd)                  */
/* -- i= 0:sunday, 1:monday, 2:tuesday, ..., 6:saturday              */
/* ----------------------------------------------------------------- */
int weekday(double jd)
{double jdc,div; int i;
/* -- Find the julian date for 0h UT -- */
jdc=floor(jd+0.5)-0.5;
div=(jdc+1.5)/7.0;
i=(int)rint((div-floor(div))*7.0);
return(i);
}

/* ----------------------------------------------------------------- */
// sur http://www.univ-lemans.fr/~hainry/articles/calart.html
// Paques : le dimanche qui suit la premiere lune de printemps
// Ascension : un jeudi 39 jours apres le dimanche de Paques
// Pentecote : un dimanche 49 jours apres le dimanche de Paques
/* ----------------------------------------------------------------- */
int easter(int jy, int *jm, int *jd)
{int a,b,c,d,e,f,g,h,i,k,q,m,n,p,z;
float t;

t=(float)jy/19.0; a=floor(0.5+(t-floor(t))*19.0);
b=jy/100; c=jy%100;
// printf("easter> a=%d b=%d c=%d\n",a,b,c);
t=(float)b/4.0; d=(int)t; e=(int)floor(0.5+(t-floor(t))*4.0);
// printf("easter> d=%d e=%d\n",d,e);
t=(float)(b+8)/25.0; f=(int)t;
// printf("easter> f=%d\n",f);
t=(float)(b-f+1)/3.0; g=(int)t;
// printf("easter> g=%d\n",g);
z=19*a+b-d-g+15;
t=(float)z/30.0; h=(int)floor(0.5+(t-floor(t))*30.0);
// printf("easter> h=%d\n",h);
t=(float)c/4.0; i=(int)t; k=(int)floor(0.5+(t-floor(t))*4.0);
// printf("easter> i=%d k=%d\n",i,k);
z=32+2*e+2*i-h-k; t=(float)z/7.0; q=(int)floor(0.5+(t-floor(t))*7.0);
// printf("easter> q=%d\n",q);
z=a+11*h+22*q; t=(float)z/451.0; m=(int)t;
// printf("easter> m=%d\n",m);
z=h+q-7*m+114; t=(float)z/31.0; n=(int)t; p=(int)floor(0.5+(t-floor(t))*31.0);
// printf("easter> n=%d p=%d\n",n,p);
*jm=n; *jd=p+1;
return(0);
}

/* ----------------------------------------------------------------- */
int congesmobstr(int jy, char string[])
/* ----------------------------------------------------------------- */
{int jm,jd; char st[32];
double tj,rh,rm,rs;
static char *day[7]={"Dim","Lun","Mar","Mer","Jeu","Ven","Sam"};

rh=12.0; rm=rs=0.0;
easter(jy,&jm,&jd);
tj=gregjd(jy,jm,jd,rh,rm,rs)+1.0;
jdgreg(tj,&jy,&jm,&jd,&rh,&rm,&rs);
sprintf(string,"Paques: %04d.%02d.%02d(%s)",jy,jm,jd,day[weekday(tj)]);
tj=gregjd(jy,jm,jd,rh,rm,rs)+38.0;
jdgreg(tj,&jy,&jm,&jd,&rh,&rm,&rs);
sprintf(st," Ascension: %04d.%02d.%02d(%s)",jy,jm,jd,day[weekday(tj)]); strcat(string,st);
tj=gregjd(jy,jm,jd,rh,rm,rs)+11.0;
jdgreg(tj,&jy,&jm,&jd,&rh,&rm,&rs);
sprintf(st," Pentecote: %04d.%02d.%02d(%s)",jy,jm,jd,day[weekday(tj)]); strcat(string,st);
return(0);
}
