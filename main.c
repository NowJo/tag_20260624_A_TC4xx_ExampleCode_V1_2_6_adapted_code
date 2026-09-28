/* /BEGIN COPYRIGHT_HEADER                                                     */
/*                                                                             */
/* Added by copyright.py V1.3.2                                                */
/*                                                                             */
/* =========================================================================== */
/* Copyright and Legal Disclaimer:                                             */
/* This Code example was provided by ETAS GmbH, Stuttgart                      */
/*                                                                             */
/* ETAS will not be held reliable for any usage of this code,                  */
/* this code is provided as example code only, and not tested and released     */
/* for production code.                                                        */
/* ETAS will not guarantee any functional part of this code in any             */
/* environment together with any ETAS tools. ETAS will not guarantee that      */
/* this code works together with any future versions of ETAS tools.            */
/*                                                                             */
/* ETAS will not guarantee that this code is free of rights of third parties.  */
/*                                                                             */
/* You are hereby granted the right to use his code as a example for your own  */
/* ECU implementation together with ETAS Tools. No licenses are granted by     */
/* implication or otherwise under any patents or trademarks of ETAS GmbH.      */
/* This software is provided on an "AS IS" basis and without warranty.         */
/*                                                                             */
/* You are not allowed to give these code to third parties without             */
/* the written permission of ETAS GmbH.                                        */
/*                                                                             */
/* To the maximum extent permitted by applicable law,                          */
/* ETAS GmbH DISCLAIMS ALL WARRANTIES WHETHER EXPRESS OR IMPLIED,              */
/* INCLUDING IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A            */
/* PARTICULAR PURPOSE AND ANY WARRANTY AGAINST INFRINGEMENT WITH REGARD        */
/* TO THE SOFTWARE (INCLUDING ANY MODIFIED VERSIONS THEREOF)                   */
/* AND ANY ACCOMPANYING WRITTEN MATERIALS.                                     */
/*                                                                             */
/*                                                                             */
/* To the maximum extent permitted by applicable law,                          */
/* IN NO EVENT SHALL ETAS BE LIABLE FOR ANY                                    */
/* DAMAGES WHATSOEVER (INCLUDING WITHOUT LIMITATION, DAMAGES FOR LOSS OF       */
/* BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION,      */
/* OR OTHER PECUNIARY LOSS)                                                    */
/* ARISING OF THE USE OR INABILITY TO USE THE SOFTWARE.                        */
/*                                                                             */
/*                                                                             */
/* ETAS GmbH assumes no responsibility for the maintenance                     */
/* and support of this software                                                */
/*                                                                             */
/*                                                                             */
/*  COPYRIGHT (c) ETAS GmbH 2025                                               */
/*  All Rights Reserved                                                        */
/* =========================================================================== */
/*                                                                             */
/* =========================================================================== */
/*                                                                             */
/*     ETK Drivers Example                         |   ETAS GmbH               */
/*     For Demonstration Purpose Only              |   Stuttgart Feuerbach     */
/*     Sample Driver Implementation                |   All rights reserved     */
/*                                                                             */
/* =========================================================================== */
/*                                                                             */
/* /END COPYRIGHT_HEADER                                                       */

#include "main.h"
#include "Base.h"
// #include "ETK/HS/GetTime.h"
// #include "ETAS_ETK\ETAS_API.h"
#include "GPIO.h"
#include "ISR.h"
#include "Init.h"
#include "calibrationParam.h"
#include "countersPerf_CPU0.h"
#include "measurements.h"
// #include "rs232.h"
#include "version_defs.h"

#include "ETK/HS/ETK_SER_Handshake.h"
#ifdef CALIBRATION_SUPPORTED
  #include "ETK/CAL/ETK_SER_Page_Ctrl_Fct.h"
#endif
#ifdef ACC_SUPPORTED
  #include "ETK/ACC/ETK_CodeCheck.h"
#endif
#ifdef DISTAB_SUPPORTED
  #include "ETK/D17/Distab17_Processes.h"
#endif
#ifdef TRACE_SUPPORT
  #include "ETK/TRACE/Trace_Fct.h"
#endif
#ifdef DATAFREEZE_SUPPORT
  #include "ETK/DF/ETK_SER_DataFreeze.h"
#endif
#ifdef D17_ENABLE_BYPASS_SUPPORT
  #include "SBB_Task.h"
#endif

// flash key pattern
// used to avoid inconsistancy between ECU projects and prof flow
// startup value is 0xCCCC 0001
// the 16 most significant bits are used for identification
// after every memory layout change the value has to be increased from e.g.
// 0xCCCC 0001 -> 0xCCCC 0002

const uint32 FlashKey __attribute__((section(".FLASH_KEY"), aligned(4))) = FLASHKEY_PATTERN;

__attribute__((section(".VERSION_XYZ"), aligned(4))) volatile uint32 version_X, version_Y, version_Z;

__attribute__((section(".dT_measurement"), aligned(4))) volatile struct DT_MEASUREMENT dT_measurement = {0, 0, 0, 0, 0, 0};

void Task_1_Impl(void);
void Task_2_Impl(void);
void Task_3_Impl(void);
void Task_4_Impl(void);
void Task_5_Impl(void);
#if ((defined CPUCLASS_TC4XX) && (defined D17_ENABLE_BYPASS_SUPPORT))
void Task_6_Impl(void);
#endif
void Task_Alive_Impl(void);

__attribute__((section(".measure_parameters"), aligned(4)))
// config for 100 MHz systick
volatile const struct TaskParameters task_param[TASK_NUM] = {
    {/* Period in ticks */ 100000,
     /* Body */ Task_1_Impl                                     }  /*100000 ticks / 100 ticks/usec = 1 ms */
    ,
    {/* Period in ticks */ 500000,    /* Body */ Task_2_Impl    }  /*5 ms*/
    ,
    {/* Period in ticks */ 1000000,   /* Body */ Task_3_Impl    }  /*10 ms*/
    ,
    {/* Period in ticks */ 1000000,   /* Body */ Task_4_Impl    }  /*10 ms*/
    ,
#if (defined CPUCLASS_TC4XX)
    {/* Period in ticks */ 20000000,  /* Body */ Task_5_Impl    }  /*200 ms*/
    ,
  #if (defined D17_ENABLE_BYPASS_SUPPORT)
    {/* Period in ticks */ 10000000,  /* Body */ Task_6_Impl    }  /*100 ms*/
    ,
  #endif
#elif (defined(CPUCLASS_TC3XX) && defined(D17_ENABLE_BYPASS_SUPPORT))
    {/* Period in ticks */ 10000000, /* Body */ Task_5_Impl} /*100 ms*/
    ,
#endif
    {/* Period in ticks */ 100000000, /* Body */ Task_Alive_Impl}  /*1 s*/
};

__attribute__((section(".measure_variables"), aligned(4)))
#if (defined(CPUCLASS_TC3XX))
  #if (defined(D17_ENABLE_BYPASS_SUPPORT))
struct TaskMeasurements task_measure[TASK_NUM] = {{0}, {0}, {0}, {0}, {0}, {0}};
  #else
struct TaskMeasurements task_measure[TASK_NUM] = {{0}, {0}, {0}, {0}, {0}};
  #endif
#elif (defined(CPUCLASS_TC4XX))
  #if (defined(D17_ENABLE_BYPASS_SUPPORT))
struct TaskMeasurements task_measure[TASK_NUM] = {{0}, {0}, {0}, {0}, {0}, {0}, {0}};
  #else
struct TaskMeasurements task_measure[TASK_NUM] = {{0}, {0}, {0}, {0}, {0}, {0}};
  #endif
#endif

// dummy variables for section IRAM
/* 256 x 8bit variable  */
__attribute__((section(".measure_dummies"), aligned(4))) volatile uint8 Dummy_Matrix_1Byte[256] = {
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08};

/* 256 x 16bit variable  */
__attribute__((section(".measure_dummies"), aligned(4))) volatile uint16 Dummy_Matrix_2Byte[256] = {
    0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616,
    0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616,
    0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616,
    0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616,
    0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616,
    0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616,
    0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616,
    0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616,
    0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616,
    0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616,
    0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616,
    0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616,
    0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616,
    0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616,
    0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616,
    0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616,
    0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616,
    0x1616};

/* 255 x 32bit variable  */
__attribute__((section(".measure_dummies"), aligned(4))) volatile uint32 Dummy_Matrix_4Byte[255] = {
    0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232,
    0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232,
    0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232,
    0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232,
    0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232,
    0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232,
    0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232,
    0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232,
    0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232,
    0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232,
    0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232,
    0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232,
    0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232,
    0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232,
    0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232,
    0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232,
    0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232,
    0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232,
    0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232,
    0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232,
    0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232,
    0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232,
    0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232,
    0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232,
    0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232,
    0x32323232, 0x32323232, 0x32323232, 0x32323232, 0x32323232};

/* 255 x 64bit double floating variable  */
__attribute__((section(".measure_dummies"), aligned(4))) volatile real64 Dummy_Matrix_8Byte[255] = {
    64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0,
    64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0,
    64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0,
    64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0,
    64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0,
    64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0,
    64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0,
    64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0,
    64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0,
    64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0,
    64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0,
    64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0,
    64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0};

volatile uint32 getRasterTimeStart, getDistabTimeStart, getTimeStop;

void Task_1_Impl(void)
{
  GPIO_ToggleLed(0);
  getRasterTimeStart = BASE_GetSystemTime(); // task 1 time get start time for INCA measurement

  Measure_counterTask1();
  cpu0CounterTask(0);                        // writes 8/16/32/64 bit counters in DSPR0, DSPR1 and
                                             // LMU RAM - virtual task 0 input
  getDistabTimeStart = BASE_GetSystemTime(); // Distab time get start time for INCA measurement

  // Distab 17 measurement for raster 1
  // 1st parameter -> Event number (starting from 1)
  // ETAS_Distab17_Process(1);
      #ifdef DISTAB_SUPPORTED
        Distab17_Process(1);
      #endif

      /* The Trace trigger is here set in a static way without using supplementary Distab.
      This is not needed if the Trace trigger is set dynamically by supplementary Distab. */
      #ifdef TRACE_SUPPORT
            TraceExecTriggerByValue(1); // Triggering the trigger 1 with Trace by value
      #endif
  getTimeStop = BASE_GetSystemTime(); // task 1 & Distab time get stop time for
                                      // INCA measurement
  // calculate dT of task 1 (raster 1)
  if (getTimeStop > getRasterTimeStart)
    dT_measurement.dT_Task_01 = getTimeStop - getRasterTimeStart;
  else
    dT_measurement.dT_Task_01 = getRasterTimeStart - getTimeStop;

  // calculate dT of Distab in task 1
  if (getTimeStop > getDistabTimeStart)
    dT_measurement.dT_Distab_01 = getTimeStop - getDistabTimeStart;
  else
    dT_measurement.dT_Distab_01 = getDistabTimeStart - getTimeStop;
}

void Task_2_Impl(void)
{
  GPIO_ToggleLed(1);
  getRasterTimeStart = BASE_GetSystemTime(); // task 2 time get start time for INCA measurement

  Measure_counterTask2();
  cpu0CounterTask(1);                        // writes 8/16/32/64 bit counters in DSPR0, DSPR1 and LMU
                                             // RAM - virtual task 1 input
  getDistabTimeStart = BASE_GetSystemTime(); // Distab time get start time for INCA measurement

  // Distab 17 measurement for raster 2
  // 1st parameter -> Event number (starting from 1)
  // ETAS_Distab17_Process(2);
      #ifdef DISTAB_SUPPORTED
        Distab17_Process(2);
      #endif

/* The Trace trigger is here set in a static way without using supplementary Distab.
This is not needed if the Trace trigger is set dynamically by supplementary Distab. */
#ifdef TRACE_SUPPORT
      TraceExecTriggerByValue(2); // Triggering the trigger 1 with Trace by value
#endif
  // calculate dT of task 2 (raster 2)
  if (getTimeStop > getRasterTimeStart)
    dT_measurement.dT_Task_02 = getTimeStop - getRasterTimeStart;
  else
    dT_measurement.dT_Task_02 = getRasterTimeStart - getTimeStop;

  // calculate dT of Distab in task 2
  if (getTimeStop > getDistabTimeStart)
    dT_measurement.dT_Distab_02 = getTimeStop - getDistabTimeStart;
  else
    dT_measurement.dT_Distab_02 = getDistabTimeStart - getTimeStop;
}

void Task_3_Impl(void)
{
  GPIO_ToggleLed(2);
  getRasterTimeStart = BASE_GetSystemTime(); // task 2 time get start time for INCA measurement

  Measure_counterTask3();
  cpu0CounterTask(2);                        // writes 8/16/32/64 bit counters in DSPR0, DSPR1 and LMU
                                             // RAM - virtual task 2 input
  cpu0CounterTask(3);                        // writes 8/16/32/64 bit counters in DSPR0, DSPR1 and LMU RAM -
  getDistabTimeStart = BASE_GetSystemTime(); // Distab time get start time for INCA measurement

  // virtual task 3 input Distab 17 measurement for raster 3
  // 1st parameter -> Event number (starting from 1)
  // ETAS_Distab17_Process(3);
      #ifdef DISTAB_SUPPORTED
        Distab17_Process(3);
      #endif

/* The Trace trigger is here set in a static way without using supplementary Distab.
This is not needed if the Trace trigger is set dynamically by supplementary Distab. */
#ifdef TRACE_SUPPORT
      TraceExecTriggerByValue(3); // Triggering the trigger 1 with Trace by value
#endif
#if (defined USE_EXCEPTION_DUMP)
  ETK_ExceptionDump_Proc();
#endif
  // calculate dT of task 3 (raster 3)
  if (getTimeStop > getRasterTimeStart)
    dT_measurement.dT_Task_03 = getTimeStop - getRasterTimeStart;
  else
    dT_measurement.dT_Task_03 = getRasterTimeStart - getTimeStop;

  // calculate dT of Distab in task 3
  if (getTimeStop > getDistabTimeStart)
    dT_measurement.dT_Distab_03 = getTimeStop - getDistabTimeStart;
  else
    dT_measurement.dT_Distab_03 = getDistabTimeStart - getTimeStop;
}

// 10ms for the handshake and page switching periodic checks
void Task_4_Impl(void)
{
  static uint32 Led_Counter_01 = 0;

  SER_Cyclic_Handshake_Check_And_Execute();

      #ifdef CALIBRATION_SUPPORTED
        if (SER_ETK_Check_PAGE_SWITCH_BY_ECU() == 1) // Page switch requested
        {
          SER_ETK_PAGE_SWITCH_BY_ECU();
        }
      #endif
      // copies the flash code check pattern to the ram address for the Advanced code check - AAC
      #ifdef ACC_SUPPORTED
        CopyCodeCheckPattern_cyclic(0);
      #endif

#if (defined CPUCLASS_TC4XX)
  updateCalibParamMeasurements(); // updates the measurements of the
                                  // corresponding test calibration parameters
#endif
  // If the last handshake was executed successfully without a timeout
  if (ECU_ETK_Status.Protocol_Success == 1 && ECU_ETK_Status.Handshake_End_State == 0)
  {
    Led_Counter_01++;
    if (Led_Counter_01 >= 10) // toggle one led every 100 msec
    {
      Led_Counter_01 = 0;
      GPIO_LedBlinkingConsecutive(); // Blinks one of the Leds between the Leds
                                     // 2 and 7
    }
  }
  /** This function should invalidate the data cache (DMI) for all the cores.
If this function is called periodically it should ensure that the calibrated
parameters value will get updated inside the cpu cache. Note: If data cache
contains data modified by the cpu, it has to be written back and
invalidated by the user ! */
        #ifdef CALIBRATION_SUPPORTED
        dataCacheInvalidate(); //function actual not available for TC4xx
        #endif
}

#if (defined CPUCLASS_TC4XX)
// processing the error mailbox of Logger Use Case
void Task_5_Impl(void)
{
  // ETAS_Handshake_LoggerMailboxProc();

  #ifdef ENABLE_WATCHDOGS
  /* enable and trigger CPU0 and SYS watchdog with a duration of 250ms */
  BASE_setWDT_CPU0(WATCHDOG_TIMER_VAL);
  BASE_setWDT_SYS(WATCHDOG_TIMER_VAL);
  #endif
}
  #ifdef D17_ENABLE_BYPASS_SUPPORT
void Task_6_Impl(void)
{
  SBB_counterTask();
}
  #endif
#elif (defined(CPUCLASS_TC3XX) && defined(D17_ENABLE_BYPASS_SUPPORT))
void Task_5_Impl(void)
{
  SBB_counterTask();
}
#endif

void Task_Alive_Impl(void)
{
  GPIO_ToggleLed(3);
#ifdef D17_ENABLE_BYPASS_SUPPORT
  SBB_processServices();
#endif
#ifdef DATAFREEZE_SAFETY_MAILBOX
      SER_ETK_Check_And_Process_DataFreeze_Safety_Mailbox();
#endif

#ifdef RS232_DEBUG_LOGGING
  RS232_DebugInfo();
#endif
}

// main function
int main(void)
{
#ifndef HIGHTEC_COMPILER
  /** We use a jump absolute command here to force the ECU to change its Program
   * Counter to the address of the cached flash section if this was compiled as
   * cached code in the linker file. */
  __asm("ja\t_absoluteJump");
  __asm("_absoluteJump:");
#endif

  // Enable instruction cache only for cached code sections (0x8xxxx xxxx)
  if (BASE_AccessDetection() == 0)
  {
    // enable instruction cache
    BASE_Enable_InstructionCache();
  }

  Init_STM_CMP0_Timer_And_Interrupt(); // setup system tick timer and interrupt,
                                       // start STM0 clock
  Init_GPIO();                         // initialize ports
  // ASC0_vInit();                        // initialize RS232 interface
#ifdef RS232_DEBUG_LOGGING
  RS232_ProjectInfo(); // project information
#endif
    #ifdef ACC_SUPPORTED
    initRAMCheckPattern();  
  #endif

  SER_Initial_Handshake_Execute();

  // If the initial handshake was not executed successfully or ran into a
  // timeout
  if (ECU_ETK_Status.Protocol_Success == 0 || ECU_ETK_Status.Handshake_End_State != 0)
  {
    /** Resets the LEDs 4-7 in the LED off state (= high output).
    This is currently used to signalize a missing ETK handshake.*/
    GPIO_LedBlinkingOff();
  }

#ifdef DATAFREEZE_STARTUP_MAILBOX
  // Checks the Startup Mailbox to see if a STARTUP_CHANGE_REQUEST_PATTERN pattern is found. Enters an infinite while
  // loop in this case.
  /** This function may not be reached during Data Freeze if the ETK is too fast or if it supports BDF (Braind Dead
   * Flashing). */
  SER_ETK_Check_And_Process_DataFreeze_Startup_Mailbox();
#endif

#ifdef DATAFREEZE_SAFETY_MAILBOX
  // Clears the Data Freeze safety mailbox
  // Should be called after the ETK Handshake protocol to be sure that the EMEM RAM is initialized before an access of
  // the mailbox.
  SER_ETK_Clear_DataFreeze_Safety_Mailbox();
#endif

#ifdef DATAFREEZE_USE_VALIDATE_PATTERN
  /* Checks the validity of the Data Freeze validate pattern.
  If the pattern is different from the correct one, an endless while loop will be entered for safety.
  In that case, the SW should stop here or perform only debugging functions to prevent a problem.
  Unknown values may be present instead of the calibration parameters located in the DATA A2L memory segments.
  Some of the program flash sectors my not have been completely written. */
  /** !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  This code will trap if the validate pattern is missing as the cpu will try to access an uninitialized ECC protected
  area which will cause a bus trap.
  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
  SER_ETK_Check_DataFreeze_Validate_Pattern();
#endif

  version_X = VERSION_X;
  version_Y = VERSION_Y;
  version_Z = VERSION_Z;

  while (1)
  {
    uint16 i;

    for (i = 0; i < ELEMENTS(task_param); ++i)
    {
      if (task_param[i].Period > 0)
      {
        const uint32 current_time = BASE_GetSystemTime();
        const uint32 task_delta   = current_time - task_measure[i].LastActivation;

        if (task_delta > task_param[i].Period)
        {
          task_measure[i].DeltaT         = task_delta;
          task_measure[i].LastActivation = current_time;
          ++(task_measure[i].ActivationCounter);
          task_param[i].Body();
        }
      }
    }
  }

  return 0;
} // main
