/*
################################################################################
#                                                                              #
#    ETK driver example                          |   ETAS GmbH                 #
#                                                |   Stuttgart Feuerbach       #
#    For Demonstration Purpose Only              |   All rights reserved       #
#    sample driver implementation                |   Alle Rechte vorbehalten   #
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
/*  COPYRIGHT (c) ETAS GmbH 2014-2023                                         */
/*  All Rights Reserved                                                       */
/******************************************************************************/


#ifndef _ETK_INTEGRATION_CFG_H
#define _ETK_INTEGRATION_CFG_H

#define ETKTYPE_ETKS20_0
#define ETKCLASS_ETK
#define ETKACCESS_SERIAL

// Defines the target for which the project will be compiled. Can be TC49x or TC4Dx.
#include "Target_Select.h"           /* Target selection file */
#include "../../a_std_type.h"        /* for the standard types */

#if (defined CPUTYPE_TC49X)
  #include "../../../sfr/TC49X_ts_MCSFR.sfr"    
#elif (defined CPUTYPE_TC4DX) || (defined CPUTYPE_TC4DX_PD)
  #include "../../../sfr/TC4DX_ts_MCSFR.sfr"
#elif (defined CPUTYPE_TC49X_N) 
  #include "../../../sfr/TC49x_N_MCSFR.sfr"
#elif (defined CPUTYPE_TC48X) 
  #include "../../../sfr/TC48x_ts_MCSFR.sfr"
#elif (defined CPUTYPE_TC46X)
  #include "../../../sfr/TC46x_ts_MCSFR.sfr"
#elif (defined CPUTYPE_TC4ZX)
  #include "../../../sfr/TC4Zx_ts_MCSFR.sfr"             
#else
  #error "Undefined sfr file !"
#endif

#define DISTAB_SUPPORTED              // Definition whether measurement via Distab 17 is supported
#ifndef CPUTYPE_TC48X                 // For TC48x is only SinglePage supported and no Trace
  #define TRACE_SUPPORTED               // Definition whether measurement via Trace is actual not supported                 
  #define CALIBRATION_SUPPORTED      // Definition whether calibration is supported
#endif
#define ACC_SUPPORTED              // Definition whether advanced code check is supported
#define COLDSTART_SUPPORTED        // Definition whether cold start is supported
// #define DATAFREEZE_SUPPORTED       // Definition whether Data Freeze is supported


/* ---------------------------------------------------------------------------*/
/* Note: This File is intended to be adapted by the User !                    */
/* This file is used to define the necessary Information                      */
/* to configure measurements and calibration with a specific (X)ETK           */
/* ---------------------------------------------------------------------------*/
/** Some definitions:
MC_WAIT -> Measurement and Calibration Wait (Handshake / Coldstart)
RP_WAIT -> Rapid Prototyping Wait (Handshake)
HDC -> Hardware Definition Code (Will be used to program the logic of the ETK/XETK)
ES device -> Interface Module between the ETK and the PC

Page switching definitions (calibration):
RPREQ -> Reference Page REQuest
RPACK -> Reference Page configuration ACKnowledge
RPERR -> Reference Page configuration ERRor
WPREQ -> Working Page REQuest
WPACK -> Working Page configuration ACKnowledge
WPERR -> Working Page configuration ERRor
MOREQ -> Modify Overlay REQuest
MOACK -> Modify Overlay memory layout ACKnowledge (Page switch)
MOERR -> Modify Overlay configuration ERRor
*/

/* ---------------------------------------------------------------------------*/
/* WatchDogTimer(s) CPU0 and SYS                                              */
/* ---------------------------------------------------------------------------*/
// #define ENABLE_WATCHDOGS

#ifndef ENABLE_WATCHDOGS
  #warning "--- Watchdog for CPU0 and SYS is disabled ---"
#endif


/* ---------------------------------------------------------------------------*/
/* RS232 and Debug Settings                                            */
/* ---------------------------------------------------------------------------*/

/** It is possible to remove all the RS232 messages from the project by commenting this define.
If the RS232 Debug message define is set different informations will be displayed during the program runtime through the
ASCLIN0 output. The RS232 baud rate is set to 115200 bps 8 bit, no parity, 1 stop bit by default. The Debug Level can
then be set with a terminal by writing the desired debug level value. The standard debug level is 1 by default, but it
can be set between 0 and 4. At the level 4 the page switching function may produce an error with INCA as it slows
greatly down the function execution speed. This can be prevented by increasing the timeout time defined in the A2L for
the PAGE_SWITCH_METHOD. */
// #define RS232_DEBUG_LOGGING

/** It is possible to set the Debug Level between 1 and 4. 1 is the default setting displaying the minimum amount of
debugging information through the RS232 port. The value can be set to 3 to display more debug messages during the
debugging phase or to 4 to force the display of all available debug information. Setting the display level to a higher
value will cause more latency and slow down the program. */
// #define RS232_DEBUG_LEVEL 1

/** The RS232 baud rate is set to 115200 bps 8 bit, no parity, 1 stop bit by default.
The following define can set it to 19200bps instead. */
// #define RS232_USE_19200_BPS_BAUDRATE

// Debug Variables
// #define DEBUG_VARIABLES 1 // 0 as Value for OFF 1 as Value for ON


/* ---------------------------------------------------------------------------*/
/* ---------------------------------------------------------------------------*/
/* Handshake Settings                                            */
/* ---------------------------------------------------------------------------*/


// Tricore Aurix specific - New Handshake method using the COMDATA register
#define ETK_COMDATA_HANDSHAKE

/**
 * Enables the use of the automatic measurement without an XCP Master in the Network.
 * Only Measurement by Distab17 is supported, no calibration etc.
 *
 * The measurement configuration for "Automatic Measurement" meaning Raster and DAQ configutation is stored
 * in the ECU Flash. An additionally First Level Info Structure is added to the ECU Info Structure. Among other
 * things the First Level Info Structure holds pointers to the Config structures in the ECU Flash.
 * The ETK can read all configuration and then setup and start the measurement by itself.
 */
// #define LOGGER_USE_CASE


/* ---------------------------------------------------------------------------*/
/* ---------------------------------------------------------------------------*/
/* Trace measurement Settings */
/* ---------------------------------------------------------------------------*/
// #define TRACE_NOT_SUPPORTED // Definition whether Trace measurement is not supported - actually only supported for TraceToPin with FETK-T4
/** Enables the Trace triggers to support Trace measurements.
  Trace triggers can either be set statically by a periodic function or dynamically by supplementary Distab.
  The first EMEM tile is reserved and set to TRACE mode with the TC27x to support the FETKT1 Trace functionality.
  The TC3xx family supports a different Trace setup from the TC2xx family.
  It is not possible to use at the same time a bus (BOB) and a processor (POB) block for the same memory with the
  TC3xx.*/
#ifdef TRACE_NOT_SUPPORTED
/* The 3xxX PD devices do not support Trace (MCDS cannot be used). */
#else
  #ifdef TRACE_SUPPORTED
  #define TRACE_SUPPORT
  #endif
#endif

/* ---------------------------------------------------------------------------*/
/* ---------------------------------------------------------------------------*/
/* CPU specific Settings                                            */
/* ---------------------------------------------------------------------------*/

// Distinguish between Motorola / Intel Type Memory Usage
// #define PROCESSOR_MEM_TYPE_MOTOROLA  // Motorola Type Memory Usage
#define PROCESSOR_MEM_TYPE_INTEL // Intel Type Memory Usage

/** If this define is set the safety and CPU0 endinit protections will be disabled, and no interrupt disabling will be
  used for page switching. It is possible to not use the endinit protection associated with the write access of some of
  the registers used for the calibration. However this will set the watchdog timers in timeout mode. This means that if
  the endinit protection is disabled, the watchdog timers will be enabled. A workaround to this is to disable the
  watchdogs if the OCDS debug mode is active. It is possible to disable the endinit watchdogs by enabling the OCDS mode
  and by setting the CBS_OCNTRL register to suspend the watchdog timers if OCDS is enabled. This code can currently be
  found in the _call_init() function in the endinit.c file. By not using the endinit protection, it is possible to
  execute a page switching process without the need to disable the interrupts. The interrupt disabling is only required
  to prevent an endinit watchdog timeout when the protection is temporarily disabled.

  As a side note, it is also possible to execute the page switching without the current workaround and without disabling
  the interrupts, if it is ensured that the endinit watchdog will not hit a timeout when the protection is disabled.
  There are two ways to do this.
  First the page switching task can be given a very high priority which will ensure that it will be executed within the
  time frame before a timeout. The second possibility is to create an OS timer interrupt triggered task with a high
  priority which will update the timer value of the watchdog to prevent a timeout. In this case this task could check if
  the endinit protection is disabled, before trying to update the watchdog timer, or the timer interrupt specific to
  this task could be enabled when a function disables the ENDINIT protection, and later disabled once the ENDINIT
  protecion is enabled again.*/
// #define ENDINIT_PROTECTION_DISABLED


/** Timer conversion formula used for the  BASE_Check_Timeout function. (number of ticks for 1 �sec - 100 ticks/�s for
 * 100 MHz) */
#define TICKS_TO_USEC 100

/* ---------------------------------------------------------------------------*/
/* ---------------------------------------------------------------------------*/
/* Coldstart Settings                                            */
/* ---------------------------------------------------------------------------*/

/** Adds more variables in order to measure the coldstart execution times.
This is a debug feature which is not needed for the core coldstart functionality.*/
// #define COLDSTART_EXECUTION_TIME

/** Saves the initial coldstart pattern before the coldstart execution and after for debugging purposes. */
// #define COLDSTART_DEBUG_PATTERN

/* ---------------------------------------------------------------------------*/
/* ---------------------------------------------------------------------------*/
/* Data freeze Settings                                            */
/* ---------------------------------------------------------------------------*/

/** Enables the data freeze SAFETY mailbox functionality */
#ifdef DATAFREEZE_SUPPORTED
  #define DATAFREEZE_SAFETY_MAILBOX
#endif

/** Enables the data freeze STARTUP mailbox functionality */
#ifdef DATAFREEZE_SUPPORTED
  #define DATAFREEZE_STARTUP_MAILBOX
#endif

/** Defines the validate pattern and its verification in the code */
/** !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
/** The current example code will trap if the validate pattern is missing as the memory can not be read if it is in a
 * cleared state without causing a trap. */
// #define DATAFREEZE_USE_VALIDATE_PATTERN


/* ---------------------------------------------------------------------------*/
/* ---------------------------------------------------------------------------*/
/* Calibration Settings                                            */
/* ---------------------------------------------------------------------------*/


/* The overlay RAM has been replaced with LMU ram and does not need a tile configuration. */


/** Define Used Overlay Memory Layout Descriptor - OMD
By removing this define it is possible to compile the project without any page switching support. */
#define OMD_TC2XX // OMD table used for the new AURIX generation of TC2xx Tricore devices...


// Define the number of calibration handles present on the target for the OMD table
#ifdef OMD_TC2XX

  /* If this define is set the code will check if the advanced code check pattern for WP in the EMU ram was written by the
    ETK before doing a page switch. When set the following PAGESWITCH_DEPENDING_ON_CODECHECK macro value will be used to
    define if this verification can block or not a page switching. */
  #define CODECHECK_WP_BEFORE_PAGESWITCH

  /* Advanced Code check used to check if the ECU pattern was copied by the ETK in the A2L specified RAM region
  Doesn't block the page switching process if it fails with PAGESWITCH_DEPENDING_ON_CODECHECK set to 0. Blocks the process
  if PAGESWITCH_DEPENDING_ON_CODECHECK is set to 1.  */
  #define PAGESWITCH_DEPENDING_ON_CODECHECK 0

  /** OMD version 2 is used for LERTv3 dynamic emulation. */
  #define IMPLEMENTED_OMD_CID 2

  /* The implemented version of the OMD table. - Compatible extensions for an existing CID. */
  #define IMPLEMENTED_OMD_VERSION 0

  /** The next two defines will decide how many calibration handles will be used by the ECU and how the OMD checks and
  tile configurations are done. It also impacts on the EMEM_RAM_ECC_Initialize function and thus decides what part of the
  EMEM memory will be initialized. */
  /* The maximum number of supported calibration handles by the target ECU. */
  #define NUMBER_OMD_MAX_CALIB_HANDLES 32

  #define CALIB_HANDLE_SIZE 0x20000 



  // The size for a structure to hold all the bit values for the maximum number of calibration handles. Has to round up
  // the value. For example 33 calib handles require 2 4bytes values.
  #define NUMBER_4BYTE_VALUES_PER_CALIB_DEFINITION \
    (NUMBER_OMD_MAX_CALIB_HANDLES / 32 + (((NUMBER_OMD_MAX_CALIB_HANDLES % 32) + 31) / 32))


  /* Tricore Specific Defines */
  
  #if (defined CPUCLASS_TC2D5 || defined CPUCLASS_TC3XX || defined CPUCLASS_TC4XX)
    /* Tricore Specific - The OMASK value to be written in the OMASK register used to define the calibration handle size.
    The OMASK value is used for address comparison for the RABR register. The mask is calculated in such a way that values
    after the block size won't be used for address comparison and translation. It consists of two values, one part of the
    mask which is "alway" true and called ONE and the user defined part for the OMASK, called OMASK. ONE has the following
    value : 0x0ffe0000 For a 16kByte handle the OMASK value would be 0x0001c000. This would give us the value (0x0ffe0000 |
    0x0001c000) = 0x0fffc000 to write in the OMASK register. The OMASK value determines the block size and also the bits
    used for address comparison in the RABR and OTAR registers. The minimum block size is 32 byte and the maximum is
    128kByte. Different OMASK values: 128 kBytes  -> 0x00000000 64 kBytes   -> 0x00010000 32 kBytes   -> 0x00018000 16
    kBytes   -> 0x0001C000 8  kBytes   -> 0x0001E000 4  kBytes   -> 0x0001F000 2  kBytes   -> 0x0001F800
        ..........
    32 Bytes   -> 0x0001FFE0
    */
    #if (CALIB_HANDLE_SIZE == 0x200)
      #define OMASK_TRICORE_CALIB_HANDLE_CONFIG 0x0ffffe00 /* for 512 Byte handle size */
    #elif (CALIB_HANDLE_SIZE == 0x400)
      #define OMASK_TRICORE_CALIB_HANDLE_CONFIG 0x0ffffc00 /* for 1kByte handle size */
    #elif (CALIB_HANDLE_SIZE == 0x800)
      #define OMASK_TRICORE_CALIB_HANDLE_CONFIG 0x0ffff800 /* for 2kByte handle size */
    #elif (CALIB_HANDLE_SIZE == 0x1000)
      #define OMASK_TRICORE_CALIB_HANDLE_CONFIG 0x0ffff000 /* for 4kByte handle size */
    #elif (CALIB_HANDLE_SIZE == 0x2000)
      #define OMASK_TRICORE_CALIB_HANDLE_CONFIG 0x0fffe000 /* for 8kByte handle size */
    #elif (CALIB_HANDLE_SIZE == 0x4000)
      #define OMASK_TRICORE_CALIB_HANDLE_CONFIG 0x0fffc000 /* for 16kByte handle size */
    #elif (CALIB_HANDLE_SIZE == 0x8000)
      #define OMASK_TRICORE_CALIB_HANDLE_CONFIG 0x0fff8000 /* for 32kByte handle size */
    #elif (CALIB_HANDLE_SIZE == 0x10000)
      #define OMASK_TRICORE_CALIB_HANDLE_CONFIG 0x0fff0000 /* for 64kByte handle size */
    #elif (CALIB_HANDLE_SIZE == 0x20000)
      #define OMASK_TRICORE_CALIB_HANDLE_CONFIG 0x0ffe0000 /* for 128kByte handle size */
    #else
      #error "Not defined OMASK calibration handle size configuration selected."
    #endif

  #endif // #if (defined CPUCLASS_TC2D5 || defined CPUCLASS_TC3XX || defined CPUCLASS_TC4XX)

#endif // #ifdef OMD_TC2XX


/* ---------------------------------------------------------------------------*/
/* ---------------------------------------------------------------------------*/
/* Compiler Settings                                            */
/* ---------------------------------------------------------------------------*/

/* ---------------------------------------------------------------------------*/
/* Use USE_FAR_DEF if a far Address modifier is needed for the target !       */
/* Use FAR_DEF to define the target specific far definition..                 */
/* ---------------------------------------------------------------------------*/
// #define USE_FAR_DEF  // Enable when FAR_DEF define shall be used
//  -> Disable if not needed or compiler does not support
//  empty define declaration
#define FAR_DEF far
// #define  FAR_DEF  __far   // Definition for Tricore if needed


/* ---------------------------------------------------------------------------*/
/* Use USE_VOLATILE_DEF if compiler can handle volatile const declarations!   */
/* Use VOLATILE_DEF to define the volatile keyword                            */
/* ---------------------------------------------------------------------------*/
#define USE_VOLATILE_DEF // Enable when FAR_DEF define shall be used
                         // -> Disable if not needed or compiler does not support
                         // empty define declaration
#define VOLATILE_DEF volatile


/* ---------------------------------------------------------------------------*/
/* Use USE_DISTAB_CONST_DEF if Distab is in ETK Data Area and not in RAM      */
/* (Serial ETK / Parallel ETK with RAM extention)                             */
/* ---------------------------------------------------------------------------*/
// #define USE_DISTAB_CONST_DEF // Enable when DISTAB_CONST_DEF define shall be used
//  -> Disable if not needed or compiler does not support
//  empty define declaration
#define DISTAB_CONST_DEF const

/* ---------------------------------------------------------------------------*/
/* Serial / Parallel ETK                                                      */
/* ---------------------------------------------------------------------------*/
#define TRG_SERIAL_ETK

/* ---------------------------------------------------------------------------*/
/* Definition of Distab 13 Raster Numbers                                     */
/* How many Measurement and Bypass Rasters are used                           */
/* Number must be:                                                            */
/* x <= 0 for bypass raster                                                  */
/* x <= 3 for measurement raster                                             */
/*                                                                            */
/* ---------------------------------------------------------------------------*/
#define NUMBER_MEASUREMENT_RASTER 3
#define NUMBER_BYPASS_RASTER      0


#endif // #ifndef _ETK_INTEGRATION_CFG_H
