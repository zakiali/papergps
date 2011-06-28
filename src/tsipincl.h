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
 * Vers  Date        Changes                                      Author
 * ----  ---------   ----------------------------------------     ----------
 * 1.1   21 Jun 93   Initial version                              pvwl
 * 1.11  17 Jul 95   Revisions                                    ahl
 * 		11 Feb 98 	Converted to tboltcht								jah
 *
 * *************************************************************************
 *
 * The following modules require TSIPINCL.H:
 *
 *    TSIPCHAT.C:  main program for communicating with TSIP devices
 *    TSIP_UTL.C:  user interface funtions
 *    TSIP_CMD.C:  command generator
 *    TSIP_RPT.C:  report interpreter
 *    TSIP_IFC.C:  standard command/report interface for TSIP
 *    TSIP_ALM.C:  grabs almanacs from, and stuffs them into, receiver
 *    TSIP_SIO.C:  serial choice routines
 *    T_SERIAL.C:  serial buffering routines
 *
 * This source code is supplied without warranty and is intended as sample
 * source code for exercising the TSIP interface.  We do appreciate comments
 * and try to support the software as much as possible.
 *
 * Developers of new applications are encouraged to use the functions in
 * TSIP_IFC.C, and pattern their drivers on main() in TCHAT.C
 *
 * *************************************************************************
 *
 */

#define TSIPCHAT_VERNUM "1.40"

#define DIAG_MENU    -3
#define REPEAT_MENU  -2
#define ALL_MENU     -1
#define x79_MENU     0x79
#define x8E_MENU     0x8E

#define OFF 	0
#define ON		1

/* Defines from Stinger Serial port config/TP task */

#define PORT_HS_NONE				0
#define PORT_HS_RTS_CTS			1
#define PORT_HS_TX_XON_XOFF	2
#define PORT_HS_TX_XANY			4  /* requires PORT_HS_TX_XON_XOFF as well */
#define PORT_HS_RX_XON_XOFF	8

#define BAUD_TEXT_LEN    	       10
#define DATABIT_TEXT_LEN 	        4
#define DGPS_MODE_TEXT_LEN	        5
#define DYN_TEXT_LEN			        5
#define FOLIAGE_MODE_TEXT_LEN      3
#define LOW_POWER_TEXT_LEN         3
#define MEAS_TEXT_LEN              3
#define PARITY_TEXT_LEN  	        3
#define POS_FIX_TEXT_LEN	        8
#define POS_RATE_TEXT_LEN          4
#define RCVR_PORT_TEXT_LEN	        3
#define STOPBIT_TEXT_LEN        	  2
#define TRACKMODE_TEXT_LEN	        3

#define ABLE_TEXT_LEN		4
#define ALT_TEXT_LEN		2
#define BAUD_TEXT_0x3D_LEN	10
#define BITS_PER_BYTETEXT_0x3D_LEN	4
#define FLOW_CNTRL_TEXT_LEN	5
#define FRMT_TEXT_LEN		2
#define IN_OUT_TEXT_LEN		4
#define MODE_TEXT_LEN		4
#define OP_TEXT_LEN			7
#define PARITYTEXT_0x3D_LEN 5
#define PREC_TEXT_LEN		2
#define PROTOCOLS_TEXT_LEN	5
#define PORT_TEXT_LEN		3
#define QUERY_TEXT_LEN		2
#define RAW_TEXT_LEN			2
#define REQ_SET_TEXT_LEN   2
#define RX_MODE_TEXT_LEN  	2
#define SNR_TEXT_LEN			2
#define STOP_BIT_TEXT_0x3D_LEN 3
#define TIME_TEXT_LEN		2
#define TOGGLE_TEXT_LEN 	2
#define TX_MODE_TEXT_LEN	6
#define TYPE_TEXT_LEN		7
#define MAX_SERIAL_TEXT_LEN     10

#define TAIP	0x01
#define TSIP	0x02
#define NMEA	0x04
#define RTCM	0x08
#define DCOL	0x10

#define BYTE_UNCHANGED	                0xFF
#define SIGNED_UNCHANGED               -1
#define UNSIGNED_UNCHANGED              0xFFFF

/* data base for keyboard command help screen */
typedef struct {
	char kb_stroke[3];      /* description if keystroke (e.g. '^F') */
	char kbch;              /* ASCII code for keystroke */
	unsigned short cmdcode;       /* TSIP command code */
	unsigned short replycode;       /* TSIP command code */
	char cmdtext[18];       /* description of TSIP command */
} KB_CMD;


/* If Borland is used, use console print; otherwise use stdout print */
#define CMDWIN       1
#define AUTOWIN      0
#define TAIPFILE     0
#define TSIPFILE     1
#define AUTOFILE     2
#define MAX_BAUD     10

/**/
/* functions called from main() in TSIPCHAT.C and TSIP_UTL.C */
void wait_for_report
   (TSIPPKT *rpt,
	unsigned char rpt_code);
unsigned char read_rpts_wait_for_kbhit
	(void);

#define  reply_code(rpt)   ((rpt)->code == 0x8F) ? \
						   ((rpt)->buf[0] + 0x100) : ((rpt)->code)
/* #define xprintf cprintf  */
#define xprintf printf 
/* in TSIP_CMD.C */
void help_screen
	(void);
void proc_kbd
   (unsigned char kch,
	short *reply);
/* in TSIP_RPT.C */
void rpt_packet
   (TSIPPKT *rpt);
short menu_cmd_prompt
	(short code,
	KB_CMD *kb_cmd);
void print_msg_table_header
	(unsigned char rptcode);
void print_primary_stinger_rcvr_config
       (GL_0xBB00_DATA *TsipxBB);
short ask_primary_stinger_rcvr_config
       (GL_0xBB00_DATA *TsipxBB);
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
	unsigned char reserved);

/************************** TSIP_ALM.C **********************************/
/* functions called from TSIP_CMD.C */
short almgetb (void);
short almputb (void);

/************************** TSIP_UTL.C **********************************/
/* Utility functions for console display */
/* put carriage return & linefeed */
void show_crlf
	(void);
/* print time of week as day, hour, minute, second */
void show_time
	(float time);
/* ask user for verification of choice */
short ask_verify
	(void);
/* without introducing new-line/clear window to end of line */
short ask_verify_nocmdcrlf
   (void);
/* prompt user to input a double */
double ask_dbl
	(char *prompt);
/* prompt user to input an unsigned long */
unsigned long ask_unsigned_long
	(char *prompt);
/* prompt user to input an unsigned short */
unsigned short ask_unsigned_short
	(char *prompt);
/* prompt user to input an integer */
short ask_byte
	(char *prompt);
/*  prompt for an integer from user  */
short ask_int2 (char *prompt);
/* ask for hex byte string, returns number of bytes read */
unsigned char ask_hex_string (
   	char *prompt,
	unsigned char *h,
	unsigned char maxchar);
unsigned char ask_char_string
   (char *prompt,
	char *h);
/* prompt user to pick among a finite number of choices */
short pick_one
   (char *prompt,
	char *code_txt[],
	short nopts);
void ask_4_hex
   (char *prompt,
	short *hex1,
	short *hex2,
	short *hex3,
	short *hex4);
short ask_AB
	(void);
/* print out TAIP long nav packets into a text file; called from  */
/* read_rpts_wait_for_kbhit() in TSIPCHAT.C */
void taip_output
   (TSIPPKT *rpt,
	FILE *taipstream);
/* DOS functions to set time */
void request_PC_time_set
	(void);
void GPS_time_from_PC
	(double *time);
void GPS_time_to_PC
	(double time);
/* BORLAND window functions */
void switch_window
	(short window_id);
void clr_cmd_window
	(short mode);
void start_menu_window
	(void);
void end_menu_window
	(void);
void new_cmd_window
	(void);
short which_window
	(void);
void initialize_screen
	(void);
void reset_screen
	(void);
/* file storage control */
void open_file_storage
	(FILE **stream, char *filename, short binasc);
void close_file_storage
	(FILE **stream, char *filename, short binasc);
short get_byte
	(void);
short send_byte
	(unsigned char outbyte);
short   serial_port_select
	(short *io_code);
void set_buf_serial_port
	(void);
short  initialize_serial_port
	(short comport);
void  reset_serial_port
	(void);
void set_current_port_to_4800_8n1
	(void);
short cmd_esc
	(void);
void set_cmd_esc
	(short x);

/************************** TSIP_TNL.C **********************************/
/* prototypes */
#ifdef TNL
#define BANNER_TEXT	YELLOW
#define BANNER_BACKGROUND	RED
short tnl_cmd 	(unsigned char kbch, KB_CMD *kb_cmd);
short tnl_rpt        (TSIPPKT *rpt);
void get_tnl_menu (KB_CMD **kb_cmd_list, short *num_kb_cmds);
#else
#define BANNER_TEXT	WHITE
#define BANNER_BACKGROUND	BLUE
#endif
/************************* TSIP_LIB.LIB *********************************/
void  initsio (
   short port,            // 1 = COM1, 2 = COM2
	unsigned short speed,           // baud (2400, 9600, etc.)
	short bits,            // 7 or 8 data bits
	short stopbits,        // 1 or 2 stopbits
	short parity);         // none, odd, even
short   get_char (        // returns -1 for no char waiting, 0-FF otherwise
	short port);           // 1 or 2
short   put_char(         // returns 0 for sucess, 1 for failure
	short port,            // 1 or 2
	unsigned char x);    // byte to be transmitted
void  closeserial(      // closes all serial buffers
	void);
