//**************************************************************************
// Header File Included Area
//**************************************************************************
.include GPCE36_CE5.inc


//**************************************************************************
// Contant Defintion Area
//**************************************************************************
.define C_ROM_Table						0 ////  use auto mode; index in Resource Table 
.define C_FilerMerger_Version1			1 //// PlayA18 use Manual_Mode ; index in fileMerger rom bin
.define C_SPEECH_Source_Type			C_FilerMerger_Version1     ////C_FilerMerger_Version1    //C_ROM_Table

//**************************************************************************
// User Defintion Area
//**************************************************************************
//.define 	D_C16MIDIEvent 			D_ON			// Ch16 Note on event only treats as user events
//.define 	MIDI_IOEvt             	D_ON            //;Declare IO Event in a MIDI
//
//**************************************************************************
// Function Call Publication Area
//**************************************************************************
.public  _USER_SPU_fptr_Volume
.public F_USER_SPU_fptr_Volume
.public  _SPU_StopPlay
.public F_SPU_StopPlay
//**************************************************************************
// External Function Declaration
//**************************************************************************
//.external 	F_SACM_Delay


//**************************************************************************
// External Variable Declaration
//**************************************************************************
.external _DPTR_H
.external _DPTR_L

//**************************************************************************
// External Table Declaration
//**************************************************************************
.external T_SACM_A3400Pro_se4Table;
.external T_SACM_PCM_sppTable;

.if C_SPEECH_Source_Type == C_FilerMerger_Version1
	.external _RES_FILEMERGER_ROM_BIN_SA;
.endif

//**************************************************************************
// RAM Definition Area
//**************************************************************************
.RAM

//*****************************************************************************
// Table Definition Area
//*****************************************************************************
.CODE
// Volume Table
T_SACM_SPU_fptr_Volume_Level:
.dw 0x0480, 0x0580, 0x0680, 0x0880			//2db
.dw	0x0A80, 0x0D80, 0x1080, 0x1480
.dw 0x1980, 0x2080, 0x2880, 0x3380
.dw	0x4F80, 0x5180, 0x6680, 0x7F80

// SPU_CH Table
T_SPU_channel_DisableLB:
.dw     0xfffe
.dw     0xfffd 
.dw     0xfffb
.dw     0xfff7
.dw     0xffef
.dw     0xffdf
.dw     0xffbf
.dw     0xff7f


//**************************************************************************
// CODE Definition Area
//**************************************************************************
.CODE	

//*****************************************************************************
// Function    : F_USER_SPU_fptr_Volume
// Description : Set speech volume
// Destory     : R1
// Parameter   : R1: volume index
// Return      : None
// Note        : None
//*****************************************************************************
 _USER_SPU_fptr_Volume: .proc
	R1 = SP + 3;
	R1 = [R1];								// volume index
F_USER_SPU_fptr_Volume:
	R2 = SEG T_SACM_SPU_fptr_Volume_Level
	R1 += OFFSET T_SACM_SPU_fptr_Volume_Level;		// loop up volume table
	R2 += 0, CARRY
	DS = R2
	R1 = D:[R1];
	R2 = [P_SPU_Volume_Ctrl];
	R2 = R2 & 0x8000;
	R1 |= R2;
	[P_SPU_Volume_Ctrl] = R1; 
	retf
	.endp
	
//****************************************************************
// Function    : F_SPU_StopPlay
// Description : Call function for StopPlay SPU Channel
// Destory     : None
// Parameter   : R1: Play Channel
//
// Return      : None
// Note        : None
//****************************************************************	
 _SPU_StopPlay:	.proc
	R1 = SP + 3;
	R1 = [R1];								// Play Channel
F_SPU_StopPlay:
	R2 = R1 + T_SPU_channel_DisableLB;
	R1 = [R2];
	R2 = [P_SPU_CH_Ctrl];
	R1 &= R2; 
	[P_SPU_CH_Ctrl] = R1; 

	retf;
	.endp		
		
//****************************************************************
// Function    : F_USER_SPU_SetPCM_sppAddr
// Description : This API allows users to set the beginning address
//               to fetch data. This address can be either a ROM address
//               or a external storage address. User would have to modify
//               the function body based on the application's need.
// Destory     : None
// Parameter   : R1: Speech index
// Return      : None
// Note        : None
//****************************************************************
.public _USER_SPU_SetPCM_sppAddr;
 _USER_SPU_SetPCM_sppAddr:	.proc
	R1 = SP + 3;
	R1 = [R1];

F_USER_SPU_SetPCM_sppAddr:
	R1 += R1
	R2 = SEG16 T_SACM_PCM_sppTable
	R3 = OFFSET T_SACM_PCM_sppTable
	R3 += R1
	R2 += 0, CARRY
	DS = R2
	R4 = D:[R3++]
	[_DPTR_L] = R4
	R4 = D:[R3]
	[_DPTR_H] = R4
	
	retf;
	.endp

//****************************************************************
// Function    : F_USER_SPU_SetA34PE_sppAddr
// Description : This API allows users to set the beginning address
//               to fetch data. This address can be either a ROM address
//               or a external storage address. User would have to modify
//               the function body based on the application's need.
// Destory     : None
// Parameter   : R1: Speech index
// Return      : None
// Note        : None
//****************************************************************
.public _USER_SPU_SetA34PE_sppAddr;
 _USER_SPU_SetA34PE_sppAddr:	.proc
	R1 = SP + 3;
	R1 = [R1];

F_USER_SPU_SetA34PE_sppAddr:
.if C_SPEECH_Source_Type == C_ROM_Table
	R1 += R1
	R2 = SEG16 T_SACM_A3400Pro_se4Table
	R3 = OFFSET T_SACM_A3400Pro_se4Table
	R3 += R1
	R2 += 0, CARRY
	DS = R2
	R4 = D:[R3++]
	[_DPTR_L] = R4
	R4 = D:[R3]
	[_DPTR_H] = R4
.endif		
	
.if C_SPEECH_Source_Type == C_FilerMerger_Version1    		
	DS = SEG _RES_FILEMERGER_ROM_BIN_SA;
	R4 = OFFSET _RES_FILEMERGER_ROM_BIN_SA;
	R3 = D:[R4++];
	R4 = D:[R4];
	R2 = R4;
    R1 = R1 lsl 2;
	R1 += 0x4;
	R1 = R1 lsr 1;
	
	R1 += R3;
	R2 += 0,Carry;
	DS = R2;

	R2 = R1;
	R1 = D:[R2++];
				
	R2 = D:[R2];
	R2 = R2 LSR 1;
	R4 += R2;	

    R2 = R2 LSR 3;
    R1 = R1 ROR 1;
	R3 += R1;
	R4 +=0,Carry
	[_DPTR_L] = R3
	[_DPTR_H] = R4
.endif		
	
	
	
	retf;
	.endp	
	