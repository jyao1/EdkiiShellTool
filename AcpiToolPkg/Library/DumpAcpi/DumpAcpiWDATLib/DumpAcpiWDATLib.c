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
#include <IndustryStandard/WatchdogActionTable.h>

VOID
DumpAcpiWdatActionTable (
  EFI_ACPI_WATCHDOG_ACTION_1_0_WATCHDOG_ACTION_INSTRUCTION_ENTRY *WatchdogActionTable
  )
{
  if (WatchdogActionTable == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       Watchdog Action Table                                             *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  Watchdog Action Table address .......................... 0x%016lx\n" :
    L"  Watchdog Action Table address .......................... 0x%08x\n",
    WatchdogActionTable
    );
  Print (
    L"    Watchdog Action ...................................... 0x%02x\n",
    WatchdogActionTable->WatchdogAction
    );
  switch (WatchdogActionTable->WatchdogAction) {
  case EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_RESET:
    Print (
      L"      Reset\n"
      );
    break;
  case EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_QUERY_CURRENT_COUNTDOWN_PERIOD:
    Print (
      L"      Query Current Countdown Period\n"
      );
    break;
  case EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_QUERY_COUNTDOWN_PERIOD:
    Print (
      L"      Query Countdown Period\n"
      );
    break;
  case EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_SET_COUNTDOWN_PERIOD:
    Print (
      L"      Set Countdown Period\n"
      );
    break;
  case EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_QUERY_RUNNING_STATE:
    Print (
      L"      Query Running State\n"
      );
    break;
  case EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_SET_RUNNING_STATE:
    Print (
      L"      Set Running State\n"
      );
    break;
  case EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_QUERY_STOPPED_STATE:
    Print (
      L"      Query Stopped State\n"
      );
    break;
  case EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_SET_STOPPED_STATE:
    Print (
      L"      Set Stopped State\n"
      );
    break;
  case EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_QUERY_REBOOT:
    Print (
      L"      Query Reboot\n"
      );
    break;
  case EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_SET_REBOOT:
    Print (
      L"      Set Reboot\n"
      );
    break;
  case EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_QUERY_SHUTDOWN:
    Print (
      L"      Query Shutdown\n"
      );
    break;
  case EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_SET_SHUTDOWN:
    Print (
      L"      Set Shutdown\n"
      );
    break;
  case EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_QUERY_WATCHDOG_STATUS:
    Print (
      L"      Query Watchdog Status\n"
      );
    break;
  case EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_SET_WATCHDOG_STATUS:
    Print (
      L"      Set Watchdog Status\n"
      );
    break;
  default:
    break;
  }
  Print (
    L"    Instruction Flags .................................... 0x%02x\n",
    WatchdogActionTable->InstructionFlags
    );
  switch (WatchdogActionTable->InstructionFlags) {
  case EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_READ_VALUE:
    Print (
      L"      Read Value\n"
      );
    break;
  case EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_READ_COUNTDOWN:
    Print (
      L"      Read Countdown\n"
      );
    break;
  case EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_WRITE_VALUE:
    Print (
      L"      Write Value\n"
      );
    break;
  case EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_WRITE_COUNTDOWN:
    Print (
      L"      Write Countdown\n"
      );
    break;
  case EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_PRESERVE_REGISTER:
    Print (
      L"      Preserve Register\n"
      );
    break;
  default:
    break;
  }
  
  Print (
    L"    Register Region\n"
    );
  DumpAcpiGAddressStructure ((EFI_ACPI_3_0_GENERIC_ADDRESS_STRUCTURE *)&(WatchdogActionTable->RegisterRegion));

  Print (
    L"    Value ................................................ 0x%08x\n",
    WatchdogActionTable->Value
    );
  Print (
    L"    Mask ................................................. 0x%08x\n",
    WatchdogActionTable->Mask
    );
  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
EFIAPI
DumpAcpiWDAT (
  VOID  *Table
  )
{
  EFI_ACPI_WATCHDOG_ACTION_1_0_TABLE                  *Wdat;
  EFI_ACPI_WATCHDOG_ACTION_1_0_WATCHDOG_ACTION_INSTRUCTION_ENTRY  *WatchdogActionTable;
  UINTN                       Index;

  Wdat = Table;
  if (Wdat == NULL) {
    return;
  }
  
  //
  // Dump Wdat table
  //
  Print (
    L"*****************************************************************************\n"
    L"*         Watchdog Action Table                                             *\n"
    L"*****************************************************************************\n"
    );
    
  if (GetAcpiDumpPropertyDumpData()) {
    DumpAcpiHex (Wdat->Header.Length, Wdat);
  }
  
  if (!GetAcpiDumpPropertyDumpVerb()) {
    goto Done;
  }

  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"WDAT address ............................................. 0x%016lx\n" :
    L"WDAT address ............................................. 0x%08x\n",
    Wdat
    );
  
  DumpAcpiTableHeader(&(Wdat->Header));
  
  Print (
    L"  Table Contents:\n"
    );
  
  Print (
    L"    Watchdog Header Length ............................... 0x%08x\n",
    Wdat->WatchdogHeaderLength
    );
  Print (
    L"    PCI Segment .......................................... 0x%04x\n",
    Wdat->PCISegment
    );
  Print (
    L"    PCI Bus Number ....................................... 0x%02x\n",
    Wdat->PCIBusNumber
    );
  Print (
    L"    PCI Device Number .................................... 0x%02x\n",
    Wdat->PCIDeviceNumber
    );
  Print (
    L"    PCI Function Number .................................. 0x%02x\n",
    Wdat->PCIFunctionNumber
    );
  Print (
    L"    Timer Period ......................................... 0x%08x\n",
    Wdat->TimerPeriod
    );
  Print (
    L"    Max Count ............................................ 0x%08x\n",
    Wdat->MaxCount
    );
  Print (
    L"    Min Count ............................................ 0x%08x\n",
    Wdat->MinCount
    );
  Print (
    L"    Watchdog Flags ....................................... 0x%02x\n",
    Wdat->WatchdogFlags
    );
  Print (
    L"      Watchdog Enabled ................................... 0x%02x\n",
    Wdat->WatchdogFlags & EFI_ACPI_WDAT_1_0_WATCHDOG_ENABLED
    );
  Print (
    L"      Watchdog Stopped In Sleep State .................... 0x%02x\n",
    Wdat->WatchdogFlags & EFI_ACPI_WDAT_1_0_WATCHDOG_STOPPED_IN_SLEEP_STATE
    );
  Print (
    L"    Number Watchdog Instruction Entries .................. 0x%08x\n",
    Wdat->NumberWatchdogInstructionEntries
    );

  WatchdogActionTable = (EFI_ACPI_WATCHDOG_ACTION_1_0_WATCHDOG_ACTION_INSTRUCTION_ENTRY *)(Wdat + 1);
  for (Index = 0; Index < Wdat->NumberWatchdogInstructionEntries; Index++) {
    DumpAcpiWdatActionTable (WatchdogActionTable);
    WatchdogActionTable++;
  }

Done:
  Print (         
    L"*****************************************************************************\n\n"
    );
  
  return;
}

EFI_STATUS
EFIAPI
DumpAcpiWDATLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return RegisterDumpAcpiTable (EFI_ACPI_3_0_WATCHDOG_ACTION_TABLE_SIGNATURE, DumpAcpiWDAT);
}

