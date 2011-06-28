/*
 * *************************************************************************
 *
 * Trimble Navigation, Ltd.
 * OEM Products Development Group
 * P.O. Box 3642
 * 645 North Mary Avenue
 * Sunnyvale, California 94088-3642
 *
 * Corporate Headquarter:
 *    Telephone:  (408) 481-8000
 *    Fax:        (408) 481-6005
 *
 * Technical Support Center:
 *    Telephone:  (800) 767-4822	(U.S. and Canada)
 *                (408) 481-6940    (outside U.S. and Canada)
 *    Fax:        (408) 481-6020
 *    BBS:        (408) 481-7800
 *    e-mail:     trimble_support@trimble.com
 *
 * *************************************************************************
 *
 * Vers	Date		   Changes				    								Author
 * ----	---------   ----------------------------------------	   ----------
 * 1.40
 * 5a10  18 jul 97   matched 5.10 manual         						pvwl
 *			11 feb 98	Converted for tboltcht								jah
 * *************************************************************************
 *
 * TSIP_RPT.C consists of a primary function rpt_packet() called by main().
 * This function takes a character buffer that has been received as a report
 * from a TSIP device and interprets it.  The character buffer has been
 * assembled using tsip_input_proc() in TSIP_IFC.C.
 *
 * A large case statement directs processing to one of many mid-level
 * functions.  The mid-level functions specific to the current report
 * code passes the report buffer to the appropriate report decoder
 * rpt_0x?? () in TSIP_IFC.C, which outputs the variable values in the
 * report buffer rpt.buf.
 *
 * *************************************************************************
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
/* -- following 2 lines were added by ICognard Dec 10, 2002 -- */
#include <time.h>
#include <sys/time.h>
#include "tsip_ifc.h"
#include "tsipincl.h"

#include "shrmemgps-extern.h"

char
	 *st_baud_text_app [BAUD_TEXT_LEN] =
		{"", "110", "300", "600", "1200",	"2400", "4800", "9600",
			"19200", "38400"},
	 *able_text2[ABLE_TEXT_LEN] 	=
		{"DISABLE", "ENABLE " },
	 *databit_text_app [DATABIT_TEXT_LEN] =
		{"5", "6", "7", "8"},
    *dgps_mode_text[DGPS_MODE_TEXT_LEN] =
      {"Manual GPS (DGPS off)", "Manual DGPS (DGPS Req'd)",
       "Auto DGPS/GPS", "Auto DGPS/GPS", "Query receiver for current mode"},
	 *dgps_mode_text2[DGPS_MODE_TEXT_LEN] =
		{"manual GPS (DGPS off)", "manual DGPS (DGPS req'd)","",
       "auto DGPS/GPS", ""},
	 *dyn_text2 [DYN_TEXT_LEN] =
		{"","land", "sea", "air (default)", "static"},
	 *flow_cntrl_text[FLOW_CNTRL_TEXT_LEN] =
		{"none", "RTS/CTS", "TX XON/XOFF", "TX XANY", "RX XON/XOFF"},
    *foliage_mode_text [FOLIAGE_MODE_TEXT_LEN] =
      {"never","sometimes","always"},
	 *in_out_text[IN_OUT_TEXT_LEN] =
		{"off", "in/out", "in only", "out only"},
    *low_power_text [LOW_POWER_TEXT_LEN] =
    	{"disabled", "auto", "Manual"},
    *meas_rate_text [MEAS_TEXT_LEN] =
    	{"1 Hertz","5 Hertz","10 Hertz"},
	 *parity_text [PARITY_TEXT_LEN] =
		{"NONE", "ODD", "EVEN"},
	 *port_text [PORT_TEXT_LEN] =
		{"COM1", "COM2", ""},
	 *pos_fix_text[POS_FIX_TEXT_LEN] =
		{"automatic","time only (0-D)", "", "horizontal (2-D)",
       "full position (3-D)", "DGPS ref", "clock hold (2-D)",
       "overdetermined clock"},
    *pos_rate_text [POS_RATE_TEXT_LEN] =
    	{"1 Hertz","5 Hertz","10 Hertz","position at measurement rate"},
	 *protocols_text[PROTOCOLS_TEXT_LEN] =
		{"TAIP ", "TSIP ", "NMEA ", "RTCM ", " "},
	 *rcvr_port_text [RCVR_PORT_TEXT_LEN] =
		{"Port 1", "Port 2", "Current Port"},
	 *req_set_text[REQ_SET_TEXT_LEN] =
		{"Request", "Set"},
	 *stopbit_text[STOPBIT_TEXT_LEN] =
		{"1", "2"},
	 *toggle_text[TOGGLE_TEXT_LEN] =
		{"off", "on"},
	 *trackmode_text3[TRACKMODE_TEXT_LEN] =
		{"mode unchanged", "over-determined fix", "weighted over-determined fix"}
;

char str[256];

/* TCHAT uses printf(); TSIPCHAT uses cprintf() */
#ifndef xprintf
#define xprintf printf
#endif

#define TSIP_RPT

#define GOOD_PARSE 0
#define BADID_PARSE 1
#define BADLEN_PARSE 2
short parsed;

void show_crlf (void)
{
        xprintf ("\r\n");
}

/* convert time of week into day-hour-minute-second and print */
void show_time (float time_of_week)
{
	short	days, hours, minutes;
	float seconds;
	double tow;
	const char
		*dname[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

	if (time_of_week == -1.0) {
		xprintf ("   <No time yet>   ");
		return;
	}
	if (time_of_week < 0.0F) time_of_week += 604800.0F;
	if ((time_of_week >= 604800.0) || (time_of_week < 0.0)) {
		xprintf ("     <Bad time>     ");
		return;
	}
	tow = time_of_week;
	days = (short)(tow / 86400.0);
	hours = (short)fmod(tow / 3600., 24.);
	minutes =  (short) fmod(tow/60., 60.);
	seconds = (float)fmod(tow, 60.);
	xprintf (" %s %02d:%02d:%05.2f   ", dname[days], hours, minutes, seconds);
}

/* 0x42 */
static void rpt_single_ECEF_position (TSIPPKT *rpt)
{
	float
		ECEF_pos[3], time_of_fix;

	/* unload rptbuf */
	if (rpt_0x42 (rpt, ECEF_pos, &time_of_fix)) {
		parsed = BADLEN_PARSE;
		return;
	}

	xprintf ("SPos: %15.2f%15.2f%15.2f    ",
		ECEF_pos[0], ECEF_pos[1], ECEF_pos[2]);
	show_time (time_of_fix);
	sprintf(shmgps->answer[0],"SPos: %15.2f%15.2f%15.2f ", ECEF_pos[0], ECEF_pos[1], ECEF_pos[2]);
	shmgps->idxAnswerString=1;
	gpslog(shmgps->answer[0]);
}

/* 0x43 */
static void rpt_single_ECEF_velocity (TSIPPKT *rpt)
{

	float
		ECEF_vel[3], freq_offset, time_of_fix;

	/* unload rptbuf */
	if (rpt_0x43 (rpt, ECEF_vel, &freq_offset, &time_of_fix)) {
		parsed = BADLEN_PARSE;
		return;
	}

	xprintf ("VelECEF: %11.3f%11.3f%11.3f%12.3f    ",
		ECEF_vel[0], ECEF_vel[1], ECEF_vel[2], freq_offset);
	show_time (time_of_fix);
	sprintf(shmgps->answer[0],"VelECEF: %11.3f%11.3f%11.3f%12.3f ", ECEF_vel[0], ECEF_vel[1], ECEF_vel[2], freq_offset);
	shmgps->idxAnswerString=1;
	gpslog(shmgps->answer[0]);
}

/*  0x45  */
static void rpt_SW_version (TSIPPKT *rpt) {
	unsigned char
		major_nav_version, minor_nav_version,
		nav_day, nav_month, nav_year,
		major_dsp_version, minor_dsp_version,
		dsp_day, dsp_month, dsp_year;

	/* unload rptbuf */
	if (rpt_0x45 (rpt,
		&major_nav_version, &minor_nav_version,
		&nav_day, &nav_month, &nav_year,
		&major_dsp_version, &minor_dsp_version,
		&dsp_day, &dsp_month, &dsp_year)) {
		parsed = BADLEN_PARSE;
		return;
	}

	xprintf (
"FW Versions:  Nav Proc %2d.%02d  %2d/%2d/%2d  Sig Proc %2d.%02d  %2d/%2d/%2d",
		major_nav_version, minor_nav_version, nav_day, nav_month, nav_year,
		major_dsp_version, minor_dsp_version, dsp_day, dsp_month, dsp_year);
	sprintf(shmgps->answer[0],"FW Versions:  Nav Proc %2d.%02d  %2d/%2d/%2d  Sig Proc %2d.%02d  %2d/%2d/%2d",
	  major_nav_version, minor_nav_version, nav_day, nav_month, nav_year,
	  major_dsp_version, minor_dsp_version, dsp_day, dsp_month, dsp_year);
	shmgps->idxAnswerString=1;
	gpslog(shmgps->answer[0]);
}

/* 0x47 */
static void rpt_SNR_all_SVs (TSIPPKT *rpt)
{
	unsigned char
		nsvs, sv_prn[12];
	short
		isv;
	float
		snr[12];

	/* unload rptbuf */
	if (rpt_0x47 (rpt, &nsvs, sv_prn, snr)) {
		parsed = BADLEN_PARSE;
		return;
	}

	xprintf ("SNR for satellites: %d", nsvs);
	shmgps->idxAnswerString=0;
	sprintf(shmgps->answer[shmgps->idxAnswerString],"SNR for %d satellites:", nsvs);
	for (isv = 0; isv < nsvs; isv++) {
		show_crlf ();
		xprintf ("   SV %2d   %6.2f", sv_prn[isv], snr[isv]);
		sprintf(str," SV#%02d=%6.2f", sv_prn[isv], snr[isv]);
		if((isv % 4) == 0) {
		  shmgps->idxAnswerString++;
		  sprintf(shmgps->answer[shmgps->idxAnswerString],"\0");
		}
		strcat(shmgps->answer[shmgps->idxAnswerString],str);
	}
	shmgps->idxAnswerString++;
	for(isv=0;isv<shmgps->idxAnswerString;isv++) gpslog(shmgps->answer[isv]);
}

/* 0x49 */
static void rpt_almanac_health_page (TSIPPKT *rpt)
{
	unsigned char
		sv_health[32];

   short
      isv;

	/* unload rptbuf */
	if (rpt_0x49 (rpt, sv_health)) {
		parsed = BADLEN_PARSE;
		return;
	}

	xprintf ("Almananc Health Page:");
	shmgps->idxAnswerString=0;
	sprintf(shmgps->answer[shmgps->idxAnswerString],"Almananc Health Page:");
   for (isv = 0; isv < 32; isv++) {
      if ( isv%3 == 0 )
         show_crlf ();
      if (sv_health[isv] == 0) {
         xprintf ("    SV #%2d : healthy  ",isv+1);
         sprintf(str," SV#%2d=healthy",isv+1);
      } else {
         xprintf ("    SV #%2d : %7X  ",isv+1,sv_health[isv]);
         sprintf(str," SV#%2d=%7X",isv+1,sv_health[isv]);
      }
      if((isv % 4) == 0) {
	shmgps->idxAnswerString++;
	sprintf(shmgps->answer[shmgps->idxAnswerString],"\0");
      }
      strcat(shmgps->answer[shmgps->idxAnswerString],str);
   }
   shmgps->idxAnswerString++;
   for(isv=0;isv<shmgps->idxAnswerString;isv++) gpslog(shmgps->answer[isv]);
}

/* 0x4A */
static void rpt_single_lla_position (TSIPPKT *rpt) {
	short
		lat_deg, lon_deg;
	float
		lat, lon,
		alt, clock_bias, time_of_fix;
	double lat_min, lon_min;
	unsigned char
		north_south, east_west;

	if (rpt_0x4A (rpt,
		&lat, &lon, &alt, &clock_bias, &time_of_fix)) {
		parsed = BADLEN_PARSE;
		return;
	}

	/* convert from radians to degrees */
	lat *= (float)R2D;
	north_south = 'N';
	if (lat < 0.0) {
		north_south = 'S';
		lat = -lat;
	}
	lat_deg = (short)lat;
	lat_min = (lat - lat_deg) * 60.0;

	lon *= (float)R2D;
	east_west = 'E';
	if (lon < 0.0) {
		east_west = 'W';
		lon = -lon;
	}
	lon_deg = (short)lon;
	lon_min = (lon - lon_deg) * 60.0;

	xprintf ("Position:%4d:%06.3f %c%5d:%06.3f %c%10.2f%12.2f",
		lat_deg, lat_min, north_south,
		lon_deg, lon_min, east_west,
		alt, clock_bias);
	sprintf(shmgps->answer[0],"Position:%4d:%06.3f %c%5d:%06.3f %c%10.2f%12.2f",
	  lat_deg, lat_min, north_south, lon_deg, lon_min, east_west, alt, clock_bias);
	shmgps->idxAnswerString=1;
	gpslog(shmgps->answer[0]);
	show_time (time_of_fix);
}

/* 0x4A_2 */
static void rpt_ref_alt (TSIPPKT *rpt) {

	float
		alt, dummy;
	unsigned char
		alt_flag;

	if (rpt_0x4A_2 (rpt,
		&alt, &dummy, &alt_flag)) {
		parsed = BADLEN_PARSE;
		return;
	}

	xprintf ("Reference Alt:   %.1f m;    %s", alt, alt_flag?"ON":"OFF");
	sprintf(shmgps->answer[0],"Reference Alt:   %.1f m;    %s", alt, alt_flag?"ON":"OFF");
	shmgps->idxAnswerString=1;
	gpslog(shmgps->answer[0]);
}

/* 0x4B */
#define MAX_AS1_MESSAGE 4
#define MAX_MID_MESSAGE 41
static void rpt_rcvr_id_and_status (TSIPPKT *rpt)
{
	const char
		*as1_text[MAX_AS1_MESSAGE] = {
		"Synthesizer fault         ",
		"No RTC Timeset at power-up",
		"A-to-D converter fault    ",
		"Almanac not complete      "} ;

	unsigned char
		machine_id, status_1, status_2;
	short
		imsg;

	/* unload rptbuf */
	if (rpt_0x4B (rpt, &machine_id, &status_1, &status_2)) {
		parsed = BADLEN_PARSE;
		return;
	}

	xprintf ("Machine/Code ID: %d   Status: %d %d", machine_id,
		status_1, status_2);
	for (imsg = 0; imsg < MAX_AS1_MESSAGE; imsg++) {
		if (status_1 & (1 << imsg)) {
			show_crlf ();
			xprintf ("               %s", as1_text[imsg]);
		}
	}
	if (status_2 & 0x01) {
		xprintf ("        Superpackets supported");
	}
}

/* 0x4F */
static void rpt_UTC_parameters (TSIPPKT *rpt)
{
	float
		a1, time_of_data;
   double
      a0;
   short
      dt_ls, wn_t, wn_lsf, dn, dt_lsf;

	/* unload rptbuf */
	if (rpt_0x4F (rpt, &a0, &a1, &dt_ls, &time_of_data,
                 &wn_t, &wn_lsf, &dn, &dt_lsf)) {
		parsed = BADLEN_PARSE;
		return;
	}

   xprintf ("UTC Parameters:"); show_crlf ();
	xprintf ("   A_0         : %g ", a0); show_crlf ();
   xprintf ("   A_1         : %g ", a1); show_crlf ();
   xprintf ("   delta_T_LS  : %d ", dt_ls); show_crlf ();
   xprintf ("   T_OT        : %f ", time_of_data); show_crlf ();
   xprintf ("   WN_T        : %d ", wn_t); show_crlf ();
   xprintf ("   WN_LSF      : %d ", wn_lsf); show_crlf ();
   xprintf ("   DN          : %d ", dn); show_crlf ();
   xprintf ("   delta_T_LSF : %d ", dt_lsf); show_crlf ();
}

/* 0x55 */
static void rpt_io_opt (TSIPPKT *rpt)
{
	unsigned char
		pos_code, vel_code, time_code, aux_code;

	/* unload rptbuf */
	if (rpt_0x55 (rpt,
		&pos_code, &vel_code, &time_code, &aux_code)) {
		parsed = BADLEN_PARSE;
		return;
	}
	/* rptbuf unloaded */

	xprintf ("I/O Options:%2X%2X%2X%2X",
		pos_code, vel_code, time_code, aux_code);

	if (pos_code & 0x01) {
		show_crlf ();
		xprintf ("    ECEF XYZ position output");
	}
	if (pos_code & 0x02) {
		show_crlf ();
		xprintf ("    LLA position output");
	}
	show_crlf ();
	xprintf ((pos_code & 0x04)?
		"    MSL altitude output (Geoid height) ":
		"    WGS-84 altitude output");
	show_crlf ();
	xprintf ((pos_code & 0x08)?
		"    MSL altitude input":"    WGS-84 altitude input");
	show_crlf ();
	xprintf ((pos_code & 0x10)?
		"    Double precision":"    Single precision");
	if (pos_code & 0x20) {
		show_crlf ();
		xprintf ("   All Enabled Superpackets");
	}
	if (vel_code & 0x01) {
		show_crlf ();
		xprintf ("    ECEF XYZ velocity output");
	}
	if (vel_code & 0x02) {
		show_crlf ();
		xprintf ("    ENU velocity output");
	}
	show_crlf ();
	xprintf ((time_code & 0x01)?
		  "    Time tags in UTC":"    Time tags in GPS time");
	if (time_code & 0x02) {
		show_crlf ();
		xprintf ("    Fixes delayed to integer seconds");
	}
	if (time_code & 0x04) {
		show_crlf ();
		xprintf ("    Fixes sent only on request");
	}
	if (time_code & 0x08) {
		show_crlf ();
		xprintf ("    Synchronized measurements");
	}
	if (time_code & 0x10) {
		show_crlf ();
		xprintf ("    Minimize measurement propagation");
	}
	if (aux_code & 0x01) {
		show_crlf ();
		xprintf ("    Raw measurement output");
	}
	if (aux_code & 0x02) {
		show_crlf ();
		xprintf ("    Code-phase smoothed before output");
	}
	if (aux_code & 0x04) {
		show_crlf ();
		xprintf ("    Additional fix status");
	}
	show_crlf ();
	xprintf ("    Signal Strength Output as %s",
		(aux_code & 0x08)? "dBHz" : "AMU");
}

/* 0x56 */
static void rpt_ENU_velocity (TSIPPKT *rpt)
{
	float
		vel_ENU[3], freq_offset, time_of_fix;

	/* unload rptbuf */
	if (rpt_0x56 (rpt, vel_ENU, &freq_offset, &time_of_fix)) {
		parsed = BADLEN_PARSE;
		return;
	}

	xprintf ("Vel ENU:%11.2f%11.2f%11.2f%12.2f    ",
		vel_ENU[0], vel_ENU[1], vel_ENU[2], freq_offset);
	show_time (time_of_fix);
}

/* 0x57 */
#define MAX_SOURCE_MESSAGE 9
static void rpt_last_fix_info (TSIPPKT *rpt)
{
	unsigned char
		source_code, diag_code;
	short
		week_num;
	float
		time_of_fix;
	const char
		*source_text[MAX_SOURCE_MESSAGE] = {
		"temporary no fix",
		"good current fix",
		"converging fix",
		"",
		"shadow fix",
		"user-supplied coarse fix",
		"user-supplied accurate fix",
		"image fix",
		"no fix available"} ;


	/* unload rptbuf */
	if (rpt_0x57 (rpt, &source_code, &diag_code, &week_num, &time_of_fix)) {
		parsed = BADLEN_PARSE;
		return;
	}

	xprintf ("%s;   diag code: %2Xh", source_text[source_code], diag_code);
	show_crlf ();
	xprintf ("Time of last fix:");
	show_time (time_of_fix);
	show_crlf ();
	xprintf ("Week of last fix: %d", week_num);
}

/* 0x58 */
#define MAX_DAT_MESSAGE 7
static void rpt_GPS_system_data (TSIPPKT *rpt)
{
	unsigned char
		op_code, data_type, sv_prn,
		data_length, data_packet[250];
	ALM_INFO
		*almanac;
	ALH_PARMS
		*almh;
	UTC_INFO
		*utc;
	ION_INFO
		*ionosphere;
	EPHEM_CLOCK
		*cdata;
	EPHEM_ORBIT
		*edata;
	NAV_INFO
		*nav_data;
	unsigned char
		curr_t_oa;
	short
		curr_wn_oa;
	const char
		*datname[MAX_DAT_MESSAGE] =
		{"", "", "Almanac",
		"Health Page, T_oa, WN_oa", "Ionosphere", "UTC ",
		"Ephemeris"};

	/* unload rptbuf */
	if (rpt_0x58 (rpt, &op_code, &data_type, &sv_prn,
		&data_length, data_packet)) {
		parsed = BADLEN_PARSE;
		return;
	}

	xprintf ("System data [%d]:  %s  SV%2.2d",
		data_type, datname[data_type], sv_prn);
	switch (op_code)
	{
	case 1:
		xprintf ("  Acknowledged");
		show_crlf ();
		break;
	case 2:
		xprintf ("  length = %d bytes", data_length);
		show_crlf ();
		switch (data_type) {
		case 2:
			/* Almanac */
			if (sv_prn == 0 || sv_prn > 32) {
				xprintf ("Binary PRN invalid");
				return;
			}
			almanac = (ALM_INFO*)data_packet;
			xprintf ("t_oa_raw = %12d  ", almanac->t_oa_raw );
			xprintf ("SV_hlth  = %12d  ", almanac->SV_health );
			show_crlf ();
			xprintf ("e        = % -12g  ", almanac->e        );
			xprintf ("t_oa     = % -12g  ", almanac->t_oa     );
			show_crlf ();
			xprintf ("i_0      = % -12g  ", almanac->i_0      );
			xprintf ("OMEGADOT = % -12g  ", almanac->OMEGADOT );
			show_crlf ();
			xprintf ("sqrt_A   = % -12g  ", almanac->sqrt_A   );
			xprintf ("OMEGA_0  = % -12g  ", almanac->OMEGA_0  );
			show_crlf ();
			xprintf ("omega    = % -12g  ", almanac->omega    );
			xprintf ("M_0      = % -12g  ", almanac->M_0      );
			show_crlf ();
			xprintf ("a_f0     = % -12g  ", almanac->a_f0     );
			xprintf ("a_f1     = % -12g  ", almanac->a_f1     );
			show_crlf ();
			xprintf ("Axis     = % -12g  ", almanac->Axis     );
			xprintf ("n        = % -12g  ", almanac->n        );
			show_crlf ();
			xprintf ("OMEGA_n  = % -12g  ", almanac->OMEGA_n  );
			xprintf ("ODOT_n   = % -12g  ", almanac->ODOT_n   );
			show_crlf ();
			xprintf ("t_zc     = % -12g  ", almanac->t_zc     );
			xprintf ("weeknum  = %12d  ", almanac->weeknum  );
			show_crlf ();
			xprintf ("wn_oa    = %12d", almanac->wn_oa    );
			break;

		case 3:
			/* Almanac health page */
			almh = (ALH_PARMS*)data_packet;
			xprintf ("t_oa = %d, wn_oa&0xFF = %d  ", almh->t_oa, almh->WN_a);
			show_crlf ();
			xprintf ("%02X %02X %02X %02X %02X %02X %02X %02X  ",
				almh->SV_health[0], almh->SV_health[1], almh->SV_health[2],
				almh->SV_health[3], almh->SV_health[4], almh->SV_health[5],
				almh->SV_health[6], almh->SV_health[7]);
			show_crlf ();
			xprintf ("%02X %02X %02X %02X %02X %02X %02X %02X  ",
				almh->SV_health[8], almh->SV_health[9], almh->SV_health[10],
				almh->SV_health[11], almh->SV_health[12], almh->SV_health[13],
				almh->SV_health[14], almh->SV_health[15]);
			show_crlf ();
			xprintf ("%02X %02X %02X %02X %02X %02X %02X %02X  ",
				almh->SV_health[16], almh->SV_health[17], almh->SV_health[18],
				almh->SV_health[19], almh->SV_health[20], almh->SV_health[21],
				almh->SV_health[22], almh->SV_health[23]);
			show_crlf ();
			xprintf ("%02X %02X %02X %02X %02X %02X %02X %02X  ",
				almh->SV_health[24], almh->SV_health[25], almh->SV_health[26],
				almh->SV_health[27], almh->SV_health[28], almh->SV_health[29],
				almh->SV_health[30], almh->SV_health[31]);
			show_crlf ();
			curr_t_oa = data_packet[34];
			curr_wn_oa = (data_packet[35]<<8) + data_packet[36];
			xprintf ("current t_oa = %d, wn_oa = %d  ", curr_t_oa, curr_wn_oa);
			show_crlf();
			break;

		case 4:
			/* Ionosphere */
			ionosphere = (ION_INFO*)data_packet;
			xprintf ("alpha_0 = % -12g ", ionosphere->alpha_0);
			xprintf ("alpha_1 = % -12g ", ionosphere->alpha_1);
			show_crlf ();
			xprintf ("alpha_2 = % -12g ", ionosphere->alpha_2);
			xprintf ("alpha_3 = % -12g ", ionosphere->alpha_3);
			show_crlf ();
			xprintf ("beta_0  = % -12g  ", ionosphere->beta_0);
			xprintf ("beta_1  = % -12g  ", ionosphere->beta_1);
			show_crlf ();
			xprintf ("beta_2  = % -12g  ", ionosphere->beta_2);
			xprintf ("beta_3  = % -12g  ", ionosphere->beta_3);
			show_crlf ();
			break;

		case 5:
			/* UTC */
			show_crlf ();
			utc = (UTC_INFO*)data_packet;
			xprintf ("A_0        = %g  ", utc->A_0);
			show_crlf ();
			xprintf ("A_1        = %g  ", utc->A_1);
			show_crlf ();
			xprintf ("delta_t_LS = %d  ", utc->delta_t_LS);
			show_crlf ();
			xprintf ("t_ot       = %f  ", utc->t_ot );
			show_crlf ();
			xprintf ("WN_t       = %d  ", utc->WN_t );
			show_crlf ();
			xprintf ("WN_LSF     = %d  ", utc->WN_LSF );
			show_crlf ();
			xprintf ("DN         = %d  ", utc->DN );
			show_crlf ();
			xprintf ("delta_t_LSF = %d  ", utc->delta_t_LSF );
			break;

		case 6: /* Ephemeris */
			show_crlf ();
			if (sv_prn == 0 || sv_prn > 32) {
				xprintf ("Binary PRN invalid");
				show_crlf ();
				return;
			}
			nav_data = (NAV_INFO*)data_packet;

			xprintf ("   SV_PRN = % -12d . ", nav_data->sv_number );
			xprintf ("  t_ephem = % -12g . ", nav_data->t_ephem );
			show_crlf ();
			cdata = &(nav_data->ephclk);
			xprintf ("  weeknum = % -12d . ", cdata->weeknum );
			xprintf ("   codeL2 = % -12d . ", cdata->codeL2 );
			xprintf ("  L2Pdata = % -12d", cdata->L2Pdata );
			show_crlf ();
			xprintf ("SVacc_raw = % -12d . ", cdata->SVacc_raw );
			xprintf ("SV_health = % -12d . ", cdata->SV_health );
			xprintf ("     IODC = % -12d", cdata->IODC );
			show_crlf ();
			xprintf ("     T_GD = % -12g . ", cdata->T_GD );
			xprintf ("     t_oc = % -12g . ", cdata->t_oc );
			xprintf ("     a_f2 = % -12g", cdata->a_f2 );
			show_crlf ();
			xprintf ("     a_f1 = % -12g . ", cdata->a_f1 );
			xprintf ("     a_f0 = % -12g . ", cdata->a_f0 );
			xprintf ("    SVacc = % -12g", cdata->SVacc );
			show_crlf ();
			edata = &(nav_data->ephorb);
			xprintf ("     IODE = % -12d . ", edata->IODE );
			xprintf ("fit_intvl = % -12d . ", edata->fit_interval );
			xprintf ("     C_rs = % -12g", edata->C_rs );
			show_crlf ();
			xprintf ("  delta_n = % -12g . ", edata->delta_n );
			xprintf ("      M_0 = % -12g . ", edata->M_0 );
			xprintf ("     C_uc = % -12g", edata->C_uc );
			show_crlf ();
			xprintf ("      ecc = % -12g . ", edata->e );
			xprintf ("     C_us = % -12g . ", edata->C_us );
			xprintf ("   sqrt_A = % -12g", edata->sqrt_A );
			show_crlf ();
			xprintf ("     t_oe = % -12g . ", edata->t_oe );
			xprintf ("     C_ic = % -12g . ", edata->C_ic );
			xprintf ("  OMEGA_0 = % -12g", edata->OMEGA_0 );
			show_crlf ();
			xprintf ("     C_is = % -12g . ", edata->C_is );
			xprintf ("      i_0 = % -12g . ", edata->i_0 );
			xprintf ("     C_rc = % -12g", edata->C_rc );
			show_crlf ();
			xprintf ("    omega = % -12g . ", edata->omega );
			xprintf (" OMEGADOT = % -12g . ", edata->OMEGADOT );
			xprintf ("     IDOT = % -12g", edata->IDOT );
			show_crlf ();
			xprintf ("     Axis = % -12g . ", edata->Axis );
			xprintf ("        n = % -12g . ", edata->n );
			xprintf ("    r1me2 = % -12g", edata->r1me2 );
			show_crlf ();
			xprintf ("  OMEGA_n = % -12g . ", edata->OMEGA_n );
			xprintf ("   ODOT_n = % -12g", edata->ODOT_n );
			break;
		}
	}
}


/* 0x59: */
static void rpt_SVs_enabled (TSIPPKT *rpt)
{
	unsigned char
		code_type, status_code[32];
	short
		iprn;

	/* unload rptbuf */
	if (rpt_0x59 (rpt, &code_type, status_code)) {
		parsed = BADLEN_PARSE;
		return;
	}

	if (code_type == 3) {
		xprintf ("SVs Enable/Disable");
		show_crlf ();

		for (iprn=0; iprn<32; iprn++) {
			xprintf ("%c%c", (iprn%10) ? ' ' : '.',
				status_code[iprn] ? 'D' : 'E');
		}
	}
	if (code_type == 6) {
		xprintf ("SVs Heed/Ignore Health");
		show_crlf ();

		for (iprn=0; iprn<32; iprn++) {
			xprintf ("%c%c", (iprn%10) ? ' ' : '.',
				status_code[iprn] ? 'I' : 'H');
		}
	}
}

/* 0x6D */
static void rpt_allSV_selection (TSIPPKT *rpt)
{
	unsigned char
		manual_mode, nsvs, ndim;
	short
		islot;
	float
		pdop, hdop, vdop, tdop;
   char
   	sv_prn[8];

	/* unload rptbuf */
	if (rpt_0x6D (rpt,
		&manual_mode, &nsvs, &ndim, sv_prn,
		&pdop, &hdop, &vdop, &tdop)) {
		parsed = BADLEN_PARSE;
		return;
	}

	xprintf ("Mode: %s  %d SV (%d-D):",
		manual_mode ? "Manual" : "Auto",
		nsvs, ndim - 1);
	for (islot = 0; islot < nsvs; islot++) {
		if (sv_prn[islot]) xprintf (" %2d", sv_prn[islot]);
	}
	show_crlf ();
	xprintf ("      P, H, V, TDOP = %.2f, %.2f, %.2f, %.2f",
		pdop, hdop, vdop, tdop);
}

/* 0x70 */
void show_pv_operation(TSIPPKT *rpt)
{
	unsigned char
               dyn_switch,
               static_switch,
               alt_switch,
               extra;

	if (rpt_0x70(rpt, &dyn_switch, &static_switch, &alt_switch, &extra)) {
		parsed = BADLEN_PARSE;
		return;
	}

	/* If dynamic filter is off, then static filter cannot run, so just
	   inform user that the entire PV filter is disabled
       */
	if (dyn_switch == 0) {
		
		xprintf("PV filter disabled");
		show_crlf();
	}
	else {
		xprintf("PV dynamic filter %s", dyn_switch?"enabled":"disabled");
		show_crlf();
		xprintf("PV static filter %s", static_switch?"enabled":"disabled");
		show_crlf();
	}
	xprintf("Altitude filter %s", alt_switch?"enabled":"disabled");
}

/* 0x83 */
static void rpt_double_ECEF_position (TSIPPKT *rpt)
{

	double
		ECEF_pos[3], clock_bias;
	float
		time_of_fix;

	/* unload rptbuf */
	if (rpt_0x83 (rpt, ECEF_pos, &clock_bias, &time_of_fix)) {
		parsed = BADLEN_PARSE;
		return;
	}

	xprintf ("DPos:%12.2f %13.2f %13.2f %12.2f",
		ECEF_pos[0], ECEF_pos[1], ECEF_pos[2], clock_bias);
	show_time (time_of_fix);
}

/* 0x84 */
static void rpt_double_lla_position (TSIPPKT *rpt)
{
	short
		lat_deg, lon_deg;
	double
		lat, lon, lat_min, lon_min,
		alt, clock_bias;
	float
		time_of_fix;
	unsigned char
		north_south, east_west;

	/* unload rptbuf */
	if (rpt_0x84 (rpt,
		&lat, &lon, &alt, &clock_bias, &time_of_fix)) {
		parsed = BADLEN_PARSE;
		return;
	}

	lat *= R2D;
	lon *= R2D;
	if (lat < 0.0) {
		north_south = 'S';
		lat = -lat;
	} else {
		north_south = 'N';
	}
	lat_deg = (short)lat;
	lat_min = (lat - lat_deg) * 60.0;

	if (lon < 0.0) {
		east_west = 'W';
		lon = -lon;
	} else {
		east_west = 'E';
	}
	lon_deg = (short)lon;
	lon_min = (lon - lon_deg) * 60.0;
	xprintf ("DPos: %2d:%08.5f %c; %3d:%08.5f %c; %10.2f %12.2f",
		lat_deg, lat_min, north_south,
		lon_deg, lon_min, east_west,
		alt, clock_bias);
	show_time (time_of_fix);
}

/* 0x8F15 */
static void rpt_8F15 (TSIPPKT *rpt)
/* Datum parameters */
{
	double
		datum_coeffs[5];
	short
		datum_idx;

	/* unload rptbuf */
	if (rpt_0x8F15 (rpt, &datum_idx, datum_coeffs)) {
		parsed = BADLEN_PARSE;
		return;
	}

	xprintf ("Datum Index %2d ", datum_idx);
	show_crlf ();
	xprintf ("    dx     dy     dz       a-axis          e-squared");
	show_crlf ();
	xprintf("%6.1f %6.1f %6.1f    %10.3f    %16.14f", datum_coeffs[0], datum_coeffs[1],
			datum_coeffs[2], datum_coeffs[3],datum_coeffs[4]);
}

#define INFO_DGPS       0x02
#define INFO_2D         0x04
#define INFO_ALTSET     0x08
#define INFO_FILTERED   0x10
static void rpt_8F20 (TSIPPKT *rpt)
{
	unsigned char
		subpacket_id, info, nsvs, *buf, sv_prn[32];
	short
		week_num, datum_index, sv_IODC[32];
	double
		lat, lon, alt, time_of_fix;
	double
		londeg, latdeg, vel[3];
	short
		isv;

	buf = rpt->buf;
	/* unload rptbuf */
	if (rpt_0x8F20 (rpt,
		&subpacket_id, &info, &lat, &lon, &alt, vel,
		&time_of_fix,
		&week_num, &nsvs, sv_prn, sv_IODC, &datum_index))
	{
		parsed = BADLEN_PARSE;
		return;
	}
	/* convert from radians to degrees */
	latdeg = R2D * fabs(lat);
	londeg = R2D * fabs(lon);
	xprintf ("SPkt 8F%c20: %02d:%02d:%05.2f %4d:%09.6f %c %5d:%09.6f %c %10.2f ",
		 buf[1]?'*':'-',
		(short)fmod(time_of_fix/3600., 24.),
		(short)fmod(time_of_fix/60., 60.),
		fmod(time_of_fix, 60.),
		(short)latdeg, fmod (latdeg, 1.)*60.0,
		(lat<0.0)?'S':'N',
		(short)londeg, fmod (londeg, 1.)*60.0,
		(lon<0.0)?'W':'E',
		alt);
	show_crlf ();

	xprintf ("*   *%10.3f E %10.3f N %10.3f U         %s%s%s    Datum%3d",
		vel[0], vel[1], vel[2],
		(info & INFO_DGPS)?"Diff-":"",
		(info & INFO_2D)?((info&INFO_ALTSET)?"2D-AltSet":"2D-AltHold"):"3D",
		(info & INFO_FILTERED)?"-Filt":"",
		datum_index);
	show_crlf ();

	xprintf ("*   *SVs");
	for (isv = 0; isv < nsvs; isv++) {
		xprintf (" %02d", sv_prn[isv]);
	}
	xprintf ("          ( IODEs ");
	for (isv = 0; isv < nsvs; isv++) {
		xprintf ("%02X ", sv_IODC[isv]&0xFF);
	}
	xprintf (")");
}

static void rpt_8F41 (TSIPPKT *rpt)
/* show manufacturing parameters */
{
	unsigned char
		subpacket_id, year, month, day, hour; /* of build */
   short
      serial_num_prefix, test_code_id_num;
   float
   	serial_num, oscillator_offset;

	if (rpt_0x8F41 (rpt, &subpacket_id, &serial_num_prefix,
   					 &serial_num, &year, &month, &day, &hour,
                   &oscillator_offset, &test_code_id_num)) {
		parsed = BADLEN_PARSE;
		return;
	}

	xprintf ("Manufacturing Operating Parameters:"); show_crlf ();
   xprintf ("  Board serial number prefix: %d", serial_num_prefix); show_crlf ();
   xprintf ("  Board serial number: %.2f", serial_num); show_crlf ();
   xprintf ("  Year of build: %d", year); show_crlf ();
   xprintf ("  Month of build: %d", month); show_crlf ();
   xprintf ("  Day of build: %d", day); show_crlf ();
   xprintf ("  Hour of build: %d", hour); show_crlf ();
   xprintf ("  Oscillator offset: %.2f", oscillator_offset); show_crlf ();
   xprintf ("  Test code id number: %d", test_code_id_num);
}

static void rpt_8F42 (TSIPPKT *rpt)
/* show production parameters */
{
	unsigned char
		subpacket_id, prod_opt_prefix, prod_num_extension; /* of build */
   short
      case_serial_num_prefix, machine_ID_num;
   unsigned long
   	case_serial_num, production_num;

	if (rpt_0x8F42 (rpt, &subpacket_id, &prod_opt_prefix,
   					 &prod_num_extension, &case_serial_num_prefix,
                   &case_serial_num, &production_num,
                   &machine_ID_num)) {
		parsed = BADLEN_PARSE;
		return;
	}

	xprintf ("Production Parameters:"); show_crlf ();
   xprintf ("  Production options prefix: %d", prod_opt_prefix); show_crlf ();
   xprintf ("  Production number extension: %d", prod_num_extension); show_crlf ();
   xprintf ("  Case serial number prefix: %d", case_serial_num_prefix); show_crlf ();
   xprintf ("  Case serial number: %lu", case_serial_num); show_crlf ();
   xprintf ("  Production number: %lu", production_num); show_crlf ();
   xprintf ("  Machine ID number: %d", machine_ID_num); show_crlf ();
}

static void rpt_8F4A (TSIPPKT *rpt)
/* show PPS characteristics */
{
	unsigned char
		subpacket_id, PPS_driver_switch, PPS_time_base, PPS_polarity;
   double
   	PPS_offset;
   float
   	bias_uncert_threshold;

	if (rpt_0x8F4A (rpt, &subpacket_id, &PPS_driver_switch,
   					 &PPS_time_base, &PPS_polarity,
                   &PPS_offset, &bias_uncert_threshold)) {
		parsed = BADLEN_PARSE;
		return;
	}

	xprintf ("PPS charateristics:"); show_crlf ();
   xprintf ("  PPS driver switch: %s", PPS_driver_switch ? "on" :
   			"off"); show_crlf ();
   xprintf ("  PPS time base: ");
   switch (PPS_time_base) {
   	case 0:
      	xprintf ("GPS");
         break;
      case 1:
      	xprintf ("UTC");
         break;
      case 2:
      	xprintf ("USER");
         break;
   }
   show_crlf ();
   xprintf ("  PPS polarity: %s", PPS_polarity ? "negative" :
   			"positve"); show_crlf ();
   xprintf ("  PPS offset: %.2f", PPS_offset); show_crlf ();
   xprintf ("  Bias uncertainty threshold: %.2f", bias_uncert_threshold);
   show_crlf ();
}

static void rpt_8F4B (TSIPPKT *rpt)
/* show survey limit */
{
	unsigned char
		subpacket_id;
   unsigned long
   	survey_limit;

	if (rpt_0x8F4B (rpt, &subpacket_id, &survey_limit)) {
		parsed = BADLEN_PARSE;
		return;
	}

	xprintf ("Auto-survey limit: %lu", survey_limit);
}

static void rpt_8FAB (TSIPPKT *rpt)
/* Primary Timing Information */
{
unsigned char subpacket_id, timing_flag, seconds, minutes, hours,
      day_of_month, month;
unsigned long time_of_week;
unsigned short week_number, UTC_offset, year;
struct timeval time2set;
struct tm tm2set,*dt;
long timegps,timehost;
int diff;
double diffd;

if (rpt_0x8FAB (rpt, &subpacket_id, &time_of_week, &week_number, &UTC_offset, &timing_flag, &seconds,
                 &minutes, &hours, &day_of_month, &month, &year)) {
  parsed = BADLEN_PARSE;
  return;
}
/* -- Fill the shared memory segment GPS -- */

/* -- this is where we should set the computer clock to GPS one -- */

tm2set.tm_sec=seconds; tm2set.tm_min=minutes; tm2set.tm_hour=hours;
tm2set.tm_mday=day_of_month; tm2set.tm_mon=month-1; tm2set.tm_year=year-1900;
tm2set.tm_isdst=0;
timegps=timegm(&tm2set)-UTC_offset;
gettimeofday(&time2set,NULL);
timehost=time2set.tv_sec;
//xprintf("GPS time: %i seconds.", timegps);
//xprintf("CPU time: %i seconds.",timehost);
diff=abs(timegps-timehost);
diffd=fabsf( ((double)timegps+0.020) - (timehost+time2set.tv_usec/1.0e6) );
shmgps->diffCPU_GPS=diffd;

xprintf ("THUNDERBOLT DATE/TIME %02d:%02d:%02d %5d/%d/%hu  (GPS UTC offset=%hu s) \n", hours, minutes, seconds, month, day_of_month, year, UTC_offset);
sprintf(gpslinelog,"ThunderBolt GPS Date/Time %5d/%d/%hu  %02d:%02d:%02d  UTC_offset=%hu s", month, day_of_month, year, hours, minutes, seconds, UTC_offset);
sprintf(gpslinelog,"diff_CPU_GPS= %.6lf sec",diffd); gpslog(gpslinelog);
if(strcmp(shmgps->s0x8FAB_3,gpslinelog) != 0) {
  strcpy(shmgps->s0x8FAB_3,gpslinelog);
  gpslog(gpslinelog);
}

//xprintf ("time of week: %lu  week number: %hu  UTC offset %hu", time_of_week, week_number, UTC_offset); show_crlf ();
//sprintf(shmgps->s0x8FAB_1,"time of week: %lu  week number: %hu  UTC offset %hu",  time_of_week, week_number, UTC_offset); 

xprintf ("Flags: ");
xprintf ((timing_flag & 0x01)?  " UTC time,": " GPS time,");
xprintf ((timing_flag & 0x02)?  " UTC PPS,": " GPS PPS,");
xprintf ((timing_flag & 0x04)?  " Time not set,": " Time set,");
xprintf ((timing_flag & 0x08)?  " Don't have UTC,": " Have UTC info,");
xprintf ((timing_flag & 0x16)?  " Time from user": " Time from GPS");  show_crlf ();

sprintf(gpslinelog,"Flags: ");
sprintf(str,(timing_flag & 0x01)?  " UTC time,": " GPS time,"); strcat(gpslinelog,str);
sprintf(str,(timing_flag & 0x02)?  " UTC PPS,": " GPS PPS,"); strcat(gpslinelog,str);
sprintf(str,(timing_flag & 0x04)?  " Time not set,": " Time set,"); strcat(gpslinelog,str);
sprintf(str,(timing_flag & 0x08)?  " Don't have UTC,": " Have UTC info,"); strcat(gpslinelog,str);
sprintf(str,(timing_flag & 0x16)?  " Time from user": " Time from GPS"); strcat(gpslinelog,str);
if(strcmp(shmgps->s0x8FAB_2,gpslinelog) != 0) {
  strcpy(shmgps->s0x8FAB_2,gpslinelog);
  gpslog(gpslinelog);
}

if( strstr(gpslinelog,"Time set") && strstr(gpslinelog,"Have UTC info") && strstr(gpslinelog,"Time from GPS") ) {
 if(diffd > 0.2) {
  time2set.tv_usec=20000;		/* 20ms probably the mean time needed for serial transfer -- */
  time2set.tv_sec=timegps;
  settimeofday(&time2set,NULL);
  printf(" Difference timegps-timehost > 0.2sec -> Set new DATE/TIME\n");
  sprintf(gpslinelog," Difference timegps-timehost > 0.2sec -> Set new DATE/TIME"); gpslog(gpslinelog);
 }
}

sprintf(gpslinelog,"ThunderBolt GPS Date/Time %5d/%d/%hu  %02d:%02d:%02d  UTC_offset=%hu",
  month, day_of_month, year, hours, minutes, seconds, UTC_offset);
if(strcmp(shmgps->s0x8FAB_3,gpslinelog) != 0) {
  strcpy(shmgps->s0x8FAB_3,gpslinelog);
  gpslog(gpslinelog);
}

}

#define MAX_CRITICAL_ALARMS 5
#define MAX_MINOR_ALARMS 9
static void rpt_8FAC (TSIPPKT *rpt)
/* Supplemental Timing Information */
{
	unsigned char
   	subpacket_id, receiver_mode, disciplining_mode, survey_progress,
      GPS_decoding_status, disciplining_activity, spare_stat1,
      spare_stat2;
   unsigned long
   	holdover_duration, DAC_value;
	unsigned short
		critical_alarms, minor_alarms;
   float
   	PPS_quality, MHz_quality, DAC_voltage, temp;
   double
   	latitude, longitude, altitude;
   short
   	imsg, cnt;
   const char
   	*critical_alarm_text[MAX_CRITICAL_ALARMS] = {
			"ROM checksum error",
		  	"RAM check has failed",
			"FPGA check has failed",
			"Power supply faiure",
      	"Oscillator control voltage at rail"},
      *minor_alarm_text[MAX_MINOR_ALARMS] = {
      	"Oscillator control voltage near rail",
         "Antenna open",
         "Antenna shorted",
         "Not tracking satellites",
         "Not disciplining oscillator",
         "Survey in progress",
         "No stored position",
         "Leap second pending",
         "In test mode"} ;

   char str[64];

	if (rpt_0x8FAC (rpt, &subpacket_id, &receiver_mode, &disciplining_mode,
   					 &survey_progress, &holdover_duration, &critical_alarms,
                   &minor_alarms, &GPS_decoding_status, &disciplining_activity,
                   &spare_stat1, &spare_stat2, &PPS_quality, &MHz_quality,
                   &DAC_value, &DAC_voltage, &temp, &latitude, &longitude,
                   &altitude)) {
		parsed = BADLEN_PARSE;
		return;
	}

   xprintf ("Receiver mode: ");
   sprintf(gpslinelog,"Receiver mode: ");
   switch (receiver_mode) {
	case 0x00:
		xprintf ("Automatic (2D/3D)");
		strcat(gpslinelog,"Automatic (2D/3D)");
		break;
   case 0x01:
		xprintf ("Single Sat (Time)");
		strcat(gpslinelog,"Single Sat (Time)");
		break;
   case 0x03:
		xprintf ("Horizontal (2D)");
		strcat(gpslinelog,"Horizontal (2D)");
		break;
   case 0x04:
		xprintf ("Full Position 3D");
		strcat(gpslinelog,"Horizontal (2D)");
		break;
   case 0x05:
		xprintf ("DGPS Reference");
		strcat(gpslinelog,"DGPS Reference");
		break;
   case 0x06:
		xprintf ("Clock Hold (2D)");
		strcat(gpslinelog,"Clock Hold (2D)");
		break;
   case 0x07:
		xprintf ("Overdetermined Clock");
		strcat(gpslinelog,"Overdetermined Clock");
		break;
   }
   show_crlf ();
   if(strcmp(shmgps->s0x8FAC_1,gpslinelog) != 0) {
     strcpy(shmgps->s0x8FAC_1,gpslinelog); gpslog(gpslinelog);
   }

   xprintf ("Disciplining mode: ");
   sprintf(gpslinelog,"Disciplining mode: ");
   switch (disciplining_mode) {
	case 0x00:
		xprintf ("Normal");
		strcat(gpslinelog,"Normal");
		break;
   case 0x01:
		xprintf ("Power-up");
		strcat(gpslinelog,"Power-up");
		break;
   case 0x02:
		xprintf ("Auto Holdover");
		strcat(gpslinelog,"Auto Holdover");
		break;
   case 0x03:
		xprintf ("Manual Holdover");
		strcat(gpslinelog,"Manual Holdover");
		break;
   case 0x04:
		xprintf ("Recovery");
		strcat(gpslinelog,"Recovery");
		break;
   case 0x05:
		xprintf ("Fast Recovery");
		strcat(gpslinelog,"Fast Recovery");
		break;
   case 0x06:
		xprintf ("Disabled");
		strcat(gpslinelog,"Disabled");
		break;
   }
   show_crlf ();
   if(strcmp(shmgps->s0x8FAC_2,gpslinelog) != 0) {
     strcpy(shmgps->s0x8FAC_2,gpslinelog); gpslog(gpslinelog);
   }

   xprintf ("Self-Survey Progress: %d%%    Holdover Duration: %lu ",
   			survey_progress, holdover_duration);  show_crlf ();
   sprintf(gpslinelog,"Self-Survey Progress: %d%%    Holdover Duration: %lu ",
   			survey_progress, holdover_duration);
   if(strcmp(shmgps->s0x8FAC_3,gpslinelog) != 0) {
     strcpy(shmgps->s0x8FAC_3,gpslinelog); gpslog(gpslinelog);
   }

   cnt = 0;
   xprintf ("Critical Alarms:");
   sprintf(gpslinelog,"Critical Alarms:");
   for (imsg = 0; imsg < MAX_CRITICAL_ALARMS; imsg++) {
		if (critical_alarms & (1 << imsg)) {
      	if (cnt == 2) {
         	show_crlf ();
            cnt = 0;
         }
			xprintf (" %s ", critical_alarm_text[imsg]);
			sprintf(str," %s ", critical_alarm_text[imsg]); strcat(gpslinelog,str);
         cnt++;
		}
	}
   show_crlf ();
   if(strcmp(shmgps->s0x8FAC_4,gpslinelog) != 0) {
     strcpy(shmgps->s0x8FAC_4,gpslinelog); gpslog(gpslinelog);
   }

   cnt = 0;
   xprintf ("Minor Alarms:");
   sprintf(gpslinelog,"Minor Alarms:");
   for (imsg = 0; imsg < MAX_MINOR_ALARMS; imsg++) {
		if (minor_alarms & (1 << imsg)) {
      	if (cnt == 2) {
         	show_crlf ();
            cnt = 0;
         }
			xprintf (" %s ", minor_alarm_text[imsg]);
			sprintf(str," %s ", minor_alarm_text[imsg]); strcat(gpslinelog,str);
         cnt++;
		}
	}
   show_crlf ();
   if(strcmp(shmgps->s0x8FAC_5,gpslinelog) != 0) {
     strcpy(shmgps->s0x8FAC_5,gpslinelog); gpslog(gpslinelog);
   }

   xprintf ("GPS Decoding Status: ");
   sprintf(gpslinelog,"GPS Decoding Status: ");
   switch (GPS_decoding_status) {
	case 0x00:
		xprintf ("Doing fixes");
		strcat(gpslinelog,"Doing fixes");
		break;
   case 0x01:
		xprintf ("Don't have GPS time");
		strcat(gpslinelog,"Don't have GPS time");
		break;
   case 0x03:
		xprintf ("PDOP is too high");
		strcat(gpslinelog,"PDOP is too high");
		break;
   case 0x08:
		xprintf ("No usable sats");
		strcat(gpslinelog,"No usable sats");
		break;
   case 0x09:
		xprintf ("Only 1 usable sat");
		strcat(gpslinelog,"Only 1 usable sat");
		break;
   case 0x0A:
		xprintf ("Only 2 usable sats");
		strcat(gpslinelog,"Only 2 usable sats");
		break;
   case 0x0B:
		xprintf ("Only 3 usable sats");
		strcat(gpslinelog,"Only 3 usable sats");
		break;
   case 0x0C:
		xprintf ("Choosen sat is unusable");
		strcat(gpslinelog,"Choosen sat is unusable");
		break;
   case 0x10:
		xprintf ("TRAIM rejected the fix");
		strcat(gpslinelog,"TRAIM rejected the fix");
		break;
   }
   show_crlf ();
   if(strcmp(shmgps->s0x8FAC_6,gpslinelog) != 0) {
     strcpy(shmgps->s0x8FAC_6,gpslinelog); gpslog(gpslinelog);
   }

   xprintf ("Disciplining activity: ");
   sprintf(gpslinelog,"Disciplining activity: ");
   switch (disciplining_activity) {
	case 0x00:
		xprintf ("Phase Locking");
		strcat(gpslinelog,"Phase Locking");
		break;
   case 0x01:
		xprintf ("Oscillator Warm-up");
		strcat(gpslinelog,"Oscillator Warm-up");
		break;
   case 0x02:
		xprintf ("Frequency Locking");
		strcat(gpslinelog,"Frequency Locking");
		break;
   case 0x03:
		xprintf ("Placing PPS");
		strcat(gpslinelog,"Placing PPS");
		break;
   case 0x04:
		xprintf ("Initializing Loop Filter");
		strcat(gpslinelog,"Initializing Loop Filter");
		break;
   case 0x05:
		xprintf ("Compenstating OCXO for aging and temperature");
		strcat(gpslinelog,"Compenstating OCXO for aging and temperature");
		break;
   case 0x06:
		xprintf ("Inactive");
		strcat(gpslinelog,"Inactive");
		break;
   }
   show_crlf ();
   if(strcmp(shmgps->s0x8FAC_7,gpslinelog) != 0) {
     strcpy(shmgps->s0x8FAC_7,gpslinelog); gpslog(gpslinelog);
   }

   xprintf ("SS1, SS2: %X, %X", spare_stat1, spare_stat2);  show_crlf ();
   sprintf(gpslinelog,"SS1, SS2: %X, %X", spare_stat1, spare_stat2);
   if(strcmp(shmgps->s0x8FAC_8,gpslinelog) != 0) {
     strcpy(shmgps->s0x8FAC_8,gpslinelog); gpslog(gpslinelog);
   }

   xprintf ("PPS Quality: %.2f  10 MHz Quality: %.2f", PPS_quality,
            MHz_quality);  show_crlf ();
   sprintf(gpslinelog,"PPS Quality: %.2fns  10 MHz Quality: %.2fppb", PPS_quality, MHz_quality);
   if(strcmp(shmgps->s0x8FAC_9,gpslinelog) != 0) {
     strcpy(shmgps->s0x8FAC_9,gpslinelog); gpslog(gpslinelog);
   }

   xprintf ("DAC Value: %lu  DAC Voltage: %f  Temp: %.2f", DAC_value,
   			DAC_voltage, temp); show_crlf ();
   sprintf(gpslinelog,"DAC Value: %lu  DAC Voltage: %f  Temp: %.2f", DAC_value, DAC_voltage, temp);
   if(strcmp(shmgps->s0x8FAC_10,gpslinelog) != 0) {
     strcpy(shmgps->s0x8FAC_10,gpslinelog); gpslog(gpslinelog);
   }

   latitude *= R2D;
   longitude *= R2D;
   xprintf ("Latitude: %.3f  Longitude: %.3f  Altitude: %.3f", latitude,
    			longitude, altitude);
   sprintf(gpslinelog,"Latitude: %.11f  Longitude: %.11f  Altitude: %.11f", latitude, longitude, altitude);
   if(strcmp(shmgps->s0x8FAC_11,gpslinelog) != 0) {
     strcpy(shmgps->s0x8FAC_11,gpslinelog); gpslog(gpslinelog);
   }
}

/* 0x8FA0 */
static void rpt_8FA0 (TSIPPKT *rpt)
/* report DAC value */
{
	unsigned char
   	subpacket_id, DAC_resolution, DAC_data_format;
	unsigned long
   	DAC_value;
   float
   	DAC_voltage, min_DAC_voltage, max_DAC_voltage;

	/* unload rptbuf */
	if (rpt_0x8FA0 (rpt, &subpacket_id, &DAC_value, &DAC_voltage,
   					 &DAC_resolution, &DAC_data_format, &min_DAC_voltage,
                   &max_DAC_voltage)) {
		parsed = BADLEN_PARSE;
		return;
	}

	xprintf ("DAC Value: %lu", DAC_value); show_crlf ();
   xprintf ("DAC Voltage: %f", DAC_voltage); show_crlf ();
   xprintf ("DAC Resolution: %d", DAC_resolution); show_crlf ();
   xprintf ("DAC Data Format: %s",  DAC_data_format ? "2's complement" :
   			"offset binary"); show_crlf ();
   xprintf ("Min Voltage: %.2f   Max Voltage: %.2f", min_DAC_voltage,
   			max_DAC_voltage);
}

/* 0x8FA1 */
static void rpt_8FA1 (TSIPPKT *rpt)
/* report 10MHz Sense */
{
	unsigned char
   	subpacket_id, MHz_sense;

	/* unload rptbuf */
	if (rpt_0x8FA1 (rpt, &subpacket_id, &MHz_sense)) {
		parsed = BADLEN_PARSE;
		return;
	}

	xprintf("10MHz Sense: %s", MHz_sense ? "falling" : "rising");
}

/* 0x8FA2 */
static void rpt_8FA2 (TSIPPKT *rpt)
/* report UTC/GPS timing mode */
{
	unsigned char
   	subpacket_id, timing_mode;

	/* unload rptbuf */
	if (rpt_0x8FA2 (rpt, &subpacket_id, &timing_mode)) {
		parsed = BADLEN_PARSE;
		return;
	}

	xprintf("Timing mode: %s, %s", (timing_mode & 1) ? "UTC time" : "GPS time",
   			(timing_mode >> 1) ? "UTC PPS" : "GPS PPS");
}

/* 0xBB, subcode 0 */
static void rpt_primary_stinger_rcvr_config(TSIPPKT *rpt)
{
	GL_0xBB00_DATA
               TsipxBB;

	if (rpt_0xBB_00(rpt, &TsipxBB)){
		parsed = BADLEN_PARSE;
		return;
	}

	xprintf("Stinger primary receiver configuration parameters:");
	print_primary_stinger_rcvr_config(&TsipxBB);
}

void print_primary_stinger_rcvr_config(GL_0xBB00_DATA *TsipxBB)
{
	show_crlf();
	xprintf("operating mode:     ");
	if (TsipxBB->operating_mode == BYTE_UNCHANGED) xprintf("unchanged");
	else xprintf("%s",
		(TsipxBB->operating_mode < POS_FIX_TEXT_LEN) ?
		pos_fix_text[TsipxBB->operating_mode]: "unknown operating mode");
	show_crlf();

	xprintf("DGPS mode:          ");
	if (TsipxBB->dgps_mode == BYTE_UNCHANGED) xprintf("unchanged");
	else xprintf("%s  ",(TsipxBB->dgps_mode<DGPS_MODE_TEXT_LEN) ? 
		dgps_mode_text2[TsipxBB->dgps_mode]: "unknown DGPS mode");
	show_crlf();

	xprintf("DGPS age limit:     ");
	if (TsipxBB->dgps_age_limit == BYTE_UNCHANGED) xprintf("unchanged");
	else xprintf("%u sec", TsipxBB->dgps_age_limit);
	show_crlf();

	xprintf("dynamics:           ");
	if (TsipxBB->dyn_code == BYTE_UNCHANGED) xprintf("unchanged");
	else xprintf("%s", (TsipxBB->dyn_code<DYN_TEXT_LEN) ?
		dyn_text2[TsipxBB->dyn_code]: "unknown dynamics code");
	show_crlf();

	xprintf("solution mode:      ");
	if (TsipxBB->trackmode == BYTE_UNCHANGED) xprintf("unchanged");
	else xprintf("%s", (TsipxBB->trackmode<TRACKMODE_TEXT_LEN) ?
		trackmode_text3[TsipxBB->trackmode]: "unknown tracking mode");
	show_crlf();

	xprintf("elev angle mask:    ");
	if (TsipxBB->elev_mask == SIGNED_UNCHANGED) xprintf("unchanged");
	else xprintf("%g deg", TsipxBB->elev_mask * R2D);
	show_crlf();

	xprintf("SNR mask:           ");
	if (TsipxBB->snr_mask == SIGNED_UNCHANGED) xprintf("unchanged");
	else xprintf("%g AMU", TsipxBB->snr_mask);
	show_crlf();

	xprintf("PDOP mask:          ");
	if (TsipxBB->pdop_mask == SIGNED_UNCHANGED) xprintf("unchanged");
	else xprintf("%g              ", TsipxBB->pdop_mask);
	show_crlf();

	xprintf("PDOP switch:        ");
	if (TsipxBB->pdop_switch == SIGNED_UNCHANGED) xprintf("unchanged");
	else xprintf("%g", TsipxBB->pdop_switch);
	show_crlf();

	xprintf("foliage mode:       ");
	if (TsipxBB->foliage_mode == BYTE_UNCHANGED) xprintf("unchanged");
	else xprintf("%s", (TsipxBB->foliage_mode<FOLIAGE_MODE_TEXT_LEN) ?
		foliage_mode_text[TsipxBB->foliage_mode]: "unknown foliage mode");
	show_crlf();

	xprintf("low power mode:     ");
	if (TsipxBB->low_power_mode == BYTE_UNCHANGED) xprintf("unchanged");
	else xprintf("%s",(TsipxBB->low_power_mode<LOW_POWER_TEXT_LEN) ?
		low_power_text[TsipxBB->low_power_mode]: "unknown low power mode");
	show_crlf();

	xprintf("clock hold capable: ");
	if (TsipxBB->clock_hold == BYTE_UNCHANGED) xprintf("unchanged");
	else xprintf("%s", (TsipxBB->clock_hold<TOGGLE_TEXT_LEN) ?
		toggle_text[TsipxBB->clock_hold]: "unknown clock hold mode");
	show_crlf();

	xprintf("measurement rate:   ");
	if (TsipxBB->meas_rate == BYTE_UNCHANGED) xprintf("unchanged");
	else xprintf("%s",(TsipxBB->meas_rate<MEAS_TEXT_LEN) ?
		meas_rate_text[TsipxBB->meas_rate]: "unknown measurement rate");
	show_crlf();

	xprintf("position fix rate:  ");
	if (TsipxBB->pos_rate == BYTE_UNCHANGED) xprintf("unchanged");
	else xprintf("%s",(TsipxBB->pos_rate<POS_RATE_TEXT_LEN) ?
		pos_rate_text[TsipxBB->pos_rate]: "unknown position fix rate");
}

void print_rcvr_serial_port_config(
	unsigned char port_num,
	unsigned char in_baud,
	unsigned char out_baud,
	unsigned char data_bits,
	unsigned char parity,
	unsigned char stop_bits,
	unsigned char flow_control,
	unsigned char protocols_in,
	unsigned char protocols_out,
	unsigned char reserved) {

	unsigned char known;

	xprintf ("   RECEIVER serial port %s config:",
		(port_num<RCVR_PORT_TEXT_LEN-1)?rcvr_port_text[port_num]:"unknown port number");

	show_crlf();
	xprintf("             Input baud %s, Output baud %s, %s - %s - %s",
		(in_baud<BAUD_TEXT_LEN)?st_baud_text_app[in_baud]:"unknown input baud rate",
		(out_baud<BAUD_TEXT_LEN)?st_baud_text_app[out_baud]:"unknown output baud rate",
		(data_bits<DATABIT_TEXT_LEN)?databit_text_app[data_bits]:"unknown data bits",
		(parity<PARITY_TEXT_LEN)?parity_text[parity]:"unknown parity",
		(stop_bits<STOPBIT_TEXT_LEN)?stopbit_text[stop_bits]:"unknown stop bits");
	show_crlf ();
	xprintf("             Flow control: ");
	if (flow_control == PORT_HS_NONE) xprintf("%s ", flow_cntrl_text[0]);
	else {
		known = FALSE;
		if (flow_control&PORT_HS_RTS_CTS) {
			xprintf("%s ", flow_cntrl_text[1]);
			known = TRUE;
		}
		if (flow_control&PORT_HS_TX_XON_XOFF) {
			xprintf("%s ", flow_cntrl_text[2]);
			known = TRUE;
		}
		if (flow_control&PORT_HS_TX_XANY) {
			xprintf("%s ", flow_cntrl_text[3]);
			known = TRUE;
		}
		if (flow_control&PORT_HS_RX_XON_XOFF){
			xprintf("%s ", flow_cntrl_text[4]);
			known = TRUE;
		}
		if (known == FALSE) xprintf("Unknown flow control setting");
	}
	show_crlf ();
	xprintf ("             Input protocols: ");
	known = FALSE;
	if (protocols_in&TAIP) {
		xprintf("%s ", protocols_text[0]);
		known = TRUE;
	}
	if (protocols_in&TSIP) {
		xprintf("%s ", protocols_text[1]);
		known = TRUE;
	}
	if (protocols_in&NMEA) {
		xprintf("%s ", protocols_text[2]);
		known = TRUE;
	}
	if (protocols_in&RTCM){
		xprintf("%s ", protocols_text[3]);
		known = TRUE;
	}
	if (protocols_in&DCOL){
		xprintf("%s ", protocols_text[4]);
		known = TRUE;
	}
	if (known == FALSE) xprintf("No known");

	show_crlf ();
	xprintf ("             Output protocols: ");
	known = FALSE;
	if (protocols_out&TAIP) {
		xprintf("%s ", protocols_text[0]);
		known = TRUE;
	}
	if (protocols_out&TSIP) {
		xprintf("%s ", protocols_text[1]);
		known = TRUE;
	}
	if (protocols_out&NMEA) {
		xprintf("%s ", protocols_text[2]);
		known = TRUE;
	}
	if (protocols_out&RTCM){
		xprintf("%s ", protocols_text[3]);
		known = TRUE;
	}
	if (protocols_out&DCOL){
		xprintf("%s ", protocols_text[4]);
		known = TRUE;
	}
	if (known == FALSE) xprintf("No known");
	reserved = reserved ;
	show_crlf ();
}

/* 0xBC */
static void rpt_rcvr_serial_port_config (TSIPPKT *rpt)
{
	unsigned char
		port_num, in_baud, out_baud, data_bits, parity, stop_bits, flow_control,
		protocols_in, protocols_out, reserved;

	/* unload rptbuf */
	if (rpt_0xBC (rpt, &port_num, &in_baud, &out_baud, &data_bits, &parity,
			&stop_bits, &flow_control, &protocols_in, &protocols_out, &reserved)) {
		parsed = BADLEN_PARSE;
		return;
	}
	/* rptbuf unloaded */

	print_rcvr_serial_port_config(port_num, in_baud, out_baud, data_bits,
		parity, stop_bits, flow_control, protocols_in, protocols_out, reserved);

 }

static void rpt_8F (TSIPPKT *rpt)
{
	switch (rpt->buf[0]) {
	case 0x15:
		rpt_8F15 (rpt);
		break;

	case 0x20:
		rpt_8F20 (rpt);
		break;

   case 0x41:
		rpt_8F41 (rpt);
		break;

   case 0x42:
   	rpt_8F42 (rpt);
      break;

	case 0x4A:
   	rpt_8F4A (rpt);
      break;

   case 0x4B:
   	rpt_8F4B (rpt);
      break;

   case 0xAB:
      rpt_8FAB (rpt);
      break;

   case 0xAC:
   	rpt_8FAC (rpt);
      break;

   case 0xA0:
   	rpt_8FA0 (rpt);
      break;

   case 0xA1:
   	rpt_8FA1 (rpt);
      break;

   case 0xA2:
   	rpt_8FA2 (rpt);
      break;

	default:
		parsed = BADID_PARSE;
		break;
	}
}

void print_msg_table_header (unsigned char rptcode)
{
	/* force header is to help auto-output function */
	/* last_rptcode is to determine whether to print a header */
	/* for the first occurence of a series of reports */
	static unsigned char
		last_rptcode = 0;

	if (rptcode!=last_rptcode) switch (rptcode)
	{
	case 0x5A:
		/* supply a header in console output */
		xprintf ("Raw Measurement Data");
		show_crlf ();
		xprintf (
"   SV  Sample   SNR  Code Phase   Doppler    Seconds     Time of Meas");
		show_crlf();
		break;

	case 0x5C:
		xprintf ("Tracking Info");
		show_crlf ();
		xprintf (
"   SV  C S Acq Eph   SNR     Time of Meas      Elev   Azim   Old Msec BadD DCol");
		show_crlf();
		break;

	case 0x85:
		xprintf ("Differential Correction Status");
		show_crlf ();
		xprintf (
"    SV  Status  StaH  SV H  IODE1  IODE2  Z-count time        PRC    RRC    DRC");
		show_crlf();
		break;
	}
	last_rptcode = rptcode;
}

static void unknown_rpt (TSIPPKT *rpt)
{
	short i;

	/* app-specific rpt packets */
	xprintf ("TSIP report packet ID %2Xh, length %d", rpt->code, rpt->cnt);
	if (parsed == BADLEN_PARSE) xprintf (": Bad length");
	if (parsed == BADID_PARSE) xprintf (": translation not supported");
	show_crlf ();
	for (i = 0; i < rpt->cnt; i++) {
		if ((i % 20) == 0) show_crlf ();
		xprintf (" %02X", rpt->buf[i]);
	}
}
/**/
void rpt_packet (TSIPPKT *rpt)
{

	parsed = GOOD_PARSE;

	show_crlf();

	/* print a header if this is the first of a series of messages */
	print_msg_table_header (rpt->code);

	switch (rpt->code) {
   case 0x42:
		rpt_single_ECEF_position (rpt);
		break;

	case 0x43:
		rpt_single_ECEF_velocity (rpt);
		break;

	case 0x45:
		rpt_SW_version (rpt);
		break;

	case 0x47:
		rpt_SNR_all_SVs (rpt);
		break;

	case 0x49:
		rpt_almanac_health_page (rpt);
		break;

	case 0x4A:
		if (rpt->cnt == 20)
		{
			rpt_single_lla_position (rpt);
		}
		else if (rpt->cnt == 9)

		{
			rpt_ref_alt (rpt);
		}
		break;

	case 0x4B:
		rpt_rcvr_id_and_status (rpt);
		break;

	case 0x4F:
		rpt_UTC_parameters (rpt);
		break;

	case 0x55:
		rpt_io_opt (rpt);
		break;

	case 0x56:
		rpt_ENU_velocity (rpt);
		break;

	case 0x57:
		rpt_last_fix_info (rpt);
		break;

	case 0x58:
		rpt_GPS_system_data (rpt);
		break;

	case 0x59:
		rpt_SVs_enabled (rpt);
		break;

   case 0x6D:
		rpt_allSV_selection (rpt);
		break;

   case 0x70:
		show_pv_operation(rpt);
		break;

	case 0x83:
		rpt_double_ECEF_position (rpt);
		break;

	case 0x84:
		rpt_double_lla_position (rpt);
		break;

	case 0x8F:
		rpt_8F (rpt);
		break;

	case 0xBB: /* Stinger receiver configuration */
		rpt_primary_stinger_rcvr_config (rpt);
		break;

	case 0xBC:  	/* Stinger receiver serial port config */
		rpt_rcvr_serial_port_config (rpt);
		break;

	default:
		parsed = BADID_PARSE;
		break;
	}

	if (parsed != GOOD_PARSE)
	{
		unknown_rpt (rpt);
	}
}


