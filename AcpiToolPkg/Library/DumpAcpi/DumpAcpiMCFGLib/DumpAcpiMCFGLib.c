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
#include <IndustryStandard/MemoryMappedConfigurationSpaceAccessTable.h>

VOID
DumpAcpiMemoryMappedConfigurationBaseAddressAllocation (
  EFI_ACPI_MEMORY_MAPPED_ENHANCED_CONFIGURATION_SPACE_BASE_ADDRESS_ALLOCATION_STRUCTURE *MemoryMappedConfigurationBaseAddressAllocation
  )
{
  if (MemoryMappedConfigurationBaseAddressAllocation == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       Memory Mapped Configuration Base Address Allocation Structure     *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  MemoryMappedConfigurationBaseAddressAllocation address . 0x%016lx\n" :
    L"  MemoryMappedConfigurationBaseAddressAllocation address . 0x%08x\n",
    MemoryMappedConfigurationBaseAddressAllocation
    );
  Print (
    L"    Base Address ......................................... 0x%016lx\n",
    MemoryMappedConfigurationBaseAddressAllocation->BaseAddress
    );
  Print (
    L"    PCI Segment Group Number ............................. 0x%04x\n",
    MemoryMappedConfigurationBaseAddressAllocation->PciSegmentGroupNumber
    );
  Print (
    L"    Start Bus Number ..................................... 0x%02x\n",
    MemoryMappedConfigurationBaseAddressAllocation->StartBusNumber
    );
  Print (
    L"    End Bus Number ....................................... 0x%02x\n",
    MemoryMappedConfigurationBaseAddressAllocation->EndBusNumber
    );
  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
EFIAPI
DumpAcpiMCFG (
  VOID  *Table
  )
{
  EFI_ACPI_MEMORY_MAPPED_CONFIGURATION_BASE_ADDRESS_TABLE_HEADER                            *Mcfg;
  EFI_ACPI_MEMORY_MAPPED_ENHANCED_CONFIGURATION_SPACE_BASE_ADDRESS_ALLOCATION_STRUCTURE *MemoryMappedConfigurationBaseAddressAllocation;
  INTN                      McfgLen;
  INTN                      TableLen;

  Mcfg = Table;
  if (Mcfg == NULL) {
    return;
  }
  
  //
  // Dump Mcfg table
  //
  Print (
    L"*****************************************************************************\n"
    L"*         PCI Express Memory Mapped Configuration Space Base Address        *\n"
    L"*                             Description Table                             *\n"
    L"*****************************************************************************\n"
    );
    
  if (GetAcpiDumpPropertyDumpData()) {
    DumpAcpiHex (Mcfg->Header.Length, Mcfg);
  }
  
  if (!GetAcpiDumpPropertyDumpVerb()) {
    goto Done;
  }

  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"MCFG address ............................................. 0x%016lx\n" :
    L"MCFG address ............................................. 0x%08x\n",
    Mcfg
    );
  
  DumpAcpiTableHeader(&(Mcfg->Header));
  
  Print (
    L"  Table Contents:\n"
    );
  
  Print (
    L"\n"
    );
  
  McfgLen  = Mcfg->Header.Length - sizeof(EFI_ACPI_MEMORY_MAPPED_CONFIGURATION_BASE_ADDRESS_TABLE_HEADER);
  TableLen = 0;
  MemoryMappedConfigurationBaseAddressAllocation = (EFI_ACPI_MEMORY_MAPPED_ENHANCED_CONFIGURATION_SPACE_BASE_ADDRESS_ALLOCATION_STRUCTURE *)(Mcfg + 1);
  while (McfgLen > 0) {
    DumpAcpiMemoryMappedConfigurationBaseAddressAllocation (MemoryMappedConfigurationBaseAddressAllocation);
    TableLen = sizeof(EFI_ACPI_MEMORY_MAPPED_ENHANCED_CONFIGURATION_SPACE_BASE_ADDRESS_ALLOCATION_STRUCTURE);
    MemoryMappedConfigurationBaseAddressAllocation++;
    McfgLen -= TableLen;
  }

Done:
  Print (         
    L"*****************************************************************************\n\n"
    );
  
  return;
}

EFI_STATUS
EFIAPI
DumpAcpiMCFGLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return RegisterDumpAcpiTable (EFI_ACPI_2_0_MEMORY_MAPPED_CONFIGURATION_BASE_ADDRESS_TABLE_SIGNATURE, DumpAcpiMCFG);
}

