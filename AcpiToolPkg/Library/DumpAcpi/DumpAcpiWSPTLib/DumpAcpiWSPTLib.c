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
} WSPT_TABLE;

#pragma pack()

VOID
EFIAPI
DumpAcpiWSPT (
  VOID  *Table
  )
{
  WSPT_TABLE                            *Wspt;

  Wspt = Table;
  if (Wspt == NULL) {
    return;
  }
  
  //
  // Dump Wspt table
  //
  Print (
    L"*****************************************************************************\n"
    L"*         Windows Specific Properties Table                                 *\n"
    L"*****************************************************************************\n"
    );
    
  if (GetAcpiDumpPropertyDumpData()) {
    DumpAcpiHex (Wspt->Header.Length, Wspt);
  }
  
  if (!GetAcpiDumpPropertyDumpVerb()) {
    goto Done;
  }

  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"WSPT address ............................................. 0x%016lx\n" :
    L"WSPT address ............................................. 0x%08x\n",
    Wspt
    );
  
  DumpAcpiTableHeader(&(Wspt->Header));
  
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
DumpAcpiWSPTLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return RegisterDumpAcpiTable (EFI_ACPI_3_0_WINDOWS_SPECIFIC_PROPERTIES_TABLE_SIGNATURE, DumpAcpiWSPT);
}
