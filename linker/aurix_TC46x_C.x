/*
################################################################################
#                                                                              #
#    TC46x                                       |   ETAS GmbH                 #
#    ETK INCA Integration Example                |   Stuttgart Feuerbach       #
#    Project specific linker script                                            #
#    For Demonstration Purpose Only              |   All rights reserved       #
#                                                |   Alle Rechte vorbehalten   #
#                                                                              #
################################################################################
*/

/******************************************************************************/
/* Copyright and Legal Disclaimer:                                            */
/* This Code example was provided by ETAS GmbH, Stuttgart                     */
/*                                                                            */
/* ETAS will not be held reliable for any usage of this code,                 */
/* this code is provided as example code only, and not tested and released    */
/* for production code.                                                       */
/* ETAS will not guarantee any functional part of this code in any            */
/* environment together with any ETAS tools. ETAS will not guarantee that     */
/* this code works together with any future versions of ETAS tools.           */
/*                                                                            */
/* ETAS will not guarantee that this code is free of rights of third parties. */
/*                                                                            */
/* You are hereby granted the right to use his code as a example for your own */
/* ECU implementation together with ETAS Tools. No licenses are granted by    */
/* implication or otherwise under any patents or trademarks of ETAS GmbH.     */
/* This software is provided on an "AS IS" basis and without warranty.        */
/*                                                                            */
/* You are not allowed to give these code to third parties without            */
/* the written permission of ETAS GmbH.                                       */
/*                                                                            */
/* To the maximum extent permitted by applicable law,                         */
/* ETAS GmbH DISCLAIMS ALL WARRANTIES WHETHER EXPRESS OR IMPLIED,             */
/* INCLUDING IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A           */
/* PARTICULAR PURPOSE AND ANY WARRANTY AGAINST INFRINGEMENT WITH REGARD       */
/* TO THE SOFTWARE (INCLUDING ANY MODIFIED VERSIONS THEREOF)                  */
/* AND ANY ACCOMPANYING WRITTEN MATERIALS.                                    */
/*                                                                            */
/*                                                                            */
/* To the maximum extent permitted by applicable law,                         */
/* IN NO EVENT SHALL ETAS BE LIABLE FOR ANY                                   */
/* DAMAGES WHATSOEVER (INCLUDING WITHOUT LIMITATION, DAMAGES FOR LOSS OF      */
/* BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION,     */
/* OR OTHER PECUNIARY LOSS)                                                   */
/* ARISING OF THE USE OR INABILITY TO USE THE SOFTWARE.                       */
/*                                                                            */
/*                                                                            */
/* ETAS GmbH assumes no responsibility for the maintenance                    */
/* and support of this software                                               */
/*                                                                            */
/*                                                                            */
/*  COPYRIGHT (c) ETAS GmbH 2023                                              */
/*  All Rights Reserved                                                       */
/******************************************************************************/


OUTPUT_FORMAT("elf32-tricore")
OUTPUT_ARCH(tricore)
ENTRY(_START)


/* the internal ram description */
__INT_CODE_RAM_BEGIN = 0x70100000;
__INT_CODE_RAM_SIZE  = 64K;
__INT_DATA_RAM_BEGIN = 0x70000000;
__INT_DATA_RAM_SIZE  = 240K;

/* used to check for HEAP_SIZE */
__RAM_END = __INT_DATA_RAM_BEGIN + __INT_DATA_RAM_SIZE;

/* defines the Context Area size in bytes to be allocated
*  One CSA is 64 bytes. To allocate 64 CSA we need 4Kbytes. */
__CSA_SIZE_REQUEST = 4K;

__ISTACK_SIZE = 0x100;
__USTACK_SIZE = 4K-0x300;
__HEAP_MIN = 0x200;

MEMORY
{
  int_prram_0             (rx): org = 0x80000000, len = 0x7C000     /* PRRAM00 */
  int_prram_0_section_2   (rx): org = 0x8007C000, len = 0x4000      /* PRRAM00 */
  int_prram_0_section_3   (rx): org = 0x80080000, len = 0x7F800     /* PRRAM00 */
  CAL_DATA                (rx): org = 0x80200000, len = 0x200000    /* PRRAM01 */

  int_prram_1             (rx): org = 0x80400000, len = 0x400000
  int_prram_2             (rx): org = 0x80800000, len = 0x200000
  int_prram_3             (rx): org = 0x80A00000, len = 0x200000
  int_prram_cs            (rx): org = 0x84000000, len = 0x80000     /* 512 Kbyte */
  int_dflash_0 (r): org = 0xAE000000, len = 512k
  int_dflash_1 (r): org = 0xAE800000, len = 128k

  int_UCB_RTC_BMHD_0     (r): org = 0xAE400900, len = 0x100
  int_UCB_RTC_BMHD_1     (r): org = 0xAE400A00, len = 0x100
  int_UCB_RTC_USERCFG    (r): org = 0xAE400200, len = 0x200  /* USERCFG_ORIG + USERCFG_COPY */
  int_UCB_CS_BMHD_0      (r): org = 0xAEC00300, len = 0x100


  /*dspr0 (w!x): org = 0x70000000, len = 240K  last 16 kB from 256KB reserved for data cache */
  dspr0 (w!x): org = 0x70000000, len = 0x3A000
  dspr0_stack (w!x): org = 0x7003A000, len = 0x2000 
  pspr0 (rx): org = 0x70100000, len = 64K  /* last 32 kB of 96 kB reserved for instruction cache */

  /*dspr1 (w!x): org = 0x60000000, len = 240K  last 16 kB from 256KB reserved for data cache */
  dspr1 (w!x): org = 0x60000000, len = 0x3A000
  dspr1_stack (w!x): org = 0x6003A000, len = 0x2000 
  pspr1 (rx): org = 0x60100000, len = 64K  /* last 32 kB of 96 kB reserved for instruction cache */

  dspr2 (w!x): org = 0x50000000, len = 240K /* last 16 kB from 256KB reserved for data cache */
  pspr2 (rx): org = 0x50100000, len = 64K  /* last 32 kB of 96 kB reserved for instruction cache */

  dspr3 (w!x): org = 0x40000000, len = 112K /* last 16 kB from 256KB reserved for data cache */
  pspr3 (rx): org = 0x40100000, len = 64K  /* last 32 kB of 96 kB reserved for instruction cache */

  dsprcs (w!x): org = 0x10000000, len = 240K /* last 16 kB from 256KB reserved for data cache */
  psprcs (rx): org = 0x10100000, len = 64K  /* last 32 kB of 96 kB reserved for instruction cache */


  dlmu0 (w!x): org = 0x90000000, len = 0x40000  /* 256 kbytes */
  dlmu1 (w!x): org = 0x90040000, len = 0x40000  /* 256 kbytes */
  dlmu2 (w!x): org = 0x90080000, len = 0x40000  /* 256 kbytes */
  dlmu3 (w!x): org = 0x900C0000, len = 0x20000  /* 128 kbytes */

  /***  emulation RAM: 0x9040 0000h - 0x9047 FFFFh (480kB) ***
  //*** LMURAM section ***/
  LMURAM (w!x): org = 0x90400000, len = 0x78000
  LMURAM_CODECHECK_PATTERN (w!x): org = 0x90478000, len = 0x8000  /* 32kB */
}

/* Logger Flash Size */
__LOGGER_FLASH_SIZE = LENGTH(int_prram_2);

/* Single Page */
/* Address Offset RP to SP for measCalibParam (0 in all linker files that don't support single page)*/
__cal_sp_start  = 0;
__cal_sp_size   = 0;
__cal_sp_offset = 0;
__cal_rp_start  = 0;
__cal_rp_end    = 0;

SECTIONS
{
  /********************************* START ************************************/

  .Boot_Mode_Header_0 0xAE400900 : ALIGN(4)
  {
    KEEP(*(.UCB_RTC_BMHD0))
  } > int_UCB_RTC_BMHD_0

  .Boot_Mode_Header_1 0xAE400A00 : ALIGN(4)
  {
    KEEP(*(.UCB_RTC_BMHD1))
  } > int_UCB_RTC_BMHD_1

  .Boot_Mode_Header_CS 0xAEC00300 : ALIGN(4)
  {
    KEEP(*(.UCB_CS_BMHD0))
  } > int_UCB_CS_BMHD_0

  .Alternate_Boot_Mode_Header_0 0x80000000 :
  {
    KEEP(*(.BMHD0))
  } > int_prram_0

  .UCB_RTC_USERCFG 0xAE400200: ALIGN(4)
  {
    KEEP(*(.UCB_RTC_USERCFG))
  } > int_UCB_RTC_USERCFG
  
  .init.startup 0x80000020 :
  {
    KEEP (*(.init.startup.code))
    . = ALIGN(8);
  } > int_prram_0

  .startup ALIGN(8) :
  {
    KEEP  (*(.startup.code))
    KEEP  (*(.cstart.code))
    KEEP  (*(.c_init.code))
    KEEP  (*(.trap_handler.code))
  } >  int_prram_0

  .traptable 0x80001000 : ALIGN(0x100)
  {
      __TRAP_TABLE = ((. + 0xFF) & ~ 0xFF);  /* for the trap table a 0x100 alignment is necessary */
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    ATTENTION: THIS ALIGNMENT IS RELATIVE TO THE SECTION INITIAL PLACEMENT. IF the section starts at 0xA00000E0 it will give alignments like 0xA00001E0.
   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

      . = __TRAP_TABLE;
      *(.traptab)
      *(.trapmmu.*)
      *(.trapmpro.*)
      *(.trapins.*)
      *(.trapcon.*)
      *(.trapbus.*)
      *(.trapass.*)
      *(.trapsys.*)
      *(.trapnmi.*)

  } > int_prram_0

  /*
   * The interrupt table is placed first so that, when it is at its maximum size,
   * it is 8K aligned. Otherwise the wrong vectors get selected...
   */
    .inttable : ALIGN(0x2000) 
    {
      __INT_TABLE = (. + 0x1FFF) & ~ 0x1FFF;   /* for the interrupt table a 8kB alignment */
      . = __INT_TABLE;
      *(.ISR0.inttab)
      *(.ISR1.inttab)
      *(.ISR2.inttab)
    } > int_prram_0



  /*****************************************************************************/
  /* CPUcs code */
  .init.startup_CPUCS 0x84000000 :
  {
    KEEP  (*(.init.startup_cpucs.code))
    . = ALIGN(8);
  } > int_prram_cs

  .startup_CPUcs ALIGN(8) :
  {
    KEEP  (*(.cstart_cpucs.code))
  } > int_prram_cs


  /********************************* DATA *************************************/

  .TraceTriggers  0x70000008: ALIGN(4)
  {
    *(.ETK_Trace_TriggerByValue);
  } > dspr0

  .data : ALIGN(8)
  {
    DATA_BASE = . ;
    *(.data)

    *(.IRAM);
    *(.dT_measurement)
    *(.measure_variables)
    *(.measure_dummies)
    *(.measureCalibParam)
    *(.VERSION_XYZ)
    *(.ETK_HandshakeInfoStruct)
    SORT(CONSTRUCTORS)
    . = ALIGN(8) ;
    DATA_END = . ;
  } > dspr0 AT> int_prram_0


  .dataNoLoad (NOLOAD) : ALIGN(4)
  {
    *(.ETK_ColdStart_MemClass);
    *(.ETK_DisTab_MemClass);
    *(.ETK_Presence_checkMemClass); /* Used by the Parallel ETK, should be in flash ? */
    *(.code_checkMemClassRam);

    . = ALIGN(8) ;
    *(.IRAM_CPU0toDSPR0_measure_variables_performance_bss_8BAlign)
    *(.IRAM_CPU0toDSPR0_measure_variables_performance_bss)
    *(.IRAM_CPU0_measure_variables_performance_bss_1BAlign)

    . = ALIGN(4) ;
  } > dspr0

  .zdata  : ALIGN(4)
  {
    ZDATA_BASE = . ;
    *(.zrodata)
    *(.zrodata.*)
    *(.zdata)
    *(.zdata.*)
    *(.gnu.linkonce.z.*)
    *(.bdata)
    *(.bdata.*)
    ZDATA_END = . ;
  } > dspr0 AT> int_prram_0

  .sdata  : ALIGN(4)
  {
    SDATA_BASE = . ;
    PROVIDE(__sdata_start = .);
    PROVIDE(_SMALL_DATA_ = . + 0x8000);  /* address definition for initializing A0 for short addressing mode (backward addressing ex:A0-0x66f0)*/

    *(.sdata)
    *(.sdata.*)
    *(.gnu.linkonce.s.*)
  } > dspr0 AT> int_prram_0

  .sbss  : ALIGN(4)
  {
    PROVIDE(__sbss_start = .);
    *(.sbss)
    *(.sbss.*)
    *(.gnu.linkonce.sb.*)
  } > dspr0

 .zbss  (NOLOAD) : ALIGN(4)
  {
    ZBSS_BASE = . ;
    *(.zbss)
    *(.zbss.*)
    *(.gnu.linkonce.zb.*)
    *(.bbss)
    *(.bbss.*)
    . = ALIGN(8);
    ZBSS_END = . ;
  } > dspr0

  .bss  (NOLOAD) : ALIGN(4)
  {
    BSS_BASE = . ;
    *(.bss)
    *(.bss.*)
    *(.gnu.linkonce.b.*)

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
You should always check if .ERAM should stay in the .bss or be put back in the .data section.
This was moved here to workaround a flashing problem when flashing a big section with 0x0 bytes
that would not get flashed.
   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

    *(.ETK_DAQChnl_MemClass);

    *(.ETK_DisTab17_EventList);
    *(.ETK_DisTab17_EventConfigArea);
    *(.ETK_DisTab17_EventOutputArea);

    *(COMMON)
  } > dspr0

  /* Define the user and interrupt stack in a different group that should not be initialized
     or the stack will empty itself in the c_init function.
     Also define the CSA Memory area with the __CSA_SIZE_REQUEST RAM size.

     To avoid initializing this memory region we should not declare it in the provided __clear_table at the end of this file.

     !! The linker may put a different ram group at the same address as the ISTACK, USTACK, and CSA because it looks empty for the linker.
     !! Thus the section declaration for these 3 sections should always be declared in one same group and at the end of the RAM section
     !! after all the other RAM section declarations to be sure nothing will be put inside it. */
  .stack_csa_CPU0  (NOLOAD) : ALIGN(64)
  {
    __CSA_BEGIN = . ;
    . += __CSA_SIZE_REQUEST;
    . = ALIGN(64);
    __CSA_END = .;

    __ISTACK_CPU0_BEGIN = . ;
    . += __ISTACK_SIZE ;
    __ISTACK_CPU0_END = . ;

    . = ALIGN(64) ;

    __USTACK_CPU0_BEGIN = . ;
    . += __USTACK_SIZE ;
    __USTACK_CPU0_END = . ;


    /* The following defines are used to check if we have enough RAM for the stacks. */
    __ISTACK = __ISTACK_CPU0_END ;
    __USTACK = __USTACK_CPU0_END ;
    __HEAP = __USTACK ;
    __HEAP_END = __RAM_END ;

    __CSA_SIZE = __CSA_END - __CSA_BEGIN;
    __CSA_BEGIN_CPU0 = __CSA_BEGIN;
    __CSA_END_CPU0 = __CSA_END;

  } > dspr0_stack


  .bss_CPU1 (NOLOAD) : ALIGN(8)
  {
    BSS_CPU1_BASE = . ;

    *(.ERAM_CPU0toDSPR1_measure_variables_performance_bss_8BAlign)
    *(.ERAM_CPU0toDSPR1_measure_variables_performance_bss)
  } > dspr1


  /* EMU RAM Codecheck - Reserved EMU RAM section for ECU use */
  .LMURAM_CODCHECK  (NOLOAD) : ALIGN(4)
  {
    *(.ETK_DataFreeze_Mailbox);
    *(.code_checkMemClassEMURam);
    *(.ETK_PageSwitch_MemClass);
    *(.ETK_OMDTable_MemClass);
  } > LMURAM_CODECHECK_PATTERN


  /********************************* CODE *************************************/
  
  /* FLASH KEY Pattern */
  .flash_key  0x800FF7F0 :
  {
    *(.FLASH_KEY)
  } > int_prram_0_section_3


  .text  : ALIGN(4)
  {
    *(.text)
    *(.text.*)
    *(.pcp_c_ptr_init)
    *(.pcp_c_ptr_init.*)
    *(.gnu.linkonce.t.*)
    /*
     * .gnu.warning sections are handled specially by elf32.em.
     */
    *(.gnu.warning)
  } > int_prram_0


  .rodata   : ALIGN(4)
  {
    *(.rodata)
    *(.rodata.*)
    *(.RoData_Core0)
    *(.gnu.linkonce.r.*)
    *(.rodata1)
    *(.toc)
    *(.jcr)
    
    *(.ETK_DisTab17_ECUTriggerMap)
    *(.ETK_DisTab17_ECUTriggerTable)
    *(.ETK_DisTabDescriptor);
    *(.IROM)
    *(.ECU_Code_checkMemClass)


    /*
     * Create the clear and copy tables that tell the startup code
     * which memory areas to clear and to copy, respectively.
     */
    . = ALIGN(4) ;
    PROVIDE(__clear_table = .) ;
    LONG(0 + ADDR(.bss));     LONG(SIZEOF(.bss));
    LONG(0 + ADDR(.sbss));    LONG(SIZEOF(.sbss));
    LONG(0 + ADDR(.zbss));    LONG(SIZEOF(.zbss));
    LONG(-1);                 LONG(-1);
    PROVIDE(__copy_table = .) ;
    LONG(LOADADDR(.data));    LONG(0 + ADDR(.data));    LONG(SIZEOF(.data));
    LONG(-1);                 LONG(-1);                 LONG(-1);
  } > int_prram_0



  /********************************* Logger Use Case ********************************/
    /*
   * Logger Template for Logger use case,
   * fixed address at 0x80800000
   * DAQ Config,
   * Raster Config,
   */
   .ETK_HandshakeInfoLoggerConfig 0x80800000: ALIGN(4)
  {
    *(.ETK_HandshakeInfoLoggerTemplate);
    *(.ETK_HandshakeInfoLoggerDaq);
    *(.ETK_HandshakeInfoLoggerRaster);
  } > int_prram_2

/********************************* CAL PARAM ********************************/

  .CAL_PARAM  : ALIGN(4)
  {
    *(.rodata.CAL_PARAM)
    *(.CAL_PARAM)
    *(.rodata.*.CAL_PARAM)
    *(.measure_parameters)
    *(.measure_parameters_CPU1)
    *(.ETK_Code_checkMemClass)
    *(.coldstart_parameters);
    *(.VIRTUAL_DATA)
  } > CAL_DATA


  .CP_1  0x80200800 : ALIGN(4)
  {
    *(.calibParam_01)
  } > CAL_DATA
  .CP_2  0x80220000 : ALIGN(4)
  {
    *(.calibParam_02)
  } > CAL_DATA
  .CP_3  0x80240000 : ALIGN(4)
  {
    *(.calibParam_03)
  } > CAL_DATA
  .CP_4  0x80260000 : ALIGN(4)
  {
    *(.calibParam_04)
  } > CAL_DATA
  .CP_5  0x80280000 : ALIGN(4)
  {
    *(.calibParam_05)
  } > CAL_DATA
  .CP_6  0x802A0000 : ALIGN(4)
  {
    *(.calibParam_06)
  } > CAL_DATA
  .CP_7  0x802C0000 : ALIGN(4)
  {
    *(.calibParam_07)
  } > CAL_DATA
  .CP_8  0x802E0000 : ALIGN(4)
  {
    *(.calibParam_08)
  } > CAL_DATA
  .CP_9  0x80300000 : ALIGN(4)
  {
    *(.calibParam_09)
  } > CAL_DATA
  .CP_10  0x80320000 : ALIGN(4)
  {
    *(.calibParam_10)
  } > CAL_DATA
  .CP_11  0x80340000 : ALIGN(4)
  {
    *(.calibParam_11)
  } > CAL_DATA
  .CP_12  0x80360000 : ALIGN(4)
  {
    *(.calibParam_12)
  } > CAL_DATA
  .CP_13  0x80380000 : ALIGN(4)
  {
    *(.calibParam_13)
  } > CAL_DATA
  .CP_14  0x803A0000 : ALIGN(4)
  {
    *(.calibParam_14)
  } > CAL_DATA
  .CP_15  0x803C0000 : ALIGN(4)
  {
    *(.calibParam_15)
  } > CAL_DATA
  .CP_16  0x803E0000 : ALIGN(4)
  {
    *(.calibParam_16)
  } > CAL_DATA
  .CP_17  0x80600000 : ALIGN(4)
  {
    *(.calibParam_17)
  } > int_prram_1
  .CP_18  0x80620000 : ALIGN(4)
  {
    *(.calibParam_18)
  } > int_prram_1
  .CP_19  0x80640000 : ALIGN(4)
  {
    *(.calibParam_19)
  } > int_prram_1
  .CP_20  0x80660000 : ALIGN(4)
  {
    *(.calibParam_20)
  } > int_prram_1
  .CP_21  0x80680000 : ALIGN(4)
  {
    *(.calibParam_21)
  } > int_prram_1
  .CP_22  0x806A0000 : ALIGN(4)
  {
    *(.calibParam_22)
  } > int_prram_1
  .CP_23  0x806C0000 : ALIGN(4)
  {
    *(.calibParam_23)
  } > int_prram_1
  .CP_24  0x806E0000 : ALIGN(4)
  {
    *(.calibParam_24)
  } > int_prram_1
  .CP_25  0x80B00000 : ALIGN(4)
  {
    *(.calibParam_25)
  } > int_prram_3
  .CP_26  0x80B20000 : ALIGN(4)
  {
    *(.calibParam_26)
  } > int_prram_3
  .CP_27  0x80B40000 : ALIGN(4)
  {
    *(.calibParam_27)
  } > int_prram_3
  .CP_28  0x80B60000 : ALIGN(4)
  {
    *(.calibParam_28)
  } > int_prram_3
  .CP_29  0x80B80000 : ALIGN(4)
  {
    *(.calibParam_29)
  } > int_prram_3
  .CP_30  0x80BA0000 : ALIGN(4)
  {
    *(.calibParam_30)
  } > int_prram_3
  .CP_31  0x80BC0000 : ALIGN(4)
  {
    *(.calibParam_31)
  } > int_prram_3
  .CP_32  0x80BE0000 : ALIGN(4)
  {
    *(.calibParam_32)
  } > int_prram_3



  /****************************************************************************/

  _end = __HEAP_END ;
  PROVIDE(end = _end) ;
  /* Make sure CSA, stack and heap addresses are properly aligned.  */
  _. = ASSERT ((__CSA_BEGIN & 0x3f) == 0 , "illegal CSA start address") ;
  _. = ASSERT ((__CSA_SIZE & 0x3f) == 0 , "illegal CSA size") ;
/*  _. = ASSERT ((__CSA_SIZE) > 0 , "NO CSA allocated") ; */
  _. = ASSERT ((__ISTACK & 7) == 0 , "ISTACK not doubleword aligned") ;
  _. = ASSERT ((__USTACK & 7) == 0 , "USTACK not doubleword aligned") ;
  _. = ASSERT ((__HEAP_END & 7) == 0 , "HEAP not doubleword aligned") ;
  /* Make sure enough memory is available for stacks and heap.  */
  _. = ASSERT (__ISTACK <= __RAM_END , "There is not enough memory for ISTACK") ;
  _. = ASSERT (__USTACK <= __RAM_END , "There is not enough memory for USTACK") ;
  _. = ASSERT ((__HEAP_END - __HEAP) >= __HEAP_MIN , "There is not enough memory for HEAP") ;
  /* Define a default symbol for address 0.  */
  NULL = DEFINED (NULL) ? NULL : 0 ;
  /*
   * DWARF debug sections.
   * Symbols in the DWARF debugging sections are relative to the
   * beginning of the section, so we begin them at 0.
   */
  /*
   * DWARF 1
   */
  .comment         0 : { *(.comment) }
  .debug           0 : { *(.debug) }
  .line            0 : { *(.line) }
  /*
   * GNU DWARF 1 extensions
   */
  .debug_srcinfo   0 : { *(.debug_srcinfo) }
  .debug_sfnames   0 : { *(.debug_sfnames) }
  /*
   * DWARF 1.1 and DWARF 2
   */
  .debug_aranges   0 : { *(.debug_aranges) }
  .debug_pubnames  0 : { *(.debug_pubnames) }
  /*
   * DWARF 2
   */
  .debug_info      0 : { *(.debug_info) }
  .debug_abbrev    0 : { *(.debug_abbrev) }
  .debug_line      0 : { *(.debug_line) }
  .debug_frame     0 : { *(.debug_frame) }
  .debug_str       0 : { *(.debug_str) }
  .debug_loc       0 : { *(.debug_loc) }
  .debug_macinfo   0 : { *(.debug_macinfo) }
  .debug_ranges    0 : { *(.debug_ranges) }
  /*
   * SGI/MIPS DWARF 2 extensions
   */
  .debug_weaknames 0 : { *(.debug_weaknames) }
  .debug_funcnames 0 : { *(.debug_funcnames) }
  .debug_typenames 0 : { *(.debug_typenames) }
  .debug_varnames  0 : { *(.debug_varnames) }
  /*
   * Optional sections that may only appear when relocating.
   */
  /*
   * Optional sections that may appear regardless of relocating.
   */
  .version_info    0 : { *(.version_info) }
  .boffs           0 : { KEEP (*(.boffs)) }
}

