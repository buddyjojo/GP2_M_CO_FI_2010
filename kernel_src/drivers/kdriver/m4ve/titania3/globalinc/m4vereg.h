////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2006-2010 MStar Semiconductor, Inc.
//
// Unless otherwise stipulated in writing, any and all information contained herein
// regardless in any format shall remain the property of MStar Semiconductor Inc.
//
// You can redistribute it and/or modify it under the terms of the GNU General Public
// License version 2 as published by the Free Foundation. This program is distributed
// in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
/////////////////////////////////////////////////////////////////////////////////////////////


#ifndef _M4VE_REG_H_
#define _M4VE_REG_H_

#define m4ve_reg_base 0xBF80A000 // at montage platform

typedef	struct _m4ve_reg_ {
	union {
		struct
		{
			unsigned short frame_start:1;  //A input is ready to be encoded
			unsigned short mode_decision_en:1; //0:inter mode, 1: do mode (intra, inter) decision
			unsigned short initial_guess_en:1; // 0:doesn't use previous vector as initial vector, 1: use previous vectors as initial vector
			unsigned short large_diamond_en:1; //0:disable large diamond search 1: enable large diamond search
			unsigned short search_4block_en:1; //0:disable 4 8x8 integer search 1: enable 4 8x8 integer search
			unsigned short half_pixel_16x16_en:1; //enable 16x16 half pixel search
			unsigned short half_pixel_8x8_en:1; // enable 4 8x8 block half pixel search
			unsigned short dynamic_me_en:1; //1: enable dynamic Motion Estimation

			unsigned short frame_type:2; //0:I frame, 1: P frame , 2: B-frame(not reconstructed), 3: B-frame (reconstructed)
			unsigned short soft_reset:1; // software reset
			unsigned short b_pred_mode:1;  //b picture pred mode 0: backward, 1: forward
			unsigned short pre_ip:1; //previous I/P indicator: 0:I frame, 1: P frame
			unsigned short update_mc:1; //update recontruct address (after set rec_address, turn on; then after some times, turn it off)
            unsigned short reg_m4ve_h263:1; //H263 bitstream enable
			unsigned short reg00_reserved:1;
		};
		unsigned short reg00;
	};

	union {
		struct
		{
#if 1//def _M4VE_BIG2_
            unsigned short search_range_left:6; //In two's complement format (default is :-16)
            unsigned short reg01_reserved:2;
            unsigned short search_range_right:6; //In two 's complement format (default is :15)
            unsigned short reg01_reserved2:2;
#else
			unsigned short search_range_left:8; //In two's complement format (default is :-16)
			unsigned short search_range_right:8; //In two 's complement format (default is :15)
#endif
		};
		unsigned short reg01;
	};


	union {
		struct
		{
			unsigned short search_pixel_threshold1:8; //Allowed search points in small diamond search
			unsigned short search_pixel_threshold2:8; //Allowed search points in large diamond search
		};
		unsigned short reg02;
	};

	union {
		struct
		{
			unsigned short motion_activity_th_s:8; // Motion activity threshold for small diamond
			unsigned short motion_activity_th_l:8; // Motion activity threshold for large diamond
		};
		unsigned short reg03;
	};
#ifdef _M4VE_BIG2_
    union {
        struct
        {
            unsigned short pic_width:6;  //The width of frames (unit: MB) (ex. 720/16)
            unsigned short reg_m4ve_mode_threshold:10; //default value is 1024. It's for inter/intra mode decision.
        };
        unsigned short reg04;
    };
    union {
        struct
        {
            unsigned short pic_height:6; //The height of frame (unit: MB) (ex. 480/16)
            unsigned short search_4block_range:1;  // 0: -2 ~ 2, 1: -1 ~ 1 (default)
            unsigned short reg05_reserved:9;
        };
        unsigned short reg05;
	};
#else
    //Triton setting
	union {
		struct
		{
			unsigned short pic_width:7;  //The width of frames (unit: MB) (ex. 720/16)
			unsigned short reg04_reserved: 1;
#if defined(_TWO_CORES_) || defined(_M4VE_T3_)
			unsigned short pic_height:7; //The height of frame (unit: MB) (ex. 1024/16)
#else
			unsigned short pic_height:6; //The height of frame (unit: MB) (ex. 480/16)
#endif
		};
		unsigned short reg04;
	};

	union {
		struct
		{
			unsigned short search_4block_range:1; // 0: -2 ~ 2, 1: -1 ~ 1 (default)
			unsigned short reg_m4ve_mode_threshold:15;  //default value is 1024. It's for inter/intra mode decision.
		};
		unsigned short reg05;
	};
#endif
	union {
		struct
		{
			unsigned short ref_y_base_adr_low:16; //Base address of y frame of reference picture
		};
		unsigned short reg06;
	};

	union {
		struct
		{
			unsigned short ref_y_base_adr_high:BASE_ADR_HIGH_BITS; //Base address of y frame of reference picture
		};
		unsigned short reg07;
	};

	union {
		struct
		{
			unsigned short ref_cb_base_adr_low:16; //Base address of cb frame of reference picture
		};
		unsigned short reg08;
	};

	union {
		struct
		{
			unsigned short ref_cb_base_adr_high:BASE_ADR_HIGH_BITS; //Base address of cb frame of reference picture
		};
		unsigned short reg09;
	};

	union {
		struct
		{
			unsigned short mc_clk_ctl_en:1;
			unsigned short mve_clk_ctl_en:1;
			unsigned short hdg_clk_ctl_en:1;
			unsigned short me_clk_ctl_en:1;
			unsigned short be_clk_ctl_en:1;
			unsigned short cvle_clk_ctl_en:1;
			unsigned short sgc_clk_ctl_en:1;
			unsigned short mb_rc_en:1;
#if defined(_M4VE_T3_) || defined(_M4VE_BIG2_)
            unsigned short reg0a_reserved:4;
            unsigned short crc_mode10:2;
            unsigned short crc_select:1;
            unsigned short crc_rst:1;
#else
			unsigned short reg0a_reserved:8;
#endif
		};
		unsigned short reg0a;
	};

	union {
		struct
		{
			unsigned short reg0b_reserved:16;
		};
		unsigned short reg0b;
	};

	union {
		struct
		{
			unsigned short cur_y_base_adr_low:16; //Base address of y frame of current picture
		};
		unsigned short reg0c;
	};

	union {
		struct
		{
			unsigned short cur_y_base_adr_high:BASE_ADR_HIGH_BITS; //Base address of y frame of current picture
		};
		unsigned short reg0d;
	};

	union {
		struct
		{
			unsigned short cur_cb_cr_base_adr_low:16; //Base address of cbcr frame of current picture
		};
		unsigned short reg0e;
	};

	union {
		struct
		{
			unsigned short cur_cb_cr_base_adr_high:BASE_ADR_HIGH_BITS; //Base address of cbcr frame of current picture
		};
		unsigned short reg0f;
	};

	union {
		struct
		{
			unsigned short rec_y_base_adr_low:16; //Base address of y frame of reconstructed picture
		};
		unsigned short reg10;
	};

	union {
		struct
		{
			unsigned short rec_y_base_adr_high:BASE_ADR_HIGH_BITS; //Base address of y frame of reconstructed picture
		};
		unsigned short reg11;
	};

	union {
		struct
		{
			unsigned short rec_u_base_adr_low:16; //Base address of u frame of reconstructed picture
		};
		unsigned short reg12;
	};
	union {
		struct
		{
			unsigned short rec_u_base_adr_high:BASE_ADR_HIGH_BITS; //Base address of u frame of reconstructed picture
		};
		unsigned short reg13;
	};

	union {
		struct
		{
			unsigned short reg14_reserved:16;
		};
		unsigned short reg14;
	};
	union {
		struct
		{
            //unsigned short reg15_reserved:3;
            unsigned short me_cur_rpri:1;
            unsigned short me_ref_rpri:1;
            unsigned short mc_wpri:1;
            unsigned short reg_m4ve_h263_resync:2; //H263 Resync Marker(00: no resync marker; 01: every 1MB row; 10: every 2MB rows; 11: every 4 MB rows)
        /*
            2・b00: resync off
            2・b01: every 1 MB row except the 1st one (size < 704x576)
            2・b10: every 2 MB rows except the 1st one
            2・b11: every 4 MB rows except the 1st one (current RTL can・t support it, I just reserve its definition first)
        */
#ifdef _M4VE_BIG2_
            unsigned short reg_m4ve_round_ctl:1;
#endif
		};
		unsigned short reg15;
	};

	union {
		struct
		{
			unsigned short mc_wblen_th:4; //MC module MIU burst length threshold (<=7)
			unsigned short mc_wblen_mode:5; //MC module MIU burst length mode, M4VE will stop sending MIU commands after how many miu commands sent  (<=31)
			unsigned short debug_mode:6; //MC module MIU burst length mode
		};
		unsigned short reg16;
	};

	union {
		struct
		{
			unsigned short irq_mask_me_err0:1;
			unsigned short irq_mask_me_err1:1;
			unsigned short irq_mask_sgc_enlines:1;
			unsigned short irq_mask_sgc_frmdone:1;
			unsigned short irq_mask_sgc_bsbufuth:1;
			unsigned short irq_mask_mc_err:1;
			unsigned short irq_mask_mc_frame_done:1;
#ifdef _M4VE_T3_
            unsigned short irq_mask_miu_check:1;
			unsigned short reg17_reserved:8;  // M4VE irq mask, only 7 bits available now
#else
		    unsigned short reg17_reserved:9;  // M4VE irq mask, only 7 bits available now
#endif
		};
		unsigned short reg17;
	};

	union {
		struct
		{
#ifdef _M4VE_T3_
            unsigned short irq_force:8; // M4VE irq force
#else
			unsigned short irq_force:7; // M4VE irq force
#endif
		};
		unsigned short reg18;
	};

	union {
		struct
		{
			unsigned short irq_clr_me_err0:1;
			unsigned short irq_clr_me_err1:1;
			unsigned short irq_clr_sgc_enlines:1;
			unsigned short irq_clr_sgc_frmdone:1;
			unsigned short irq_clr_sgc_bsbufuth:1;  //bsbfuth irq for bitstream buf full
			unsigned short irq_clr_mc_err:1;
			unsigned short irq_clr_mc_frame_done:1;
#ifdef _M4VE_T3_
            unsigned short irq_clr_miu_check:1; //(miu wadr out of boundary)
            unsigned short reg19_reserved:8;  // M4VE irq mask, only 7 bits available now
#else
			unsigned short reg19_reserved:9;  // M4VE irq mask, only 7 bits available now
#endif
		};
		unsigned short reg19;
	};

	union {
		struct
		{
			unsigned short sgc_bsbfsadr_low:16; //Bitstream buffer segment start address (double-word address)
		};
		unsigned short reg1a;
	};

	union {
		struct
		{
			unsigned short sgc_bsbfsadr_high:BASE_ADR_HIGH_BITS; //Bitstream buffer segment start address (double-word address)
		};
		unsigned short reg1b;
	};

	union {
		struct
		{
			unsigned short sgc_bsbfeadr_low:16; //Bitstream buffer segment end address (double-word address)
		};
		unsigned short reg1c;
	};

	union {
		struct
		{
			unsigned short sgc_bsbfeadr_high:BASE_ADR_HIGH_BITS; //Bitstream buffer segment end address (double-word address)
		};
		unsigned short reg1d;
	};

	union {
		struct
		{
			unsigned short sgc_bsbfrptr_low:16; //Bitstream buffer read pointer (double-word address)
		};
		unsigned short reg1e;
	};

	union {
		struct
		{
			unsigned short sgc_bsbfrptr_high:BASE_ADR_HIGH_BITS; //Bitstream buffer read pointer (double-word address)
			//unsigned short reg1f_reserved:2;
			unsigned short sgc_putrptr: 1; // SW already put new value to read pointer
		};
		unsigned short reg1f;
	};

	union {
		struct
		{
			unsigned short sgc_swbfwptr_low:16; //Bitstream buffer updated write pointer (double-word address)
		};
		unsigned short reg20;
	};

	union {
		struct
		{
			unsigned short sgc_swbfwptr_high:BASE_ADR_HIGH_BITS; //Bitstream buffer updated write pointer (double-word address)
			//unsigned short reg21_reserved:2;
			unsigned short sgc_putwptr: 1; // SW already put new value to write pointer
		};
		unsigned short reg21;
	};

	union {
		struct
		{
			unsigned short sgc_bsbfth:12; //Bitstream buffer space threshold (unit: dword)
		};
		unsigned short reg22;
	};

	union {
		struct
		{
			unsigned short sgc_bsbfwptr_low:16; //Bistream buffer current write pointer (double-word address)
		};
		unsigned short reg23;
	};

	union {
		struct
		{
			unsigned short sgc_bsbfwptr_high:BASE_ADR_HIGH_BITS; //Bistream buffer current write pointer (double-word address)
			//unsigned short reg24_reserved:2;
			unsigned short sgc_readwptr: 1; // SW already get value of write pointer
		};
		unsigned short reg24;
	};

	union {
		struct
		{
			unsigned short enc_linesth:6; //The line threshold for encode (unit: 16 lines)
			unsigned short reg25_reserved:1;
			unsigned short sgc_wpri: 1; // SGC write priority for MIU
			unsigned short vlc_uidv: 1; // Use_Intra_DC_VLC enable
			unsigned short sgc_ptrvld: 1; // SGC R/W pointer valid (init once in the beginning)
            /*  if you want to set reg_sgc_bsbfsadr, reg_sgc_bsbfeadr, reg_sgc_bsbfrptr and reg_sgc_swbfwptr
                set sgc_ptrvld to 0 before you set new address, and set sgc_ptrvld to 1 after you set new address
            */
            //unsigned short sgc_multi_obuf: 1;    // multi-obuf enable
            unsigned short rst_frag_num: 1; //reset frag number to 0
            unsigned short sgc_multi_obuf_set: 1;  // set after reg_sgc_bsbfsadr, reg_sgc_bsbfeadr, reg_sgc_bsbfrptr, and reg_sgc_swbfwptr are set to new address

		};
		unsigned short reg25;
	};

	union {
		struct
		{
			unsigned short sgc_bsbitlen_low:16; //Encoded bitstream length (unit: byte)
		};
		unsigned short reg26;
	};

	union {
		struct
		{
			unsigned short sgc_bsbitlen_high:BASE_ADR_HIGH_BITS; //Encoded bitstream length (unit: byte)
			unsigned short reg27_reserved:7;
			unsigned short sgc_clrblen: 1; // SW clear bitstream length counter
		};
		unsigned short reg27;
	};

	union {
		struct
		{
			unsigned short sgc_cpubsout:16; //SW put_bits() bistream
		};
		unsigned short reg28;
	};

	union {
		struct
		{
			unsigned short sgc_cpubslen:5; //SW put_bits() length (unit: bit)
			unsigned short reg29_reserved:3;
			unsigned short sgc_cpubsvld:1; //SW put_bits() valid
			unsigned short sgc_cpubsend:1; //SW put_bits() end enable
			unsigned short sgc_cpuwrdy:1; //Ready for SW put_bits()
		};
		unsigned short reg29;
	};

	union {
		struct
		{
			unsigned short quant_type:1; //Quantization type (0: H263, 1: mpeg4)
			unsigned short qscale:5; //Quantization scaler value (QP)
		};
		unsigned short reg2a;
	};

	union {
		struct
		{
			unsigned short mc_err_th:12; //MC error count threshold
			unsigned short mc_err_en:1; //MC error flag enable
		};
		unsigned short reg2b;
	};

	union {
		struct
		{
			unsigned short m4ve_mbx_match:7; //MC error count threshold
			unsigned short m4ve_mby_match:7; //MC error flag enable
		};
		unsigned short reg2c;
	};

	union {
		struct
		{
#ifdef _M4VE_BIG2_
			unsigned short h263_unit:1; //MC error count threshold
#elif defined(_M4VE_T3_)
            unsigned short crc_mode2:1; //MC error count threshold
#endif
			unsigned short reg31_reserved:15; //MC error flag enable
		};
		unsigned short reg31;
	};

	union {
		struct
		{
			unsigned short reg_m4ve_mbr_left_bound: 7;
			unsigned short reg_m4ve_mbr_right_bound: 7;
		};
		unsigned short reg50;
	};

	union {
		struct
		{
#if defined(_M4VE_BIG2_) || defined(_M4VE_TRITON_)
            unsigned short reg_m4ve_mbr_top_bound: 6;
			unsigned short reg_m4ve_mbr_bot_bound: 6;
#else
			unsigned short reg_m4ve_mbr_top_bound: 7;
			unsigned short reg_m4ve_mbr_bot_bound: 7;
#endif
		};
		unsigned short reg51;
	};

	union {
		struct
		{
			unsigned short reg_m4ve_mbr_target_bits_shift_left: 2;
			unsigned short reg_m4ve_mbr_target_bits_shift_right: 2;
			unsigned short reg_m4ve_mbr_i_vop_dq_inc_value: 2;
			unsigned short reg_m4ve_mbr_i_vop_var_comp: 5;
			unsigned short reg_m4ve_mbr_p_vop_dq_inc_value: 2;
		};
		unsigned short reg52;
	};

	union {
		struct
		{
			unsigned short reg_m4ve_mbr_p_vop_var_comp: 5;
			unsigned short reg_m4ve_mbr_b_vop_var_comp: 5;
#if defined(_M4VE_BIG2_)||defined(_TWO_CORES_)||defined(_M4VE_T3_)
			unsigned short reg_m4ve_mbr_var_fixed_value: 6;
#endif
		};
		unsigned short reg53;
	};

	union {
		struct
		{
			unsigned short reg_m4ve_me_s_16: 1;   //0: M4VE ME Search range 32, 1: Search range 16
			unsigned short reg_m4ve_pskip_off: 1; //0: p skip function off, 1: p skip function on
			unsigned short reg_m4ve_filter_en: 1; //0: average filter function off, 1: average filter function on
		};
		unsigned short reg54;
	};

	union {
		struct
		{
			unsigned short reg_m4ve_f3x3_mode: 3; // average filter mode
			unsigned short reg_m4ve_f3x3_sw_rpri_count: 8;
			unsigned short reg_m4ve_f3x3_sw_rpri_en: 1;
			unsigned short reg_m4ve_f3x3_sw_rpri: 1;
		};
		unsigned short reg58;
	};

    union {
        struct
        {
            unsigned short reg_m4ve_sad_minus_value: 11; // sad16 (0,0) will minus this value
        };
        unsigned short reg59;
    };

	union {
		struct
		{
			unsigned short irq_cpu_me_err0:1;
			unsigned short irq_cpu_me_err1:1;
			unsigned short irq_cpu_sgc_enlines:1;
			unsigned short irq_cpu_sgc_frmdone:1;
			unsigned short irq_cpu_sgc_bsbufuth:1;
			unsigned short irq_cpu_mc_err:1;
			unsigned short irq_cpu_mc_frame_done:1;
#ifdef _M4VE_T3_
            unsigned short irq_cpu_miu_check:1;
            unsigned short reg60_reserved:8;  // M4VE irq mask, only 7 bits available now
#else
			unsigned short reg60_reserved:9;  // M4VE irq mask, only 7 bits available now
#endif
		};
		unsigned short reg60;
	};

	union {
		struct
		{
			unsigned short irq_ip_me_err0:1;
			unsigned short irq_ip_me_err1:1;
			unsigned short irq_ip_sgc_enlines:1;
			unsigned short irq_ip_sgc_frmdone:1;
			unsigned short irq_ip_sgc_bsbufuth:1;
			unsigned short irq_ip_mc_err:1;
			unsigned short irq_ip_mc_frame_done:1;
#ifdef _M4VE_T3_
            unsigned short irq_ip_miu_check:1;
			unsigned short reg61_reserved:8;  // M4VE irq mask, only 7 bits available now
#else
			unsigned short reg61_reserved:9;  // M4VE irq mask, only 7 bits available now
#endif
		};
		unsigned short reg61;
	};

	union {
		struct
		{
			unsigned short m4ve_mbx_count:7;
#if defined(_TWO_CORES_) || defined(_M4VE_T3_)
			unsigned short m4ve_mby_count:7;
#else
            unsigned short m4ve_mby_count:6;
#endif
		};
		unsigned short reg62;
	};

	union {
		struct
		{
			unsigned short sgc_fendptr_low:16; //Bistream buffer current write pointer (double-word address)
		};
		unsigned short reg64;
	};

	union {
		struct
		{
			unsigned short sgc_fendptr_high:BASE_ADR_HIGH_BITS;
			unsigned short sgc_frag_num:3; //Encoded bitstream last dword unused byte number
		};
		unsigned short reg65;
	};

	union {
		struct
		{
			unsigned short tcount_one_mb:15;
		};
		unsigned short reg67;
	};

	union {
		struct
		{
			unsigned short time_4b:15;
		};
		unsigned short reg68;
	};

	union {
		struct
		{
			unsigned short time_hmb:15;
		};
		unsigned short reg69;
	};

	union {
		struct
		{
			unsigned short time_h4b:15;
		};
		unsigned short reg70;
	};

	union {
		struct
		{
			unsigned short time_cbcr:15;
		};
		unsigned short reg71;
	};

	union {
		struct
		{
			unsigned short reg_m4ve_lower_mbqp_diff_f:3; //minMBQP=(FrameQP-reg_lower_MBQP_diff)<1?1:(FrameQP-reg_lower_MBQP_diff)
			unsigned short reg_m4ve_upper_mbqp_diff_f:3; //maxMBQP=(FrameQP+reg_upper_MBQP_diff)>31?31:(FrameQP+reg_upper_MBQP_diff)
			unsigned short reg72_reserved:2;
			unsigned short reg_m4ve_lower_mbqp_diff_v:3; //lower bound between current MB and vertical neighbor
			unsigned short reg_m4ve_upper_mbqp_diff_v:3; //upper bound between current MB and vertical neighbor
		};
		unsigned short reg72;
	};

	union {
		struct
		{
			unsigned short reg_m4ve_lower_mbqp_diff_h:3; //lower bound between current MB and horizontal neighbor
			unsigned short reg_m4ve_upper_mbqp_diff_h:3; //upper bound between current MB and horizontal neighbor
			unsigned short reg_m4ve_avgact_coeff:2;
			unsigned short reg_m4ve_var_coeff:2;
		};
		unsigned short reg73;
	};

	union {
		struct
		{
			unsigned short reg_m4ve_mbqp_rounding:4;
			unsigned short reg_m4ve_nbits_weighting:4;
			unsigned short reg_m4ve_avgact_weighting:4;
		};
		unsigned short reg74;
	};

	union {
		struct
		{
			unsigned short reg_m4ve_target_mb_bits:12;
		};
		unsigned short reg75;
	};

	union {
		struct
		{
			unsigned short reg_m4ve_itlc:1;
			unsigned short reg_m4ve_field_dct_threshold:9;
		};
		unsigned short reg76;
	};

	union {
		struct
		{
			unsigned short me_rpri_en: 1;
			unsigned short cur_miu_blen: 2;
            //before T3:    0 means 8miu, 1 means 16miu
            //after T3:     0 means 4miu, 1 means 8miu
			unsigned short me_cur_rpri_count: 8;
		    unsigned short sw_miu_blen: 3;
            //before T3:    0 means 8miu, 1 means 16miu
            //after T3:     0 means 4miu, 1 means 8miu
			unsigned short umvc_sw_rpri_en: 1;
			unsigned short umvc_sw_rpri: 1;
		};
		unsigned short reg77;
	};

	union {
		struct
		{
			unsigned short me_sw_rpri_count:8;
			unsigned short umvc_sw_rpri_count:8;
		};
		unsigned short reg78;
	};

	union {
		struct
		{
			unsigned short me_umv_en:1;
			unsigned short umvc_th:4;
			unsigned short umvc_blen:5;
		};
		unsigned short reg79;
	};
#if defined(_M4VE_BIG2_) || defined(_TWO_CORES_) || defined(_M4VE_T3_)
	union {
		struct
		{
			unsigned short pvop_sad_weight:5;
			unsigned short reg7a_reserved:11;
		};
		unsigned short reg7a;
	};

	union {
		struct
		{
            unsigned short mby_fak_start:7;
            unsigned short reg7b_reserved:1;
            unsigned short mby_fak_end:7;
            unsigned short reg7b_reserved1:1;
			//unsigned short pvop_sad_thr:16;
		};
		unsigned short reg7b;
	};

	union {
		struct
		{
            unsigned short enc_height:6;
#ifdef _M4VE_T3_
            unsigned short miu_off:1;
            unsigned short reg7c_reserved:9;
#else
            unsigned short reg7c_reserved:10;
#endif
			//unsigned short pvop_var_thr0:8;
			//unsigned short pvop_abs_thr0:8;
		};
		unsigned short reg7c;
	};

	union {
		struct
		{
			unsigned short pvop_mean_min:8;
			unsigned short pvop_mean_max:8;
		};
		unsigned short reg7d;
	};

	union {
		struct
		{
			unsigned short pvop_var_thr1:8;
			unsigned short pvop_var_thr2:8;
		};
		unsigned short reg7e;
	};

	union {
		struct
		{
			unsigned short pvop_var_thr3:8;
			unsigned short pvop_abs_thr2:8;
		};
		unsigned short reg7f;
	};

	union {
		struct
		{
			unsigned short mbx_count:6; //Current processing MBx status for software polling (x direction)
			unsigned short mby_count:6; //Current processing MBx status for software polling (y direction)
		};
		unsigned short reg80;
	};
#ifdef _M4VE_T3_
    //bank1
    union {
        struct
        {
            unsigned short sw_qtab_en:1;
            unsigned short wadr_oob_chk:1; //wadr out of boundary check enable
            unsigned short regc0_reserved:14;
        };
        unsigned short regc0;
	};
	union {
		struct
		{
            unsigned short wadr_ubound0_low:16; //sgc wadr upper bound 0
		};
		unsigned short regc1;
	};
    union {
        struct
        {
            unsigned short wadr_ubound0_high:BASE_ADR_HIGH_BITS; //sgc wadr upper bound 0
        };
        unsigned short regc2;
	};
    union {
        struct
        {
            unsigned short wadr_lbound0_low:16; //sgc wadr lower bound 0
        };
        unsigned short regc3;
    };
    union {
        struct
        {
            unsigned short wadr_lbound0_high:BASE_ADR_HIGH_BITS; //sgc wadr lower bound 0
        };
        unsigned short regc4;
	};
    union {
        struct
        {
            unsigned short wadr_ubound1_low:16; //MC wadr upper bound 1
        };
        unsigned short regc5;
    };
    union {
        struct
        {
            unsigned short wadr_ubound1_high:BASE_ADR_HIGH_BITS; //MC wadr upper bound 1
        };
        unsigned short regc6;
	};
    union {
        struct
        {
            unsigned short wadr_lbound1_low:16; //MC wadr lower bound 1
        };
        unsigned short regc7;
    };
    union {
        struct
        {
            unsigned short wadr_lbound1_high:BASE_ADR_HIGH_BITS; //MC wadr lower bound 1
        };
        unsigned short regc8;
	};
    union {
        struct
        {
            unsigned short miu_adr_oob0_low:16; //wadr out of boundary 0
        };
        unsigned short regc9;
    };
    union {
        struct
        {
            unsigned short miu_adr_oob0_high:BASE_ADR_HIGH_BITS; //wadr out of boundary 0
        };
        unsigned short regca;
	};
    union {
        struct
        {
            unsigned short miu_adr_oob1_low:16; //wadr out of boundary 1
        };
        unsigned short regcb;
    };
    union {
        struct
        {
            unsigned short miu_adr_oob1_high:BASE_ADR_HIGH_BITS; //wadr out of boundary 1
        };
        unsigned short regcc;
	};
#endif //defined(_M4VE_T3_)
#endif //defined(_M4VE_BIG2_) || defined(_TWO_CORES_) || defined(_M4VE_T3_)
} M4VE_REG;

#endif
