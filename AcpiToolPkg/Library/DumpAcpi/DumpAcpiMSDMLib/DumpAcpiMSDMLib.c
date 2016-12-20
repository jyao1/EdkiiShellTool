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
DumpAcpiMSDM (
  VOID  *Table
  )
{
  EFI_ACPI_DESCRIPTION_HEADER                            *Msdm;

  Msdm = Table;
  if (Msdm == NULL) {
    return;
  }
  
  //
  // Dump Msdm table
  //
  Print (
    L"*****************************************************************************\n"
    L"*         Microsoft Data Management Table                                   *\n"
    L"*****************************************************************************\n"
    );
    
  if (GetAcpiDumpPropertyDumpData()) {
    DumpAcpiHex (Msdm->Length, Msdm);
  }
  
  if (!GetAcpiDumpPropertyDumpVerb()) {
    goto Done;
  }

  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"MSDM address ............................................. 0x%016lx\n" :
    L"MSDM address ............................................. 0x%08x\n",
    Msdm
    );
  
  DumpAcpiTableHeader(Msdm);
  
  Print (
    L"  Table Contents:\n"
    );

  Print (
    L"    Software Licensing Structure:\n"
    );
  DumpAcpiHex (Msdm->Length - sizeof(EFI_ACPI_DESCRIPTION_HEADER), (VOID *)((UINTN)Msdm + sizeof(EFI_ACPI_DESCRIPTION_HEADER)));

Done:
  Print (         
    L"*****************************************************************************\n\n"
    );
  
  return;
}

EFI_STATUS
EFIAPI
DumpAcpiMSDMLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return RegisterDumpAcpiTable (EFI_ACPI_5_0_DATA_MANAGEMENT_TABLE_SIGNATURE, DumpAcpiMSDM);
}
