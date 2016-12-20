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
#include <IndustryStandard/WatchdogResourceTable.h>

VOID
DumpAcpiWdrtv1 (
  EFI_ACPI_WATCHDOG_RESOURCE_1_0_TABLE                         *Wdrtv1
  )
{
  if (Wdrtv1 == NULL) {
    return;
  }
  
  Print (
    L"    Control Register Address\n"
    );
  DumpAcpiGAddressStructure ((EFI_ACPI_3_0_GENERIC_ADDRESS_STRUCTURE *)&(Wdrtv1->ControlRegisterAddress));
 
  Print (
    L"    Count Register Address\n"
    );
  DumpAcpiGAddressStructure ((EFI_ACPI_3_0_GENERIC_ADDRESS_STRUCTURE *)&(Wdrtv1->CountRegisterAddress));

  Print (
    L"    PCI Device ID ........................................ 0x%04x\n",
    Wdrtv1->PCIDeviceID
    );
  Print (
    L"    PCI Vendor ID ........................................ 0x%04x\n",
    Wdrtv1->PCIVendorID
    );
  Print (
    L"    PCI Bus Number ....................................... 0x%02x\n",
    Wdrtv1->PCIBusNumber
    );
  Print (
    L"    PCI Device Number .................................... 0x%02x\n",
    Wdrtv1->PCIDeviceNumber
    );
  Print (
    L"    PCI Function Number .................................. 0x%02x\n",
    Wdrtv1->PCIFunctionNumber
    );
  Print (
    L"    PCI Segment .......................................... 0x%02x\n",
    Wdrtv1->PCISegment
    );
  Print (
    L"    Max Count ............................................ 0x%04x\n",
    Wdrtv1->MaxCount
    );
  Print (
    L"    Units ................................................ 0x%02x\n",
    Wdrtv1->Units
    );
  switch (Wdrtv1->Units) {
  case EFI_ACPI_WDRT_1_0_COUNT_UNIT_1_SEC_PER_COUNT:
    Print (
      L"      1 seconds/count\n"
      );
    break;
  case EFI_ACPI_WDRT_1_0_COUNT_UNIT_100_MILLISEC_PER_COUNT:
    Print (
      L"      100 milliseconds/count\n"
      );
    break;
  case EFI_ACPI_WDRT_1_0_COUNT_UNIT_10_MILLISEC_PER_COUNT:
    Print (
      L"      10 milliseconds/count\n"
      );
    break;
  default:
    break;
  }

  return;
}

VOID
EFIAPI
DumpAcpiWDRT (
  VOID  *Table
  )
{
  EFI_ACPI_WATCHDOG_RESOURCE_1_0_TABLE                            *Wdrt;

  Wdrt = Table;
  if (Wdrt == NULL) {
    return;
  }
  
  //
  // Dump Wdrt table
  //
  Print (
    L"*****************************************************************************\n"
    L"*         Watchdog Resource Table                                           *\n"
    L"*****************************************************************************\n"
    );
    
  if (GetAcpiDumpPropertyDumpData()) {
    DumpAcpiHex (Wdrt->Header.Length, Wdrt);
  }
  
  if (!GetAcpiDumpPropertyDumpVerb()) {
    goto Done;
  }

  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"WDRT address ............................................. 0x%016lx\n" :
    L"WDRT address ............................................. 0x%08x\n",
    Wdrt
    );
  
  DumpAcpiTableHeader(&(Wdrt->Header));
  
  Print (
    L"  Table Contents:\n"
    );

  DumpAcpiWdrtv1 (Wdrt);

Done:
  Print (         
    L"*****************************************************************************\n\n"
    );
  
  return;
}

EFI_STATUS
EFIAPI
DumpAcpiWDRTLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return RegisterDumpAcpiTable (EFI_ACPI_3_0_WATCHDOG_RESOURCE_TABLE_SIGNATURE, DumpAcpiWDRT);
}

