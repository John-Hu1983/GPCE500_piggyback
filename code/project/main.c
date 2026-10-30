//==========================================================================
// File Name   : main.c
// Description : 
// Programmer : Jerry Hsu
// Last modified date: 2023/12/13
// Version: 
// Note: 
//==========================================================================
//**************************************************************************
// Header File Included Area
//**************************************************************************
#include "GPCE36_CE5.h"
#include "System.h"
#include "Resource.h"

//**************************************************************************
// Contant Defintion Area
//**************************************************************************
#define A3400_4bit		0
#define A3400_5bit		1
#define PCM_8bit		2
#define PCM_16bit		3

#define C_SPU_SampleFreq_12K   	 	0x1333  ////(12000 / 40960000) * 16777216   ////// 12k/40960k  * 16777216
#define C_SPU_SampleFreq_48K   	 	0x4CCC  ////(48000 / 40960000) * 16777216 

//**************************************************************************
// Global Variable Defintion Area
//**************************************************************************
unsigned Key = 0;
unsigned DPTR_H = 0;
unsigned DPTR_L = 0;
unsigned PCM_Index = 0;
unsigned A34PE_Index = 0;
unsigned UserTemp01 = 0;
unsigned UserTemp02 = 0;
unsigned PWMorCUR_Flg = 0; // 0:CUR DACOut ,1:PWM Out
unsigned VolIndex_SPU = 9;  ////0 ~ 15 


//***************************************************************************************
// Main Function Area
//***************************************************************************************
int main()
{		
	//add your code here	
	Key = 0;
	
	UserTemp01 = C_SPU_SampleFreq_12K;
	UserTemp01 = C_SPU_SampleFreq_48K;
	
	System_Initial();			          	// System initial
	PWMorCUR_Flg = 0;        //Ñ¡Ôñ 1:PWM DACOut;  0:CUR DACOut
	
	USER_Set_Audio_OUT();    //Set audio output is  CUR OUT 
	
	SPU_Init();
	USER_SPU_fptr_Volume(VolIndex_SPU);
	
	while(1)
	{
		Key = SP_GetCh();
		switch(Key)
		{	
			case 0x0001:	// IOA0 + Vcc
				A34PE_Index ++;
				if(A34PE_Index >= 5)         //// index in ROM Table 0 ~ 1;  //in fileMerger rom bin  0 ~ 4 is A34PE_Index 
					A34PE_Index = 0;
				SPU_SetResAddr(0, A3400_4bit, C_SPU_SampleFreq_12K, A34PE_Index);
				SPU_Play(0);  ////Channel
				break;
			
			case 0x0002:	// IOA1 + Vcc
				SPU_StopPlay(0);  ////Channel
				break;
	
			case 0x0004:	// IOA2 + Vcc
				A34PE_Index = 0;
				SPU_SetResAddr(1, A3400_4bit, C_SPU_SampleFreq_12K, A34PE_Index);
				SPU_Play(1);
				break;
	
			case 0x0008:	// IOA3 + Vcc
				SPU_StopPlay(1);  ////Channel
				break;
				
			case 0x0010:	// IOA4 + Vcc
				VolIndex_SPU ++;
				if((VolIndex_SPU < 0) || (VolIndex_SPU >= 16))
				{
					VolIndex_SPU = 0;
				}
				USER_SPU_fptr_Volume(VolIndex_SPU);
				
				break;
				
			case 0x0020:	// IOA5 + Vcc
				
				break;
				
			case 0x0040:	// IOA6 + Vcc
				
				break;
				
			case 0x0080:	// IOA7 + Vcc
				
				break;
				
			default:
				break;
		} // end of switch
		
		System_ServiceLoop();
	}
	
	return 0;
}

void SPU_Init(void)
{	
 	*P_SPU_CLK_Ctrl = C_SPUCLK_DIV_2;
	*P_SPU_Volume_Ctrl = C_SPU_Mixer_MaxVolume | C_SPU_Mixer_FullSwing;
	*P_SPU_Mixer_Ctrl = C_SPU_Mixer_Ch0_En | C_SPU_Mixer_Ch1_En; 	
	
	*P_SPU_INT_Ctrl = C_SPU_Ch0_INT_En | C_SPU_Ch1_INT_En;
	*P_INT_Ctrl = C_IRQ5_SPU;
	__asm("IRQ ON");
}


void SPU_SetResAddr(unsigned Channel, unsigned Format,unsigned SampleFreq, unsigned PlayIndex)
{	
	unsigned long Addr;
	
	
	if(Format == A3400_4bit)
	{
		USER_SPU_SetA34PE_sppAddr(PlayIndex);	
		Addr = DPTR_H; 
		Addr = Addr <<16;
		Addr |= DPTR_L;
		Addr = 2*Addr + 9;
		DPTR_L = (unsigned)(Addr & 0x0000FFFF);
		DPTR_H = (unsigned)((Addr >>16) | 0x0000);
	}
	else if(Format == A3400_5bit)
	{
		USER_SPU_SetA34PE_sppAddr(PlayIndex);	
		Addr = DPTR_H; 
		Addr = Addr <<16;
		Addr |= DPTR_L;
		Addr = 2*Addr + 9;
		DPTR_L = (unsigned)(Addr & 0x0000FFFF);
		DPTR_H = (unsigned)((Addr >>16) | 0x4000);
	}
	else if(Format == PCM_8bit)
	{
		USER_SPU_SetPCM_sppAddr(PlayIndex);	
		Addr = DPTR_H; 
		Addr = Addr <<16;
		Addr |= DPTR_L;
		Addr = 2*Addr - 1;
		DPTR_L = (unsigned)(Addr & 0x0000FFFF);
		DPTR_H = (unsigned)((Addr >>16) | 0x8000);
	}
	else if(Format == PCM_16bit)
	{
		USER_SPU_SetPCM_sppAddr(PlayIndex);	
		Addr = DPTR_H; 
		Addr = Addr <<16;
		Addr |= DPTR_L;
		Addr = 2*Addr - 1;
		DPTR_L = (unsigned)(Addr & 0x0000FFFF);
		DPTR_H = (unsigned)((Addr >>16) | 0xC000);
	}
	
 	*(P_SPU_Ch0_Volume_Ctrl + 0x10 * Channel) = 0xFFFF;
 	*(P_SPU_Ch0_SampleFreq + 0x10 * Channel) = SampleFreq; // 0x1333;	//(SR/FSpu) * 2^24) , 12K/40.96M*2^24 = 0x1333  //// 12k/40960k  * 16777216 
 	*(P_SPU_Ch0_DPTR_L + 0x10 * Channel) = DPTR_L;
 	*(P_SPU_Ch0_DPTR_H + 0x10 * Channel) = DPTR_H;
 	*(P_SPU_Ch0_LoopLength + 0x10 * Channel) = 0x00;
 	*(P_SPU_Ch0_NextDPTR_L + 0x10 * Channel) = 0x00;
 	*(P_SPU_Ch0_NextDPTR_H + 0x10 * Channel) = 0x00;
 	*(P_SPU_Ch0_Filter_S2 + 0x10 * Channel) = 0x00;
 	*(P_SPU_Ch0_Filter_S1 + 0x10 * Channel) = 0x00;
}

void SPU_Play(unsigned Channel)
{
	*P_SPU_CH_Ctrl |= C_SPU_Ch0_En << Channel;
}
