//**************************************************************************
// Header File Included Area
//**************************************************************************
.include GPCE36_CE5.inc

//**************************************************************************
// External Table Declaration
//**************************************************************************

//**************************************************************************
// Contant Defintion Area
//**************************************************************************
.define		D_ChannelNo		8
.define 	D_ON			1
.define 	D_OFF 			0


//**************************************************************************
// Function Call Publication Area
//**************************************************************************
//.public 	_SPUPowerUpInitial
//.public 	F_SPUPowerUpInitial

//.public 	R_ChSpeechStatus


//**************************************************************************
// RAM Definition Area
//**************************************************************************
.RAM
//R_ChSpeechStatus:       .dw     D_ChannelNo 	DUP(?)     //For each SPU channel
//.define		D_SpeechPlaying 	0x0008
//.define		D_SpeechRepeat      0x0004	
//.define		D_SpeechPause   	0x0002	
//.define		D_SpeechZCJump  	0x0001 
//*****************************************************************************
// Table Definition Area
//*****************************************************************************
.TEXT
//.public 	T_ChMap
//T_ChMap:
//    .dw     0x0000
//    .dw     0x0010
//    .dw     0x0020
//    .dw     0x0030
//    .dw     0x0040
//    .dw     0x0050
//    .dw     0x0060
//    .dw     0x0070
//    
//.public 	T_ChEnableLB   
//.public 	T_ChEnableHB     
//T_ChEnableLB:                             //Enable Channel
//    .dw     0x0001
//    .dw     0x0002 
//    .dw     0x0004
//    .dw     0x0008
//    .dw     0x0010 
//    .dw     0x0020
//    .dw     0x0040
//    .dw     0x0080
//T_ChEnableHB:                             //Enable Channel
//    .dw     0x0100
//    .dw     0x0200 
//    .dw     0x0400
//    .dw     0x0800
//    .dw     0x1000 
//    .dw     0x2000
//    .dw     0x4000
//    .dw     0x8000    
//    
//.public 	T_ChDisableLB   
//.public 	T_ChDisableHB   
//T_ChDisableLB:                            //Disable Channel
//    .dw     0xfffe
//    .dw     0xfffd 
//    .dw     0xfffb
//    .dw     0xfff7
//    .dw     0xffef
//    .dw     0xffdf
//    .dw     0xffbf
//    .dw     0xff7f
//T_ChDisableHB:                            //Disable Channel
//    .dw     0xfeff
//    .dw     0xfdff 
//    .dw     0xfbff
//    .dw     0xf7ff
//    .dw     0xefff
//    .dw     0xdfff
//    .dw     0xbfff
//    .dw     0x7fff    
    
//**************************************************************************
// CODE Definition Area
//**************************************************************************
.CODE    
//****************************************************************
// Function    : F_SPUPowerUpInitial
// Description : 
// Destory     : 
// Parameter   : 
// Return      : 
// Note        : 
//****************************************************************	    
// _SPUPowerUpInitial:	.proc
//F_SPUPowerUpInitial:
//	push R1, R3 to [sp]
//	R1 = 0
//	R3 = 0
//L_ClrLoop?:	
//	R2 = R1 + R_ChSpeechStatus
//	[R2] = R3
//	R1 += 1
//	cmp R1, D_ChannelNo
//	jb L_ClrLoop?
//	pop R1, R3 from [sp]
//	retf
//	.endp
	
	
	
.public 	_SPUTesting
 _SPUTesting:	.proc
 	push R1, R5 to [SP]
 				
// 	R1 = [P_INT_Ctrl]
//	R1 |= C_IRQ5_SPU
//	[P_INT_Ctrl] = R1
//	
//	R1 = [P_SPU_INT_Ctrl]
//	R1 |= C_SPU_Ch_Sel
//	[P_SPU_INT_Ctrl] = R1		

	R1 = C_PWM_sData | C_PWM_Mute_OFF;
	[P_AUDIO_Ctrl3] = R1;
			
	R1 = C_AUDIO_PWMIP_Enable | C_AUDIO_PWM_Enable;
	R1 |= 0x03 | C_AUDIO_PWM_DS6 | C_MuteControl_DATACHAGE;
	[P_AUDIO_Ctrl1] = R1;
	
 	//R1 = C_SPU_Mixer_FullSwing + C_SPU_Mixer_MaxVolume
	R1 = 0xFFFF
	[P_SPU_Volume_Ctrl] = R1
	[P_SPU_Mixer_Ctrl] = R1
 	[P_SPU_Ch0_Volume_Ctrl] = R1	
 	
 	R1 = 0x1333						//(SR/FSpu) * 2^24) , 12K/40.96M*2^24 = 0x20c5
 	
 	[P_SPU_Ch0_SampleFreq] = R1;	
 	//R1 = 0xA009						//A34proE: Memory Map Start address * 2 (bytes mode) + 9
 	R1 = 0xCA09
 	[P_SPU_Ch0_DPTR_L] = R1
 	//R1 = 0x004C						//P_SPU_Ch0_DPTR_H[15:14]:00b = A34proE , SPU_Ch0_DPTR_H[13] : 1 = Absolute jump mode , 0 = Relative jump mode
 	R1 = 0x0004
 	[P_SPU_Ch0_DPTR_H] = R1
 	[P_SPU_Ch0_LoopLength] = R1 - R1
 	[P_SPU_Ch0_Filter_S2] = R1 - R1
 	[P_SPU_Ch0_Filter_S1] = R1 - R1

	//R1 = 0xA009
	//R1 = 0xCA09
 	//[P_SPU_Ch0_NextDPTR_L] = R1
 	//R1 = 0x004C
 	//R1 = 0x0004
 	//[P_SPU_Ch0_NextDPTR_H] = R1
 	
 //	setb [P_SPU_CH_Ctrl], 0	
 
//  	R1 = 0x0001;
// 	[P_SPU_CH_Ctrl] = R1
 	
	IRQ ON;
 	pop R1, R5 from [SP]
 	retf
 	.endp
 	
 //0x216ca1