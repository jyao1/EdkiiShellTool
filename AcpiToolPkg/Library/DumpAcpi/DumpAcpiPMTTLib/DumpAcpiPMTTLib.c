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
DumpPmttDevice (
  EFI_ACPI_5_0_PMMT_COMMON_MEMORY_AGGREGATOR_DEVICE_STRUCTURE             *CommonMemoryAggregatorDevice
  )
{
  Print (
    L"    Type ................................................. 0x%02x\n",
    CommonMemoryAggregatorDevice->Type
    );
  Print (
    L"    Length ............................................... 0x%04x\n",
    CommonMemoryAggregatorDevice->Length
    );
  Print (
    L"    Flags ................................................ 0x%04x\n",
    CommonMemoryAggregatorDevice->Flags
    );
  if ((CommonMemoryAggregatorDevice->Flags & 0x1) != 0) {
    Print (
      L"      top level aggregator device\n"
      );
  } else {
    Print (
      L"      not top level aggregator device\n"
      );
  }
  if ((CommonMemoryAggregatorDevice->Flags & 0x2) != 0) {
    Print (
      L"      physical element of the topology\n"
      );
  } else {
    Print (
      L"      logical element of the topology\n"
      );
  }
  switch ((CommonMemoryAggregatorDevice->Flags & 0xC) >> 2) {
  case 0x0:
    Print (
      L"      all components aggregated by this device implements volatile memory\n"
      );
    break;
  case 0x1:
    Print (
      L"      all components aggregated by this device implements both volatile and non-volatile memory\n"
      );
    break;
  case 0x2:
    Print (
      L"      all components aggregated by this device implements non-volatile memory\n"
      );
    break;
  default:
    break;
  }
}

VOID
DumpPmttDimm (
  EFI_ACPI_5_0_PMMT_DIMM_MEMORY_AGGREGATOR_DEVICE_STRUCTURE               *DimmDevice
  )
{
  if (DimmDevice == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       DIMM Device                                                       *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  DIMM Device address .................................... 0x%016lx\n" :
    L"  DIMM Device address .................................... 0x%08x\n",
    DimmDevice
    );

  DumpPmttDevice (&DimmDevice->Header);
  Print (         
    L"    Physical Component Identifier ........................ 0x%04x\n",
    DimmDevice->PhysicalComponentIdentifier
    );
  Print (         
    L"    Size Of Dimm (in MB) ................................. 0x%08x\n",
    DimmDevice->SizeOfDimm
    );
  Print (         
    L"    SMBIOS Handler ....................................... 0x%08x\n",
    DimmDevice->SmbiosHandle
    );

  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
DumpPmttMemoryController (
  EFI_ACPI_5_0_PMMT_MEMORY_CONTROLLER_MEMORY_AGGREGATOR_DEVICE_STRUCTURE  *MemoryControllerDevice
  )
{
  UINTN                                                                   Index;
  UINT32                                                                  *ProximityDomain;
  INTN                                                                    MemoryControllerLen;
  EFI_ACPI_5_0_PMMT_DIMM_MEMORY_AGGREGATOR_DEVICE_STRUCTURE               *DimmDevice;

  if (MemoryControllerDevice == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       Memory Controller Device                                          *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  Memory Controller Device address ....................... 0x%016lx\n" :
    L"  Memory Controller Device address ....................... 0x%08x\n",
    MemoryControllerDevice
    );

  DumpPmttDevice (&MemoryControllerDevice->Header);
  Print (         
    L"    Read Latency ......................................... 0x%08x\n",
    MemoryControllerDevice->ReadLatency
    );
  Print (         
    L"    Write Latency ........................................ 0x%08x\n",
    MemoryControllerDevice->WriteLatency
    );
  Print (         
    L"    Read Bandwidth ....................................... 0x%08x\n",
    MemoryControllerDevice->ReadBandwidth
    );
  Print (         
    L"    Write Bandwidth ...................................... 0x%08x\n",
    MemoryControllerDevice->WriteBandwidth
    );
  Print (         
    L"    Optimal Access Unit .................................. 0x%04x\n",
    MemoryControllerDevice->OptimalAccessUnit
    );
  Print (         
    L"    Optimal Access Alignment ............................. 0x%04x\n",
    MemoryControllerDevice->OptimalAccessAlignment
    );
  Print (         
    L"    Number Of Proximity Domains .......................... 0x%04x\n",
    MemoryControllerDevice->NumberOfProximityDomains
    );

  ProximityDomain = (UINT32 *)(MemoryControllerDevice + 1);
  for (Index = 0; Index < MemoryControllerDevice->NumberOfProximityDomains; Index++) {
    Print (         
      L"    Proximity Domains .................................... 0x%08x\n",
      *ProximityDomain
      );
  }

  MemoryControllerLen  = MemoryControllerDevice->Header.Length - sizeof(EFI_ACPI_5_0_PMMT_MEMORY_CONTROLLER_MEMORY_AGGREGATOR_DEVICE_STRUCTURE) - sizeof(UINT32) * MemoryControllerDevice->NumberOfProximityDomains;
  DimmDevice = (EFI_ACPI_5_0_PMMT_DIMM_MEMORY_AGGREGATOR_DEVICE_STRUCTURE *)((UINTN)MemoryControllerDevice + sizeof(EFI_ACPI_5_0_PMMT_MEMORY_CONTROLLER_MEMORY_AGGREGATOR_DEVICE_STRUCTURE) + sizeof(UINT32) * MemoryControllerDevice->NumberOfProximityDomains);
  while (MemoryControllerLen > 0) {
    DumpPmttDimm (DimmDevice);
    MemoryControllerLen -= DimmDevice->Header.Length;
    DimmDevice = (EFI_ACPI_5_0_PMMT_DIMM_MEMORY_AGGREGATOR_DEVICE_STRUCTURE *)((UINTN)DimmDevice + DimmDevice->Header.Length);
  }

  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
DumpPmttSocket (
  EFI_ACPI_5_0_PMMT_SOCKET_MEMORY_AGGREGATOR_DEVICE_STRUCTURE             *SocketDevice
  )
{
  INTN                                                                    SocketLen;
  EFI_ACPI_5_0_PMMT_MEMORY_CONTROLLER_MEMORY_AGGREGATOR_DEVICE_STRUCTURE  *MemoryControllerDevice;

  if (SocketDevice == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       Socket Device                                                     *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  Socket Device address .................................. 0x%016lx\n" :
    L"  Socket Device address .................................. 0x%08x\n",
    SocketDevice
    );

  DumpPmttDevice (&SocketDevice->Header);
  Print (         
    L"    Socket Identifier .................................... 0x%02x\n",
    SocketDevice->SocketIdentifier
    );

  SocketLen  = SocketDevice->Header.Length - sizeof(EFI_ACPI_5_0_PMMT_SOCKET_MEMORY_AGGREGATOR_DEVICE_STRUCTURE);
  MemoryControllerDevice = (EFI_ACPI_5_0_PMMT_MEMORY_CONTROLLER_MEMORY_AGGREGATOR_DEVICE_STRUCTURE *)(SocketDevice + 1);
  while (SocketLen > 0) {
    DumpPmttMemoryController (MemoryControllerDevice);
    SocketLen -= MemoryControllerDevice->Header.Length;
    MemoryControllerDevice = (EFI_ACPI_5_0_PMMT_MEMORY_CONTROLLER_MEMORY_AGGREGATOR_DEVICE_STRUCTURE *)((UINTN)MemoryControllerDevice + MemoryControllerDevice->Header.Length);
  }

  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
EFIAPI
DumpAcpiPMTT (
  VOID  *Table
  )
{
  EFI_ACPI_5_0_MEMORY_TOPOLOGY_TABLE                                      *Pmtt;
  EFI_ACPI_5_0_PMMT_COMMON_MEMORY_AGGREGATOR_DEVICE_STRUCTURE             *CommonMemoryAggregatorDevice;
  INTN                                                                    PmttLen;

  Pmtt = Table;
  if (Pmtt == NULL) {
    return;
  }
  
  //
  // Dump Pmtt table
  //
  Print (
    L"*****************************************************************************\n"
    L"*         Memory Topology Table                                             *\n"
    L"*****************************************************************************\n"
    );
    
  if (GetAcpiDumpPropertyDumpData()) {
    DumpAcpiHex (Pmtt->Header.Length, Pmtt);
  }
  
  if (!GetAcpiDumpPropertyDumpVerb()) {
    goto Done;
  }

  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"PMTT address ............................................. 0x%016lx\n" :
    L"PMTT address ............................................. 0x%08x\n",
    Pmtt
    );
  
  DumpAcpiTableHeader(&(Pmtt->Header));
  
  Print (
    L"  Table Contents:\n"
    );
  PmttLen  = Pmtt->Header.Length - sizeof(EFI_ACPI_5_0_MEMORY_TOPOLOGY_TABLE);
  CommonMemoryAggregatorDevice = (EFI_ACPI_5_0_PMMT_COMMON_MEMORY_AGGREGATOR_DEVICE_STRUCTURE *)(Pmtt + 1);
  while (PmttLen > 0) {
    switch (CommonMemoryAggregatorDevice->Type) {
    case EFI_ACPI_5_0_PMMT_MEMORY_AGGREGATOR_DEVICE_TYPE_SOCKET:
      DumpPmttSocket ((EFI_ACPI_5_0_PMMT_SOCKET_MEMORY_AGGREGATOR_DEVICE_STRUCTURE *)CommonMemoryAggregatorDevice);
      break;
    case EFI_ACPI_5_0_PMMT_MEMORY_AGGREGATOR_DEVICE_TYPE_MEMORY_CONTROLLER:
      DumpPmttMemoryController ((EFI_ACPI_5_0_PMMT_MEMORY_CONTROLLER_MEMORY_AGGREGATOR_DEVICE_STRUCTURE *)CommonMemoryAggregatorDevice);
      break;
    case EFI_ACPI_5_0_PMMT_MEMORY_AGGREGATOR_DEVICE_TYPE_DIMM:
      DumpPmttDimm ((EFI_ACPI_5_0_PMMT_DIMM_MEMORY_AGGREGATOR_DEVICE_STRUCTURE *)CommonMemoryAggregatorDevice);
      break;
    default:
      break;
    }
    CommonMemoryAggregatorDevice = (EFI_ACPI_5_0_PMMT_COMMON_MEMORY_AGGREGATOR_DEVICE_STRUCTURE *)((UINT8 *)CommonMemoryAggregatorDevice + CommonMemoryAggregatorDevice->Length);
    PmttLen -= CommonMemoryAggregatorDevice->Length;
  }

Done:
  Print (         
    L"*****************************************************************************\n\n"
    );
  
  return;
}

EFI_STATUS
EFIAPI
DumpAcpiPMTTLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return RegisterDumpAcpiTable (EFI_ACPI_5_0_PLATFORM_MEMORY_TOPOLOGY_TABLE_SIGNATURE, DumpAcpiPMTT);
}

