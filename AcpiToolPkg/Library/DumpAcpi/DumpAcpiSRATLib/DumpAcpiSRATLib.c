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
  UINT8                 Type;
  UINT8                 Length;
} AFFINITY_STRUCT_HEADER;

#pragma pack()

VOID
DumpAcpiProcessorLocalApicSapicAffinity (
  EFI_ACPI_3_0_PROCESSOR_LOCAL_APIC_SAPIC_AFFINITY_STRUCTURE  *ProcessorLocalApicSapicAffinity
  )
{
  if (ProcessorLocalApicSapicAffinity == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       Processor Local APIC/SAPIC Affinity Structure                     *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  Processor Local APIC/SAPIC Affinity address ............ 0x%016lx\n" :
    L"  Processor Local APIC/SAPIC Affinity address ............ 0x%08x\n",
    ProcessorLocalApicSapicAffinity
    );
  Print (
    L"    Type ................................................. 0x%02x\n",
    ProcessorLocalApicSapicAffinity->Type
    );
  Print (
    L"    Length ............................................... 0x%02x\n",
    ProcessorLocalApicSapicAffinity->Length
    );
  Print (
    L"    Proximity Domain [7:0] ............................... 0x%02x\n",
    ProcessorLocalApicSapicAffinity->ProximityDomain7To0
    );
  Print (
    L"    APIC ID .............................................. 0x%02x\n",
    ProcessorLocalApicSapicAffinity->ApicId
    );
  Print (
    L"    Flags ................................................ 0x%08x\n",
    ProcessorLocalApicSapicAffinity->Flags
    );
  Print (
    L"      Enabled ............................................ 0x%08x\n",
    ProcessorLocalApicSapicAffinity->Flags & EFI_ACPI_3_0_PROCESSOR_LOCAL_APIC_SAPIC_ENABLED
    );
  Print (
    L"    Local SAPIC EID ...................................... 0x%02x\n",
    ProcessorLocalApicSapicAffinity->LocalSapicEid
    );
  Print (
    L"    Proximity Domain [15:8] .............................. 0x%02x\n",
    ProcessorLocalApicSapicAffinity->ProximityDomain31To8[0]
    );
  Print (
    L"    Proximity Domain [23:16] ............................. 0x%02x\n",
    ProcessorLocalApicSapicAffinity->ProximityDomain31To8[1]
    );
  Print (
    L"    Proximity Domain [31:24] ............................. 0x%02x\n",
    ProcessorLocalApicSapicAffinity->ProximityDomain31To8[2]
    );
  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
DumpAcpiMemoryAffinity (
  EFI_ACPI_3_0_MEMORY_AFFINITY_STRUCTURE                      *MemoryAffinity
  )
{
  if (MemoryAffinity == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       Memory Affinity Structure                                         *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  Memory Affinity address ................................ 0x%016lx\n" :
    L"  Memory Affinity address ................................ 0x%08x\n",
    MemoryAffinity
    );
  Print (
    L"    Type ................................................. 0x%02x\n",
    MemoryAffinity->Type
    );
  Print (
    L"    Length ............................................... 0x%02x\n",
    MemoryAffinity->Length
    );
  Print (
    L"    Proximity Domain ..................................... 0x%08x\n",
    MemoryAffinity->ProximityDomain
    );
  Print (
    L"    Base Address Low ..................................... 0x%08x\n",
    MemoryAffinity->AddressBaseLow
    );
  Print (
    L"    Base Address High .................................... 0x%08x\n",
    MemoryAffinity->AddressBaseHigh
    );
  Print (
    L"    Length Low ........................................... 0x%08x\n",
    MemoryAffinity->LengthLow
    );
  Print (
    L"    Length High .......................................... 0x%08x\n",
    MemoryAffinity->LengthHigh
    );
  Print (
    L"    Flags ................................................ 0x%08x\n",
    MemoryAffinity->Flags
    );
  Print (
    L"      Enabled ............................................ 0x%08x\n",
    MemoryAffinity->Flags & EFI_ACPI_3_0_MEMORY_ENABLED
    );
  Print (
    L"      Hot Pluggable ...................................... 0x%08x\n",
    MemoryAffinity->Flags & EFI_ACPI_3_0_MEMORY_HOT_PLUGGABLE
    );
  Print (
    L"      NonVolatile ........................................ 0x%08x\n",
    MemoryAffinity->Flags & EFI_ACPI_3_0_MEMORY_NONVOLATILE
    );
  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
EFIAPI
DumpAcpiSRAT (
  VOID  *Table
  )
{
  EFI_ACPI_3_0_SYSTEM_RESOURCE_AFFINITY_TABLE_HEADER                            *Srat;
  AFFINITY_STRUCT_HEADER    *AffinityStructHeader;
  INTN                      SratLen;
  INTN                      TableLen;

  Srat = Table;
  if (Srat == NULL) {
    return;
  }
  
  //
  // Dump Srat table
  //
  Print (
    L"*****************************************************************************\n"
    L"*         System Resource Affinity Table                                    *\n"
    L"*****************************************************************************\n"
    );
    
  if (GetAcpiDumpPropertyDumpData()) {
    DumpAcpiHex (Srat->Header.Length, Srat);
  }
  
  if (!GetAcpiDumpPropertyDumpVerb()) {
    goto Done;
  }

  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"SRAT address ............................................. 0x%016lx\n" :
    L"SRAT address ............................................. 0x%08x\n",
    Srat
    );
  
  DumpAcpiTableHeader(&(Srat->Header));
  
  Print (
    L"  Table Contents:\n"
    );
  if (Srat->Header.Revision < EFI_ACPI_3_0_SYSTEM_RESOURCE_AFFINITY_TABLE_REVISION) {
    Print (
      L"    TableRevision ........................................ 0x%08x\n",
      Srat->Reserved1
      );
  }

  Print (
    L"\n"
    );
  
  SratLen  = Srat->Header.Length - sizeof(EFI_ACPI_3_0_SYSTEM_RESOURCE_AFFINITY_TABLE_HEADER);
  TableLen = 0;
  AffinityStructHeader = (AFFINITY_STRUCT_HEADER *)(Srat + 1);
  while (SratLen > 0) {
    switch (AffinityStructHeader->Type) {
    case EFI_ACPI_3_0_PROCESSOR_LOCAL_APIC_SAPIC_AFFINITY:
      DumpAcpiProcessorLocalApicSapicAffinity ((EFI_ACPI_3_0_PROCESSOR_LOCAL_APIC_SAPIC_AFFINITY_STRUCTURE *)AffinityStructHeader);
      TableLen = sizeof(EFI_ACPI_3_0_PROCESSOR_LOCAL_APIC_SAPIC_AFFINITY_STRUCTURE);
      break;
    case EFI_ACPI_3_0_MEMORY_AFFINITY:
      DumpAcpiMemoryAffinity ((EFI_ACPI_3_0_MEMORY_AFFINITY_STRUCTURE *)AffinityStructHeader);
      TableLen = sizeof(EFI_ACPI_3_0_MEMORY_AFFINITY_STRUCTURE);
      break;
    default:
      break;
    }
    AffinityStructHeader = (AFFINITY_STRUCT_HEADER *)((UINT8 *)AffinityStructHeader + AffinityStructHeader->Length);
    SratLen         -= TableLen;
  }

Done:
  Print (         
    L"*****************************************************************************\n\n"
    );
  
  return;
}

EFI_STATUS
EFIAPI
DumpAcpiSRATLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return RegisterDumpAcpiTable (EFI_ACPI_2_0_STATIC_RESOURCE_AFFINITY_TABLE_SIGNATURE, DumpAcpiSRAT);
}
