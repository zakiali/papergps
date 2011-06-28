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
 * Vers	Date		   Changes									    			Author
 * ----	---------   ----------------------------------------	   ----------
 * 1.40              initial
 * 752a  18 jul 97   matched 7.52 manual                          pvwl
 *			11 feb 98	converted for tboltcht								jah
 *
 * 1.41  remove "static" from bgetdouble as it produced error on R61 fedora linux
 * *************************************************************************
 *
 * TSIP_IFC.C carries several interface functions observed as primitive
 * functions that perform two specific tasks with the receiver:  prepare
 * commands to be transferred to the receiver and interpret reports received
 * from the receiver.  The module can be used as is, or with minimal changes
 * if there is a need to create a TSIP communications application separate
 * from TSIPCHAT. (The construction of most argument lists do not use
 * structures, but the developer is encouraged to reconstruct them using such
 * definitions to meet the project requirements.)  Declarations of TSIP_IFC.C
 * functions are included in TSIP_IFC.H to provide prototyping definitions.
 * Two serial port primitives defined in TSIPCHAT.C, getb() and sendb(), are
 * required for these functions.  (Note: getb() is no longer used in this file.
 * sendb() is still required.)
 *
 * 1) Command Functions:  Command functions come in the form cmd_0x??().
 * Command functions handle data/command packets to be transferred to the
 * receiver.  The function send_cmd() is called by all packet interpreters.
 * The function send_cmd() supplies the DLE stuffing to a command string and
 * then sends the command to the serial port using the primitive function
 * sendb().
 *
 * The functions cmd_0x??() are command string generators patterned after
 * the document called "Trimble Standard Interface Protocol" listed in
 * Appendix A of the "System Designer Reference Guide."  These functions
 * are called from a command input parser such as the function proc_kbd()
 * in TSIP_CMD.C.  Because a TSIP command code may have different argument
 * lists depending on optional operating modes, there are more functions
 * than command command codes.  Each cmd_0x??() function corresponds to a
 * different functional mode for the command.  The following suffix convention
 * allows each function to serve a unique purpose and cleans up the argument
 * somewhat.
 *
 *           cmd_0x??q   -  query for current value
 * 	         cmd_0x??c   -  clear value (0x1D, 0x73 only)
 * 	         cmd_0x??s   -  set new values
 * 	         cmd_0x??e   -  enable
 * 	         cmd_0x??d   -  disable
 *
 * 2) Report Functions:  Report functions come in the form rpt_0x??().
 * Report functions take care of the incoming packets from the receiver.
 * There is only one function that is used in the main functions in TCHAT.C,
 * TSIPCHAT.C, and TSIPPRNT.C:
 *                            tsip_input_proc().
 *
 * The function tsip_input_proc() accumulates bytes from the receiver,
 * strips control bytes (DLE), and checks if the report end sequence (DLE ETX)
 * has been received.  rpt.status is defined as TSIP_PARSED_FULL (== 1)
 * if a complete packet is available.
 *
 * The functions rpt_0x??() are report string interpreters patterned after
 * the document called "Trimble Standard Interface Protocol", and are called
 * by the report output function rpt_packet() in TSIP_RPT.C.  It should be
 * noted that if the report buffer is sent into the receiver with the wrong
 * length (byte count), the rpt_0x??() returns the Boolean equivalence for
 * TRUE.
 *
 * *************************************************************************
 *
 */

#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include "tsip_ifc.h"

/* for DOS machines, reverse order of bytes as they come through the
 * serial port. */
#define BYTESWAP
#ifdef BYTESWAP

short sendb(unsigned char byte)
{
   printf("sendb() does not do anything\n");
}

static void byteswap (unsigned char *in, unsigned char *out, unsigned char numbytes)
{
	unsigned char *inptr;
	inptr = in + numbytes;
	do {*out++ = *(--inptr);} while (inptr > in);
}
static short bgetshort (unsigned char *bp)
{
	unsigned char outval[2];
	byteswap (bp, outval, 2);
	return (*(short*)outval);
}
static long bgetlong (unsigned char *bp)
{
	unsigned char outval[4];
	byteswap (bp, outval, 4);
	return (*(long*)outval);
}
static unsigned long bgetulong (unsigned char *bp)
{
	unsigned char outval[4];
	byteswap (bp, outval, 4);
	return (*(unsigned long*)outval);
}
static float bgetfloat (unsigned char *bp)
{
	unsigned char outval[4];
	byteswap (bp, outval, 4);
	return (*(float*)outval);
}
double bgetdouble (unsigned char *bp)
{
	unsigned char outval[8];
	byteswap (bp, outval, 8);
	return (*(double*)outval);
}
#define bputshort(a,cmdbuf) 	byteswap((unsigned char*)&(a),cmdbuf,2)
#define bputlong(a,cmdbuf) 		byteswap((unsigned char*)&(a),cmdbuf,4)
#define bputfloat(a,cmdbuf) 	byteswap((unsigned char*)&(a),cmdbuf,4)
#define bputdouble(a,cmdbuf) 	byteswap((unsigned char*)&(a),cmdbuf,8)

#else	/* not BYTESWAP */

#define bgetshort(bp) 	(*(short*)(bp))
#define bgetlong(bp) 	(*(long*)(bp))
#define bgetulong(bp) 	(*(unsigned long*)(bp))
#define bgetfloat(bp) 	(*(float*)(bp))
#define bgetdouble(bp)	(*(double*)(bp))
void bputshort (short a, unsigned char *cmdbuf){*(short*) cmdbuf = a;}
void bputlong (long a, unsigned char *cmdbuf) 	{*(long*) cmdbuf = a;}
void bputfloat (float a, unsigned char *cmdbuf){*(float*) cmdbuf = a;}
void bputdouble (double a, unsigned char *cmdbuf){*(double*) cmdbuf = a;}

#endif






/* the following two routines give the itme of the most recent fix */
static double
	most_recent_fix_time = 0.0;

double get_tsip_time (void)
{
	return most_recent_fix_time;
}

static void set_tsip_time (double time_of_week, short week_num) {
	if (!week_num) {
		if (most_recent_fix_time == 0.0) return;
		week_num = (short)((most_recent_fix_time - time_of_week) / WEEK + 0.5);
	}
	most_recent_fix_time = time_of_week + week_num*WEEK;
}

/**/
void send_cmd (TSIPPKT *cmd)
/* formats a command for sending to TSIP receiver */
{
	short
		i;

	sendb (DLE);
	sendb (cmd->code);
	for (i = 0; i < cmd->cnt; i++) {
		if (cmd->buf[i] == DLE) sendb (DLE);
		sendb (cmd->buf[i]);
	}
	sendb (DLE);
	sendb (ETX);
}

void cmd_0x1E  (unsigned char reset_type)
/* clear battery back-up, then reset */
{
	TSIPPKT cmd;
	cmd.buf[0] = reset_type;
	cmd.cnt = 1;
	cmd.code = 0x1E;
	send_cmd (&cmd);
}

void cmd_0x1F (void)
/* request software versions */
{
	TSIPPKT cmd;
	cmd.cnt = 0;
	cmd.code = 0x1F;
	send_cmd (&cmd);
}

void cmd_0x24 (void)
/* request GPS receiver postition fix mode */
{
	TSIPPKT cmd;
   cmd.cnt = 0;
   cmd.code = 0x24;
   send_cmd (&cmd);
}

void cmd_0x25 (void)
/* initiate soft reset & self-test (equivalent to power cycle) */
{
	TSIPPKT cmd;
	cmd.cnt = 0;
	cmd.code = 0x25;
	send_cmd (&cmd);
}

void cmd_0x27 (void)
/* request signal levels */
{
	TSIPPKT cmd;
	cmd.cnt = 0;
	cmd.code = 0x27;
	send_cmd (&cmd);
}

void cmd_0x29 (void)
/* request almanac health page */
{
	TSIPPKT cmd;
	cmd.cnt = 0;
	cmd.code = 0x29;
	send_cmd (&cmd);
}

void cmd_0x2F (void)
/* request UTC parameters */
{
	TSIPPKT cmd;
	cmd.cnt = 0;
	cmd.code = 0x2F;
	send_cmd (&cmd);
}

/**/
void cmd_0x31 (float ECEF_pos[3])
/* initial accurate position in ECEF coordinates */	
{
	TSIPPKT cmd;
	bputfloat (ECEF_pos[0], &cmd.buf[0]);
	bputfloat (ECEF_pos[1], &cmd.buf[4]);
	bputfloat (ECEF_pos[2], &cmd.buf[8]);
	cmd.cnt = 12;
	cmd.code = 0x31;
	send_cmd (&cmd);
}

void cmd_0x32 (float lat, float lon, float alt)
/* initial accurate position in latitude-longitude-altitude */	
{
	TSIPPKT cmd;
	bputfloat (lat, &cmd.buf[0]);
	bputfloat (lon, &cmd.buf[4]);
	bputfloat (alt, &cmd.buf[8]);
	cmd.cnt = 12;
	cmd.code = 0x32;
	send_cmd (&cmd);
}

void cmd_0x34 (unsigned char sv_prn)
/* set satellite number for 1D timing mode */
{
	TSIPPKT cmd;
	cmd.buf[0] = sv_prn;
	cmd.cnt = 1;
	cmd.code = 0x34;
	send_cmd (&cmd);
}

void cmd_0x35q (void)
/* request serial I/O options */
{
	TSIPPKT cmd;
	cmd.cnt = 0;
	cmd.code = 0x35;
	send_cmd (&cmd);
}

void cmd_0x35s (unsigned char pos_code, unsigned char vel_code, 
	unsigned char time_code, unsigned char opts_code)
/* set serial I/O options */	
{
	TSIPPKT cmd;
	cmd.buf[0] = pos_code;
	cmd.buf[1] = vel_code;
	cmd.buf[2] = time_code;
	cmd.buf[3] = opts_code;
	cmd.cnt = 4;
	cmd.code = 0x35;
	send_cmd (&cmd);
}

void cmd_0x37 (void)
/* request last position, velocity, and status */
{
	TSIPPKT cmd;
	cmd.cnt = 0;
	cmd.code = 0x37;
	send_cmd (&cmd);
}

void cmd_0x38q (unsigned char data_type, unsigned char sv_prn)
/* request GPS system data (binary) */
{
	TSIPPKT cmd;
	cmd.buf[0] = 1;
	cmd.buf[1] = data_type;
	cmd.buf[2] = sv_prn;
	cmd.cnt = 3;
	cmd.code = 0x38;
	send_cmd (&cmd);
}

void cmd_0x38s (unsigned char data_type, unsigned char sv_prn,
	unsigned char data_length, unsigned char *databuf)
/* load GPS system data (binary) */	
{
	TSIPPKT cmd;
	cmd.buf[0] = 2;
	cmd.buf[1] = data_type;
	cmd.buf[2] = sv_prn;
	cmd.buf[3] = data_length;
	memcpy (&cmd.buf[4], databuf, (short)data_length);
	cmd.cnt = (short)data_length + 4;
	cmd.code = 0x38;
	send_cmd (&cmd);
}

void cmd_0x39 (unsigned char op_code, unsigned char sv_prn)
/* set or request enable/health-heed status of satellites */	
{
	TSIPPKT cmd;
	cmd.buf[0] = op_code;
	cmd.buf[1] = sv_prn;
	cmd.cnt = 2;
	cmd.code = 0x39;
	send_cmd (&cmd);
}

void cmd_0x70q (void)
/* filter configuration query */
{
	TSIPPKT cmd;

   cmd.cnt = 0;
   cmd.code = 0x70;
   send_cmd (&cmd);
}

void cmd_0x70s(unsigned char dyn_switch,
               unsigned char static_switch,
               unsigned char alt_switch,
               unsigned char extra)
/* set filter configuration */
{
	TSIPPKT cmd;

	cmd.buf[0] = dyn_switch;
	cmd.buf[1] = static_switch;
	cmd.buf[2] = alt_switch;
	cmd.buf[3] = extra;
	cmd.cnt = 4;
	cmd.code = 0x70;
	send_cmd(&cmd);
}

/**/
void cmd_0x8E15q (void)
/* datum query */
{
   TSIPPKT cmd;

	cmd.cnt = 1;
	cmd.code = 0x8E;
	cmd.buf[0] = 0x15;
	send_cmd (&cmd);
}

void cmd_0x8E15s (unsigned char datum_index)
/* datum set - standard parameters */
{
   TSIPPKT cmd;

	cmd.buf[0] = 0x15;
	cmd.buf[1] = 0;	/* datum index is a short; this is always 0 */
	cmd.buf[2] = datum_index;
	cmd.cnt = 3;
	cmd.code = 0x8E;
	send_cmd (&cmd);
}

void cmd_0x8E15s_2 (double datum_coeffs[5])
/* datum set - special parameters */
{
	TSIPPKT cmd;

	cmd.buf[0] = 0x15;  /*Subcode 15 */
	bputdouble (datum_coeffs[0], &cmd.buf[1]);
	bputdouble (datum_coeffs[1], &cmd.buf[9]);
	bputdouble (datum_coeffs[2], &cmd.buf[17]);
	bputdouble (datum_coeffs[3], &cmd.buf[25]);
	bputdouble (datum_coeffs[4], &cmd.buf[33]);
	cmd.cnt = 41;
	cmd.code = 0x8E;
	send_cmd (&cmd);
}

void cmd_0x8E20q (void)
/* UTM SP msg query */
{
   TSIPPKT cmd;

	cmd.cnt = 1;
	cmd.code = 0x8E;
	cmd.buf[0] = 0x20;
	send_cmd (&cmd);
}

void cmd_0x8E20d (void)
/* 8F-20 msg auto-output disable */
{
   TSIPPKT cmd;

	cmd.cnt = 2;
	cmd.code = 0x8E;
	cmd.buf[0] = 0x20;
	cmd.buf[1] = 0;
	send_cmd (&cmd);
}

void cmd_0x8E20e (void)
/* 8F-20 msg auto-output enable */
{
   TSIPPKT cmd;

	cmd.cnt = 2;
	cmd.code = 0x8E;
	cmd.buf[0] = 0x20;
	cmd.buf[1] = 1;
	send_cmd (&cmd);
}

void cmd_0x8E41q (void)
/* request manufacturing parameters */
{
   TSIPPKT cmd;

	cmd.cnt = 1;
	cmd.code = 0x8E;
	cmd.buf[0] = 0x41;
	send_cmd (&cmd);
}

void cmd_0x8E42q (void)
/* request production parameters */
{
   TSIPPKT cmd;

	cmd.cnt = 1;
	cmd.code = 0x8E;
	cmd.buf[0] = 0x42;
	send_cmd (&cmd);
}

void cmd_0x8E45s (unsigned char segment)
/* revert to default settings */
{
	TSIPPKT cmd;
   cmd.cnt = 2;
   cmd.code = 0x8E;
   cmd.buf[0] = 0x45;
   cmd.buf[1] = segment;
   send_cmd (&cmd);
}

void cmd_0x8E4Aq (void)
/* request PPS characteristics */
{
   TSIPPKT cmd;

	cmd.cnt = 1;
	cmd.code = 0x8E;
	cmd.buf[0] = 0x4A;
	send_cmd (&cmd);
}

void cmd_0x8E4As (
	unsigned char PPS_driver_switch,
   unsigned char PPS_time_base,
   unsigned char PPS_polarity,
   double PPS_offset,
   float bias_uncert_threshold)
/* set PPS characteristics */
{
	TSIPPKT cmd;

   cmd.code = 0x8E;
 	cmd.buf[0] = 0x4A;
   cmd.buf[1] = PPS_driver_switch;
   cmd.buf[2] = PPS_time_base;
   cmd.buf[3] = PPS_polarity;
   bputdouble (PPS_offset, &cmd.buf[4]);
   bputfloat (bias_uncert_threshold, &cmd.buf[12]);
   cmd.cnt = 16;
   send_cmd (&cmd);
}

void cmd_0x8E4Bq (void)
/* request survey limit */
{
   TSIPPKT cmd;

	cmd.cnt = 1;
	cmd.code = 0x8E;
	cmd.buf[0] = 0x4B;
	send_cmd (&cmd);
}

void cmd_0x8E4Bs (
	unsigned long survey_limit)
/* set survey limit */
{
	TSIPPKT cmd;

   cmd.code = 0x8E;
   cmd.buf[0] = 0x4B;
   bputlong (survey_limit, &cmd.buf[1]);
   cmd.cnt = 5;
   send_cmd (&cmd);
}

void cmd_0x8E4Cs (unsigned char segment)
/* save see segment settings */
{
	TSIPPKT cmd;
   cmd.cnt = 2;
   cmd.code = 0x8E;
   cmd.buf[0] = 0x4C;
   cmd.buf[1] = segment;
   send_cmd (&cmd);
}

void cmd_0x8EA0q (void)
/* request DAC value */
{
  	TSIPPKT cmd;

   cmd.code = 0x8E;
   cmd.buf[0] = 0xA0;
   cmd.cnt = 1;
   send_cmd (&cmd);
}

void cmd_0x8EA0s (float voltage)
/* set DAC voltage */
{
   TSIPPKT cmd;

	cmd.code = 0x8E;
	cmd.buf[0] = 0xA0;
	cmd.buf[1] = 0;	/* set voltage flag */
	bputfloat (voltage, &cmd.buf[2]);
	cmd.cnt = 6;
	send_cmd (&cmd);
}

void cmd_0x8EA0s_2 (unsigned long value)
/* set DAC value */
{
	TSIPPKT cmd;

   cmd.code = 0x8E;
   cmd.buf[0] = 0xA0;
   cmd.buf[1] = 1;   /* set value flag */
   bputlong (value, &cmd.buf[2]);
   cmd.cnt = 6;
   send_cmd (&cmd);
}

void cmd_0x8EA1q (void)
/* request 10MHz Sense */
{
  	TSIPPKT cmd;

   cmd.code = 0x8E;
   cmd.buf[0] = 0xA1;
   cmd.cnt = 1;
   send_cmd (&cmd);
}

void cmd_0x8EA1s (unsigned char MHz_sense)
/* set 10MHz Sense */
{
   TSIPPKT cmd;

   cmd.code = 0x8E;
	cmd.buf[0] = 0xA1;
	cmd.buf[1] = MHz_sense;
   cmd.cnt = 2;
	send_cmd (&cmd);
}

void cmd_0x8EA2q (void)
/* request UTC/GPS timing mode */
{
  	TSIPPKT cmd;

   cmd.code = 0x8E;
   cmd.buf[0] = 0xA2;
   cmd.cnt = 1;
   send_cmd (&cmd);
}

void cmd_0x8EA2s (unsigned char timing_mode)
/* set UTC/GPS timing mode */
{
   TSIPPKT cmd;

   cmd.code = 0x8E;
	cmd.buf[0] = 0xA2;
	cmd.buf[1] = timing_mode;
   cmd.cnt = 2;
	send_cmd (&cmd);
}

void cmd_0x8EA3s (unsigned char disciplining_command)
/* set disciplining command */
{
   TSIPPKT cmd;

   cmd.code = 0x8E;
	cmd.buf[0] = 0xA3;
	cmd.buf[1] = disciplining_command;
   cmd.cnt = 2;
	send_cmd (&cmd);
}

void cmd_0x8EA6s (unsigned char self_survey_command)
/* set self survey command */
{
   TSIPPKT cmd;

   cmd.code = 0x8E;
	cmd.buf[0] = 0xA6;
	cmd.buf[1] = self_survey_command;
   cmd.cnt = 2;
	send_cmd (&cmd);
}

void cmd_0x8E (unsigned char hexbyte[], unsigned char hexnum)
/* turn on superpacket, etc. */
{
	TSIPPKT cmd;
	memcpy (cmd.buf, hexbyte, (short)hexnum);
	cmd.cnt = hexnum;
	cmd.code = 0x8E;
	send_cmd (&cmd);
}

void cmd_0xBB_00s(GL_0xBB00_DATA *TsipxBB)
/* set primary Stinger receiver configuration */
{
	unsigned char
		i;
	TSIPPKT
		cmd;

	cmd.cnt = sizeof(*TsipxBB);
	cmd.code = 0xBB;
    	cmd.buf[offsetof(GL_0xBB00_DATA, subcode)] = TsipxBB->subcode;
    	cmd.buf[offsetof(GL_0xBB00_DATA, operating_mode)] = TsipxBB->operating_mode;
    	cmd.buf[offsetof(GL_0xBB00_DATA, dgps_mode)] = TsipxBB->dgps_mode;
    	cmd.buf[offsetof(GL_0xBB00_DATA, dyn_code)] = TsipxBB->dyn_code;
    	cmd.buf[offsetof(GL_0xBB00_DATA, meas_rate)] = TsipxBB->meas_rate;
    	cmd.buf[offsetof(GL_0xBB00_DATA, pos_rate)] = TsipxBB->pos_rate;
    	cmd.buf[offsetof(GL_0xBB00_DATA, trackmode)] = TsipxBB->trackmode;
    	cmd.buf[offsetof(GL_0xBB00_DATA, clock_hold)] = TsipxBB->clock_hold;
    	cmd.buf[offsetof(GL_0xBB00_DATA, foliage_mode)] = TsipxBB->foliage_mode;
    	cmd.buf[offsetof(GL_0xBB00_DATA, low_power_mode)] = TsipxBB->low_power_mode;
	bputfloat(TsipxBB->elev_mask, &cmd.buf[offsetof(GL_0xBB00_DATA,elev_mask)]);
	bputfloat(TsipxBB->snr_mask, &cmd.buf[offsetof(GL_0xBB00_DATA,snr_mask)]);
	bputfloat(TsipxBB->pdop_mask, &cmd.buf[offsetof(GL_0xBB00_DATA,pdop_mask)]);
	bputfloat(TsipxBB->pdop_switch, &cmd.buf[offsetof(GL_0xBB00_DATA,pdop_switch)]);
	cmd.buf[offsetof(GL_0xBB00_DATA, dgps_age_limit)] = TsipxBB->dgps_age_limit;
	for (i=27; i<40; i++) cmd.buf[i] = 0xFF;	/* extra */
	send_cmd(&cmd);
}

void cmd_0xBBq(unsigned char subcode)
/* request receiver configuration */
{
	TSIPPKT
		cmd;

	cmd.buf[0] = subcode;
	cmd.cnt = 1;
	cmd.code = 0xBB;
	send_cmd(&cmd);
}

void cmd_0xBCq (unsigned char port_num)
/* request receiver port configuration */
{
	TSIPPKT cmd;

	cmd.buf[0] = port_num;
	cmd.cnt = 1;
	cmd.code = 0xBC;
	send_cmd (&cmd);
}

void cmd_0xBCs (
	unsigned char port_num,
	unsigned char in_baud,
	unsigned char out_baud,
	unsigned char data_bits,
	unsigned char parity,
	unsigned char stop_bits,
	unsigned char flow_control,
	unsigned char protocols_in,
	unsigned char protocols_out,
	unsigned char reserved)
/* set receiver port configuration */
{
	TSIPPKT cmd;

	cmd.buf[0] = port_num;
	cmd.buf[1] = in_baud;
	cmd.buf[2] = out_baud;
	cmd.buf[3] = data_bits;
	cmd.buf[4] = parity;
	cmd.buf[5] = stop_bits;
	cmd.buf[6] = flow_control;
	cmd.buf[7] = protocols_in;
	cmd.buf[8] = protocols_out;
	cmd.buf[9] = reserved;
	cmd.cnt = 10;
	cmd.code = 0xBC;
	send_cmd (&cmd);
}

void cmd_0xxx (unsigned char hexcode, unsigned char hexbyte[], unsigned char hexnum)
/* turn on superpacket, etc. */
{
	TSIPPKT cmd;
	memcpy (cmd.buf, hexbyte, (short)hexnum);
	cmd.cnt = hexnum;
	cmd.code = hexcode;
	send_cmd (&cmd);
}
/**/
void tsip_input_proc (TSIPPKT *rpt, short inbyte)
/* reads bytes until serial buffer is empty or a complete report
 * has been received; end of report is signified by DLE ETX.
 */
{                 
	unsigned char newbyte;

	if (inbyte & 0xFF00) return;

	newbyte = (unsigned char)(inbyte & 0x00FF);
	switch (rpt->status)
	{
	case TSIP_PARSED_DLE_1:
		switch (newbyte)
		{
		case 0:
		case DLE:
		case ETX:
			rpt->status = TSIP_PARSED_EMPTY;
			break;
		default:
			rpt->status = TSIP_PARSED_DATA;
			rpt->code = newbyte;
			break;
		}
		break;
	case TSIP_PARSED_DATA:
		switch (newbyte) {
		case DLE:
			rpt->status = TSIP_PARSED_DLE_2;
			break;
		default:
			rpt->buf[rpt->cnt++] = newbyte;
			break;
		}
		break;
	case TSIP_PARSED_DLE_2:
		switch (newbyte) {
		case DLE:
			rpt->status = TSIP_PARSED_DATA;
			rpt->buf[rpt->cnt++] = newbyte;
			break;
		case ETX:
			/* end of message; return TRUE here. */
			rpt->status = TSIP_PARSED_FULL;
			break;
		default:
			/* error: start new report packet */
			rpt->status = TSIP_PARSED_DLE_1;
			rpt->code = newbyte;
		}
		break;
	case TSIP_PARSED_FULL:
	case TSIP_PARSED_EMPTY:
	default:
		rpt->status = (newbyte==DLE) ? TSIP_PARSED_DLE_1 : TSIP_PARSED_EMPTY;
		break;
	}
	if (rpt->cnt > MAX_RPTBUF) {
		/* error: start new report packet */
		rpt->status = TSIP_PARSED_EMPTY;
	}
	if (rpt->status==TSIP_PARSED_EMPTY || rpt->status==TSIP_PARSED_DLE_1)
	{
		rpt->cnt = 0;
	}
}

short rpt_0x42 (TSIPPKT *rpt,
	float pos_ECEF[3],
	float *time_of_fix)
/* position in ECEF, single precision */
{
	unsigned char *buf;
	buf = rpt->buf;
	
	if (rpt->cnt != 16) return TRUE;
	pos_ECEF[0] = bgetfloat (buf);
	pos_ECEF[1]= bgetfloat (&buf[4]);
	pos_ECEF[2]= bgetfloat (&buf[8]);
	*time_of_fix = bgetfloat (&buf[12]);
	set_tsip_time (*time_of_fix, 0);
	return FALSE;
}

short rpt_0x43 (TSIPPKT *rpt,
	float ECEF_vel[3],
	float *freq_offset,
	float *time_of_fix)
/* velocity in ECEF, single precision */
{
	unsigned char *buf;
	buf = rpt->buf;

	if (rpt->cnt != 20) return TRUE;
	ECEF_vel[0] = bgetfloat (buf);
	ECEF_vel[1] = bgetfloat (&buf[4]);
	ECEF_vel[2] = bgetfloat (&buf[8]);
	*freq_offset = bgetfloat (&buf[12]);
	*time_of_fix = bgetfloat (&buf[16]);
	return FALSE;
}

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
	unsigned char *dsp_year)
/* software versions */	
{
	unsigned char *buf;
	buf = rpt->buf;

	if (rpt->cnt != 10) return TRUE;
	*major_nav_version = buf[0];
	*minor_nav_version = buf[1];
	*nav_day = buf[2];
	*nav_month = buf[3];
	*nav_year = buf[4];
	*major_dsp_version = buf[5];
	*minor_dsp_version = buf[6];
	*dsp_day = buf[7];
	*dsp_month = buf[8];
	*dsp_year = buf[9];
	return FALSE;
}

short rpt_0x47 (TSIPPKT *rpt,
	unsigned char *nsvs, unsigned char *sv_prn,
	float *snr)
/* signal levels for all satellites tracked */
{
	short isv;
	unsigned char *buf;
	buf = rpt->buf;

	if (rpt->cnt != 1 + 5*buf[0]) return TRUE;
	*nsvs = buf[0];
	for (isv = 0; isv < (*nsvs); isv++) {
		sv_prn[isv] = buf[5*isv + 1];
		snr[isv] = bgetfloat (&buf[5*isv + 2]);
	}
	return FALSE;
}

short rpt_0x49 (TSIPPKT *rpt,
	unsigned char *sv_health)
/* Almanac health page */
{
   short isv;
	unsigned char *buf;
	buf = rpt->buf;

	if (rpt->cnt != 32) return TRUE;
	for (isv = 0; isv < 32; isv++)
      sv_health[isv] = buf[isv];
	return FALSE;
}

short rpt_0x4A (TSIPPKT *rpt,
	float *lat,
	float *lon,
	float *alt,
	float *clock_bias,
	float *time_of_fix)
/* position in lat-lon-alt, single precision */
{
	unsigned char *buf;
	buf = rpt->buf;

	if (rpt->cnt != 20) return TRUE;
	*lat = bgetfloat (buf);
	*lon = bgetfloat (&buf[4]);
	*alt = bgetfloat (&buf[8]);
	*clock_bias = bgetfloat (&buf[12]);
	*time_of_fix = bgetfloat (&buf[16]);
	set_tsip_time (*time_of_fix, 0);
	return FALSE;
}

short rpt_0x4A_2 (TSIPPKT *rpt,
	float *alt, float *dummy , unsigned char *alt_flag)
/* reference altitude parameters */
{
	unsigned char *buf;

	buf = rpt->buf;

	if (rpt->cnt != 9) return TRUE;
	*alt = bgetfloat (buf);
	*dummy = bgetfloat (&buf[4]);
	*alt_flag = buf[8];
	return FALSE;
}

short rpt_0x4B (TSIPPKT *rpt,
	unsigned char *machine_id,
	unsigned char *status_1,
	unsigned char *status_2)
/* machine ID code, status */
{
	unsigned char *buf;
	buf = rpt->buf;

	if (rpt->cnt != 3) return TRUE;
	*machine_id = buf[0];
	*status_1 = buf[1];
	*status_2 = buf[2];
	return FALSE;
}

short rpt_0x4F (TSIPPKT *rpt,
	double *a0,
	float *a1,
   short *dt_ls,
	float *time_of_data,
	short *wn_t,
	short *wn_lsf,
	short *dn,
	short *dt_lsf)
/* UTC Parameters */
{
	unsigned char *buf;
	buf = rpt->buf;

	if (rpt->cnt != 26) return TRUE;
	*a0 = bgetdouble (buf);
   *a1 = bgetfloat (&buf[8]);
   *dt_ls = bgetshort (&buf[12]);
   *time_of_data = bgetfloat (&buf[14]);
   *wn_t = bgetshort (&buf[18]);
   *wn_lsf = bgetshort (&buf[20]);
   *dn = bgetshort (&buf[22]);
   *dt_lsf = bgetshort (&buf[24]);
	return FALSE;
}

/**/
short rpt_0x55 (TSIPPKT *rpt,
	unsigned char *pos_code,
	unsigned char *vel_code,
	unsigned char *time_code,
	unsigned char *aux_code)
/* I/O serial options */
{
	unsigned char *buf;
	buf = rpt->buf;

	if (rpt->cnt != 4) return TRUE;
	*pos_code = buf[0];
	*vel_code = buf[1];
	*time_code = buf[2];
	*aux_code = buf[3];
	return FALSE;
}

short rpt_0x56 (TSIPPKT *rpt,
	float vel_ENU[3], float *freq_offset, float *time_of_fix)
/* velocity in east-north-up coordinates */
{
	unsigned char *buf;
	buf = rpt->buf;

	if (rpt->cnt != 20) return TRUE;
	/* east */
	vel_ENU[0] = bgetfloat (buf);
	/* north */
	vel_ENU[1] = bgetfloat (&buf[4]);
	/* up */
	vel_ENU[2] = bgetfloat (&buf[8]);
	*freq_offset = bgetfloat (&buf[12]);
	*time_of_fix = bgetfloat (&buf[16]);
	return FALSE;
}

short rpt_0x57 (TSIPPKT *rpt,
	unsigned char *source_code, unsigned char *diag_code,
	short *week_num,
	float *time_of_fix)
/* info about last computed fix */
{
	unsigned char *buf;
	buf = rpt->buf;
	
	if (rpt->cnt != 8) return TRUE;
	*source_code = buf[0];
	*diag_code = buf[1];
	*time_of_fix = bgetfloat (&buf[2]);
	*week_num = bgetshort (&buf[6]);
	return FALSE;
}

short rpt_0x58 (TSIPPKT *rpt,
	unsigned char *op_code, unsigned char *data_type, unsigned char *sv_prn,
	unsigned char *data_length, unsigned char *data_packet)
/* GPS system data or acknowledgment of GPS system data load */
{
	unsigned char *buf, *buf4;
	short dl;
	ALM_INFO* alminfo;
	ION_INFO* ioninfo;
	UTC_INFO* utcinfo;
	NAV_INFO* navinfo;

	buf = rpt->buf;

	if (buf[0] == 2) {
		if (rpt->cnt < 4) return TRUE;
		if (rpt->cnt != 4+buf[3]) return TRUE;
	}
	else if (rpt->cnt != 3) {
		return TRUE;
	}
	*op_code = buf[0];
	*data_type = buf[1];
	*sv_prn = buf[2];
	if (*op_code == 2) {
		dl = buf[3];
		*data_length = (unsigned char)dl;
		buf4 = &buf[4];
		switch (*data_type) {
		case 2:
			/* Almanac */
			if (*data_length != sizeof (ALM_INFO)) return TRUE;
			alminfo = (ALM_INFO*)data_packet;
			alminfo->t_oa_raw  = buf4[0];
			alminfo->SV_health = buf4[1];
			alminfo->e         = bgetfloat(&buf4[2]);
			alminfo->t_oa      = bgetfloat(&buf4[6]);
			alminfo->i_0       = bgetfloat(&buf4[10]);
			alminfo->OMEGADOT  = bgetfloat(&buf4[14]);
			alminfo->sqrt_A    = bgetfloat(&buf4[18]);
			alminfo->OMEGA_0   = bgetfloat(&buf4[22]);
			alminfo->omega     = bgetfloat(&buf4[26]);
			alminfo->M_0       = bgetfloat(&buf4[30]);
			alminfo->a_f0      = bgetfloat(&buf4[34]);
			alminfo->a_f1      = bgetfloat(&buf4[38]);
			alminfo->Axis      = bgetfloat(&buf4[42]);
			alminfo->n         = bgetfloat(&buf4[46]);
			alminfo->OMEGA_n   = bgetfloat(&buf4[50]);
			alminfo->ODOT_n    = bgetfloat(&buf4[54]);
			alminfo->t_zc      = bgetfloat(&buf4[58]);
			alminfo->weeknum   = bgetshort(&buf4[62]);
			alminfo->wn_oa     = bgetshort(&buf4[64]);
			break;

		case 3:
			/* Almanac health page */
			if (*data_length != sizeof (ALH_PARMS) + 3) return TRUE;

			/* this record is returned raw */
			memcpy (data_packet, buf4, dl);
			break;

		case 4:
			/* Ionosphere */
			if (*data_length != sizeof (ION_INFO) + 8) return TRUE;
			ioninfo = (ION_INFO*)data_packet;
			ioninfo->alpha_0   = bgetfloat (&buf4[8]);
			ioninfo->alpha_1   = bgetfloat (&buf4[12]);
			ioninfo->alpha_2   = bgetfloat (&buf4[16]);
			ioninfo->alpha_3   = bgetfloat (&buf4[20]);
			ioninfo->beta_0    = bgetfloat (&buf4[24]);
			ioninfo->beta_1    = bgetfloat (&buf4[28]);
			ioninfo->beta_2    = bgetfloat (&buf4[32]);
			ioninfo->beta_3    = bgetfloat (&buf4[36]);
			break;

		case 5:
			/* UTC */
			if (*data_length != sizeof (UTC_INFO) + 13) return TRUE;
			utcinfo = (UTC_INFO*)data_packet;
			utcinfo->A_0       = bgetdouble (&buf4[13]);
			utcinfo->A_1       = bgetfloat (&buf4[21]);
			utcinfo->delta_t_LS = bgetshort (&buf4[25]);
			utcinfo->t_ot      = bgetfloat(&buf4[27]);
			utcinfo->WN_t      = bgetshort (&buf4[31]);
			utcinfo->WN_LSF    = bgetshort (&buf4[33]);
			utcinfo->DN        = bgetshort (&buf4[35]);
			utcinfo->delta_t_LSF = bgetshort (&buf4[37]);
			break;

		case 6:
			/* Ephemeris */
			if (*data_length != sizeof (NAV_INFO) - 1) return TRUE;

			navinfo = (NAV_INFO*)data_packet;

			navinfo->sv_number = buf4[0];
			navinfo->t_ephem = bgetfloat (&buf4[1]);
			navinfo->ephclk.weeknum = bgetshort (&buf4[5]);

			navinfo->ephclk.codeL2 = buf4[7];
			navinfo->ephclk.L2Pdata = buf4[8];
			navinfo->ephclk.SVacc_raw = buf4[9];
			navinfo->ephclk.SV_health = buf4[10];
			navinfo->ephclk.IODC = bgetshort (&buf4[11]);
			navinfo->ephclk.T_GD = bgetfloat (&buf4[13]);
			navinfo->ephclk.t_oc = bgetfloat (&buf4[17]);
			navinfo->ephclk.a_f2 = bgetfloat (&buf4[21]);
			navinfo->ephclk.a_f1 = bgetfloat (&buf4[25]);
			navinfo->ephclk.a_f0 = bgetfloat (&buf4[29]);
			navinfo->ephclk.SVacc = bgetfloat (&buf4[33]);

			navinfo->ephorb.IODE = buf4[37];
			navinfo->ephorb.fit_interval = buf4[38];
			navinfo->ephorb.C_rs = bgetfloat (&buf4[39]);
			navinfo->ephorb.delta_n = bgetfloat (&buf4[43]);
			navinfo->ephorb.M_0 = bgetdouble (&buf4[47]);
			navinfo->ephorb.C_uc = bgetfloat (&buf4[55]);
			navinfo->ephorb.e = bgetdouble (&buf4[59]);
			navinfo->ephorb.C_us = bgetfloat (&buf4[67]);
			navinfo->ephorb.sqrt_A = bgetdouble (&buf4[71]);
			navinfo->ephorb.t_oe = bgetfloat (&buf4[79]);
			navinfo->ephorb.C_ic = bgetfloat (&buf4[83]);
			navinfo->ephorb.OMEGA_0 = bgetdouble (&buf4[87]);
			navinfo->ephorb.C_is = bgetfloat (&buf4[95]);
			navinfo->ephorb.i_0 = bgetdouble (&buf4[99]);
			navinfo->ephorb.C_rc = bgetfloat (&buf4[107]);
			navinfo->ephorb.omega = bgetdouble (&buf4[111]);
			navinfo->ephorb.OMEGADOT=bgetfloat (&buf4[119]);
			navinfo->ephorb.IDOT = bgetfloat (&buf4[123]);
			navinfo->ephorb.Axis = bgetdouble (&buf4[127]);
			navinfo->ephorb.n = bgetdouble (&buf4[135]);
			navinfo->ephorb.r1me2 = bgetdouble (&buf4[143]);
			navinfo->ephorb.OMEGA_n=bgetdouble (&buf4[151]);
			navinfo->ephorb.ODOT_n = bgetdouble (&buf4[159]);
			break;
		}
	}
	return FALSE;
}

short rpt_0x59 (TSIPPKT *rpt,
	unsigned char *code_type,
	unsigned char status_code[32])
/* satellite enable/disable or health heed/ignore list */	
{
	short iprn;
	unsigned char *buf;
	buf = rpt->buf;
	
	if (rpt->cnt != 33) return TRUE;
	*code_type = buf[0];
	for (iprn = 0; iprn < 32; iprn++)
		status_code[iprn] = buf[iprn + 1];
	return FALSE;
}

short rpt_0x6D (TSIPPKT *rpt,
	unsigned char *manual_mode,
	unsigned char *nsvs,
	unsigned char *ndim,
	char sv_prn[],
	float *pdop,
	float *hdop,
	float *vdop,
	float *tdop)
/* over-determined satellite selection for position fixes, PDOP, fix mode */
{
	short islot;
	unsigned char *buf;
	buf = rpt->buf;

	*nsvs = (buf[0] & 0xF0) >> 4;
	if ((*nsvs)>8) return TRUE;
	if (rpt->cnt != 17 + (*nsvs) ) return TRUE;

	*manual_mode = buf[0] & 0x08;
	*ndim  = (buf[0] & 0x07);
	*pdop = bgetfloat (&buf[1]);
	*hdop = bgetfloat (&buf[5]);
	*vdop = bgetfloat (&buf[9]);
	*tdop = bgetfloat (&buf[13]);
	for (islot = 0; islot < (*nsvs); islot++)
		sv_prn[islot] = buf[islot + 17];
	return FALSE;
}

short rpt_0x70(TSIPPKT *rpt,
               unsigned char *dyn_switch,
               unsigned char *static_switch,
               unsigned char *alt_switch,
               unsigned char *extra)
{
	unsigned char
		*buf;

	buf = rpt->buf;
	if (rpt->cnt != 4) return TRUE;
	*dyn_switch = buf[0];
	*static_switch = buf[1];
	*alt_switch = buf[2];
	*extra = buf[3];
	return FALSE;
}

short rpt_0x83 (TSIPPKT *rpt,
	double ECEF_pos[3],
	double *clock_bias,
	float *time_of_fix)
/* position, ECEF double precision */
{
	unsigned char *buf;
	buf = rpt->buf;

	if (rpt->cnt != 36) return TRUE;
	ECEF_pos[0] = bgetdouble (buf);
	ECEF_pos[1] = bgetdouble (&buf[8]);
	ECEF_pos[2] = bgetdouble (&buf[16]);
	*clock_bias  = bgetdouble (&buf[24]);
	*time_of_fix = bgetfloat (&buf[32]);
	set_tsip_time (*time_of_fix, 0);
	return FALSE;
}

short rpt_0x84 (TSIPPKT *rpt,
	double *lat,
	double *lon,
	double *alt,
	double *clock_bias,
	float *time_of_fix)
/* position, lat-lon-alt double precision */	
{
	unsigned char *buf;
	buf = rpt->buf;
	
	if (rpt->cnt != 36) return TRUE;
	*lat = bgetdouble (buf);
	*lon = bgetdouble (&buf[8]);
	*alt = bgetdouble (&buf[16]);
	*clock_bias = bgetdouble (&buf[24]);
	*time_of_fix = bgetfloat (&buf[32]);
	set_tsip_time (*time_of_fix, 0);
	return FALSE;
}

short rpt_0x8F15 (TSIPPKT *rpt,
	short *datum_idx,
	double datum_coeffs[5])
/*  datum index and coefficients  */
{
	unsigned char *buf;
	buf = rpt->buf;

	if (rpt->cnt != 43) return TRUE;
	*datum_idx = bgetshort(&buf[1]);
	datum_coeffs[0] = bgetdouble (&buf[3]);
	datum_coeffs[1] = bgetdouble (&buf[11]);
	datum_coeffs[2] = bgetdouble (&buf[19]);
	datum_coeffs[3] = bgetdouble (&buf[27]);
	datum_coeffs[4] = bgetdouble (&buf[35]);
	return FALSE;
}

#define MAX_LONG  (2147483648.)   /* 2**31 */
short rpt_0x8F20 (TSIPPKT *rpt,
	unsigned char *subpacket_id,
	unsigned char *info,
	double *lat,
	double *lon,
	double *alt,
	double vel_enu[],
	double *time_of_fix,
	short *week_num,
	unsigned char *nsvs,
	unsigned char sv_prn[],
	short sv_IODC[],
	short *datum_index)
{
	short
		isv;
	unsigned char
		*buf, prnx;
	unsigned long
		ulongtemp;
	long
		longtemp;
	double
		vel_scale;

	buf = rpt->buf;

	if (rpt->cnt != 56) return TRUE;
	*subpacket_id = buf[0];

	vel_scale = (buf[24]&1)? 0.020 : 0.005;
	vel_enu[0] = bgetshort (buf+2)*vel_scale;
	vel_enu[1] = bgetshort (buf+4)*vel_scale;
	vel_enu[2] = bgetshort (buf+6)*vel_scale;

	*time_of_fix = bgetulong (buf+8)*.001;

	longtemp = bgetlong (buf+12);
	*lat = longtemp*(GPS_PI/MAX_LONG);

	ulongtemp = bgetulong (buf+16);
	*lon = ulongtemp*(GPS_PI/MAX_LONG);
	if (*lon > GPS_PI) *lon -= 2.0*GPS_PI;

	*alt = bgetlong (buf+20)*.001;
	/* 25 blank; 29 = UTC */
	*datum_index = (short)buf[26] - 1;
	*info = buf[27];
	*nsvs = buf[28];
	*week_num = bgetshort (&buf[30]);
	for (isv = 0; isv < 8; isv++) {
		prnx = buf[32+2*isv];
		sv_prn[isv] = prnx&0x3F;
		sv_IODC[isv] = buf[32 + 2 * isv + 1] + (short)(prnx - sv_prn[isv]) * 4;
	}
	set_tsip_time (*time_of_fix, *week_num);
	return FALSE;
}

short rpt_0x8F41 (TSIPPKT *rpt,
   unsigned char *subpacket_id,
   short *serial_num_prefix,
   float *serial_num,
   unsigned char *year,
   unsigned char *month,
   unsigned char *day,
   unsigned char *hour,
   float *oscillator_offset,
   short *test_code_id_num)
/* show manufacturing operating parameters */
{
   unsigned char *buf;
   buf = rpt->buf;

   if (rpt->cnt != 17) return TRUE;
   *subpacket_id = buf[0];
   *serial_num_prefix = bgetshort (&buf[1]);
   *serial_num = bgetfloat (&buf[3]);
   *year = buf[7];
   *month = buf[8];
   *day = buf[9];
   *hour = buf[10];
   *oscillator_offset = bgetfloat (&buf[11]);
   *test_code_id_num = bgetshort (&buf[15]);
   return FALSE;
}

short rpt_0x8F42 (TSIPPKT *rpt,
	unsigned char *subpacket_id,
   unsigned char *prod_opt_prefix,
   unsigned char *prod_num_extension,
   short *case_serial_num_prefix,
   unsigned long *case_serial_num,
   unsigned long *production_num,
   short *machine_ID_num)
/* show production parameters */
{
  	unsigned char *buf;
   buf = rpt->buf;

   if (rpt->cnt != 19) return TRUE;
   *subpacket_id = buf[0];
   *prod_opt_prefix = buf[1];
   *prod_num_extension = buf [2];
   *case_serial_num_prefix = bgetshort (&buf[3]);
   *case_serial_num = bgetlong (&buf[5]);
   *production_num = bgetlong (&buf[9]);
   *machine_ID_num = bgetshort (&buf[15]);
   return FALSE;
}

short rpt_0x8F4A (TSIPPKT *rpt,
	unsigned char *subpacket_id,
   unsigned char *PPS_driver_switch,
   unsigned char *PPS_time_base,
   unsigned char *PPS_polarity,
	double *PPS_offset,
   float *bias_uncert_threshold)
/* show PPS characteristics */
{
 	unsigned char *buf;
   buf = rpt->buf;

   if (rpt->cnt != 16) return TRUE;
   *subpacket_id = buf[0];
   *PPS_driver_switch = buf[1];
   *PPS_time_base = buf[2];
   *PPS_polarity = buf[3];
   *PPS_offset = bgetdouble (&buf[4]);
   *bias_uncert_threshold = bgetfloat (&buf[12]);
   return FALSE;
}

short rpt_0x8F4B (TSIPPKT *rpt,
	unsigned char *subpacket_id,
   unsigned long *survey_limit)
/* show survey limit */
{
	unsigned char *buf;
   buf = rpt->buf;

   if (rpt->cnt != 5) return TRUE;
   *subpacket_id = buf[0];
   *survey_limit = bgetlong (&buf[1]);
   return FALSE;
}

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
   unsigned short *year)
/* primary timing packet */
{
	unsigned char *buf;
   buf = rpt->buf;

   if (rpt->cnt != 17) return TRUE;
   *subpacket_id = buf[0];
   *time_of_week = bgetlong (&buf[1]);
   *week_number = bgetshort (&buf[5]);
   *UTC_offset = bgetshort (&buf[7]);
   *timing_flag = buf[9];
   *seconds = buf[10];
   *minutes = buf[11];
   *hours = buf[12];
   *day_of_month = buf[13];
   *month = buf[14];
   *year = bgetshort (&buf[15]);
   return FALSE;
}

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
   double *altitude)
/* supplemental timing packet */
{
	unsigned char *buf;
   buf = rpt->buf;

   if (rpt->cnt != 68) return TRUE;
   *subpacket_id = buf[0];
   *receiver_mode = buf[1];
   *discipling_mode = buf[2];
   *survey_progress = buf[3];
   *holdover_duration = bgetlong (&buf[4]);
   *critical_alarms = bgetshort (&buf[8]);
   *minor_alarms = bgetshort (&buf[10]);
   *GPS_decoding_status = buf[12];
   *discipling_activity = buf[13];
   *spare_stat1 = buf[14];
   *spare_stat2 = buf[15];
   *PPS_quality = bgetfloat (&buf[16]);
   *MHz_quality = bgetfloat (&buf[20]);
   *DAC_value = bgetlong (&buf[24]);
   *DAC_voltage = bgetfloat (&buf[28]);
   *temp = bgetfloat (&buf[32]);
   *latitude = bgetdouble (&buf[36]);
   *longitude = bgetdouble (&buf[44]);
   *altitude = bgetdouble (&buf[52]);
   return FALSE;
}

short rpt_0x8FA0 (TSIPPKT *rpt,
	unsigned char *subpacket_id,
   unsigned long *DAC_value,
   float *DAC_voltage,
   unsigned char *DAC_resolution,
   unsigned char *DAC_data_format,
   float *min_DAC_voltage,
   float *max_DAC_voltage)
/* DAC Value report */
{
	unsigned char *buf;
   buf = rpt->buf;

   if (rpt->cnt != 19) return TRUE;
   *subpacket_id = buf[0];
   *DAC_value = bgetlong (&buf[1]);
   *DAC_voltage = bgetfloat (&buf[5]);
   *DAC_resolution = buf[9];
   *DAC_data_format = buf[10];
   *min_DAC_voltage = bgetfloat (&buf[11]);
   *max_DAC_voltage = bgetfloat (&buf[15]);
   return FALSE;
}

short rpt_0x8FA1 (TSIPPKT *rpt,
	unsigned char *subpacket_id,
   unsigned char *MHz_sense)
/* 10MHz Sense report */
{
  	unsigned char *buf;
   buf = rpt->buf;

   if (rpt->cnt != 2) return TRUE;
   *subpacket_id = buf[0];
   *MHz_sense = buf[1];
   return FALSE;
}

short rpt_0x8FA2 (TSIPPKT *rpt,
	unsigned char *subpacket_id,
   unsigned char *timing_mode)
/* GPS/UTC timing mode report */
{
  	unsigned char *buf;
   buf = rpt->buf;

   if (rpt->cnt != 2) return TRUE;
   *subpacket_id = buf[0];
   *timing_mode = buf[1];
   return FALSE;
}

short rpt_0xBB_00(TSIPPKT *rpt,
                  GL_0xBB00_DATA *TsipxBB)
{

	unsigned char
		*buf;

	buf = rpt->buf;
	if (rpt->cnt != sizeof(*TsipxBB)) return TRUE;
	TsipxBB->subcode = buf[offsetof(GL_0xBB00_DATA, subcode)];
	if (TsipxBB->subcode != 0) return TRUE;

	TsipxBB->operating_mode	= buf[offsetof(GL_0xBB00_DATA, operating_mode)];
	TsipxBB->dgps_mode = buf[offsetof(GL_0xBB00_DATA, dgps_mode)];
	TsipxBB->dyn_code = buf[offsetof(GL_0xBB00_DATA, dyn_code)];
	TsipxBB->trackmode = buf[offsetof(GL_0xBB00_DATA, trackmode)];
	TsipxBB->elev_mask = bgetfloat(&buf[offsetof(GL_0xBB00_DATA,elev_mask)]);
	TsipxBB->snr_mask = bgetfloat(&buf[offsetof(GL_0xBB00_DATA,snr_mask)]);
	TsipxBB->pdop_mask = bgetfloat(&buf[offsetof(GL_0xBB00_DATA,pdop_mask)]);
	TsipxBB->pdop_switch = bgetfloat(&buf[offsetof(GL_0xBB00_DATA,pdop_switch)]);
	TsipxBB->dgps_age_limit	= buf[offsetof(GL_0xBB00_DATA,dgps_age_limit)];
	TsipxBB->foliage_mode = buf[offsetof(GL_0xBB00_DATA, foliage_mode)];
	TsipxBB->low_power_mode	= buf[offsetof(GL_0xBB00_DATA, low_power_mode)];
	TsipxBB->clock_hold = buf[offsetof(GL_0xBB00_DATA, clock_hold)];
	TsipxBB->meas_rate = buf[offsetof(GL_0xBB00_DATA, meas_rate)];
	TsipxBB->pos_rate = buf[offsetof(GL_0xBB00_DATA, pos_rate)];
	return FALSE;
}

short rpt_0xBC (TSIPPKT *rpt, unsigned char *port_num, unsigned char *in_baud,
	unsigned char *out_baud, unsigned char *data_bits,
	unsigned char *parity, unsigned char *stop_bits, unsigned char *flow_control,
	unsigned char *protocols_in, unsigned char *protocols_out, unsigned char *reserved)
/* Receiver serial port configuration */
{
	unsigned char *buf;
	buf = rpt->buf;

	if (rpt->cnt != 10) return TRUE;
	*port_num = buf[0];
	*in_baud = buf[1];
	*out_baud = buf[2];
	*data_bits = buf[3];
	*parity = buf[4];
	*stop_bits = buf[5];
	*flow_control = buf[6];
	*protocols_in = buf[7];
	*protocols_out = buf[8];
	*reserved = buf[9];

	return FALSE;
}
