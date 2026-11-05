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
#include "System.h"
#include "..\audio\audio_spu.h"

//**************************************************************************
// Global Variable Defintion Area
//**************************************************************************
unsigned Key = 0;
unsigned PCM_Index = 0;
unsigned A34PE_Index = 0;
unsigned UserTemp01 = 0;
unsigned UserTemp02 = 0;
unsigned PWMorCUR_Flg = 0; // 0:CUR DACOut ,1:PWM Out
unsigned VolIndex_SPU = 9;  ////0 ~ 15 


//***************************************************************************************
// Main Function Area
//***************************************************************************************
int main(void)
{		
	//add your code here	
	Key = 0;
	
	UserTemp01 = SPU_SAMPLE_FREQ_12K;
	UserTemp01 = SPU_SAMPLE_FREQ_48K;
	
	System_Initial();			          	// System initial
	PWMorCUR_Flg = 1;        //ѡ�� 1:PWM DACOut;  0:CUR DACOut
	
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
				SPU_SetResAddr(0, SPU_FORMAT_A3400_4BIT, SPU_SAMPLE_FREQ_12K, A34PE_Index);
				SPU_Play(0);  ////Channel
				break;
			
			case 0x0002:	// IOA1 + Vcc
				SPU_StopPlay(0);  ////Channel
				break;
	
			case 0x0004:	// IOA2 + Vcc
				A34PE_Index = 0;
				SPU_SetResAddr(1, SPU_FORMAT_A3400_4BIT, SPU_SAMPLE_FREQ_12K, A34PE_Index);
				SPU_Play(1);
				break;
	
			case 0x0008:	// IOA3 + Vcc
				SPU_StopPlay(1);  ////Channel
				break;
				
			case 0x0010:	// IOA4 + Vcc
				VolIndex_SPU ++;
				if(VolIndex_SPU >= 16)
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
