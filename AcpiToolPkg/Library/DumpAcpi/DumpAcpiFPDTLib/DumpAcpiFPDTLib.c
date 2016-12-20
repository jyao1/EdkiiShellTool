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
DumpFpdtRecordHeader (
  EFI_ACPI_5_0_FPDT_PERFORMANCE_RECORD_HEADER            *FpdtRecord
  )
{
  if (FpdtRecord == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       Performance Record                                                *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  Fpdt Record address .................................... 0x%016lx\n" :
    L"  Fpdt Record address .................................... 0x%08x\n",
    FpdtRecord
    );
  Print (
    L"    Type ................................................. 0x%04x\n",
    FpdtRecord->Type
    );
  Print (
    L"    Length ............................................... 0x%02x\n",
    FpdtRecord->Length
    );
  Print (
    L"    Revision ............................................. 0x%02x\n",
    FpdtRecord->Revision
    );
  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
DumpFpdtBasicBootData (
  EFI_ACPI_5_0_FPDT_FIRMWARE_BASIC_BOOT_RECORD            *FpdtRecord
  )
{
  if (FpdtRecord == NULL) {
    return;
  }

  Print (
    L"  ***************************************************************************\n"
    L"  *       Basic Boot Performance Record                                     *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  Fpdt Record address .................................... 0x%016lx\n" :
    L"  Fpdt Record address .................................... 0x%08x\n",
    FpdtRecord
    );
  Print (
    L"    Type ................................................. 0x%04x\n",
    FpdtRecord->Header.Type
    );
  Print (
    L"    Length ............................................... 0x%02x\n",
    FpdtRecord->Header.Length
    );
  Print (
    L"    Revision ............................................. 0x%02x\n",
    FpdtRecord->Header.Revision
    );
  Print (
    L"    Reset End ............................................ 0x%016lx\n",
    FpdtRecord->ResetEnd
    );
  Print (
    L"    OS Loader LoadImage Start ............................ 0x%016lx\n",
    FpdtRecord->OsLoaderLoadImageStart
    );
  Print (
    L"    OS Loader StartImage Start ........................... 0x%016lx\n",
    FpdtRecord->OsLoaderStartImageStart
    );
  Print (
    L"    ExitBootServices Entry ............................... 0x%016lx\n",
    FpdtRecord->ExitBootServicesEntry
    );
  Print (
    L"    ExitBootServices Exit ................................ 0x%016lx\n",
    FpdtRecord->ExitBootServicesExit
    );
  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
DumpFpdtBasicBootTable (
  EFI_ACPI_5_0_FPDT_FIRMWARE_BASIC_BOOT_TABLE            *FpdtRecordTable
  )
{
  EFI_ACPI_5_0_FPDT_PERFORMANCE_RECORD_HEADER  *FpdtRecord;
  INTN                                         FpdtRecordLen;

  if (FpdtRecordTable == NULL) {
    return;
  }
  Print (
    L"  ***************************************************************************\n"
    L"  *       Basic Boot Performance Table                                      *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  Fpdt Record address .................................... 0x%016lx\n" :
    L"  Fpdt Record address .................................... 0x%08x\n",
    FpdtRecordTable
    );
  Print (
    L"    Signature ............................................ 0x%08x\n",
    FpdtRecordTable->Header.Signature
    );
  Print (
    L"    Length ............................................... 0x%08x\n",
    FpdtRecordTable->Header.Length
    );
  Print (       
    L"  ***************************************************************************\n\n"
    );

  FpdtRecord = (EFI_ACPI_5_0_FPDT_PERFORMANCE_RECORD_HEADER *)(FpdtRecordTable + 1);
  FpdtRecordLen = FpdtRecordTable->Header.Length - sizeof(FpdtRecordTable->Header);
  while (FpdtRecordLen > 0) {
    switch (FpdtRecord->Type) {
    case EFI_ACPI_5_0_FPDT_RUNTIME_RECORD_TYPE_FIRMWARE_BASIC_BOOT:
      DumpFpdtBasicBootData ((EFI_ACPI_5_0_FPDT_FIRMWARE_BASIC_BOOT_RECORD *)FpdtRecord);
      break;
    default:
      DumpFpdtRecordHeader (FpdtRecord);
      break;
    }
    FpdtRecordLen -= FpdtRecord->Length;
    FpdtRecord = (EFI_ACPI_5_0_FPDT_PERFORMANCE_RECORD_HEADER *)((UINTN)FpdtRecord + FpdtRecord->Length);
  }

  return ;
}

VOID
DumpFpdtBootPerformanceTablePointer (
  EFI_ACPI_5_0_FPDT_BOOT_PERFORMANCE_TABLE_POINTER_RECORD            *FpdtRecord
  )
{
  if (FpdtRecord == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       Boot Performance Table Pointer Record                             *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  Fpdt Record address .................................... 0x%016lx\n" :
    L"  Fpdt Record address .................................... 0x%08x\n",
    FpdtRecord
    );
  Print (
    L"    Type ................................................. 0x%04x\n",
    FpdtRecord->Header.Type
    );
  Print (
    L"    Length ............................................... 0x%02x\n",
    FpdtRecord->Header.Length
    );
  Print (
    L"    Revision ............................................. 0x%02x\n",
    FpdtRecord->Header.Revision
    );
  Print (
    L"    Boot Performance Table Pointer ....................... 0x%016lx\n",
    FpdtRecord->BootPerformanceTablePointer
    );
  Print (       
    L"  ***************************************************************************\n\n"
    );

  DumpFpdtBasicBootTable ((EFI_ACPI_5_0_FPDT_FIRMWARE_BASIC_BOOT_TABLE *)(UINTN)FpdtRecord->BootPerformanceTablePointer);

  return;
}

VOID
DumpFpdtS3PerformanceTableS3Resume (
  EFI_ACPI_5_0_FPDT_S3_RESUME_RECORD            *FpdtRecord
  )
{
  if (FpdtRecord == NULL) {
    return;
  }

  Print (
    L"  ***************************************************************************\n"
    L"  *       S3 Resume Performance Record                                      *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  Fpdt Record address .................................... 0x%016lx\n" :
    L"  Fpdt Record address .................................... 0x%08x\n",
    FpdtRecord
    );
  Print (
    L"    Type ................................................. 0x%04x\n",
    FpdtRecord->Header.Type
    );
  Print (
    L"    Length ............................................... 0x%02x\n",
    FpdtRecord->Header.Length
    );
  Print (
    L"    Revision ............................................. 0x%02x\n",
    FpdtRecord->Header.Revision
    );
  Print (
    L"    Resume Count ......................................... 0x%08x\n",
    FpdtRecord->ResumeCount
    );
  Print (
    L"    Full Resume .......................................... 0x%016lx\n",
    FpdtRecord->FullResume
    );
  Print (
    L"    Average Resume ....................................... 0x%016lx\n",
    FpdtRecord->AverageResume
    );
  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
DumpFpdtS3PerformanceTableS3Suspend (
  EFI_ACPI_5_0_FPDT_S3_SUSPEND_RECORD            *FpdtRecord
  )
{
  if (FpdtRecord == NULL) {
    return;
  }

  Print (
    L"  ***************************************************************************\n"
    L"  *       S3 Suspend Performance Record                                     *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  Fpdt Record address .................................... 0x%016lx\n" :
    L"  Fpdt Record address .................................... 0x%08x\n",
    FpdtRecord
    );
  Print (
    L"    Type ................................................. 0x%04x\n",
    FpdtRecord->Header.Type
    );
  Print (
    L"    Length ............................................... 0x%02x\n",
    FpdtRecord->Header.Length
    );
  Print (
    L"    Revision ............................................. 0x%02x\n",
    FpdtRecord->Header.Revision
    );
  Print (
    L"    Suspend Start ........................................ 0x%016lx\n",
    FpdtRecord->SuspendStart
    );
  Print (
    L"    Suspend End .......................................... 0x%016lx\n",
    FpdtRecord->SuspendEnd
    );
  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
DumpFpdtS3BootTable (
  EFI_ACPI_5_0_FPDT_FIRMWARE_S3_BOOT_TABLE            *FpdtRecordTable
  )
{
  EFI_ACPI_5_0_FPDT_PERFORMANCE_RECORD_HEADER  *FpdtRecord;
  INTN                                         FpdtRecordLen;

  if (FpdtRecordTable == NULL) {
    return;
  }
  Print (
    L"  ***************************************************************************\n"
    L"  *       S3 Boot Performance Table                                         *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  Fpdt Record address .................................... 0x%016lx\n" :
    L"  Fpdt Record address .................................... 0x%08x\n",
    FpdtRecordTable
    );
  Print (
    L"    Signature ............................................ 0x%08x\n",
    FpdtRecordTable->Header.Signature
    );
  Print (
    L"    Length ............................................... 0x%08x\n",
    FpdtRecordTable->Header.Length
    );
  Print (       
    L"  ***************************************************************************\n\n"
    );

  FpdtRecord = (EFI_ACPI_5_0_FPDT_PERFORMANCE_RECORD_HEADER *)(FpdtRecordTable + 1);
  FpdtRecordLen = FpdtRecordTable->Header.Length - sizeof(FpdtRecordTable->Header);
  while (FpdtRecordLen > 0) {
    switch (FpdtRecord->Type) {
    case EFI_ACPI_5_0_FPDT_RUNTIME_RECORD_TYPE_S3_RESUME:
      DumpFpdtS3PerformanceTableS3Resume ((EFI_ACPI_5_0_FPDT_S3_RESUME_RECORD *)FpdtRecord);
      break;
    case EFI_ACPI_5_0_FPDT_RUNTIME_RECORD_TYPE_S3_SUSPEND:
      DumpFpdtS3PerformanceTableS3Suspend ((EFI_ACPI_5_0_FPDT_S3_SUSPEND_RECORD *)FpdtRecord);
      break;
    default:
      DumpFpdtRecordHeader (FpdtRecord);
      break;
    }
    FpdtRecordLen -= FpdtRecord->Length;
    FpdtRecord = (EFI_ACPI_5_0_FPDT_PERFORMANCE_RECORD_HEADER *)((UINTN)FpdtRecord + FpdtRecord->Length);
  }

  return ;
}

VOID
DumpFpdtS3PerformanceTablePointer (
  EFI_ACPI_5_0_FPDT_S3_PERFORMANCE_TABLE_POINTER_RECORD            *FpdtRecord
  )
{
  if (FpdtRecord == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       S3 Performance Table Pointer Record                               *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  Fpdt Record address .................................... 0x%016lx\n" :
    L"  Fpdt Record address .................................... 0x%08x\n",
    FpdtRecord
    );
  Print (
    L"    Type ................................................. 0x%04x\n",
    FpdtRecord->Header.Type
    );
  Print (
    L"    Length ............................................... 0x%02x\n",
    FpdtRecord->Header.Length
    );
  Print (
    L"    Revision ............................................. 0x%02x\n",
    FpdtRecord->Header.Revision
    );
  Print (
    L"    S3 Performance Table Pointer ......................... 0x%016lx\n",
    FpdtRecord->S3PerformanceTablePointer
    );
  Print (       
    L"  ***************************************************************************\n\n"
    );

  DumpFpdtS3BootTable ((EFI_ACPI_5_0_FPDT_FIRMWARE_S3_BOOT_TABLE *)(UINTN)FpdtRecord->S3PerformanceTablePointer);

  return;
}

VOID
EFIAPI
DumpAcpiFPDT (
  VOID  *Table
  )
{
  EFI_ACPI_5_0_FIRMWARE_PERFORMANCE_RECORD_TABLE         *Fpdt;
  EFI_ACPI_5_0_FPDT_PERFORMANCE_RECORD_HEADER            *FpdtRecord;
  INTN                                                   FpdtLen;

  Fpdt = Table;
  if (Fpdt == NULL) {
    return;
  }
  
  //
  // Dump Fpdt table
  //
  Print (
    L"*****************************************************************************\n"
    L"*         Firmware Performance Data Table                                   *\n"
    L"*****************************************************************************\n"
    );
    
  if (GetAcpiDumpPropertyDumpData()) {
    DumpAcpiHex (Fpdt->Header.Length, Fpdt);
  }
  
  if (!GetAcpiDumpPropertyDumpVerb()) {
    goto Done;
  }

  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"FPDT address ............................................. 0x%016lx\n" :
    L"FPDT address ............................................. 0x%08x\n",
    Fpdt
    );
  
  DumpAcpiTableHeader(&(Fpdt->Header));
  
  Print (
    L"  Table Contents:\n"
    );

  FpdtLen  = Fpdt->Header.Length - sizeof(EFI_ACPI_5_0_FIRMWARE_PERFORMANCE_RECORD_TABLE);
  FpdtRecord = (EFI_ACPI_5_0_FPDT_PERFORMANCE_RECORD_HEADER *)(Fpdt + 1);

  while (FpdtLen > 0) {
    switch (FpdtRecord->Type) {
    case EFI_ACPI_5_0_FPDT_RECORD_TYPE_FIRMWARE_BASIC_BOOT_POINTER:
      DumpFpdtBootPerformanceTablePointer ((EFI_ACPI_5_0_FPDT_BOOT_PERFORMANCE_TABLE_POINTER_RECORD *)FpdtRecord);
      break;
    case EFI_ACPI_5_0_FPDT_RECORD_TYPE_S3_PERFORMANCE_TABLE_POINTER:
      DumpFpdtS3PerformanceTablePointer ((EFI_ACPI_5_0_FPDT_S3_PERFORMANCE_TABLE_POINTER_RECORD *)FpdtRecord);
      break;
    default:
      DumpFpdtRecordHeader (FpdtRecord);
      break;
    }
    FpdtLen -= FpdtRecord->Length;
    FpdtRecord = (EFI_ACPI_5_0_FPDT_PERFORMANCE_RECORD_HEADER *)((UINTN)FpdtRecord + FpdtRecord->Length);
  }

Done:
  Print (         
    L"*****************************************************************************\n\n"
    );
  
  return;
}

EFI_STATUS
EFIAPI
DumpAcpiFPDTLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return RegisterDumpAcpiTable (EFI_ACPI_5_0_FIRMWARE_PERFORMANCE_DATA_TABLE_SIGNATURE, DumpAcpiFPDT);
}
