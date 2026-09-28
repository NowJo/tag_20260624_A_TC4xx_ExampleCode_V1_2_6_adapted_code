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


#include "../HS/ETK_Integration_Cfg.h"     /* for the configuration */
// #include "Base.h"
#include "ETK_SER_Page_Ctrl_Fct.h"
#include "../HS/ETK_SER_Handshake.h"


#ifdef HIGHTEC_COMPILER
  #include <machine/intrinsics.h> // reference to the __mfcr(), __enable() function
#endif


/** Disables interrupts. This is needed when accessing ENDINIT protected registers for the page switching.
This could be replaced by a code raising the processor priority level.
It is important to check that the ENDINIT watchdog does not run into a timeout until the ENDINIT protection is enabled
again. */
void Disable_Interrupts(void)
{
#ifdef ASM_CODE_ENABLE_DISABLE
  #ifdef HIGHTEC_COMPILER
  __asm__("disable");
  #else
  __asm("disable");
  #endif
#else
  #ifdef HIGHTEC_COMPILER
  _disable();
  #else
  __disable();
  #endif
#endif
}

/** Enables interrupts. This is needed when accessing ENDINIT protected registers for the page switching.
This could be replaced by a code lowering the processor priority level.
It is important to check that the ENDINIT watchdog does not run into a timeout until the ENDINIT protection is enabled
again. */
void Enable_Interrupts(void)
{
#ifdef ASM_CODE_ENABLE_DISABLE
  #ifdef HIGHTEC_COMPILER
  __asm__("enable");
  #else
  __asm("enable");
  #endif
#else
  #ifdef HIGHTEC_COMPILER
  _enable();
  #else
  __enable();
  #endif
#endif
}

static uint32 getOmaskFromOvlSize(uint32 size);


/** Initializes the RABR registers for a CPU starting from the given address.
This will configure the calibration handles to disable the calibration, and to select the EMURAM (or DSPR RAM for a PD
device) as the base emulation memory used for the calibration. It will also configure the base address to be used in the
EMURAM for each handle. The base address offset is defined with the CALIB_HANDLE_SIZE macro. The base address currently
does not change after the initial configuration. */
// TC4xx with LMU RAM
void Init_RABR(uint32* registerPtr)
{
  #if (defined CPUTYPE_TC49X)
  uint32 oBaseOffset = 0x00300000; // LMU0 initial address 0x9/B0300000
  #elif (defined CPUTYPE_TC4DX || defined CPUTYPE_TC49X_N || defined CPUTYPE_TC46X || defined CPUTYPE_TC4ZX)
  uint32 oBaseOffset = 0x00400000; // LMU0 initial address 0x9/B0400000
  #elif (defined CPUTYPE_TC4DX_PD)
  uint32 oBaseOffset = 0x0; // DLMU0 initial address 0x9/B0000000
  #else
  uint32 oBaseOffset = 0;
  #endif

  int i;
  for (i = 0; i < NUMBER_OMD_MAX_CALIB_HANDLES; i++)
  {
    if (oBaseOffset < 0x00400000)
    {
      // Disable overlay, redirection to LMU low, and OBase Address
      *registerPtr = (0x08000000 | oBaseOffset);
    }
    else if (oBaseOffset < 0x00800000)
    {
      // Disable overlay, redirection to LMU (mid: TC4Dx, high: TC49x), and OBase Address
      *registerPtr = (0x09000000 | oBaseOffset);
    }
  #if (defined CPUTYPE_TC4DX)
    else if (oBaseOffset >= 0x00800000)
    {
      // Disable overlay, redirection to LMU high (only TC4Dx), and OBase Address
      *registerPtr = (0x0A000000 | oBaseOffset);
    }
  #endif
    registerPtr += 3;                 // 3 registers offset of 0x0C
    oBaseOffset += CALIB_HANDLE_SIZE; // Calibration handle size added as offset for the next handle.
  }
}


/** Initializes the OTAR registers for a CPU starting from the given address.
This will reset all the emulation target addresses to 0x00000000 in order to prevent an inconsistency with an earlier
state. */
void Init_OTAR(uint32* registerPtr)
{
  int i;

  for (i = 0; i < NUMBER_OMD_MAX_CALIB_HANDLES; i++)
  {
    *registerPtr = 0x0; // OTAR initialized to 0 value (initial reset value)
    registerPtr += 3;   // 3 registers offset of 0x0C
  }
}

/** Initializes the OMASK registers for a CPU starting from the given address.
This will define the size of the calibration handles and does't need to be modified after the initial configuration.
The value written to the register can be defined with the  OMASK_TRICORE_CALIB_HANDLE_CONFIG macro.*/
void Init_OMASK(uint32* registerPtr)
{
  int i;

  for (i = 0; i < NUMBER_OMD_MAX_CALIB_HANDLES; i++)
  {
    *registerPtr = OMASK_TRICORE_CALIB_HANDLE_CONFIG; // Set ONE and OMASK values to the value corresponding to the
                                                      // desired calibration handle size
    registerPtr += 3;                                 // 3 registers offset of 0x0C
  }
}

/** Updates the OTAR registers for all the CPUs if a modify bit is given in the OMD.
This will update the emulation target addresses if it is requested by the corresponding  modify bit in the OMD. */
void Update_OTAR(VOLATILE_DEF struct OMD_TABLE* OMD_table_Ptr)
{
  int i, relativeCalibHandlePos;
  uint32 modifyEMUHandles = 0, modifyHandleValue;
  uint32 OTARConfig, OMASKConfig, RABRConfig, oBaseOffset, RedirectConfig;
  uint8 redirectAddr;

  if (OMD_table_Ptr->CID == IMPLEMENTED_OMD_CID) // CID version 2 detected - LERTv3 dynamic calibration handles mode
  {
    for (i = 0; i < NUMBER_OMD_MAX_CALIB_HANDLES; i++)
    {
      relativeCalibHandlePos = i % 32;
      if (relativeCalibHandlePos == 0) // updates the modify value with the right one if the modify table is bigger than 32 bits
      {
        modifyEMUHandles = OMD_table_Ptr->Modify_EMU_Handles[i / 32];
      }

      modifyHandleValue = (modifyEMUHandles >> relativeCalibHandlePos) & 0x00000001;

      if (modifyHandleValue)                                // calibration handles has to be allocated
      {
        oBaseOffset = OMD_table_Ptr->Target_Address[3 * i]; // Physical Address: The address of the overlay memory (RAM)
                                                            // that shall overlay another memory range
        redirectAddr = (uint8)((oBaseOffset & 0xFF000000) >> 24);

        switch (redirectAddr)
        {
#ifdef CPUCLASS_TC4XX
          case 0xB0:
          case 0x90:
  #if (defined CPUTYPE_TC4DX)
            if ((oBaseOffset & 0x00FFFFFF) >= 0x00800000)
            {
              RedirectConfig = 0x0A000000; // redirection to LMU HIGH
            }
            else
  #endif
                if ((oBaseOffset & 0x00FFFFFF) < 0x00400000)
            {
              RedirectConfig = 0x08000000; // redirection to LMU LOW
            }
            else
            {
              RedirectConfig = 0x09000000; // redirection to LMU MID/HIGH
            }
            break;

          case 0x70:
            RedirectConfig = 0x00000000; // redirection to Core 0 DSPR/PSPR memory
            break;
          case 0x60:
            RedirectConfig = 0x01000000; // redirection to Core 1 DSPR/PSPR memory
            break;
          case 0x50:
            RedirectConfig = 0x02000000; // redirection to Core 2 DSPR/PSPR memory
            break;
          case 0x40:
            RedirectConfig = 0x03000000; // redirection to Core 3 DSPR/PSPR memory
            break;
        #if !defined(CPUTYPE_TC46X)      
          case 0x30:
            RedirectConfig = 0x04000000; // redirection to Core 4 DSPR/PSPR memory
            break;
        #endif
      #if (defined CPUCLASS_TC4XX)
        #if !defined(CPUTYPE_TC49X_N) && !defined(CPUTYPE_TC46X)
          case 0x20:
            RedirectConfig = 0x05000000; // redirection to Core 5 DSPR/PSPR memory
            break;
           #endif
          case 0x10:
            RedirectConfig = 0x06000000; // redirection to Core CPUcs DSPR/PSPR memory
            break;
      #else
          case 0x10:
            RedirectConfig = 0x06000000; // redirection to Core CS DSPR/PSPR memory
            break;
      #endif

          default:
            // ERROR - Setting the default value to LMU/EMEM redirection
            // TODO: Add error processing and stop configuration process with returned error
            RedirectConfig = 0x08000000; // redirection to LMU low
            // RS232_TxString("Unknown OTAR address specified in OMD: Redirection set to LMU LOW by default !\r\n");
            break;
#else
  #error "Missing OMDv2 calibration configuration code for µC!"
#endif // #ifdef CPUCLASS_TC4XX
        }

        // Disable overlay, redirection to EMEM/LMU/high/low, and OBase Address
        if (i < 2)
        {
          RABRConfig = (RedirectConfig | (oBaseOffset & 0x00FFFFE0));
        }
        else
        {
          RABRConfig = (RedirectConfig | (oBaseOffset & 0x003FFFE0));
        }

#if (defined CPUCLASS_TC4XX)
        // writes the config to the RABR register corresponding to the handle number
        *((uint32*)(&CPU0_RABR0 + (3 * i))) = RABRConfig;
        *((uint32*)(&CPU1_RABR0 + (3 * i))) = RABRConfig;
        *((uint32*)(&CPU2_RABR0 + (3 * i))) = RABRConfig;
        *((uint32*)(&CPU3_RABR0 + (3 * i))) = RABRConfig;
        #if !defined(CPUTYPE_TC46X) && !defined(CPUTYPE_TC48X)  // Support CPU[0..3]
          *((uint32*)(&CPU4_RABR0 + (3 * i))) = RABRConfig;
          #if !defined(CPUTYPE_TC49X_N)                         // Support CPU[0..4]
          *((uint32*)(&CPU5_RABR0 + (3 * i))) = RABRConfig; // only for TC49x/TC4Dx, TC49x_N has only 5 RABR registers
          #endif
        #endif
#else
  #error "Missing OMDv2 calibration configuration code for µC!"
#endif // CPUCLASS_TC4XX


        OMASKConfig = getOmaskFromOvlSize(OMD_table_Ptr->Target_Address[(3 * i) + 2]);

#if (defined CPUCLASS_TC4XX)
        // writes the config to the OMASK register corresponding to the handle number
        *((uint32*)(&CPU0_OMASK0 + (3 * i))) = OMASKConfig;
        *((uint32*)(&CPU1_OMASK0 + (3 * i))) = OMASKConfig;
        *((uint32*)(&CPU2_OMASK0 + (3 * i))) = OMASKConfig;
        *((uint32*)(&CPU3_OMASK0 + (3 * i))) = OMASKConfig;
        #if !defined(CPUTYPE_TC46X) && !defined(CPUTYPE_TC48X)  // Support CPU[0..3]
          *((uint32*)(&CPU4_OMASK0 + (3 * i))) = OMASKConfig;
          #if !defined(CPUTYPE_TC49X_N)                         // Support CPU[0..4]
            *((uint32*)(&CPU5_OMASK0 + (3 * i))) = OMASKConfig; // only for TC49x/TC4Dx, TC49x_N has only 5 OMASK registers
          #endif
        #endif
#endif // CPUCLASS_TC4XX


        // Virtual Address: The address of the memory (read-only flash) that shall be overlaid
        OTARConfig = (OMD_table_Ptr->Target_Address[3 * i + 1] & 0x0fffffe0);

#if (defined CPUCLASS_TC4XX)
        // writes the config to the OTAR register corresponding to the handle number
        *((uint32*)(&CPU0_OTAR0 + (3 * i))) = OTARConfig;
        *((uint32*)(&CPU1_OTAR0 + (3 * i))) = OTARConfig;
        *((uint32*)(&CPU2_OTAR0 + (3 * i))) = OTARConfig;
        *((uint32*)(&CPU3_OTAR0 + (3 * i))) = OTARConfig;
        #if !defined(CPUTYPE_TC46X) && !defined(CPUTYPE_TC48X)  // Support CPU[0..3]
          *((uint32*)(&CPU4_OTAR0 + (3 * i))) = OTARConfig;
          #if !defined(CPUTYPE_TC49X_N)                               // Support CPU[0..3]
          *((uint32*)(&CPU5_OTAR0 + (3 * i))) = OTARConfig; // only for TC49x/TC4Dx, TC49x_N has only 5 OTAR registers
          #endif
        #endif
#endif // CPUCLASS_TC4XX
      }
    }
  }
  else // ERROR undefined CID value
  {

  }
}


/** Updates all the OTAR registers for all the CPUs without checking the modify bit in the OMD.
This will initialize the emulation target addresses with the target addresses found in the OMD. */
void Update_All_OTAR(VOLATILE_DEF struct OMD_TABLE* OMD_table_Ptr)
{
  int i;
  uint32 OTARConfig, OMASKConfig, RABRConfig, oBaseOffset, RedirectConfig;
  uint8 redirectAddr;

  if (OMD_table_Ptr->CID == IMPLEMENTED_OMD_CID) // CID version 2 detected - LERTv3 dynamic calibration handles mode
  {
    for (i = 0; i < NUMBER_OMD_MAX_CALIB_HANDLES; i++)
    {
      oBaseOffset = OMD_table_Ptr->Target_Address[3 * i]; // Physical Address: The address of the overlay memory (RAM)
                                                          // that shall overlay another memory range
      redirectAddr = (uint8)((oBaseOffset & 0xFF000000) >> 24);

      switch (redirectAddr)
      {
#ifdef CPUCLASS_TC4XX
        case 0xB0:
        case 0x90:
  #if (defined CPUTYPE_TC4DX)
          if ((oBaseOffset & 0x00FFFFFF) >= 0x00800000)
          {
            RedirectConfig = 0x0A000000; // redirection to LMU HIGH
          }
          else
  #endif
              if ((oBaseOffset & 0x00FFFFFF) < 0x00400000)
          {
            RedirectConfig = 0x08000000; // redirection to LMU LOW
          }
          else
          {
            RedirectConfig = 0x09000000; // redirection to LMU MID/HIGH
          }
          break;

        case 0x70:
          RedirectConfig = 0x00000000; // redirection to Core 0 DSPR/PSPR memory
          break;
        case 0x60:
          RedirectConfig = 0x01000000; // redirection to Core 1 DSPR/PSPR memory
          break;
        case 0x50:
          RedirectConfig = 0x02000000; // redirection to Core 2 DSPR/PSPR memory
          break;
        case 0x40:
          RedirectConfig = 0x03000000; // redirection to Core 3 DSPR/PSPR memory
          break;
        case 0x30:
          RedirectConfig = 0x04000000; // redirection to Core 4 DSPR/PSPR memory
          break;
        case 0x20:
          RedirectConfig = 0x05000000; // redirection to Core 5 DSPR/PSPR memory
          break;
        case 0x10:
          RedirectConfig = 0x06000000; // redirection to Core CS DSPR/PSPR memory
          break;

        default:
          // ERROR - Setting the default value to LMU/EMEM redirection
          // TODO: Add error processing and stop configuration process with returned error
          RedirectConfig = 0x09000000; // redirection to LMU HIGH
          // RS232_TxString("Unknown OTAR address specified in OMD: Redirection set to LMU LOW by default !\r\n");
          break;
#else
  #error "Missing OMDv2 calibration configuration code for µC!"
#endif //  CPUCLASS_TC4XX
      }

      // Disable overlay, redirection to EMEM/LMU/high/low, and OBase Address
      if (i < 2)
      {
        RABRConfig = (RedirectConfig | (oBaseOffset & 0x00FFFFE0));
      }
      else
      {
        RABRConfig = (RedirectConfig | (oBaseOffset & 0x003FFFE0));
      }

#if (defined CPUCLASS_TC4XX)
      // writes the config to the RABR register corresponding to the handle number
      *((uint32*)(&CPU0_RABR0 + (3 * i))) = RABRConfig;
      *((uint32*)(&CPU1_RABR0 + (3 * i))) = RABRConfig;
      *((uint32*)(&CPU2_RABR0 + (3 * i))) = RABRConfig;
      *((uint32*)(&CPU3_RABR0 + (3 * i))) = RABRConfig;
      #if !defined(CPUTYPE_TC46X) && !defined(CPUTYPE_TC48X)  // Support CPU[0..3]
      *((uint32*)(&CPU4_RABR0 + (3 * i))) = RABRConfig;
        #if !defined(CPUTYPE_TC49X_N) 
        *((uint32*)(&CPU5_RABR0 + (3 * i))) = RABRConfig; // only for TC49x/TC4Dx, TC49x_N has only 5 RABR registers
        #endif
      #endif
#else
  #error "Missing OMDv2 calibration configuration code for µC!"
#endif // CPUTYPE_TC49X || CPUTYPE_TC4DX


      OMASKConfig = getOmaskFromOvlSize(OMD_table_Ptr->Target_Address[(3 * i) + 2]);

#if (defined CPUCLASS_TC4XX)
      // writes the config to the OMASK register corresponding to the handle number
      *((uint32*)(&CPU0_OMASK0 + (3 * i))) = OMASKConfig;
      *((uint32*)(&CPU1_OMASK0 + (3 * i))) = OMASKConfig;
      *((uint32*)(&CPU2_OMASK0 + (3 * i))) = OMASKConfig;
      *((uint32*)(&CPU3_OMASK0 + (3 * i))) = OMASKConfig;
      #if !defined(CPUTYPE_TC46X) && !defined(CPUTYPE_TC48X)  // Support CPU[0..3]
      *((uint32*)(&CPU4_OMASK0 + (3 * i))) = OMASKConfig;
        #if !defined(CPUTYPE_TC49X_N) 
        *((uint32*)(&CPU5_OMASK0 + (3 * i))) = OMASKConfig; // only for TC49x/TC4Dx, TC49x_N has only 5 OMASK registers
        #endif
      #endif
#endif // #ifdef CPUTYPE_TC49X


      // Virtual Address: The address of the memory (read-only flash) that shall be overlaid
      OTARConfig = (OMD_table_Ptr->Target_Address[3 * i + 1] & 0x0fffffe0);

#if (defined CPUCLASS_TC4XX)
      // writes the config to the OTAR register corresponding to the handle number
      *((uint32*)(&CPU0_OTAR0 + (3 * i))) = OTARConfig;
      *((uint32*)(&CPU1_OTAR0 + (3 * i))) = OTARConfig;
      *((uint32*)(&CPU2_OTAR0 + (3 * i))) = OTARConfig;
      *((uint32*)(&CPU3_OTAR0 + (3 * i))) = OTARConfig;
      #if !defined(CPUTYPE_TC46X) && !defined(CPUTYPE_TC48X)  // Support CPU[0..3]
      *((uint32*)(&CPU4_OTAR0 + (3 * i))) = OTARConfig;
        #if !defined(CPUTYPE_TC49X_N) 
        *((uint32*)(&CPU5_OTAR0 + (3 * i))) = OTARConfig; // only for TC49x/TC4Dx, TC49x_N has only 5 OTAR registers
        #endif
      #endif
#endif // CPUCLASS_TC4XX
    }
  }
  else // ERROR undefined CID value
  {

  }
}


/** Configures the initial value of the RABR, OTAR and OMASK registers used for the calibration configuration, and sets
 * the OVC_ENABLE value for the required CPU cores */
void Initialize_Calib_Registers(void)
{

#if (defined CPUCLASS_TC4XX)
  Init_RABR((uint32*)&CPU0_RABR0);   // Initializing RABR registers for CPU0
  Init_RABR((uint32*)&CPU1_RABR0);   // Initializing RABR registers for CPU1
  Init_RABR((uint32*)&CPU2_RABR0);   // Initializing RABR registers for CPU2
  Init_RABR((uint32*)&CPU3_RABR0);   // Initializing RABR registers for CPU3
  #if !defined(CPUTYPE_TC46X) && !defined(CPUTYPE_TC48X)  // Support CPU[0..3]
  Init_RABR((uint32*)&CPU4_RABR0);   // Initializing RABR registers for CPU4
    #if !defined(CPUTYPE_TC49X_N) 
    Init_RABR((uint32*)&CPU5_RABR0);   // - only for TC49x/TC4Dx, TC49x_N has only 5 RABR registers
    #endif
  #endif

  Init_OTAR((uint32*)&CPU0_OTAR0);   // Initializing OTAR registers for CPU0
  Init_OTAR((uint32*)&CPU1_OTAR0);   // Initializing OTAR registers for CPU1
  Init_OTAR((uint32*)&CPU2_OTAR0);   // Initializing OTAR registers for CPU2
  Init_OTAR((uint32*)&CPU3_OTAR0);   // Initializing OTAR registers for CPU3
  #if !defined(CPUTYPE_TC46X) && !defined(CPUTYPE_TC48X)  // Support CPU[0..3]
  Init_OTAR((uint32*)&CPU4_OTAR0);   // Initializing OTAR registers for CPU4
    #if !defined(CPUTYPE_TC49X_N) 
    Init_OTAR((uint32*)&CPU5_OTAR0);   // - only for TC49x/TC4Dx, TC49x_N has only 5 OTAR registers
    #endif
  #endif

  Init_OMASK((uint32*)&CPU0_OMASK0); // Initializing OMASK registers for CPU0
  Init_OMASK((uint32*)&CPU1_OMASK0); // Initializing OMASK registers for CPU1
  Init_OMASK((uint32*)&CPU2_OMASK0); // Initializing OMASK registers for CPU2
  Init_OMASK((uint32*)&CPU3_OMASK0); // Initializing OMASK registers for CPU3
  #if !defined(CPUTYPE_TC46X) && !defined(CPUTYPE_TC48X)  // Support CPU[0..3]
  Init_OMASK((uint32*)&CPU4_OMASK0); // Initializing OMASK registers for CPU4
    #if !defined(CPUTYPE_TC49X_N) 
    Init_OMASK((uint32*)&CPU5_OMASK0); // - only for TC49x/TC4Dx, TC49x_N has only 5 OMASK registers
    #endif
  #endif

  // enables OVC on the CPU 0-5 in the SCU_OVCENABLE register
  #ifdef CPUCLASS_TC4XX
  SCU_OVCENABLE.U = 0x3F; // TC4xx uses PROT instead of ENDINIT to protect register accesses
  #endif

#endif // CPUTYPE_TC49X || CPUTYPE_TC4DX
}


/** Sets all the EMEM tiles to the calibration mode which allows an access to the calibration RAM for the ETK or the use
 * of the RAM as internal ECU RAM. */
void Configure_EMEM_Tiles_For_ECU_RAM(void)
{
#ifdef CPUCLASS_TC4XX
  /* The TC4xx uses LMU RAM instead of the EMEM RAM for calibration, and does not have a tile configuration. */
#endif
}


/** Disables the calibration handles (same as switching to the reference page)
This function will clear the overlay bits in the RABRx registers. */
void Disable_Calibration_Handles(void)
{
#if (defined CPUCLASS_TC4XX)
  
  CPU0_OSEL.U = 0x0; // Disable all the overlays in the CPU0 when OVC_CON.OVSTRT is set to 1
  CPU1_OSEL.U = 0x0; // Disable all the overlays in the CPU1 when OVC_CON.OVSTRT is set to 1
  CPU2_OSEL.U = 0x0; // Disable all the overlays in the CPU2 when OVC_CON.OVSTRT is set to 1
  CPU3_OSEL.U = 0x0; // Disable all the overlays in the CPU3 when OVC_CON.OVSTRT is set to 1
  #if !defined(CPUTYPE_TC46X) && !defined(CPUTYPE_TC48X)  // Support CPU[0..3]
  CPU4_OSEL.U = 0x0; // Disable all the overlays in the CPU4 when OVC_CON.OVSTRT is set to 1
    #if !defined(CPUTYPE_TC49X_N) 
    CPU5_OSEL.U = 0x0; // Disable all the overlays in the CPU5 when OVC_CON.OVSTRT is set to 1 - only for TC49x/TC4Dx, TC49x_N has only 5 OSEL registers
    #endif
  #endif

  // Disable OVC for the CPU 0-5 in the SCU_OVCCON register -> OVCx_RABRy.OVEN cleared for all the CPUs
  // The OVCCON register is not protected by the safety endinit module starting from the TC3xx family.
  SCU_OVCCON.U = 0x0006003F; // OVC_OVSTP, DCINVAL and OVC_CSEL0 to OVC_CSEL5
  
#else
  #error "Undefined Disable_Calibration_Handles function for the current µC !"
#endif
}


/** Sets all the overlays in the CPUs to the defined value in the OMD after OVC_CON.OVSTRT is set to 1. */
void Set_Calibration_Handles(VOLATILE_DEF struct OMD_TABLE* OMD_table_Ptr)
{
#if (defined CPUCLASS_TC4XX)
  
  CPU0_OSEL.U = OMD_table_Ptr->Activate_EMU_Handles[0]; // Sets up the overlays in the CPU0 OSEL. Initialized when
                                                        // OVC_CON.OVSTRT is set to 1.
  CPU1_OSEL.U = OMD_table_Ptr->Activate_EMU_Handles[0]; // Sets up the overlays in the CPU1 OSEL. Initialized when
                                                        // OVC_CON.OVSTRT is set to 1.
  CPU2_OSEL.U = OMD_table_Ptr->Activate_EMU_Handles[0]; // Sets up the overlays in the CPU2 OSEL. Initialized when
                                                        // OVC_CON.OVSTRT is set to 1.
  CPU3_OSEL.U = OMD_table_Ptr->Activate_EMU_Handles[0]; // Sets up the overlays in the CPU3 OSEL. Initialized when
                                                        // OVC_CON.OVSTRT is set to 1.
  #if !defined(CPUTYPE_TC46X) && !defined(CPUTYPE_TC48X)  // Support CPU[0..3]
  CPU4_OSEL.U = OMD_table_Ptr->Activate_EMU_Handles[0]; // Sets up the overlays in the CPU4 OSEL. Initialized when
                                                        // OVC_CON.OVSTRT is set to 1.
    #if !defined(CPUTYPE_TC49X_N) 
    CPU5_OSEL.U = OMD_table_Ptr->Activate_EMU_Handles[0]; // Sets up the overlays in the CPU5 OSEL. Initialized when
                                                          // OVC_CON.OVSTRT is set to 1. - only for TC49x/TC4Dx, TC49x_N has only 5 OSEL registers
    #endif
  #endif

  /* Start the OVC for the CPU 0-5 in the SCU_OVCCON register and invalidate the DMI cache.
  -> OVCx_RABRy.OVEN set for the CPUs defined in the OVCx_OSEL registers */
  // The OVCCON register is not protected by the safety endinit module starting from the TC3xx family.
  SCU_OVCCON.U = 0x0005003F; // OVC_OVSTRT, DCINVAL and OVC_CSEL0 to OVC_CSEL5

#else
  #error "Undefined cpu type in the function Set_Calibration_Handles() !"
#endif
}

/** Returns 1 if no calibration handles is active. This means the ECU is in RP mode.*/
uint8 Check_Calibration_Handles_in_RP_mode(void)
{
#if (defined CPUCLASS_TC4XX)
  #if defined(CPUTYPE_TC46X) || defined(CPUTYPE_TC48X)  // Support CPU[0..3]
  if (CPU0_OSEL.U == 0 && CPU1_OSEL.U == 0 && CPU2_OSEL.U == 0 && CPU3_OSEL.U == 0)
  #elif defined CPUTYPE_TC49X_N // Support CPU[0..4]
  if (CPU0_OSEL.U == 0 && CPU1_OSEL.U == 0 && CPU2_OSEL.U == 0 && CPU3_OSEL.U == 0 && CPU4_OSEL.U == 0)
  #else
  if (CPU0_OSEL.U == 0 && CPU1_OSEL.U == 0 && CPU2_OSEL.U == 0 && CPU3_OSEL.U == 0 && CPU4_OSEL.U == 0 && CPU5_OSEL.U == 0)
  #endif
#else
  #error "Undefined OVCx_OSEL test in Check_Calibration_Handles_in_RP_mode() !"
#endif
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

/** This function should invalidate the data cache (DMI) for all the cores.
If this function is called periodically it should ensure that the calibrated parameters value will get updated inside
the cpu cache. Note: If data cache contains data modified by the cpu, it has to be written back and invalidated by the
user ! */
void dataCacheInvalidate(void)
{
#if (defined CPUCLASS_TC4XX)
  /* Invalidate the data cache (DMI) for the CPU 0-5 in the SCU_OVCCON register. */
  // The OVCCON register is not protected by the safety endinit module starting from the TC3xx family.
  SCU_OVCCON.U = 0x0004003F; // DCINVAL and OVC_CSEL0 to OVC_CSEL5

#else
  #error "Undefined cpu type in the function dataCacheInvalidate() !"
#endif
}

static uint32 getOmaskFromOvlSize(uint32 size)
{
  /* Calculating the OMASK value corresponding to the requested handle size. */


  uint32 shiftMask = 0xFFFFFFE0; // lower 5 Bits (Bit 0 .. Bit 4) are always zero
  while ((shiftMask & size) != 0)
  {
    shiftMask <<= 1;
  }
  return (shiftMask >> 1) & 0x0FFFFFE0;

}