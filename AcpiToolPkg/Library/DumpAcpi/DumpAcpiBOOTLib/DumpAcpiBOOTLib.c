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
//#include <IndustryStandard/SimpleBootFlag.h>

VOID
EFIAPI
DumpAcpiBOOT (
  VOID  *Table
  )
{
  EFI_ACPI_DESCRIPTION_HEADER                            *Boot;

  Boot = Table;
  if (Boot == NULL) {
    return;
  }
  
  //
  // Dump Boot table
  //
  Print (
    L"*****************************************************************************\n"
    L"*         Simple Boot Flag Table                                            *\n"
    L"*****************************************************************************\n"
    );
    
  if (GetAcpiDumpPropertyDumpData()) {
    DumpAcpiHex (Boot->Length, Boot);
  }
  
  if (!GetAcpiDumpPropertyDumpVerb()) {
    goto Done;
  }

  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"BOOT address ............................................. 0x%016lx\n" :
    L"BOOT address ............................................. 0x%08x\n",
    Boot
    );
  
  DumpAcpiTableHeader(Boot);

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
DumpAcpiBOOTLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return RegisterDumpAcpiTable (EFI_ACPI_2_0_SIMPLE_BOOT_FLAG_TABLE_SIGNATURE, DumpAcpiBOOT);
}
