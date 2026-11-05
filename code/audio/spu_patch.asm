.external T_SACM_PCM_sppTable
.external T_SACM_A3400Pro_se4Table

.public _Audio_SPU_GetPCMTableAddress
.public _Audio_SPU_GetA34PETableAddress

.CODE
// Return the word address as unsigned long: R1 = low word, R2 = high word.
_Audio_SPU_GetPCMTableAddress: .proc
    R1 = OFFSET T_SACM_PCM_sppTable
    R2 = SEG16 T_SACM_PCM_sppTable
    retf
    .endp

_Audio_SPU_GetA34PETableAddress: .proc
    R1 = OFFSET T_SACM_A3400Pro_se4Table
    R2 = SEG16 T_SACM_A3400Pro_se4Table
    retf
    .endp
