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
DumpAcpiECDT (
  VOID  *Table
  )
{
  EFI_ACPI_2_0_EMBEDDED_CONTROLLER_BOOT_RESOURCES_TABLE                            *Ecdt;

  Ecdt = Table;
  if (Ecdt == NULL) {
    return;
  }
  
  //
  // Dump Ecdt table
  //
  Print (
    L"*****************************************************************************\n"
    L"*         Embedded Controller Boot Resources Table                          *\n"
    L"*****************************************************************************\n"
    );
    
  if (GetAcpiDumpPropertyDumpData()) {
    DumpAcpiHex (Ecdt->Header.Length, Ecdt);
  }
  
  if (!GetAcpiDumpPropertyDumpVerb()) {
    goto Done;
  }

  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"ECDT address ............................................. 0x%016lx\n" :
    L"ECDT address ............................................. 0x%08x\n",
    Ecdt
    );
  
  DumpAcpiTableHeader(&(Ecdt->Header));
  
  Print (
    L"  Table Contents:\n"
    );
   
  Print (
    L"    EC_CONTROL\n"
    );
  DumpAcpiGAddressStructure ((EFI_ACPI_3_0_GENERIC_ADDRESS_STRUCTURE *)&(Ecdt->EcControl));
   
  Print (
    L"    EC_DATA\n"
    );
  DumpAcpiGAddressStructure ((EFI_ACPI_3_0_GENERIC_ADDRESS_STRUCTURE *)&(Ecdt->EcData));

  Print (
    L"    UID .................................................. 0x%08x\n",
    Ecdt->Uid
    );
  Print (
    L"    GPE_BIT .............................................. 0x%02x\n",
    Ecdt->GpeBit
    );
  Print (
    L"    EC_ID ................................................ '%a'\n",
    (Ecdt + 1)
    );

Done:
  Print (         
    L"*****************************************************************************\n\n"
    );
  
  return;
}

EFI_STATUS
EFIAPI
DumpAcpiECDTLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return RegisterDumpAcpiTable (EFI_ACPI_2_0_EMBEDDED_CONTROLLER_BOOT_RESOURCES_TABLE_SIGNATURE, DumpAcpiECDT);
}
