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

GUID mEfiAcpiNfitGuidVolatileMemoryRegion = EFI_ACPI_6_0_NFIT_GUID_VOLATILE_MEMORY_REGION;
GUID mEfiAcpiNfitGuidByteAddressablePersistentMemoryRegion = EFI_ACPI_6_0_NFIT_GUID_BYTE_ADDRESSABLE_PERSISTENT_MEMORY_REGION;
GUID mEfiAcpiNfitGuidNVDIMMControlRegion = EFI_ACPI_6_0_NFIT_GUID_NVDIMM_CONTROL_REGION;
GUID mEfiAcpiNfitGuidNVDIMMBlockDataWindowRegion = EFI_ACPI_6_0_NFIT_GUID_NVDIMM_BLOCK_DATA_WINDOW_REGION;
GUID mEfiAcpiNfitGuidRamDiskSupportingVirtualDiskRegionVolatile = EFI_ACPI_6_0_NFIT_GUID_RAM_DISK_SUPPORTING_VIRTUAL_DISK_REGION_VOLATILE;
GUID mEfiAcpiNfitGuidRamDiskSupportingVirtualCDRegionVolatile = EFI_ACPI_6_0_NFIT_GUID_RAM_DISK_SUPPORTING_VIRTUAL_CD_REGION_VOLATILE;
GUID mEfiAcpiNfitGuidRamDiskSupportingVirtualDiskRegionPersistent = EFI_ACPI_6_0_NFIT_GUID_RAM_DISK_SUPPORTING_VIRTUAL_DISK_REGION_PERSISTENT;
GUID mEfiAcpiNfitGuidRamDiskSupportingVirtualCDRegionPersistent = EFI_ACPI_6_0_NFIT_GUID_RAM_DISK_SUPPORTING_VIRTUAL_CD_REGION_PERSISTENT;

VOID
DumpNfitFlushHintAddressStruct (
  EFI_ACPI_6_0_NFIT_FLUSH_HINT_ADDRESS_STRUCTURE *FlushHintAddress
  )
{
  UINT64                                      *Address;
  UINTN                                       Index;

  if (FlushHintAddress == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       Flush Hint Address Structure                                      *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  Flush Hint Address Structure address ................... 0x%016lx\n" :
    L"  Flush Hint Address Structure address ................... 0x%08x\n",
    FlushHintAddress
    );
  Print (
    L"    Type ................................................. 0x%04x\n",
    FlushHintAddress->Type
    );
  Print (
    L"    Length ............................................... 0x%04x\n",
    FlushHintAddress->Length
    );
  Print (
    L"    NFIT Device Handle ................................... 0x%08x\n",
    *(UINT32 *)&FlushHintAddress->NFITDeviceHandle
    );
  Print (
    L"      DIMM Number ........................................ 0x%08x\n",
    FlushHintAddress->NFITDeviceHandle.DIMMNumber
    );
  Print (
    L"      Memory Channel Number .............................. 0x%08x\n",
    FlushHintAddress->NFITDeviceHandle.MemoryChannelNumber
    );
  Print (
    L"      Memory Controller ID ............................... 0x%08x\n",
    FlushHintAddress->NFITDeviceHandle.MemoryControllerID
    );
  Print (
    L"      Socket ID .......................................... 0x%08x\n",
    FlushHintAddress->NFITDeviceHandle.SocketID
    );
  Print (
    L"      Node Controller ID ................................. 0x%08x\n",
    FlushHintAddress->NFITDeviceHandle.NodeControllerID
    );
  Print (
    L"    Number Of Flush Hint Addresses ....................... 0x%04x\n",
    FlushHintAddress->NumberOfFlushHintAddresses
    );

  Address = (UINT64 *)(FlushHintAddress + 1);
  for (Index = 0; Index < FlushHintAddress->NumberOfFlushHintAddresses; Index++) {
    Print (
      L"    Flush Hint Address %03d ............................... 0x%016lx\n",
      Index,
      Address[Index]
      );
  }

  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
DumpNfitNVDIMMBlockDataWindowRegionStruct (
  EFI_ACPI_6_0_NFIT_NVDIMM_BLOCK_DATA_WINDOW_REGION_STRUCTURE *NVDIMMBlockDataWindowRegion
  )
{
  if (NVDIMMBlockDataWindowRegion == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       NVDIMM Block Data Window Region Structure                         *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  NVDIMM Block Data Window Region Structure address ...... 0x%016lx\n" :
    L"  NVDIMM Block Data Window Region Structure address ...... 0x%08x\n",
    NVDIMMBlockDataWindowRegion
    );
  Print (
    L"    Type ................................................. 0x%04x\n",
    NVDIMMBlockDataWindowRegion->Type
    );
  Print (
    L"    Length ............................................... 0x%04x\n",
    NVDIMMBlockDataWindowRegion->Length
    );
  Print (
    L"    NVDIMM Control Region Structure Index ................ 0x%04x\n",
    NVDIMMBlockDataWindowRegion->NVDIMMControlRegionStructureIndex
    );
  Print (
    L"    Number Of Block Data Windows ......................... 0x%04x\n",
    NVDIMMBlockDataWindowRegion->NumberOfBlockDataWindows
    );
  Print (
    L"    Block Data Window Start Offset ....................... 0x%016lx\n",
    NVDIMMBlockDataWindowRegion->BlockDataWindowStartOffset
    );
  Print (
    L"    Size Of Block Data Window ............................ 0x%016lx\n",
    NVDIMMBlockDataWindowRegion->SizeOfBlockDataWindow
    );
  Print (
    L"    Block Accessible Memory Capacity ..................... 0x%016lx\n",
    NVDIMMBlockDataWindowRegion->BlockAccessibleMemoryCapacity
    );
  Print (
    L"    Beginning Address Of First Block In Block Accessible . 0x%016lx\n",
    NVDIMMBlockDataWindowRegion->BeginningAddressOfFirstBlockInBlockAccessibleMemory
    );

  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
DumpNfitNVDIMMControlRegionStruct (
  EFI_ACPI_6_0_NFIT_NVDIMM_CONTROL_REGION_STRUCTURE *NVDIMMControlRegion
  )
{
  if (NVDIMMControlRegion == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       NVDIMM Control Region Structure                                   *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  NVDIMM Control Region Structure address ................ 0x%016lx\n" :
    L"  NVDIMM Control Region Structure address ................ 0x%08x\n",
    NVDIMMControlRegion
    );
  Print (
    L"    Type ................................................. 0x%04x\n",
    NVDIMMControlRegion->Type
    );
  Print (
    L"    Length ............................................... 0x%04x\n",
    NVDIMMControlRegion->Length
    );
  Print (
    L"    NVDIMM Control Region Structure Index ................ 0x%04x\n",
    NVDIMMControlRegion->NVDIMMControlRegionStructureIndex
    );
  Print (
    L"    Vendor ID ............................................ 0x%04x\n",
    NVDIMMControlRegion->VendorID
    );
  Print (
    L"    Device ID ............................................ 0x%04x\n",
    NVDIMMControlRegion->DeviceID
    );
  Print (
    L"    Revision ID .......................................... 0x%04x\n",
    NVDIMMControlRegion->RevisionID
    );
  Print (
    L"    Subsystem Vendor ID .................................. 0x%04x\n",
    NVDIMMControlRegion->SubsystemVendorID
    );
  Print (
    L"    Subsystem Device ID .................................. 0x%04x\n",
    NVDIMMControlRegion->SubsystemDeviceID
    );
  Print (
    L"    Subsystem Revision ID ................................ 0x%04x\n",
    NVDIMMControlRegion->SubsystemRevisionID
    );
  Print (
    L"    Serial Number ........................................ 0x%08x\n",
    NVDIMMControlRegion->SerialNumber
    );
  Print (
    L"    Region Format Interface Code ......................... 0x%04x\n",
    NVDIMMControlRegion->RegionFormatInterfaceCode
    );
  Print (
    L"    Number Of Block Control Windows ...................... 0x%04x\n",
    NVDIMMControlRegion->NumberOfBlockControlWindows
    );
  Print (
    L"    Size Of Block Control Window ......................... 0x%016lx\n",
    NVDIMMControlRegion->SizeOfBlockControlWindow
    );
  Print (
    L"    Command Register Offset In Block Control Window ...... 0x%016lx\n",
    NVDIMMControlRegion->CommandRegisterOffsetInBlockControlWindow
    );
  Print (
    L"    Size Of Command Register In Block Control Windows .... 0x%016lx\n",
    NVDIMMControlRegion->SizeOfCommandRegisterInBlockControlWindows
    );
  Print (
    L"    Status Register Offset In Block Control Window ....... 0x%016lx\n",
    NVDIMMControlRegion->StatusRegisterOffsetInBlockControlWindow
    );
  Print (
    L"    Size Of Status Register In Block Control Windows ..... 0x%016lx\n",
    NVDIMMControlRegion->SizeOfStatusRegisterInBlockControlWindows
    );
  Print (
    L"    NVDIMM Control Region Flag ........................... 0x%04x\n",
    NVDIMMControlRegion->NVDIMMControlRegionFlag
    );
  Print (
    L"      Block Data Windows implementation is buffered ...... 0x%04x\n",
    NVDIMMControlRegion->NVDIMMControlRegionFlag & EFI_ACPI_6_0_NFIT_NVDIMM_CONTROL_REGION_FLAGS_BLOCK_DATA_WINDOWS_BUFFERED
    );

  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
DumpNfitSMBIOSManagementInformationStruct (
  EFI_ACPI_6_0_NFIT_SMBIOS_MANAGEMENT_INFORMATION_STRUCTURE *SMBIOSManagementInformation
  )
{
  UINT8                                       *Data;
  UINTN                                       DataSize;

  if (SMBIOSManagementInformation == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       SMBIOS Management Information Structure                           *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  SMBIOS Management Information Structure address ........ 0x%016lx\n" :
    L"  SMBIOS Management Information Structure address ........ 0x%08x\n",
    SMBIOSManagementInformation
    );
  Print (
    L"    Type ................................................. 0x%04x\n",
    SMBIOSManagementInformation->Type
    );
  Print (
    L"    Length ............................................... 0x%04x\n",
    SMBIOSManagementInformation->Length
    );

  Data = (UINT8 *)(SMBIOSManagementInformation + 1);
  DataSize = SMBIOSManagementInformation->Length - sizeof(*SMBIOSManagementInformation);
  Print (
    L"    Data ................................................. \n"
    );
  DumpAcpiHex (DataSize, Data);

  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
DumpNfitInterleaveStruct (
  EFI_ACPI_6_0_NFIT_INTERLEAVE_STRUCTURE *Interleave
  )
{
  UINT32                                      *LineOffset;
  UINTN                                       Index;

  if (Interleave == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       Interleave Structure                                              *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  Interleave Structure address ........................... 0x%016lx\n" :
    L"  Interleave Structure address ........................... 0x%08x\n",
    Interleave
    );
  Print (
    L"    Type ................................................. 0x%04x\n",
    Interleave->Type
    );
  Print (
    L"    Length ............................................... 0x%04x\n",
    Interleave->Length
    );
  Print (
    L"    Interleave Structure Index ........................... 0x%04x\n",
    Interleave->InterleaveStructureIndex
    );
  Print (
    L"    Number Of Lines ...................................... 0x%08x\n",
    Interleave->NumberOfLines
    );
  Print (
    L"    Line Size ............................................ 0x%08x\n",
    Interleave->LineSize
    );

  LineOffset = (UINT32 *)(Interleave + 1);
  for (Index = 0; Index < Interleave->NumberOfLines; Index++) {
    Print (
      L"    Line %03d 0ffset ...................................... 0x%08x\n",
      Index,
      LineOffset[Index]
      );
  }

  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
DumpNfitMemoryDeviceToSystemPhysicalAddressRangeMappingStruct (
  EFI_ACPI_6_0_NFIT_MEMORY_DEVICE_TO_SYSTEM_ADDRESS_RANGE_MAP_STRUCTURE *MemoryDeviceToSystemPhysicalAddressRangeMapping
  )
{
  if (MemoryDeviceToSystemPhysicalAddressRangeMapping == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       Memory Device To System Physical Address Range Mapping Structure  *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  Memory Device To System Physical Address Range Mapping . 0x%016lx\n" :
    L"  Memory Device To System Physical Address Range Mapping . 0x%08x\n",
    MemoryDeviceToSystemPhysicalAddressRangeMapping
    );
  Print (
    L"    Type ................................................. 0x%04x\n",
    MemoryDeviceToSystemPhysicalAddressRangeMapping->Type
    );
  Print (
    L"    Length ............................................... 0x%04x\n",
    MemoryDeviceToSystemPhysicalAddressRangeMapping->Length
    );
  Print (
    L"    NFIT Device Handle ................................... 0x%08x\n",
    *(UINT32 *)&MemoryDeviceToSystemPhysicalAddressRangeMapping->NFITDeviceHandle
    );
  Print (
    L"      DIMM Number ........................................ 0x%08x\n",
    MemoryDeviceToSystemPhysicalAddressRangeMapping->NFITDeviceHandle.DIMMNumber
    );
  Print (
    L"      Memory Channel Number .............................. 0x%08x\n",
    MemoryDeviceToSystemPhysicalAddressRangeMapping->NFITDeviceHandle.MemoryChannelNumber
    );
  Print (
    L"      Memory Controller ID ............................... 0x%08x\n",
    MemoryDeviceToSystemPhysicalAddressRangeMapping->NFITDeviceHandle.MemoryControllerID
    );
  Print (
    L"      Socket ID .......................................... 0x%08x\n",
    MemoryDeviceToSystemPhysicalAddressRangeMapping->NFITDeviceHandle.SocketID
    );
  Print (
    L"      Node Controller ID ................................. 0x%08x\n",
    MemoryDeviceToSystemPhysicalAddressRangeMapping->NFITDeviceHandle.NodeControllerID
    );
  Print (
    L"    Memory Device  Physical ID ........................... 0x%04x\n",
    MemoryDeviceToSystemPhysicalAddressRangeMapping->MemoryDevicePhysicalID
    );
  Print (
    L"    Memory Device Region ID .............................. 0x%04x\n",
    MemoryDeviceToSystemPhysicalAddressRangeMapping->MemoryDeviceRegionID
    );
  Print (
    L"    SPA Range Structure Index ............................ 0x%04x\n",
    MemoryDeviceToSystemPhysicalAddressRangeMapping->SPARangeStructureIndex
    );
  Print (
    L"    NVDIMM Control Region Structure Index ................ 0x%04x\n",
    MemoryDeviceToSystemPhysicalAddressRangeMapping->NVDIMMControlRegionStructureIndex
    );
  Print (
    L"    Memory Device Region Size ............................ 0x%016lx\n",
    MemoryDeviceToSystemPhysicalAddressRangeMapping->MemoryDeviceRegionSize
    );
  Print (
    L"    Region Offset ........................................ 0x%016lx\n",
    MemoryDeviceToSystemPhysicalAddressRangeMapping->RegionOffset
    );
  Print (
    L"    Memory Device Physical Address Region Base ........... 0x%016lx\n",
    MemoryDeviceToSystemPhysicalAddressRangeMapping->MemoryDevicePhysicalAddressRegionBase
    );
  Print (
    L"    Interleave Structure Index ........................... 0x%04x\n",
    MemoryDeviceToSystemPhysicalAddressRangeMapping->InterleaveStructureIndex
    );
  Print (
    L"    Interleave Ways ...................................... 0x%04x\n",
    MemoryDeviceToSystemPhysicalAddressRangeMapping->InterleaveWays
    );
  Print (
    L"    Memory Device State Flags ............................ 0x%04x\n",
    MemoryDeviceToSystemPhysicalAddressRangeMapping->MemoryDeviceStateFlags
    );
  Print (
    L"      previous SAVE to the Memory Device failed .......... 0x%04x\n",
    MemoryDeviceToSystemPhysicalAddressRangeMapping->MemoryDeviceStateFlags & EFI_ACPI_6_0_NFIT_MEMORY_DEVICE_STATE_FLAGS_PREVIOUS_SAVE_FAIL
    );
  Print (
    L"      last RESTORE from the Memory Device failed ......... 0x%04x\n",
    MemoryDeviceToSystemPhysicalAddressRangeMapping->MemoryDeviceStateFlags & EFI_ACPI_6_0_NFIT_MEMORY_DEVICE_STATE_FLAGS_LAST_RESTORE_FAIL
    );
  Print (
    L"      platform flush of data to Memory Device failed ..... 0x%04x\n",
    MemoryDeviceToSystemPhysicalAddressRangeMapping->MemoryDeviceStateFlags & EFI_ACPI_6_0_NFIT_MEMORY_DEVICE_STATE_FLAGS_PLATFORM_FLUSH_FAIL
    );
  Print (
    L"      Memory Device is not armed prior to OSPM hand off .. 0x%04x\n",
    MemoryDeviceToSystemPhysicalAddressRangeMapping->MemoryDeviceStateFlags & EFI_ACPI_6_0_NFIT_MEMORY_DEVICE_STATE_FLAGS_NOT_ARMED_PRIOR_TO_OSPM_HAND_OFF
    );
  Print (
    L"      SMART and health events prior OSPM handoff ......... 0x%04x\n",
    MemoryDeviceToSystemPhysicalAddressRangeMapping->MemoryDeviceStateFlags & EFI_ACPI_6_0_NFIT_MEMORY_DEVICE_STATE_FLAGS_SMART_HEALTH_EVENTS_PRIOR_OSPM_HAND_OFF
    );
  Print (
    L"      firmware enabled to notify SMART and health events . 0x%04x\n",
    MemoryDeviceToSystemPhysicalAddressRangeMapping->MemoryDeviceStateFlags & EFI_ACPI_6_0_NFIT_MEMORY_DEVICE_STATE_FLAGS_FIRMWARE_ENABLED_TO_NOTIFY_OSPM_ON_SMART_HEALTH_EVENTS
    );

  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
DumpNfitSystemPhysicalAddressRangeStruct (
  EFI_ACPI_6_0_NFIT_SYSTEM_PHYSICAL_ADDRESS_RANGE_STRUCTURE *SystemPhysicalAddressRangeStruct
  )
{
  if (SystemPhysicalAddressRangeStruct == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       System Physical Address Range Structure                           *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  System Physical Address Range Structure address ........ 0x%016lx\n" :
    L"  System Physical Address Range Structure address ........ 0x%08x\n",
    SystemPhysicalAddressRangeStruct
    );
  Print (
    L"    Type ................................................. 0x%04x\n",
    SystemPhysicalAddressRangeStruct->Type
    );
  Print (
    L"    Length ............................................... 0x%04x\n",
    SystemPhysicalAddressRangeStruct->Length
    );
  Print (
    L"    SPA Range Structure Index ............................ 0x%02x\n",
    SystemPhysicalAddressRangeStruct->SPARangeStructureIndex
    );
  Print (
    L"    Flags ................................................ 0x%04x\n",
    SystemPhysicalAddressRangeStruct->Flags
    );
  Print (
    L"      Control Region for management during hot add/online  0x%04x\n",
    SystemPhysicalAddressRangeStruct->Flags & EFI_ACPI_6_0_NFIT_SYSTEM_PHYSICAL_ADDRESS_RANGE_FLAGS_CONTROL_REGION_FOR_MANAGEMENT
    );
  Print (
    L"      Proximity Domain field valid ....................... 0x%04x\n",
    SystemPhysicalAddressRangeStruct->Flags & EFI_ACPI_6_0_NFIT_SYSTEM_PHYSICAL_ADDRESS_RANGE_FLAGS_CONTROL_REGION_FOR_MANAGEMENT
    );
  Print (
    L"    Proximity Domain ..................................... 0x%08x\n",
    SystemPhysicalAddressRangeStruct->ProximityDomain
    );
  Print (
    L"    Address Range Type GUID .............................. %g\n",
    &SystemPhysicalAddressRangeStruct->AddressRangeTypeGUID
    );
  if (CompareGuid (&SystemPhysicalAddressRangeStruct->AddressRangeTypeGUID, &mEfiAcpiNfitGuidVolatileMemoryRegion)) {
    Print (
      L"      Volatile Memory Region\n"
      );
  } else if (CompareGuid (&SystemPhysicalAddressRangeStruct->AddressRangeTypeGUID, &mEfiAcpiNfitGuidByteAddressablePersistentMemoryRegion)) {
    Print (
      L"      Byte Addressable Persistent Memory (PM) Region\n"
      );
  } else if (CompareGuid (&SystemPhysicalAddressRangeStruct->AddressRangeTypeGUID, &mEfiAcpiNfitGuidNVDIMMControlRegion)) {
    Print (
      L"      NVDIMM Control Region\n"
      );
  } else if (CompareGuid (&SystemPhysicalAddressRangeStruct->AddressRangeTypeGUID, &mEfiAcpiNfitGuidNVDIMMBlockDataWindowRegion)) {
    Print (
      L"      NVDIMM Block Data Window Region\n"
      );
  } else if (CompareGuid (&SystemPhysicalAddressRangeStruct->AddressRangeTypeGUID, &mEfiAcpiNfitGuidRamDiskSupportingVirtualDiskRegionVolatile)) {
    Print (
      L"      A RAM Disk Supporting a Virtual Disk Region - Volatile\n"
      );
  } else if (CompareGuid (&SystemPhysicalAddressRangeStruct->AddressRangeTypeGUID, &mEfiAcpiNfitGuidRamDiskSupportingVirtualCDRegionVolatile)) {
    Print (
      L"      A RAM Disk Supporting a Virtual CD Region - Volatile\n"
      );
  } else if (CompareGuid (&SystemPhysicalAddressRangeStruct->AddressRangeTypeGUID, &mEfiAcpiNfitGuidRamDiskSupportingVirtualDiskRegionPersistent)) {
    Print (
      L"      A RAM Disk Supporting a Virtual Disk Region - Persistent\n"
      );
  } else if (CompareGuid (&SystemPhysicalAddressRangeStruct->AddressRangeTypeGUID, &mEfiAcpiNfitGuidRamDiskSupportingVirtualCDRegionPersistent)) {
    Print (
      L"      A RAM Disk Supporting a Virtual CD Region - Persistent\n"
      );
  }
  Print (
    L"    System Physical Address Range Base ................... 0x%016lx\n",
    SystemPhysicalAddressRangeStruct->SystemPhysicalAddressRangeBase
    );
  Print (
    L"    System Physical Address Range Length ................. 0x%016lx\n",
    SystemPhysicalAddressRangeStruct->SystemPhysicalAddressRangeLength
    );
  Print (
    L"    Address Range Memory Mapping Attribute ............... 0x%016lx\n",
    SystemPhysicalAddressRangeStruct->AddressRangeMemoryMappingAttribute
    );
  switch (SystemPhysicalAddressRangeStruct->AddressRangeMemoryMappingAttribute) {
  case EFI_MEMORY_UC:
    Print (
      L"      EFI_MEMORY_UC\n"
      );
    break;
  case EFI_MEMORY_WC:
    Print (
      L"      EFI_MEMORY_WC\n"
      );
    break;
  case EFI_MEMORY_WT:
    Print (
      L"      EFI_MEMORY_WT\n"
      );
    break;
  case EFI_MEMORY_WB:
    Print (
      L"      EFI_MEMORY_WB\n"
      );
    break;
  case EFI_MEMORY_UCE:
    Print (
      L"      EFI_MEMORY_UCE\n"
      );
    break;
  case EFI_MEMORY_WP:
    Print (
      L"      EFI_MEMORY_WP\n"
      );
    break;
  case EFI_MEMORY_RP:
    Print (
      L"      EFI_MEMORY_RP\n"
      );
    break;
  case EFI_MEMORY_XP:
    Print (
      L"      EFI_MEMORY_XP\n"
      );
    break;
  case 0x00008000:
    Print (
      L"      EFI_MEMORY_NV\n"
      );
    break;
  case 0x00010000:
    Print (
      L"      EFI_MEMORY_MORE_RELIABLE\n"
      );
    break;
  default:
    break;
  }

  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
EFIAPI
DumpAcpiNFIT (
  VOID  *Table
  )
{
  EFI_ACPI_6_0_NVDIMM_FIRMWARE_INTERFACE_TABLE *Nfit;
  EFI_ACPI_6_0_NFIT_STRUCTURE_HEADER           *NfitHeader;
  INTN                                         NfitLen;

  Nfit = Table;
  if (Nfit == NULL) {
    return;
  }
  
  //
  // Dump Nfit table
  //
  Print (
    L"*****************************************************************************\n"
    L"*         NFIT Table                                                        *\n"
    L"*****************************************************************************\n"
    );
    
  if (GetAcpiDumpPropertyDumpData()) {
    DumpAcpiHex (Nfit->Header.Length, Nfit);
  }
  
  if (!GetAcpiDumpPropertyDumpVerb()) {
    goto Done;
  }

  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"NFIT address ............................................. 0x%016lx\n" :
    L"NFIT address ............................................. 0x%08x\n",
    Nfit
    );
  
  DumpAcpiTableHeader(&(Nfit->Header));
  
  Print (
    L"  Table Contents:\n"
    );

  NfitLen  = Nfit->Header.Length - sizeof(EFI_ACPI_6_0_NVDIMM_FIRMWARE_INTERFACE_TABLE);
  NfitHeader = (EFI_ACPI_6_0_NFIT_STRUCTURE_HEADER *)(Nfit + 1);
  while (NfitLen > 0) {
    switch (NfitHeader->Type) {
    case EFI_ACPI_6_0_NFIT_SYSTEM_PHYSICAL_ADDRESS_RANGE_STRUCTURE_TYPE:
      DumpNfitSystemPhysicalAddressRangeStruct ((EFI_ACPI_6_0_NFIT_SYSTEM_PHYSICAL_ADDRESS_RANGE_STRUCTURE *)NfitHeader);
      break;
    case EFI_ACPI_6_0_NFIT_MEMORY_DEVICE_TO_SYSTEM_ADDRESS_RANGE_MAP_STRUCTURE_TYPE:
      DumpNfitMemoryDeviceToSystemPhysicalAddressRangeMappingStruct ((EFI_ACPI_6_0_NFIT_MEMORY_DEVICE_TO_SYSTEM_ADDRESS_RANGE_MAP_STRUCTURE *)NfitHeader);
      break;
    case EFI_ACPI_6_0_NFIT_INTERLEAVE_STRUCTURE_TYPE:
      DumpNfitInterleaveStruct ((EFI_ACPI_6_0_NFIT_INTERLEAVE_STRUCTURE *)NfitHeader);
      break;
    case EFI_ACPI_6_0_NFIT_SMBIOS_MANAGEMENT_INFORMATION_STRUCTURE_TYPE:
      DumpNfitSMBIOSManagementInformationStruct ((EFI_ACPI_6_0_NFIT_SMBIOS_MANAGEMENT_INFORMATION_STRUCTURE *)NfitHeader);
      break;
    case EFI_ACPI_6_0_NFIT_NVDIMM_CONTROL_REGION_STRUCTURE_TYPE:
      DumpNfitNVDIMMControlRegionStruct ((EFI_ACPI_6_0_NFIT_NVDIMM_CONTROL_REGION_STRUCTURE *)NfitHeader);
      break;
    case EFI_ACPI_6_0_NFIT_NVDIMM_BLOCK_DATA_WINDOW_REGION_STRUCTURE_TYPE:
      DumpNfitNVDIMMBlockDataWindowRegionStruct ((EFI_ACPI_6_0_NFIT_NVDIMM_BLOCK_DATA_WINDOW_REGION_STRUCTURE *)NfitHeader);
      break;
    case EFI_ACPI_6_0_NFIT_FLUSH_HINT_ADDRESS_STRUCTURE_TYPE:
      DumpNfitFlushHintAddressStruct ((EFI_ACPI_6_0_NFIT_FLUSH_HINT_ADDRESS_STRUCTURE *)NfitHeader);
      break;
    default:
      break;
    }
    NfitLen -= NfitHeader->Length;
    NfitHeader = (EFI_ACPI_6_0_NFIT_STRUCTURE_HEADER *)((UINTN)NfitHeader + NfitHeader->Length);
  }
 
Done:
  Print (         
    L"*****************************************************************************\n\n"
    );
  
  return;
}

EFI_STATUS
EFIAPI
DumpAcpiNFITLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return RegisterDumpAcpiTable (EFI_ACPI_6_0_NVDIMM_FIRMWARE_INTERFACE_TABLE_STRUCTURE_SIGNATURE, DumpAcpiNFIT);
}
