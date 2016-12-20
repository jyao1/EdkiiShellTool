/** @file

Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Base.h>
#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiLib.h>
#include <Library/DumpAcpiTableFuncLib.h>
#include <IndustryStandard/Acpi.h>

VOID
EFIAPI
DumpAcpiFACS (
  VOID  *Table
  )
{
  EFI_ACPI_4_0_FIRMWARE_ACPI_CONTROL_STRUCTURE                            *Facs;
  UINT8                     *SignatureByte;

  Facs = Table;
  if (Facs == NULL) {
    return;
  }
  
  SignatureByte = (UINT8*)&Facs->Signature;
  
  //
  // Dump Facs table
  //
  Print (         
    L"*****************************************************************************\n"
    L"*         Firmware ACPI Control Structure                                   *\n"
    L"*****************************************************************************\n"
    );
    
  if (GetAcpiDumpPropertyDumpData()) {
    DumpAcpiHex (Facs->Length, Facs);
  }

  if (!GetAcpiDumpPropertyDumpVerb()) {
    goto Done;
  }

  Print (         
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"FACS address ............................................. 0x%016lx\n" :
    L"FACS address ............................................. 0x%08x\n",
    Facs
    );

  Print (         
    L"  Signature .............................................. '%c%c%c%c'\n",
    SignatureByte[0],
    SignatureByte[1],
    SignatureByte[2],
    SignatureByte[3]
    );
  Print (         
    L"  Length ................................................. 0x%08x\n",
    Facs->Length
    );
  Print (         
    L"  Hardware Signature ..................................... 0x%08x\n",
    Facs->HardwareSignature
    );
  Print (         
    L"  Firmware Waking Vector ................................. 0x%08x\n",
    Facs->FirmwareWakingVector
    );
  Print (         
    L"  Global Lock ............................................ 0x%08x\n",
    Facs->GlobalLock
    );
  Print (
    L"    Pending .............................................. 0x%08x\n",
    Facs->GlobalLock & 0x1
    );
  Print (
    L"    Owned ................................................ 0x%08x\n",
    Facs->GlobalLock & 0x2
    );
  Print (         
    L"  Flags .................................................. 0x%08x\n",
    Facs->Flags
    );
  Print (
    L"    S4BIOS_F ............................................. 0x%08x\n",
    Facs->Flags & EFI_ACPI_1_0_S4BIOS_F
    );
  if (Facs->Version >= EFI_ACPI_4_0_FIRMWARE_ACPI_CONTROL_STRUCTURE_VERSION) {
    Print (
      L"    64BIT_WAKE_SUPPORTED_F ............................... 0x%08x\n",
      Facs->Flags & EFI_ACPI_4_0_64BIT_WAKE_SUPPORTED_F
      );
  }
  
  if (Facs->Version >= EFI_ACPI_2_0_FIRMWARE_ACPI_CONTROL_STRUCTURE_VERSION) {
    Print (       
      L"  64bit Firmware Waking Vector ........................... 0x%016lx\n",
      Facs->XFirmwareWakingVector
      );
    Print (       
      L"  Version ................................................ 0x%02x\n",
      Facs->Version
      );
    if (Facs->Version >= EFI_ACPI_4_0_FIRMWARE_ACPI_CONTROL_STRUCTURE_VERSION) {
      Print (       
        L"  OspmFlags .............................................. 0x%08x\n",
        Facs->OspmFlags
        );
      Print (
        L"    OSPM_64BIT_WAKE_F .................................... 0x%08x\n",
        Facs->OspmFlags & EFI_ACPI_4_0_OSPM_64BIT_WAKE__F
        );
    }
  }

Done:
  Print (       
    L"*****************************************************************************\n\n"
    );

  return;
}

