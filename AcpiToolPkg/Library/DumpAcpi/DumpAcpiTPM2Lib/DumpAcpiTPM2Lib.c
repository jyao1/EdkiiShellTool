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
#include <Library/IoLib.h>
#include <Library/UefiLib.h>
#include <Library/DumpAcpiTableFuncLib.h>
#include <IndustryStandard/Acpi.h>
#include <IndustryStandard/Tpm2Acpi.h>

UINT64
InternalMmioRead64 (
  UINTN  Address
  )
{
  return (UINT64)MmioRead32 (Address) + LShiftU64 ((UINT64)MmioRead32 (Address + sizeof(UINT32)), 32);
}

VOID
DumpAcpiTpm2ControlArea (
  EFI_TPM2_ACPI_CONTROL_AREA                     *ControlArea
  )
{
  if (ControlArea == NULL) {
    return ;
  }

  Print (
    L"  ***************************************************************************\n"
    L"  *         TPM2 Control Area                                               *\n"
    L"  ***************************************************************************\n"
    );

  Print (
    L"    Error ................................................ 0x%08x\n",
    MmioRead32 ((UINTN)&ControlArea->Error)
    );
  Print (
    L"    Cancel ............................................... 0x%08x\n",
    MmioRead32 ((UINTN)&ControlArea->Cancel)
    );
  Print (
    L"    Start ................................................ 0x%08x\n",
    MmioRead32 ((UINTN)&ControlArea->Start)
    );
  Print (
    L"    Interrupt Control .................................... 0x%016lx\n",
    InternalMmioRead64 ((UINTN)&ControlArea->InterruptControl)
    );
  Print (
    L"    Command Size ......................................... 0x%08x\n",
    MmioRead32 ((UINTN)&ControlArea->CommandSize)
    );
  Print (
    L"    Command .............................................. 0x%016lx\n",
    InternalMmioRead64 ((UINTN)&ControlArea->Command)
    );
  Print (
    L"    Response Size ........................................ 0x%08x\n",
    MmioRead32 ((UINTN)&ControlArea->ResponseSize)
    );
  Print (
    L"    Response ............................................. 0x%016lx\n",
    InternalMmioRead64 ((UINTN)&ControlArea->Response)
    );

  Print (         
    L"  ***************************************************************************\n\n"
    );
}

VOID
EFIAPI
DumpAcpiTPM2 (
  VOID  *Table
  )
{
  EFI_TPM2_ACPI_TABLE                            *Tpm2;

  Tpm2 = Table;
  if (Tpm2 == NULL) {
    return;
  }
  
  //
  // Dump Tpm2 table
  //
  Print (
    L"*****************************************************************************\n"
    L"*         Trusted Computing Platform 2 Table                                *\n"
    L"*****************************************************************************\n"
    );
    
  if (GetAcpiDumpPropertyDumpData()) {
    DumpAcpiHex (Tpm2->Header.Length, Tpm2);
  }
  
  if (!GetAcpiDumpPropertyDumpVerb()) {
    goto Done;
  }

  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"TPM2 address ............................................. 0x%016lx\n" :
    L"TPM2 address ............................................. 0x%08x\n",
    Tpm2
    );
  
  DumpAcpiTableHeader(&(Tpm2->Header));
  
  Print (
    L"  Table Contents:\n"
    );
  Print (
    L"    Flags ................................................ 0x%08x\n",
    ((EFI_TPM2_ACPI_TABLE *)Tpm2)->Flags
    );
  Print (
    L"    Address Of Control Area .............................. 0x%016lx\n",
    Tpm2->AddressOfControlArea
    );
  Print (
    L"    Start Method ......................................... 0x%08x\n",
    Tpm2->StartMethod
    );
  switch (Tpm2->StartMethod) {
  case EFI_TPM2_ACPI_TABLE_START_METHOD_ACPI:
    Print (
      L"      ACPI\n"
      );
    break;
  case EFI_TPM2_ACPI_TABLE_START_METHOD_TIS:
    Print (
      L"      TIS\n"
      );
  case EFI_TPM2_ACPI_TABLE_START_METHOD_COMMAND_RESPONSE_BUFFER_INTERFACE:
    Print (
      L"      CRB\n"
      );
  case EFI_TPM2_ACPI_TABLE_START_METHOD_COMMAND_RESPONSE_BUFFER_INTERFACE_WITH_ACPI:
    Print (
      L"      CRB with ACPI\n"
      );
    break;
  }

  DumpAcpiTpm2ControlArea ((EFI_TPM2_ACPI_CONTROL_AREA *)(UINTN)Tpm2->AddressOfControlArea);

Done:
  Print (         
    L"*****************************************************************************\n\n"
    );
  
  return;
}

EFI_STATUS
EFIAPI
DumpAcpiTPM2LibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return RegisterDumpAcpiTable (EFI_ACPI_5_0_TRUSTED_COMPUTING_PLATFORM_2_TABLE_SIGNATURE, DumpAcpiTPM2);
}
