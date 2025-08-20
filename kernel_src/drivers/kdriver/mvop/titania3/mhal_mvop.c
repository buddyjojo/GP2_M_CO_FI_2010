#include "mhal_mvop.h"

#include "mhal_chiptop_reg.h"
//#define ENABLE_MVOP_DUPLICATE //LGE gbtogether(080909) by Toni

/******************************************************************************/
/// Initialize VOP hardware and set it to hardwire mode
/******************************************************************************/
void MHal_MVOP_Init()
{
    // DC0 clock [4] = 0 : enable DC0 clock
    //TOP_REG(REG_TOP_MVD_DC0_RVD_GE) &= ~(BIT4);
    CKLGEN0_REG(REG_CLKGEN0_DC) &= ~(CLKGEN0_CKG_DC0_DIS);


    // 0x2D [0] = 0 : enable PSRAM clock
	//TOP_REG(0x002D) &= 0xFFFE ;
	CKLGEN0_REG(REG_CLKGEN0_PSRAM) &= ~(CLKGEN0_CKG_PSRAM0_DIS);

    // Y gray level
	MVOP_REG(MVOP_TST_IMG)= 0x40;
	//
	MHal_MVOP_Input_Mode( MVOPINPUT_HARDWIRE, NULL );
	// disable MVOP
    MHal_MVOP_Enable(FALSE);
}

/******************************************************************************/
/// Set enable black background
/******************************************************************************/
void MHal_MVOP_EnableBlackBg ( void )
{
    U32 regval;

    //set MVOP test pattern to black
    // Y:0x10
    MVOP_REG(MVOP_TST_IMG) = (MVOP_REG(MVOP_TST_IMG) & 0x00ff ) | 0x1000 ;
    // U:0x80 V:0x80
    MVOP_REG(MVOP_U_PAT) = 0x8080 ;

    regval = MVOP_REG(MVOP_TST_IMG);
    // frame color {reg_y, reg_u, reg_v (0x10, 0x80, 0x80)}
    MVOP_REG(MVOP_TST_IMG) = (MVOP_REG(MVOP_TST_IMG) & 0xfff8 ) | 0x02 ;
    mb();
    //MVOP_REG(MVOP_TST_IMG) = (MVOP_REG(MVOP_TST_IMG) & 0xfffc ) | 0x00 ;
    MVOP_REG(MVOP_TST_IMG) = regval;
    mb();
}

/******************************************************************************/
/// Enable and Disable VOP
/// @param bEnable \b IN
///   - # TRUE  Enable
///   - # FALSE Disable and reset
/******************************************************************************/
void MHal_MVOP_Enable ( B16 bEnable )
{
    //joelin44 2008-06-24: mark out
    //#if MIU_PROTECT
    //MDrv_MIU_SetReqMask(MVOP_R, 1);
    //MDrv_Timer_Delayms(1);
    //#endif

#if 1
    U32 regval;
    if ( bEnable )
    {
        // croma weighting off
        MVOP_REG(MVOP_STRIP_ALIGN) |= 0x02;
        // normal image from DRAM.
        MVOP_REG(MVOP_TST_IMG) = (MVOP_REG(MVOP_TST_IMG) & 0xfffc );
        // active register 0x20 ~ 0x26.
        MVOP_REG(MVOP_REG_WR) |= BIT0;
        MVOP_REG(MVOP_REG_WR) &= ~BIT0;

        regval = MVOP_REG(MVOP_CTRL0);
        mb();
        regval |= 0x1;
    }
    else
    {
        // set VOP test pattern to black
        // Y:0x10 U:0x80 V:0x80
        MVOP_REG(MVOP_TST_IMG) = (MVOP_REG(MVOP_TST_IMG) & 0x00ff ) | 0x1000 ;
        MVOP_REG(MVOP_U_PAT) = 0x8080 ;
        // frame color {reg_y, reg_u, reg_v (0x10, 0x80, 0x80)}
        MVOP_REG(MVOP_TST_IMG) = (MVOP_REG(MVOP_TST_IMG) & 0xfffc ) | 0x02 ;

        // make changed registers take effect
        MVOP_REG(MVOP_REG_WR) |= BIT0;
        MVOP_REG(MVOP_REG_WR) &= ~BIT0;

        regval = MVOP_REG(MVOP_CTRL0);
        mb();
        regval &= ~0x1;
    }
    mb();

    if(VOP_ON_MIU1)
    {
        MIU_MASK_MVOP_1;
        // enable/disable MVOP
        MVOP_REG(MVOP_CTRL0) = regval;
        MIU_UNMASK_MVOP_1;
    }
    else
    {
        MIU_MASK_MVOP_0;
    // enable/disable MVOP
    MVOP_REG(MVOP_CTRL0) = regval;
        MIU_UNMASK_MVOP_0;
    }


#else       //for triton
    if ( bEnable )
    {
        //XBYTE[VOP_TST_IMG] |= 0x20;  //enable OSD
        //MVOP_REG(MVOP_TST_IMG) |= 0x20;  //enable OSD
        //XBYTE[VOP_INPUT_SWITCH] |= 0x01;  //add Audio
        MVOP_REG(MVOP_INPUT_SWITCH) |= 0x0100;  //add Audio
        //XBYTE[VOP_STRIP_ALIGN] |= 0x02;  //Croma weighting off
        MVOP_REG(MVOP_STRIP_ALIGN) |= 0x02;  //Croma weighting off

        //XBYTE[VOP_TST_IMG] &= 0xFC;
        MVOP_REG(MVOP_TST_IMG) = (MVOP_REG(MVOP_TST_IMG) & 0xfffc );
        // make changed registers take effect
        //XBYTE[VOP_REG_WR] |= BIT0;
        //XBYTE[VOP_REG_WR] &= ~BIT0;
        MVOP_REG(MVOP_REG_WR) |= BIT0;
        MVOP_REG(MVOP_REG_WR) &= ~BIT0;
        //XBYTE[VOP_CTRL0] |= 0x01;
        MVOP_REG(MVOP_CTRL0) |= 0x01;
     }
     else
     {
        //set VOP test pattern to black
        //XBYTE[VOP_TST_IMG + 1] = 0x10;
        //XBYTE[VOP_U_PAT      ] = 0x80;
        //XBYTE[VOP_U_PAT   + 1] = 0x80;
        MVOP_REG(MVOP_TST_IMG) = (MVOP_REG(MVOP_TST_IMG) & 0x00ff ) | 0x1000 ;
        MVOP_REG(MVOP_U_PAT) = 0x8080 ;

        //XBYTE[VOP_TST_IMG] &= 0xFC;
        //XBYTE[VOP_TST_IMG] |= 0x02;
        MVOP_REG(MVOP_TST_IMG) = (MVOP_REG(MVOP_TST_IMG) & 0xfffc ) | 0x02 ;


        // make changed registers take effect
        //XBYTE[VOP_REG_WR] |= BIT0;
        //XBYTE[VOP_REG_WR] &= ~BIT0;
        MVOP_REG(MVOP_REG_WR) |= BIT0;
        MVOP_REG(MVOP_REG_WR) &= ~BIT0;

        //msAPI_Timer_Delayms(100);
        //XBYTE[VOP_CTRL0] &= 0xFE;
        MVOP_REG(MVOP_CTRL0) &= 0xFE;

    }
#endif
    //joelin44 2008-06-24: mark out
    //#if MIU_PROTECT
    //MDrv_Timer_Delayms(1);
    //MDrv_MIU_SetReqMask(MVOP_R, 0);
    //#endif
}


void MHal_MVOP_SetOutputTiming ( MS_MVOP_TIMING *ptiming )
{
    U16 u16TF_VS, u16BF_VS;
    U32 regval;

    //XBYTE[VOP_FRAME_VCOUNT    ] = LOWBYTE( ptiming->u16V_TotalCount );
    //XBYTE[VOP_FRAME_VCOUNT + 1] = HIGHBYTE( ptiming->u16V_TotalCount );
    MVOP_REG(MVOP_FRAME_VCOUNT) = ptiming->u16V_TotalCount ;

    //XBYTE[VOP_FRAME_HCOUNT    ] = LOWBYTE( ptiming->u16H_TotalCount );
    //XBYTE[VOP_FRAME_HCOUNT + 1] = HIGHBYTE( ptiming->u16H_TotalCount );
    MVOP_REG(MVOP_FRAME_HCOUNT) = ptiming->u16H_TotalCount   ;

    //XBYTE[VOP_VB0_STR     ] = LOWBYTE( ptiming->u16VBlank0_Start );
    //XBYTE[VOP_VB0_STR  + 1] = HIGHBYTE( ptiming->u16VBlank0_Start );
    MVOP_REG(MVOP_VB0_STR) = ptiming->u16VBlank0_Start    ;
    //XBYTE[VOP_VB0_END     ] = LOWBYTE( ptiming->u16VBlank0_End );
    //XBYTE[VOP_VB0_END  + 1] = HIGHBYTE( ptiming->u16VBlank0_End );
    MVOP_REG(MVOP_VB0_END) =   ptiming->u16VBlank0_End   ;
    //XBYTE[VOP_VB1_STR     ] = LOWBYTE( ptiming->u16VBlank1_Start );
    //XBYTE[VOP_VB1_STR  + 1] = HIGHBYTE( ptiming->u16VBlank1_Start );
    MVOP_REG(MVOP_VB1_STR) =    ptiming->u16VBlank1_Start  ;
    //XBYTE[VOP_VB1_END     ] = LOWBYTE( ptiming->u16VBlank1_End );
    //XBYTE[VOP_VB1_END  + 1] = HIGHBYTE( ptiming->u16VBlank1_End );
    MVOP_REG(MVOP_VB1_END) =    ptiming->u16VBlank1_End      ;

    //Progressive use original setting, interlace use new setting for 420CUP
    if(!ptiming->bInterlace)//victor 20090112, Feathering in DTV, Fix MVOP setting
    {
        //XBYTE[VOP_TF_STR      ] = LOWBYTE( ptiming->u16TopField_Start );
        //XBYTE[VOP_TF_STR   + 1] = HIGHBYTE( ptiming->u16TopField_Start );
        MVOP_REG(MVOP_TF_STR) =    ptiming->u16TopField_Start      ;//victor 20090112, Feathering in DTV, Fix MVOP setting
        //XBYTE[VOP_BF_STR      ] = LOWBYTE( ptiming->u16BottomField_Start );
        //XBYTE[VOP_BF_STR   + 1] = HIGHBYTE( ptiming->u16BottomField_Start );
        MVOP_REG(MVOP_BF_STR) =    ptiming->u16BottomField_Start   ;//victor 20090112, Feathering in DTV, Fix MVOP setting
    }
    //XBYTE[VOP_HACT_STR    ] = LOWBYTE( ptiming->u16HActive_Start );
    //XBYTE[VOP_HACT_STR + 1] = HIGHBYTE( ptiming->u16HActive_Start );
    MVOP_REG(MVOP_HACT_STR) =    ptiming->u16HActive_Start   ;
    // Use VOP_VS_OFFSET in S3.
    // Instead, we use VOP_TF_VS & VOP_BF_VS directly to locate Vsync.
    u16TF_VS = ptiming->u16VBlank0_Start + (U16)ptiming->u8VSync_Offset;
    //XBYTE[VOP_TF_VS      ] = LOWBYTE( u16TF_VS );
    //XBYTE[VOP_TF_VS   + 1] = HIGHBYTE( u16TF_VS );
    MVOP_REG(MVOP_TF_VS)   =         u16TF_VS;
    if(ptiming->bInterlace)//victor 20090112, Feathering in DTV, Fix MVOP setting
        MVOP_REG(MVOP_TF_STR)  =    u16TF_VS;//victor 20090112, Feathering in DTV, Fix MVOP setting
    u16BF_VS = ptiming->u16VBlank1_Start + (U16)ptiming->u8VSync_Offset;
    if (ptiming->bInterlace) u16BF_VS -= 1;
    //XBYTE[VOP_BF_VS      ] = LOWBYTE( u16BF_VS );
    //XBYTE[VOP_BF_VS   + 1] = HIGHBYTE( u16BF_VS );
    MVOP_REG(MVOP_BF_VS)   =         u16BF_VS;
    if(ptiming->bInterlace)//victor 20090112, Feathering in DTV, Fix MVOP setting
        MVOP_REG(MVOP_BF_STR)  =    u16BF_VS;//victor 20090112, Feathering in DTV, Fix MVOP setting
    // + S3, set default IMG_HSTR, IMG_VSTR0, IMG_VSTR1
    //XBYTE[VOP_IMG_HSTR    ] = LOWBYTE( ptiming->u16HActive_Start );
    //XBYTE[VOP_IMG_HSTR + 1] = HIGHBYTE( ptiming->u16HActive_Start );
    MVOP_REG(MVOP_IMG_HSTR)   =         ptiming->u16HActive_Start;
    //XBYTE[VOP_IMG_VSTR0   ] = LOWBYTE( ptiming->u16VBlank0_End );
    //XBYTE[VOP_IMG_VSTR0+ 1] = HIGHBYTE( ptiming->u16VBlank0_End );
    MVOP_REG(MVOP_IMG_VSTR0)   =         ptiming->u16VBlank0_End;
    //XBYTE[VOP_IMG_VSTR1   ] = LOWBYTE( ptiming->u16VBlank1_End );
    //XBYTE[VOP_IMG_VSTR1+ 1] = HIGHBYTE( ptiming->u16VBlank1_End );
    MVOP_REG(MVOP_IMG_VSTR1)   =         ptiming->u16VBlank1_End;
    // select mvop output from frame color(black)
    //XBYTE[VOP_TST_IMG + 1] = 0x10;
    //XBYTE[VOP_U_PAT      ] = 0x80;
    //XBYTE[VOP_U_PAT   + 1] = 0x80;
    MVOP_REG(MVOP_TST_IMG) = (MVOP_REG(MVOP_TST_IMG) & 0x00ff ) | 0x1000 ;
    MVOP_REG(MVOP_U_PAT) = 0x8080 ;

    // set mvop src to test pattern
    regval = MVOP_REG(MVOP_TST_IMG);
    MVOP_REG(MVOP_TST_IMG) = (regval & 0xfffc) | 0x02;
    // make changed registers take effect
    MVOP_REG(MVOP_REG_WR) |= BIT0;
    MVOP_REG(MVOP_REG_WR) &= ~BIT0;

    //#if MIU_PROTECT
    //MDrv_MIU_SetReqMask(MVOP_R, 1); // mask MVOP2MI to protect MIU
    //MDrv_Timer_Delayms(1);
    //#endif

    // reset mvop to avoid timing change cause mvop hang-up
    //XBYTE[VOP_CTRL0] &= ~0x1;
    //XBYTE[VOP_CTRL0] |=  0x1;

    if(VOP_ON_MIU1)
    {
        MIU_MASK_MVOP_1;
    MVOP_REG(MVOP_CTRL0)  &= ~0x1;
        MIU_UNMASK_MVOP_1;
    }
    else
    {
        MIU_MASK_MVOP_0;
        MVOP_REG(MVOP_CTRL0)  &= ~0x1;
        MIU_UNMASK_MVOP_0;
    }

    MVOP_REG(MVOP_CTRL0)  |=  0x1;
    //#if MIU_PROTECT
    //MDrv_Timer_Delayms(1);
    //MDrv_MIU_SetReqMask(MVOP_R, 0); // unmask MVOP2MI
    //#endif
    // select mvop output from mvd
    //XBYTE[VOP_TST_IMG] = 0x00;
    //XBYTE[VOP_TST_IMG] = regval;
    MVOP_REG(MVOP_TST_IMG) = MVOP_REG(MVOP_TST_IMG) & 0xfffc ;
    MVOP_REG(MVOP_TST_IMG) = regval;
#ifdef TQM_MODE //LGE gbtogether(080909) by Toni
    MVOP_REG(MVOP_CTRL0) = (ptiming->bHDuplicate) ? (MVOP_REG(MVOP_CTRL0) | 0x4) : (MVOP_REG(MVOP_CTRL0) & ~0x4);
#endif
    // H pixel duplicate
    //VOP_DBG(printf("VTot=%u\n",ptiming->u16V_TotalCount));
    //VOP_DBG(printf("HTot=%u\n",ptiming->u16H_TotalCount));
    //VOP_DBG(printf("I/P=%bu\n",ptiming->bInterlace));
    //VOP_DBG(printf("FRate=%bu\n",ptiming->u8Framerate));
    //VOP_DBG(printf("HFreq=%u\n",ptiming->u16H_Freq));
    //VOP_DBG(printf("Num=0x%x\n",ptiming->u16Num));
    //VOP_DBG(printf("Den=0x%x\n",ptiming->u16Den));
    //VOP_DBG(printf("W=%u\n",ptiming->u16Width));
    //VOP_DBG(printf("H=%u\n",ptiming->u16Height));
    //VOP_DBG(printf("u16ExpFRate=0x%u\n", ptiming->u16ExpFrameRate));

}

//junyou add for H264 thumbnail<20091001>
void MHal_MVOP_SetH264_Order(B16 bEnable)
{
    if(bEnable == TRUE)
    {
        MVOP_REG(MVOP_REG_WR) |= 0x02;
    }
    else
    {
        MVOP_REG(MVOP_REG_WR) &= ~0x02;
    }
}

/******************************************************************************/
/// Set VOP input mode
/// @param mode \b IN \copydoc VOPINPUTMODE
/// @param pparam \b IN \copydoc VOPINPUTPARAM members should be set for
///   - #VOPINPUT_HARDWIRE     N/A
///   - #VOPINPUT_HARDWIRECLIP HSize and VSize
///   - #VOPINPUT_MCUCTRL      Y Offset, UV offset, HSize adn VSize
/******************************************************************************/
void MHal_MVOP_Input_Mode ( MVOPINPUTMODE mode, MVOPINPUTPARAM *pparam )
{
    //LONG32_BYTE u32tmp;
    U32 regval;
    U16 u16strip;

    //set VOP test pattern to black
    MHal_MVOP_EnableBlackBg();

    // Disable H264 or RM Input [10:11] = (0,0) disable rvd, hvd.
    //XBYTE[VOP_INPUT_SWITCH] = XBYTE[VOP_INPUT_SWITCH] & 0xF3;
    MVOP_REG(MVOP_INPUT_SWITCH) &= 0xF3FF;

    //Disable DRAM H264 frame order read out
    //XBYTE[VOP_REG_WR] &= 0xFD;
    MVOP_REG(MVOP_REG_WR) &= 0xFD;

    //Diable inverse the field for MPEG2
    //XBYTE[VOP_CTRL0+1] &= ~BIT3;
    MVOP_REG(MVOP_REG_WR) &= 0xF7FF;

    //REGVAL = XBYTE[VOP_MPG_JPG_SWITCH];
    //REGVAL = 0;
    //REGVAL |= ( MODE & 0X3 );

// joelin44 2008-06-24: mark out
//#if defined(ENABLE_DMP) || defined(ENABLE_MEDIAPLAYER)
//    // clear extend strip len bit by default
//    XBYTE[0x147E] &= 0xfe;
//    XBYTE[0x147F] = XBYTE[0x147F];
//#endif

    // clear extend strip len bit by default
    //XBYTE[VOP_STRIP_ALIGN] &= 0xfe;
    MVOP_REG(MVOP_STRIP_ALIGN) &= 0xfe;
    //XBYTE[0x147F] = XBYTE[0x147F];

    //XBYTE[VOP_TST_IMG] &= (~(BIT1|BIT0));
    MVOP_REG(MVOP_TST_IMG) &= 0xfffc;

    if ( mode == MVOPINPUT_HARDWIRE )
    {
        //XBYTE[VOP_MPG_JPG_SWITCH] = regval;
//        MVOP_REG(MVOP_MPG_JPG_SWITCH) = (MVOP_REG(MVOP_MPG_JPG_SWITCH) & 0xFFFFFC) | mode;
        MVOP_REG(MVOP_MPG_JPG_SWITCH) = (MVOP_REG(MVOP_MPG_JPG_SWITCH) & 0xFFFF5C) | mode;
    }
    else if ( mode == MVOPINPUT_HARDWIRECLIP )
    {
        //XBYTE[VOP_MPG_JPG_SWITCH] = regval;
		MVOP_REG(MVOP_MPG_JPG_SWITCH) = (MVOP_REG(MVOP_MPG_JPG_SWITCH) & 0xFFFFFC) | mode;

        // HSize, VSize
        //XBYTE[VOP_JPG_HSIZE    ] = LOWBYTE( pparam->u16HSize );
        //XBYTE[VOP_JPG_HSIZE + 1] = HIGHBYTE( pparam->u16HSize );
        MVOP_REG(MVOP_JPG_HSIZE) = pparam->u16HSize ;
        //XBYTE[VOP_JPG_VSIZE    ] = LOWBYTE( pparam->u16VSize );
        //XBYTE[VOP_JPG_VSIZE + 1] = HIGHBYTE( pparam->u16VSize );
        MVOP_REG(MVOP_JPG_VSIZE ) = pparam->u16VSize ;
    }
    else if ( mode == MVOPINPUT_MCUCTRL )
    {
    	regval = MVOP_REG(MVOP_MPG_JPG_SWITCH) ;
		regval = (regval & 0xFFFC) | mode;

        if(pparam != NULL)
        {
            if ( pparam->bProgressive )
                regval |= 0x4;
            else
                regval &= ~0x4;

            if ( pparam->bYUV422 )
                regval |= 0x10;
            else
                regval &= ~0x10;

            if ( pparam->b422pack )
                regval |= 0x80;

            if ( pparam->bDramRdContd == 1 )
                regval |= 0x20;
            else
                regval &= ~0x20;

            // for backward compatable to saturn
            // [3] UV-7bit mode don't care
            // [5] dram_rd_md =0
            // [6] Fld don't care
            // [7] 422pack don'care
            //XBYTE[VOP_MPG_JPG_SWITCH] = regval;

            if (pparam->u16StripSize == 0)
            {
            	if (pparam->bSD)
                	u16strip = 720;
            	else
                	u16strip = 1920;
            }
            else
            {
                u16strip = pparam->u16StripSize;
            }

            // set dc_strip[7:0]
            if ( pparam->bDramRdContd == 0 ) {
                //u16strip = ( ( (U32)u16strip*32/8 ) >> 5);
                u16strip = u16strip/8;
            }
            else {
                if ( pparam->b422pack ) {
                    // [071016 Andy] support YUV422 pack mode
                    if (u16strip < 1024)
                    {
                    	u16strip = u16strip/4;
                        // dont extend strip len
                        //XBYTE[0x147E] &= 0xfe;
                        //XBYTE[0x147F] = XBYTE[0x147F];
                        MVOP_REG(MVOP_STRIP_ALIGN) = MVOP_REG(MVOP_STRIP_ALIGN) & 0xfffe;
                    }
                    else
                    {
                        u16strip = u16strip/8;
                        // extend strip len to 2048
                        //XBYTE[0x147E] |= 1;
                        //XBYTE[0x147F] = XBYTE[0x147F];
                        MVOP_REG(MVOP_STRIP_ALIGN) |= 1;
                    }
                }
                else {
                    //u16strip = ( (U32)u16strip/8 );
                    u16strip = u16strip/8;
                }
            }

            //ASSERT(u16strip < 256 );



            regval = (regval & 0x00ff) | (u16strip<<8) ;
            //XBYTE[VOP_MPG_JPG_SWITCH + 1] = regval;
            MVOP_REG(MVOP_MPG_JPG_SWITCH ) = regval;


            // Y offset
            //u32tmp.u32Num = pparam->u32YOffset >> 3;
            regval = pparam->u32YOffset >> 3;
            //XBYTE[VOP_JPG_YSTR0_L    ] = u32tmp.u8Num[3];
            //XBYTE[VOP_JPG_YSTR0_L + 1] = u32tmp.u8Num[2];
            //XBYTE[VOP_JPG_YSTR0_H    ] = u32tmp.u8Num[1];
    		MVOP_REG(MVOP_JPG_YSTR0_L) =  regval & 0xffff;
            MVOP_REG(MVOP_JPG_YSTR0_H) =  (regval & 0xff0000) >> 16;
            if (pparam->b422pack)
            {
                pparam->u32UVOffset = pparam->u32YOffset + 8;
            }
            // UV offset
            regval = pparam->u32UVOffset >> 3;
            //XBYTE[VOP_JPG_UVSTR0_L    ] = u32tmp.u8Num[3];
            //XBYTE[VOP_JPG_UVSTR0_L + 1] = u32tmp.u8Num[2];
            //XBYTE[VOP_JPG_UVSTR0_H    ] = u32tmp.u8Num[1];
            MVOP_REG(MVOP_JPG_UVSTR0_L) =  regval & 0xffff;
            MVOP_REG(MVOP_JPG_UVSTR0_H) =  (regval & 0xff0000) >> 16;


            // HSize, VSize
            //XBYTE[VOP_JPG_HSIZE    ] = LOWBYTE( pparam->u16HSize );
            //XBYTE[VOP_JPG_HSIZE + 1] = HIGHBYTE( pparam->u16HSize );
            MVOP_REG(MVOP_JPG_HSIZE) =pparam->u16HSize;
            //XBYTE[VOP_JPG_VSIZE    ] = LOWBYTE( pparam->u16VSize );
            //XBYTE[VOP_JPG_VSIZE + 1] = HIGHBYTE( pparam->u16VSize );
            MVOP_REG(MVOP_JPG_VSIZE) = pparam->u16VSize ;
        }
    }

    //DRAM H264 frame order read out
    //XBYTE[VOP_REG_WR] &= ~0x02;
    MVOP_REG(MVOP_REG_WR) &= ~0x02;
    MVOP_REG(MVOP_REG_WR) |= (BIT4| BIT0);
    MVOP_REG(MVOP_REG_WR) &= ~(BIT4|BIT0);


    // For T3 MVD
    MVOP_REG(MVOP_UV_SHT) &= ~BIT4;

    //XBYTE[VOP_REG_WR] |= BIT0;
    //XBYTE[VOP_REG_WR] &= ~BIT0;
}

void MHal_MVOP_Set_YUV_Address ( MVOPINPUTMODE mode, MVOPINPUTPARAM *pparam )
{
    //LONG32_BYTE u32tmp;
    U32 regval;

    if ( mode == MVOPINPUT_MCUCTRL )
    {
        // Y offset
        //u32tmp.u32Num = pparam->u32YOffset >> 3;
        regval = pparam->u32YOffset >> 3;
        //XBYTE[VOP_JPG_YSTR0_L    ] = u32tmp.u8Num[3];
        //XBYTE[VOP_JPG_YSTR0_L + 1] = u32tmp.u8Num[2];
        //XBYTE[VOP_JPG_YSTR0_H    ] = u32tmp.u8Num[1];
        MVOP_REG(MVOP_JPG_YSTR0_L) =  regval & 0xffff;
        MVOP_REG(MVOP_JPG_YSTR0_H) =  (regval & 0xff0000) >> 16;
        if (pparam->b422pack)
        {
            pparam->u32UVOffset = pparam->u32YOffset + 8;
        }
        // UV offset
        regval = pparam->u32UVOffset >> 3;
        //XBYTE[VOP_JPG_UVSTR0_L    ] = u32tmp.u8Num[3];
        //XBYTE[VOP_JPG_UVSTR0_L + 1] = u32tmp.u8Num[2];
        //XBYTE[VOP_JPG_UVSTR0_H    ] = u32tmp.u8Num[1];
        MVOP_REG(MVOP_JPG_UVSTR0_L) =  regval & 0xffff;
        MVOP_REG(MVOP_JPG_UVSTR0_H) =  (regval & 0xff0000) >> 16;
    }
    //DRAM H264 frame order read out
    //XBYTE[VOP_REG_WR] &= ~0x02;
    MVOP_REG(MVOP_REG_WR) &= ~0x02;
    MVOP_REG(MVOP_REG_WR) |= (BIT0);
    MVOP_REG(MVOP_REG_WR) &= ~(BIT0);
    //XBYTE[VOP_REG_WR] |= BIT0;
    //XBYTE[VOP_REG_WR] &= ~BIT0;
}


/******************************************************************************/
/// Set VOP output mode (Progressive / Interlaced)
/// @param bEnable \b IN
///   - # TRUE  Fleid mode (interlaced)
///   - # FALSE Frame mode (progressive)
/******************************************************************************/
void MHal_MVOP_Output_EnableInterlace ( B16 bEnable )
{
    U16 regval;

    regval = MVOP_REG(MVOP_CTRL0);

    if ( bEnable )
    {
        regval |= 0x0080;
    }
    else
    {
        regval &= ~0x0080;
    }

    MVOP_REG(MVOP_CTRL0) = regval;
}


void MHal_MVOP_SetMVOPSynClk ( MS_MVOP_TIMING *ptiming )
{
    // select num/den
    //MDrv_WriteByteMask(REG_CKG_DC0, (0<<6), (3<<6)); // [7:6] set to 0
    //XBYTE[REG_CKG_AEON1DC0] &= ~DC0_CLK_MASK;
    //XBYTE[REG_DC0_NUM  ] = LOWBYTE( ptiming->u16Num);
    //XBYTE[REG_DC0_NUM+1] = HIGHBYTE(ptiming->u16Num);
    //TOP_REG(REG_TOP_M4VD_DC0_DHC_SBM_GE) &= 0xFFE3;
    //TOP_REG(REG_TOP_MCU_USB_STC0) &= ~(1<<14);
    //TOP_REG(0x002D) &= 0xFFE3;
    CKLGEN0_REG(REG_CLKGEN0_PSRAM) &= 0xFFE3;

	//TOP_REG(REG_TOP_M4VD_DC0_DHC_SBM_GE) &= ~(BIT4);
    //TOP_REG(REG_TOP_MVD_DC0_RVD_GE) &= ~(BIT4);
    CKLGEN0_REG(REG_CLKGEN0_DC) &= ~(CLKGEN0_CKG_DC0_DIS);

	//TOP_REG(REG_TOP_DC0_NUM) = ptiming->u16Num;
	CKLGEN0_REG(REG_CLKGEN0_DC0_NUM) = ptiming->u16Num;
	//TOP_REG(REG_TOP_DC0_DEN) = ptiming->u16Den;
    CKLGEN0_REG(REG_CLKGEN0_DC0_DEN) = ptiming->u16Den;
    printk("======mhal mvop==> ptiming->u16Num = 0x%x, ptiming->u16Den = 0x%x\n", ptiming->u16Num, ptiming->u16Den);
//jmkim    printk("MVOP NUM = 0x%4x , Den=0x%4x \n",ptiming->u16Num,ptiming->u16Den);
    //XBYTE[REG_DC0_DEN  ] = LOWBYTE( ptiming->u16Den);
    //XBYTE[REG_DC0_DEN+1] = HIGHBYTE(ptiming->u16Den);
    // set reg_update_dc0_sync_cw[14]=1
//jmkim    printk("REG chip to 0x11 = 0x%4x \n",TOP_REG(REG_TOP_MCU_USB_STC0));
    //TOP_REG(REG_TOP_MCU_USB_STC0) |= (1<<14);
    CKLGEN0_REG(REG_CLKGEN0_STC0_DC0) |= CLKGEN0_UPDATE_DC0_SYNC_CW;


//jmkim	printk("REG chip to 0x11 = 0x%4x \n",TOP_REG(REG_TOP_MCU_USB_STC0));
	//TOP_REG(REG_TOP_MCU_USB_STC0) &= ~(1<<14);
    CKLGEN0_REG(REG_CLKGEN0_STC0_DC0) &= ~(CLKGEN0_UPDATE_DC0_SYNC_CW);

//jmkim	printk("REG chip to 0x11 = 0x%4x \n",TOP_REG(REG_TOP_MCU_USB_STC0));
	//TOP_REG(REG_TOP_MCU_USB_STC0) = (TOP_REG(REG_TOP_MCU_USB_STC0)& 0xBFFF)
    //MDrv_WriteByteMask(REG_UPDATE_DC0_SYNC_CW+1, (1<<6), (1<<6));
    //MDrv_WriteByteMask(REG_UPDATE_DC0_SYNC_CW+1, (0<<6), (1<<6));

}

/******************************************************************************/
/// Set VOP for H264 //Only for MCU Control Mode but we should use Hardwire Mode
/******************************************************************************/
void MHal_MVOP_SetMVOPH264 (U16 u16HorSize, U16 u16VerSize, U16 u16Pitch, B16 bInterlace)
{
    //MCU Control Mode
    //XBYTE[VOP_MPG_JPG_SWITCH] = 0x02;
    //U32 regval;
    MVOP_REG(MVOP_MPG_JPG_SWITCH)  = (MVOP_REG(MVOP_MPG_JPG_SWITCH) & 0xfffc) | 0x02;
    //DRAM H264 frame order read out
    //XBYTE[VOP_REG_WR] = 0x02;
    MVOP_REG(MVOP_REG_WR) = MVOP_REG(MVOP_REG_WR) | 0x02;
    //Set image height and width
    //XBYTE[VOP_JPG_HSIZE] = (U8)(u16HorSize & 0x00ff);
    //XBYTE[VOP_JPG_HSIZE+1] = (U8)((u16HorSize & 0xff00) >> 8);
    MVOP_REG(MVOP_JPG_HSIZE) = u16HorSize;
    //XBYTE[VOP_JPG_VSIZE] = (U8)(u16VerSize & 0x00ff);
    //XBYTE[VOP_JPG_VSIZE+1] = (U8)((u16VerSize & 0xff00) >> 8);
    MVOP_REG(MVOP_JPG_VSIZE) = u16VerSize ;
    //Pitch
    //XBYTE[VOP_MPG_JPG_SWITCH + 1] = (U8)(u16Pitch/8);
    MVOP_REG(MVOP_MPG_JPG_SWITCH)  = (MVOP_REG(MVOP_MPG_JPG_SWITCH) & 0x00ff) | (u16Pitch/8);

    // switch vop input to H.264
    //XBYTE[VOP_INPUT_SWITCH] &= ~(BIT2|BIT3);
    //XBYTE[VOP_INPUT_SWITCH] |= BIT3;
    MVOP_REG(MVOP_INPUT_SWITCH) &= 0xF3FF;
    MVOP_REG(MVOP_INPUT_SWITCH) |= 0x0800;


    //External Field Control
    if(bInterlace)
    {
        //XBYTE[VOP_CTRL0] |= BIT6;
        MVOP_REG(MVOP_CTRL0)|= BIT6;
    }
    else
    {
        //XBYTE[VOP_CTRL0] &= ~BIT6;
        MVOP_REG(MVOP_CTRL0)&= ~BIT6;
	}
    //Force Load
    //XBYTE[VOP_REG_WR] |= BIT4;
    //XBYTE[VOP_REG_WR] &= ~BIT4;
    MVOP_REG(MVOP_REG_WR) |= BIT4;
    MVOP_REG(MVOP_REG_WR) &= ~BIT4;


}

//------------------------------------------------------------------------------
/// Set the mvop in MLink mode
/// @param bMode : 0 for normal DC function, 1 for receiving MLink data and
/// caculating chroma weighting, then send 444 data to scalar.
/// @return None
/// @internal
//------------------------------------------------------------------------------
void MHal_MVOP_SetMlinkMode ( B16 bMode )
{
    U32 regval;
    regval = MVOP_REG(MVOP_REG_WR);

    if ( bMode )
    {
        regval |= BIT8;
    }
    else
    {
        regval &= ~BIT8;
    }

    MVOP_REG(MVOP_REG_WR) = regval;
}

//------------------------------------------------------------------------------
/// Set the Chroma enhance in MLink mode
/// @param bMode : 0 for using DC 444 data to scalar (chroma weight),
/// 1 for Using MLink 444 data to scalar (chroma repeat).
/// @return None
/// @internal
//------------------------------------------------------------------------------
void MHal_MVOP_SetMlinkByPassMode ( B16 bMode )
{
    U32 regval;
    regval = MVOP_REG(MVOP_REG_WR);

    if ( bMode )
    {
        regval |= BIT9;
    }
    else
    {
        regval &= ~BIT9;
    }

    MVOP_REG(MVOP_REG_WR) = regval;
}

#define VOP_HARDWIRE_MODE  (~(BIT1|BIT0))
#define VOP_INPUT_H264  0x0800
#define VOP_INPUT_RM    0x0400
/******************************************************************************/
/// Set VOP for H264  Hardwire Mode
/******************************************************************************/
void MHal_MVOP_SetH264HardwireMode(void)
{
    U16 u16RegValue;

    //XBYTE[VOP_MPG_JPG_SWITCH] &= VOP_HARDWIRE_MODE;
    MVOP_REG(MVOP_MPG_JPG_SWITCH) &= VOP_HARDWIRE_MODE;
    //XBYTE[VOP_TST_IMG] &= (~(BIT1|BIT0));
    MVOP_REG(MVOP_TST_IMG) &= 0xfffc;
    // Don't need to do inverse the field for H264 since H264 f/w version 1.5.8.53
    //XBYTE[VOP_CTRL0+1] |= 0x08;

    //DRAM H264 frame order read out
    //XBYTE[VOP_REG_WR] = 0x02;
    MVOP_REG(MVOP_REG_WR) = MVOP_REG(MVOP_REG_WR) | 0x02;

    //u8RegValue = XBYTE[VOP_INPUT_SWITCH] & 0xF3;
    u16RegValue = MVOP_REG(MVOP_INPUT_SWITCH) & 0xF3FF;
    //XBYTE[VOP_INPUT_SWITCH] = u8RegValue | VOP_INPUT_H264;
    MVOP_REG(MVOP_INPUT_SWITCH) = u16RegValue | VOP_INPUT_H264;


    // For T3 H264
    MVOP_REG(MVOP_UV_SHT) |= BIT4;

    return;
}

void MHAL_MVOP_SetJpegHardwireMode(void)
{
    U8 regval = 0x00;

    regval |= 0x80; // packmode
    regval |= 0x20; // Dram Rd Contd
    MVOP_REG(MVOP_MPG_JPG_SWITCH) = regval;

    // Write trigger
    /*
    MVOP_REG(MVOP_REG_WR) |= BIT0;
    MVOP_REG(MVOP_REG_WR) &= ~BIT0;
    */
}

/******************************************************************************/
/// Set VOP for RM  Hardwire Mode
/******************************************************************************/
#if 0
void MHal_MVOP_SetRMHardwireMode(void)
{
    U16 u16RegValue;

    //XBYTE[VOP_MPG_JPG_SWITCH] &= VOP_HARDWIRE_MODE;
    MVOP_REG(MVOP_MPG_JPG_SWITCH) &= VOP_HARDWIRE_MODE;
    //u8RegValue = XBYTE[VOP_INPUT_SWITCH] & 0xF3;
    u16RegValue = MVOP_REG(MVOP_INPUT_SWITCH) & 0xF3FF;
    //XBYTE[VOP_INPUT_SWITCH] = u8RegValue | VOP_INPUT_RM;
    MVOP_REG(MVOP_INPUT_SWITCH) = u16RegValue | VOP_INPUT_RM;
    return;
}
#else
void MHal_MVOP_SetRMHardwireMode(void)
{
    U16 u16RegValue;

    printk("MHal_MVOP_SetRMHardwireMode\n");

    //XBYTE[VOP_MPG_JPG_SWITCH] &= VOP_HARDWIRE_MODE;
    MVOP_REG(MVOP_MPG_JPG_SWITCH) &= VOP_HARDWIRE_MODE;
    //XBYTE[VOP_TST_IMG] &= (~(BIT1|BIT0));
    MVOP_REG(MVOP_TST_IMG) &= 0xfffc;
    // Don't need to do inverse the field for H264 since H264 f/w version 1.5.8.53
    //XBYTE[VOP_CTRL0+1] |= 0x08;
    //MVOP_REG(MVOP_STRIP_ALIGN) = MVOP_REG(MVOP_STRIP_ALIGN) | 0xFE;
    //DRAM H264 frame order read out
    //XBYTE[VOP_REG_WR] = 0x02;

    printk("[0] MVOP_REG_WR = %x\n", MVOP_REG(MVOP_REG_WR));
    MVOP_REG(MVOP_REG_WR) = MVOP_REG(MVOP_REG_WR) | 0x02;
    printk("[1] MVOP_REG_WR = %x\n", MVOP_REG(MVOP_REG_WR));

    //u8RegValue = XBYTE[VOP_INPUT_SWITCH] & 0xF3;
    u16RegValue = MVOP_REG(MVOP_INPUT_SWITCH) & 0xF3FF;
    //XBYTE[VOP_INPUT_SWITCH] = u8RegValue | VOP_INPUT_H264;
    MVOP_REG(MVOP_INPUT_SWITCH) = u16RegValue | VOP_INPUT_RM;

    // For T3
    MVOP_REG(MVOP_UV_SHT) |= BIT4;
    return;
}
#endif

//------------------------------------------------------------------------------
/// Set Test pattern on MVOP
/// @param patternIdx, pattern
/// @return None
/// @internal
//------------------------------------------------------------------------------
void MHal_MVOP_SetTestPattern(U8 patternIdx, U8 pattern)	// lemonic LGE 080908
{
    U16 regval;

    regval = MVOP_REG(MVOP_TST_IMG);

	// Pattern type to [patternIdx]
	regval &= ~(BIT2|BIT1|BIT0);
	regval |= patternIdx & (BIT2|BIT1|BIT0);

	// Pattern Color Y to [pattern]
	regval &= ~(0xFF00);
	regval |= ((U16) pattern << 8);

    MVOP_REG(MVOP_TST_IMG) = regval;


	// pattern Color U/V (Cb/Cr) to default
    MVOP_REG(MVOP_U_PAT) = (0x80 << 8) | (0x80);
}

//LGE gbtogether(081217) ->to fix ChromaArtifact when 420to422 by Junyou.Lin
void MHal_MVOP_SetUVShift(B16 bEnable)
{
    if(bEnable == TRUE)
    {

        MVOP_REG(MVOP_UV_SHT) |= 0x01;
        MVOP_REG(MVOP_CTRL0) |= BIT11;//victor 20090112, Feathering in DTV, Fix MVOP setting
        MVOP_REG(MVOP_CTRL0) |= BIT12;//victor 20090112, Feathering in DTV, Fix MVOP setting
    }
    else
    {
        MVOP_REG(MVOP_UV_SHT) &= ~(0x01);

        MVOP_REG(MVOP_CTRL0) &= ~BIT11;//victor 20090112, Feathering in DTV, Fix MVOP setting
        MVOP_REG(MVOP_CTRL0) &= ~BIT12;//victor 20090112, Feathering in DTV, Fix MVOP setting
    }
}
