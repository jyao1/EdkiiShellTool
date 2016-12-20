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
DumpAcpiMaximumProximityDomainInformation (
  EFI_ACPI_4_0_MAXIMUM_PROXIMITY_DOMAIN_INFORMATION_STRUCTURE *MaximumProximityDomainInformation
  )
{
  if (MaximumProximityDomainInformation == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       Maximum Proximity Domain Information Structure                    *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  MaximumProximityDomainInformation address .............. 0x%016lx\n" :
    L"  MaximumProximityDomainInformation address .............. 0x%08x\n",
    MaximumProximityDomainInformation
    );
  Print (
    L"    Revision ............................................. 0x%02x\n",
    MaximumProximityDomainInformation->Revision
    );
  Print (
    L"    Length ............................................... 0x%02x\n",
    MaximumProximityDomainInformation->Length
    );
  Print (
    L"    Proximity Domain Range Low ........................... 0x%04x\n",
    MaximumProximityDomainInformation->ProximityDomainRangeLow
    );
  Print (
    L"    Proximity Domain Range High .......................... 0x%04x\n",
    MaximumProximityDomainInformation->ProximityDomainRangeHigh
    );
  Print (
    L"    Maximum Processor Capacity ........................... 0x%04x\n",
    MaximumProximityDomainInformation->MaximumProcessorCapacity
    );
  Print (
    L"    Maximum Memory Capacity .............................. 0x%08x\n",
    MaximumProximityDomainInformation->MaximumMemoryCapacity
    );
  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
EFIAPI
DumpAcpiMSCT (
  VOID  *Table
  )
{
  EFI_ACPI_4_0_MAXIMUM_SYSTEM_CHARACTERISTICS_TABLE_HEADER    *Msct;
  EFI_ACPI_4_0_MAXIMUM_PROXIMITY_DOMAIN_INFORMATION_STRUCTURE *MaximumProximityDomainInformation;
  UINTN                                                       Index;

  Msct = Table;
  if (Msct == NULL) {
    return;
  }
  
  //
  // Dump Msct table
  //
  Print (
    L"*****************************************************************************\n"
    L"*         Maximum System Characteristics Table                              *\n"
    L"*****************************************************************************\n"
    );
    
  if (GetAcpiDumpPropertyDumpData()) {
    DumpAcpiHex (Msct->Header.Length, Msct);
  }
  
  if (!GetAcpiDumpPropertyDumpVerb()) {
    goto Done;
  }

  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"MSCT address ............................................. 0x%016lx\n" :
    L"MSCT address ............................................. 0x%08x\n",
    Msct
    );
  
  DumpAcpiTableHeader(&(Msct->Header));
  
  Print (
    L"  Table Contents:\n"
    );

  Print (
    L"    Offset to Proximity Domain Information Structure ..... 0x%08x\n",
    Msct->OffsetProxDomInfo
    );
  Print (
    L"    Maximum Number of Proximity Domains .................. 0x%08x\n",
    Msct->MaximumNumberOfProximityDomains
    );
  Print (
    L"    Maximum Number of Clock Domains ...................... 0x%08x\n",
    Msct->MaximumNumberOfClockDomains
    );
  Print (
    L"    Maximum Physical Address ............................. 0x%016lx\n",
    Msct->MaximumPhysicalAddress
    );

  MaximumProximityDomainInformation = (EFI_ACPI_4_0_MAXIMUM_PROXIMITY_DOMAIN_INFORMATION_STRUCTURE *)((UINTN)Msct + Msct->OffsetProxDomInfo);
  for (Index = 0; Index <= Msct->MaximumNumberOfProximityDomains; Index++) {
    DumpAcpiMaximumProximityDomainInformation (MaximumProximityDomainInformation);
    MaximumProximityDomainInformation = (EFI_ACPI_4_0_MAXIMUM_PROXIMITY_DOMAIN_INFORMATION_STRUCTURE *)((UINTN)MaximumProximityDomainInformation + MaximumProximityDomainInformation->Length);
  }

Done:
  Print (         
    L"*****************************************************************************\n\n"
    );
  
  return;
}

EFI_STATUS
EFIAPI
DumpAcpiMSCTLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return RegisterDumpAcpiTable (EFI_ACPI_4_0_MAXIMUM_SYSTEM_CHARACTERISTICS_TABLE_SIGNATURE, DumpAcpiMSCT);
}

