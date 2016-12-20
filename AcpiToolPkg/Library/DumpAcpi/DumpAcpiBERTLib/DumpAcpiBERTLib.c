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
DumpBootErrorRegion (
  EFI_ACPI_4_0_BOOT_ERROR_REGION_STRUCTURE *BootErrorRegion,
  UINTN                                    BootErrorRegionLength
  )
{
  if (BootErrorRegion == NULL) {
    return;
  }

  Print (
    L"  ***************************************************************************\n"
    L"  *       Boot Error Region                                                 *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    L"    Block Status ......................................... 0x%08x\n",
    *(UINT32 *)&BootErrorRegion->BlockStatus
    );
  Print (
    L"      Uncorrectable Error Valid .......................... 0x%08x\n",
    BootErrorRegion->BlockStatus.UncorrectableErrorValid
    );
  Print (
    L"      Correctable Error Valid ............................ 0x%08x\n",
    BootErrorRegion->BlockStatus.CorrectableErrorValid
    );
  Print (
    L"      Multiple Uncorrectable Errors ...................... 0x%08x\n",
    BootErrorRegion->BlockStatus.MultipleUncorrectableErrors
    );
  Print (
    L"      Multiple Correctable Errors ........................ 0x%08x\n",
    BootErrorRegion->BlockStatus.MultipleCorrectableErrors
    );
  Print (
    L"      Error Data Entry Count ............................. 0x%08x\n",
    BootErrorRegion->BlockStatus.ErrorDataEntryCount
    );
  Print (
    L"    Raw Data Offset ...................................... 0x%08x\n",
    BootErrorRegion->RawDataOffset
    );
  Print (
    L"    Raw Data Length ...................................... 0x%08x\n",
    BootErrorRegion->RawDataLength
    );
  Print (
    L"    Data Length .......................................... 0x%08x\n",
    BootErrorRegion->DataLength
    );
  Print (
    L"    Error Severity ....................................... 0x%08x\n",
    BootErrorRegion->ErrorSeverity
    );
  switch (BootErrorRegion->ErrorSeverity) {
  case EFI_ACPI_4_0_ERROR_SEVERITY_CORRECTABLE:
    Print (
      L"      Correctable\n"
      );
    break;
  case EFI_ACPI_4_0_ERROR_SEVERITY_FATAL:
    Print (
      L"      Fatal\n"
      );
    break;
  case EFI_ACPI_4_0_ERROR_SEVERITY_CORRECTED:
    Print (
      L"      Corrected\n"
      );
    break;
  case EFI_ACPI_4_0_ERROR_SEVERITY_NONE:
    Print (
      L"      None\n"
      );
    break;
  default:
    break;
  }
  // TBD --

  Print (
    L"  ***************************************************************************\n\n"
    );
}

VOID
EFIAPI
DumpAcpiBERT (
  VOID  *Table
  )
{
  EFI_ACPI_4_0_BOOT_ERROR_RECORD_TABLE_HEADER                            *Bert;

  Bert = Table;
  if (Bert == NULL) {
    return;
  }
  
  //
  // Dump Bert table
  //
  Print (
    L"*****************************************************************************\n"
    L"*         Boot Error Record Table                                           *\n"
    L"*****************************************************************************\n"
    );
    
  if (GetAcpiDumpPropertyDumpData()) {
    DumpAcpiHex (Bert->Header.Length, Bert);
  }
  
  if (!GetAcpiDumpPropertyDumpVerb()) {
    goto Done;
  }

  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"BERT address ............................................. 0x%016lx\n" :
    L"BERT address ............................................. 0x%08x\n",
    Bert
    );
  
  DumpAcpiTableHeader(&(Bert->Header));
  
  Print (
    L"  Table Contents:\n"
    );
  Print (
    L"    Boot Error Region Length ............................. 0x%08x\n",
    Bert->BootErrorRegionLength
    );
  Print (
    L"    Boot Error Region .................................... 0x%016lx\n",
    Bert->BootErrorRegion
    );
  DumpBootErrorRegion ((EFI_ACPI_4_0_BOOT_ERROR_REGION_STRUCTURE *)(UINTN)Bert->BootErrorRegion, (UINTN)Bert->BootErrorRegionLength);

Done:
  Print (         
    L"*****************************************************************************\n\n"
    );
  
  return;
}

EFI_STATUS
EFIAPI
DumpAcpiBERTLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return RegisterDumpAcpiTable (EFI_ACPI_4_0_BOOT_ERROR_RECORD_TABLE_SIGNATURE, DumpAcpiBERT);
}
