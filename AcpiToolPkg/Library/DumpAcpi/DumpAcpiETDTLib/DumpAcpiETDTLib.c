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

#pragma pack(1)

typedef struct {
  EFI_ACPI_DESCRIPTION_HEADER    Header;
} ETDT_TABLE;

#pragma pack()

VOID
EFIAPI
DumpAcpiETDT (
  VOID  *Table
  )
{
  ETDT_TABLE                            *Etdt;

  Etdt = Table;
  if (Etdt == NULL) {
    return;
  }
  
  //
  // Dump Etdt table
  //
  Print (
    L"*****************************************************************************\n"
    L"*         Event Timer Description Table                                     *\n"
    L"*****************************************************************************\n"
    );
    
  if (GetAcpiDumpPropertyDumpData()) {
    DumpAcpiHex (Etdt->Header.Length, Etdt);
  }
  
  if (!GetAcpiDumpPropertyDumpVerb()) {
    goto Done;
  }

  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"ETDT address ............................................. 0x%016lx\n" :
    L"ETDT address ............................................. 0x%08x\n",
    Etdt
    );
  
  DumpAcpiTableHeader(&(Etdt->Header));
  
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
DumpAcpiETDTLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return RegisterDumpAcpiTable (EFI_ACPI_2_0_EVENT_TIMER_DESCRIPTION_TABLE_SIGNATURE, DumpAcpiETDT);
}
