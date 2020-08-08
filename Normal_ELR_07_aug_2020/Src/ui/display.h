/**
  ******************************************************************************
  * @file           : mfm_display.h
  * @brief          : Header for mfm_display.c file.
  *                   This file contains the common defines of the 
											display application.
  ******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MFM_DISPLAY_H
#define __MFM_DISPLAY_H


#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------*/
	
//#include "mfm_common_def.h"
#include "stm32f0xx_hal.h" // Hardware header file
//#include "ebdg.h"
#include "disp_comn_vari.h"
#include "Rms.h"
/******************************************************************************
UI Configurations
******************************************************************************/	 
#define CFG_MFM_DISP_BTN_RELEASE_EVENT (0)
#define CFG_MFM_DISP_BTN_LONGPRESS_EVENT (1)
	 
/* Segment Map
D3 | D2 | D1 | D0	-	Row1

ABCDEFGHIJ

A-G=7-segment digits
H=DP, DP, DP, i, DP, DP, DP, r, DP, DP, DP, $
I=a, c, e, g, j, l, n, p, s, u, w, y
J=b, d, f, h, k, m, o, q, t, v, x, z 
*/

/* following enum defines display indicator bitmask, 
	which will be used for determining 'indicator status' value.
*/
typedef enum
{
	// Shares data line with Digit 4
		DISP_IND_Pr  = 0x00000001,	//  2
		DISP_IND_5   = 0x00000002,	//  6
    DISP_IND_A   = 0x00000004,	//  -
    DISP_IND_EE  = 0x00000008,	//	dp bit of digit
 	// Shares data line with Digit 3
		DISP_IND_4   = 0x00000010,	//  3
		DISP_IND_V   = 0x00000020,	//	Mega
    DISP_IND_DD  = 0x00000040,	//  -
	// Shares data line with Digit 2
		DISP_IND_3   = 0x00000080,	//  4
		DISP_IND_M   = 0x00000100,	//  V
		DISP_IND_CC  = 0x00000200,	//  -
	// Shares data line with Digit 1
		DISP_IND_2   = 0x00000400,	//  5
		DISP_IND_6   = 0x00000800,	//  A
		DISP_IND_BB  = 0x00001000,	//  -

	// Shares data line with Digit 4
	// extra digit 4
    DISP_IND_MV      = 0x00002000,	//  -
    DISP_IND_MINUS   = 0x00004000,
    DISP_IND_k       = 0x00008000,
    DISP_IND_m       = 0x00010000,
    DISP_IND_S       = 0x00020000,
    DISP_IND_R1      = 0x00040000,
    DISP_IND_R2      = 0x00080000,
    DISP_IND_PK      = 0x00100000,
    DISP_IND_1       = 0x00200000,  //  1
    DISP_IND_SV      = 0x00400000,	//  SV
    DISP_IND_AA      = 0x00800000,	//  -

   } dispIndicator_t;




/******************************************************************************
Hardware Configurations
******************************************************************************/
typedef enum
{
	DISP_ROW_1=0,
	DISP_ROW_2,
	DISP_ROW_3,
	
	DISP_ROW_SIZE,
	zero = 0,
	one,
	two,
	three,
	four,
	
} DISP_ROW_ID;




 


/*
	7-Segment BCD Conversion Table:
	
Idx	Digit DP G F E D C B A 	
0		0     0 0 1 1 1 1 1 1 -> 0x3F	
1		1     0 0 0 0 0 1 1 0 -> 0x06	
2		2     0 1 0 1 1 0 1 1 -> 0x5B	
3		3     0 1 0 0 1 1 1 1 -> 0x4F	
4		4     0 1 1 0 0 1 1 0 -> 0x66	
5		5     0 1 1 0 1 1 0 1 -> 0x6D	
6		6     0 1 1 1 1 1 0 1 -> 0x7D	
7		7     0 0 0 0 0 1 1 1 -> 0x07	
8		8     0 1 1 1 1 1 1 1 -> 0x7F	
9		9     0 1 1 0 1 1 1 1 -> 0x6F	
10	A	
11	B	
12	C	    0 0 1 1 1 0 0 1 -> 0x39
13	D	
14	E	
15	F	
16	G	
17	H	
18	I	
19	J	
20	K	
21	L	
22	M	
23	N	
24	O	
25	P	
26	Q	
27	R	
28	S	
29	T	
30	U	
31	V	
32	W	
33	X	
34	Y	
35	Z	
36	a	
37	b	
38	c	
39	d	
40	e	
41	f	
42	g	
43	h	
44	i	
45	j	
46	k	
47	l	
48	m	
49	n	
50	o	
51	p	
52	q	
53	r	
54	s	
55	t	    0 1 1 1 1 0 0 0 -> 0x78	
56	u	
57	v	
58	w	
59	x	
60	y	
61	z	
62	-		


*/

typedef enum 
{
	BCD_NONE = 0x00,
	
	BCD_0 = 0x3F,
	BCD_1 = 0x06,
	BCD_2 = 0x5B,
	BCD_3 = 0x4F,
	BCD_4 = 0x66,
	BCD_5 = 0x6D,
	BCD_6 = 0x7D,
	BCD_7 = 0x07,
	BCD_8 = 0x7F,
	BCD_9 = 0x6F,
	
	BCD_A = 0x77,
	BCD_B = 0x7c,
	BCD_C = 0x39,
	BCD_D = 0x5E,
	BCD_E = 0x79,
	BCD_F = 0x71,
	BCD_G = 0x7d,
	BCD_H = 0x76,
	BCD_I = 0x30,
	BCD_J = 0x1e,
	BCD_K = 0x70,
	BCD_L = 0x38,
	BCD_M = 0x55,
	BCD_N = 0x54,
	BCD_O = 0x3f,
	BCD_P = 0x73,
	BCD_Q = 0x67 ,
	BCD_R = 0x50,
	BCD_S = 0x6D,
	BCD_T = 0x78,
	BCD_U = 0x3e,
	BCD_V = 0x1c,
	BCD_W = 0x1d,
	BCD_X = 0x00,
	BCD_Y = 0x6E,
	BCD_Z = 0x00,
	
	BCD_a = 0x77,
	BCD_b = 0x7c,
	BCD_c = 0x39,
	BCD_d = 0x5E,
	BCD_e = 0x79,
	BCD_f = 0x00,
	BCD_g = 0x6f,
	BCD_h = 0x34,
	BCD_i = 0x10,
	BCD_j = 0x1e,
	BCD_k = 0x70,
	BCD_l = 0x38,
	BCD_m = 0x55,
	BCD_n = 0x54,
	BCD_o = 0x5c,
	BCD_p = 0x73,
	BCD_q = 0x67,
	BCD_r = 0x50,
	BCD_s = 0x6D,
	BCD_t = 0x78,
	BCD_u = 0x3e,
	BCD_v = 0x1c,
	BCD_w = 0x1d,
	BCD_x = 0x00,
	BCD_y = 0x6E,
	BCD_z = 0x00,
	
	BCD_OR = 0x30,
	BCD_MINUS = 0x40,
	BCD_M_1 = 0x33,
	BCD_M_2 = 0x27,
	BCD_W_1 = 0x3c,
	BCD_W_2 = 0x1e,
	BCD_X_1 = 0x39,
	BCD_X_2 = 0x0f,
} BCD_List_t;


/******************************************************************************
Parameter list configurations 
******************************************************************************/

typedef enum
{
	DISP_ID_NONE = 0,
	DP_CUR_LEAKAGE_T = RMS_LEAKAGE_CURRENT,
	DP_FAULT_1 = FAULT_1	,
	DP_FAULT_2   = FAULT_2 ,
	DP_FAULT_3 =FAULT_3 ,
	DP_FAULT_4 =FAULT_4 ,
	DP_FAULT_5 =FAULT_5 ,
	DP_TRIP_SET_VALUE = TRIP_VALUE,
	DP_FAULT_VALUE =  FAULT_VALUE,
	/* Numeric Items */
	
	DISP_ID_TOTAL_NUMBER_TEXT = 1499,
	
	// Text items
	DP_TEXT_NGB,
	DP_TEXT_TRIP,
	DP_TEXT_CAL,
	DP_TEXT_SET,
	DP_TEXT_END,
	DP_TEXT_TIME,
	DP_TEXT_DATE,
	DP_TEXT_ADRES,
	DP_TEXT_BUAD,
	DP_TEXT_PAR,
	DP_TEXT_DELAY,
	DP_TEXT_NONE,
	DP_TEXT_ODD,
	DP_TEXT_EVEN,
	DP_CT_ERROR,
	DP_TEXT_ON,
	DP_TEXT_OFF,
	DP_TEXT_FAULT,
	DP_TEXT_RECLAIM,
	DP_ALARM,
	DP_TEST,
	DP_TEXT_CBCT,
	DP_TEXT_OPEN,
	DP_TEXT_DONE
} DISP_ID_PARAM;


#define _CFG_DECIMAL_SCALER_DIGIT (3U)
#define _CFG_SF_LEAKAGE_CURRENT (2U)
#define _CFG_SF_BDR (0U)

/* Available scaling factor options for selecting decimal point position */
typedef enum
{
	DISP_SF_NONE=0,
	DISP_SF_2,
	DISP_SF_CUR,
	DISP_SF_SETUP,
	DISP_NONE_SETUP,

	DISP_TOTAL_SF
	
} DISP_SF;

/******************************************************************************
Display Interfaces
******************************************************************************/

void display_setScalingFactor( DISP_SF sf );
void display_flashData( uint32_t digit );
void display_rowNumeric ( int32_t Number, uint32_t pageFixIndicatorStatus  );
void display_rowText ( DISP_ID_PARAM ui_id, uint32_t pageIndicatorStatus   );
void display_clearRow(  uint32_t pageIndicatorStatus );
void segment_skipper(uint32_t digiter);
void  _setupNumberText (int32_t Number, uint32_t pageFixIndicatorStatus, uint8_t text);
void segment_on( void );

#ifdef __cplusplus
}
#endif

#endif /* __MFM_DISPLAY_H */

/************************ (C) COPYRIGHT Nagoba Electronics *****END OF FILE****/
