
// Resource Table
// Created by IDE, Do not modify this table

.CODE
.public _RES_Table;
.public _SPI_Resources_Folder_Table;
.external __RES_AINTWORRIED_12K_WAV_SE4_sa
.public _RES_AINTWORRIED_12K_WAV_SE4_SA;
.external __RES_AINTWORRIED_12K_WAV_SE4_ea;
.public _RES_AINTWORRIED_12K_WAV_SE4_EA;
.external __RES_GIRL_SKY_WAV_SE4_sa
.public _RES_GIRL_SKY_WAV_SE4_SA;
.external __RES_GIRL_SKY_WAV_SE4_ea;
.public _RES_GIRL_SKY_WAV_SE4_EA;
.external __RES_FILEMERGER_ROM_BIN_sa
.public _RES_FILEMERGER_ROM_BIN_SA;
.external __RES_FILEMERGER_ROM_BIN_ea;
.public _RES_FILEMERGER_ROM_BIN_EA;


_RES_Table:


_SPI_Resources_Folder_Table:

_RES_AINTWORRIED_12K_WAV_SE4_SA:
	.DW offset __RES_AINTWORRIED_12K_WAV_SE4_sa,seg16 __RES_AINTWORRIED_12K_WAV_SE4_sa;
_RES_AINTWORRIED_12K_WAV_SE4_EA:
	.DW offset __RES_AINTWORRIED_12K_WAV_SE4_ea,seg16 __RES_AINTWORRIED_12K_WAV_SE4_ea;

_RES_GIRL_SKY_WAV_SE4_SA:
	.DW offset __RES_GIRL_SKY_WAV_SE4_sa,seg16 __RES_GIRL_SKY_WAV_SE4_sa;
_RES_GIRL_SKY_WAV_SE4_EA:
	.DW offset __RES_GIRL_SKY_WAV_SE4_ea,seg16 __RES_GIRL_SKY_WAV_SE4_ea;

_RES_FILEMERGER_ROM_BIN_SA:
	.DW offset __RES_FILEMERGER_ROM_BIN_sa,seg16 __RES_FILEMERGER_ROM_BIN_sa;
_RES_FILEMERGER_ROM_BIN_EA:
	.DW offset __RES_FILEMERGER_ROM_BIN_ea,seg16 __RES_FILEMERGER_ROM_BIN_ea;


// End Table
//------------------------------------------------
// SACM A3400Pro se4 Table
// -- User nees to add in the song/speech 
//    for playback based on the _RES_Table above.
//------------------------------------------------
.public T_SACM_A3400Pro_se4Table;
T_SACM_A3400Pro_se4Table:
	.DW offset __RES_GIRL_SKY_WAV_SE4_sa,seg16 __RES_GIRL_SKY_WAV_SE4_sa;
	.DW offset __RES_AINTWORRIED_12K_WAV_SE4_sa,seg16 __RES_AINTWORRIED_12K_WAV_SE4_sa;
//	.DW offset __RES_CALUM_12K_WAV_SE4_sa,seg16 __RES_CALUM_12K_WAV_SE4_sa;
//	.DW offset __RES_CHEAP_12K_WAV_SE4_sa,seg16 __RES_CHEAP_12K_WAV_SE4_sa;
//	.DW offset __RES_HERCULES_12K_WAV_SE4_sa,seg16 __RES_HERCULES_12K_WAV_SE4_sa;
//	.DW offset __RES_THEWAYYOU12K_WAV_SE4_sa,seg16 __RES_THEWAYYOU12K_WAV_SE4_sa;
//	.DW offset __RES_YY_29_12K_WAV_SE4_sa,seg16 __RES_YY_29_12K_WAV_SE4_sa;
	
	
// SACM PCM spp Table
.public T_SACM_PCM_sppTable;
T_SACM_PCM_sppTable:
//	.DW offset __RES_AVRIL_48K_WAV_SPP_sa,seg16 __RES_AVRIL_48K_WAV_SPP_sa;     //0
//	.DW offset __RES_STORY_48K_WAV_SPP_sa,seg16 __RES_STORY_48K_WAV_SPP_sa;     //1
