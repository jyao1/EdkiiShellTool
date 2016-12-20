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
//#include <IndustryStandard/IOVirtualizationReportingStructure.h>

VOID
EFIAPI
DumpAcpiIVRS (
  VOID  *Table
  )
{
  EFI_ACPI_DESCRIPTION_HEADER                            *Ivrs;

  Ivrs = Table;
  if (Ivrs == NULL) {
    return;
  }
  
  //
  // Dump Ivrs table
  //
  Print (
    L"*****************************************************************************\n"
    L"*         IO Virtualization Reporting Structure                             *\n"
    L"*****************************************************************************\n"
    );
    
  if (GetAcpiDumpPropertyDumpData()) {
    DumpAcpiHex (Ivrs->Length, Ivrs);
  }
  
  if (!GetAcpiDumpPropertyDumpVerb()) {
    goto Done;
  }

  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"IVRS address ............................................. 0x%016lx\n" :
    L"IVRS address ............................................. 0x%08x\n",
    Ivrs
    );
  
  DumpAcpiTableHeader(Ivrs);

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
DumpAcpiIVRSLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return RegisterDumpAcpiTable (EFI_ACPI_4_0_IO_VIRTUALIZATION_REPORTING_STRUCTURE_SIGNATURE, DumpAcpiIVRS);
}
