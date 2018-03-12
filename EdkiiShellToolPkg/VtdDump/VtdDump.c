/** @file

Copyright (c) 2018, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <PiDxe.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiLib.h>
#include <Library/HobLib.h>
#include <Library/PciSegmentLib.h>
#include <Library/IoLib.h>
#include <Library/DevicePathLib.h>
#include <Protocol/IoMmu.h>
#include <Protocol/PciIo.h>
#include <Protocol/PlatformVtdPolicy.h>
#include <Guid/Acpi.h>
#include <IndustryStandard/Pci.h>
#include <IndustryStandard/DmaRemappingReportingTable.h>
#include <IndustryStandard/Vtd.h>

#include "Vtd/Map.h"

#pragma pack(1)

typedef struct {
  EFI_ACPI_DESCRIPTION_HEADER  Header;
  UINT32                       Entry;
} RSDT_TABLE;

typedef struct {
  EFI_ACPI_DESCRIPTION_HEADER  Header;
  UINT64                       Entry;
} XSDT_TABLE;

#pragma pack()

#define EXCEPTION_RECORD_SIGNATURE  SIGNATURE_32 ('E', 'X', 'C', 'P')

typedef struct {
  UINT32           Signature;
  LIST_ENTRY       Link;
  UINT16           Segment;
  UINT8            Bus;
  UINT8            Device;
  UINT8            Function;
} EXCEPTION_RECORD;

#define RMRR_RECORD_SIGNATURE  SIGNATURE_32 ('R', 'M', 'R', 'S')

typedef struct {
  UINT32           Signature;
  LIST_ENTRY       Link;
  UINT16           Segment;
  UINT8            Bus;
  UINT8            Device;
  UINT8            Function;
  UINT64           Address;
  UINT64           Length;
} RMRR_RECORD;

LIST_ENTRY                        gExceptionRecord = INITIALIZE_LIST_HEAD_VARIABLE(gExceptionRecord);

LIST_ENTRY                        gRmrrRecord = INITIALIZE_LIST_HEAD_VARIABLE(gRmrrRecord);

#define VTD_64BITS_ADDRESS(Lo, Hi) (LShiftU64 (Lo, 12) | LShiftU64 (Hi, 32))

//
// This is the initial max PCI DATA number.
// The number may be enlarged later.
//
#define MAX_VTD_PCI_DATA_NUMBER             0x100

typedef struct {
  UINT8                            DeviceType;
  VTD_SOURCE_ID                    PciSourceId;
  EDKII_PLATFORM_VTD_PCI_DEVICE_ID PciDeviceId;
  // for statistic analysis
  UINTN                            AccessCount;
} PCI_DEVICE_DATA;

typedef struct {
  BOOLEAN                          IncludeAllFlag;
  UINTN                            PciDeviceDataNumber;
  UINTN                            PciDeviceDataMaxNumber;
  PCI_DEVICE_DATA                  *PciDeviceData;
} PCI_DEVICE_INFORMATION;

typedef struct {
  UINTN                            VtdUnitBaseAddress;
  UINT16                           Segment;
  VTD_CAP_REG                      CapReg;
  VTD_ECAP_REG                     ECapReg;
  PCI_DEVICE_INFORMATION           PciDeviceInfo;
} VTD_UNIT_INFORMATION;

EFI_ACPI_DMAR_HEADER             *mAcpiDmarTable = NULL;
UINTN                            mVtdUnitNumber = 0;
VTD_UNIT_INFORMATION             *mVtdUnitInformation;
BOOLEAN                          mHasError;

/**
  The scan bus callback function.

  It is called in PCI bus scan for each PCI device under the bus.

  @param[in]  Context               The context of the callback.
  @param[in]  Segment               The segment of the source.
  @param[in]  Bus                   The bus of the source.
  @param[in]  Device                The device of the source.
  @param[in]  Function              The function of the source.

  @retval EFI_SUCCESS           The specific PCI device is processed in the callback.
**/
typedef
EFI_STATUS
(EFIAPI *SCAN_BUS_FUNC_CALLBACK_FUNC) (
  IN VOID           *Context,
  IN UINT16         Segment,
  IN UINT8          Bus,
  IN UINT8          Device,
  IN UINT8          Function
  );

LIST_ENTRY                        *gMaps;

VOID
GetIoMmuMapList (
  IN MAP_INFO *IMapping
  )
{
  LIST_ENTRY      *ListEntry;
  MAP_INFO        *Mapping;

  ListEntry = &IMapping->Link;
  for (ListEntry = ListEntry->ForwardLink;
       ListEntry != &IMapping->Link;
       ListEntry = ListEntry->ForwardLink) {
    Mapping = BASE_CR(ListEntry, MAP_INFO, Link);
    if (Mapping->Signature != MAP_INFO_SIGNATURE) {
      // BUGBUG: NT32 will load image to BS memory, but execute code in DLL.
      // Do not use LoadedImage->ImageBase/ImageSize
      gMaps = ListEntry;
      break;
    }
  }
  return ;
}

VOID
DumpIoMmuMapList (
  VOID
  )
{
  LIST_ENTRY                *ListEntry;
  MAP_INFO                  *Mapping;
  LIST_ENTRY                *MapHandleListEntry;
  MAP_HANDLE_INFO           *MapHandleInfo;
  CHAR16                    *PathStr;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  EFI_STATUS                Status;
  EFI_PCI_IO_PROTOCOL       *PciIo;
  UINTN                     Segment;
  UINTN                     Bus;
  UINTN                     Device;
  UINTN                     Function;

  ListEntry = gMaps;
  for (ListEntry = ListEntry->BackLink;
       ListEntry != gMaps;
       ListEntry = ListEntry->BackLink) {
    Mapping = CR(ListEntry, MAP_INFO, Link, MAP_INFO_SIGNATURE);
    DEBUG ((DEBUG_INFO, "IOMMU Mapping - 0x%x\n", Mapping));
    Print (L"IOMMU Mapping - 0x%x\n", Mapping);
    DEBUG ((DEBUG_INFO, "  Operation       - 0x%08x\n", Mapping->Operation));
    Print (L"  Operation       - 0x%08x\n", Mapping->Operation);
    DEBUG ((DEBUG_INFO, "  NumberOfBytes   - 0x%08x\n", Mapping->NumberOfBytes));
    Print (L"  NumberOfBytes   - 0x%08x\n", Mapping->NumberOfBytes);
    DEBUG ((DEBUG_INFO, "  NumberOfPages   - 0x%08x\n", Mapping->NumberOfPages));
    Print (L"  NumberOfPages   - 0x%08x\n", Mapping->NumberOfPages);
    DEBUG ((DEBUG_INFO, "  HostAddress     - 0x%016lx\n", Mapping->HostAddress));
    Print (L"  HostAddress     - 0x%016lx\n", Mapping->HostAddress);
    DEBUG ((DEBUG_INFO, "  DeviceAddress   - 0x%016lx\n", Mapping->DeviceAddress));
    Print (L"  DeviceAddress   - 0x%016lx\n", Mapping->DeviceAddress);

    MapHandleListEntry = &Mapping->HandleList;
    for (MapHandleListEntry = MapHandleListEntry->BackLink;
         MapHandleListEntry != &Mapping->HandleList;
         MapHandleListEntry = MapHandleListEntry->BackLink) {
      MapHandleInfo = CR(MapHandleListEntry, MAP_HANDLE_INFO, Link, MAP_HANDLE_INFO_SIGNATURE);
      DEBUG ((DEBUG_INFO, "  IOMMU Map Handle Info - 0x%x\n", MapHandleInfo));
      Print (L"  IOMMU Map Handle Info - 0x%x\n", MapHandleInfo);
      DEBUG ((DEBUG_INFO, "    Handle        - 0x%08x", MapHandleInfo->Handle));
      Print (L"    Handle        - 0x%08x", MapHandleInfo->Handle);
      Status = gBS->HandleProtocol (MapHandleInfo->Handle, &gEfiPciIoProtocolGuid, (VOID **)&PciIo);
      if (!EFI_ERROR(Status)) {
        Status = PciIo->GetLocation (PciIo, &Segment, &Bus, &Device, &Function);
        if (!EFI_ERROR(Status)) {
          DEBUG ((DEBUG_INFO, " (S%04xB%02xD%02xF%02x)", Segment, Bus, Device, Function));
          Print (L" (S%04xB%02xD%02xF%02x)", Segment, Bus, Device, Function);
        } else {
          DEBUG ((DEBUG_INFO, " (Unknown PCI)"));
          Print (L" (Unknown PCI)");
        }
      } else {
        DEBUG ((DEBUG_INFO, " (Not PCI)"));
        Print (L" (Not PCI)");
      }
      Status = gBS->HandleProtocol (MapHandleInfo->Handle, &gEfiDevicePathProtocolGuid, (VOID **)&DevicePath);
      if (!EFI_ERROR(Status)) {
        PathStr = ConvertDevicePathToText(DevicePath, TRUE, TRUE);
        if (PathStr != NULL) {
          DEBUG ((DEBUG_INFO, " (%s)", PathStr));
          Print (L" (%s)", PathStr);
        }
      }
      DEBUG ((DEBUG_INFO, "\n"));
      Print (L"\n");
      DEBUG ((DEBUG_INFO, "    IoMmuAccess   - 0x%016lx\n", MapHandleInfo->IoMmuAccess));
      Print (L"    IoMmuAccess   - 0x%016lx\n", MapHandleInfo->IoMmuAccess);
    }
  }
}

VOID
DumpIoMmuMap (
  VOID
  )
{
  EDKII_IOMMU_PROTOCOL  *IoMmu;
  EFI_STATUS            Status;
  VOID                  *HostAddress;
  UINTN                 NumberOfBytes;
  EFI_PHYSICAL_ADDRESS  DeviceAddress;
  VOID                  *Mapping;

  Status = gBS->LocateProtocol (&gEdkiiIoMmuProtocolGuid, NULL, (VOID **)&IoMmu);
  if (EFI_ERROR(Status)) {
    Print (L"No IOMMU Protocol\n");
    return ;
  }

  Status = IoMmu->AllocateBuffer (IoMmu, 0, EfiBootServicesData, 1, &HostAddress, EDKII_IOMMU_ATTRIBUTE_DUAL_ADDRESS_CYCLE);
  if (EFI_ERROR(Status)) {
    Print (L"IoMmu->AllocateBuffer - %r\n", Status);
    return ;
  }
  NumberOfBytes = 1;
  Status = IoMmu->Map (IoMmu, EdkiiIoMmuOperationBusMasterCommonBuffer64, HostAddress, &NumberOfBytes, &DeviceAddress, &Mapping);
  if (EFI_ERROR(Status)) {
    Print (L"IoMmu->Map - %r\n", Status);
    IoMmu->FreeBuffer (IoMmu, 1, HostAddress);
    return ;
  }
  
  GetIoMmuMapList((MAP_INFO *)Mapping);

  IoMmu->Unmap (IoMmu, Mapping);
  IoMmu->FreeBuffer (IoMmu, 1, HostAddress);

  DumpIoMmuMapList ();

  return ;
}

BOOLEAN
ExistInIoMmuMap (
  IN UINTN                          Segment,
  IN UINTN                          Bus,
  IN UINTN                          Device,
  IN UINTN                          Function,
  IN UINT64                         Address,
  IN UINT64                         IoMmuAccess
  )
{
  LIST_ENTRY                *ListEntry;
  MAP_INFO                  *Mapping;
  LIST_ENTRY                *MapHandleListEntry;
  MAP_HANDLE_INFO           *MapHandleInfo;
  EFI_STATUS                Status;
  EFI_PCI_IO_PROTOCOL       *PciIo;
  UINTN                     MySegment;
  UINTN                     MyBus;
  UINTN                     MyDevice;
  UINTN                     MyFunction;

  ListEntry = gMaps;
  for (ListEntry = ListEntry->BackLink;
       ListEntry != gMaps;
       ListEntry = ListEntry->BackLink) {
    Mapping = CR(ListEntry, MAP_INFO, Link, MAP_INFO_SIGNATURE);

    if ((Address < Mapping->DeviceAddress) && (Address >= Mapping->DeviceAddress + EFI_PAGES_TO_SIZE(Mapping->NumberOfPages))) {
      continue;
    }

    MapHandleListEntry = &Mapping->HandleList;
    for (MapHandleListEntry = MapHandleListEntry->BackLink;
         MapHandleListEntry != &Mapping->HandleList;
         MapHandleListEntry = MapHandleListEntry->BackLink) {
      MapHandleInfo = CR(MapHandleListEntry, MAP_HANDLE_INFO, Link, MAP_HANDLE_INFO_SIGNATURE);

      if (MapHandleInfo->IoMmuAccess != IoMmuAccess) {
        continue;
      }

      Status = gBS->HandleProtocol (MapHandleInfo->Handle, &gEfiPciIoProtocolGuid, (VOID **)&PciIo);
      if (!EFI_ERROR(Status)) {
        Status = PciIo->GetLocation (PciIo, &MySegment, &MyBus, &MyDevice, &MyFunction);
        if (!EFI_ERROR(Status)) {
          if ((MySegment != Segment) || (MyBus != Bus) || (MyDevice != Device) || (MyFunction != Function)) {
            continue;
          }
          return TRUE;
        } else {
          return TRUE;
        }
      } else {
        return TRUE;
      }
    }
  }

  return FALSE;
}

VOID
AddExceptionDevice (
  IN EDKII_PLATFORM_VTD_PCI_DEVICE_ID   *PciDeviceId
  )
{
  UINTN                            Index;
  UINTN                            SubIndex;
  PCI_DEVICE_INFORMATION           *PciDeviceInfo;
  EXCEPTION_RECORD                 *ExceptionRecord;
  PCI_DEVICE_DATA                  *PciDeviceData;

  DEBUG ((DEBUG_INFO, "GetPciDeviceData - 0x%04x:0x%04x\n", PciDeviceId->VendorId, PciDeviceId->DeviceId));

  for (Index = 0; Index < mVtdUnitNumber; Index++) {
    PciDeviceInfo = &mVtdUnitInformation[Index].PciDeviceInfo;
    for (SubIndex = 0; SubIndex < PciDeviceInfo->PciDeviceDataNumber; SubIndex++) {
      DEBUG ((DEBUG_INFO, " Checking - 0x%04x:0x%04x\n", PciDeviceInfo->PciDeviceData[SubIndex].PciDeviceId.VendorId, PciDeviceInfo->PciDeviceData[SubIndex].PciDeviceId.DeviceId));
      if (((PciDeviceId->VendorId == 0xFFFF) || (PciDeviceId->VendorId == PciDeviceInfo->PciDeviceData[SubIndex].PciDeviceId.VendorId)) &&
          ((PciDeviceId->DeviceId == 0xFFFF) || (PciDeviceId->DeviceId == PciDeviceInfo->PciDeviceData[SubIndex].PciDeviceId.DeviceId)) &&
          ((PciDeviceId->RevisionId == 0xFF) || (PciDeviceId->RevisionId == PciDeviceInfo->PciDeviceData[SubIndex].PciDeviceId.RevisionId)) &&
          ((PciDeviceId->SubsystemVendorId == 0xFFFF) || (PciDeviceId->SubsystemVendorId == PciDeviceInfo->PciDeviceData[SubIndex].PciDeviceId.SubsystemVendorId)) &&
          ((PciDeviceId->SubsystemDeviceId == 0xFFFF) || (PciDeviceId->SubsystemDeviceId == PciDeviceInfo->PciDeviceData[SubIndex].PciDeviceId.SubsystemDeviceId)) ) {
        DEBUG ((DEBUG_INFO, "Found\n"));
        PciDeviceData = &PciDeviceInfo->PciDeviceData[SubIndex];

        ExceptionRecord = AllocatePool (sizeof (EXCEPTION_RECORD));
        if (ExceptionRecord != NULL) {
          ExceptionRecord->Signature = EXCEPTION_RECORD_SIGNATURE;
          ExceptionRecord->Segment   = mVtdUnitInformation[Index].Segment;
          ExceptionRecord->Bus       = PciDeviceData->PciSourceId.Bits.Bus;
          ExceptionRecord->Device    = PciDeviceData->PciSourceId.Bits.Device;
          ExceptionRecord->Function  = PciDeviceData->PciSourceId.Bits.Function;
          InsertTailList (&gExceptionRecord, &ExceptionRecord->Link);
        }

      }
    }
  }
  return ;
}


VOID
DumpExceptionRecord (
  VOID
  )
{
  EDKII_PLATFORM_VTD_POLICY_PROTOCOL        *PlatformVtd;
  EFI_STATUS                                Status;
  UINTN                                     DeviceInfoCount;
  VOID                                      *DeviceInfo;
  EDKII_PLATFORM_VTD_EXCEPTION_DEVICE_INFO  *ThisDeviceInfo;
  EDKII_PLATFORM_VTD_DEVICE_SCOPE           *DeviceScope;
  EDKII_PLATFORM_VTD_PCI_DEVICE_ID          *PciDeviceId;
  EFI_ACPI_DMAR_PCI_PATH                    *PciPath;
  UINTN                                     Index;
  EXCEPTION_RECORD                          *ExceptionRecord;
  LIST_ENTRY                                *ListEntry;

  Status = gBS->LocateProtocol (&gEdkiiPlatformVTdPolicyProtocolGuid, NULL, (VOID **)&PlatformVtd);
  if (EFI_ERROR(Status)) {
    Print (L"No PlatformVtd Protocol\n");
    return ;
  }

  Status = PlatformVtd->GetExceptionDeviceList (PlatformVtd, &DeviceInfoCount, &DeviceInfo);
  if (EFI_ERROR(Status)) {
    Print (L"No ExceptionDeviceList from PlatformVtd\n");
    return ;
  }

  DEBUG ((DEBUG_INFO, "ExceptionDeviceList: (0x%x)\n", DeviceInfo));
  Print (L"ExceptionDeviceList: (0x%x)\n", DeviceInfo);
  ThisDeviceInfo = DeviceInfo;
  for (Index = 0; Index < DeviceInfoCount; Index++) {
    if (ThisDeviceInfo->Type == EDKII_PLATFORM_VTD_EXCEPTION_DEVICE_INFO_TYPE_END) {
      break;
    }
    switch (ThisDeviceInfo->Type) {
    case EDKII_PLATFORM_VTD_EXCEPTION_DEVICE_INFO_TYPE_DEVICE_SCOPE:
      DeviceScope = (VOID *)(ThisDeviceInfo + 1);
      DEBUG ((DEBUG_INFO, "DeviceScope(%d):\n", Index, DeviceScope));
      Print (L"DeviceScope(%d):\n", Index, DeviceScope);
      DEBUG ((DEBUG_INFO, "  SegmentNumber - 0x%04x\n", DeviceScope->SegmentNumber));
      Print (L"  SegmentNumber - 0x%04x\n", DeviceScope->SegmentNumber);
      DEBUG ((DEBUG_INFO, "  StartBusNumber - 0x%04x\n", DeviceScope->DeviceScope.StartBusNumber));
      Print (L"  StartBusNumber - 0x%04x\n", DeviceScope->DeviceScope.StartBusNumber);
      PciPath = (VOID *)(DeviceScope + 1);
      DEBUG ((DEBUG_INFO, "  Device   - 0x%04x\n", PciPath->Device));
      Print (L"  Device   - 0x%04x\n", PciPath->Device);
      DEBUG ((DEBUG_INFO, "  Function - 0x%04x\n", PciPath->Function));
      Print (L"  Function - 0x%04x\n", PciPath->Function);

        ExceptionRecord = AllocatePool (sizeof (EXCEPTION_RECORD));
        if (ExceptionRecord != NULL) {
          ExceptionRecord->Signature = EXCEPTION_RECORD_SIGNATURE;
          ExceptionRecord->Segment   = DeviceScope->SegmentNumber;
          ExceptionRecord->Bus       = DeviceScope->DeviceScope.StartBusNumber;
          ExceptionRecord->Device    = PciPath->Device;
          ExceptionRecord->Function  = PciPath->Function;
          InsertTailList (&gExceptionRecord, &ExceptionRecord->Link);
        }

      break;

    case EDKII_PLATFORM_VTD_EXCEPTION_DEVICE_INFO_TYPE_PCI_DEVICE_ID:
      PciDeviceId = (VOID *)(ThisDeviceInfo + 1);
      DEBUG ((DEBUG_INFO, "PciDeviceId(%d):\n", Index, PciDeviceId));
      Print (L"PciDeviceId(%d):\n", Index, PciDeviceId);
      DEBUG ((DEBUG_INFO, "  VendorId          - 0x%04x\n", PciDeviceId->VendorId));
      Print (L"  VendorId          - 0x%04x\n", PciDeviceId->VendorId);
      DEBUG ((DEBUG_INFO, "  DeviceId          - 0x%04x\n", PciDeviceId->DeviceId));
      Print (L"  DeviceId          - 0x%04x\n", PciDeviceId->DeviceId);
      DEBUG ((DEBUG_INFO, "  RevisionId        - 0x%02x\n", PciDeviceId->RevisionId));
      Print (L"  RevisionId        - 0x%02x\n", PciDeviceId->RevisionId);
      DEBUG ((DEBUG_INFO, "  SubsystemVendorId - 0x%04x\n", PciDeviceId->SubsystemVendorId));
      Print (L"  SubsystemVendorId - 0x%04x\n", PciDeviceId->SubsystemVendorId);
      DEBUG ((DEBUG_INFO, "  SubsystemDeviceId - 0x%04x\n", PciDeviceId->SubsystemDeviceId));
      Print (L"  SubsystemDeviceId - 0x%04x\n", PciDeviceId->SubsystemDeviceId);

      AddExceptionDevice (PciDeviceId);

      break;

    default:
      break;
    }
    ThisDeviceInfo = (VOID *)((UINTN)ThisDeviceInfo + ThisDeviceInfo->Length);
  }
  FreePool (DeviceInfo);

  //
  // Dump
  //
  DEBUG ((DEBUG_INFO, "ExceptionDevice:\n"));
  Print (L"ExceptionDevice:\n");
  ListEntry = &gExceptionRecord;
  for (ListEntry = ListEntry->BackLink;
       ListEntry != &gExceptionRecord;
       ListEntry = ListEntry->BackLink) {
    ExceptionRecord = CR(ListEntry, EXCEPTION_RECORD, Link, EXCEPTION_RECORD_SIGNATURE);
    DEBUG ((DEBUG_INFO, "ExceptionRecord\n"));
    Print (L"ExceptionRecord\n");
    DEBUG ((DEBUG_INFO, "  Segment   - 0x%x\n", ExceptionRecord->Segment));
    Print (L"  Segment   - 0x%x\n", ExceptionRecord->Segment);
    DEBUG ((DEBUG_INFO, "  Bus       - 0x%x\n", ExceptionRecord->Bus));
    Print (L"  Bus       - 0x%x\n", ExceptionRecord->Bus);
    DEBUG ((DEBUG_INFO, "  Device    - 0x%x\n", ExceptionRecord->Device));
    Print (L"  Device    - 0x%x\n", ExceptionRecord->Device);
    DEBUG ((DEBUG_INFO, "  Function  - 0x%x\n", ExceptionRecord->Function));
    Print (L"  Function  - 0x%x\n", ExceptionRecord->Function);
  }
}

BOOLEAN
ExistInExceptionList (
  IN UINTN                          Segment,
  IN UINTN                          Bus,
  IN UINTN                          Device,
  IN UINTN                          Function
  )
{
  LIST_ENTRY                *ListEntry;
  EXCEPTION_RECORD          *ExceptionRecord;

  ListEntry = &gExceptionRecord;
  for (ListEntry = ListEntry->BackLink;
       ListEntry != &gExceptionRecord;
       ListEntry = ListEntry->BackLink) {
    ExceptionRecord = CR(ListEntry, EXCEPTION_RECORD, Link, EXCEPTION_RECORD_SIGNATURE);

    if ((ExceptionRecord->Segment == Segment) && (ExceptionRecord->Bus == Bus) && (ExceptionRecord->Device == Device) && (ExceptionRecord->Function == Function)) {
      return TRUE;
    }
  }

  return FALSE;
}

VOID
DumpRmrrRecord (
  VOID
  )
{
  LIST_ENTRY                *ListEntry;
  RMRR_RECORD               *RmrrRecord;

  ListEntry = &gRmrrRecord;
  for (ListEntry = ListEntry->BackLink;
       ListEntry != &gRmrrRecord;
       ListEntry = ListEntry->BackLink) {
    RmrrRecord = CR(ListEntry, RMRR_RECORD, Link, RMRR_RECORD_SIGNATURE);
    DEBUG ((DEBUG_INFO, "RmrrRecord\n"));
    Print (L"RmrrRecord\n");
    DEBUG ((DEBUG_INFO, "  Segment  - 0x%x\n", RmrrRecord->Segment));
    Print (L"  Segment  - 0x%x\n", RmrrRecord->Segment);
    DEBUG ((DEBUG_INFO, "  Bus      - 0x%x\n", RmrrRecord->Bus));
    Print (L"  Bus      - 0x%x\n", RmrrRecord->Bus);
    DEBUG ((DEBUG_INFO, "  Device   - 0x%x\n", RmrrRecord->Device));
    Print (L"  Device   - 0x%x\n", RmrrRecord->Device);
    DEBUG ((DEBUG_INFO, "  Function - 0x%x\n", RmrrRecord->Function));
    Print (L"  Function - 0x%x\n", RmrrRecord->Function);
    DEBUG ((DEBUG_INFO, "  Address  - 0x%lx\n", RmrrRecord->Address));
    Print (L"  Address  - 0x%lx\n", RmrrRecord->Address);
    DEBUG ((DEBUG_INFO, "  Length   - 0x%lx\n", RmrrRecord->Length));
    Print (L"  Length   - 0x%lx\n", RmrrRecord->Length);
  }
}

BOOLEAN
ExistInRmrr (
  IN UINTN                          Segment,
  IN UINTN                          Bus,
  IN UINTN                          Device,
  IN UINTN                          Function,
  IN UINT64                         Address,
  IN UINT64                         IoMmuAccess
  )
{
  LIST_ENTRY                *ListEntry;
  RMRR_RECORD               *RmrrRecord;

  ListEntry = &gRmrrRecord;
  for (ListEntry = ListEntry->BackLink;
       ListEntry != &gRmrrRecord;
       ListEntry = ListEntry->BackLink) {
    RmrrRecord = CR(ListEntry, RMRR_RECORD, Link, RMRR_RECORD_SIGNATURE);

    if ((Address < RmrrRecord->Address) && (Address >= RmrrRecord->Address + RmrrRecord->Length)) {
      continue;
    }

    if ((RmrrRecord->Segment == Segment) && (RmrrRecord->Bus == Bus) && (RmrrRecord->Device == Device) && (RmrrRecord->Function == Function)) {
      return TRUE;
    }
  }

  return FALSE;
}

VOID
CheckPtEntry (
  IN UINTN                          Segment,
  IN UINTN                          Bus,
  IN UINTN                          Device,
  IN UINTN                          Function,
  IN VTD_SECOND_LEVEL_PAGING_ENTRY  *PtEntry,
  IN UINT64                         Size
  )
{
  UINT64   Address;
  BOOLEAN  Exist;
  UINT64   IoMmuAccess;
  UINTN    Index;

  if (PtEntry->Bits.Read == 0 && PtEntry->Bits.Write == 0) {
    return ;
  }
  IoMmuAccess = 0;
  if (PtEntry->Bits.Read != 0) {
    IoMmuAccess |= EDKII_IOMMU_ACCESS_READ;
  }
  if (PtEntry->Bits.Write != 0) {
    IoMmuAccess |= EDKII_IOMMU_ACCESS_WRITE;
  }

  Address = VTD_64BITS_ADDRESS(PtEntry->Bits.AddressLo, PtEntry->Bits.AddressHi);

  Exist = FALSE;
  switch (Size) {
  case SIZE_4KB:
    Exist = ExistInExceptionList (Segment, Bus, Device, Function);
    if (Exist) {
      return ;
    }
    Exist = ExistInRmrr (Segment, Bus, Device, Function, Address, IoMmuAccess);
    if (Exist) {
      return ;
    }

    DEBUG ((DEBUG_INFO, "CheckPtEntry - 0x%lx(0x%lx) S%04xB%02xD%02xF%02x\n", Address, Size, Segment, Bus, Device, Function));
    Exist = ExistInIoMmuMap (Segment, Bus, Device, Function, Address, IoMmuAccess);
    if (!Exist) {
      DEBUG ((DEBUG_INFO, "-- ERROR: Address (0x%lx) is not in Map entry\n", Address));
      Print (L"-- ERROR: Address (0x%lx) is not in Map entry\n", Address);
      mHasError = TRUE;
    } else {
      //Print (L"-- PASS : Address (0x%lx) is in Map entry\n", Address);
    }
    break;
  case SIZE_2MB:
    for (Index = 0; Index < SIZE_2MB/SIZE_4KB; Index++) {
      Exist = ExistInExceptionList (Segment, Bus, Device, Function);
      if (Exist) {
        return ;
      }
      Exist = ExistInRmrr (Segment, Bus, Device, Function, Address + Index * SIZE_4KB, IoMmuAccess);
      if (Exist) {
        return ;
      }

    DEBUG ((DEBUG_INFO, "CheckPtEntry - 0x%lx(0x%lx) S%04xB%02xD%02xF%02x\n", Address, Size, Segment, Bus, Device, Function));
    Exist = ExistInIoMmuMap (Segment, Bus, Device, Function, Address + Index * SIZE_4KB, IoMmuAccess);
      if (!Exist) {
        DEBUG ((DEBUG_INFO, "-- ERROR: Address (0x%lx) is not in Map entry\n", Address + Index * SIZE_4KB));
        Print (L"-- ERROR: Address (0x%lx) is not in Map entry\n", Address + Index * SIZE_4KB);
      } else {
        //Print (L"-- PASS : Address (0x%lx) is in Map entry\n", Address + Index * SIZE_4KB);
      }
    }
    break;
  default:
    Print (L"-- ERROR: Invalid configuration (0x%lx-0x%lx)\n", Address, Size);
    ASSERT(FALSE);
    return ;
  }

}


/**
  Dump DMAR second level paging entry.

  @param[in]  SecondLevelPagingEntry The second level paging entry.
**/
VOID
DumpSecondLevelPagingEntry (
  IN UINTN   Segment,
  IN UINTN   Bus,
  IN UINTN   Device,
  IN UINTN   Function,
  IN VOID    *SecondLevelPagingEntry
  )
{
  UINTN                          Index4;
  UINTN                          Index3;
  UINTN                          Index2;
  UINTN                          Index1;
  VTD_SECOND_LEVEL_PAGING_ENTRY  *Lvl4PtEntry;
  VTD_SECOND_LEVEL_PAGING_ENTRY  *Lvl3PtEntry;
  VTD_SECOND_LEVEL_PAGING_ENTRY  *Lvl2PtEntry;
  VTD_SECOND_LEVEL_PAGING_ENTRY  *Lvl1PtEntry;

  DEBUG ((DEBUG_INFO, "================\n"));
  Print (L"================\n");
  DEBUG ((DEBUG_INFO, "DMAR Second Level Page Table:\n"));
  Print (L"DMAR Second Level Page Table:\n");

  DEBUG ((DEBUG_INFO, "SecondLevelPagingEntry Base - 0x%x\n", SecondLevelPagingEntry));
  Print (L"SecondLevelPagingEntry Base - 0x%x\n", SecondLevelPagingEntry);
  Lvl4PtEntry = (VTD_SECOND_LEVEL_PAGING_ENTRY *)SecondLevelPagingEntry;
  for (Index4 = 0; Index4 < SIZE_4KB/sizeof(VTD_SECOND_LEVEL_PAGING_ENTRY); Index4++) {
    if (Lvl4PtEntry[Index4].Uint64 != 0) {
      DEBUG ((DEBUG_INFO, "  Lvl4Pt Entry(0x%03x) - 0x%016lx\n", Index4, Lvl4PtEntry[Index4].Uint64));
      Print (L"  Lvl4Pt Entry(0x%03x) - 0x%016lx\n", Index4, Lvl4PtEntry[Index4].Uint64);
    }
    if (Lvl4PtEntry[Index4].Uint64 == 0) {
      continue;
    }
    Lvl3PtEntry = (VTD_SECOND_LEVEL_PAGING_ENTRY *)(UINTN)VTD_64BITS_ADDRESS(Lvl4PtEntry[Index4].Bits.AddressLo, Lvl4PtEntry[Index4].Bits.AddressHi);
    for (Index3 = 0; Index3 < SIZE_4KB/sizeof(VTD_SECOND_LEVEL_PAGING_ENTRY); Index3++) {
      if (Lvl3PtEntry[Index3].Uint64 != 0) {
        DEBUG ((DEBUG_INFO, "    Lvl3Pt Entry(0x%03x) - 0x%016lx\n", Index3, Lvl3PtEntry[Index3].Uint64));
        Print (L"    Lvl3Pt Entry(0x%03x) - 0x%016lx\n", Index3, Lvl3PtEntry[Index3].Uint64);
      }
      if (Lvl3PtEntry[Index3].Uint64 == 0) {
        continue;
      }

      Lvl2PtEntry = (VTD_SECOND_LEVEL_PAGING_ENTRY *)(UINTN)VTD_64BITS_ADDRESS(Lvl3PtEntry[Index3].Bits.AddressLo, Lvl3PtEntry[Index3].Bits.AddressHi);
      for (Index2 = 0; Index2 < SIZE_4KB/sizeof(VTD_SECOND_LEVEL_PAGING_ENTRY); Index2++) {
        if (Lvl2PtEntry[Index2].Uint64 != 0) {
          DEBUG ((DEBUG_INFO, "      Lvl2Pt Entry(0x%03x) - 0x%016lx\n", Index2, Lvl2PtEntry[Index2].Uint64));
          Print (L"      Lvl2Pt Entry(0x%03x) - 0x%016lx\n", Index2, Lvl2PtEntry[Index2].Uint64);
        }
        if (Lvl2PtEntry[Index2].Uint64 == 0) {
          continue;
        }
        if (Lvl2PtEntry[Index2].Bits.PageSize == 0) {
          Lvl1PtEntry = (VTD_SECOND_LEVEL_PAGING_ENTRY *)(UINTN)VTD_64BITS_ADDRESS(Lvl2PtEntry[Index2].Bits.AddressLo, Lvl2PtEntry[Index2].Bits.AddressHi);
          for (Index1 = 0; Index1 < SIZE_4KB/sizeof(VTD_SECOND_LEVEL_PAGING_ENTRY); Index1++) {
            if (Lvl1PtEntry[Index1].Uint64 != 0) {
              DEBUG ((DEBUG_INFO, "        Lvl1Pt Entry(0x%03x) - 0x%016lx\n", Index1, Lvl1PtEntry[Index1].Uint64));
              Print (L"        Lvl1Pt Entry(0x%03x) - 0x%016lx\n", Index1, Lvl1PtEntry[Index1].Uint64);
              CheckPtEntry (Segment, Bus, Device, Function, &Lvl1PtEntry[Index1], SIZE_4KB);
            }
          }
        } else {
          CheckPtEntry (Segment, Bus, Device, Function, &Lvl2PtEntry[Index2], SIZE_2MB);
        }
      }
    }
  }
  DEBUG ((DEBUG_INFO, "================\n"));
  Print (L"================\n");
}

/**
  Dump DMAR context entry table.

  @param[in]  RootEntry DMAR root entry.
**/
VOID
DumpDmarContextEntryTable (
  IN UINTN          Segment,
  IN VTD_ROOT_ENTRY *RootEntry
  )
{
  UINTN                 Index;
  UINTN                 Index2;
  VTD_CONTEXT_ENTRY     *ContextEntry;

  DEBUG ((DEBUG_INFO, "=========================\n"));
  Print (L"=========================\n");
  DEBUG ((DEBUG_INFO, "DMAR Context Entry Table:\n"));
  Print (L"DMAR Context Entry Table:\n");

  DEBUG ((DEBUG_INFO, "RootEntry Address - 0x%x\n", RootEntry));
  Print (L"RootEntry Address - 0x%x\n", RootEntry);

  for (Index = 0; Index < VTD_ROOT_ENTRY_NUMBER; Index++) {
    if ((RootEntry[Index].Uint128.Uint64Lo != 0) || (RootEntry[Index].Uint128.Uint64Hi != 0)) {
      DEBUG ((DEBUG_INFO, "  RootEntry(0x%02x) B%02x - 0x%016lx %016lx\n",
        Index, Index, RootEntry[Index].Uint128.Uint64Hi, RootEntry[Index].Uint128.Uint64Lo));
      Print (L"  RootEntry(0x%02x) B%02x - 0x%016lx %016lx\n",
        Index, Index, RootEntry[Index].Uint128.Uint64Hi, RootEntry[Index].Uint128.Uint64Lo);
    }
    if (RootEntry[Index].Bits.Present == 0) {
      continue;
    }
    ContextEntry = (VTD_CONTEXT_ENTRY *)(UINTN)VTD_64BITS_ADDRESS(RootEntry[Index].Bits.ContextTablePointerLo, RootEntry[Index].Bits.ContextTablePointerHi);
    for (Index2 = 0; Index2 < VTD_CONTEXT_ENTRY_NUMBER; Index2++) {
      if ((ContextEntry[Index2].Uint128.Uint64Lo != 0) || (ContextEntry[Index2].Uint128.Uint64Hi != 0)) {
        DEBUG ((DEBUG_INFO, "    ContextEntry(0x%02x) D%02xF%02x - 0x%016lx %016lx\n",
          Index2, Index2 >> 3, Index2 & 0x7, ContextEntry[Index2].Uint128.Uint64Hi, ContextEntry[Index2].Uint128.Uint64Lo));
        Print (L"    ContextEntry(0x%02x) D%02xF%02x - 0x%016lx %016lx\n",
          Index2, Index2 >> 3, Index2 & 0x7, ContextEntry[Index2].Uint128.Uint64Hi, ContextEntry[Index2].Uint128.Uint64Lo);
      }
      if (ContextEntry[Index2].Bits.Present == 0) {
        continue;
      }
      DumpSecondLevelPagingEntry (Segment, Index, Index2 >> 3, Index2 & 0x7,
        (VOID *)(UINTN)VTD_64BITS_ADDRESS(ContextEntry[Index2].Bits.SecondLevelPageTranslationPointerLo, ContextEntry[Index2].Bits.SecondLevelPageTranslationPointerHi));
    }
  }
  DEBUG ((DEBUG_INFO, "=========================\n"));
  Print (L"=========================\n");
}

/**
  Dump DMAR extended context entry table.

  @param[in]  ExtRootEntry DMAR extended root entry.
**/
VOID
DumpDmarExtContextEntryTable (
  IN UINTN              Segment,
  IN VTD_EXT_ROOT_ENTRY *ExtRootEntry
  )
{
  UINTN                 Index;
  UINTN                 Index2;
  VTD_EXT_CONTEXT_ENTRY *ExtContextEntry;

  DEBUG ((DEBUG_INFO, "=========================\n"));
  Print (L"=========================\n");
  DEBUG ((DEBUG_INFO, "DMAR ExtContext Entry Table:\n"));
  Print (L"DMAR ExtContext Entry Table:\n");

  DEBUG ((DEBUG_INFO, "ExtRootEntry Address - 0x%x\n", ExtRootEntry));
  Print (L"ExtRootEntry Address - 0x%x\n", ExtRootEntry);

  for (Index = 0; Index < VTD_ROOT_ENTRY_NUMBER; Index++) {
    if ((ExtRootEntry[Index].Uint128.Uint64Lo != 0) || (ExtRootEntry[Index].Uint128.Uint64Hi != 0)) {
      DEBUG ((DEBUG_INFO, "  ExtRootEntry(0x%02x) B%02x - 0x%016lx %016lx\n",
        Index, Index, ExtRootEntry[Index].Uint128.Uint64Hi, ExtRootEntry[Index].Uint128.Uint64Lo));
      Print (L"  ExtRootEntry(0x%02x) B%02x - 0x%016lx %016lx\n",
        Index, Index, ExtRootEntry[Index].Uint128.Uint64Hi, ExtRootEntry[Index].Uint128.Uint64Lo);
    }
    if (ExtRootEntry[Index].Bits.LowerPresent == 0) {
      continue;
    }
    ExtContextEntry = (VTD_EXT_CONTEXT_ENTRY *)(UINTN)VTD_64BITS_ADDRESS(ExtRootEntry[Index].Bits.LowerContextTablePointerLo, ExtRootEntry[Index].Bits.LowerContextTablePointerHi);
    for (Index2 = 0; Index2 < VTD_CONTEXT_ENTRY_NUMBER/2; Index2++) {
      if ((ExtContextEntry[Index2].Uint256.Uint64_1 != 0) || (ExtContextEntry[Index2].Uint256.Uint64_2 != 0) ||
          (ExtContextEntry[Index2].Uint256.Uint64_3 != 0) || (ExtContextEntry[Index2].Uint256.Uint64_4 != 0)) {
        DEBUG ((DEBUG_INFO, "    ExtContextEntryLower(0x%02x) D%02xF%02x - 0x%016lx %016lx %016lx %016lx\n",
          Index2, Index2 >> 3, Index2 & 0x7, ExtContextEntry[Index2].Uint256.Uint64_4, ExtContextEntry[Index2].Uint256.Uint64_3, ExtContextEntry[Index2].Uint256.Uint64_2, ExtContextEntry[Index2].Uint256.Uint64_1));
        Print (L"    ExtContextEntryLower(0x%02x) D%02xF%02x - 0x%016lx %016lx %016lx %016lx\n",
          Index2, Index2 >> 3, Index2 & 0x7, ExtContextEntry[Index2].Uint256.Uint64_4, ExtContextEntry[Index2].Uint256.Uint64_3, ExtContextEntry[Index2].Uint256.Uint64_2, ExtContextEntry[Index2].Uint256.Uint64_1);
      }
      if (ExtContextEntry[Index2].Bits.Present == 0) {
        continue;
      }
      DumpSecondLevelPagingEntry (Segment, Index, Index2 >> 3, Index2 & 0x7,
        (VOID *)(UINTN)VTD_64BITS_ADDRESS(ExtContextEntry[Index2].Bits.SecondLevelPageTranslationPointerLo, ExtContextEntry[Index2].Bits.SecondLevelPageTranslationPointerHi));
    }

    if (ExtRootEntry[Index].Bits.UpperPresent == 0) {
      continue;
    }
    ExtContextEntry = (VTD_EXT_CONTEXT_ENTRY *)(UINTN)VTD_64BITS_ADDRESS(ExtRootEntry[Index].Bits.UpperContextTablePointerLo, ExtRootEntry[Index].Bits.UpperContextTablePointerHi);
    for (Index2 = 0; Index2 < VTD_CONTEXT_ENTRY_NUMBER/2; Index2++) {
      if ((ExtContextEntry[Index2].Uint256.Uint64_1 != 0) || (ExtContextEntry[Index2].Uint256.Uint64_2 != 0) ||
          (ExtContextEntry[Index2].Uint256.Uint64_3 != 0) || (ExtContextEntry[Index2].Uint256.Uint64_4 != 0)) {
        DEBUG ((DEBUG_INFO, "    ExtContextEntryUpper(0x%02x) D%02xF%02x - 0x%016lx %016lx %016lx %016lx\n",
          Index2, (Index2 + 128) >> 3, (Index2 + 128) & 0x7, ExtContextEntry[Index2].Uint256.Uint64_4, ExtContextEntry[Index2].Uint256.Uint64_3, ExtContextEntry[Index2].Uint256.Uint64_2, ExtContextEntry[Index2].Uint256.Uint64_1));
        Print (L"    ExtContextEntryUpper(0x%02x) D%02xF%02x - 0x%016lx %016lx %016lx %016lx\n",
          Index2, (Index2 + 128) >> 3, (Index2 + 128) & 0x7, ExtContextEntry[Index2].Uint256.Uint64_4, ExtContextEntry[Index2].Uint256.Uint64_3, ExtContextEntry[Index2].Uint256.Uint64_2, ExtContextEntry[Index2].Uint256.Uint64_1);
      }
      if (ExtContextEntry[Index2].Bits.Present == 0) {
        continue;
      }
    }
  }
  DEBUG ((DEBUG_INFO, "=========================\n"));
  Print (L"=========================\n");
}


/**
  Dump DMAR DeviceScopeEntry.

  @param[in]  DmarDeviceScopeEntry  DMAR DeviceScopeEntry
**/
VOID
DumpDmarDeviceScopeEntry (
  IN EFI_ACPI_DMAR_DEVICE_SCOPE_STRUCTURE_HEADER     *DmarDeviceScopeEntry
  )
{
  UINTN   PciPathNumber;
  UINTN   PciPathIndex;
  EFI_ACPI_DMAR_PCI_PATH  *PciPath;

  if (DmarDeviceScopeEntry == NULL) {
    return;
  }

  DEBUG ((DEBUG_INFO,
    "    *************************************************************************\n"
    ));
  Print (L"    *************************************************************************\n");
  DEBUG ((DEBUG_INFO,
    "    *       DMA-Remapping Device Scope Entry Structure                      *\n"
    ));
  Print (L"    *       DMA-Remapping Device Scope Entry Structure                      *\n");
  DEBUG ((DEBUG_INFO,
    "    *************************************************************************\n"
    ));
  Print (L"    *************************************************************************\n");
  DEBUG ((DEBUG_INFO,
    (sizeof(UINTN) == sizeof(UINT64)) ?
    "    DMAR Device Scope Entry address ...................... 0x%016lx\n" :
    "    DMAR Device Scope Entry address ...................... 0x%08x\n",
    DmarDeviceScopeEntry
    ));
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"    DMAR Device Scope Entry address ...................... 0x%016lx\n" :
    L"    DMAR Device Scope Entry address ...................... 0x%08x\n",
    DmarDeviceScopeEntry
    );
  DEBUG ((DEBUG_INFO,
    "      Device Scope Entry Type ............................ 0x%02x\n",
    DmarDeviceScopeEntry->Type
    ));
  Print (
    L"      Device Scope Entry Type ............................ 0x%02x\n",
    DmarDeviceScopeEntry->Type
    );
  switch (DmarDeviceScopeEntry->Type) {
  case EFI_ACPI_DEVICE_SCOPE_ENTRY_TYPE_PCI_ENDPOINT:
    DEBUG ((DEBUG_INFO,
      "        PCI Endpoint Device\n"
      ));
    Print (
      L"        PCI Endpoint Device\n"
      );
    break;
  case EFI_ACPI_DEVICE_SCOPE_ENTRY_TYPE_PCI_BRIDGE:
    DEBUG ((DEBUG_INFO,
      "        PCI Sub-hierachy\n"
      ));
    Print (
      L"        PCI Sub-hierachy\n"
      );
    break;
  case EFI_ACPI_DEVICE_SCOPE_ENTRY_TYPE_IOAPIC:
    DEBUG ((DEBUG_INFO,
      "        IOAPIC\n"
      ));
    Print (
      L"        IOAPIC\n"
      );
    break;
  case EFI_ACPI_DEVICE_SCOPE_ENTRY_TYPE_MSI_CAPABLE_HPET:
    DEBUG ((DEBUG_INFO,
      "        MSI Capable HPET\n"
      ));
    Print (
      L"        MSI Capable HPET\n"
      );
    break;
  case EFI_ACPI_DEVICE_SCOPE_ENTRY_TYPE_ACPI_NAMESPACE_DEVICE:
    DEBUG ((DEBUG_INFO,
      "        ACPI Namespace Device\n"
      ));
    Print (
      L"        ACPI Namespace Device\n"
      );
    break;
  default:
    break;
  }
  DEBUG ((DEBUG_INFO,
    "      Length ............................................. 0x%02x\n",
    DmarDeviceScopeEntry->Length
    ));
  Print (
    L"      Length ............................................. 0x%02x\n",
    DmarDeviceScopeEntry->Length
    );
  DEBUG ((DEBUG_INFO,
    "      Enumeration ID ..................................... 0x%02x\n",
    DmarDeviceScopeEntry->EnumerationId
    ));
  Print (
    L"      Enumeration ID ..................................... 0x%02x\n",
    DmarDeviceScopeEntry->EnumerationId
    );
  DEBUG ((DEBUG_INFO,
    "      Starting Bus Number ................................ 0x%02x\n",
    DmarDeviceScopeEntry->StartBusNumber
    ));
  Print (
    L"      Starting Bus Number ................................ 0x%02x\n",
    DmarDeviceScopeEntry->StartBusNumber
    );

  PciPathNumber = (DmarDeviceScopeEntry->Length - sizeof(EFI_ACPI_DMAR_DEVICE_SCOPE_STRUCTURE_HEADER)) / sizeof(EFI_ACPI_DMAR_PCI_PATH);
  PciPath = (EFI_ACPI_DMAR_PCI_PATH *)(DmarDeviceScopeEntry + 1);
  for (PciPathIndex = 0; PciPathIndex < PciPathNumber; PciPathIndex++) {
    DEBUG ((DEBUG_INFO,
      "      Device ............................................. 0x%02x\n",
      PciPath[PciPathIndex].Device
      ));
    Print (
      L"      Device ............................................. 0x%02x\n",
      PciPath[PciPathIndex].Device
      );
    DEBUG ((DEBUG_INFO,
      "      Function ........................................... 0x%02x\n",
      PciPath[PciPathIndex].Function
      ));
    Print (
      L"      Function ........................................... 0x%02x\n",
      PciPath[PciPathIndex].Function
      );
  }

  DEBUG ((DEBUG_INFO,
    "    *************************************************************************\n\n"
    ));
  Print (L"    *************************************************************************\n\n");

  return;
}

/**
  Dump DMAR ANDD table.

  @param[in]  Andd  DMAR ANDD table
**/
VOID
DumpDmarAndd (
  IN EFI_ACPI_DMAR_ANDD_HEADER *Andd
  )
{
  if (Andd == NULL) {
    return;
  }

  DEBUG ((DEBUG_INFO,
    "  ***************************************************************************\n"
    ));
  Print (L"  ***************************************************************************\n");
  DEBUG ((DEBUG_INFO,
    "  *       ACPI Name-space Device Declaration Structure                      *\n"
    ));
  Print (L"  *       ACPI Name-space Device Declaration Structure                      *\n");
  DEBUG ((DEBUG_INFO,
    "  ***************************************************************************\n"
    ));
  Print (L"  ***************************************************************************\n");
  DEBUG ((DEBUG_INFO,
    (sizeof(UINTN) == sizeof(UINT64)) ?
    "  ANDD address ........................................... 0x%016lx\n" :
    "  ANDD address ........................................... 0x%08x\n",
    Andd
    ));
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  ANDD address ........................................... 0x%016lx\n" :
    L"  ANDD address ........................................... 0x%08x\n",
    Andd
    );
  DEBUG ((DEBUG_INFO,
    "    Type ................................................. 0x%04x\n",
    Andd->Header.Type
    ));
  Print (
    L"    Type ................................................. 0x%04x\n",
    Andd->Header.Type
    );
  DEBUG ((DEBUG_INFO,
    "    Length ............................................... 0x%04x\n",
    Andd->Header.Length
    ));
  Print (
    L"    Length ............................................... 0x%04x\n",
    Andd->Header.Length
    );
  DEBUG ((DEBUG_INFO,
    "    ACPI Device Number ................................... 0x%02x\n",
    Andd->AcpiDeviceNumber
    ));
  Print (
    L"    ACPI Device Number ................................... 0x%02x\n",
    Andd->AcpiDeviceNumber
    );
  DEBUG ((DEBUG_INFO,
    "    ACPI Object Name ..................................... '%a'\n",
    (Andd + 1)
    ));
  Print (
    L"    ACPI Object Name ..................................... '%a'\n",
    (Andd + 1)
    );

  DEBUG ((DEBUG_INFO,
    "  ***************************************************************************\n\n"
    ));
  Print (L"  ***************************************************************************\n\n");

  return;
}

/**
  Dump DMAR RHSA table.

  @param[in]  Rhsa  DMAR RHSA table
**/
VOID
DumpDmarRhsa (
  IN EFI_ACPI_DMAR_RHSA_HEADER *Rhsa
  )
{
  if (Rhsa == NULL) {
    return;
  }

  DEBUG ((DEBUG_INFO,
    "  ***************************************************************************\n"
    ));
  Print (L"  ***************************************************************************\n");
  DEBUG ((DEBUG_INFO,
    "  *       Remapping Hardware Status Affinity Structure                      *\n"
    ));
  Print (L"  *       Remapping Hardware Status Affinity Structure                      *\n");
  DEBUG ((DEBUG_INFO,
    "  ***************************************************************************\n"
    ));
  Print (L"  ***************************************************************************\n");
  DEBUG ((DEBUG_INFO,
    (sizeof(UINTN) == sizeof(UINT64)) ?
    "  RHSA address ........................................... 0x%016lx\n" :
    "  RHSA address ........................................... 0x%08x\n",
    Rhsa
    ));
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  RHSA address ........................................... 0x%016lx\n" :
    L"  RHSA address ........................................... 0x%08x\n",
    Rhsa
    );
  DEBUG ((DEBUG_INFO,
    "    Type ................................................. 0x%04x\n",
    Rhsa->Header.Type
    ));
  Print (
    L"    Type ................................................. 0x%04x\n",
    Rhsa->Header.Type
    );
  DEBUG ((DEBUG_INFO,
    "    Length ............................................... 0x%04x\n",
    Rhsa->Header.Length
    ));
  Print (
    L"    Length ............................................... 0x%04x\n",
    Rhsa->Header.Length
    );
  DEBUG ((DEBUG_INFO,
    "    Register Base Address ................................ 0x%016lx\n",
    Rhsa->RegisterBaseAddress
    ));
  Print (
    L"    Register Base Address ................................ 0x%016lx\n",
    Rhsa->RegisterBaseAddress
    );
  DEBUG ((DEBUG_INFO,
    "    Proximity Domain ..................................... 0x%08x\n",
    Rhsa->ProximityDomain
    ));
  Print (
    L"    Proximity Domain ..................................... 0x%08x\n",
    Rhsa->ProximityDomain
    );

  DEBUG ((DEBUG_INFO,
    "  ***************************************************************************\n\n"
    ));
  Print (L"  ***************************************************************************\n\n");

  return;
}

/**
  Dump DMAR ATSR table.

  @param[in]  Atsr  DMAR ATSR table
**/
VOID
DumpDmarAtsr (
  IN EFI_ACPI_DMAR_ATSR_HEADER *Atsr
  )
{
  EFI_ACPI_DMAR_DEVICE_SCOPE_STRUCTURE_HEADER       *DmarDeviceScopeEntry;
  INTN                                    AtsrLen;

  if (Atsr == NULL) {
    return;
  }

  DEBUG ((DEBUG_INFO,
    "  ***************************************************************************\n"
    ));
  Print (L"  ***************************************************************************\n");
  DEBUG ((DEBUG_INFO,
    "  *       Root Port ATS Capability Reporting Structure                      *\n"
    ));
  Print (L"  *       Root Port ATS Capability Reporting Structure                      *\n");
  DEBUG ((DEBUG_INFO,
    "  ***************************************************************************\n"
    ));
  Print (L"  ***************************************************************************\n");
  DEBUG ((DEBUG_INFO,
    (sizeof(UINTN) == sizeof(UINT64)) ?
    "  ATSR address ........................................... 0x%016lx\n" :
    "  ATSR address ........................................... 0x%08x\n",
    Atsr
    ));
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  ATSR address ........................................... 0x%016lx\n" :
    L"  ATSR address ........................................... 0x%08x\n",
    Atsr
    );
  DEBUG ((DEBUG_INFO,
    "    Type ................................................. 0x%04x\n",
    Atsr->Header.Type
    ));
  Print (
    L"    Type ................................................. 0x%04x\n",
    Atsr->Header.Type
    );
  DEBUG ((DEBUG_INFO,
    "    Length ............................................... 0x%04x\n",
    Atsr->Header.Length
    ));
  Print (
    L"    Length ............................................... 0x%04x\n",
    Atsr->Header.Length
    );
  DEBUG ((DEBUG_INFO,
    "    Flags ................................................ 0x%02x\n",
    Atsr->Flags
    ));
  Print (
    L"    Flags ................................................ 0x%02x\n",
    Atsr->Flags
    );
  Print (
    L"      ALL_PORTS .......................................... 0x%02x\n",
    Atsr->Flags & EFI_ACPI_DMAR_ATSR_FLAGS_ALL_PORTS
    );
  DEBUG ((DEBUG_INFO,
    "      ALL_PORTS .......................................... 0x%02x\n",
    Atsr->Flags & EFI_ACPI_DMAR_ATSR_FLAGS_ALL_PORTS
    ));
  DEBUG ((DEBUG_INFO,
    "    Segment Number ....................................... 0x%04x\n",
    Atsr->SegmentNumber
    ));
  Print (
    L"    Segment Number ....................................... 0x%04x\n",
    Atsr->SegmentNumber
    );

  AtsrLen  = Atsr->Header.Length - sizeof(EFI_ACPI_DMAR_ATSR_HEADER);
  DmarDeviceScopeEntry = (EFI_ACPI_DMAR_DEVICE_SCOPE_STRUCTURE_HEADER *)(Atsr + 1);
  while (AtsrLen > 0) {
    DumpDmarDeviceScopeEntry (DmarDeviceScopeEntry);
    AtsrLen -= DmarDeviceScopeEntry->Length;
    DmarDeviceScopeEntry = (EFI_ACPI_DMAR_DEVICE_SCOPE_STRUCTURE_HEADER *)((UINTN)DmarDeviceScopeEntry + DmarDeviceScopeEntry->Length);
  }

  DEBUG ((DEBUG_INFO,
    "  ***************************************************************************\n\n"
    ));
  Print (L"  ***************************************************************************\n\n");

  return;
}

/**
  Dump DMAR RMRR table.

  @param[in]  Rmrr  DMAR RMRR table
**/
VOID
DumpDmarRmrr (
  IN EFI_ACPI_DMAR_RMRR_HEADER *Rmrr
  )
{
  EFI_ACPI_DMAR_DEVICE_SCOPE_STRUCTURE_HEADER       *DmarDeviceScopeEntry;
  INTN                                    RmrrLen;

  if (Rmrr == NULL) {
    return;
  }

  DEBUG ((DEBUG_INFO,
    "  ***************************************************************************\n"
    ));
  Print (L"  ***************************************************************************\n");
  DEBUG ((DEBUG_INFO,
    "  *       Reserved Memory Region Reporting Structure                        *\n"
    ));
  Print (L"  *       Reserved Memory Region Reporting Structure                        *\n");
  DEBUG ((DEBUG_INFO,
    "  ***************************************************************************\n"
    ));
  Print (L"  ***************************************************************************\n");
  DEBUG ((DEBUG_INFO,
    (sizeof(UINTN) == sizeof(UINT64)) ?
    "  RMRR address ........................................... 0x%016lx\n" :
    "  RMRR address ........................................... 0x%08x\n",
    Rmrr
    ));
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  RMRR address ........................................... 0x%016lx\n" :
    L"  RMRR address ........................................... 0x%08x\n",
    Rmrr
    );
  DEBUG ((DEBUG_INFO,
    "    Type ................................................. 0x%04x\n",
    Rmrr->Header.Type
    ));
  Print (
    L"    Type ................................................. 0x%04x\n",
    Rmrr->Header.Type
    );
  DEBUG ((DEBUG_INFO,
    "    Length ............................................... 0x%04x\n",
    Rmrr->Header.Length
    ));
  Print (
    L"    Length ............................................... 0x%04x\n",
    Rmrr->Header.Length
    );
  DEBUG ((DEBUG_INFO,
    "    Segment Number ....................................... 0x%04x\n",
    Rmrr->SegmentNumber
    ));
  Print (
    L"    Segment Number ....................................... 0x%04x\n",
    Rmrr->SegmentNumber
    );
  DEBUG ((DEBUG_INFO,
    "    Reserved Memory Region Base Address .................. 0x%016lx\n",
    Rmrr->ReservedMemoryRegionBaseAddress
    ));
  Print (
    L"    Reserved Memory Region Base Address .................. 0x%016lx\n",
    Rmrr->ReservedMemoryRegionBaseAddress
    );
  DEBUG ((DEBUG_INFO,
    "    Reserved Memory Region Limit Address ................. 0x%016lx\n",
    Rmrr->ReservedMemoryRegionLimitAddress
    ));
  Print (
    L"    Reserved Memory Region Limit Address ................. 0x%016lx\n",
    Rmrr->ReservedMemoryRegionLimitAddress
    );

  RmrrLen  = Rmrr->Header.Length - sizeof(EFI_ACPI_DMAR_RMRR_HEADER);
  DmarDeviceScopeEntry = (EFI_ACPI_DMAR_DEVICE_SCOPE_STRUCTURE_HEADER *)(Rmrr + 1);
  while (RmrrLen > 0) {
    DumpDmarDeviceScopeEntry (DmarDeviceScopeEntry);
    RmrrLen -= DmarDeviceScopeEntry->Length;
    DmarDeviceScopeEntry = (EFI_ACPI_DMAR_DEVICE_SCOPE_STRUCTURE_HEADER *)((UINTN)DmarDeviceScopeEntry + DmarDeviceScopeEntry->Length);
  }

  DEBUG ((DEBUG_INFO,
    "  ***************************************************************************\n\n"
    ));
  Print (L"  ***************************************************************************\n\n");

  return;
}

/**
  Dump DMAR DRHD table.

  @param[in]  Drhd  DMAR DRHD table
**/
VOID
DumpDmarDrhd (
  IN EFI_ACPI_DMAR_DRHD_HEADER *Drhd
  )
{
  EFI_ACPI_DMAR_DEVICE_SCOPE_STRUCTURE_HEADER       *DmarDeviceScopeEntry;
  INTN                                    DrhdLen;

  if (Drhd == NULL) {
    return;
  }

  DEBUG ((DEBUG_INFO,
    "  ***************************************************************************\n"
    ));
  Print (L"  ***************************************************************************\n");
  DEBUG ((DEBUG_INFO,
    "  *       DMA-Remapping Hardware Definition Structure                       *\n"
    ));
  Print (L"  *       DMA-Remapping Hardware Definition Structure                       *\n");
  DEBUG ((DEBUG_INFO,
    "  ***************************************************************************\n"
    ));
  Print (L"  ***************************************************************************\n");
  DEBUG ((DEBUG_INFO,
    (sizeof(UINTN) == sizeof(UINT64)) ?
    "  DRHD address ........................................... 0x%016lx\n" :
    "  DRHD address ........................................... 0x%08x\n",
    Drhd
    ));
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  DRHD address ........................................... 0x%016lx\n" :
    L"  DRHD address ........................................... 0x%08x\n",
    Drhd
    );
  DEBUG ((DEBUG_INFO,
    "    Type ................................................. 0x%04x\n",
    Drhd->Header.Type
    ));
  Print (
    L"    Type ................................................. 0x%04x\n",
    Drhd->Header.Type
    );
  DEBUG ((DEBUG_INFO,
    "    Length ............................................... 0x%04x\n",
    Drhd->Header.Length
    ));
  Print (
    L"    Length ............................................... 0x%04x\n",
    Drhd->Header.Length
    );
  DEBUG ((DEBUG_INFO,
    "    Flags ................................................ 0x%02x\n",
    Drhd->Flags
    ));
  Print (
    L"    Flags ................................................ 0x%02x\n",
    Drhd->Flags
    );
  DEBUG ((DEBUG_INFO,
    "      INCLUDE_PCI_ALL .................................... 0x%02x\n",
    Drhd->Flags & EFI_ACPI_DMAR_DRHD_FLAGS_INCLUDE_PCI_ALL
    ));
  Print (
    L"      INCLUDE_PCI_ALL .................................... 0x%02x\n",
    Drhd->Flags & EFI_ACPI_DMAR_DRHD_FLAGS_INCLUDE_PCI_ALL
    );
  DEBUG ((DEBUG_INFO,
    "    Segment Number ....................................... 0x%04x\n",
    Drhd->SegmentNumber
    ));
  Print (
    L"    Segment Number ....................................... 0x%04x\n",
    Drhd->SegmentNumber
    );
  DEBUG ((DEBUG_INFO,
    "    Register Base Address ................................ 0x%016lx\n",
    Drhd->RegisterBaseAddress
    ));
  Print (
    L"    Register Base Address ................................ 0x%016lx\n",
    Drhd->RegisterBaseAddress
    );

  DrhdLen  = Drhd->Header.Length - sizeof(EFI_ACPI_DMAR_DRHD_HEADER);
  DmarDeviceScopeEntry = (EFI_ACPI_DMAR_DEVICE_SCOPE_STRUCTURE_HEADER *)(Drhd + 1);
  while (DrhdLen > 0) {
    DumpDmarDeviceScopeEntry (DmarDeviceScopeEntry);
    DrhdLen -= DmarDeviceScopeEntry->Length;
    DmarDeviceScopeEntry = (EFI_ACPI_DMAR_DEVICE_SCOPE_STRUCTURE_HEADER *)((UINTN)DmarDeviceScopeEntry + DmarDeviceScopeEntry->Length);
  }

  DEBUG ((DEBUG_INFO,
    "  ***************************************************************************\n\n"
    ));
  Print (L"  ***************************************************************************\n\n");

  return;
}

/**
  Dump DMAR ACPI table.

  @param[in]  Dmar  DMAR ACPI table
**/
VOID
DumpAcpiDMAR (
  IN EFI_ACPI_DMAR_HEADER  *Dmar
  )
{
  EFI_ACPI_DMAR_STRUCTURE_HEADER *DmarHeader;
  INTN                  DmarLen;

  if (Dmar == NULL) {
    return;
  }

  //
  // Dump Dmar table
  //
  DEBUG ((DEBUG_INFO,
    "*****************************************************************************\n"
    ));
  Print (L"*****************************************************************************\n");
  DEBUG ((DEBUG_INFO,
    "*         DMAR Table                                                        *\n"
    ));
  Print (L"*         DMAR Table                                                        *\n");
  DEBUG ((DEBUG_INFO,
    "*****************************************************************************\n"
    ));
  Print (L"*****************************************************************************\n");

  DEBUG ((DEBUG_INFO,
    (sizeof(UINTN) == sizeof(UINT64)) ?
    "DMAR address ............................................. 0x%016lx\n" :
    "DMAR address ............................................. 0x%08x\n",
    Dmar
    ));
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"DMAR address ............................................. 0x%016lx\n" :
    L"DMAR address ............................................. 0x%08x\n",
    Dmar
    );

  DEBUG ((DEBUG_INFO,
    "  Table Contents:\n"
    ));
  Print (
    L"  Table Contents:\n"
    );
  DEBUG ((DEBUG_INFO,
    "    Host Address Width ................................... 0x%02x\n",
    Dmar->HostAddressWidth
    ));
  Print (
    L"    Host Address Width ................................... 0x%02x\n",
    Dmar->HostAddressWidth
    );
  DEBUG ((DEBUG_INFO,
    "    Flags ................................................ 0x%02x\n",
    Dmar->Flags
    ));
  Print (
    L"    Flags ................................................ 0x%02x\n",
    Dmar->Flags
    );
  DEBUG ((DEBUG_INFO,
    "      INTR_REMAP ......................................... 0x%02x\n",
    Dmar->Flags & EFI_ACPI_DMAR_FLAGS_INTR_REMAP
    ));
  Print (
    L"      INTR_REMAP ......................................... 0x%02x\n",
    Dmar->Flags & EFI_ACPI_DMAR_FLAGS_INTR_REMAP
    );
  DEBUG ((DEBUG_INFO,
    "      X2APIC_OPT_OUT_SET ................................. 0x%02x\n",
    Dmar->Flags & EFI_ACPI_DMAR_FLAGS_X2APIC_OPT_OUT
    ));
  Print (
    L"      X2APIC_OPT_OUT_SET ................................. 0x%02x\n",
    Dmar->Flags & EFI_ACPI_DMAR_FLAGS_X2APIC_OPT_OUT
    );
  DEBUG ((DEBUG_INFO,
    "      DMA_CTRL_PLATFORM_OPT_IN_FLAG ...................... 0x%02x\n",
    Dmar->Flags & EFI_ACPI_DMAR_FLAGS_DMA_CTRL_PLATFORM_OPT_IN_FLAG
    ));
  Print (
    L"      DMA_CTRL_PLATFORM_OPT_IN_FLAG ...................... 0x%02x\n",
    Dmar->Flags & EFI_ACPI_DMAR_FLAGS_DMA_CTRL_PLATFORM_OPT_IN_FLAG
    );

  DmarLen  = Dmar->Header.Length - sizeof(EFI_ACPI_DMAR_HEADER);
  DmarHeader = (EFI_ACPI_DMAR_STRUCTURE_HEADER *)(Dmar + 1);
  while (DmarLen > 0) {
    switch (DmarHeader->Type) {
    case EFI_ACPI_DMAR_TYPE_DRHD:
      DumpDmarDrhd ((EFI_ACPI_DMAR_DRHD_HEADER *)DmarHeader);
      break;
    case EFI_ACPI_DMAR_TYPE_RMRR:
      DumpDmarRmrr ((EFI_ACPI_DMAR_RMRR_HEADER *)DmarHeader);
      break;
    case EFI_ACPI_DMAR_TYPE_ATSR:
      DumpDmarAtsr ((EFI_ACPI_DMAR_ATSR_HEADER *)DmarHeader);
      break;
    case EFI_ACPI_DMAR_TYPE_RHSA:
      DumpDmarRhsa ((EFI_ACPI_DMAR_RHSA_HEADER *)DmarHeader);
      break;
    case EFI_ACPI_DMAR_TYPE_ANDD:
      DumpDmarAndd ((EFI_ACPI_DMAR_ANDD_HEADER *)DmarHeader);
      break;
    default:
      break;
    }
    DmarLen -= DmarHeader->Length;
    DmarHeader = (EFI_ACPI_DMAR_STRUCTURE_HEADER *)((UINTN)DmarHeader + DmarHeader->Length);
  }

  DEBUG ((DEBUG_INFO,
    "*****************************************************************************\n\n"
    ));
  Print (L"*****************************************************************************\n\n");

  return;
}

/**
  Dump DMAR ACPI table.
**/
VOID
VtdDumpDmarTable (
  VOID
  )
{
  DumpAcpiDMAR ((EFI_ACPI_DMAR_HEADER *)(UINTN)mAcpiDmarTable);
}

/**
  This function scan ACPI table in RSDT.

  @param[in]  Rsdt      ACPI RSDT
  @param[in]  Signature ACPI table signature

  @return ACPI table
**/
VOID *
ScanTableInRSDT (
  IN RSDT_TABLE                   *Rsdt,
  IN UINT32                       Signature
  )
{
  UINTN                         Index;
  UINT32                        EntryCount;
  UINT32                        *EntryPtr;
  EFI_ACPI_DESCRIPTION_HEADER   *Table;

  EntryCount = (Rsdt->Header.Length - sizeof (EFI_ACPI_DESCRIPTION_HEADER)) / sizeof(UINT32);

  EntryPtr = &Rsdt->Entry;
  for (Index = 0; Index < EntryCount; Index ++, EntryPtr ++) {
    Table = (EFI_ACPI_DESCRIPTION_HEADER*)((UINTN)(*EntryPtr));
    if ((Table != NULL) && (Table->Signature == Signature)) {
      return Table;
    }
  }

  return NULL;
}

/**
  This function scan ACPI table in XSDT.

  @param[in]  Xsdt      ACPI XSDT
  @param[in]  Signature ACPI table signature

  @return ACPI table
**/
VOID *
ScanTableInXSDT (
  IN XSDT_TABLE                   *Xsdt,
  IN UINT32                       Signature
  )
{
  UINTN                        Index;
  UINT32                       EntryCount;
  UINT64                       EntryPtr;
  UINTN                        BasePtr;
  EFI_ACPI_DESCRIPTION_HEADER  *Table;

  EntryCount = (Xsdt->Header.Length - sizeof (EFI_ACPI_DESCRIPTION_HEADER)) / sizeof(UINT64);

  BasePtr = (UINTN)(&(Xsdt->Entry));
  for (Index = 0; Index < EntryCount; Index ++) {
    CopyMem (&EntryPtr, (VOID *)(BasePtr + Index * sizeof(UINT64)), sizeof(UINT64));
    Table = (EFI_ACPI_DESCRIPTION_HEADER*)((UINTN)(EntryPtr));
    if ((Table != NULL) && (Table->Signature == Signature)) {
      return Table;
    }
  }

  return NULL;
}

/**
  This function scan ACPI table in RSDP.

  @param[in]  Rsdp      ACPI RSDP
  @param[in]  Signature ACPI table signature

  @return ACPI table
**/
VOID *
FindAcpiPtr (
  IN EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER *Rsdp,
  IN UINT32                                       Signature
  )
{
  EFI_ACPI_DESCRIPTION_HEADER                    *AcpiTable;
  RSDT_TABLE                                     *Rsdt;
  XSDT_TABLE                                     *Xsdt;

  AcpiTable = NULL;

  //
  // Check ACPI2.0 table
  //
  Rsdt = (RSDT_TABLE *)(UINTN)Rsdp->RsdtAddress;
  Xsdt = NULL;
  if ((Rsdp->Revision >= 2) && (Rsdp->XsdtAddress < (UINT64)(UINTN)-1)) {
    Xsdt = (XSDT_TABLE *)(UINTN)Rsdp->XsdtAddress;
  }
  //
  // Check Xsdt
  //
  if (Xsdt != NULL) {
    AcpiTable = ScanTableInXSDT (Xsdt, Signature);
  }
  //
  // Check Rsdt
  //
  if ((AcpiTable == NULL) && (Rsdt != NULL)) {
    AcpiTable = ScanTableInRSDT (Rsdt, Signature);
  }

  return AcpiTable;
}

/**
  Get the DMAR ACPI table.

  @retval EFI_SUCCESS           The DMAR ACPI table is got.
  @retval EFI_ALREADY_STARTED   The DMAR ACPI table has been got previously.
  @retval EFI_NOT_FOUND         The DMAR ACPI table is not found.
**/
EFI_STATUS
GetDmarAcpiTable (
  VOID
  )
{
  VOID                              *AcpiTable;
  EFI_STATUS                        Status;

  AcpiTable = NULL;
  Status = EfiGetSystemConfigurationTable (
             &gEfiAcpi20TableGuid,
             &AcpiTable
             );
  if (EFI_ERROR (Status)) {
    Status = EfiGetSystemConfigurationTable (
               &gEfiAcpi10TableGuid,
               &AcpiTable
               );
  }
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }
  ASSERT (AcpiTable != NULL);

  mAcpiDmarTable = FindAcpiPtr (
                      (EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER *)AcpiTable,
                      EFI_ACPI_4_0_DMA_REMAPPING_TABLE_SIGNATURE
                      );
  if (mAcpiDmarTable == NULL) {
    return EFI_NOT_FOUND;
  }
  Print (L"\nDMAR Table - 0x%08x\n", mAcpiDmarTable);
  VtdDumpDmarTable();

  return EFI_SUCCESS;
}

/**
  Get VTd engine number.
**/
UINTN
GetVtdEngineNumber (
  VOID
  )
{
  EFI_ACPI_DMAR_STRUCTURE_HEADER                    *DmarHeader;
  UINTN                                             VtdIndex;

  VtdIndex = 0;
  DmarHeader = (EFI_ACPI_DMAR_STRUCTURE_HEADER *)((UINTN)(mAcpiDmarTable + 1));
  while ((UINTN)DmarHeader < (UINTN)mAcpiDmarTable + mAcpiDmarTable->Header.Length) {
    switch (DmarHeader->Type) {
    case EFI_ACPI_DMAR_TYPE_DRHD:
      VtdIndex++;
      break;
    default:
      break;
    }
    DmarHeader = (EFI_ACPI_DMAR_STRUCTURE_HEADER *)((UINTN)DmarHeader + DmarHeader->Length);
  }
  return VtdIndex ;
}

/**
  Return the index of PCI data.

  @param[in]  VtdIndex          The index used to identify a VTd engine.
  @param[in]  Segment           The Segment used to identify a VTd engine.
  @param[in]  SourceId          The SourceId used to identify a VTd engine and table entry.

  @return The index of the PCI data.
  @retval (UINTN)-1  The PCI data is not found.
**/
UINTN
GetPciDataIndex (
  IN UINTN          VtdIndex,
  IN UINT16         Segment,
  IN VTD_SOURCE_ID  SourceId
  )
{
  UINTN          Index;
  VTD_SOURCE_ID  *PciSourceId;

  if (Segment != mVtdUnitInformation[VtdIndex].Segment) {
    return (UINTN)-1;
  }

  for (Index = 0; Index < mVtdUnitInformation[VtdIndex].PciDeviceInfo.PciDeviceDataNumber; Index++) {
    PciSourceId = &mVtdUnitInformation[VtdIndex].PciDeviceInfo.PciDeviceData[Index].PciSourceId;
    if ((PciSourceId->Bits.Bus == SourceId.Bits.Bus) &&
        (PciSourceId->Bits.Device == SourceId.Bits.Device) &&
        (PciSourceId->Bits.Function == SourceId.Bits.Function) ) {
      return Index;
    }
  }

  return (UINTN)-1;
}

/**
  Register PCI device to VTd engine.

  @param[in]  VtdIndex              The index of VTd engine.
  @param[in]  Segment               The segment of the source.
  @param[in]  SourceId              The SourceId of the source.
  @param[in]  DeviceType            The DMAR device scope type.
  @param[in]  CheckExist            TRUE: ERROR will be returned if the PCI device is already registered.
                                    FALSE: SUCCESS will be returned if the PCI device is registered.

  @retval EFI_SUCCESS           The PCI device is registered.
  @retval EFI_OUT_OF_RESOURCES  No enough resource to register a new PCI device.
  @retval EFI_ALREADY_STARTED   The device is already registered.
**/
EFI_STATUS
RegisterPciDevice (
  IN UINTN          VtdIndex,
  IN UINT16         Segment,
  IN VTD_SOURCE_ID  SourceId,
  IN UINT8          DeviceType,
  IN BOOLEAN        CheckExist
  )
{
  PCI_DEVICE_INFORMATION           *PciDeviceInfo;
  VTD_SOURCE_ID                    *PciSourceId;
  UINTN                            PciDataIndex;
  UINTN                            Index;
  PCI_DEVICE_DATA                  *NewPciDeviceData;
  EDKII_PLATFORM_VTD_PCI_DEVICE_ID *PciDeviceId;

  PciDeviceInfo = &mVtdUnitInformation[VtdIndex].PciDeviceInfo;

  if (PciDeviceInfo->IncludeAllFlag) {
    //
    // Do not register device in other VTD Unit
    //
    for (Index = 0; Index < VtdIndex; Index++) {
      PciDataIndex = GetPciDataIndex (Index, Segment, SourceId);
      if (PciDataIndex != (UINTN)-1) {
        DEBUG ((DEBUG_INFO, "  RegisterPciDevice: PCI S%04x B%02x D%02x F%02x already registered by Other Vtd(%d)\n", Segment, SourceId.Bits.Bus, SourceId.Bits.Device, SourceId.Bits.Function, Index));
        Print (L"  RegisterPciDevice: PCI S%04x B%02x D%02x F%02x already registered by Other Vtd(%d)\n", Segment, SourceId.Bits.Bus, SourceId.Bits.Device, SourceId.Bits.Function, Index);
        return EFI_SUCCESS;
      }
    }
  }

  PciDataIndex = GetPciDataIndex (VtdIndex, Segment, SourceId);
  if (PciDataIndex == (UINTN)-1) {
    //
    // Register new
    //

    if (PciDeviceInfo->PciDeviceDataNumber >= PciDeviceInfo->PciDeviceDataMaxNumber) {
      //
      // Reallocate
      //
      NewPciDeviceData = AllocateZeroPool (sizeof(*NewPciDeviceData) * (PciDeviceInfo->PciDeviceDataMaxNumber + MAX_VTD_PCI_DATA_NUMBER));
      if (NewPciDeviceData == NULL) {
        return EFI_OUT_OF_RESOURCES;
      }
      PciDeviceInfo->PciDeviceDataMaxNumber += MAX_VTD_PCI_DATA_NUMBER;
      if (PciDeviceInfo->PciDeviceData != NULL) {
        CopyMem (NewPciDeviceData, PciDeviceInfo->PciDeviceData, sizeof(*NewPciDeviceData) * PciDeviceInfo->PciDeviceDataNumber);
        FreePool (PciDeviceInfo->PciDeviceData);
      }
      PciDeviceInfo->PciDeviceData = NewPciDeviceData;
    }

    ASSERT (PciDeviceInfo->PciDeviceDataNumber < PciDeviceInfo->PciDeviceDataMaxNumber);

    PciSourceId = &PciDeviceInfo->PciDeviceData[PciDeviceInfo->PciDeviceDataNumber].PciSourceId;
    PciSourceId->Bits.Bus = SourceId.Bits.Bus;
    PciSourceId->Bits.Device = SourceId.Bits.Device;
    PciSourceId->Bits.Function = SourceId.Bits.Function;

    DEBUG ((DEBUG_INFO, "  RegisterPciDevice: PCI S%04x B%02x D%02x F%02x", Segment, SourceId.Bits.Bus, SourceId.Bits.Device, SourceId.Bits.Function));
    Print (L"  RegisterPciDevice: PCI S%04x B%02x D%02x F%02x", Segment, SourceId.Bits.Bus, SourceId.Bits.Device, SourceId.Bits.Function);

    PciDeviceId = &PciDeviceInfo->PciDeviceData[PciDeviceInfo->PciDeviceDataNumber].PciDeviceId;
    if ((DeviceType == EFI_ACPI_DEVICE_SCOPE_ENTRY_TYPE_PCI_ENDPOINT) ||
        (DeviceType == EFI_ACPI_DEVICE_SCOPE_ENTRY_TYPE_PCI_BRIDGE)) {
      PciDeviceId->VendorId   = PciSegmentRead16 (PCI_SEGMENT_LIB_ADDRESS(Segment, SourceId.Bits.Bus, SourceId.Bits.Device, SourceId.Bits.Function, PCI_VENDOR_ID_OFFSET));
      PciDeviceId->DeviceId   = PciSegmentRead16 (PCI_SEGMENT_LIB_ADDRESS(Segment, SourceId.Bits.Bus, SourceId.Bits.Device, SourceId.Bits.Function, PCI_DEVICE_ID_OFFSET));
      PciDeviceId->RevisionId = PciSegmentRead8 (PCI_SEGMENT_LIB_ADDRESS(Segment, SourceId.Bits.Bus, SourceId.Bits.Device, SourceId.Bits.Function, PCI_REVISION_ID_OFFSET));

      DEBUG ((DEBUG_INFO, " (%04x:%04x:%02x", PciDeviceId->VendorId, PciDeviceId->DeviceId, PciDeviceId->RevisionId));
      Print (L" (%04x:%04x:%02x", PciDeviceId->VendorId, PciDeviceId->DeviceId, PciDeviceId->RevisionId);

      if (DeviceType == EFI_ACPI_DEVICE_SCOPE_ENTRY_TYPE_PCI_ENDPOINT) {
        PciDeviceId->SubsystemVendorId = PciSegmentRead16 (PCI_SEGMENT_LIB_ADDRESS(Segment, SourceId.Bits.Bus, SourceId.Bits.Device, SourceId.Bits.Function, PCI_SUBSYSTEM_VENDOR_ID_OFFSET));
        PciDeviceId->SubsystemDeviceId = PciSegmentRead16 (PCI_SEGMENT_LIB_ADDRESS(Segment, SourceId.Bits.Bus, SourceId.Bits.Device, SourceId.Bits.Function, PCI_SUBSYSTEM_ID_OFFSET));
        DEBUG ((DEBUG_INFO, ":%04x:%04x", PciDeviceId->SubsystemVendorId, PciDeviceId->SubsystemDeviceId));
        Print (L":%04x:%04x", PciDeviceId->SubsystemVendorId, PciDeviceId->SubsystemDeviceId);
      }
      DEBUG ((DEBUG_INFO, ")"));
      Print (L")");
    }

    PciDeviceInfo->PciDeviceData[PciDeviceInfo->PciDeviceDataNumber].DeviceType = DeviceType;

    if ((DeviceType != EFI_ACPI_DEVICE_SCOPE_ENTRY_TYPE_PCI_ENDPOINT) &&
        (DeviceType != EFI_ACPI_DEVICE_SCOPE_ENTRY_TYPE_PCI_BRIDGE)) {
      DEBUG ((DEBUG_INFO, " (*)"));
      Print (L" (*)");
    }
    DEBUG ((DEBUG_INFO, "\n"));
    Print (L"\n");

    PciDeviceInfo->PciDeviceDataNumber++;
  } else {
    if (CheckExist) {
      DEBUG ((DEBUG_INFO, "  RegisterPciDevice: PCI S%04x B%02x D%02x F%02x already registered\n", Segment, SourceId.Bits.Bus, SourceId.Bits.Device, SourceId.Bits.Function));
      Print (L"  RegisterPciDevice: PCI S%04x B%02x D%02x F%02x already registered\n", Segment, SourceId.Bits.Bus, SourceId.Bits.Device, SourceId.Bits.Function);
      return EFI_ALREADY_STARTED;
    }
  }

  return EFI_SUCCESS;
}

/**
  The scan bus callback function to register PCI device.

  @param[in]  Context               The context of the callback.
  @param[in]  Segment               The segment of the source.
  @param[in]  Bus                   The bus of the source.
  @param[in]  Device                The device of the source.
  @param[in]  Function              The function of the source.

  @retval EFI_SUCCESS           The PCI device is registered.
**/
EFI_STATUS
EFIAPI
ScanBusCallbackRegisterPciDevice (
  IN VOID           *Context,
  IN UINT16         Segment,
  IN UINT8          Bus,
  IN UINT8          Device,
  IN UINT8          Function
  )
{
  VTD_SOURCE_ID           SourceId;
  UINTN                   VtdIndex;
  UINT8                   BaseClass;
  UINT8                   SubClass;
  UINT8                   DeviceType;
  EFI_STATUS              Status;

  VtdIndex = (UINTN)Context;
  SourceId.Bits.Bus = Bus;
  SourceId.Bits.Device = Device;
  SourceId.Bits.Function = Function;

  DeviceType = EFI_ACPI_DEVICE_SCOPE_ENTRY_TYPE_PCI_ENDPOINT;
  BaseClass = PciSegmentRead8 (PCI_SEGMENT_LIB_ADDRESS(Segment, Bus, Device, Function, PCI_CLASSCODE_OFFSET + 2));
  if (BaseClass == PCI_CLASS_BRIDGE) {
    SubClass = PciSegmentRead8 (PCI_SEGMENT_LIB_ADDRESS(Segment, Bus, Device, Function, PCI_CLASSCODE_OFFSET + 1));
    if (SubClass == PCI_CLASS_BRIDGE_P2P) {
      DeviceType = EFI_ACPI_DEVICE_SCOPE_ENTRY_TYPE_PCI_BRIDGE;
    }
  }

  Status = RegisterPciDevice (VtdIndex, Segment, SourceId, DeviceType, FALSE);
  return Status;
}

/**
  Scan PCI bus and invoke callback function for each PCI devices under the bus.

  @param[in]  Context               The context of the callback function.
  @param[in]  Segment               The segment of the source.
  @param[in]  Bus                   The bus of the source.
  @param[in]  Callback              The callback function in PCI scan.

  @retval EFI_SUCCESS           The PCI devices under the bus are scaned.
**/
EFI_STATUS
ScanPciBus (
  IN VOID                         *Context,
  IN UINT16                       Segment,
  IN UINT8                        Bus,
  IN SCAN_BUS_FUNC_CALLBACK_FUNC  Callback
  )
{
  UINT8                   Device;
  UINT8                   Function;
  UINT8                   SecondaryBusNumber;
  UINT8                   HeaderType;
  UINT8                   BaseClass;
  UINT8                   SubClass;
  UINT32                  MaxFunction;
  UINT16                  VendorID;
  UINT16                  DeviceID;
  EFI_STATUS              Status;

  // Scan the PCI bus for devices
  for (Device = 0; Device < PCI_MAX_DEVICE + 1; Device++) {
    HeaderType = PciSegmentRead8 (PCI_SEGMENT_LIB_ADDRESS(Segment, Bus, Device, 0, PCI_HEADER_TYPE_OFFSET));
    MaxFunction = PCI_MAX_FUNC + 1;
    if ((HeaderType & HEADER_TYPE_MULTI_FUNCTION) == 0x00) {
      MaxFunction = 1;
    }
    for (Function = 0; Function < MaxFunction; Function++) {
      VendorID  = PciSegmentRead16 (PCI_SEGMENT_LIB_ADDRESS(Segment, Bus, Device, Function, PCI_VENDOR_ID_OFFSET));
      DeviceID  = PciSegmentRead16 (PCI_SEGMENT_LIB_ADDRESS(Segment, Bus, Device, Function, PCI_DEVICE_ID_OFFSET));
      if (VendorID == 0xFFFF && DeviceID == 0xFFFF) {
        continue;
      }

      Status = Callback (Context, Segment, Bus, Device, Function);
      if (EFI_ERROR (Status)) {
        return Status;
      }

      BaseClass = PciSegmentRead8 (PCI_SEGMENT_LIB_ADDRESS(Segment, Bus, Device, Function, PCI_CLASSCODE_OFFSET + 2));
      if (BaseClass == PCI_CLASS_BRIDGE) {
        SubClass = PciSegmentRead8 (PCI_SEGMENT_LIB_ADDRESS(Segment, Bus, Device, Function, PCI_CLASSCODE_OFFSET + 1));
        if (SubClass == PCI_CLASS_BRIDGE_P2P) {
          SecondaryBusNumber = PciSegmentRead8 (PCI_SEGMENT_LIB_ADDRESS(Segment, Bus, Device, Function, PCI_BRIDGE_SECONDARY_BUS_REGISTER_OFFSET));
          DEBUG ((DEBUG_INFO,"  ScanPciBus: PCI bridge S%04x B%02x D%02x F%02x (SecondBus:%02x)\n", Segment, Bus, Device, Function, SecondaryBusNumber));
          Print (L"  ScanPciBus: PCI bridge S%04x B%02x D%02x F%02x (SecondBus:%02x)\n", Segment, Bus, Device, Function, SecondaryBusNumber);
          if (SecondaryBusNumber != 0) {
            Status = ScanPciBus (Context, Segment, SecondaryBusNumber, Callback);
            if (EFI_ERROR (Status)) {
              return Status;
            }
          }
        }
      }
    }
  }

  return EFI_SUCCESS;
}

/**
  Dump the PCI device information managed by this VTd engine.

  @param[in]  VtdIndex              The index of VTd engine.
**/
VOID
DumpPciDeviceInfo (
  IN UINTN  VtdIndex
  )
{
  UINTN  Index;

  DEBUG ((DEBUG_INFO,"PCI Device Information (Number 0x%x, IncludeAll - %d):\n",
    mVtdUnitInformation[VtdIndex].PciDeviceInfo.PciDeviceDataNumber,
    mVtdUnitInformation[VtdIndex].PciDeviceInfo.IncludeAllFlag
    ));
  Print (L"PCI Device Information (Number 0x%x, IncludeAll - %d):\n",
    mVtdUnitInformation[VtdIndex].PciDeviceInfo.PciDeviceDataNumber,
    mVtdUnitInformation[VtdIndex].PciDeviceInfo.IncludeAllFlag
    );
  for (Index = 0; Index < mVtdUnitInformation[VtdIndex].PciDeviceInfo.PciDeviceDataNumber; Index++) {
    DEBUG ((DEBUG_INFO,"  S%04x B%02x D%02x F%02x\n",
      mVtdUnitInformation[VtdIndex].Segment,
      mVtdUnitInformation[VtdIndex].PciDeviceInfo.PciDeviceData[Index].PciSourceId.Bits.Bus,
      mVtdUnitInformation[VtdIndex].PciDeviceInfo.PciDeviceData[Index].PciSourceId.Bits.Device,
      mVtdUnitInformation[VtdIndex].PciDeviceInfo.PciDeviceData[Index].PciSourceId.Bits.Function
      ));
    Print (L"  S%04x B%02x D%02x F%02x\n",
      mVtdUnitInformation[VtdIndex].Segment,
      mVtdUnitInformation[VtdIndex].PciDeviceInfo.PciDeviceData[Index].PciSourceId.Bits.Bus,
      mVtdUnitInformation[VtdIndex].PciDeviceInfo.PciDeviceData[Index].PciSourceId.Bits.Device,
      mVtdUnitInformation[VtdIndex].PciDeviceInfo.PciDeviceData[Index].PciSourceId.Bits.Function
      );
  }
}

/**
  Get PCI device information from DMAR DevScopeEntry.

  @param[in]  Segment               The segment number.
  @param[in]  DmarDevScopeEntry     DMAR DevScopeEntry
  @param[out] Bus                   The bus number.
  @param[out] Device                The device number.
  @param[out] Function              The function number.

  @retval EFI_SUCCESS  The PCI device information is returned.
**/
EFI_STATUS
GetPciBusDeviceFunction (
  IN  UINT16                                      Segment,
  IN  EFI_ACPI_DMAR_DEVICE_SCOPE_STRUCTURE_HEADER *DmarDevScopeEntry,
  OUT UINT8                                       *Bus,
  OUT UINT8                                       *Device,
  OUT UINT8                                       *Function
  )
{
  EFI_ACPI_DMAR_PCI_PATH                     *DmarPciPath;
  UINT8                                      MyBus;
  UINT8                                      MyDevice;
  UINT8                                      MyFunction;

  DmarPciPath = (EFI_ACPI_DMAR_PCI_PATH *)((UINTN)(DmarDevScopeEntry + 1));
  MyBus = DmarDevScopeEntry->StartBusNumber;
  MyDevice = DmarPciPath->Device;
  MyFunction = DmarPciPath->Function;

  switch (DmarDevScopeEntry->Type) {
  case EFI_ACPI_DEVICE_SCOPE_ENTRY_TYPE_PCI_ENDPOINT:
  case EFI_ACPI_DEVICE_SCOPE_ENTRY_TYPE_PCI_BRIDGE:
    while ((UINTN)DmarPciPath + sizeof(EFI_ACPI_DMAR_PCI_PATH) < (UINTN)DmarDevScopeEntry + DmarDevScopeEntry->Length) {
      MyBus = PciSegmentRead8 (PCI_SEGMENT_LIB_ADDRESS(Segment, MyBus, MyDevice, MyFunction, PCI_BRIDGE_SECONDARY_BUS_REGISTER_OFFSET));
      DmarPciPath ++;
      MyDevice = DmarPciPath->Device;
      MyFunction = DmarPciPath->Function;
    }
    break;
  case EFI_ACPI_DEVICE_SCOPE_ENTRY_TYPE_IOAPIC:
  case EFI_ACPI_DEVICE_SCOPE_ENTRY_TYPE_MSI_CAPABLE_HPET:
  case EFI_ACPI_DEVICE_SCOPE_ENTRY_TYPE_ACPI_NAMESPACE_DEVICE:
    break;
  }

  *Bus = MyBus;
  *Device = MyDevice;
  *Function = MyFunction;

  return EFI_SUCCESS;
}

/**
  Process DMAR DHRD table.

  @param[in]  VtdIndex  The index of VTd engine.
  @param[in]  DmarDrhd  The DRHD table.

  @retval EFI_SUCCESS The DRHD table is processed.
**/
EFI_STATUS
ProcessDhrd (
  IN UINTN                      VtdIndex,
  IN EFI_ACPI_DMAR_DRHD_HEADER  *DmarDrhd
  )
{
  EFI_ACPI_DMAR_DEVICE_SCOPE_STRUCTURE_HEADER       *DmarDevScopeEntry;
  UINT8                                             Bus;
  UINT8                                             Device;
  UINT8                                             Function;
  UINT8                                             SecondaryBusNumber;
  EFI_STATUS                                        Status;
  VTD_SOURCE_ID                                     SourceId;

  mVtdUnitInformation[VtdIndex].VtdUnitBaseAddress = (UINTN)DmarDrhd->RegisterBaseAddress;
  DEBUG ((DEBUG_INFO,"  VTD (%d) BaseAddress -  0x%016lx\n", VtdIndex, DmarDrhd->RegisterBaseAddress));
  Print (L"  VTD (%d) BaseAddress -  0x%016lx\n", VtdIndex, DmarDrhd->RegisterBaseAddress);

  mVtdUnitInformation[VtdIndex].Segment = DmarDrhd->SegmentNumber;

  if ((DmarDrhd->Flags & EFI_ACPI_DMAR_DRHD_FLAGS_INCLUDE_PCI_ALL) != 0) {
    mVtdUnitInformation[VtdIndex].PciDeviceInfo.IncludeAllFlag = TRUE;
    DEBUG ((DEBUG_INFO,"  ProcessDhrd: with INCLUDE ALL\n"));
    Print (L"  ProcessDhrd: with INCLUDE ALL\n");

    Status = ScanPciBus((VOID *)VtdIndex, DmarDrhd->SegmentNumber, 0, ScanBusCallbackRegisterPciDevice);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  } else {
    mVtdUnitInformation[VtdIndex].PciDeviceInfo.IncludeAllFlag = FALSE;
    DEBUG ((DEBUG_INFO,"  ProcessDhrd: without INCLUDE ALL\n"));
    Print (L"  ProcessDhrd: without INCLUDE ALL\n");
  }

  DmarDevScopeEntry = (EFI_ACPI_DMAR_DEVICE_SCOPE_STRUCTURE_HEADER *)((UINTN)(DmarDrhd + 1));
  while ((UINTN)DmarDevScopeEntry < (UINTN)DmarDrhd + DmarDrhd->Header.Length) {

    Status = GetPciBusDeviceFunction (DmarDrhd->SegmentNumber, DmarDevScopeEntry, &Bus, &Device, &Function);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    DEBUG ((DEBUG_INFO,"  ProcessDhrd: "));
    Print (L"  ProcessDhrd: ");
    switch (DmarDevScopeEntry->Type) {
    case EFI_ACPI_DEVICE_SCOPE_ENTRY_TYPE_PCI_ENDPOINT:
      DEBUG ((DEBUG_INFO,"PCI Endpoint"));
      Print (L"PCI Endpoint");
      break;
    case EFI_ACPI_DEVICE_SCOPE_ENTRY_TYPE_PCI_BRIDGE:
      DEBUG ((DEBUG_INFO,"PCI-PCI bridge"));
      Print (L"PCI-PCI bridge");
      break;
    case EFI_ACPI_DEVICE_SCOPE_ENTRY_TYPE_IOAPIC:
      DEBUG ((DEBUG_INFO,"IOAPIC"));
      Print (L"IOAPIC");
      break;
    case EFI_ACPI_DEVICE_SCOPE_ENTRY_TYPE_MSI_CAPABLE_HPET:
      DEBUG ((DEBUG_INFO,"MSI Capable HPET"));
      Print (L"MSI Capable HPET");
      break;
    case EFI_ACPI_DEVICE_SCOPE_ENTRY_TYPE_ACPI_NAMESPACE_DEVICE:
      DEBUG ((DEBUG_INFO,"ACPI Namespace Device"));
      Print (L"ACPI Namespace Device");
      break;
    }
    DEBUG ((DEBUG_INFO," S%04x B%02x D%02x F%02x\n", DmarDrhd->SegmentNumber, Bus, Device, Function));
    Print (L" S%04x B%02x D%02x F%02x\n", DmarDrhd->SegmentNumber, Bus, Device, Function);

    SourceId.Bits.Bus = Bus;
    SourceId.Bits.Device = Device;
    SourceId.Bits.Function = Function;

    Status = RegisterPciDevice (VtdIndex, DmarDrhd->SegmentNumber, SourceId, DmarDevScopeEntry->Type, TRUE);
    if (EFI_ERROR (Status)) {
      //
      // There might be duplication for special device other than standard PCI device.
      //
      switch (DmarDevScopeEntry->Type) {
      case EFI_ACPI_DEVICE_SCOPE_ENTRY_TYPE_PCI_ENDPOINT:
      case EFI_ACPI_DEVICE_SCOPE_ENTRY_TYPE_PCI_BRIDGE:
        return Status;
      }
    }

    switch (DmarDevScopeEntry->Type) {
    case EFI_ACPI_DEVICE_SCOPE_ENTRY_TYPE_PCI_BRIDGE:
      SecondaryBusNumber = PciSegmentRead8 (PCI_SEGMENT_LIB_ADDRESS(DmarDrhd->SegmentNumber, Bus, Device, Function, PCI_BRIDGE_SECONDARY_BUS_REGISTER_OFFSET));
      Status = ScanPciBus ((VOID *)VtdIndex, DmarDrhd->SegmentNumber, SecondaryBusNumber, ScanBusCallbackRegisterPciDevice);
      if (EFI_ERROR (Status)) {
        return Status;
      }
      break;
    default:
      break;
    }

    DmarDevScopeEntry = (EFI_ACPI_DMAR_DEVICE_SCOPE_STRUCTURE_HEADER *)((UINTN)DmarDevScopeEntry + DmarDevScopeEntry->Length);
  }

  return EFI_SUCCESS;
}

/**
  Process DMAR RMRR table.

  @param[in]  DmarRmrr  The RMRR table.

  @retval EFI_SUCCESS The RMRR table is processed.
**/
EFI_STATUS
ProcessRmrr (
  IN EFI_ACPI_DMAR_RMRR_HEADER  *DmarRmrr
  )
{
  EFI_ACPI_DMAR_DEVICE_SCOPE_STRUCTURE_HEADER       *DmarDevScopeEntry;
  UINT8                                             Bus;
  UINT8                                             Device;
  UINT8                                             Function;
  EFI_STATUS                                        Status;
  RMRR_RECORD                                       *RmrrRecord;

  DEBUG ((DEBUG_INFO,"  RMRR (Base 0x%016lx, Limit 0x%016lx)\n", DmarRmrr->ReservedMemoryRegionBaseAddress, DmarRmrr->ReservedMemoryRegionLimitAddress));
  Print (L"  RMRR (Base 0x%016lx, Limit 0x%016lx)\n", DmarRmrr->ReservedMemoryRegionBaseAddress, DmarRmrr->ReservedMemoryRegionLimitAddress);

  DmarDevScopeEntry = (EFI_ACPI_DMAR_DEVICE_SCOPE_STRUCTURE_HEADER *)((UINTN)(DmarRmrr + 1));
  while ((UINTN)DmarDevScopeEntry < (UINTN)DmarRmrr + DmarRmrr->Header.Length) {
    if (DmarDevScopeEntry->Type != EFI_ACPI_DEVICE_SCOPE_ENTRY_TYPE_PCI_ENDPOINT) {
      DEBUG ((DEBUG_INFO,"RMRR DevScopeEntryType is not endpoint, type[0x%x] \n", DmarDevScopeEntry->Type));
      Print (L"RMRR DevScopeEntryType is not endpoint, type[0x%x] \n", DmarDevScopeEntry->Type);
      return EFI_DEVICE_ERROR;
    }

    Status = GetPciBusDeviceFunction (DmarRmrr->SegmentNumber, DmarDevScopeEntry, &Bus, &Device, &Function);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    DEBUG ((DEBUG_INFO,"RMRR S%04x B%02x D%02x F%02x\n", DmarRmrr->SegmentNumber, Bus, Device, Function));
    Print (L"RMRR S%04x B%02x D%02x F%02x\n", DmarRmrr->SegmentNumber, Bus, Device, Function);

    RmrrRecord = AllocatePool (sizeof (RMRR_RECORD));
    if (RmrrRecord != NULL) {
      RmrrRecord->Signature = RMRR_RECORD_SIGNATURE;
      RmrrRecord->Segment   = DmarRmrr->SegmentNumber;
      RmrrRecord->Bus       = Bus;
      RmrrRecord->Device    = Device;
      RmrrRecord->Function  = Function;
      RmrrRecord->Address   = DmarRmrr->ReservedMemoryRegionBaseAddress;
      RmrrRecord->Length    = DmarRmrr->ReservedMemoryRegionLimitAddress + 1 - DmarRmrr->ReservedMemoryRegionBaseAddress;
      InsertTailList (&gRmrrRecord, &RmrrRecord->Link);
    }

    DmarDevScopeEntry = (EFI_ACPI_DMAR_DEVICE_SCOPE_STRUCTURE_HEADER *)((UINTN)DmarDevScopeEntry + DmarDevScopeEntry->Length);
  }

  return EFI_SUCCESS;
}

/**
  Parse DMAR DRHD table.

  @return EFI_SUCCESS  The DMAR DRHD table is parsed.
**/
EFI_STATUS
ParseDmarAcpiTableDrhd (
  VOID
  )
{
  EFI_ACPI_DMAR_STRUCTURE_HEADER                    *DmarHeader;
  EFI_STATUS                                        Status;
  UINTN                                             VtdIndex;

  mVtdUnitNumber = GetVtdEngineNumber ();
  DEBUG ((DEBUG_INFO,"  VtdUnitNumber - %d\n", mVtdUnitNumber));
  Print (L"  VtdUnitNumber - %d\n", mVtdUnitNumber);
  ASSERT (mVtdUnitNumber > 0);
  if (mVtdUnitNumber == 0) {
    return EFI_DEVICE_ERROR;
  }

  mVtdUnitInformation = AllocateZeroPool (sizeof(*mVtdUnitInformation) * mVtdUnitNumber);
  ASSERT (mVtdUnitInformation != NULL);
  if (mVtdUnitInformation == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  VtdIndex = 0;
  DmarHeader = (EFI_ACPI_DMAR_STRUCTURE_HEADER *)((UINTN)(mAcpiDmarTable + 1));
  while ((UINTN)DmarHeader < (UINTN)mAcpiDmarTable + mAcpiDmarTable->Header.Length) {
    switch (DmarHeader->Type) {
    case EFI_ACPI_DMAR_TYPE_DRHD:
      ASSERT (VtdIndex < mVtdUnitNumber);
      Status = ProcessDhrd (VtdIndex, (EFI_ACPI_DMAR_DRHD_HEADER *)DmarHeader);
      if (EFI_ERROR (Status)) {
        return Status;
      }
      VtdIndex++;

      break;

    default:
      break;
    }
    DmarHeader = (EFI_ACPI_DMAR_STRUCTURE_HEADER *)((UINTN)DmarHeader + DmarHeader->Length);
  }
  ASSERT (VtdIndex == mVtdUnitNumber);

  for (VtdIndex = 0; VtdIndex < mVtdUnitNumber; VtdIndex++) {
    DumpPciDeviceInfo (VtdIndex);
  }
  return EFI_SUCCESS ;
}

/**
  Parse DMAR DRHD table.

  @return EFI_SUCCESS  The DMAR DRHD table is parsed.
**/
EFI_STATUS
ParseDmarAcpiTableRmrr (
  VOID
  )
{
  EFI_ACPI_DMAR_STRUCTURE_HEADER                    *DmarHeader;
  EFI_STATUS                                        Status;

  DmarHeader = (EFI_ACPI_DMAR_STRUCTURE_HEADER *)((UINTN)(mAcpiDmarTable + 1));
  while ((UINTN)DmarHeader < (UINTN)mAcpiDmarTable + mAcpiDmarTable->Header.Length) {
    switch (DmarHeader->Type) {
    case EFI_ACPI_DMAR_TYPE_RMRR:
      Status = ProcessRmrr ((EFI_ACPI_DMAR_RMRR_HEADER *)DmarHeader);
      if (EFI_ERROR (Status)) {
        return Status;
      }
      break;
    default:
      break;
    }
    DmarHeader = (EFI_ACPI_DMAR_STRUCTURE_HEADER *)((UINTN)DmarHeader + DmarHeader->Length);
  }
  return EFI_SUCCESS ;
}

VOID
DumpVtdAcpi (
  VOID
  )
{
  EFI_STATUS  Status;

  Status = GetDmarAcpiTable ();
  if (EFI_ERROR(Status)) {
    DEBUG ((DEBUG_INFO, "DMAR table not found\n"));
    Print (L"DMAR table not found\n");
    return ;
  }

  ParseDmarAcpiTableDrhd ();
  ParseDmarAcpiTableRmrr ();
  DumpRmrrRecord ();
}

VOID
DumpVtdTranslation (
  VOID
  )
{
  UINTN                            Index;
  UINT64                           Address;
  UINT64                           RootEntryTableAddress;

  for (Index = 0; Index < mVtdUnitNumber; Index++) {
    DEBUG ((DEBUG_INFO, "\nVTD engine (%d) - Base:0x%08x\n", Index, mVtdUnitInformation[Index].VtdUnitBaseAddress));
    Print (L"\nVTD engine (%d) - Base:0x%08x\n", Index, mVtdUnitInformation[Index].VtdUnitBaseAddress);
    Address = MmioRead64 (mVtdUnitInformation[Index].VtdUnitBaseAddress + R_RTADDR_REG);
    RootEntryTableAddress = Address & ~0xFFF;
    
    if ((Address & BIT11) != 0) {
      DumpDmarExtContextEntryTable (mVtdUnitInformation[Index].Segment, (VTD_EXT_ROOT_ENTRY *)(UINTN)RootEntryTableAddress);
    } else {
      DumpDmarContextEntryTable (mVtdUnitInformation[Index].Segment, (VTD_ROOT_ENTRY *)(UINTN)RootEntryTableAddress);
    }
  }
}


EFI_STATUS
EFIAPI
VtdDumpEntrypoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  DumpIoMmuMap ();

  DumpVtdAcpi ();
  DumpExceptionRecord ();
  DumpVtdTranslation ();

  if (mHasError) {
    DEBUG ((DEBUG_INFO, "Check FAIL!!!\n"));
    Print (L"Check FAIL!!!\n");
  } else {
    DEBUG ((DEBUG_INFO, "Check PASS!!!\n"));
    Print (L"Check PASS!!!\n");
  }

  return EFI_SUCCESS;
}
