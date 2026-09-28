/*
################################################################################
#                                                                              #
#    TC49x                                       |   ETAS GmbH                 #
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
/*  COPYRIGHT (c) ETAS GmbH 2022                                              */
/*  All Rights Reserved                                                       */
/******************************************************************************/


OUTPUT_FORMAT("elf32-tricore")
OUTPUT_ARCH(tricore)
ENTRY(_START)


/* the internal ram description */
__INT_CODE_RAM_BEGIN = 0x70100000;
__INT_CODE_RAM_SIZE  = 64K;
__INT_DATA_RAM_BEGIN = 0x70000000;
__INT_DATA_RAM_SIZE  = 96K;

/* used to check for HEAP_SIZE - The last 8 Kbytes are reserved for the bypass and calibration variables and should not be used in the RAM end. */
__RAM_END = __INT_DATA_RAM_BEGIN + __INT_DATA_RAM_SIZE - 8K;

/* defines the Context Area size in bytes to be allocated
*  One CSA is 64 bytes. To allocate 64 CSA we need 4Kbytes. */
__CSA_SIZE_REQUEST = 4K;

/* the pcp memory description */
__PCP_CODE_RAM_BEGIN = 0xF0060000;
__PCP_CODE_RAM_SIZE  = 0K;
__PCP_DATA_RAM_BEGIN = 0xF0050000;
__PCP_DATA_RAM_SIZE  = 0K;

__ISTACK_SIZE = 2K;
__USTACK_SIZE = 12K;
__HEAP_MIN = 1K;

MEMORY
{

  int_cflash    (rx!p): org = 0x70020000, len = 0x1C000

  /*** !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ***/
  /*** NEVER CLEAR the BMI HEADER 1 at 0xa0020000--0xa0027fff  ; PS0, S8 (BMI Header) ***/
  /*** !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ***/

  /*int_cflash_section_3    (rx!p): org = 0xA0080000, len = 0x7F800
  RESERVED_DATA (rx!p): org = 0xA00FF800, len = 0x800
  OFFLINE_DATA  (rx!p): org = 0xA0100000, len = 0x800
  CAL_DATA      (rx!p): org = 0xA0100800, len = 0xFF800
  */
  int_pflash_cs   (rx!p): org = 0x10020000, len = 0x1C000

  int_dflash_0 (r!p): org = 0xAE000000, len = 1024k

  dspr0 (w!xp): org = 0x70000000, len = 0x20000
  pspr0 (w!xp): org = 0x70100000, len = 0x10000

  dspr1 (w!xp): org = 0x60000000, len = 120K
  pspr1 (rx!p): org = 0x60100000, len = 64K
  
  dsprcs (w!xp): org = 0x10000000, len = 0x20000
  psprcs (rx!p): org = 0x10100000, len = 0x10000


  dlmu0 (w!xp): org = 0xB0000000, len = 0x80000
  dlmu1 (w!xp): org = 0xB0080000, len = 0x80000
  dlmu2 (w!xp): org = 0xB0100000, len = 0x60000
  dlmu3 (w!xp): org = 0xB0160000, len = 0x40000
  dlmu4 (w!xp): org = 0xB01A0000, len = 0x40000
  dlmu5 (w!xp): org = 0xB01E0000, len = 0x40000

  /***  emulation RAM: B030 0000h - B06F FFFFh (4096kB) ***
  //*** LMURAM section ***/

  LMURAM (w!xp): org = 0xB0300000, len = 0x3F8000
  LMURAM_CODECHECK_PATTERN (w!xp): org = 0xB06F8000, len = 32K
  
  
}

SECTIONS
{
  /********************************* START ************************************/

  .Boot_Mode_Header_0 0x70100000 :
  {
    KEEP(*(.UCB_RTC_BMHD0))
 /*   KEEP(*(.BMHD0)) */
  } > int_cflash
  /* } > int_cflash =0 // for a fillByte of 0x0 or =0xFFFFFFFF for the corresponding fillByte */

  .Boot_Mode_Header_CS 0x10100000 :
  {
    KEEP(*(.UCB_CS_BMHD0))
  } > int_pflash_cs

  .Boot_Mode_Header_2 0x7010FFE0 :
  {
    KEEP(*(.BMHD2))
  } > int_cflash

  /*.Boot_Mode_Startup_2 0x70110000 :
  {
    KEEP(*(.special_Startup_BMHD2))
  } > int_cflash */

  .init.startup 0x70020020 :
  {
    KEEP (*(.init.startup.code))
    KEEP(*(.special_Startup_BMHD2))
    . = ALIGN(8);
  } > int_cflash

  .startup ALIGN(8) :
  {
    KEEP  (*(.startup.code))
    KEEP  (*(.cstart.code))
    KEEP  (*(.c_init.code))
    KEEP  (*(.trap_handler.code))
  } >  int_cflash

  /*.traptable : ALIGN(0x100) FLAGS(ax) */
  .traptable 0x70101000 :  FLAGS(ax)
  {
      __TRAP_TABLE = ((. + 0xFF) & ~ 0xFF);  /* for the trap table a 0x100 alignment is necessary */
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    ATTENTION: THIS ALIGNMENT IS RELATIVE TO THE SECTION INITIAL PLACEMENT. IF the section starts at 0xA00000e0 it will give alignments like 0xA00001e0.
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

  } > pspr0 AT> int_cflash

  /*
   * The interrupt table is placed first so that, when it is at its maximum size,
   * it is 8K aligned. Otherwise the wrong vectors get selected...
   */
    .inttable : ALIGN(0x2000) FLAGS(ax)
    {
      __INT_TABLE = (. + 0x1FFF) & ~ 0x1FFF;   /* for the interrupt table a 8kB alignment */
      . = __INT_TABLE;
      *(.ISR0.inttab)
      *(.ISR1.inttab)
      *(.ISR2.inttab)
      *(.inttab)
    } > pspr0 AT> int_cflash

  .init  : ALIGN(8)
  {
    *(.init)
    *(.fini)
  } > pspr0 AT> int_cflash

  .CAL_PARAM : ALIGN(4)
  {
    *(.C_P_27)  /* Should be inside of the next section section (0xD0000--0xD8000) */
    *(.rodata.CAL_PARAM)
    *(.CAL_PARAM)
    *(.rodata.*.CAL_PARAM)
    *(.ETK_Code_checkMemClass)
    *(.trigger_parameters);
    *(.handshake_parameters);
    *(.coldstart_parameters);
    *(.rs232_parameters);
    *(.measure_parameters);
    *(.measure_parameters_CPU0);
    *(.VIRTUAL_DATA)
  } > int_cflash
  
  
  /*****************************************************************************/
  /* CPUcs code */
  .init.startup_CPUCS 0x10020000 :
  {
    KEEP  (*(.init.startup_cpucs.code))
    . = ALIGN(8);
  } > int_pflash_cs

  .startup_CPUcs ALIGN(8) :
  {
    KEEP  (*(.cstart_cpucs.code))
  } > int_pflash_cs


  /********************************* DATA *************************************/

  .TraceTriggers  0x70000008: ALIGN(4)
  {
    *(.ETK_Trace_TriggerByValue);
  } > dspr0

  .data : ALIGN(8)
  {
    DATA_BASE = . ;
    *(.data)
    *(.data.*)
    *(.gnu.linkonce.d.*)

    *(.IRAM);
    *(.ETK_RAM); /*no external ETK RAM available, so section is located in internal RAM */
    *(.WriteToRAM);
    *(.ETK_TRIGGER_ID);

    *(.dT_measurement)
    *(.measure_variables)
    *(.measure_dummies)
    *(.measureCalibParam)
    *(.VERSION_XYZ)

    SORT(CONSTRUCTORS)
    . = ALIGN(8) ;
    DATA_END = . ;
  } > dspr0 AT> int_cflash

  .dataNoLoad (NOLOAD) : ALIGN(4)
  {

    *(.ETK_ColdStart_MemClass);
    *(.ETK_DisTab_MemClass);
    *(.ETK_BYPASS_RET_MemClass);
    *(.ETK_Presence_checkMemClass); /* Used by the Parallel ETK, should be in flash ? */
    *(.code_checkMemClassRam);

    . = ALIGN(8) ;
    *(.IRAM_CPU0toDSPR0_measure_variables_performance_bss_8BAlign)
    *(.IRAM_CPU0toDSPR0_measure_variables_performance_bss)
    *(.IRAM_CPU0_measure_variables_performance_bss_1BAlign)
    . = ALIGN(4) ;

    *(.ETK_DAQChnl15_MemClass);
    *(.ETK_DisTab15_AddrPtr_MemClass);
    *(.ETK_DisTab15_MemClass);
    *(.ETK_DisTab16_MemClass);

    *(.CalWakeUpMemClassRam);
    *(.NVRAM);
    *(.ETK_OCTTable_MemClass);

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
  } > dspr0 AT> int_cflash

  .sdata  : ALIGN(4)
  {
    SDATA_BASE = . ;
    PROVIDE(__sdata_start = .);
    PROVIDE(_SMALL_DATA_ = . + 0x8000);  /* address definition for initializing A0 for short addressing mode (backward addressing ex:A0-0x66f0)*/

    *(.sdata)
    *(.sdata.*)
    *(.gnu.linkonce.s.*)
  } > dspr0 AT> int_cflash

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

    *(.ERAM); /*no external RAM available, so section is located in internal RAM */
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

  } > dspr0

  .bss_CPU1 (NOLOAD) : ALIGN(4)
  {
    BSS_CPU1_BASE = . ;
    *(.measure_values_CPU1);

    . = ALIGN(8) ;
    *(.ERAM_CPU0toDSPR1_measure_variables_performance_bss_8BAlign)
    *(.ERAM_CPU0toDSPR1_measure_variables_performance_bss)
    . = ALIGN(4) ;
    . = ALIGN(64) ;

    *(.ETK_DAQChnl_MemClass_CPU1);

  } > dspr1

  .data_CPU1 : ALIGN(64)
  {
    *(.measure_dummies_CPU1);
    *(.IRAM_CPU1);

  } > dspr1 AT> int_cflash

  .dataNoLoad_CPU1  (NOLOAD) : ALIGN(8)
  {
    *(.ETK_DisTab_MemClass_CPU1);

  } > dspr1


  .stack_csa_CPU1  (NOLOAD) : ALIGN(64)
  {
    __CSA_CPU1_BEGIN = . ;
    . += __CSA_SIZE_REQUEST;
    . = ALIGN(64);
    __CSA_CPU1_END = .;

    __ISTACK_CPU1_BEGIN = . ;
    . += __ISTACK_SIZE ;
    __ISTACK_CPU1_END = . ;

    . = ALIGN(64) ;

    __USTACK_CPU1_BEGIN = . ;
    . += __USTACK_SIZE ;
    __USTACK_CPU1_END = . ;


    /* The following defines are used to check if we have enough RAM for the stacks. */
    __ISTACK_CPU1 = __ISTACK_CPU1_END ;
    __USTACK_CPU1 = __USTACK_CPU1_END ;
    __HEAP_CPU1 = __USTACK_CPU1 ;
    /*__HEAP_CPU1_END = __RAM_CPU1_END ;*/

    __CSA_SIZE_CPU1 = __CSA_CPU1_END - __CSA_CPU1_BEGIN;
    __CSA_BEGIN_CPU1 = __CSA_CPU1_BEGIN;
    __CSA_END_CPU1 = __CSA_CPU1_END;

  } > dspr1



  /* EMU RAM Codecheck - Reserved EMU RAM section for ECU use */
  .LMURAM_CODCHECK  (NOLOAD) : ALIGN(4)
  {

    *(.ETK_DataFreeze_Mailbox);
    *(.code_checkMemClassEMURAM);
    *(.ETK_PageSwitch_MemClass);
    *(.ETK_OMDTable_MemClass);

  } > LMURAM_CODECHECK_PATTERN


  /********************************* CODE *************************************/
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
    . = ALIGN(8);
  } > int_cflash

  .rodata   : ALIGN(4)
  {
    *(.rodata)
    *(.rodata.*)
    *(.RoData_Core0)
    *(.gnu.linkonce.r.*)
    *(.rodata1)
    *(.toc)
    *(.jcr)
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
    LONG(LOADADDR(.traptable));    LONG(0 + ADDR(.traptable));    LONG(SIZEOF(.traptable));
    LONG(LOADADDR(.inttable));    LONG(0 + ADDR(.inttable));    LONG(SIZEOF(.inttable));
    LONG(LOADADDR(.init));    LONG(0 + ADDR(.init));    LONG(SIZEOF(.init));
    LONG(LOADADDR(.data));    LONG(0 + ADDR(.data));    LONG(SIZEOF(.data));
    LONG(LOADADDR(.sdata));   LONG(0 + ADDR(.sdata));   LONG(SIZEOF(.sdata));
    LONG(LOADADDR(.zdata));   LONG(0 + ADDR(.zdata));   LONG(SIZEOF(.zdata));
    LONG(-1);                 LONG(-1);                 LONG(-1);

    /* CPU1 tables */
    . = ALIGN(4) ;
    PROVIDE(__clear_table_CPU1 = .) ;
    LONG(0 + ADDR(.bss_CPU1));     LONG(SIZEOF(.bss_CPU1));
    LONG(-1);                 LONG(-1);
    PROVIDE(__copy_table_CPU1 = .) ;
    LONG(LOADADDR(.data_CPU1));    LONG(0 + ADDR(.data_CPU1));    LONG(SIZEOF(.data_CPU1));
    LONG(-1);                 LONG(-1);                 LONG(-1);

  } > int_cflash

  .sdata2  : ALIGN(4)
  {
    *(.sdata.rodata)
    *(.sdata.rodata.*)
    . = ALIGN(8);
  } > int_cflash
  .eh_frame  : ALIGN(4)
  {
    *(.gcc_except_table)
    __EH_FRAME_BEGIN__ = . ;
    KEEP (*(.eh_frame))
    __EH_FRAME_END__ = . ;
    . = ALIGN(8);
  } > int_cflash
  .ctors : ALIGN(4)
  {
    __CTOR_LIST__ = . ;
    LONG((__CTOR_END__ - __CTOR_LIST__) / 4 - 2);
    *(.ctors)
    LONG(0) ;
    __CTOR_END__ = . ;
    . = ALIGN(8);
  } > int_cflash
  .dtors : ALIGN(4)
  {
    __DTOR_LIST__ = . ;
    LONG((__DTOR_END__ - __DTOR_LIST__) / 4 - 2);
    *(.dtors)
    LONG(0) ;
    __DTOR_END__ = . ;
    . = ALIGN(8);
  } > int_cflash




  /********************************* CAL PARAM ********************************/


    /* FLASH KEY Pattern */
  .flash_key  :
  {
    *(.FLASH_KEY)
  } > int_cflash




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

