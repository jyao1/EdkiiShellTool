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
DumpAcpiSBST (
  VOID  *Table
  )
{
  EFI_ACPI_2_0_SMART_BATTERY_DESCRIPTION_TABLE                            *Sbst;

  Sbst = Table;
  if (Sbst == NULL) {
    return;
  }
  
  //
  // Dump Sbst table
  //
  Print (
    L"*****************************************************************************\n"
    L"*         Smart Battery Description Table                                   *\n"
    L"*****************************************************************************\n"
    );
    
  if (GetAcpiDumpPropertyDumpData()) {
    DumpAcpiHex (Sbst->Header.Length, Sbst);
  }
  
  if (!GetAcpiDumpPropertyDumpVerb()) {
    goto Done;
  }

  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"SBST address ............................................. 0x%016lx\n" :
    L"SBST address ............................................. 0x%08x\n",
    Sbst
    );
  
  DumpAcpiTableHeader(&(Sbst->Header));
  
  Print (
    L"  Table Contents:\n"
    );
  Print (
    L"    Warning Energy Level ................................. 0x%08x\n",
    Sbst->WarningEnergyLevel
    );
  Print (
    L"    Low Energy Level ..................................... 0x%08x\n",
    Sbst->LowEnergyLevel
    );
  Print (
    L"    Critical Energy Level ................................ 0x%08x\n",
    Sbst->CriticalEnergyLevel
    );

Done:
  Print (         
    L"*****************************************************************************\n\n"
    );
  
  return;
}

EFI_STATUS
EFIAPI
DumpAcpiSBSTLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return RegisterDumpAcpiTable (EFI_ACPI_1_0_SMART_BATTERY_SPECIFICATION_TABLE_SIGNATURE, DumpAcpiSBST);
}
