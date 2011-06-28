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
 * Vers	Date		   Changes										 			Author
 * ----	---------   ----------------------------------------	   ----------
 * v1.40
 * 7.52a			   	matched 7.52 manual									pvwl
 *			11 Feb 98   converted to tboltcht								jah
 * *************************************************************************
 *
 * This header file contains definitions for general TSIP structures and
 * prototypes for general functions.
 *
 * This source code is supplied without warranty and is intended only as
 * sample source code for exercising the TSIP interface.  We do appreciate
 * comments and try to support the software as much as possible.
 *
 * Developers of new applications are encouraged to use the functions
 * defined in TSIP_IFC.C, and pattern their drivers on the function main()
 * in TSIPCHAT.C
 *
 * *************************************************************************
 *
 */
#define TSIP_H_DEFINED 140
#define TSIP_VERNUM "7.52a"
#define v752

#define FALSE 	(0)
#define TRUE 	(!FALSE)
#define GPS_PI 	(3.1415926535898)
#define	D2R		(GPS_PI/180.0)
#define	R2D		(180.0/GPS_PI)
#define WEEK 	(604800.)
#define MAXCHAN  (8)

/* control characters for TSIP packets */
#define DLE 	(0x10)
#define ETX 	(0x03)

#define MAX_RPTBUF (256)

/* values of TSIPPKT.status */
#define TSIP_PARSED_EMPTY 	0
#define TSIP_PARSED_FULL 	1
#define TSIP_PARSED_DLE_1 	2
#define TSIP_PARSED_DATA 	3
#define TSIP_PARSED_DLE_2 	4

/* TSIP packets have the following structure, whether report or command. */
typedef struct {
	short
		cnt;				/* size of buf; < MAX_RPTBUF unsigned chars */
	unsigned char
		status,			 /* TSIP packet format/parse status */
		code;				/* TSIP code */
	unsigned char
		buf[MAX_RPTBUF];	/* report or command string */
} TSIPPKT;

/* TSIP binary data structures */
typedef struct {
	unsigned char
		t_oa_raw, SV_health;
	float
		e, t_oa, i_0, OMEGADOT, sqrt_A,
		OMEGA_0, omega, M_0, a_f0, a_f1,
		Axis, n, OMEGA_n, ODOT_n, t_zc;
	short
		weeknum, wn_oa;
} ALM_INFO;

typedef struct {     /*  Almanac health page (25) parameters  */
	unsigned char
		WN_a, SV_health[32], t_oa;
} ALH_PARMS;

typedef struct {     /*  Universal Coordinated Time (UTC) parms */
	double
		A_0;
	float
		A_1;
	short
		delta_t_LS;
	float
		t_ot;
	short
		WN_t, WN_LSF, DN, delta_t_LSF;
} UTC_INFO;

typedef struct {      /*  Ionospheric info (float)  */
	float
		alpha_0, alpha_1, alpha_2, alpha_3,
		beta_0, beta_1, beta_2, beta_3;
} ION_INFO;

typedef struct {      /*  Subframe 1 info (float)  */
	short
		weeknum;
	unsigned char
		codeL2, L2Pdata, SVacc_raw, SV_health;
	short
		IODC;
	float
		T_GD, t_oc, a_f2, a_f1, a_f0, SVacc;
} EPHEM_CLOCK;

typedef	struct {     /*  Ephemeris info (float)  */
	unsigned char
		IODE, fit_interval;
	float
		C_rs, delta_n;
	double
		M_0;
	float
		C_uc;
	double
		e;
	float
		C_us;
	double
		sqrt_A;
	float
		t_oe, C_ic;
	double
		OMEGA_0;
	float
		C_is;
	double
		i_0;
	float
		C_rc;
	double
		omega;
	float
		OMEGADOT, IDOT;
	double
		Axis, n, r1me2, OMEGA_n, ODOT_n;
} EPHEM_ORBIT;

typedef struct {     /* Navigation data structure */
	short
		sv_number;     /* SV number (0 = no entry) */
	float
		t_ephem;       /* time of ephemeris collection */
	EPHEM_CLOCK
		ephclk;        /* subframe 1 data */
	EPHEM_ORBIT
		ephorb;        /* ephemeris data */
} NAV_INFO;

typedef struct {
	float
		flt1;
	unsigned char
		chr1, chr2, chr3, chr4;
	float
		flt2, flt3, flt4, flt5;
	unsigned char
		chr5;
} TSIP_POS_FILT_PARMS;

typedef struct {
	unsigned char		bSubcode ;
	unsigned char		bProdOptionsPre ;
	unsigned char 		bProdNumberExt ;
	unsigned short		iCaseSerialNumberPre ;
	unsigned long		iiCaseSerialNumber ;
	unsigned long		iiProdNumber ;
	unsigned short		iReservedOp ;
	unsigned short		iMachineID ;
	unsigned short		iReserved ;
} GL_8x42_DATA ;

typedef struct {
	unsigned char
               subcode,
               operating_mode,
               dgps_mode,
               dyn_code,
               trackmode;
	float
               elev_mask,
               snr_mask,
               pdop_mask,
               pdop_switch;
	unsigned char
               dgps_age_limit,
               foliage_mode,
               low_power_mode,
               clock_hold,
               meas_rate,
               pos_rate,
               extra[13];
} GL_0xBB00_DATA;

/*
typedef struct cph1_datablock_struct {
	unsigned char prn, flagbyte, reserved;
	signed char elev;
	short azim;
	unsigned char signal_strength;
	double pseudorange, carrier_phase;
	float Doppler;
};

typedef struct cph1_msg_struct {
	unsigned char subcode, preamble;
	unsigned short length;
	double receive_time, clock_offset;
	unsigned char numsvs;
	struct cph1_datablock_struct db[MAXCHAN];
	unsigned short checksum;
	unsigned char postamble;
};
*/

/*******************        PROTOYPES            *****************/
/*******************        PROTOYPES            *****************/
/*******************        PROTOYPES            *****************/

/*
 * This routine sends a command to the receiver.  It calls a
 * function sendb() that must be supplied by the user.  All
 * calls to this subroutine are within TSIP_IFC.C.
 */

void send_cmd
	(TSIPPKT *cmd);

/* functions to add a byte from the receiver and "unstuffs" it */
void tsip_input_proc (
	TSIPPKT *rpt, short newbyte);

/**/
/*************************** in TSIP_IFC.C ************************/
/* Functions for low-level serial port access calls.              */
/******************************************************************/
short getb
	(void);
short sendb
	(unsigned char db);


/* gets time of most recent report; returns zero until week number known */
double get_tsip_time (void);

/* functions to get items from a TSIP report packet character buffer */
/* Byte order is reversed for Intel machines; controlled by BYTESWAP */
short bgetint
	(unsigned char *bp);
float bgetsingle
	(unsigned char *bp);
double bgetdouble
	(unsigned char *bp);

/* This routine puts floats into a TSIP command packet character  */
/* buffer.  Byte order is reversed for Intel machines. */
void bputsingle
	(float a, unsigned char *cmdbuf);

/**/
/* prototypes for command-encode primitives with suffix convention:  */
/* c = clear, s = set, q = query, e = enable, d = disable            */
void cmd_0x1E  (
	unsigned char reset_type);
void cmd_0x1F  (void);
void cmd_0x24  (void);
void cmd_0x25  (void);
void cmd_0x27  (void);
void cmd_0x29  (void);
void cmd_0x2F  (void);
void cmd_0x31  (
	float ECEF_pos[3]);
void cmd_0x32  (
	float lat,
	float lon,
	float alt);
void cmd_0x34  (
   unsigned char sv_prn);   
void cmd_0x35q (void);
void cmd_0x35s (
	unsigned char pos_code,
	unsigned char vel_code,
	unsigned char time_code,
	unsigned char opts_code);
void cmd_0x37  (void);
void cmd_0x38q (
	unsigned char data_type,
	unsigned char sv_prn);
void cmd_0x38s (
	unsigned char data_type,
	unsigned char sv_prn,
	unsigned char data_length,
	unsigned char *databuf);
void cmd_0x39  (
	unsigned char op_code,
	unsigned char sv_prn);
void cmd_0x70q  (void);
void cmd_0x70s  (
	unsigned char dyn_switch,
	unsigned char static_switch,
	unsigned char alt_switch,
	unsigned char extra) ;
void cmd_0x8E15q (void);
void cmd_0x8E15s (unsigned char datum_index);
void cmd_0x8E15s_2 (double datum_parms[5]);
void cmd_0x8E20q (void);
void cmd_0x8E20d (void);
void cmd_0x8E20e (void);
void cmd_0x8E41q (void);
void cmd_0x8E42q (void);
void cmd_0x8E45s (unsigned char segment);
void cmd_0x8E4Aq (void);
void cmd_0x8E4As (
	unsigned char PPS_driver_switch,
   unsigned char PPS_time_base,
   unsigned char PPS_polarity,
   double PPS_offset,
   float bias_uncert_threshold);
void cmd_0x8E4Bq (void);
void cmd_0x8E4Bs (unsigned long survey_limit);
void cmd_0x8E4Cs (unsigned char segment);
void cmd_0x8EA0q (void);
void cmd_0x8EA0s (float voltage);
void cmd_0x8EA0s_2 (unsigned long value);
void cmd_0x8EA1q (void);
void cmd_0x8EA1s (unsigned char MHz_sense);
void cmd_0x8EA2q (void);
void cmd_0x8EA2s (unsigned char timing_mode);
void cmd_0x8EA3s (unsigned char disciplining_command);
void cmd_0x8EA6s (unsigned char self_survey_command);
void cmd_0x8E  (
	unsigned char hexbyte[],
	unsigned char hexnum);
void cmd_0xBB_00s
       (GL_0xBB00_DATA *TsipxBB);
void cmd_0xBBq
       (unsigned char subcode);
void cmd_0xBCs (
	unsigned char port_num,
	unsigned char in_baud,
	unsigned char out_baud,
	unsigned char data_bits,
	unsigned char parity,
	unsigned char stop_bits,
	unsigned char flow_control,
	unsigned char bReserved1,
	unsigned char bReserved2,
	unsigned char reserved);
void cmd_0xBCq (unsigned char port_num);
void cmd_0xxx (
	unsigned char hexcode, unsigned char hexbyte[], unsigned char hexnum);
/**/
/*  prototypes for report-decode primitives */
/* in TSIP_IFC.C */
short rpt_0x42 (TSIPPKT *rpt,
	float ECEF_pos[3],
	float *time_of_fix);
short rpt_0x43 (TSIPPKT *rpt,
	float ECEF_vel[3],
	float *freq_offset,
	float *time_of_fix);
short rpt_0x45 (TSIPPKT *rpt,
	unsigned char *major_nav_version,
	unsigned char *minor_nav_version,
	unsigned char *nav_day,
	unsigned char *nav_month,
	unsigned char *nav_year,
	unsigned char *major_dsp_version,
	unsigned char *minor_dsp_version,
	unsigned char *dsp_day,
	unsigned char *dsp_month,
	unsigned char *dsp_year);
short rpt_0x47 (TSIPPKT *rpt,
	unsigned char *nsvs,
	unsigned char *sv_prn,
	float *snr);
short rpt_0x49 (TSIPPKT *rpt,
	unsigned char *sv_health);
short rpt_0x4A (TSIPPKT *rpt,
	float *lat,
	float *lon,
	float *alt,
	float *clock_bias,
	float *time_of_fix);
short rpt_0x4A_2 (TSIPPKT *rpt,
	float *alt,
	float *dummy,
	unsigned char *alt_flag);
short rpt_0x4B (TSIPPKT *rpt,
	unsigned char *machine_id,
	unsigned char *status_1,
	unsigned char *status_2);
short rpt_0x4F (TSIPPKT *rpt,
	double *a0,
	float *a1,
   short *dt_ls,
	float *time_of_data,
	short *wn_t,
	short *wn_lsf,
	short *dn,
	short *dt_lsf);
short rpt_0x55 (TSIPPKT *rpt,
	unsigned char *pos_code,
	unsigned char *vel_code,
	unsigned char *time_code,
	unsigned char *aux_code);
short rpt_0x56 (TSIPPKT *rpt,
	float vel_ENU[3],
	float *freq_offset,
	float *time_of_fix);
short rpt_0x57 (TSIPPKT *rpt,
	unsigned char *source_code,
	unsigned char *diag_code,
	short *week_num,
	float *time_of_fix);
short rpt_0x58 (TSIPPKT *rpt,
	unsigned char *op_code,
	unsigned char *data_type,
	unsigned char *sv_prn,
	unsigned char *data_length,
	unsigned char *data_packet);
short rpt_0x59 (TSIPPKT *rpt,
	unsigned char *code_type,
	unsigned char status_code[32]);
short rpt_0x6D (TSIPPKT *rpt,
	unsigned char *manual_mode,
	unsigned char *nsvs,
	unsigned char *ndim,
	char sv_prn[],
	float *pdop,
	float *hdop,
	float *vdop,
	float *tdop);
short rpt_0x70 (TSIPPKT *rpt,
	 unsigned char *dyn_switch,
	 unsigned char *static_switch,
	 unsigned char *alt_switch,
	 unsigned char *extra);
short rpt_0x83 (TSIPPKT *rpt,
	double ECEF_pos[3],
	double *clock_bias,
	float *time_of_fix);
short rpt_0x84 (TSIPPKT *rpt,
	double *lat,
	double *lon,
	double *alt,
	double *clock_bias,
	float *time_of_fix);
short rpt_0x8F15 (TSIPPKT *rpt,
	short *datum_idx,
	double datum_coeffs[5]);
short rpt_0x8F20 (TSIPPKT *rpt,
	unsigned char *subpacket_id,
	unsigned char *info,
	double *lat,
	double *lon,
	double *alt,
	double vel_enu[],
	double *time_of_fix,
	short *week_num,
	unsigned char *nsvs, unsigned char sv_prn[],
	short sv_IODC[],
	short *datum_index);
short rpt_0x8F41 (TSIPPKT *rpt,
   unsigned char *subpacket_id,
   short *serial_num_prefix,
   float *serial_num,
   unsigned char *year,
   unsigned char *month,
   unsigned char *day,
   unsigned char *hour,
   float *oscillator_offset,
   short *test_code_id_num) ;
short rpt_0x8F42 (TSIPPKT *rpt,
	unsigned char *subpacket_id,
   unsigned char *prod_opt_prefix,
   unsigned char *prod_num_extension,
   short *case_serial_num_prefix,
   unsigned long *case_serial_num,
   unsigned long *production_num,
   short *machine_ID_num) ;
short rpt_0x8F4A (TSIPPKT *rpt,
	unsigned char *subpacket_id,
   unsigned char *PPS_driver_switch,
   unsigned char *PPS_time_base,
   unsigned char *PPS_polarity,
	double *PPS_offset,
   float *bias_uncert_threshold) ;
short rpt_0x8F4B (TSIPPKT *rpt,
	unsigned char *subpacket_id,
   unsigned long *survey_limit) ;
short rpt_0x8FAB (TSIPPKT *rpt,
	unsigned char *subpacket_id,
   unsigned long *time_of_week,
   unsigned short *week_number,
   unsigned short *UTC_offset,
   unsigned char *timing_flag,
   unsigned char *seconds,
   unsigned char *minutes,
   unsigned char *hours,
   unsigned char *day_of_month,
   unsigned char *month,
   unsigned short *year) ;
short rpt_0x8FAC (TSIPPKT *rpt,
   unsigned char *subpacket_id,
   unsigned char *receiver_mode,
   unsigned char *discipling_mode,
   unsigned char *survey_progress,
   unsigned long *holdover_duration,
   unsigned short *critical_alarms,
	unsigned short *minor_alarms,
   unsigned char *GPS_decoding_status,
   unsigned char *discipling_activity,
   unsigned char *spare_stat1,
   unsigned char *spare_stat2,
   float *PPS_quality,
   float *MHz_quality,
   unsigned long *DAC_value,
   float *DAC_voltage,
   float *temp,
   double *latitude,
   double *longitude,
   double *altitude) ;
short rpt_0x8FA0 (TSIPPKT *rpt,
	unsigned char *subpacket_id,
   unsigned long *DAC_value,
   float *DAC_voltage,
   unsigned char *DAC_resolution,
   unsigned char *DAC_data_format,
   float *min_DAC_voltage,
   float *max_DAC_voltage) ;
short rpt_0x8FA1 (TSIPPKT *rpt,
	unsigned char *subpacket_id,
   unsigned char *MHz_sense);
short rpt_0x8FA2 (TSIPPKT *rpt,
	unsigned char *subpacket_id,
   unsigned char *timing_mode);
short rpt_0xBB_00
       (TSIPPKT *rpt,
        GL_0xBB00_DATA *TsipxBB);
short rpt_0xBC   (TSIPPKT *rpt,
	unsigned char *port_num,
	unsigned char *in_baud,
	unsigned char *out_baud,
	unsigned char *data_bits,
	unsigned char *parity,
	unsigned char *stop_bits,
	unsigned char *flow_control,
	unsigned char *protocols_in,
	unsigned char *protocols_out,
	unsigned char *reserved);

