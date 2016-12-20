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
#include <IndustryStandard/WatchdogDescriptionTable.h>

VOID
EFIAPI
DumpAcpiWDDT (
  VOID  *Table
  )
{
  EFI_ACPI_1_0_WATCH_DOG_DESCRIPTION_TABLE                            *Wddt;

  Wddt = Table;
  if (Wddt == NULL) {
    return;
  }
  
  //
  // Dump Wddt table
  //
  Print (
    L"*****************************************************************************\n"
    L"*         Watchdog Description Table                                        *\n"
    L"*****************************************************************************\n"
    );
    
  if (GetAcpiDumpPropertyDumpData()) {
    DumpAcpiHex (Wddt->Header.Length, Wddt);
  }
  
  if (!GetAcpiDumpPropertyDumpVerb()) {
    goto Done;
  }

  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"WDDT address ............................................. 0x%016lx\n" :
    L"WDDT address ............................................. 0x%08x\n",
    Wddt
    );
  
  DumpAcpiTableHeader(&(Wddt->Header));
  
  Print (
    L"  Table Contents:\n"
    );
  Print (
    L"    Spec Version ......................................... 0x%04x\n",
    Wddt->SpecVersion
    );
  Print (
    L"    Table Version ........................................ 0x%04x\n",
    Wddt->TableVersion
    );
  Print (
    L"    Vid .................................................. 0x%04x\n",
    Wddt->Vid
    );

  Print (
    L"    Base Address\n"
    );
  DumpAcpiGAddressStructure ((EFI_ACPI_3_0_GENERIC_ADDRESS_STRUCTURE *)&(Wddt->BaseAddress));

  Print (
    L"    Timer Max Count ...................................... 0x%04x\n",
    Wddt->TimerMaxCount
    );
  Print (
    L"    Timer Min Count ...................................... 0x%04x\n",
    Wddt->TimerMinCount
    );
  Print (
    L"    Timer Count Period ................................... 0x%04x\n",
    Wddt->TimerCountPeriod
    );

  Print (
    L"    Status ............................................... 0x%04x\n",
    Wddt->Status
    );
  Print (
    L"      Available .......................................... 0x%04x\n",
    Wddt->Status & EFI_ACPI_WDDT_STATUS_AVAILABLE
    );
  Print (
    L"      Active ............................................. 0x%04x\n",
    Wddt->Status & EFI_ACPI_WDDT_STATUS_ACTIVE
    );
  Print (
    L"      Owned By OS (Or BIOS) .............................. 0x%04x\n",
    Wddt->Status & EFI_ACPI_WDDT_STATUS_OWNED_BY_OS
    );
  Print (
    L"      User Reset Event ................................... 0x%04x\n",
    Wddt->Status & EFI_ACPI_WDDT_STATUS_USER_RESET_EVENT
    );
  Print (
    L"      Watchdog Timer Event ............................... 0x%04x\n",
    Wddt->Status & EFI_ACPI_WDDT_STATUS_WDT_EVENT
    );
  Print (
    L"      Power Fail Event ................................... 0x%04x\n",
    Wddt->Status & EFI_ACPI_WDDT_STATUS_POWER_FAIL_EVENT
    );
  Print (
    L"      Unknown Reset Event ................................ 0x%04x\n",
    Wddt->Status & EFI_ACPI_WDDT_STATUS_UNKNOWN_RESET_EVENT
    );

  Print (
    L"    Capability ........................................... 0x%04x\n",
    Wddt->Capability
    );
  Print (
    L"      Auto Reset ......................................... 0x%04x\n",
    Wddt->Capability & EFI_ACPI_WDDT_CAPABILITY_AUTO_RESET
    );
  Print (
    L"      Alert Support ...................................... 0x%04x\n",
    Wddt->Capability & EFI_ACPI_WDDT_CAPABILITY_ALERT_SUPPORT
    );
  Print (
    L"      Platform Shutdown .................................. 0x%04x\n",
    Wddt->Capability & EFI_ACPI_WDDT_CAPABILITY_PLATFORM_SHUTDOWN
    );
  Print (
    L"      Immediate Shutdown ................................. 0x%04x\n",
    Wddt->Capability & EFI_ACPI_WDDT_CAPABILITY_IMMEDIATE_SHUTDOWN
    );
  Print (
    L"      BIOS Handoff Support ............................... 0x%04x\n",
    Wddt->Capability & EFI_ACPI_WDDT_CAPABILITY_BIOS_HANDOFF_SUPPORT
    );

Done:
  Print (         
    L"*****************************************************************************\n\n"
    );
  
  return;
}

EFI_STATUS
EFIAPI
DumpAcpiWDDTLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return RegisterDumpAcpiTable (EFI_ACPI_1_0_WDDT_SIGNATURE, DumpAcpiWDDT);
}
