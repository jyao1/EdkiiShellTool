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
#include <Uefi/UefiAcpiDataTable.h>
#include <Pi/PiSmmCommunicationAcpiTable.h>
#include <Protocol/SmmCommunication.h>

VOID
EFIAPI
DumpAcpiUEFI (
  VOID  *Table
  )
{
  EFI_ACPI_DATA_TABLE                            *Uefi;
  EFI_SMM_COMMUNICATION_ACPI_TABLE               *PiSmm;

  Uefi = Table;
  if (Uefi == NULL) {
    return;
  }
  
  //
  // Dump Uefi table
  //
  Print (
    L"*****************************************************************************\n"
    L"*         UEFI ACPI Data Table                                              *\n"
    L"*****************************************************************************\n"
    );
    
  if (GetAcpiDumpPropertyDumpData()) {
    DumpAcpiHex (Uefi->Header.Length, Uefi);
  }
  
  if (!GetAcpiDumpPropertyDumpVerb()) {
    goto Done;
  }

  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"UEFI address ............................................. 0x%016lx\n" :
    L"UEFI address ............................................. 0x%08x\n",
    Uefi
    );
  
  DumpAcpiTableHeader(&(Uefi->Header));
  
  Print (
    L"  Table Contents:\n"
    );
  Print (
    L"    Identifier ........................................... %g\n",
    &Uefi->Identifier
    );
  Print (
    L"    Data Offset .......................................... 0x%02x\n",
    Uefi->DataOffset
    );

  if (CompareGuid (&Uefi->Identifier, &gEfiSmmCommunicationProtocolGuid)) {
    PiSmm = Table;
    Print (
      L"    SwSmi Number ......................................... 0x%08x\n",
      PiSmm->SwSmiNumber
      );
    Print (
      L"    BufferPtrAddress ..................................... 0x%016lx\n",
      PiSmm->BufferPtrAddress
      );
  }

Done:
  Print (         
    L"*****************************************************************************\n\n"
    );
  
  return;
}

EFI_STATUS
EFIAPI
DumpAcpiUEFILibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return RegisterDumpAcpiTable (EFI_ACPI_4_0_UEFI_ACPI_DATA_TABLE_SIGNATURE, DumpAcpiUEFI);
}
