#ifndef __M4VD_MSG_QUE_H__
#define __M4VD_MSG_QUE_H__


#define OFFSET_BASE              0x00020000
#define FW_VOL_INFO_START        (0x00020000+OFFSET_BASE)
#define FW_FRAME_INFO_START      (0x00020100+OFFSET_BASE)
#define FW_DIVX_INFO_START       (0x00020200+OFFSET_BASE)
#define RCV_SEQ_START            (0x00020400+OFFSET_BASE)
#define DEBUG_BUF_START          (0x00020500+OFFSET_BASE)
#define DEC_FRMAE_INFO_START     (0x00020600+OFFSET_BASE)

// VC1_SEQ_INFO put on 65K



#define FW_VERSION               0x01292326
#define INTERFACE_VERSION        0x00000001
#define EN_SECTION_START         0x20000000

typedef struct _FW_VOL_INFO
{
    //VOL infomation
    unsigned short        vol_info;        //reg0x40
//    D[5] short_video_header; //1
//    D[4] vol_interlaced;
//    D[3] vol_qpel;
//    D[2] vol_rsync_marker_disable;
//    D[1] vol_dp_enable;
//    D[0] vol_rvlc_enable;

    unsigned short        sprite_usage;

    unsigned int        width;
    unsigned int        height;

    unsigned short        pts_incr;
    unsigned short        reserved0;

    unsigned char        aspect_ratio;
    unsigned char        progressive_sequence;
    unsigned char        mpeg1;
    unsigned char        play_mode;

    unsigned char        mpeg_frc_mode;
    unsigned char        first_display;
    unsigned char        low_delay;
    unsigned char        video_range;

    unsigned int        bit_rate;

    unsigned short        vol_time_incr_res;
    unsigned short        fixed_vop_time_incr; //0: not fixed_vop_rate  others : vop_time_incr

    unsigned char        par_width;
    unsigned char        par_height;
    unsigned char        reserved1;
    unsigned char        reserved2;

    unsigned int        vc1_frame_rate;
    unsigned int        frame_rate;
    unsigned char        key_gen[32];

}FW_VOL_INFO,*pFW_VOL_INFO;//16 byte

#define OFFSET_VOL_INFO                0
#define OFFSET_SPRITE_USAGE            2
#define OFFSET_WIDTH                   4
#define OFFSET_HEIGHT                  8
#define OFFSET_PTS_INCR                12
#define OFFSET_RESERVED0               14
#define OFFSET_ASPECT_RATIO            16
#define OFFSET_PROGRESSIVE_SEQUENCE    17
#define OFFSET_MPEG1                   18
#define OFFSET_PLAY_MODE               19
#define OFFSET_MPEG_FRC_MODE           20
#define OFFSET_FIRST_DISPLAY           21
#define OFFSET_LOW_DELAY               22
#define OFFSET_VIDEO_RANGE             23
#define OFFSET_BIT_RATE                24
#define OFFSET_VOL_TIME_INCR_RES       28
#define OFFSET_FIXED_VOP_TIME_INCR     30
#define OFFSET_PAR_WIDTH               32
#define OFFSET_PAR_HEIGHT              33
#define OFFSET_VC1_FRAME_RATE          36
#define OFFSET_FRAME_RATE              40
#define OFFSET_KEY_GEN                 44

typedef struct
{
	union
	{
		struct
		{
			unsigned int mvdcmd_handshake_pause : 1;        // 1 for handshake ready with CMD_PAUSE,
			unsigned int mvdcmd_handshake_slq_reset : 1;    // 1 for handshake ready with CMD_VC1_HW_SLQ_RESET
			unsigned int mvdcmd_handshake_stop : 1;         // 1 for handshake ready with CMD_STOP
			unsigned int mvdcmd_handshake_skip_data : 1;    // 1 for handshake ready with CMD_SKIP_DATA
			unsigned int mvdcmd_handshake_skip_to_pts : 1;  // 1 for handshake ready with CMD_SKIP_TO_PTS
			unsigned int mvdcmd_handshake_single_step : 1;  // 1 for handshake ready with CMD_SINGLE_STEP
			unsigned int mvdcmd_handshake_scaler_data_ready : 1;
			unsigned int mvdcmd_handshake_reserved : 25;    // reserved for extend
		};
		unsigned int value;
	};
}MVD_CMD_HANDSHADE_INDEX;

typedef struct _FW_FRAME_INFO
{
    unsigned int            frame_count;    //1
    unsigned int            slq_tbl_rptr;   //==>ms
    unsigned int            vol_update;

    unsigned int            error_code;     //4
    unsigned int            error_status;   //20

    unsigned int            skip_frame_count;

    unsigned int            picture_type; //0:I frame 1:P frame 2:B frame
    unsigned int            slq_sw_index;   //32

    unsigned char            fb_index;
    unsigned char            top_ff;
    unsigned char            repeat_ff;
    unsigned char            invalidstream;//36

    unsigned int            vld_err_count;
    unsigned short            tmp_ref;
    unsigned char            first_frame;
    unsigned char            first_I_found;//44

    unsigned int            gop_i_fcnt;      //12
    unsigned int            gop_p_fcnt;
    unsigned int            gop_b_fcnt;
    unsigned int            overflow_count;//60

    unsigned int            time_incr;       //16
    unsigned int            self_rst_count;
    unsigned int            sw_vd_count;//72
	unsigned int            step_disp_done;
	unsigned int            step_to_pts_done; //20
	MVD_CMD_HANDSHADE_INDEX cmd_handshake_index;
	unsigned int            last_frame_show_done;
	unsigned int            meet_file_end_sc;
	unsigned int			rcv_payload_lenth;
	unsigned int			firmware_version;
	unsigned int			ic_version;
	unsigned int			interface_version;

    unsigned char           color_primaries;
    unsigned char           transfer_char;
    unsigned char           matrix_coef;
    unsigned char           video_format;

    unsigned short          disp_h_size;
    unsigned short          disp_v_size;

    unsigned char           time_code_hours;
    unsigned char           time_code_minutes;
    unsigned char           time_code_seconds;
    unsigned char           time_code_pictures;

    unsigned char           drop_frame_flag;
    unsigned char           time_code_hours_disp;
    unsigned char           time_code_minutes_disp;
    unsigned char           time_code_seconds_disp;

    unsigned char           time_code_pictures_disp;
    unsigned char           drop_frame_flag_disp;
    unsigned char           reserved0;
    unsigned char           reserved1;
}FW_FRAME_INFO, *pFW_FRAME_INFO;

#define OFFSET_FRAME_COUNT         0
#define OFFSET_SLQ_TBL_RPTR        4
#define OFFSET_VOL_UPDATE          8
#define OFFSET_ERROR_CODE          12
#define OFFSET_ERROR_STATUS        16
#define OFFSET_SKIP_FRAME_COUNT    20
#define OFFSET_PICTURE_TYPE        24
#define OFFSET_SLQ_SW_INDEX        28
#define OFFSET_FB_INDEX            32
#define OFFSET_TOP_FF              33
#define OFFSET_REPEAT_FF           34
#define OFFSET_INVALIDSTREAM       35
#define OFFSET_VLD_ERR_COUNT       36
#define OFFSET_TMP_REF             40
#define OFFSET_FIRST_FRAME         42
#define OFFSET_FIRST_I_FOUND       43
#define OFFSET_GOP_I_FCNT          44
#define OFFSET_GOP_P_FCNT          48
#define OFFSET_GOP_B_FCNT          52
#define OFFSET_OVERFLOW_COUNT      56
#define OFFSET_TIME_INCR           60
#define OFFSET_SELF_RST_COUNT      64
#define OFFSET_SW_VD_COUNT         68
#define OFFSET_STEP_DISP_DONE      72
#define OFFSET_STEP_TO_PTS_DONE    76
#define OFFSET_CMD_HANDSHAKE_INDEX 80
#define OFFSET_CMD_LAST_FRAME_SHOW 84
#define OFFSET_MEET_FILE_END_SC    88
#define OFFSET_RCV_PAYLOAD_LENGTH  92
#define OFFSET_FIRMWARE_VERSION    96
#define OFFSET_IC_VERSION          100
#define OFFSET_INTERFACE_VERSION   104
#define OFFSET_COLOR_PRIMARIES     108
#define OFFSET_TRANSFER_CHAR       109
#define OFFSET_MATRIX_COEF         110
#define OFFSET_VIDEO_FORMAT        111
#define OFFSET_DISP_H_SIZE         112
#define OFFSET_DISP_V_SIZE         114
#define OFFSET_TIME_CODE_HOURS     116 // for decoding frame
#define OFFSET_TIME_CODE_MINUTES   117 // for decoding frame
#define OFFSET_TIME_CODE_SECONDS   118 // for decoding frame
#define OFFSET_TIME_CODE_PICTURES  119 // for decoding frame
#define OFFSET_DROP_FRAME_FLAG     120 // for decoding frame
#define OFFSET_TIME_CODE_HOURS_DISP     121 // for displaying frame
#define OFFSET_TIME_CODE_MINUTES_DISP   122 // for displaying frame
#define OFFSET_TIME_CODE_SECONDS_DISP   123 // for displaying frame
#define OFFSET_TIME_CODE_PICTURES_DISP  124 // for displaying frame
#define OFFSET_DROP_FRAME_FLAG_DISP     125 // for displaying frame
#define OFFSET_FRAME_INFO_RESERVE0 126
#define OFFSET_FRAME_INFO_RESERVE1 127

typedef struct _FW_DIVX_INFO
{
    unsigned int     vol_handle_done;

    unsigned int     width;
    unsigned int     height;
    unsigned int     frame_count;//4
    unsigned int     frame_time;

    unsigned short     pts_incr;
    unsigned short     reserve0;

    unsigned char     aspect_ratio;
    unsigned char     progressive_sequence;
    unsigned char     mpeg1;
    unsigned char     play_mode;

    unsigned char     mpeg_frc_mode;//8
    unsigned char     invalidstream;
    unsigned char     reserve[2];
    unsigned int      frame_rate;
}FW_DIVX_INFO, *pFW_DIVX_INFO;

#define OFFSET_DIVX_VOL_HANDLE_DONE      0
#define OFFSET_DIVX_WIDTH                4
#define OFFSET_DIVX_HEIGHT               8
#define OFFSET_DIVX_FRAME_COUNT          12
#define OFFSET_DIVX_FRAME_TIME           16
#define OFFSET_DIVX_PTS_INCR             20
#define OFFSET_DIVX_RESERVE0             22
#define OFFSET_DIVX_ASPECT_RATIO         24
#define OFFSET_DIVX_PROGRESSIVE_SEQUENCE 25
#define OFFSET_DIVX_MPEG1                26
#define OFFSET_DIVX_PLAY_MODE            27
#define OFFSET_DIVX_MPEG_FRC_MODE        28
#define OFFSET_DIVX_INVALIDSTREAM        29
#define OFFSET_DIVX_RESERVED             30
#define OFFSET_DIVX_FRAME_RATE           32

#define STATUS_VIDEO_SYNC    (1<<0)
#define STATUS_VIDEO_FREERUN (1<<1)
#define STATUS_VIDEO_SKIP    (1<<2)
#define STATUS_VIDEO_REPEAT  (1<<3)

typedef struct _FW_USER_DATA_BUF
{
    unsigned char picType;             /* picture type: 1->I picture, 2->P,3->B */
    unsigned char top_ff;              /* Top field first: 1 if top field first*/
    unsigned char rpt_ff;              /* Repeat first field: 1 if repeat field first*/
    unsigned char userdatabytecnt;

    unsigned short     tmpRef;             /* Temporal reference of the picture*/

    unsigned char userdata[250];
}FW_USER_DATA_BUF,*pFW_USER_DATA_BUF;


typedef struct _DecFrameInfo
{
    unsigned int      u32DecLumaAddr;
    unsigned int      u32DecChromaAddr;
    unsigned int      u32DecTimeStamp;
    unsigned int      u32DecID_L;
    unsigned int      u32DecID_H;
    unsigned short    u16DecPitch;
    unsigned short    u16DecWidth;
    unsigned short    u16DecHeight;
    unsigned short    u16DeceFrameType;
	unsigned int      u32DispLumaAddr;
	unsigned int      u32DispChromaAddr;
	unsigned int      u32DispTimeStamp;
	unsigned int      u32DispID_L;
	unsigned int      u32DispID_H;
	unsigned short    u16DispPitch;
	unsigned short    u16DispWidth;
	unsigned short    u16DispHeight;
	unsigned short    u16DispeFrameType;
}DecFrameInfo, *pDecFrameInfo;

#define OFFSET_DECFRAMEINFO_DEC_LUMAADDR           0
#define OFFSET_DECFRAMEINFO_DEC_CHROMAADDR         4
#define OFFSET_DECFRAMEINFO_DEC_TIMESTAMP          8
#define OFFSET_DECFRAMEINFO_DEC_ID_L              12
#define OFFSET_DECFRAMEINFO_DEC_ID_H              16
#define OFFSET_DECFRAMEINFO_DEC_PITCH             20
#define OFFSET_DECFRAMEINFO_DEC_WIDTH             22
#define OFFSET_DECFRAMEINFO_DEC_HEIGHT            24
#define OFFSET_DECFRAMEINFO_DEC_FRAMETYPE         26
#define OFFSET_DECFRAMEINFO_DISP_LUMAADDR         28
#define OFFSET_DECFRAMEINFO_DISP_CHROMAADDR       32
#define OFFSET_DECFRAMEINFO_DISP_TIMESTAMP        36
#define OFFSET_DECFRAMEINFO_DISP_ID_L             40
#define OFFSET_DECFRAMEINFO_DISP_ID_H             44
#define OFFSET_DECFRAMEINFO_DISP_PITCH            48
#define OFFSET_DECFRAMEINFO_DISP_WIDTH            50
#define OFFSET_DECFRAMEINFO_DISP_HEIGHT           52
#define OFFSET_DECFRAMEINFO_DISP_FRAMETYPE        54

typedef struct _DEBUG_INFO
{
    //
    volatile unsigned short max_coded_width;
    volatile unsigned short max_coded_height;
    volatile unsigned short sync_status;              // defined by AV_SYNC

    volatile unsigned short REG67;
    volatile unsigned short REG68;
    volatile unsigned short REG69;

    volatile unsigned short REG6a;
    volatile unsigned short REG6b;
    volatile unsigned short REG6c;

    volatile unsigned short REG6d;
    volatile unsigned short REG6e;
    volatile unsigned short REG6f;

    //
    volatile unsigned short overflow_count;
    volatile unsigned short underflow_count;
    volatile unsigned short vlderr_count;
    volatile unsigned short frame_conut;//0

    volatile unsigned int   y_start_addr; //in byte unit
    volatile unsigned int   uv_start_addr;//in byte unit

    volatile unsigned int   width;
    volatile unsigned int   height;

    // where
    volatile unsigned short mb_x;
    volatile unsigned short mb_y;

    volatile unsigned short file_end;
    //16-byte aligned
    volatile unsigned char reserved[24];

}DebugInfo;

typedef struct VC1_SEQ_INFO
{
    volatile    unsigned int        PROFILE;
    volatile    unsigned int        FRMRTQ_POSTPROC;
    volatile    unsigned int        BITRTQ_POSTPROC;
    volatile    unsigned int        LOOPFILTER;
    volatile    unsigned int        MULTIRES;
    volatile    unsigned int        FASTUVMC;
    volatile    unsigned int        EXTENDED_MV;
    volatile    unsigned int        DQUANT;
    volatile    unsigned int        VSTRANSFORM;
    volatile    unsigned int        OVERLAP;
    volatile    unsigned int        SYNCMARKER;
    volatile    unsigned int        RANGERED;
    volatile    unsigned int        MAXBFRAMES;
    volatile    unsigned int        QUANTIZER;
    volatile    unsigned int        FINTERPFLAG;
	volatile    unsigned int        LEVEL;
	volatile    unsigned int        CBR;
	volatile    unsigned int        FRAMERATE;
	volatile    unsigned int        VERT_SIZE;
	volatile    unsigned int        HORIZ_SIZE;
}VC1_SEQUENCE_INFO, *pVC1_SEQUENCE_INFO;

#define OFFSET_RCV_PROFILE                        0
#define OFFSET_RCV_FRMRTQ_POSTPROC                4
#define OFFSET_RCV_BITRTQ_POSTPROC                8
#define OFFSET_RCV_LOOPFILTER                    12
#define OFFSET_RCV_MULTIRES                      16
#define OFFSET_RCV_FASTUVMC                      20
#define OFFSET_RCV_EXTENDED_MV                   24
#define OFFSET_RCV_DQUANT                        28
#define OFFSET_RCV_VSTRANSFORM                   32
#define OFFSET_RCV_OVERLAP                       36
#define OFFSET_RCV_SYNCMARKER                    40
#define OFFSET_RCV_RANGERED                      44
#define OFFSET_RCV_MAXBFRAMES                    48
#define OFFSET_RCV_QUANTIZER                     52
#define OFFSET_RCV_FINTERPFLAG                   56
#define OFFSET_RCV_LEVEL                         60
#define OFFSET_RCV_CBR                           64
#define OFFSET_RCV_FRAMERATE                     68
#define OFFSET_RCV_VERT_SIZE                     72
#define OFFSET_RCV_HORIZ_SIZE                    76

typedef struct _FW_AVSYNC_TABLE
{
    unsigned int      byte_cnt;         //23 valid bits
    unsigned int      dummy_cnt;        //dummy packet counter
    unsigned int      id_low;           //ID specified by player
    unsigned int      id_high;

    unsigned int      time_stamp;       //pts or dts
    unsigned int      reserved_int0;
    unsigned int      reserved_int1;
    unsigned int      reserved_int2;
}FW_AVSYNC_TABLE, *pFW_AVSYNC_TABLE;

#define OFFSET_BYTE_CNT                         0
#define OFFSET_DUMMY_CNT                        4
#define OFFSET_ID_LOW                           8
#define OFFSET_ID_HIGH                         12
#define OFFSET_TIME_STAMP                      16

#ifdef M4VDPLAYER
extern    pFW_VOL_INFO        gp_vol_info;
extern    pFW_DIVX_INFO       gp_divx_info;
#endif

//interupt flag
#define INT_CC_NEW          (1<<0)
#define INT_USER_DATA       (1<<0)
#define INT_VBUF_OVF        (1<<1)
#define INT_VBUF_UNF        (1<<2)
#define INT_VES_VALID       (1<<3)
#define INT_VES_INVALID     (1<<4)
#define INT_SEQ_FOUND       (1<<5)
#define INT_PIC_FOUND       (1<<6)
#define INT_DEC_ERR         (1<<7)
#define INT_FIRST_FRAME     (1<<8)
#define INT_DISP_RDY        (1<<9)
#define INT_SYN_SKIP        (1<<10)
#define INT_SYN_REP         (1<<11)

#define INT_SYN_SKIP_P       10
#define INT_SYN_REP_P        11

// decoding state definition
#define DEC_STAT_IDLE                0x00
#define DEC_STAT_FIND_SC             0x01
#define DEC_STAT_FIND_SPE_SC         0x11
#define DEC_STAT_FIND_FRAMEBUFFER    0x02
#define DEC_STAT_WAIT_DECODE_DONE    0x03
#define DEC_STAT_DECODE_DONE         0x04
#define DEC_STAT_WAIT_VDFIFO         0x05
#define DEC_STAT_INIT_SUCCESS        0x06

//error_code
#define VOL_SHAPE                       1  //error_status 0:rectanglular    1:binary 2: binary only 3: grayscale
#define VOL_USED_SPRITE                 2  //error_status 0:sprite not used 1:static 2: GMC 3: reserved
#define VOL_NOT_8_BIT                   3  //error_status : bits per pixel
#define VOL_NERPRED_ENABLE              4
#define VOL_REDUCED_RES_ENABLE          5
#define VOL_SCALABILITY                 6
#define VOL_OTHER                       7
#define VOL_H263_ERROR                  8
#define VOL_RES_NOT_SUPPORT             9 //error_status : none
#define VOL_MPEG4_NOT_SUPPORT          10 //error_status : none
#define VOL_PROFILE_NOT_SUPPORT        11

#define CODEC_MPEG4                    0x00 //arg0: 0: mpeg4, 1: mpeg4 with short_video_header, 2: DivX311
#define CODEC_MPEG4_SHORT_VIDEO_HEADER 0x01
#define CODEC_DIVX311                  0x02
#define CODEC_MPEG2                    0x10
typedef enum //arg1: 0: file mode 1:slq  2:live stream mode 3:slqtbl 4: Ts file mode
{
    FILE_MODE = 0,
    SLQ_MODE,
    STREAM_MODE,
    SLQ_TBL_MODE,
    TS_FILE_MODE,
    OTHER
}stream_type;
#define ENABLE_PARSER                  0x00 //arg2: 0/1 enable/disable parser;
#define DISABLE_PARSER                 0x01
#define ENABLE_PKT_LEN                 0x02
#define PARSER_MPEG2                   0x00 //arg3: 0 13818-1 pes header;
#define PARSER_MPEG1                   0x01 //      1 11172-1 pes header;

#define FrcNormal                         0
#define FrcDisplayTwice                   1 //output rate is twice of input rate (ex. 30p a 60p)
#define Frc32Pulldown                     2 //3:2 pulldown mode (ex. 24p a 60i or 60p)
#define FrcPALtoNTSC                      3 //PALaNTSC conversion (50i a 60i)
#define FrcNTSCtoPAL                      4 //NTSCaPAL conversion (60i a 50i)
#define FrcShowOneFiled                   5

#define MVD3_FILE_SD_MODE              0x02 //960*544
#define MVD3_HD_MODE                   0x10 //1920*1088
#define MVD3_SD_MODE                   0x00 //720*576

#define MVD3_TRITON                       0
#define MVD3_PLUTO                        1
#define MVD3_OBERON                       3
#define MVD3_PIONEER                      4
#define MVD3_TITANIA2                     5

// File mode avsync related
#define NONE_FILE_MODE                    0
#define FILE_PTS_MODE                     1
#define FILE_DTS_MODE                     2

// argument for "CMD_DISPLAY_PAUSE"
#define DISPLAY_PAUSE_OFF              0x00
#define DISPLAY_PAUSE_ON               0x01

//command interface
#define CMD_PLAY                       0x01
#define CMD_PAUSE                      0x02
#define CMD_STOP                       0x03
#define CMD_FIND_SEQ                   0x04 //find seq header and set command = pause at picture header start code found
#define CMD_SINGLE_STEP                0x05
#define CMD_PLAY_NO_SQE                0x06
#define CMD_FAST_SLOW                  0x07 //arg0: 0: nomarl mode, 1: decode I only,  2: deocde I/P only,  3: slow motion
#define CMD_CODEC_INFO                 0x08 //arg0: 0: mpeg4, 1: mpeg4 with short_video_header, 2: DivX311
#define CMD_SYN_THRESHOLD              0x09
#define CMD_SYNC_ON                    0x0a
#define CMD_SYNC_OFFSET                0x0b
#define CMD_DISPLAY_CTL                0x0c
//arg0: 0/1-display by display/decode order
//arg1:   1-drop display decoding error frame
//arg2:   1-drop display when decode fast than display
//arg3:set frame rate conversion mode
#define CMD_GET_SYNC_STAT               0x0d //return arg0: 0/1 sync off/on ; arg1: 3 sync init done
#define CMD_GET_AFD                     0x0e
#define CMD_SKIP_DATA                   0x0f //set to skip all data till find FW_SPE_SCODE to resume normal play

#define CMD_STREAM_BUF_START           0x10
#define CMD_STREAM_BUF_END             0x11
#define CMD_FB_BASE                    0x12 //Frame buffer base address, from LSB to MSB are arg0, arg1, arg2, arg3
#define CMD_IAP_BUF_START              0x13
#define CMD_DP_BUF_START               0x14
#define CMD_MV_BUF_START               0x15
#define CMD_DMA_OVFTH                  0x16
#define CMD_DMA_UNFTH                  0x17
#define CMD_VC1_MIU_PROTECT_START      0x18
#define CMD_VC1_MIU_PROTECT_END        0x19
#define CMD_DISP_SPEED_CTRL            0x1a
#define CMD_STEP_DISP_DECODE_ONE       0x1b
#define CMD_STEP_DISP_ING              0x1c // repeat disp this frame
#define CMD_STEP_TO_PTS                0x1d
#define CMD_HANDSHAKE_STATUS           0x1e //report handshake status
#define CMD_DISPLAY_PAUSE              0x1f // display pause

#define CMD_USER_BUF_START             0x20
#define CMD_USER_BUF_SIZE              0x21
#define CMD_RD_USER_WP                 0x22
#define CMD_WD_USER_RP                 0x23
#define CMD_RD_CC_PKTCNT               0x24
#define CMD_RD_CC_OV                   0x25
#define CMD_CLOSE_CC                   0x26

#define CMD_GET_INT_STAT               0x30
#define CMD_PARSE_M4V_PACKMD           0x31
#define CMD_RD_PTS                     0x32
#define CMD_FLUSH_LAST_IPFRAME         0x33
#define CMD_DECODE_STATUS              0x34 // arg0 = lastcommand ; arg1 = decode_status
#define CMD_VBUFFER_COUNT              0x35
#define CMD_START_DEC_STRICT           0x36 // start decoding in First I and skip non reference frame B decoding
#define CMD_SW_RESET                   0x37
#define CMD_MVD_FAST_INT               0x38
#define CMD_DIU_WIDTH_ALIGN            0x39
#define CMD_SW_IDX_ADJ                 0x3a // arg0=1 set sw_index as previous queue index infomation
#define CMD_PARSER_READ_POSITION       0x3b
#define CMD_REPEAT_MODE                0x3c // arg0=1 when frame display repeat only show one field
#define CMD_PTS_BASE                   0x3d
#define CMD_SKIP_TO_PTS                0x3e
#define CMD_AVSYNC_FREERUN_THRESHOLD   0x3f

#define CMD_DEBUG_BUF_START            0x40
#define CMD_DEBUG_CTL                  0x42
#define CMD_RD_IO                      0x43
#define CMD_WR_IO                      0x44
#define CMD_FB_RED_SET                 0x45
#define CMD_FB_NUM                     0x46

#define CMD_SLQ_START                  0x50 //SLQ start address, from LSB to MSB are arg0, arg1, arg2, arg3
#define CMD_SLQ_END                    0x51 //SLQ end address, from LSB to MSB are arg0, arg1, arg2, arg3
#define CMD_SLQ_AVAIL_LEVEL            0x52 //arg0: 4-0
#define CMD_FPGA_COMP                  0x53 //arg0: 1/0:enable/disable FPGA comp
#define CMD_DIVX_PATCH                 0x54 //arg0: D[0] divx mv p interlace chroma adjust
#define CMD_HEADER_INFO_BUF            0x55 //header info buffer base address, from LSB to MSB are arg0, arg1, arg2, arg3
#define CMD_IDCT_SEL                   0x56 // arg0 D[0]:0/1 llm/divx6   D[1]:0/1 unbias/bias rounding mode
#define CMD_VOL_INFO_BUF               0x57
#define CMD_FRAME_INFO_BUF             0x58
#define CMD_CODE_OFFSET                0x59
#define CMD_RESET_FRAMECOUNT           0x5a
#define CMD_CHIPID                     0x5b
#define CMD_DEC_FRAME_INFO_BUF		   0x5c
#define CMD_GET_FW_VERSION             0x5E
#define CMD_GET_EN_CATCH_DATA          0x5F

#define CMD_SLQ_TBL_BUF_START          0x60
#define CMD_SLQ_TBL_BUF_END            0x61
#define CMD_SLQ_UPDATE_TBL_WPTR        0x62
#define CMD_SLQ_GET_TBL_RPTR           0x63

// FW stop updating frames when vsync, but decoding process is still going.
#define CMD_FREEZE_DISP				   0x64
#define CMD_DS_VIRTUAL_BOX			   0x65

#define CMD_DUMP_BITSTREAM_BASE        0x70
#define CMD_DUMP_BITSTREAM_LENGTH      0x71

#define CMD_MVD_IDLE                   0x73
#define CMD_INTERFACE_VERSION          0x74
#define CMD_VC1_STREAM_TYPE_JPEG       0x75
#define CMD_VC1_STREAM_TYPE_MJPEG      0x76
#define CMD_VC1_BYPASS_MODE            0x77
#define CMD_VC1_UPDATE_SLQ             0x78
#define CMD_VC1_HW_SLQ_RESET           0x79
#define CMD_FLUSH_DISP_QUEUE           0x7A
#define CMD_VC1_FORCE_INTLACE_DISP     0x7B
#define CMD_VC1_IP_SCALE_THRESHOLD     0x7C
#define CMD_MOTION_COM_REDUCE          0x7D
#define CMD_CLOSE_DEBLOCK              0x7E
#define CMD_FIXED_FRAME_BUFFER         0x7F


#define CMD_ENABLE_INTERQ              0x80
#define CMD_DISPLAY_FRAME              0x81
#define CMD_SET_DTS                    0x82
#define CMD_READ_DTS                   0x83
#define CMD_FLUSH_INTERQ               0x84

// File mode avsync related
#define CMD_ENABLE_FILE_SYNC           0x85
#define CMD_PTS_TBL_START              0x86
#define CMD_FORCE_BLUE_SCREEN          0x87
#define CMD_ENABLE_LAST_FRAME_SHOW     0x88
#define CMD_DYNAMIC_SCALE_BASE         0x89
#define CMD_ENABLE_DYNAMIC_SCALE       0x8A
#define CMD_SCALER_INFO_BASE           0x8B

// JPEG command
#define CMD_JPEG_CONSTRAIN_SIZE        0x91
#define CMD_JPEG_STATUS                0x92
#define CMD_JPEG_SCALEFACTOR           0x93
#define CMD_JPEG_ROI                   0x94
#define CMD_JPEG_ROI_DIM               0x95
#define CMD_JPEG_IPM                   0x96

// add data in bitstream from skip mode back to normal
// 00_00_01_C5_ab_08_06_27
#define FW_SPE_SCODE                     0xC5
#define FW_RESUME1                       0xab08
#define FW_RESUME2                       0x0627
#define FILE_PAUSE_SC                    0xBE
#define FILE_END_SC                      0xC6
#define FILE_END_EXT1                    0xaabb
#define FILE_END_EXT2                    0xccdd
#define FILE_END_EXT3                    0xeeff
#define FILE_END_EXT4                    0xffff
#define FILE_END_EXT5                    0x0000
#endif
