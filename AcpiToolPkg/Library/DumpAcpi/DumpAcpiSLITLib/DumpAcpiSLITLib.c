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
DumpAcpiSLIT (
  VOID  *Table
  )
{
  EFI_ACPI_3_0_SYSTEM_LOCALITY_DISTANCE_INFORMATION_TABLE_HEADER                            *Slit;
  UINT64                EntryIndexI;
  UINT64                EntryIndexJ;
  UINT8                 *Entry;

  Slit = Table;
  if (Slit == NULL) {
    return;
  }
  
  //
  // Dump Slit table
  //
  Print (
    L"*****************************************************************************\n"
    L"*         System Locality Distance Information Table                        *\n"
    L"*****************************************************************************\n"
    );
    
  if (GetAcpiDumpPropertyDumpData()) {
    DumpAcpiHex (Slit->Header.Length, Slit);
  }
  
  if (!GetAcpiDumpPropertyDumpVerb()) {
    goto Done;
  }

  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"SLIT address ............................................. 0x%016lx\n" :
    L"SLIT address ............................................. 0x%08x\n",
    Slit
    );
  
  DumpAcpiTableHeader(&(Slit->Header));
  
  Print (
    L"  Table Contents:\n"
    );
  Print (
    L"    Number Of System Localities .......................... 0x%016lx\n",
    Slit->NumberOfSystemLocalities
    );

  Entry = (UINT8 *)(Slit + 1);
  for (EntryIndexI = 0; EntryIndexI < Slit->NumberOfSystemLocalities; EntryIndexI++) {
    for (EntryIndexJ = 0; EntryIndexJ < Slit->NumberOfSystemLocalities; EntryIndexJ++) {
      Print (
        L"    Matrix entry (%016lxh,%016lxh) ... 0x%02x\n",
        *Entry
        );
      Entry++;
    }
  }

Done:
  Print (         
    L"*****************************************************************************\n\n"
    );
  
  return;
}

EFI_STATUS
EFIAPI
DumpAcpiSLITLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return RegisterDumpAcpiTable (EFI_ACPI_2_0_SYSTEM_LOCALITY_INFORMATION_TABLE_SIGNATURE, DumpAcpiSLIT);
}
