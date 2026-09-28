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
/*  COPYRIGHT (c) ETAS GmbH 2012-2023                                         */
/*  All Rights Reserved                                                       */
/******************************************************************************/


#include "ETK_Integration_Cfg.h"
// #include "GetTime.h"
#include "../../Base.h"    // is needed for referring to registers from .sfr file
#include "../../GPIO.h"
#ifdef CALIBRATION_SUPPORTED
  #include "../CAL/ETK_SER_Page_Ctrl_Fct.h"
#endif
#include "ETK_SER_Handshake.h"
#ifdef COLDSTART_SUPPORTED
  #include "../CS/ETK_SER_Coldstart.h"
#endif
#include "CompileTime.h"


#ifdef HIGHTEC_COMPILER
  #include <machine/intrinsics.h> // reference to the __mfcr(), __enable() function
#endif


const volatile uint32 Handshake_End_Wait_Time      = 100000; // 1ms waiting time before the ECU handshake end timeout
const volatile uint32 Protocol_Detection_Wait_Time = 500000; // 5 ms wait time for the initial ETK protocol detection
const volatile uint32 CRSYNC_Detection_Wait_Time   = 500000; // 5 ms waiting time for the CRSYNC bit detection
                                                             // until the ETK starts the handshake
struct ECU_ETK_Protocol_Status ECU_ETK_Status = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // contains the current ETK protocol status


// clang-format off
  #pragma section ".ETK_HandshakeInfoStruct" 4
static volatile struct ETK_HandshakeInfoStructure ETK_HandshakeInfoStruct =
{
  ECU_INFO_STRUCT_MAGIC_PATTERN,  /* Magic pattern "INFO" in ASCII */
  0x00000000,                     /* XOR checksum, beginning with version field, ending with 0xBAADFEED */
  0x00000001,                     /* Version 1 */
  0x00000000,                     /* length, will be calculated dynamically, must be updated in the struct BEFORE calculating checksum */

  E_ECU_INFO_STRUCT_ACC_KEY,
  0x00000004,                                     // Size
  (uint32)&CODE_CHECK_PATTERN_DATA_DATE,

  E_ECU_INFO_STRUCT_EPK_KEY,
  0x00000040,                                     // Size
  "11.05.2023;14:26:29;EPK;4731ecee96c85e6b7add1f6af65a252ce288c6bd",

  E_ECU_INFO_STRUCT_FILENAME_WO_PATH_AND_EXT_KEY,
  0x00000008,                                     // Size
  "100_Test",

  E_ECU_INFO_STRUCT_FILENAME_W_PATH_AND_EXT_KEY,
  0x00000017,                                     // Size
  "D:/Project/100_Test.a2l",

  #if (defined LOGGER_USE_CASE)
  E_ECU_INFO_LOGGER_FIRST_LEVEL_KEY, // First Level Info type
  sizeof(struct ETK_HandshakeLoggerStructure),
  // these values are dynamically initialized during Handshake
  // with the method ETK_SER_Handshake_LoggerFirstLevelInit()
  {
    0x0000,                               // Version, First Level info version
    0x0000,                               // Reserved1
    {0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0}},  // Global Unique ID
    0x00000000,                           // Flags, For checksum and memory segment check
    0x00000000,                           // Raster config address in the ECU Flash
    0x00000000,                           // Raster config size in bytes in the ECU flash
    0x00000000,                           // DAQ config address in the ECU flash
    0x00000000,                           // DAQ config size in bytes in the ECu flash
    0x00,                                 // Number of rasters
    0x00,                                 // Number of DAQs
    0x0000,                               // Reserved2
    0x00000000,                           // Distab output tables address
    0x00000000,                           // Distab output tables size
    0x00000000,                           // Distab address tables address
    0x00000000,                           // Distab address tables size
    0x00000000,                           // Distab17 table alignment
    0x0000,                               // Event list number
    0x0000,                               // Max number of allowed events
    0x00000000,                           // Event List Address
    0x0000,                               // Version of ECU error mailbox
    0x0000,                               // ECU error mailbox size
    0x00000000                            // ECU error mailbox address
  },
  #endif
  E_ECU_INFO_STRUCT_END_KEY           // end of stucture
};
#pragma section
// clang-format on

// const volatile enum HandshakeType Handshake_Answer_Type = ACC_PATTERN_ADDR;
const volatile enum HandshakeType Handshake_Answer_Type = ETK_INFO_STRUCT_ADDR;

uint32 ETK_Protocol_Handshake_End_Timing_Start = 0;
uint32 ETK_Protocol_Handshake_End_Timing_End   = 0;

uint32 ETK_Detection_Protocol_Timing_Start = 0;
uint32 ETK_Detection_Protocol_Timing_End   = 0;

uint32 CRSYNC_Detection_Timing_Start = 0;
uint32 CRSYNC_Detection_Timing_End   = 0;


/* Saves the initial start time when the ECU waits for the ETK Detection Protocol. */
inline void Start_Timing_Handshake_Protocol_End(void)
{
  ETK_Protocol_Handshake_End_Timing_Start = GetSystemTime();
}

/* Compares the current time with the maximum timeout wait time for the ETK Detection Protocol. Handshake_End_Wait_Time
 * is defined in the ETK_startup ASCET module. */
uint8 Check_Timeout_Handshake_Protocol_End(void)
{
  if ((ETK_Protocol_Handshake_End_Timing_Start + Handshake_End_Wait_Time) >= GetSystemTime())
    return 0;
  else
    return 1;
}

/* Saves the initial start time when the ECU waits for the ETK Detection Protocol. */
inline void Start_Timing_ETK_Detection_Protocol(void)
{
  ETK_Detection_Protocol_Timing_Start = GetSystemTime();
}

/* Compares the current time with the maximum timeout wait time for the ETK Detection Protocol.
 * Protocol_Detection_Wait_Time is defined in the ETK_startup ASCET module. */
uint8 Check_Timeout_ETK_Detection_Protocol(void)
{
  if ((ETK_Detection_Protocol_Timing_Start + Protocol_Detection_Wait_Time) >= GetSystemTime())
    return 0;
  else
    return 1;
}

/* Saves the initial start time when the ECU waits for the CRSYNC bit Detection. */
inline void Start_Timing_CRSYNC_Detection(void)
{
  CRSYNC_Detection_Timing_Start = GetSystemTime();
}

/* Compares the current time with the maximum timeout wait time for the ETK Detection Protocol.
 * Protocol_Detection_Wait_Time is defined in the ETK_startup ASCET module. */
uint8 Check_Timeout_CRSYNC_Detection(void)
{
  if ((CRSYNC_Detection_Timing_Start + CRSYNC_Detection_Wait_Time) >= GetSystemTime())
    return 0;
  else
    return 1;
}


/* Reads the communication Data Register to detect the different bit settings given by the ETK during the handshake.
Checks until the timeout. Returns 1 if the Protocol is read from the COMDATA successfully. Returns 0 if no ETK Detection
Pattern Protocol is found until the timeout. */
uint16 SER_ETK_Detection_Protocol_Handshake(void)
{
  union ETK_Detection_Pattern_Protocol CBS_COMDATA_Value; // ETK Detection Pattern Protocol

  uint8 CRSYNC_Detected = 0; // CRSYNC will be set to 1 when the ETK starts the handshake and is polling COMDATA.

  ECU_ETK_Status.Protocol_Success    = 0;   // ETK Detection Protocol not found
  ECU_ETK_Status.Handshake_End_State = 0;   // Handshake ended successfully without a timeout
  ECU_ETK_Status.ETK_Available       = 0;   // ETK detected during the handshake  ->  set to false
  ECU_ETK_Status.Debugger_Available  = 0;   // Debugger detected during the handshake  ->  set to false
  ECU_ETK_Status.ETK_Data_Valid      = 0;   // Data powerfail detected. Invalidate the calibration data, and switch to the RP
                                            // -> validity set to false
  ECU_ETK_Status.ETK_RAM_Valid = 0;         // RAM powerfail detected. Invalidate the Distab and bypass functions
                                            // -> validity set to false
  ECU_ETK_Status.ETK_MC_Wait = 0;           // Wait for MC configuration in the coldstart. Coldstart measurement will be
                                            // configured after the handshake.  ->  set to false
  ECU_ETK_Status.ETK_RP_Wait = 0;           // Wait for RP configuration in the coldstart. Bypass will be configured after
                                            // the handshake. -> set to false
  ECU_ETK_Status.ETK_ES_Master_Present = 0; // ES device present to control the ETK -> set to false
  ECU_ETK_Status.ETK_CalWakeUp         = 0; // CalWakeUp activated -> set to false
  ECU_ETK_Status.ETK_Flash_Request     = 0; // Flash programming was requested by the tool -> set to false




  if (ETK_INFO_STRUCT_ADDR == Handshake_Answer_Type)
  {
#if (defined LOGGER_USE_CASE)
    ETK_SER_Handshake_LoggerFirstLevelInit(&ETK_HandshakeInfoStruct.Logger);
#endif

    // Calculate length of payload (BEFORE calculating checksum)
    ETK_HandshakeInfoStruct.LengthBody = sizeof(struct ETK_HandshakeInfoStructure) - 4 * sizeof(uint32);

    // Calculate checksum of ECU Info Structure
    volatile uint32* curAdr  = &ETK_HandshakeInfoStruct.Version;
    volatile uint32* endAdr  = &ETK_HandshakeInfoStruct.TypeEND;
    volatile uint32 checksum = 0;

    while (curAdr <= endAdr)
    {
      checksum ^= *curAdr;
      curAdr++;
    }
    ETK_HandshakeInfoStruct.Checksum = checksum;
  }

  Start_Timing_CRSYNC_Detection();

  do
  {
    if (CBS_IOSR.B.CRSYNC == 1) // The ETK started polling COMDATA, thus setting CRSYNC to 1, and started the handshake.
    {
      CRSYNC_Detected             = 1;
      CRSYNC_Detection_Timing_End = GetSystemTime(); // Time until the CRSYNC bit was set to 1 saved

/** Initial Clear of COMDATA after a reset to allow the ETK to write into COMDATA as it only detects a written value by
the ECU SW once and not the value present passively in the COMDATA register. The COMDATA reset value is 0 but a chip
specific value different from 0 is written by the Infineon Boot code for the TC2xx and TC3xx. */

      CBS_COMDATA.U = 0x0; // The ECU writes the initial 0x0 value to allow the ETK to detect the initial 0 value.
    }
  } while ((Check_Timeout_CRSYNC_Detection() == 0) && CRSYNC_Detected == 0);

  if (CRSYNC_Detected == 0) // Handshake timeout occured
  {
    ECU_ETK_Status.Protocol_Success = 0; // Handshake timeout occurred - CRSYNC not set to 1 by the ETK
    return ECU_ETK_Status.Protocol_Success;
  }


  /* Gets the SystemTime for the timeout start of the protocol detection - global timeout*/
  Start_Timing_ETK_Detection_Protocol();

  /* Polls the COMDATA register */
  do
  {
    if (CBS_IOSR.B.CWSYNC == 1) // if the ETK has sent data to be read by the ECU in the COMDATA
    {
      CBS_COMDATA_Value.U = CBS_COMDATA.U;

      /* Errata: OCDS TC.043: Read-Modifiy-Write Bus transaction to cerberus registers.
      It is recommended to only do a write access to the IOSR register when setting the CW_ACK bit.
      Also only 32 bit accesses are allowed to the CBS_IOSr register. */
      CBS_IOSR.U = 0x00000040; // Clears the COMDATA pending data status CWSYNC bit.

      /* Protocol type: bits 24-31*/
      if (CBS_COMDATA_Value.B.ProtocolType == 0x01) // ETK Detection Pattern Protocol detected
      {
        ECU_ETK_Status.Protocol_Success = 1; // ETK Detection Pattern Protocol successfully read

        /* Tool type: bits 16-23 */
        ECU_ETK_Status.ETK_Available      = CBS_COMDATA_Value.B.ETKPresent;
        ECU_ETK_Status.Debugger_Available = CBS_COMDATA_Value.B.DebuggerPresent;

        /* Protocol specific parameters: bits 0-15 */
        ECU_ETK_Status.ETK_Data_Valid        = CBS_COMDATA_Value.B.DataValid;
        ECU_ETK_Status.ETK_RAM_Valid         = CBS_COMDATA_Value.B.RAMValid;
        ECU_ETK_Status.ETK_MC_Wait           = CBS_COMDATA_Value.B.MCWait;
        ECU_ETK_Status.ETK_RP_Wait           = CBS_COMDATA_Value.B.RPWait;
        ECU_ETK_Status.ETK_ES_Master_Present = CBS_COMDATA_Value.B.ESMasterPresent;
        ECU_ETK_Status.ETK_CalWakeUp         = CBS_COMDATA_Value.B.CalWakeUp;
        ECU_ETK_Status.ETK_Flash_Request     = CBS_COMDATA_Value.B.FlashProgRequest;


      }
    }
  } while ((Check_Timeout_ETK_Detection_Protocol() == 0) &&
           (ECU_ETK_Status.Protocol_Success == 0)); // Waiting until one successfull protocol read or the timeout

  ETK_Detection_Protocol_Timing_End = GetSystemTime(); // saves the time value at the end of the protocol to see the
                                                       // elapsed time since the protocol time wait start

  return ECU_ETK_Status.Protocol_Success;
}


/* Reads the communication Data Register to detect the ETK Protocol. Only checks once.
Returns 1 and updates the ETK_Status settings if the Protocol is read from the COMDATA successfully.
Returns 0 if no ETK Detection Pattern Protocol is found. */
uint16 SER_ETK_Check_Protocol_Handshake(void)
{
  volatile union ETK_Detection_Pattern_Protocol CBS_COMDATA_Value; // ETK Detection Pattern Protocol

  if (CBS_IOSR.B.CWSYNC == 1) // if the ETK has sent data to be read by the ECU in the COMDATA
  {
    // poll the COMDATA register
    CBS_COMDATA_Value.U = CBS_COMDATA.U;

    /* Errata: OCDS TC.043: Read-Modifiy-Write Bus transaction to cerberus registers.
    It is recommended to only do a write access to the IOSR register when setting the CW_ACK bit.
    Also only 32 bit accesses are allowed to the CBS_IOSr register. */
    CBS_IOSR.U = 0x00000040; // Clears the COMDATA pending data status CWSYNC bit.


    /* Protocol type: bits 24-31*/
    if (CBS_COMDATA_Value.B.ProtocolType == 0x01) // ETK Detection Pattern Protocol detected
    {
      ECU_ETK_Status.Protocol_Success = 1; // ETK Detection Pattern Protocol successfully read

      /* Tool type: bits 16-23 */
      ECU_ETK_Status.ETK_Available      = CBS_COMDATA_Value.B.ETKPresent;
      ECU_ETK_Status.Debugger_Available = CBS_COMDATA_Value.B.DebuggerPresent;

      // ETK_Available = (uint8) CBS_COMDATA_Value.ETKPresent; // needed for Distab13

      /* Protocol specific parameters: bits 0-15 */
      ECU_ETK_Status.ETK_Data_Valid        = CBS_COMDATA_Value.B.DataValid;
      ECU_ETK_Status.ETK_RAM_Valid         = CBS_COMDATA_Value.B.RAMValid;
      ECU_ETK_Status.ETK_MC_Wait           = CBS_COMDATA_Value.B.MCWait;
      ECU_ETK_Status.ETK_RP_Wait           = CBS_COMDATA_Value.B.RPWait;
      ECU_ETK_Status.ETK_ES_Master_Present = CBS_COMDATA_Value.B.ESMasterPresent;
      ECU_ETK_Status.ETK_CalWakeUp         = CBS_COMDATA_Value.B.CalWakeUp;
      ECU_ETK_Status.ETK_Flash_Request     = CBS_COMDATA_Value.B.FlashProgRequest;



      return 1; // Successful ETK Protocol read
    }
    else
    {
      return 0; // No ETK Detection Protocol pattern found
    }
  }
  else
  {
    return 0; // No ETK Detection Protocol pattern found
  }
}


/* This function writes back the advanced code check pattern in the COMDATA register and then waits for the ETK to end
the handshake by clearing the COMDATA register. If a timeout occurs the ECU will clear the register itself.  Returns 0
if the handshake was ended by the ETK correctly, 1 if the ECU cleared the COMDATA after a timeout, and 2 if the COMDATA
was already cleared by the ETK before this step. */
uint8 SER_ETK_Handshake_End(void)
{
  uint8 handshakeState;

  if (CBS_IOSR.B.CWSYNC == 0) // if the ETK has not yet cleared the COMDATA after a timeout
  {

    handshakeState = 1; // Active wait state - Not cleared by the ETK

    Start_Timing_Handshake_Protocol_End(); // gets the SystemTime for the timeout start of the handshake end

    do // Waiting for the ETK to be ready to receive the COMDATA answer from the ECU, and then writing the answer
    {
      if (CBS_IOSR.B.CRSYNC == 1) // if the ETK is waiting for the ECU to write in the COMDATA
      {
        handshakeState = 2; // Active wait state - Response written by the ECU -> Waiting for the ETK to clear the
                            // COMDATA and end the handshake

        if (ACC_PATTERN_ADDR == Handshake_Answer_Type)
        {
          /* ECU writes the Advanced code check pattern in flash address back to the COMDATA register to end the
          handshake phase. IMPORTANT:  The address of the code check pattern written by the ECU should always be at
          least 2 bytes aligned. The ETK protocol writes the first bit of COMDATA to 1 in order to distinguish it from
          an ECU answer which should give a 4 byte aligned address back in this case. Thus in the ECU answer the first
          bit is set to 0. */
          CBS_COMDATA.U = (uint32)&CODE_CHECK_PATTERN_DATA_DATE;
        }
        else if (ETK_INFO_STRUCT_ADDR == Handshake_Answer_Type)
        {
          /* In this case the ACC pattern address is replaced with the advanced
           * ETK information structure address.*/
          CBS_COMDATA.U = (uint32)&ETK_HandshakeInfoStruct;
        }
        else
        {
          CBS_COMDATA.U = 0; // Error, this should not happen
        }

        // Saves the system time at the start of the coldstart after the ECU write to the COMDATA if a coldstart is
        // requested.
        #ifdef COLDSTART_SUPPORTED
          Start_Timing_Coldstart();
        #endif
      }
    } while (Check_Timeout_Handshake_Protocol_End() == 0 && handshakeState == 1);

    // If the ECU answer in the COMDATA succeeded, waits for the ETK to clear the COMDATA and end the handshake
    while (Check_Timeout_Handshake_Protocol_End() == 0 && handshakeState == 2)
    {
      if (CBS_IOSR.B.CWSYNC == 1)
      // if the ETK has sent data to be read by the ECU in the COMDATA after the CRSYNC has been cleared -(
      // CRSYNC blocks the ETK write until it is cleared when the ECU writes to the COMDATA)
      {
        if (CBS_COMDATA.U == 0) // Cleared register by the ETK. End of the Handshake.
        {
          handshakeState = 0; // The handshake ended correctly
        }
        else // The value written by the ETK is not the expected 0x0 clear value
        {
          handshakeState = 3; // Unexpected value found in the COMDATA register at the handshake end
        }

        /* Errata: OCDS TC.043: Read-Modifiy-Write Bus transaction to cerberus registers.
        It is recommended to only do a write access to the IOSR register when setting the CW_ACK bit.
        Also only 32 bit accesses are allowed to the CBS_IOSr register. */
        CBS_IOSR.U = 0x00000040; // Clears the COMDATA pending data status CWSYNC bit.
      }
    }

    ETK_Protocol_Handshake_End_Timing_End = GetSystemTime(); // saves the time value at the end of the handshake to see
                                                             // the elapsed time since the handshake end time wait start

    switch (handshakeState)
    {
      case 0: // 0 -> The handshake ended correctly
        /* !!!!!!!!!!! The ECU has to write 0x0 to the COMDATA register even if it was already cleared by the ETK. The
         * value has a different meaning if it is written by the ETK or ECU.  !!!!!!!!!!!  */
        CBS_COMDATA.U = 0; // The ECU also writes 0 to the COMDATA register.
        break;

      case 1:              // 1 -> ECU response not written in COMDATA - No answer request from the ETK
        CBS_COMDATA.U = 0; // The ECU clears the COMDATA register.

#ifdef CALIBRATION_SUPPORTED
        Page_Switch_Mailbox.Error_Code =
            0xA0000001; // Handshake timeout from the ECU side - No answer request from the ETK - No coldstart performed
#endif
        break;

      case 2:              // 2 -> ECU timeout after the ECU write in the COMDATA - COMDATA not cleared by the ETK
        CBS_COMDATA.U = 0; // The ECU clears the COMDATA register.

#ifdef CALIBRATION_SUPPORTED
        Page_Switch_Mailbox.Error_Code =
            0xA0000002; // Handshake timeout from the ECU side - COMDATA not cleared by the ETK - No coldstart performed
#endif
        break;

      case 3:              // 3 -> Unexpected value found in COMDATA at the end of the handshake instead of the 0x0 clear value -
                           // COMDATA cleared by the ECU
        CBS_COMDATA.U = 0; // The ECU clears the COMDATA register.

#ifdef CALIBRATION_SUPPORTED
        Page_Switch_Mailbox.Error_Code =
            0xA0000003; // Unexpected value found in COMDATA at the end of the handshake instead of the 0x0 clear value
                        // - COMDATA cleared by the ECU - No coldstart performed
#endif
        break;

      default: // Default -> This should never be hit.
        break;
    }
  }
  else // ETK Timeout - COMDATA already cleared before the ECU COMDATA answer
  {
    if (CBS_COMDATA.U == 0) // Cleared too fast by the ETK
    {
      handshakeState = 4; // ETK timeout - Cleared too fast by the ETK

      /* Errata: OCDS TC.043: Read-Modifiy-Write Bus transaction to cerberus registers.
      It is recommended to only do a write access to the IOSR register when setting the CW_ACK bit.
      Also only 32 bit accesses are allowed to the CBS_IOSr register. */
      CBS_IOSR.U = 0x00000040; // Clears the COMDATA pending data status CWSYNC bit.

      /* !!!!!!!!!!! The ECU has to write 0x0 to the COMDATA register even if it was already cleared by the ETK. The
       * value has a different meaning if it is written by the ETK or ECU.  !!!!!!!!!!!  */
      CBS_COMDATA.U = 0; // The ECU also writes 0 to the COMDATA register.

#ifdef CALIBRATION_SUPPORTED
      Page_Switch_Mailbox.Error_Code = 0xA0000004; // Handshake timeout from the ETK side - COMDATA already cleared
                                                   // before the ECU COMDATA answer - No coldstart performed
#endif
    }
    else // Cleared too fast by the ETK - The value written by the ETK is not the expected 0x0 clear value
    {
      handshakeState = 5; // ETK timeout - Cleared too fast by the ETK - Unexpected value found in the COMDATA register
                          // at the handshake end

      /* Errata: OCDS TC.043: Read-Modifiy-Write Bus transaction to cerberus registers.
      It is recommended to only do a write access to the IOSR register when setting the CW_ACK bit.
      Also only 32 bit accesses are allowed to the CBS_IOSr register. */
      CBS_IOSR.U = 0x00000040; // Clears the COMDATA pending data status CWSYNC bit.

      CBS_COMDATA.U = 0; // The ECU clears the COMDATA register.

#ifdef CALIBRATION_SUPPORTED
      Page_Switch_Mailbox.Error_Code =
          0xA0000005; // Handshake timeout from the ETK side - COMDATA already cleared before the ECU COMDATA answer -
                      // Unexpected value found in the COMDATA register at the handshake end - No coldstart performed
#endif
    }

    // The time of the start and the end is the same
    ETK_Protocol_Handshake_End_Timing_Start = GetSystemTime();
    ETK_Protocol_Handshake_End_Timing_End   = ETK_Protocol_Handshake_End_Timing_Start;
  }

  // Saving the handshake end state
  ECU_ETK_Status.Handshake_End_State = handshakeState;

  return (uint8)handshakeState;
}


/** It is required to enable the OCDS mode in order to activate the triggers, starting from the Aurix TC27x B-step
silicon. If OCDS mode is not enabled the triggers will not be set when writen by the ECU. Also if OCDS is later disabled
the ETK connection will be lost. */
void Enable_OCDS_Triggers(void)
{
  if (CBS_OSTATE.B.OEN == 0) // If OCDS is currently disabled 
  {
    CBS_OEC.U = 0xA1;
    CBS_OEC.U = 0x5E;
    CBS_OEC.U = 0xA1;
    CBS_OEC.U = 0x5E; // OCDS enable sequence --> Suspends the ENDINIT protection with watchdog !!!!!
  }
}

#if (defined PD_DEVICE)

void EMEM_RAM_ECC_Initialize(void) {}

#elif defined CPUCLASS_TC4XX

void EMEM_RAM_ECC_Initialize(void)
{
  #if (defined CPUTYPE_TC48X)
  memSet64(0xB0000000, 480, 0); // DLMU0, last 32kB of DLMU0 reserved for CodeCheck, D17, OMD, MailBox
  #elif (defined CPUTYPE_TC49X)
  memSet64(0xB0300000, 4064, 0); // LMU0 .. 7, last 32kB of LMU reserved for CodeCheck, D17, OMD, MailBox
  #elif (defined CPUTYPE_TC49X_N)
  memSet64(0xB0400000, 3072, 0); // LMU0 .. 5, last 32kB of LMU reserved for CodeCheck, D17, OMD, MailBox
  #elif (defined CPUTYPE_TC4DX)
  memSet64(0xB0400000, 5088, 0); // LMU0 .. 9, last 32kB of LMU reserved for CodeCheck, D17, OMD, MailBox
  #elif (defined CPUTYPE_TC4DX_PD)
  memSet64(0xB0000000, 3040, 0); // DLMU0 .. 5, last 32kB of DLMU reserved for CodeCheck, D17, OMD, MailBox
  #elif (defined CPUTYPE_TC46X)
  memSet64(0xB0400000, 512, 0); // LMU0, last 32kB of LMU reserved for CodeCheck, D17, OMD, MailBox
  #elif (defined CPUTYPE_TC4ZX)
  memSet64(0xB0400000, 3040, 0); // LMU0 .. 5, last 32kB of LMU reserved for CodeCheck, D17, OMD, MailBox
  #endif
}

#else // ED device with TC2xx or TC3xx

/* Initializes the program memory and the EMEM memory with 0 initial values for ECC.
The EMEM memory has to be written entirely once before being accessible after a start. */

#endif // #ifdef PD_DEVICE
