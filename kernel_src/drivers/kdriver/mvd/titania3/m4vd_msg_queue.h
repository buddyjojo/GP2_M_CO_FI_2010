#ifndef __M4VD_MSG_QUE_H
#define __M4VD_MSG_QUE_H

#ifdef OR1K
#include "typedef.h"
#endif

#ifdef __C51__
#include "datatype.h"
//typedef U8 UINT8;
//typedef U16 UINT16;
//typedef U32 UINT32;
#endif

//#ifdef CONFIG_MIPS32
typedef U8 UINT8;
typedef U16 UINT16;
typedef U32 UINT32;
//#endif


#ifdef FPGA_SIM
#define FW_VOL_INFO_START    0x00010000
#define FW_FRAME_INFO_START  0x00010100

#ifdef _DEBUG
#define DEBUG_BUF_START      0x00200000
#endif
#endif

#ifndef PIONEER_BRING_UP
#define PIONEER_BRING_UP 0
#endif

#if PIONEER_BRING_UP
#define MVD3_FW_VERSION		 0x00710000  // v00.72.03.30
#else 
#define MVD3_FW_VERSION		 0x00710000  // v00.72.03.30
#endif

#define EN_SECTION_START     0x20000000

typedef struct _FW_VOL_INFO
{
    //VOL infomation
    UINT16 vol_info;        //reg0x40
//    D[5] short_video_header; //1
//    D[4] vol_interlaced;
//    D[3] vol_qpel;
//    D[2] vol_rsync_marker_disable;
//    D[1] vol_dp_enable;
//    D[0] vol_rvlc_enable;

    UINT16 sprite_usage;

    UINT32 width;
    UINT32 height;

    UINT16 pts_incr;
	UINT16 frame_rate;

    UINT8 aspect_ratio;
    UINT8 progressive_sequence;
    UINT8 mpeg1;
    UINT8 play_mode;

	UINT8 mpeg_frc_mode;
    UINT8 reserved1;
    UINT8 low_delay;
    UINT8 reserved;

    UINT32 bit_rate;
    
    UINT16 vol_time_incr_res;
    UINT8  par_width;
    UINT8  par_height;

    UINT8  key_gen[32];
    
    

}FW_VOL_INFO,*pFW_VOL_INFO;//16*4 byte

#define OFFSET_VOL_INFO             0
#define OFFSET_SPRITE_USAGE         2
#define OFFSET_WIDTH                4
#define OFFSET_HEIGHT               8
#define OFFSET_PTS_INCR             12
#define OFFSET_FRAME_RATE           14
#define OFFSET_ASPECT_RATIO         16
#define OFFSET_PROGRESSIVE_SEQUENCE 17
#define OFFSET_MPEG1                18
#define OFFSET_PLAY_MODE            19
#define OFFSET_MPEG_FRC_MODE        20
//#define reserver1                 21
#define OFFSET_LOW_DELAY            22
//#define OFFSET_RESERVED;          23
#define OFFSET_BIT_RATE             24
#define OFFSET_VOL_TIME_INCR_RES    28
#define OFFSET_PAR_WIDTH            30
#define OFFSET_PAR_HEIGHT           31
#define OFFSET_KEY_GEN              32

typedef struct _FW_FRAME_INFO
{
    UINT32 frame_count;
    UINT32 slq_tbl_rptr;    //==>ms
    UINT32 vol_update;

    UINT32 error_code;
    UINT32 error_status;

    UINT32 skip_frame_count;

    UINT32 picture_type; //0:I frame 1:P frame 2:B frame
    UINT32 slq_sw_index;

	UINT8  fb_index;
	UINT8  top_ff;
    UINT8  repeat_ff;
    UINT8  invalidstream;

    UINT32 vld_err_count;

    UINT16 tmp_ref;
    UINT8  first_frame;
    UINT8  first_I_found;

	UINT32 gop_i_fcnt; 
    UINT32 gop_p_fcnt;
    UINT32 gop_b_fcnt;
    UINT32 overflow_count;
    
    UINT32 time_incr;
    UINT32 self_rst_count;
    UINT32 sw_vd_count;
    
}FW_FRAME_INFO, *pFW_FRAME_INFO;//16*4 byte

#define OFFSET_FRAME_COUNT      0
#define OFFSET_SLQ_TBL_RPTR     4
#define OFFSET_VOL_UPDATE       8
#define OFFSET_ERROR_CODE       12
#define OFFSET_ERROR_STATUS     16
#define OFFSET_SKIP_FRAME_COUNT 20
#define OFFSET_PICTURE_TYPE     24
#define OFFSET_SLQ_SW_INDEX     28
#define OFFSET_FB_INDEX         32
#define OFFSET_TOP_FF           33
#define OFFSET_REPEAT_FF        34
#define OFFSET_INVALIDSTREAM    35
#define OFFSET_VLD_ERR_COUNT    36
#define OFFSET_TMP_REF          40
#define OFFSET_FIRST_FRAME      42
#define OFFSET_FIRST_I_FOUND    43
#define OFFSET_GOP_I_FCNT       44
#define OFFSET_GOP_P_FCNT       48
#define OFFSET_GOP_B_FCNT       52
#define OFFSET_OVERFLOW_COUNT   56
#define OFFSET_TIME_INCR        60
#define OFFSET_SELF_RST_COUNT   64

typedef struct _FW_DIVX_INFO
{
    UINT32 vol_handle_done;

    UINT32 width;
    UINT32 height;
    UINT32 frame_count;
    UINT32 frame_time;

    UINT16 pts_incr;
	UINT16 frame_rate;

    UINT8 aspect_ratio;
    UINT8 progressive_sequence;
    UINT8 mpeg1;
    UINT8 play_mode;

	UINT8 mpeg_frc_mode;

}FW_DIVX_INFO, *pFW_DIVX_INFO; //29 byte

#define OFFSET_DIVX_VOL_HANDLE_DONE      0
#define OFFSET_DIVX_WIDTH                4
#define OFFSET_DIVX_HEIGHT               8
#define OFFSET_DIVX_FRAME_COUNT          12
#define OFFSET_DIVX_FRAME_TIME           16
#define OFFSET_DIVX_PTS_INCR             20
#define OFFSET_DIVX_FRAME_RATE           22
#define OFFSET_DIVX_ASPECT_RATIO         24
#define OFFSET_DIVX_PROGRESSIVE_SEQUENCE 25
#define OFFSET_DIVX_MPEG1                26
#define OFFSET_DIVX_PLAY_MODE            27
#define OFFSET_DIVX_MPEG_FRC_MODE        28


typedef struct _FW_USER_DATA_BUF
{
    UINT8 picType;             /* picture type: 1->I picture, 2->P,3->B */
    UINT8 top_ff;              /* Top field first: 1 if top field first*/
    UINT8 rpt_ff;              /* Repeat first field: 1 if repeat field first*/
	UINT8 userdatabytecnt;
    
    UINT16 tmpRef;             /* Temporal reference of the picture*/
    
    UINT8 userdata[250];
}FW_USER_DATA_BUF,*pFW_USER_DATA_BUF;

typedef struct _DEBUG_INFO
{
    //
    UINT16 REG5d;
    UINT16 REG5e;
    UINT16 REG5f;

    UINT16 REG67;
    UINT16 REG68;
    UINT16 REG69;

    UINT16 REG6a;
    UINT16 REG6b;
    UINT16 REG6c;

    UINT16 REG6d;
    UINT16 REG6e;
    UINT16 REG6f;

    //
    UINT16 overflow_count;
    UINT16 underflow_count;
    UINT16 vlderr_count;
    UINT16 frame_conut;//0

    UINT32 y_start_addr; //in byte unit
    UINT32 uv_start_addr;//in byte unit

    UINT32 width;
    UINT32 height;

    // where
    UINT16 mb_x;
    UINT16 mb_y;
#ifdef FPGA
    //16-byte aligned
    UINT8 reserved[22];
#endif

}DebugInfo;

//interupt flag
#define INT_CC_NEW          (1<<0)
#define INT_USER_DATA		(1<<0)
#define INT_VBUF_OVF        (1<<1)
#define INT_VBUF_UNF        (1<<2)
#define INT_VES_VALID       (1<<3)
#define INT_VES_INVALID     (1<<4)
#define INT_SEQ_FOUND       (1<<5)
#define INT_PIC_FOUND       (1<<6)
#define INT_DEC_ERR  		(1<<7)
#define INT_FIRST_FRAME		(1<<8)
#define INT_DISP_RDY		(1<<9)
#define INT_SYN_SKIP 		(1<<10)
#define INT_SYN_REP 		(1<<11)

#define INT_SYN_SKIP_P		10
#define INT_SYN_REP_P		11

// decode_status definition
#define DEC_STAT_IDLE         	    0x00
#define DEC_STAT_FIND_SC         	0x01
	#define DEC_STAT_FIND_SPE_SC   	0x11
#define DEC_STAT_FIND_FRAMEBUFFER  	0x02
#define DEC_STAT_WAIT_DECODE_DONE	0x03
#define DEC_STAT_DECODE_DONE  	    0x04
#define DEC_STAT_WAIT_VDFIFO  	    0x05
#define DEC_STAT_INIT_SUCCESS       0x06

//error_code
#define VOL_SHAPE              1  //error_status 0:rectanglular    1:binary 2: binary only 3: grayscale
#define VOL_USED_SPRITE        2  //error_status 0:sprite not used 1:static 2: GMC 3: reserved
#define VOL_NOT_8_BIT          3  //error_status : bits per pixel
#define VOL_NERPRED_ENABLE     4
#define VOL_REDUCED_RES_ENABLE 5
#define VOL_SCALABILITY        6
#define VOL_OTHER		       7
#define VOL_H263_ERROR    	   8
#define VOL_RES_NOT_SUPPORT    9 //error_status : none 
#define VOL_MPEG4_NOT_SUPPORT 10 //error_status : none 


//command interface
#define CMD_PLAY             0x01
#define CMD_PAUSE            0x02
#define CMD_STOP             0x03
#define CMD_FIND_SEQ         0x04 //find seq header and set command = pause at picture header start code found
#define CMD_SINGLE_STEP      0x05 


#define CMD_FORCE_SQE        0x06
#define CMD_FAST_SLOW        0x07 //arg0: 0: nomarl mode, 1: decode I only,  2: deocde I/P only,  3: slow motion 
#define CMD_CODEC_INFO       0x08 
    #define CODEC_MPEG4      0x00 //arg0: 0: mpeg4, 1: mpeg4 with short_video_header, 2: DivX311
    #define CODEC_MPEG4_SHORT_VIDEO_HEADER 0x01
    #define CODEC_DIVX311    0x02
    #define CODEC_MPEG2      0x10
	#define FILE_MODE        0x00//arg1: 0: file mode 1:slq  2:live stream mode 3:slqtbl 4: Ts file mode
	#define SLQ_MODE         0x01
	#define TS_MODE          0x02
	#define SLQ_TBL_MODE     0x03
	#define TS_FILE_MODE     0x04
	#define ENABLE_PARSER    0x00//arg2: 0/1 enable/disable parser;
	#define DISABLE_PARSER   0x01
	#define DISABLE_PKT_LEN  0x02

#define CMD_SYN_THRESHOLD    0x09
#define CMD_SYNC_ON          0x0a
#define CMD_SYNC_OFFSET      0x0b
#define CMD_DISPLAY_CTL      0x0c
	//arg0: 0/1-display by display/decode order
	//arg1:   1-drop display decoding error frame
	//arg2:   1-drop display when decode fast than display
	//arg3:set frame rate conversion mode
	#define FrcNormal         0
	#define FrcDisplayTwice   1 //output rate is twice of input rate (ex. 30p a 60p)
	#define Frc32Pulldown     2 //3:2 pulldown mode (ex. 24p a 60i or 60p)
	#define FrcPALtoNTSC      3 //PALaNTSC conversion (50i a 60i)
	#define FrcNTSCtoPAL      4 //NTSCaPAL conversion (60i a 50i)
	#define FrcShowOneFiled	  5
	
#define CMD_GET_SYNC_STAT    0x0d //return arg0: 0/1 sync off/on ; arg1: 3 sync init done
#define CMD_GET_AFD          0x0e
#define CMD_SKIP_DATA        0x0f //set to skip all data
                                  //till find FW_SPE_SCODE to resume normal play

#define CMD_STREAM_BUF_START 0x10
#define CMD_STREAM_BUF_END   0x11
#define CMD_FB_BASE          0x12 //Frame buffer base address, from LSB to MSB are arg0, arg1, arg2, arg3
	#define MVD3_FILE_SD_MODE    0x02 //960*544
    #define MVD3_HD_MODE         0x01 //1920*1088
    #define MVD3_SD_MODE         0x00 //720*576

#define CMD_IAP_BUF_START    0x13
#define CMD_DP_BUF_START     0x14
#define CMD_MV_BUF_START     0x15
#define CMD_DMA_OVFTH        0x16
#define CMD_DMA_UNFTH        0x17

#define CMD_CC_PBUF_START	 0x20
#define CMD_CC_PBUF_SIZE 	 0x21
#define CMD_RD_CC_WP     	 0x22
#define CMD_WR_CC_RP     	 0x23

#define CMD_USER_BUF_START	 0x20
#define CMD_USER_BUF_SIZE	 0x21
#define CMD_RD_USER_WP		 0x22
#define CMD_WD_USER_RP		 0x23
#define CMD_RD_CC_PKTCNT 	 0x24
#define CMD_RD_CC_OV     	 0x25

#define CMD_GET_INT_STAT     0x30
#define CMD_PARSE_M4V_PACKMD 0x31
#define CMD_RD_PTS			 0x32
#define CMD_FLUSH_LAST_IPFRAME 0x33
#define CMD_DECODE_STATUS    0x34 // arg0 = lastcommand ; arg1 = decode_status
#define CMD_VBUFFER_COUNT    0x35
#define CMD_START_DEC_STRICT 0x36 // start decoding in First I and skip non reference frame B decoding
#define CMD_MVD_SWRST        0x37 // arg0=1 stop cpu and parser
#define CMD_MVD_FAST_INT     0x38
#define CMD_DIU_WIDTH_ALIGN  0x39
#define CMD_SW_IDX_ADJ       0x3a // arg0=1 set sw_index as previous quene index infomation 
#define CMD_FW_ADJ_FF        0x3b // arg0=1 : 2x,  2: 4x,
								  // arg1=0/1 turn on/off frc32
#define CMD_DEBUG_BUF_START  0x40

#define CMD_DEBUG_CTL        0x42

#define CMD_RD_IO            0x43
#define CMD_WR_IO            0x44
#define CMD_FB_RED_SET       0x45

#define CMD_SLQ_START        0x50 //SLQ start address, from LSB to MSB are arg0, arg1, arg2, arg3
#define CMD_SLQ_END          0x51 //SLQ end address, from LSB to MSB are arg0, arg1, arg2, arg3
#define CMD_SLQ_AVAIL_LEVEL  0x52 //arg0: 4-0
#define CMD_FPGA_COMP        0x53 //arg0: 1/0:enable/disable FPGA comp
#define CMD_DIVX_PATCH       0x54 //arg0: D[0] divx mv p interlace chroma adjust

#define CMD_HEADER_INFO_BUF  0x55 //header info buffer base address, from LSB to MSB are arg0, arg1, arg2, arg3
#define CMD_IDCT_SEL         0x56 // arg0 D[0]:0/1 llm/divx6   D[1]:0/1 unbias/bias rounding mode

#define CMD_VOL_INFO_BUF     0x57
#define CMD_FRAME_INFO_BUF   0x58
#define CMD_CODE_OFFSET      0x59
#define CMD_RESET_FRAMECOUNT 0x5a
#define CMD_CHIPID           0x5b
	#define MVD3_TRITON   0 
	#define MVD3_PLUTO    1
	#define MVD3_OBERON   3
	#define MVD3_PIONEER  4
	#define MVD3_TITANIA2 5

#define CMD_GET_FW_VERSION         0x5E
#define CMD_GET_EN_CATCH_DATA      0x5F

#define CMD_SLQ_TBL_BUF_START      0x60
#define CMD_SLQ_TBL_BUF_END        0x61
#define CMD_SLQ_UPDATE_TBL_WPTR    0x62
#define CMD_SLQ_GET_TBL_RPTR       0x63

#define CMD_GET_KEY                0x70
// add data in bitstream from skip mode back to normal 
// 00_00_01_C5_ab_08_06_27
#define FW_SPE_SCODE   			   0xC5
#define FW_RESUME1				   0xab08	
#define FW_RESUME2				   0x0627	


// add data in the end of bitstream to push the last I/P frame
// 00_00_01_C5_bc_08_08_04
//#define FW_FLUSH_LAST1			   0xbc08	
//#define FW_FLUSH_LAST2			   0x0804	

#endif
