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
DumpAcpiCpepStruct (
  EFI_ACPI_4_0_CPEP_PROCESSOR_APIC_SAPIC_STRUCTURE                           *CpepStruct
  )
{
  if (CpepStruct == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       CPEP Structure                                                    *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  CPEP Structure address ................................. 0x%016lx\n" :
    L"  CPEP Structure address ................................. 0x%08x\n",
    CpepStruct
    );
  Print (
    L"    Type ................................................. 0x%02x\n",
    CpepStruct->Type
    );
  Print (
    L"    Length ............................................... 0x%02x\n",
    CpepStruct->Length
    );
  Print (
    L"    Processor ID ......................................... 0x%02x\n",
    CpepStruct->ProcessorId
    );
  Print (
    L"    Processor EID ........................................ 0x%02x\n",
    CpepStruct->ProcessorEid
    );
  Print (
    L"    Polling Interval ..................................... 0x%08x\n",
    CpepStruct->PollingInterval
    );
  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}


VOID
EFIAPI
DumpAcpiCPEP (
  VOID  *Table
  )
{
  EFI_ACPI_4_0_CORRECTED_PLATFORM_ERROR_POLLING_TABLE_HEADER                *Cpep;
  EFI_ACPI_4_0_CPEP_PROCESSOR_APIC_SAPIC_STRUCTURE               *CpepStruct;
  INTN                      CpepLen;
  INTN                      TableLen;

  Cpep = Table;
  if (Cpep == NULL) {
    return;
  }
  
  //
  // Dump Cpep table
  //
  Print (
    L"*****************************************************************************\n"
    L"*         Corrected Platform Error Polling Table                            *\n"
    L"*****************************************************************************\n"
    );
    
  if (GetAcpiDumpPropertyDumpData()) {
    DumpAcpiHex (Cpep->Header.Length, Cpep);
  }
  
  if (!GetAcpiDumpPropertyDumpVerb()) {
    goto Done;
  }

  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"CPEP address ............................................. 0x%016lx\n" :
    L"CPEP address ............................................. 0x%08x\n",
    Cpep
    );
  
  DumpAcpiTableHeader(&(Cpep->Header));
  
  Print (
    L"  Table Contents:\n"
    );
  
  Print (
    L"\n"
    );
  
  CpepLen  = Cpep->Header.Length - sizeof(EFI_ACPI_4_0_CORRECTED_PLATFORM_ERROR_POLLING_TABLE_HEADER);
  TableLen = 0;
  CpepStruct = (EFI_ACPI_4_0_CPEP_PROCESSOR_APIC_SAPIC_STRUCTURE *)(Cpep + 1);
  while (CpepLen > 0) {
    DumpAcpiCpepStruct (CpepStruct);
    TableLen = sizeof(EFI_ACPI_4_0_CPEP_PROCESSOR_APIC_SAPIC_STRUCTURE);
    CpepStruct++;
    CpepLen -= TableLen;
  }

Done:
  Print (         
    L"*****************************************************************************\n\n"
    );
  
  return;
}

EFI_STATUS
EFIAPI
DumpAcpiCPEPLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return RegisterDumpAcpiTable (EFI_ACPI_3_0_CORRECTED_PLATFORM_ERROR_POLLING_TABLE_SIGNATURE, DumpAcpiCPEP);
}
