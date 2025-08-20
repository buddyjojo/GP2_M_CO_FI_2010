#ifndef _MDRV_TSP_IO_H_
#define _MDRV_TSP_IO_H_


/* Use 't' as magic number */
#define TSP_IOC_MAGIC           't'

//------------------------------------------------------------------------------
// Signal
//------------------------------------------------------------------------------
#define TSP_IOC_SIGNAL                  _IOR (TSP_IOC_MAGIC, 0x00, DrvTSP_Signal_t)

//------------------------------------------------------------------------------
// PidFlt
//------------------------------------------------------------------------------
// DRVTSP_RESULT MDrv_TSP_PidFlt_Alloc(MS_U32 u32EngId, DrvTSP_FltType eFilterType, MS_U32 *pu32PidFltId);
#define TSP_IOC_PIDFLT_ALLOC            _IOWR(TSP_IOC_MAGIC, 0x10, DrvTSP_PidFlt_Alloc_t)
// DRVTSP_RESULT MDrv_TSP_PidFlt_Free(MS_U32 u32EngId, MS_U32 u32PidFltId);
#define TSP_IOC_PIDFLT_FREE             _IOW (TSP_IOC_MAGIC, 0x11, DrvTSP_PidFlt_Free_t)
// DRVTSP_RESULT MDrv_TSP_PidFlt_SetPid(MS_U32 u32EngId, MS_U32 u32PidFltId, MS_U32 u32PID);
#define TSP_IOC_PIDFLT_SET_PID          _IOW (TSP_IOC_MAGIC, 0x12, DrvTSP_PidFlt_Pid_t)
// DRVTSP_RESULT MDrv_TSP_PidFlt_SelSecFlt(MS_U32 u32EngId, MS_U32 u32PidFltId, MS_U32 u32SecFltId);
#define TSP_IOC_PIDFLT_SEL_SECFLT       _IOW (TSP_IOC_MAGIC, 0x13, DrvTSP_PidFlt_SelSecFlt_t)
// DRVTSP_RESULT MDrv_TSP_PidFlt_Enable(MS_U32 u32EngId, MS_U32 u32PidFltId, MS_BOOL bEnable);
#define TSP_IOC_PIDFLT_ENABLE           _IOW (TSP_IOC_MAGIC, 0x14, DrvTSP_PidFlt_Enable_t)
// DRVTSP_RESULT MDrv_TSP_PidFlt_GetState(MS_U32 u32EngId, MS_U32 u32PidFltId, DrvTSP_FltState *peState);
#define TSP_IOC_PIDFLT_STATE            _IOWR(TSP_IOC_MAGIC, 0x15, DrvTSP_PidFlt_State_t)
// DRVTSP_RESULT MDrv_TSP_PidFlt_ScmbStatus(MS_U32 u32EngId, MS_U32 u32PidFltId, DrvTSP_Scmb_Level* pScmbLevel);
#define TSP_IOC_PIDFLT_SCMB_STATUS       _IOWR(TSP_IOC_MAGIC, 0x16, DrvTSP_PidFlt_Scmb_Status_t)
// DRVTSP_RESULT MDrv_TSP_PidFlt_SetSource(U32 u32EngId, U32 u32PidFltId, U32 u32Src)
#define TSP_IOC_PIDFLT_SET_SRC          _IOW(TSP_IOC_MAGIC, 0x17, DrvTSP_PidFlt_Set_Src_t)
// DRVTSP_RESULT MDrv_TSP_PidFlt_GetPid(MS_U32 u32EngId, MS_U32 u32PidFltId, MS_U32 *u32PID);
#define TSP_IOC_PIDFLT_GET_PID          _IOWR(TSP_IOC_MAGIC, 0x18, DrvTSP_PidFlt_Pid_t)
//
#define TSP_IOC_PIDFLT_GET_SRC          _IOWR(TSP_IOC_MAGIC, 0x19, DrvTSP_PidFlt_Set_Src_t)

//------------------------------------------------------------------------------
// SecFlt
//------------------------------------------------------------------------------
// DRVTSP_RESULT MDrv_TSP_SecFlt_Alloc(MS_U32 u32EngId, MS_U32 *pu32SecFltId);
#define TSP_IOC_SECFLT_ALLOC            _IOWR(TSP_IOC_MAGIC, 0x20, DrvTSP_SecFlt_Alloc_t)
// DRVTSP_RESULT MDrv_TSP_SecFlt_Free(MS_U32 u32EngId, MS_U32 u32SecFltId);
#define TSP_IOC_SECFLT_FREE             _IOW (TSP_IOC_MAGIC, 0x21, DrvTSP_SecFlt_Free_t)
// DRVTSP_RESULT MDrv_TSP_SecFlt_SetMode(MS_U32 u32EngId, MS_U32 u32SecFltId, DrvTSP_FltMode eSecFltMode);
#define TSP_IOC_SECFLT_MODE             _IOW (TSP_IOC_MAGIC, 0x22, DrvTSP_SecFlt_Mode_t)
// DRVTSP_RESULT MDrv_TSP_SecFlt_SetPattern(MS_U32 u32EngId, MS_U32 u32SecFltId, MS_U8 *pu8Match, MS_U8 *pu8Mask, MS_BOOL bNotMatch);
#define TSP_IOC_SECFLT_PATTERN          _IOW (TSP_IOC_MAGIC, 0x23, DrvTSP_SecFlt_Pattern_t)
// DRVTSP_RESULT MDrv_TSP_SecFlt_SetReqCount(MS_U32 u32EngId, MS_U32 u32SecFltId, MS_U32 u32ReqCount);
#define TSP_IOC_SECFLT_REQCOUNT         _IOW (TSP_IOC_MAGIC, 0x24, DrvTSP_SecFlt_ReqCount_t)
// DRVTSP_RESULT MDrv_TSP_SecFlt_ResetBuffer(MS_U32 u32EngId, MS_U32 u32SecFltId);
#define TSP_IOC_SECFLT_BUFFER_RESET     _IOW (TSP_IOC_MAGIC, 0x25, DrvTSP_SecFlt_Buffer_Reset)
// DRVTSP_RESULT MDrv_TSP_SecFlt_SetBuffer(MS_U32 u32EngId, MS_U32 u32SecFltId, MS_U32 u32StartAddr, MS_U32 u32BufSize);
#define TSP_IOC_SECFLT_BUFFER_SET       _IOW (TSP_IOC_MAGIC, 0x26, DrvTSP_SecFlt_Buffer_Set_t)
// DRVTSP_RESULT MDrv_TSP_SecFlt_GetBufStart(MS_U32 u32EngId, MS_U32 u32SecFltId, MS_U32 *pu32BufStart);
#define TSP_IOC_SECFLT_BUFFER_ADRR      _IOWR(TSP_IOC_MAGIC, 0x27, DrvTSP_SecFlt_Buffer_Addr_Get_t)
// DRVTSP_RESULT MDrv_TSP_SecFlt_GetBufSize(MS_U32 u32EngId, MS_U32 u32SecFltId, MS_U32 *pu32BufSize);
#define TSP_IOC_SECFLT_BUFFER_SIZE      _IOWR(TSP_IOC_MAGIC, 0x28, DrvTSP_SecFlt_Buffer_Size_Get_t)
// DRVTSP_RESULT MDrv_TSP_SecFlt_GetReadAddr(MS_U32 u32EngId, MS_U32 u32SecFltId, MS_U32 *pu32ReadAddr);
#define TSP_IOC_SECFLT_BUFFER_READ_GET  _IOWR(TSP_IOC_MAGIC, 0x29, DrvTSP_SecFlt_Buffer_Read_Get_t)
// DRVTSP_RESULT MDrv_TSP_SecFlt_SetReadAddr(MS_U32 u32EngId, MS_U32 u32SecFltId, MS_U32 u32ReadAddr);
#define TSP_IOC_SECFLT_BUFFER_READ_SET  _IOW (TSP_IOC_MAGIC, 0x2a, DrvTSP_SecFlt_Buffer_Read_Set_t)
// DRVTSP_RESULT MDrv_TSP_SecFlt_GetWriteAddr(MS_U32 u32EngId, MS_U32 u32SecFltId, MS_U32 *pu32WriteAddr);
#define TSP_IOC_SECFLT_BUFFER_WRITE_GET _IOWR(TSP_IOC_MAGIC, 0x2b, DrvTSP_SecFlt_Buffer_Write_Get_t)
// DRVTSP_RESULT MDrv_TSP_SecFlt_GetCRC32(MS_U32 u32EngId, MS_U32 u32SecFltId, MS_U32 *pu32CRC32);
#define TSP_IOC_SECFLT_CRC32            _IOWR(TSP_IOC_MAGIC, 0x2c, DrvTSP_SecFlt_Crc32_t)
// DRVTSP_RESULT MDrv_TSP_SecFlt_Notify(MS_U32 u32EngId, MS_U32 u32SecFltId, DrvTSP_Event eEvents, P_DrvTSP_EvtCallback pfCallback);
#define TSP_IOC_SECFLT_NOTIFY           _IOWR(TSP_IOC_MAGIC, 0x2d, DrvTSP_SecFlt_Notify_t)
// DRVTSP_RESULT MDrv_TSP_SecFlt_GetState(MS_U32 u32EngId, MS_U32 u32SecFltId, DrvTSP_FltState *peState)
#define TSP_IOC_SECFLT_STATE_GET        _IOWR(TSP_IOC_MAGIC, 0x2e, DrvTSP_SecFlt_State_t)
//
#define TSP_IOC_SECFLT_CTRL_GET         _IOWR(TSP_IOC_MAGIC, 0x2f, DrvTSP_SecFlt_Crc32_t)

//------------------------------------------------------------------------------
// PVR
//------------------------------------------------------------------------------
// DRVTSP_RESULT MDrv_TSP_PVR_SetBuffer(MS_U32 u32BufStart0, MS_U32 u32BufStart1, MS_U32 u32BufSize);
#define TSP_IOC_PVR_BUFFER_SET          _IOW (TSP_IOC_MAGIC, 0x30, DrvTSP_Pvr_Buffer_Set_t)
// DRVTSP_RESULT MDrv_TSP_PVR_Start(DrvTSP_RecMode eRecMode, MS_BOOL bStart);
#define TSP_IOC_PVR_START               _IOW (TSP_IOC_MAGIC, 0x31, DrvTSP_Pvr_Start_t)
// DRVTSP_RESULT MDrv_TSP_PVR_GetWriteAddr(MS_U32 *pu32WriteAddr);
#define TSP_IOC_PVR_WRITE_GET           _IOWR(TSP_IOC_MAGIC, 0x32, unsigned int)
// DRVTSP_RESULT MDrv_TSP_PVR_Notify(DrvTSP_Event eEvents, P_DrvTSP_EvtCallback pfCallback);
#define TSP_IOC_PVR_NOTIFY              _IOWR(TSP_IOC_MAGIC, 0x33, DrvTSP_Event)
// DRVTSP_RESULT MDrv_TSP_MLink_Start(MS_U32 bStart);
#define TSP_IOC_MLINK_START             _IOWR(TSP_IOC_MAGIC, 0x34, unsigned int)

#define TSP_IOC_PVR_SET_REC_STAMP       _IOW (TSP_IOC_MAGIC, 0x35, unsigned int)
#define TSP_IOC_PVR_GET_REC_STAMP       _IOWR(TSP_IOC_MAGIC, 0x36, unsigned int)
#define TSP_IOC_PVR_SET_PLAY_STAMP      _IOW (TSP_IOC_MAGIC, 0x37, unsigned int)
#define TSP_IOC_PVR_GET_PLAY_STAMP      _IOWR(TSP_IOC_MAGIC, 0x38, unsigned int)
#define TSP_IOC_PVR_STAMP_ENABLE        _IOW (TSP_IOC_MAGIC, 0x39, unsigned int)
#define TSP_IOC_PVR_GET_FILEIN_STAMP    _IOWR(TSP_IOC_MAGIC, 0x3A, unsigned int)
#define TSP_IOC_PVR_RECORD_STAMP_ENABLE _IOW (TSP_IOC_MAGIC, 0x3B, unsigned int)

//------------------------------------------------------------------------------
// File in
//------------------------------------------------------------------------------
// DRVTSP_RESULT MDrv_TSP_M2T_SetAddr(MS_U32 u32StreamAddr);
#define TSP_IOC_M2T_BUFFER_ADDR_SET     _IOW (TSP_IOC_MAGIC, 0x40, unsigned int)
// DRVTSP_RESULT MDrv_TSP_M2T_SetSize(MS_U32 u32StreamSize);
#define TSP_IOC_M2T_BUFFER_SIZE_SET     _IOW (TSP_IOC_MAGIC, 0x41, unsigned int)
// DRVTSP_RESULT MDrv_TSP_M2T_SetSTC(MS_U32 u32EngId, MS_U32 u32STC_32, MS_U32 u32STC);
#define TSP_IOC_M2T_STC_SET             _IOW (TSP_IOC_MAGIC, 0x42, DrvTSP_M2T_Stc_Set_t)
// DRVTSP_RESULT MDrv_TSP_M2T_Start(DrvTSP_M2tMode eM2tMode);
#define TSP_IOC_M2T_START               _IOW (TSP_IOC_MAGIC, 0x43, DrvTSP_M2tMode)
// DRVTSP_RESULT MDrv_TSP_M2T_Pause(void);
#define TSP_IOC_M2T_PAUSE               _IO  (TSP_IOC_MAGIC, 0x44)
// DRVTSP_RESULT MDrv_TSP_M2T_Resume(void);
#define TSP_IOC_M2T_RESUME              _IO  (TSP_IOC_MAGIC, 0x45)
// DRVTSP_RESULT MDrv_TSP_GetM2tState(DrvTSP_M2tState *peM2tState);
#define TSP_IOC_M2T_STATE               _IOWR(TSP_IOC_MAGIC, 0x46, DrvTSP_M2tState)
// DRVTSP_RESULT MDrv_TSP_SetM2tRate(MS_U32 u32Div2);
#define TSP_IOC_M2T_RATE_SET            _IOW (TSP_IOC_MAGIC, 0x47, unsigned int)
// DRVTSP_RESULT MDrv_TSP_GetM2tSlot(MS_U32 *pu32EmptySlot);
#define TSP_IOC_M2T_SLOT_GET            _IOWR(TSP_IOC_MAGIC, 0x48, unsigned int)
// DRVTSP_RESULT MDrv_TSP_M2T_GetAddr(MS_U32* u32StreamAddr);
#define TSP_IOC_M2T_BUFFER_RP_GET       _IOWR(TSP_IOC_MAGIC, 0x49, unsigned int)
// DRVTSP_RESULT MDrv_TSP_GetM2tWriteLevel(U32 *pu32Level)
#define TSP_IOC_M2T_WRLEVEL_GET         _IOWR(TSP_IOC_MAGIC, 0x4A, unsigned int)
// DRVTSP_RESULT MDrv_TSP_M2T_Reset(void);
#define TSP_IOC_M2T_RESET               _IO  (TSP_IOC_MAGIC, 0x4B)

//------------------------------------------------------------------------------
// Misc
//------------------------------------------------------------------------------
// DRVTSP_RESULT MDrv_TSP_GetSTC(MS_U32 u32EngId, MS_U32 *pu32STC_32, MS_U32 *pu32STC);
#define TSP_IOC_GET_STC                 _IOWR(TSP_IOC_MAGIC, 0x60, DrvTSP_Stc_Get_t)
// DRVTSP_RESULT MDrv_TSP_SetMode(MS_U32 u32EngId, DrvTSP_CtrlMode eCtrlMode);
#define TSP_IOC_SET_MODE                _IOW (TSP_IOC_MAGIC, 0x61, DrvTSP_Set_Mode_t)
// DRVTSP_RESULT MDrv_TSP_SelPad(MS_U32 u32EngId, DrvTSP_PadIn ePad);
#define TSP_IOC_SEL_PAD                 _IOW (TSP_IOC_MAGIC, 0x62, DrvTSP_Sel_Pad_t)
// DRVTSP_RESULT MDrv_TSP_PowerOff(void);
#define TSP_IOC_POWER                   _IOW (TSP_IOC_MAGIC, 0x63, unsigned int)
// DRVTSP_RESULT MDrv_TSP_GetLastErr(void);
#define TSP_IOC_GET_LAST_ERR            _IOWR(TSP_IOC_MAGIC, 0x64, unsigned int)
// DRVTSP_RESULT MDrv_TSP_Init(void);
#define TSP_IOC_INIT                    _IO  (TSP_IOC_MAGIC, 0x65)
// DRVTSP_RESULT MDrv_TSP_Scmb_Status(MS_U32 u32EngId, DrvTSP_Scmb_Level* pScmbLevel);
#define TSP_IOC_SCMB_STATUS             _IOWR(TSP_IOC_MAGIC, 0x66, DrvTSP_Scmb_Status_t)
// DRVTSP_RESULT MDrv_TSP_SetTSIFType(U32 u32EngId, U32 u32TSIFType)
#define TSP_IOC_SET_TSIF_TYPE           _IOW (TSP_IOC_MAGIC, 0x67, DrvTSP_Set_TSIF_Type_t)

#define TSP_IOC_CFG                     _IOWR(TSP_IOC_MAGIC, 0x68, DrvTSP_Cfg_t)

#define TSP_IOC_SET_SCRM_PATH           _IOW (TSP_IOC_MAGIC, 0x69, DrvTSP_Set_Scmb_Path_t)
#define TSP_IOC_SET_ESA_MODE            _IOW (TSP_IOC_MAGIC, 0x70, DrvTSP_CSA_Protocol)
#define TSP_IOC_SET_CIPHERKEY           _IOW (TSP_IOC_MAGIC, 0x71, DrvTSP_Set_CipherKey_t)
#define TSP_IOC_SET_PACKET_MODE         _IOW (TSP_IOC_MAGIC, 0x72, unsigned int)

#define TSP_IOC_FIFO_FLUSH              _IOW (TSP_IOC_MAGIC, 0x73, DrvTSP_Fifo_Flush_t)
#define TSP_IOC_FIFO_FETCH              _IOWR(TSP_IOC_MAGIC, 0x74, DrvTSP_Fifo_Fetch_t)

#define TSP_IOC_GET_PCR                 _IOWR(TSP_IOC_MAGIC, 0x75, DrvTSP_Stc_Get_t)

#if 0
//------------------------------------------------------------------------------
// Dmx Flt
//------------------------------------------------------------------------------
// DRVTSP_RESULT MDrv_DMX_Flt_Alloc(DrvTSP_FltType eFilterType, MS_U32 *pu32PidFltId);
#define TSP_IOC_DMXFLT_ALLOC            _IOWR(TSP_IOC_MAGIC, 0x70, DrvTSP_DmxFlt_Alloc_t)
// DRVTSP_RESULT MDrv_DMX_Flt_Free(MS_U32 u32PidFltId);
#define TSP_IOC_DMXFLT_FREE             _IOW (TSP_IOC_MAGIC, 0x71, unsigned int)
// DRVTSP_RESULT MDrv_DMX_Flt_SetPid(MS_U32 u32PidFltId, MS_U32 u32Pid);
#define TSP_IOC_DMXFLT_SET              _IOW (TSP_IOC_MAGIC, 0x72, DrvTSP_DmxFlt_Set_t)
// DRVTSP_RESULT MDrv_DMX_Flt_SelSecBuf(MS_U32 u32PidFltId, MS_U32 u32SecBufId);
#define TSP_IOC_DMXFLT_SET_SECBUF       _IOW (TSP_IOC_MAGIC, 0x73, DrvTSP_DmxFlt_SetSecBuf_t)
// DRVTSP_RESULT MDrv_DMX_Flt_GetSecBuf(MS_U32 u32PidFltId, MS_U32* pu32BufId);
#define TSP_IOC_DMXFLT_GET_SECBUF       _IOWR(TSP_IOC_MAGIC, 0x74, DrvTSP_DmxFlt_GetSecBuf_t)
// DRVTSP_RESULT MDrv_DMX_Flt_Enable(MS_U32 u32PidFltId, MS_BOOL bEnable);
#define TSP_IOC_DMXFLT_ENABLE           _IOW (TSP_IOC_MAGIC, 0x75, DrvTSP_DmxFlt_Enable)
// DRVTSP_RESULT MDrv_DMX_Flt_SetMode(MS_U32 u32PidFltId, DrvTSP_FltMode eSecFltMode);
#define TSP_IOC_DMXFLT_SETMODE          _IOW (TSP_IOC_MAGIC, 0x76, DrvTSP_DmxFlt_SetMode)
// DRVTSP_RESULT MDrv_DMX_Flt_SetPattern(MS_U32 u32PidFltId, MS_U8 *pu8Match, MS_U8 *pu8Mask, MS_BOOL bNotMatch);
#define TSP_IOC_DMXFLT_SETPATTERN       _IOW (TSP_IOC_MAGIC, 0x77, DrvTSP_DmxFlt_SetPattern)
// DRVTSP_RESULT MDrv_DMX_Flt_GetState(MS_U32 u32PidFltId, DrvTSP_FltState *peState);
#define TSP_IOC_DMXFLT_GETSTATE         _IOWR(TSP_IOC_MAGIC, 0x78, DrvTSP_DmxFlt_GetState)
// DRVTSP_RESULT MDrv_DMX_Flt_Notify(MS_U32 u32PidFltId, DrvTSP_Event eEvents, P_DrvTSP_EvtCallback pfCallback);
#define TSP_IOC_DMXFLT_NOTIFY           _IOW (TSP_IOC_MAGIC, 0x79, DrvTSP_DmxFlt_Notify)
// DRVTSP_RESULT MDrv_DMX_Flt_SetReqCount(MS_U32 u32PidFltId, MS_U32 u32ReqCount);
#define TSP_IOC_DMXFLT_REQCOUNT         _IOW (TSP_IOC_MAGIC, 0x7a, DrvTSP_DmxFlt_ReqCount)
// DRVTSP_RESULT MDrv_DMX_Flt_GetCRC32(MS_U32 u32SecBufId, MS_U32 *pu32CRC32);
#define TSP_IOC_DMXFLT_CRC32            _IOWR(TSP_IOC_MAGIC, 0x7b, DrvTSP_DmxFlt_CRC32)
// DRVTSP_RESULT MDrv_DMX_Flt_ResetState(MS_U32 u32FltId);
#define TSP_IOC_DMXFLT_RESET_STATE      _IOW (TSP_IOC_MAGIC, 0x7c, unsigned int)

//------------------------------------------------------------------------------
// Dmx Sec Buf
//------------------------------------------------------------------------------
// DRVTSP_RESULT MDrv_DMX_SecBuf_Alloc(MS_U32 *pu32SecFltId);
#define TSP_IOC_DMXSECBUF_ALLOC         _IOWR(TSP_IOC_MAGIC, 0x80, unsigned int)
// DRVTSP_RESULT MDrv_DMX_SecBuf_Free(MS_U32 u32SecFltId);
#define TSP_IOC_DMXSECBUF_FREE          _IOW (TSP_IOC_MAGIC, 0x81, unsigned int)
// DRVTSP_RESULT MDrv_DMX_SecBuf_SetBuffer(MS_U32 u32SecBufId, MS_U32 u32StartAddr, MS_U32 u32BufSize);
#define TSP_IOC_DMXSECBUF_BUF_SET       _IOW (TSP_IOC_MAGIC, 0x82, DrvTSP_DmxSecBuf_BufSet)
// DRVTSP_RESULT MDrv_DMX_SecBuf_ResetBuffer(MS_U32 u32SecFltId);
#define TSP_IOC_DMXSECBUF_BUF_RESET     _IOW (TSP_IOC_MAGIC, 0x83, unsigned int)
// DRVTSP_RESULT MDrv_DMX_SecBuf_GetBufStart(MS_U32 u32SecBufId, MS_U32 *pu32BufStart);
#define TSP_IOC_DMXSECBUF_BUF_BUF_ADDR  _IOW (TSP_IOC_MAGIC, 0x84, DrvTSP_DmxSecBuf_BufAddr)
// DRVTSP_RESULT MDrv_DMX_SecBuf_GetBufSize(MS_U32 u32SecBufId, MS_U32 *pu32BufSize);
#define TSP_IOC_DMXSECBUF_BUF_BUF_SIZE  _IOW (TSP_IOC_MAGIC, 0x85, DrvTSP_DmxSecBuf_BufSize)
// DRVTSP_RESULT MDrv_DMX_SecBuf_GetReadAddr(MS_U32 u32SecBufId, MS_U32 *pu32ReadAddr);
#define TSP_IOC_DMXSECBUF_BUF_READ_GET  _IOWR(TSP_IOC_MAGIC, 0x86, DrvTSP_DmxSecBuf_BufReadGet)
// DRVTSP_RESULT MDrv_DMX_SecBuf_SetReadAddr(MS_U32 u32SecBufId, MS_U32 u32ReadAddr);
#define TSP_IOC_DMXSECBUF_BUF_READ_SET  _IOW (TSP_IOC_MAGIC, 0x87, DrvTSP_DmxSecBuf_BufReadSet)
// DRVTSP_RESULT MDrv_DMX_SecBuf_GetWriteAddr(MS_U32 u32SecBufId, MS_U32 *pu32WriteAddr);
#define TSP_IOC_DMXSECBUF_BUF_WRITE_GET _IOWR(TSP_IOC_MAGIC, 0x88, DrvTSP_DmxSecBuf_BufWriteGet)
#endif

#endif
