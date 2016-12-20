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
DumpAcpiSLIC (
  VOID  *Table
  )
{
  EFI_ACPI_DESCRIPTION_HEADER                            *Slic;

  Slic = Table;
  if (Slic == NULL) {
    return;
  }
  
  //
  // Dump Slic table
  //
  Print (
    L"*****************************************************************************\n"
    L"*         Microsoft Software Licensing Table                                *\n"
    L"*****************************************************************************\n"
    );
    
  if (GetAcpiDumpPropertyDumpData()) {
    DumpAcpiHex (Slic->Length, Slic);
  }
  
  if (!GetAcpiDumpPropertyDumpVerb()) {
    goto Done;
  }

  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"SLIC address ............................................. 0x%016lx\n" :
    L"SLIC address ............................................. 0x%08x\n",
    Slic
    );
  
  DumpAcpiTableHeader(Slic);
  
  Print (
    L"  Table Contents:\n"
    );

  Print (
    L"    Software Licensing Structure:\n"
    );
  DumpAcpiHex (Slic->Length - sizeof(EFI_ACPI_DESCRIPTION_HEADER), (VOID *)((UINTN)Slic + sizeof(EFI_ACPI_DESCRIPTION_HEADER)));

Done:
  Print (         
    L"*****************************************************************************\n\n"
    );
  
  return;
}

EFI_STATUS
EFIAPI
DumpAcpiSLICLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return RegisterDumpAcpiTable (EFI_ACPI_5_0_SOFTWARE_LICENSING_TABLE_SIGNATURE, DumpAcpiSLIC);
}
