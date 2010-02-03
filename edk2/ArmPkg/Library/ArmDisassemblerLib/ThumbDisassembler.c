/** @file
  Thumb Dissassembler. Still a work in progress.

  Wrong output is a bug, so please fix it. 
  Hex output means there is not yet an entry or a decode bug.
  gOpThumb[] are Thumb 16-bit, and gOpThumb2[] work on the 32-bit 
  16-bit stream of Thumb2 instruction. Then there are big case 
  statements to print everything out. If you are adding instructions
  try to reuse existing case entries if possible.

  Copyright (c) 2008-2010, Apple Inc. All rights reserved.
  
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Base.h>
#include <Library/BaseLib.h>
#include <Library/PrintLib.h>

extern CHAR8 *gCondition[];

extern CHAR8 *gReg[];

// Thumb address modes
#define LOAD_STORE_FORMAT1            1
#define LOAD_STORE_FORMAT2            2
#define LOAD_STORE_FORMAT3            3
#define LOAD_STORE_FORMAT4            4
#define LOAD_STORE_MULTIPLE_FORMAT1   5 
#define PUSH_FORMAT                   6 
#define POP_FORMAT                  106 
#define IMMED_8                       7
#define CONDITIONAL_BRANCH            8
#define UNCONDITIONAL_BRANCH          9
#define UNCONDITIONAL_BRANCH_SHORT  109
#define BRANCH_EXCHANGE              10
#define DATA_FORMAT1                 11
#define DATA_FORMAT2                 12
#define DATA_FORMAT3                 13
#define DATA_FORMAT4                 14
#define DATA_FORMAT5                 15
#define DATA_FORMAT6_SP              16
#define DATA_FORMAT6_PC             116
#define DATA_FORMAT7                 17
#define DATA_FORMAT8                 19
#define CPS_FORMAT                   20
#define ENDIAN_FORMAT                21
#define DATA_CBZ                     22
#define ADR_FORMAT                   23

// Thumb2 address modes
#define B_T3                        200
#define B_T4                        201
#define BL_T2                       202
#define POP_T2                      203
#define POP_T3                      204
#define STM_FORMAT                  205
#define LDM_REG_IMM12_SIGNED        206
#define LDM_REG_IMM12_LSL           207
#define LDM_REG_IMM8                208
#define LDM_REG_IMM12               209
#define LDM_REG_INDIRECT_LSL        210
#define LDM_REG_IMM8_SIGNED         211
#define LDRD_REG_IMM8               212
#define LDREXB                      213
#define LDREXD                      214
#define SRS_FORMAT                  215
#define RFE_FORMAT                  216
#define LDRD_REG_IMM8_SIGNED        217




typedef struct {
  CHAR8   *Start;
  UINT32  OpCode;
  UINT32  Mask;
  UINT32  AddressMode;
} THUMB_INSTRUCTIONS;

THUMB_INSTRUCTIONS gOpThumb[] = {
// Thumb 16-bit instrucitons
//          Op       Mask   Format
  { "ADC" , 0x4140, 0xffc0, DATA_FORMAT5 },
  { "ADR",  0xa000, 0xf800, ADR_FORMAT   },  // ADR <Rd>, <label>
  { "ADD" , 0x1c00, 0xfe00, DATA_FORMAT2 },
  { "ADD" , 0x3000, 0xf800, DATA_FORMAT3 },
  { "ADD" , 0x1800, 0xfe00, DATA_FORMAT1 },
  { "ADD" , 0x4400, 0xff00, DATA_FORMAT8 },   // A8.6.9
  { "ADD" , 0xa000, 0xf100, DATA_FORMAT6_PC },
  { "ADD" , 0xa800, 0xf800, DATA_FORMAT6_SP }, 
  { "ADD" , 0xb000, 0xff80, DATA_FORMAT7 },

  { "AND" , 0x4000, 0xffc0, DATA_FORMAT5 },

  { "ASR" , 0x1000, 0xf800, DATA_FORMAT4 },
  { "ASR" , 0x4100, 0xffc0, DATA_FORMAT5 },

  { "B"   , 0xd000, 0xf000, CONDITIONAL_BRANCH },
  { "B"   , 0xe000, 0xf800, UNCONDITIONAL_BRANCH_SHORT },
  { "BLX" , 0x4780, 0xff80, BRANCH_EXCHANGE },
  { "BX"  , 0x4700, 0xff87, BRANCH_EXCHANGE },

  { "BIC" , 0x4380, 0xffc0, DATA_FORMAT5 },
  { "BKPT", 0xdf00, 0xff00, IMMED_8 },
  { "CBZ",  0xb100, 0xfd00, DATA_CBZ },
  { "CBNZ", 0xb900, 0xfd00, DATA_CBZ },
  { "CMN" , 0x42c0, 0xffc0, DATA_FORMAT5 },

  { "CMP" , 0x2800, 0xf800, DATA_FORMAT3 },
  { "CMP" , 0x4280, 0xffc0, DATA_FORMAT5 },
  { "CMP" , 0x4500, 0xff00, DATA_FORMAT8 },

  { "CPS" , 0xb660, 0xffe8, CPS_FORMAT },
  { "MOV" , 0x4600, 0xff00, DATA_FORMAT8 },
  { "EOR" , 0x4040, 0xffc0, DATA_FORMAT5 },

  { "LDMIA" , 0xc800, 0xf800, LOAD_STORE_MULTIPLE_FORMAT1 },
  { "LDR"   , 0x6800, 0xf800, LOAD_STORE_FORMAT1 },
  { "LDR"   , 0x5800, 0xfe00, LOAD_STORE_FORMAT2 },
  { "LDR"   , 0x4800, 0xf800, LOAD_STORE_FORMAT3 },
  { "LDR"   , 0x9800, 0xf800, LOAD_STORE_FORMAT4 },
  { "LDRB"  , 0x7800, 0xf800, LOAD_STORE_FORMAT1 },
  { "LDRB"  , 0x5c00, 0xfe00, LOAD_STORE_FORMAT2 },
  { "LDRH"  , 0x8800, 0xf800, LOAD_STORE_FORMAT1 },
  { "LDRH"  , 0x7a00, 0xfe00, LOAD_STORE_FORMAT2 },
  { "LDRSB" , 0x5600, 0xfe00, LOAD_STORE_FORMAT2 },
  { "LDRSH" , 0x5e00, 0xfe00, LOAD_STORE_FORMAT2 },
 
  { "MOVS", 0x0000, 0xffc0, DATA_FORMAT5 },   // LSL with imm5 == 0 is a MOVS, so this must go before LSL
  { "LSL" , 0x0000, 0xf800, DATA_FORMAT4 },
  { "LSL" , 0x4080, 0xffc0, DATA_FORMAT5 },
  { "LSR" , 0x0001, 0xf800, DATA_FORMAT4 },
  { "LSR" , 0x40c0, 0xffc0, DATA_FORMAT5 },

  { "MOVS", 0x2000, 0xf800, DATA_FORMAT3 },
  { "MOV" , 0x1c00, 0xffc0, DATA_FORMAT3 },
  { "MOV" , 0x4600, 0xff00, DATA_FORMAT8 },

  { "MUL" , 0x4340, 0xffc0, DATA_FORMAT5 },
  { "MVN" , 0x41c0, 0xffc0, DATA_FORMAT5 },
  { "NEG" , 0x4240, 0xffc0, DATA_FORMAT5 },
  { "ORR" , 0x4180, 0xffc0, DATA_FORMAT5 },
  { "POP" , 0xbc00, 0xfe00, POP_FORMAT },
  { "PUSH", 0xb400, 0xfe00, PUSH_FORMAT },

  { "REV"   , 0xba00, 0xffc0, DATA_FORMAT5 },
  { "REV16" , 0xba40, 0xffc0, DATA_FORMAT5 },
  { "REVSH" , 0xbac0, 0xffc0, DATA_FORMAT5 },

  { "ROR"    , 0x41c0, 0xffc0, DATA_FORMAT5 },
  { "SBC"    , 0x4180, 0xffc0, DATA_FORMAT5 },
  { "SETEND" , 0xb650, 0xfff0, ENDIAN_FORMAT },

  { "STMIA" , 0xc000, 0xf800, LOAD_STORE_MULTIPLE_FORMAT1 },
  { "STR"   , 0x6000, 0xf800, LOAD_STORE_FORMAT1 },
  { "STR"   , 0x5000, 0xfe00, LOAD_STORE_FORMAT2 },
  { "STR"   , 0x4000, 0xf800, LOAD_STORE_FORMAT3 },
  { "STR"   , 0x9000, 0xf800, LOAD_STORE_FORMAT4 },
  { "STRB"  , 0x7000, 0xf800, LOAD_STORE_FORMAT1 },
  { "STRB"  , 0x5800, 0xfe00, LOAD_STORE_FORMAT2 },
  { "STRH"  , 0x8000, 0xf800, LOAD_STORE_FORMAT1 },
  { "STRH"  , 0x5200, 0xfe00, LOAD_STORE_FORMAT2 },

  { "SUB" , 0x1e00, 0xfe00, DATA_FORMAT2 },
  { "SUB" , 0x3800, 0xf800, DATA_FORMAT3 },
  { "SUB" , 0x1a00, 0xfe00, DATA_FORMAT1 },
  { "SUB" , 0xb080, 0xff80, DATA_FORMAT7 },

  { "SWI" , 0xdf00, 0xff00, IMMED_8 },
  { "SXTB", 0xb240, 0xffc0, DATA_FORMAT5 },
  { "SXTH", 0xb200, 0xffc0, DATA_FORMAT5 },
  { "TST" , 0x4200, 0xffc0, DATA_FORMAT5 },
  { "UXTB", 0xb2c0, 0xffc0, DATA_FORMAT5 },
  { "UXTH", 0xb280, 0xffc0, DATA_FORMAT5 }

};

THUMB_INSTRUCTIONS gOpThumb2[] = {
//Instruct  OpCode      OpCode Mask  Addressig Mode
  { "B",    0xf0008000, 0xf800d000, B_T3  },             // B<c> <label>
  { "B",    0xf0009000, 0xf800d000, B_T4  },             // B<c> <label>
  { "BL",   0xf000d000, 0xf800d000, B_T4  },             // BL<c> <label>
  { "BLX",  0xf000c000, 0xf800d000, BL_T2 },             // BLX<c> <label>

  { "POP",   0xe8bd0000, 0xffff2000, POP_T2 },           // POP <registers>
  { "POP",   0xf85d0b04, 0xffff0fff, POP_T3 },           // POP <register>
  { "PUSH",  0xe8ad0000, 0xffffa000, POP_T2 },           // PUSH <registers>
  { "PUSH",  0xf84d0d04, 0xffff0fff, POP_T3 },           // PUSH <register>
  { "STM"  , 0xe8800000, 0xffd0a000,  STM_FORMAT },      // STM <Rn>{!},<registers>
  { "STMDB", 0xe9800000, 0xffd0a000,  STM_FORMAT },      // STMDB <Rn>{!},<registers>
  { "LDM"  , 0xe8900000, 0xffd02000,  STM_FORMAT },      // LDM <Rn>{!},<registers>
  { "LDMDB", 0xe9100000, 0xffd02000,  STM_FORMAT },      // LDMDB <Rn>{!},<registers>
  
  { "LDR",   0xf8d00000, 0xfff00000,  LDM_REG_IMM12 },          // LDR   <rt>, [<rn>, {, #<imm12>]}
  { "LDRB",  0xf8900000, 0xfff00000,  LDM_REG_IMM12 },          // LDRB  <rt>, [<rn>, {, #<imm12>]}
  { "LDRH",  0xf8b00000, 0xfff00000,  LDM_REG_IMM12 },          // LDRH  <rt>, [<rn>, {, #<imm12>]}
  { "LDRSB", 0xf9900000, 0xfff00000,  LDM_REG_IMM12 },          // LDRSB <rt>, [<rn>, {, #<imm12>]}
  { "LDRSH", 0xf9b00000, 0xfff00000,  LDM_REG_IMM12 },          // LDRSH <rt>, [<rn>, {, #<imm12>]}

  { "LDR",   0xf85f0000, 0xff7f0000,  LDM_REG_IMM12_SIGNED },   // LDR   <Rt>, <label> 
  { "LDRB",  0xf81f0000, 0xff7f0000,  LDM_REG_IMM12_SIGNED },   // LDRB  <Rt>, <label> 
  { "LDRH",  0xf83f0000, 0xff7f0000,  LDM_REG_IMM12_SIGNED },   // LDRH  <Rt>, <label> 
  { "LDRSB", 0xf91f0000, 0xff7f0000,  LDM_REG_IMM12_SIGNED },   // LDRSB <Rt>, <label> 
  { "LDRSH", 0xf93f0000, 0xff7f0000,  LDM_REG_IMM12_SIGNED },   // LDRSB <Rt>, <label> 
  
  { "LDR",   0xf8500000, 0xfff00fc0,  LDM_REG_INDIRECT_LSL },   // LDR   <rt>, [<rn>, <rm> {, LSL #<imm2>]}
  { "LDRB",  0xf8100000, 0xfff00fc0,  LDM_REG_INDIRECT_LSL },   // LDRB  <rt>, [<rn>, <rm> {, LSL #<imm2>]}
  { "LDRH",  0xf8300000, 0xfff00fc0,  LDM_REG_INDIRECT_LSL },   // LDRH  <rt>, [<rn>, <rm> {, LSL #<imm2>]}
  { "LDRSB", 0xf9100000, 0xfff00fc0,  LDM_REG_INDIRECT_LSL },   // LDRSB <rt>, [<rn>, <rm> {, LSL #<imm2>]}
  { "LDRSH", 0xf9300000, 0xfff00fc0,  LDM_REG_INDIRECT_LSL },   // LDRSH <rt>, [<rn>, <rm> {, LSL #<imm2>]}

  { "LDR",   0xf8500800, 0xfff00800,  LDM_REG_IMM8 },           // LDR    <rt>, [<rn>, {, #<imm8>]}
  { "LDRBT", 0xf8100e00, 0xfff00f00,  LDM_REG_IMM8 },           // LDRBT  <rt>, [<rn>, {, #<imm8>]}
  { "LDRHT", 0xf8300e00, 0xfff00f00,  LDM_REG_IMM8 },           // LDRHT  <rt>, [<rn>, {, #<imm8>]}
  { "LDRSB", 0xf9900800, 0xfff00800,  LDM_REG_IMM8 },           // LDRHT  <rt>, [<rn>, {, #<imm8>]}  {!} form? 
  { "LDRSBT",0xf9100e00, 0xfff00f00,  LDM_REG_IMM8 },           // LDRHBT <rt>, [<rn>, {, #<imm8>]}  {!} form? 
  { "LDRSH" ,0xf9300800, 0xfff00800,  LDM_REG_IMM8 },           // LDRSH  <rt>, [<rn>, {, #<imm8>]}  
  { "LDRSHT",0xf9300e00, 0xfff00f00,  LDM_REG_IMM8 },           // LDRSHT <rt>, [<rn>, {, #<imm8>]}   
  { "LDRT",  0xf8500e00, 0xfff00f00,  LDM_REG_IMM8 },           // LDRT   <rt>, [<rn>, {, #<imm8>]}    

  { "LDRD",  0xe8500000, 0xfe500000,  LDRD_REG_IMM8_SIGNED },   // LDRD <rt>, <rt2>, [<rn>, {, #<imm8>]}{!}
  { "LDRD",  0xe8500000, 0xfe500000,  LDRD_REG_IMM8       },    // LDRD <rt>, <rt2>, <label>
   
  { "LDREX",  0xe8500f00, 0xfff00f00,  LDM_REG_IMM8 },           // LDREX <Rt>, [Rn, {#imm8}]]                     
  { "LDREXB", 0xe8d00f4f, 0xfff00fff,  LDREXB  },                // LDREXB <Rt>, [<Rn>]                     
  { "LDREXH", 0xe8d00f5f, 0xfff00fff,  LDREXB  },                // LDREXH <Rt>, [<Rn>]                     
 
  { "LDREXD", 0xe8d00f4f, 0xfff00fff,  LDREXD  },                // LDREXD <Rt>, <Rt2>, [<Rn>]                     

  { "STR",   0xf8c00000, 0xfff00000,  LDM_REG_IMM12 },          // STR   <rt>, [<rn>, {, #<imm12>]}  
  { "STRB",  0xf8800000, 0xfff00000,  LDM_REG_IMM12 },          // STRB  <rt>, [<rn>, {, #<imm12>]}
  { "STRH",  0xf8a00000, 0xfff00000,  LDM_REG_IMM12 },          // STRH  <rt>, [<rn>, {, #<imm12>]}
 
  { "STR",   0xf8400000, 0xfff00fc0,  LDM_REG_INDIRECT_LSL },   // STR   <rt>, [<rn>, <rm> {, LSL #<imm2>]}
  { "STRB",  0xf8000000, 0xfff00fc0,  LDM_REG_INDIRECT_LSL },   // STRB  <rt>, [<rn>, <rm> {, LSL #<imm2>]}
  { "STRH",  0xf8200000, 0xfff00fc0,  LDM_REG_INDIRECT_LSL },   // STRH  <rt>, [<rn>, <rm> {, LSL #<imm2>]}

  { "STR",   0xf8400800, 0xfff00800,  LDM_REG_IMM8 },           // STR    <rt>, [<rn>, {, #<imm8>]}
  { "STRH",  0xf8200800, 0xfff00800,  LDM_REG_IMM8 },           // STRH   <rt>, [<rn>, {, #<imm8>]}
  { "STRBT", 0xf8000e00, 0xfff00f00,  LDM_REG_IMM8 },           // STRBT  <rt>, [<rn>, {, #<imm8>]}
  { "STRHT", 0xf8200e00, 0xfff00f00,  LDM_REG_IMM8 },           // STRHT  <rt>, [<rn>, {, #<imm8>]}
  { "STRT",  0xf8400e00, 0xfff00f00,  LDM_REG_IMM8 },           // STRT   <rt>, [<rn>, {, #<imm8>]}    

  { "STRD",  0xe8400000, 0xfe500000,  LDRD_REG_IMM8_SIGNED },    // STRD <rt>, <rt2>, [<rn>, {, #<imm8>]}{!}

  { "STREX",  0xe8400f00, 0xfff00f00,  LDM_REG_IMM8 },           // STREX <Rt>, [Rn, {#imm8}]]                     
  { "STREXB", 0xe8c00f4f, 0xfff00fff,  LDREXB  },                // STREXB <Rd>, <Rt>, [<Rn>]                     
  { "STREXH", 0xe8c00f5f, 0xfff00fff,  LDREXB  },                // STREXH <Rd>, <Rt>, [<Rn>]                     
 
  { "STREXD", 0xe8d00f4f, 0xfff00fff,  LDREXD  },                // STREXD <Rd>, <Rt>, <Rt2>, [<Rn>]                     

  { "SRSDB", 0xe80dc000, 0xffdffff0, SRS_FORMAT },       // SRSDB<c> SP{!},#<mode>
  { "SRS"  , 0xe98dc000, 0xffdffff0, SRS_FORMAT },       // SRS{IA}<c> SP{!},#<mode>
  { "RFEDB", 0xe810c000, 0xffd0ffff, RFE_FORMAT },       // RFEDB<c> <Rn>{!}
  { "RFE"  , 0xe990c000, 0xffd0ffff, RFE_FORMAT }        // RFE{IA}<c> <Rn>{!}
};

CHAR8 mThumbMregListStr[4*15 + 1];

CHAR8 *
ThumbMRegList (
  UINT32  RegBitMask
  )
{
  UINTN     Index, Start, End;
  CHAR8     *Str;
  BOOLEAN   First;
 
  Str = mThumbMregListStr;
  *Str = '\0';
  AsciiStrCat  (Str, "{");
  // R0 - R7, PC
  for (Index = 0, First = TRUE; Index <= 15; Index++) {
    if ((RegBitMask & (1 << Index)) != 0) {
      Start = End = Index;
      for (Index++; ((RegBitMask & (1 << Index)) != 0) && (Index <= 9); Index++) {
        End = Index;
      }
      
      if (!First) {
        AsciiStrCat  (Str, ",");
      } else {
        First = FALSE;
      }
      
      if (Start == End) {
        AsciiStrCat  (Str, gReg[Start]);
      } else {
        AsciiStrCat  (Str, gReg[Start]);
        AsciiStrCat  (Str, "-");
        AsciiStrCat  (Str, gReg[End]);
      }
    }
  }
  if (First) {
    AsciiStrCat  (Str, "ERROR");
  }
  AsciiStrCat  (Str, "}");
  
  // BugBug: Make caller pass in buffer it is cleaner
  return mThumbMregListStr;
}

UINT32
SignExtend32 (
  IN  UINT32  Data,
  IN  UINT32  TopBit
  )
{
  if (((Data & TopBit) == 0) || (TopBit == BIT31)) {
    return Data;
  }
 
  do {
    TopBit <<= 1;
    Data |= TopBit; 
  } while ((TopBit & BIT31) != BIT31);

  return Data;
}

/**
  Place a dissasembly of of **OpCodePtr into buffer, and update OpCodePtr to 
  point to next instructin. 
  
  We cheat and only decode instructions that access 
  memory. If the instruction is not found we dump the instruction in hex.
   
  @param  OpCodePtrPtr  Pointer to pointer of ARM Thumb instruction to disassemble.  
  @param  Buf           Buffer to sprintf disassembly into.
  @param  Size          Size of Buf in bytes. 
  @param  Extended    TRUE dump hex for instruction too.
  
**/
VOID
DisassembleThumbInstruction (
  IN  UINT16    **OpCodePtrPtr,
  OUT CHAR8     *Buf,
  OUT UINTN     Size,
  IN  BOOLEAN   Extended
  )
{
  UINT16  *OpCodePtr;
  UINT16  OpCode;
  UINT32  OpCode32;
  UINT32  Index;
  UINT32  Offset;
  UINT16  Rd, Rn, Rm, Rt, Rt2;
  BOOLEAN H1, H2, imod;
  UINT32  PC, Target;
  CHAR8   *Cond;
  BOOLEAN S, J1, J2, P, U, W;

  OpCodePtr = *OpCodePtrPtr;
  OpCode = **OpCodePtrPtr;
  
  // Thumb2 is a stream of 16-bit instructions not a 32-bit instruction.
  OpCode32 = (((UINT32)OpCode) << 16) | *(OpCodePtr + 1);

  // These register names match branch form, but not others
  Rd = OpCode & 0x7;
  Rn = (OpCode >> 3) & 0x7;
  Rm = (OpCode >> 6) & 0x7;
  H1 = (OpCode & BIT7) != 0;
  H2 = (OpCode & BIT6) != 0;
  imod = (OpCode & BIT4) != 0;
  PC = (UINT32)(UINTN)OpCodePtr;

  // Increment by the minimum instruction size, Thumb2 could be bigger
  *OpCodePtrPtr += 1;
  
  for (Index = 0; Index < sizeof (gOpThumb)/sizeof (THUMB_INSTRUCTIONS); Index++) {
    if ((OpCode & gOpThumb[Index].Mask) == gOpThumb[Index].OpCode) {
      if (Extended) {
        Offset = AsciiSPrint (Buf, Size, "0x%04x       %-6a", OpCode, gOpThumb[Index].Start);   
      } else {
        Offset = AsciiSPrint (Buf, Size, "%-6a", gOpThumb[Index].Start);   
      }
      switch (gOpThumb[Index].AddressMode) {
      case LOAD_STORE_FORMAT1:
        // A6.5.1  <Rd>, [<Rn>, #<5_bit_offset>]
        AsciiSPrint (&Buf[Offset], Size - Offset, " r%d, [r%d #0x%x]", Rd, Rn, (OpCode >> 4) & 0x7c);   
        return;
      case LOAD_STORE_FORMAT2:
        // A6.5.1  <Rd>, [<Rn>, <Rm>]
        AsciiSPrint (&Buf[Offset], Size - Offset, " r%d, [r%d, r%d]", Rd, Rn, Rm);   
        return;
      case LOAD_STORE_FORMAT3:
        // A6.5.1 <Rd>, [PC, #<8_bit_offset>]
        Target = (OpCode & 0xff) << 2;
        AsciiSPrint (&Buf[Offset], Size - Offset, " r%d, [pc, #0x%x] ;0x%08x", (OpCode >> 8) & 7, Target, PC + 2 + Target);   
        return;
      case LOAD_STORE_FORMAT4:
        // Rt, [SP, #imm8]
        Target = (OpCode & 0xff) << 2;
        AsciiSPrint (&Buf[Offset], Size - Offset, " r%d, [sp, #0x%x]", (OpCode >> 8) & 7, Target, PC + 2 + Target);   
        return;
      
      case LOAD_STORE_MULTIPLE_FORMAT1:
        // <Rn>!, {r0-r7}
        AsciiSPrint (&Buf[Offset], Size - Offset, " r%d!, %a", (OpCode >> 8) & 7, ThumbMRegList (OpCode & 0xff));   
        return;
 
      case POP_FORMAT:
        // POP {r0-r7,pc}
        AsciiSPrint (&Buf[Offset], Size - Offset, " %a", ThumbMRegList ((OpCode & 0xff) | ((OpCode & BIT8) == BIT8 ? BIT15 : 0)));   
        return;

      case PUSH_FORMAT:
        // PUSH {r0-r7,lr}
        AsciiSPrint (&Buf[Offset], Size - Offset, " %a", ThumbMRegList ((OpCode & 0xff) | ((OpCode & BIT8) == BIT8 ? BIT14 : 0)));   
        return;

      
      case IMMED_8:
        // A6.7 <immed_8>
        AsciiSPrint (&Buf[Offset], Size - Offset, " 0x%x", OpCode & 0xff);   
        return;

      case CONDITIONAL_BRANCH:
        // A6.3.1 B<cond> <target_address>
        // Patch in the condition code. A little hack but based on "%-6a"
        Cond = gCondition[(OpCode >> 8) & 0xf];
        Buf[Offset-5] = *Cond++;
        Buf[Offset-4] = *Cond;
        AsciiSPrint (&Buf[Offset], Size - Offset, " 0x%04x",  PC + 4 + SignExtend32 ((OpCode & 0xff) << 1, BIT8));   
        return;
      case UNCONDITIONAL_BRANCH_SHORT:
        // A6.3.2 B  <target_address>
        AsciiSPrint (&Buf[Offset], Size - Offset, " 0x%04x", PC + 4 + SignExtend32 ((OpCode & 0x3ff) << 1, BIT11));   
        return;
 
      case BRANCH_EXCHANGE:
        // A6.3.3 BX|BLX <Rm>
        AsciiSPrint (&Buf[Offset], Size - Offset, " %a", gReg[Rn | (H2 ? 8:0)]);   
        return;

      case DATA_FORMAT1:
        // A6.4.3  <Rd>, <Rn>, <Rm>
        AsciiSPrint (&Buf[Offset], Size - Offset, " r%d, r%d, r%d", Rd, Rn, Rm);   
        return;
      case DATA_FORMAT2:
        // A6.4.3  <Rd>, <Rn>, #3_bit_immed
        AsciiSPrint (&Buf[Offset], Size - Offset, " r%d, r%d, 0x%x", Rd, Rn, Rm);   
        return;
      case DATA_FORMAT3:
        // A6.4.3  <Rd>|<Rn>, #imm8
        AsciiSPrint (&Buf[Offset], Size - Offset, " r%d, #0x%x", (OpCode >> 8) & 7, OpCode & 0xff);   
        return;
      case DATA_FORMAT4:
        // A6.4.3  <Rd>|<Rm>, #immed_5
        AsciiSPrint (&Buf[Offset], Size - Offset, " r%d, r%d, 0x%x", Rn, Rd, (OpCode >> 6) & 0x1f);   
        return;
      case DATA_FORMAT5:
        // A6.4.3  <Rd>|<Rm>, <Rm>|<Rs>
        AsciiSPrint (&Buf[Offset], Size - Offset, " r%d, r%d", Rd, Rn);   
        return;
      case DATA_FORMAT6_SP:
        // A6.4.3  <Rd>, <reg>, #<8_Bit_immed>
        AsciiSPrint (&Buf[Offset], Size - Offset, " r%d, sp, 0x%x", (OpCode >> 8) & 7, (OpCode & 0xff) << 2);   
        return;
      case DATA_FORMAT6_PC:
        // A6.4.3  <Rd>, <reg>, #<8_Bit_immed>
        AsciiSPrint (&Buf[Offset], Size - Offset, " r%d, pc, 0x%x", (OpCode >> 8) & 7, (OpCode & 0xff) << 2);   
        return;
      case DATA_FORMAT7:
        // A6.4.3  SP, SP, #<7_Bit_immed>
        AsciiSPrint (&Buf[Offset], Size - Offset, " sp, sp, 0x%x", (OpCode & 0x7f)*4);   
        return;
      case DATA_FORMAT8:
        // A6.4.3  <Rd>|<Rn>, <Rm>
        AsciiSPrint (&Buf[Offset], Size - Offset, " %a, %a", gReg[Rd | (H1 ? 8:0)], gReg[Rn | (H2 ? 8:0)]);   
        return;
      
      case CPS_FORMAT:
        // A7.1.24
        AsciiSPrint (&Buf[Offset], Size - Offset, "%a %a%a%a", imod ? "ID":"IE", ((OpCode & BIT2) == 0) ? "":"a",  ((OpCode & BIT1) == 0) ? "":"i", ((OpCode & BIT0) == 0) ? "":"f");   
        return;

      case ENDIAN_FORMAT:
        // A7.1.24
        AsciiSPrint (&Buf[Offset], Size - Offset, " %a", (OpCode & BIT3) == 0 ? "LE":"BE");   
        return;

      case DATA_CBZ:
        // CB{N}Z <Rn>, <Lable>
        Target = ((OpCode >> 2) & 0x3e) | (((OpCode & BIT9) == BIT9) ? BIT6 : 0);
        AsciiSPrint (&Buf[Offset], Size - Offset, " %a, %08x", gReg[Rd], PC + 4 + Target); 
        return;

      case ADR_FORMAT:
        // ADR <Rd>, <Label>
        Target = (OpCode & 0xff) << 2;
        AsciiSPrint (&Buf[Offset], Size - Offset, " %a, %08x", gReg[(OpCode >> 8) & 7], PC + 4 + Target); 
        return;
      }
    }
  }

  
  // Thumb2 are 32-bit instructions
  *OpCodePtrPtr += 1;
  Rt  = (OpCode32 >> 12) & 0xf;
  Rt2 = (OpCode32 >> 8) & 0xf;
  Rm  = (OpCode32 & 0xf);
  Rn  = (OpCode32 >> 16) & 0xf;
  for (Index = 0; Index < sizeof (gOpThumb2)/sizeof (THUMB_INSTRUCTIONS); Index++) {
    if ((OpCode32 & gOpThumb2[Index].Mask) == gOpThumb2[Index].OpCode) {
      if (Extended) {
        Offset = AsciiSPrint (Buf, Size, "0x%04x   %-6a", OpCode32, gOpThumb2[Index].Start);   
      } else {
        Offset = AsciiSPrint (Buf, Size, "   %-6a", gOpThumb2[Index].Start);   
      }
      switch (gOpThumb2[Index].AddressMode) {
      case B_T3:
        Cond = gCondition[(OpCode32 >> 22) & 0xf];
        Buf[Offset-5] = *Cond++;
        Buf[Offset-4] = *Cond;
        // S:J2:J1:imm6:imm11:0
        Target = ((OpCode32 << 1) & 0xffe) + ((OpCode32 >> 4) & 0x3f000);
        Target |= ((OpCode32 & BIT11) == BIT11)? BIT19 : 0;  // J2
        Target |= ((OpCode32 & BIT13) == BIT13)? BIT18 : 0;  // J1
        Target |= ((OpCode32 & BIT26) == BIT26)? BIT20 : 0;  // S
        Target = SignExtend32 (Target, BIT20);
        AsciiSPrint (&Buf[Offset], Size - Offset, " 0x%08x", PC + 4 + Target);   
        return;
      case B_T4:
        // S:I1:I2:imm10:imm11:0
        Target = ((OpCode32 << 1) & 0xffe) + ((OpCode32 >> 4) & 0x3ff000);
        S  = (OpCode32 & BIT26) == BIT26;
        J1 = (OpCode32 & BIT13) == BIT13;
        J2 = (OpCode32 & BIT11) == BIT11;
        Target |= (!(J2 ^ S) ? BIT22 : 0);  // I2
        Target |= (!(J1 ^ S) ? BIT23 : 0);  // I1
        Target |= (S ? BIT24 : 0);  // S
        Target = SignExtend32 (Target, BIT24);
        AsciiSPrint (&Buf[Offset], Size - Offset, " 0x%08x", PC + 4 + Target);   
        return;

      case BL_T2:
        // S:I1:I2:imm10:imm11:00
        Target = ((OpCode32 << 2) & 0x1ffc) + ((OpCode32 >> 3) & 0x7fe000);
        S  = (OpCode32 & BIT26) == BIT26;
        J1 = (OpCode32 & BIT13) == BIT13;
        J2 = (OpCode32 & BIT11) == BIT11;
        Target |= (!(J2 ^ S) ? BIT23 : 0);  // I2
        Target |= (!(J1 ^ S) ? BIT24 : 0);  // I1
        Target |= (S ? BIT25 : 0);  // S
        Target = SignExtend32 (Target, BIT25);
        AsciiSPrint (&Buf[Offset], Size - Offset, " 0x%08x", PC + 4 + Target);   
        return;

      case POP_T2:
        // <reglist>  some must be zero, handled in table
        AsciiSPrint (&Buf[Offset], Size - Offset, " %a", ThumbMRegList (OpCode32 & 0xffff));
        return;

      case POP_T3:
        // <register> 
        AsciiSPrint (&Buf[Offset], Size - Offset, " %a", gReg[(OpCode32 >> 12) & 0xf]);
        return;

      case STM_FORMAT:
        // <Rn>{!}, <registers>
        W = (OpCode32 & BIT21) == BIT21;
        AsciiSPrint (&Buf[Offset], Size - Offset, " %a %a", gReg[(OpCode32 >> 16) & 0xf], W ? "!":"", ThumbMRegList (OpCode32 & 0xffff));
        return;

      case LDM_REG_IMM12_SIGNED:
        // <rt>, <label>
        Target = OpCode32 & 0xfff; 
        if ((OpCode32 & BIT23) == 0) {
          // U == 0 means subtrack, U == 1 means add
          Target = -Target;
        }
        AsciiSPrint (&Buf[Offset], Size - Offset, " %a, %a", gReg[(OpCode32 >> 12) & 0xf], PC + 4 + Target);
        return;

      case LDM_REG_INDIRECT_LSL:
        // <rt>, [<rn>, <rm> {, LSL #<imm2>]}
        Offset += AsciiSPrint (&Buf[Offset], Size - Offset, " %a, [%a, %a", gReg[Rt], gReg[Rn], gReg[Rm]);
        if (((OpCode32 >> 4) && 3) == 0) {
          AsciiSPrint (&Buf[Offset], Size - Offset, "]");
        } else {
          AsciiSPrint (&Buf[Offset], Size - Offset, ", LSL #%d]", (OpCode32 >> 4) && 3);
        }
        return;
      
      case LDM_REG_IMM12:
        // <rt>, [<rn>, {, #<imm12>]}
        Offset += AsciiSPrint (&Buf[Offset], Size - Offset, " %a, [%a", gReg[Rt], gReg[Rn]);
        if ((OpCode32 && 0xfff) == 0) {
          AsciiSPrint (&Buf[Offset], Size - Offset, "]");
        } else {
          AsciiSPrint (&Buf[Offset], Size - Offset, ", #0x%x]", OpCode32 & 0xfff);
        }
        return;

      case LDM_REG_IMM8:
        // <rt>, [<rn>, {, #<imm8>}]{!}
        W = (OpCode32 & BIT8) == BIT8;
        U = (OpCode32 & BIT9) == BIT9;
        P = (OpCode32 & BIT10) == BIT10;
        Offset += AsciiSPrint (&Buf[Offset], Size - Offset, " %a, [%a", gReg[Rt], gReg[Rn]);
        if (P) {
          if ((OpCode32 && 0xff) == 0) {
            AsciiSPrint (&Buf[Offset], Size - Offset, "]%a", W?"!":"");
          } else {
            AsciiSPrint (&Buf[Offset], Size - Offset, ", #%a0x%x]%a", OpCode32 & 0xff, U?"":"-" ,W?"!":"");
          }
        } else {
          AsciiSPrint (&Buf[Offset], Size - Offset, "], #%a0x%x]%a", OpCode32 & 0xff, U?"":"-");
        }
        return;

      case LDRD_REG_IMM8_SIGNED:
        // LDRD <rt>, <rt2>, [<rn>, {, #<imm8>]}{!}
        P = (OpCode32 & BIT24) == BIT24;  // index = P
        U = (OpCode32 & BIT23) == BIT23;  
        W = (OpCode32 & BIT21) == BIT21;
        Offset += AsciiSPrint (&Buf[Offset], Size - Offset, " %a, %a, [%a", gReg[Rt], gReg[Rt2], gReg[Rn]);
        if (P) {
          if ((OpCode32 && 0xff) == 0) {
            AsciiSPrint (&Buf[Offset], Size - Offset, "]");
          } else {
            AsciiSPrint (&Buf[Offset], Size - Offset, ", #%a0x%x]%a", U?"":"-", (OpCode32 & 0xff) << 2, W?"!":"");
          }
        } else {
          if ((OpCode32 && 0xff) != 0) {
            AsciiSPrint (&Buf[Offset], Size - Offset, ", #%a0x%x", U?"":"-", (OpCode32 & 0xff) << 2);
          }
        }
        return;

      case LDRD_REG_IMM8: 
        // LDRD <rt>, <rt2>, <label>   
        Target = (OpCode32 & 0xff) << 2; 
        if ((OpCode32 & BIT23) == 0) {
          // U == 0 means subtrack, U == 1 means add
          Target = -Target;
        }
        AsciiSPrint (&Buf[Offset], Size - Offset, " %a, %a, %a", gReg[Rt], gReg[Rt2], PC + 4 + Target);
        return;

      case LDREXB:
        // LDREXB <Rt>, [Rn]
        AsciiSPrint (&Buf[Offset], Size - Offset, " %a, [%a]", gReg[Rt], gReg[Rn]);
        return;

      case LDREXD:
        // LDREXD <Rt>, <Rt2>, [<Rn>]
        AsciiSPrint (&Buf[Offset], Size - Offset, " %a, ,%a, [%a]", gReg[Rt], gReg[Rt2], gReg[Rn]);
        return;
      
      case SRS_FORMAT:
        // SP{!}, #<mode>
        W = (OpCode32 & BIT21) == BIT21;
        AsciiSPrint (&Buf[Offset], Size - Offset, " SP%a, #0x%x", W?"!":"", OpCode32 & 0x1f);
        return;

      case RFE_FORMAT:
        // <Rn>{!}
        AsciiSPrint (&Buf[Offset], Size - Offset, " %a%a, #0x%x", gReg[Rn], W?"!":"");
        return;

      }
    }
  }

  AsciiSPrint (Buf, Size, "0x%08x", OpCode32);
}



VOID
DisassembleArmInstruction (
  IN  UINT32    **OpCodePtr,
  OUT CHAR8     *Buf,
  OUT UINTN     Size,
  IN  BOOLEAN   Extended
  );


/**
  Place a dissasembly of of **OpCodePtr into buffer, and update OpCodePtr to 
  point to next instructin. 
  
  We cheat and only decode instructions that access 
  memory. If the instruction is not found we dump the instruction in hex.
   
  @param  OpCodePtrPtr  Pointer to pointer of ARM Thumb instruction to disassemble.  
  @param  Thumb         TRUE for Thumb(2), FALSE for ARM instruction stream
  @param  Extended      TRUE dump hex for instruction too.
  @param  Buf           Buffer to sprintf disassembly into.
  @param  Size          Size of Buf in bytes. 
  
**/
VOID
DisassembleInstruction (
  IN  UINT8     **OpCodePtr,
  IN  BOOLEAN   Thumb,
  IN  BOOLEAN   Extended,
  OUT CHAR8     *Buf,
  OUT UINTN     Size
  )
{
  if (Thumb) {
    DisassembleThumbInstruction ((UINT16 **)OpCodePtr, Buf, Size, Extended);
  } else {
    DisassembleArmInstruction ((UINT32 **)OpCodePtr, Buf, Size, Extended);
  }
}
 