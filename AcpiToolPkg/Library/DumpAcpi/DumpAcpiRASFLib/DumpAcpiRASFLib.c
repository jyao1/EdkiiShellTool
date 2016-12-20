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
DumpAcpiRASF (
  VOID  *Table
  )
{
  EFI_ACPI_5_0_RAS_FEATURE_TABLE                            *Rasf;
  UINTN                                                     Index;

  Rasf = Table;
  if (Rasf == NULL) {
    return;
  }
  
  //
  // Dump Rasf table
  //
  Print (
    L"*****************************************************************************\n"
    L"*         ACPI RAS Feature Table                                            *\n"
    L"*****************************************************************************\n"
    );
    
  if (GetAcpiDumpPropertyDumpData()) {
    DumpAcpiHex (Rasf->Header.Length, Rasf);
  }
  
  if (!GetAcpiDumpPropertyDumpVerb()) {
    goto Done;
  }

  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"RASF address ............................................. 0x%016lx\n" :
    L"RASF address ............................................. 0x%08x\n",
    Rasf
    );
  
  DumpAcpiTableHeader(&(Rasf->Header));
  
  Print (
    L"  Table Contents:\n"
    );
  Print (
    L"    Platform Communication Channel Identifier ............ "
    );
  for (Index = 0; Index < sizeof(Rasf->PlatformCommunicationChannelIdentifier); Index++) {
    Print (L"%02x", Rasf->PlatformCommunicationChannelIdentifier[Index]);
  }
  Print (L"\n");

Done:
  Print (         
    L"*****************************************************************************\n\n"
    );
  
  return;
}

EFI_STATUS
EFIAPI
DumpAcpiRASFLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return RegisterDumpAcpiTable (EFI_ACPI_5_0_ACPI_RAS_FEATURE_TABLE_SIGNATURE, DumpAcpiRASF);
}
