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
DumpAcpiBGRT (
  VOID  *Table
  )
{
  EFI_ACPI_5_0_BOOT_GRAPHICS_RESOURCE_TABLE                            *Bgrt;

  Bgrt = Table;
  if (Bgrt == NULL) {
    return;
  }
  
  //
  // Dump Bgrt table
  //
  Print (
    L"*****************************************************************************\n"
    L"*         Boot Graphics Resource Table                                      *\n"
    L"*****************************************************************************\n"
    );
    
  if (GetAcpiDumpPropertyDumpData()) {
    DumpAcpiHex (Bgrt->Header.Length, Bgrt);
  }
  
  if (!GetAcpiDumpPropertyDumpVerb()) {
    goto Done;
  }

  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"BGRT address ............................................. 0x%016lx\n" :
    L"BGRT address ............................................. 0x%08x\n",
    Bgrt
    );
  
  DumpAcpiTableHeader(&(Bgrt->Header));
  
  Print (
    L"  Table Contents:\n"
    );
  Print (
    L"    Version .............................................. 0x%04x\n",
    Bgrt->Version
    );
  Print (
    L"    Status ............................................... 0x%02x\n",
    Bgrt->Status
    );
  Print (
    L"      Valid .............................................. 0x%02x\n",
    Bgrt->Status & EFI_ACPI_5_0_BGRT_STATUS_VALID
    );
  Print (
    L"    Image Type ........................................... 0x%02x\n",
    Bgrt->ImageType
    );
  switch (Bgrt->ImageType) {
  case EFI_ACPI_5_0_BGRT_IMAGE_TYPE_BMP:
    Print (
      L"      BMP\n"
      );
    break;
  default:
    break;
  }
  Print (
    L"    Image Address ........................................ 0x%016lx\n",
    Bgrt->ImageAddress
    );
  Print (
    L"    Image OffsetX ........................................ 0x%08x\n",
    Bgrt->ImageOffsetX
    );
  Print (
    L"    Image OffsetY ........................................ 0x%08x\n",
    Bgrt->ImageOffsetY
    );

Done:
  Print (         
    L"*****************************************************************************\n\n"
    );
  
  return;
}

EFI_STATUS
EFIAPI
DumpAcpiBGRTLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return RegisterDumpAcpiTable (EFI_ACPI_5_0_BOOT_GRAPHICS_RESOURCE_TABLE_SIGNATURE, DumpAcpiBGRT);
}
