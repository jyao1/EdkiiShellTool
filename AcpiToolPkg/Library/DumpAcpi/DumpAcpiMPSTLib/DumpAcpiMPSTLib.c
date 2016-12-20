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
DumpMemoryPowerNode (
  EFI_ACPI_5_0_MPST_MEMORY_POWER_STRUCTURE                        *MemoryPowerNode
  )
{
  EFI_ACPI_5_0_MPST_MEMORY_POWER_STATE              *PowerState;
  UINT16                                            *PhysicalComponentIdentifier;
  UINTN                                             Index;

  if (MemoryPowerNode == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       Memory Power Node                                                 *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  Memory Power Node address .............................. 0x%016lx\n" :
    L"  Memory Power Node address .............................. 0x%08x\n",
    MemoryPowerNode
    );
  Print (
    L"    Flag ................................................. 0x%02x\n",
    MemoryPowerNode->Flag
    );
  Print (
    L"      Enabled ............................................ 0x%02x\n",
    MemoryPowerNode->Flag & EFI_ACPI_5_0_MPST_MEMORY_POWER_STRUCTURE_FLAG_ENABLE
    );
  Print (
    L"      Power Managed ...................................... 0x%02x\n",
    MemoryPowerNode->Flag & EFI_ACPI_5_0_MPST_MEMORY_POWER_STRUCTURE_FLAG_POWER_MANAGED
    );
  Print (
    L"      Hot Pluggable ...................................... 0x%02x\n",
    MemoryPowerNode->Flag & EFI_ACPI_5_0_MPST_MEMORY_POWER_STRUCTURE_FLAG_HOT_PLUGGABLE
    );
  Print (
    L"    Memory Power Node Id ................................. 0x%04x\n",
    MemoryPowerNode->MemoryPowerNodeId
    );
  Print (
    L"    Length ............................................... 0x%08x\n",
    MemoryPowerNode->Length
    );
  Print (
    L"    Address Base ......................................... 0x%016lx\n",
    MemoryPowerNode->AddressBase
    );
  Print (
    L"    Address Length ....................................... 0x%016lx\n",
    MemoryPowerNode->AddressLength
    );
  Print (
    L"    Number Of Power States ............................... 0x%02x\n",
    MemoryPowerNode->NumberOfPowerStates
    );
  Print (
    L"    Number Of Physical Components ........................ 0x%02x\n",
    MemoryPowerNode->NumberOfPhysicalComponents
    );

  PowerState = (EFI_ACPI_5_0_MPST_MEMORY_POWER_STATE *)(MemoryPowerNode + 1);
  for (Index = 0; Index < MemoryPowerNode->NumberOfPowerStates; Index++) {
    Print (
      L"    Power State Value .................................... 0x%02x\n",
      PowerState->PowerStateValue
      );
    Print (
      L"    Power State Info Index ............................... 0x%02x\n",
      PowerState->PowerStateInformationIndex
      );
    PowerState++;
  }

  PhysicalComponentIdentifier = (UINT16 *)(PowerState);
  for (Index = 0; Index < MemoryPowerNode->NumberOfPhysicalComponents; Index++) {
    Print (
      L"    Physical Component Identifier ........................ 0x%04x\n",
      *PhysicalComponentIdentifier
      );
    PhysicalComponentIdentifier++;
  }
  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
DumpMemoryPowerStateCharacteristic (
  EFI_ACPI_5_0_MPST_MEMORY_POWER_STATE_CHARACTERISTICS_STRUCTURE  *MemoryPowerStateCharacteristic
  )
{
  if (MemoryPowerStateCharacteristic == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       Memory Power State Characteristic                                 *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  Memory Power State Characteristic address .............. 0x%016lx\n" :
    L"  Memory Power State Characteristic address .............. 0x%08x\n",
    MemoryPowerStateCharacteristic
    );
  Print (
    L"    Power State Structure ID ............................. 0x%02x\n",
    MemoryPowerStateCharacteristic->PowerStateStructureID
    );
  Print (
    L"    Flag ................................................. 0x%02x\n",
    MemoryPowerStateCharacteristic->Flag
    );
  Print (
    L"      Memory Contents Preserved .......................... 0x%02x\n",
    MemoryPowerStateCharacteristic->Flag & EFI_ACPI_5_0_MPST_MEMORY_POWER_STATE_CHARACTERISTICS_STRUCTURE_FLAG_MEMORY_CONTENT_PRESERVED
    );
  Print (
    L"      Autonomous Memory Power State Entry ................ 0x%02x\n",
    MemoryPowerStateCharacteristic->Flag & EFI_ACPI_5_0_MPST_MEMORY_POWER_STATE_CHARACTERISTICS_STRUCTURE_FLAG_AUTONOMOUS_MEMORY_POWER_STATE_ENTRY
    );
  Print (
    L"      Autonomous Memory Power State Exit ................. 0x%02x\n",
    MemoryPowerStateCharacteristic->Flag & EFI_ACPI_5_0_MPST_MEMORY_POWER_STATE_CHARACTERISTICS_STRUCTURE_FLAG_AUTONOMOUS_MEMORY_POWER_STATE_EXIT
    );
  Print (
    L"    Average Power Consumed in MPS0 ....................... 0x%08x\n",
    MemoryPowerStateCharacteristic->AveragePowerConsumedInMPS0
    );
  Print (
    L"    Relative Power Saving to MPS0 ........................ 0x%08x\n",
    MemoryPowerStateCharacteristic->RelativePowerSavingToMPS0
    );
  Print (
    L"    Exit Latency To Mps0 ................................. 0x%016lx\n",
    MemoryPowerStateCharacteristic->ExitLatencyToMPS0
    );
  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
EFIAPI
DumpAcpiMPST (
  VOID  *Table
  )
{
  EFI_ACPI_5_0_MEMORY_POWER_STATUS_TABLE                          *Mpst;
  UINTN                                                           Index;
  EFI_ACPI_5_0_MPST_MEMORY_POWER_NODE_TABLE                       *MemoryPowerNodeTable;
  EFI_ACPI_5_0_MPST_MEMORY_POWER_STRUCTURE                        *MemoryPowerNode;
  EFI_ACPI_5_0_MPST_MEMORY_POWER_STATE_CHARACTERISTICS_TABLE      *MemoryPowerStateCharacteristicTable;
  EFI_ACPI_5_0_MPST_MEMORY_POWER_STATE_CHARACTERISTICS_STRUCTURE  *MemoryPowerStateCharacteristic;

  Mpst = Table;
  if (Mpst == NULL) {
    return;
  }
  
  //
  // Dump Mpst table
  //
  Print (
    L"*****************************************************************************\n"
    L"*         Memory Power State Table                                          *\n"
    L"*****************************************************************************\n"
    );
    
  if (GetAcpiDumpPropertyDumpData()) {
    DumpAcpiHex (Mpst->Header.Length, Mpst);
  }
  
  if (!GetAcpiDumpPropertyDumpVerb()) {
    goto Done;
  }

  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"MPST address ............................................. 0x%016lx\n" :
    L"MPST address ............................................. 0x%08x\n",
    Mpst
    );
  
  DumpAcpiTableHeader(&(Mpst->Header));
  
  Print (
    L"  Table Contents:\n"
    );
  Print (
    L"    Platform Communication Channel Identifier ............ 0x%08x\n",
    Mpst->PlatformCommunicationChannelIdentifier
    );

  MemoryPowerNodeTable = (EFI_ACPI_5_0_MPST_MEMORY_POWER_NODE_TABLE *)(Mpst + 1);
  MemoryPowerNode = (EFI_ACPI_5_0_MPST_MEMORY_POWER_STRUCTURE *)(MemoryPowerNodeTable + 1);
  for (Index = 0; Index < MemoryPowerNodeTable->MemoryPowerNodeCount; Index++) {
    DumpMemoryPowerNode (MemoryPowerNode);
    MemoryPowerNode = (EFI_ACPI_5_0_MPST_MEMORY_POWER_STRUCTURE *)((UINTN)MemoryPowerNode + MemoryPowerNode->Length);
  }
  
  MemoryPowerStateCharacteristicTable = (EFI_ACPI_5_0_MPST_MEMORY_POWER_STATE_CHARACTERISTICS_TABLE *)MemoryPowerNode;
  MemoryPowerStateCharacteristic = (EFI_ACPI_5_0_MPST_MEMORY_POWER_STATE_CHARACTERISTICS_STRUCTURE *)(MemoryPowerStateCharacteristicTable + 1);
  for (Index = 0; Index < MemoryPowerStateCharacteristicTable->MemoryPowerStateCharacteristicsCount; Index++) {
    DumpMemoryPowerStateCharacteristic (MemoryPowerStateCharacteristic);
    MemoryPowerStateCharacteristic ++;
  }

Done:
  Print (         
    L"*****************************************************************************\n\n"
    );
  
  return;
}

EFI_STATUS
EFIAPI
DumpAcpiMPSTLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return RegisterDumpAcpiTable (EFI_ACPI_5_0_MEMORY_POWER_STATE_TABLE_SIGNATURE, DumpAcpiMPST);
}

