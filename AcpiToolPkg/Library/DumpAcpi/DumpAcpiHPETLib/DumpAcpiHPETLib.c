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
#include <IndustryStandard/HighPrecisionEventTimerTable.h>

VOID
EFIAPI
DumpAcpiHPET (
  VOID  *Table
  )
{
  EFI_ACPI_HIGH_PRECISION_EVENT_TIMER_TABLE_HEADER                            *Hpet;

  Hpet = Table;
  if (Hpet == NULL) {
    return;
  }
  
  //
  // Dump Hpet table
  //
  Print (
    L"*****************************************************************************\n"
    L"*         High Precision Event Timers Description Table                     *\n"
    L"*****************************************************************************\n"
    );
    
  if (GetAcpiDumpPropertyDumpData()) {
    DumpAcpiHex (Hpet->Header.Length, Hpet);
  }
  
  if (!GetAcpiDumpPropertyDumpVerb()) {
    goto Done;
  }

  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"HPET address ............................................. 0x%016lx\n" :
    L"HPET address ............................................. 0x%08x\n",
    Hpet
    );
  
  DumpAcpiTableHeader(&(Hpet->Header));
  
  Print (
    L"  Table Contents:\n"
    );
  Print (
    L"    Event Timer Block ID ................................. 0x%08x\n",
    Hpet->EventTimerBlockId
    );
  Print (
    L"      Hardware Rev ID .................................... 0x%08x\n",
    Hpet->EventTimerBlockId & 0xFF
    );
  Print (
    L"      Number of Comparators in 1st Timer Block ........... 0x%08x\n",
    Hpet->EventTimerBlockId & 0x1F00
    );
  Print (
    L"      COUNT_SIZE_CAP counter size ........................ 0x%08x\n",
    Hpet->EventTimerBlockId & 0x2000
    );
  Print (
    L"      LegacyReplacement IRQ Routing Capable .............. 0x%08x\n",
    Hpet->EventTimerBlockId & 0x8000
    );
  Print (
    L"      PCI Vendor ID of 1st Timer Block ................... 0x%08x\n",
    Hpet->EventTimerBlockId & 0xFFFF0000
    );
  
  Print (
    L"    BASE_ADDRESS Lower 32-bit\n"
    );
  DumpAcpiGAddressStructure ((EFI_ACPI_3_0_GENERIC_ADDRESS_STRUCTURE *)&(Hpet->BaseAddressLower32Bit));

  Print (
    L"    HPET Number .......................................... 0x%02x\n",
    Hpet->HpetNumber
    );
  Print (
    L"    Main Counter Minimum Clock_tick in Periodic Mode ..... 0x%04x\n",
    Hpet->MainCounterMinimumClockTickInPeriodicMode
    );
  Print (
    L"    Page Protection And OEM Attribute .................... 0x%02x\n",
    Hpet->PageProtectionAndOemAttribute
    );
  switch (Hpet->PageProtectionAndOemAttribute & 0xF) {
  case EFI_ACPI_NO_PAGE_PROTECTION:
    Print (
      L"      no guarantee for page protection\n"
      );
    break;
  case EFI_ACPI_4KB_PAGE_PROTECTION:
    Print (
      L"      4KB page protected\n"
      );
    break;
  case EFI_ACPI_64KB_PAGE_PROTECTION:
    Print (
      L"      64KB page protected\n"
      );
    break;
  default:
    break;
  }

Done:
  Print (         
    L"*****************************************************************************\n\n"
    );
  
  return;
}

EFI_STATUS
EFIAPI
DumpAcpiHPETLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return RegisterDumpAcpiTable (EFI_ACPI_3_0_HIGH_PRECISION_EVENT_TIMER_TABLE_SIGNATURE, DumpAcpiHPET);
}
