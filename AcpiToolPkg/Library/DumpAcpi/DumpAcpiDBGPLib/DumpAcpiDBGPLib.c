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
#include <IndustryStandard/DebugPortTable.h>

VOID
EFIAPI
DumpAcpiDBGP (
  VOID  *Table
  )
{
  EFI_ACPI_DEBUG_PORT_DESCRIPTION_TABLE                            *Dbgp;

  Dbgp = Table;
  if (Dbgp == NULL) {
    return;
  }
  
  //
  // Dump Dbgp table
  //
  Print (
    L"*****************************************************************************\n"
    L"*         Debug Port Table                                                  *\n"
    L"*****************************************************************************\n"
    );
    
  if (GetAcpiDumpPropertyDumpData()) {
    DumpAcpiHex (Dbgp->Header.Length, Dbgp);
  }
  
  if (!GetAcpiDumpPropertyDumpVerb()) {
    goto Done;
  }

  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"DBGP address ............................................. 0x%016lx\n" :
    L"DBGP address ............................................. 0x%08x\n",
    Dbgp
    );
  
  DumpAcpiTableHeader(&(Dbgp->Header));
  
  Print (
    L"  Table Contents:\n"
    );
  Print (
    L"    Interface Type ....................................... 0x%02x\n",
    Dbgp->InterfaceType
    );
  switch (Dbgp->InterfaceType) {
  case EFI_ACPI_DBGP_INTERFACE_TYPE_FULL_16550:
    Print (
      L"      full 16550 interface\n"
      );
    break;
  case EFI_ACPI_DBGP_INTERFACE_TYPE_16550_SUBSET_COMPATIBLE_WITH_MS_DBGP_SPEC:
    Print (
      L"      16550 subset interface compatible with Microsoft Debug Port Specification\n"
      );
    break;
  default:
    break;
  }
 
  Print (
    L"    Base Address\n"
    );
  DumpAcpiGAddressStructure ((EFI_ACPI_3_0_GENERIC_ADDRESS_STRUCTURE *)&(Dbgp->BaseAddress));

Done:
  Print (         
    L"*****************************************************************************\n\n"
    );
  
  return;
}

EFI_STATUS
EFIAPI
DumpAcpiDBGPLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return RegisterDumpAcpiTable (EFI_ACPI_2_0_DEBUG_PORT_TABLE_SIGNATURE, DumpAcpiDBGP);
}
