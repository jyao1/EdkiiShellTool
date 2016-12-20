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
//#include <IndustryStandard/WindowsAcpiEmulatedDevicesTable.h>

VOID
EFIAPI
DumpAcpiWAET (
  VOID  *Table
  )
{
  EFI_ACPI_DESCRIPTION_HEADER                  *Waet;

  Waet = Table;
  if (Waet == NULL) {
    return;
  }
  
  //
  // Dump Waet table
  //
  Print (
    L"*****************************************************************************\n"
    L"*         Windows ACPI Emulated Devices Table                               *\n"
    L"*****************************************************************************\n"
    );
    
  if (GetAcpiDumpPropertyDumpData()) {
    DumpAcpiHex (Waet->Length, Waet);
  }
  
  if (!GetAcpiDumpPropertyDumpVerb()) {
    goto Done;
  }

  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"WAET address ............................................. 0x%016lx\n" :
    L"WAET address ............................................. 0x%08x\n",
    Waet
    );
  
  DumpAcpiTableHeader(Waet);
  
  Print (
    L"  Table Contents:\n"
    );
Done:
  Print (         
    L"*****************************************************************************\n\n"
    );
  
  return;
}

EFI_STATUS
EFIAPI
DumpAcpiWAETLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return RegisterDumpAcpiTable (EFI_ACPI_4_0_WINDOWS_ACPI_ENLIGHTENMENT_TABLE_SIGNATURE, DumpAcpiWAET);
}

