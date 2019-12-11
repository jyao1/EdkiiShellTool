/** @file

Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
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

#include <Guid/SystemResourceTable.h>
#include <Guid/FmpCapsule.h>

#include <IndustryStandard/WindowsUxCapsule.h>

#include <Protocol/FirmwareManagement.h>
#include <Protocol/EsrtManagement.h>

#include "EsrtDxe/EsrtImpl.h"

EFI_GUID mEsrtDxeDriverGuid = ESRT_DXE_DRIVER_GUID;

CHAR8 *mFwTypeString[] = {
  "Unknown",
  "SystemFirmware",
  "DeviceFirmware",
  "UefiDriver",
};

CHAR8 *mLastAttemptStatusString[] = {
  "Success",
  "Error: Unsuccessful",
  "Error: Insufficient Resources",
  "Error: Incorrect Version",
  "Error: Invalid Format",
  "Error: Auth Error",
  "Error: Power Event AC",
  "Error: Power Event Battery",
};

CHAR8 *
FwTypeToString(
  IN UINT32 FwType
  )
{
  if (FwType < sizeof(mFwTypeString) / sizeof(mFwTypeString[0])) {
    return mFwTypeString[FwType];
  } else {
    return "Invalid";
  }
}

CHAR8 *
LastAttemptStatusToString(
  IN UINT32 LastAttemptStatus
  )
{
  if (LastAttemptStatus < sizeof(mLastAttemptStatusString) / sizeof(mLastAttemptStatusString[0])) {
    return mLastAttemptStatusString[LastAttemptStatus];
  } else {
    return "Error: Unknown";
  }
}

VOID
DumpEsrtEntry(
  IN EFI_SYSTEM_RESOURCE_ENTRY  *EsrtEntry
  )
{
  Print(L"  FwClass                  - %g\n", &EsrtEntry->FwClass);
  Print(L"  FwType                   - 0x%x (%a)\n", EsrtEntry->FwType, FwTypeToString(EsrtEntry->FwType));
  Print(L"  FwVersion                - 0x%x\n", EsrtEntry->FwVersion);
  Print(L"  LowestSupportedFwVersion - 0x%x\n", EsrtEntry->LowestSupportedFwVersion);
  Print(L"  CapsuleFlags             - 0x%x\n", EsrtEntry->CapsuleFlags);
  Print(L"    PERSIST_ACROSS_RESET   - 0x%x\n", EsrtEntry->CapsuleFlags & CAPSULE_FLAGS_PERSIST_ACROSS_RESET);
  Print(L"    POPULATE_SYSTEM_TABLE  - 0x%x\n", EsrtEntry->CapsuleFlags & CAPSULE_FLAGS_POPULATE_SYSTEM_TABLE);
  Print(L"    INITIATE_RESET         - 0x%x\n", EsrtEntry->CapsuleFlags & CAPSULE_FLAGS_INITIATE_RESET);
  Print(L"  LastAttemptVersion       - 0x%x\n", EsrtEntry->LastAttemptVersion);
  Print(L"  LastAttemptStatus        - 0x%x (%a)\n", EsrtEntry->LastAttemptStatus, LastAttemptStatusToString(EsrtEntry->LastAttemptStatus));
}

VOID
DumpEsrt(
  IN EFI_SYSTEM_RESOURCE_TABLE  *Esrt
  )
{
  UINTN                      Index;
  EFI_SYSTEM_RESOURCE_ENTRY  *EsrtEntry;

  Print(L"EFI_SYSTEM_RESOURCE_TABLE:\n");
  Print(L"FwResourceCount    - 0x%x\n", Esrt->FwResourceCount);
  Print(L"FwResourceCountMax - 0x%x\n", Esrt->FwResourceCountMax);
  Print(L"FwResourceVersion  - 0x%lx\n", Esrt->FwResourceVersion);

  EsrtEntry = (VOID *)(Esrt + 1);
  for (Index = 0; Index < Esrt->FwResourceCount; Index++) {
    Print(L"EFI_SYSTEM_RESOURCE_ENTRY (%d):\n", Index);
    DumpEsrtEntry(EsrtEntry);
    EsrtEntry++;
  }
}

VOID
DumpEsrtAcpiData (
  VOID
  )
{
  EFI_STATUS                 Status;
  EFI_SYSTEM_RESOURCE_TABLE  *Esrt;

  Print(L"###################\n");
  Print(L"# ESRT ACPI TABLE #\n");
  Print(L"###################\n");

  Status = EfiGetSystemConfigurationTable (&gEfiSystemResourceTableGuid, (VOID **)&Esrt);
  if (EFI_ERROR(Status)) {
    Print(L"ESRT - %r\n", Status);
    return;
  }
  DumpEsrt(Esrt);
  Print(L"\n");
}

VOID
DumpFmpImageInfo(
  IN UINTN                           ImageInfoSize,
  IN EFI_FIRMWARE_IMAGE_DESCRIPTOR   *ImageInfo,
  IN UINT32                          DescriptorVersion,
  IN UINT8                           DescriptorCount,
  IN UINTN                           DescriptorSize,
  IN UINT32                          PackageVersion,
  IN CHAR16                          *PackageVersionName
  )
{
  EFI_FIRMWARE_IMAGE_DESCRIPTOR                 *CurrentImageInfo;
  UINTN                                         Index;

  Print(L"  DescriptorVersion  - 0x%x\n", DescriptorVersion);
  Print(L"  DescriptorCount    - 0x%x\n", DescriptorCount);
  Print(L"  DescriptorSize     - 0x%x\n", DescriptorSize);
  Print(L"  PackageVersion     - 0x%x\n", PackageVersion);
  Print(L"  PackageVersionName - \"%s\"\n", PackageVersionName);
  CurrentImageInfo = ImageInfo;
  for (Index = 0; Index < DescriptorCount; Index++) {
    Print(L"  ImageDescriptor (%d)\n", Index);
    Print(L"    ImageIndex                  - 0x%x\n", CurrentImageInfo->ImageIndex);
    Print(L"    ImageTypeId                 - %g\n", &CurrentImageInfo->ImageTypeId);
    Print(L"    ImageId                     - 0x%lx\n", CurrentImageInfo->ImageId);
    Print(L"    ImageIdName                 - \"%s\"\n", CurrentImageInfo->ImageIdName);
    Print(L"    Version                     - 0x%x\n", CurrentImageInfo->Version);
    Print(L"    VersionName                 - \"%s\"\n", CurrentImageInfo->VersionName);
    Print(L"    Size                        - 0x%x\n", CurrentImageInfo->Size);
    Print(L"    AttributesSupported         - 0x%lx\n", CurrentImageInfo->AttributesSupported);
    Print(L"      IMAGE_UPDATABLE           - 0x%lx\n", CurrentImageInfo->AttributesSupported & IMAGE_ATTRIBUTE_IMAGE_UPDATABLE);
    Print(L"      RESET_REQUIRED            - 0x%lx\n", CurrentImageInfo->AttributesSupported & IMAGE_ATTRIBUTE_RESET_REQUIRED);
    Print(L"      AUTHENTICATION_REQUIRED   - 0x%lx\n", CurrentImageInfo->AttributesSupported & IMAGE_ATTRIBUTE_AUTHENTICATION_REQUIRED);
    Print(L"      IN_USE                    - 0x%lx\n", CurrentImageInfo->AttributesSupported & IMAGE_ATTRIBUTE_IN_USE);
    Print(L"      UEFI_IMAGE                - 0x%lx\n", CurrentImageInfo->AttributesSupported & IMAGE_ATTRIBUTE_UEFI_IMAGE);
    Print(L"    AttributesSetting           - 0x%lx\n", CurrentImageInfo->AttributesSetting);
    Print(L"      IMAGE_UPDATABLE           - 0x%lx\n", CurrentImageInfo->AttributesSetting & IMAGE_ATTRIBUTE_IMAGE_UPDATABLE);
    Print(L"      RESET_REQUIRED            - 0x%lx\n", CurrentImageInfo->AttributesSetting & IMAGE_ATTRIBUTE_RESET_REQUIRED);
    Print(L"      AUTHENTICATION_REQUIRED   - 0x%lx\n", CurrentImageInfo->AttributesSetting & IMAGE_ATTRIBUTE_AUTHENTICATION_REQUIRED);
    Print(L"      IN_USE                    - 0x%lx\n", CurrentImageInfo->AttributesSetting & IMAGE_ATTRIBUTE_IN_USE);
    Print(L"      UEFI_IMAGE                - 0x%lx\n", CurrentImageInfo->AttributesSetting & IMAGE_ATTRIBUTE_UEFI_IMAGE);
    Print(L"    Compatibilities             - 0x%lx\n", CurrentImageInfo->Compatibilities);
    Print(L"      COMPATIB_CHECK_SUPPORTED  - 0x%lx\n", CurrentImageInfo->Compatibilities & IMAGE_COMPATIBILITY_CHECK_SUPPORTED);
    if (DescriptorVersion > 1) {
      Print(L"    LowestSupportedImageVersion - 0x%x\n", CurrentImageInfo->LowestSupportedImageVersion);
      if (DescriptorVersion > 2) {
        Print(L"    LastAttemptVersion          - 0x%x\n", CurrentImageInfo->LastAttemptVersion);
        Print(L"    LastAttemptStatus           - 0x%x (%a)\n", CurrentImageInfo->LastAttemptStatus, LastAttemptStatusToString(CurrentImageInfo->LastAttemptStatus));
        Print(L"    HardwareInstance            - 0x%lx\n", CurrentImageInfo->HardwareInstance);
      }
    }
    //
    // Use DescriptorSize to move ImageInfo Pointer to stay compatible with different ImageInfo version
    //
    CurrentImageInfo = (EFI_FIRMWARE_IMAGE_DESCRIPTOR *)((UINT8 *)CurrentImageInfo + DescriptorSize);
  }
}

VOID
DumpFmpPackageInfo(
  IN UINT32                           PackageVersion,
  IN CHAR16                           *PackageVersionName,
  IN UINT32                           PackageVersionNameMaxLen,
  IN UINT64                           AttributesSupported,
  IN UINT64                           AttributesSetting
  )
{
  Print(L"  PackageVersion              - 0x%x\n", PackageVersion);
  Print(L"  PackageVersionName          - \"%s\"\n", PackageVersionName);
  Print(L"  PackageVersionNameMaxLen    - 0x%x\n", PackageVersionNameMaxLen);
  Print(L"  AttributesSupported         - 0x%lx\n", AttributesSupported);
  Print(L"    IMAGE_UPDATABLE           - 0x%lx\n", AttributesSupported & IMAGE_ATTRIBUTE_IMAGE_UPDATABLE);
  Print(L"    RESET_REQUIRED            - 0x%lx\n", AttributesSupported & IMAGE_ATTRIBUTE_RESET_REQUIRED);
  Print(L"    AUTHENTICATION_REQUIRED   - 0x%lx\n", AttributesSupported & IMAGE_ATTRIBUTE_AUTHENTICATION_REQUIRED);
  Print(L"    IN_USE                    - 0x%lx\n", AttributesSupported & IMAGE_ATTRIBUTE_IN_USE);
  Print(L"    UEFI_IMAGE                - 0x%lx\n", AttributesSupported & IMAGE_ATTRIBUTE_UEFI_IMAGE);
  Print(L"  AttributesSetting           - 0x%lx\n", AttributesSetting);
  Print(L"    IMAGE_UPDATABLE           - 0x%lx\n", AttributesSetting & IMAGE_ATTRIBUTE_IMAGE_UPDATABLE);
  Print(L"    RESET_REQUIRED            - 0x%lx\n", AttributesSetting & IMAGE_ATTRIBUTE_RESET_REQUIRED);
  Print(L"    AUTHENTICATION_REQUIRED   - 0x%lx\n", AttributesSetting & IMAGE_ATTRIBUTE_AUTHENTICATION_REQUIRED);
  Print(L"    IN_USE                    - 0x%lx\n", AttributesSetting & IMAGE_ATTRIBUTE_IN_USE);
  Print(L"    UEFI_IMAGE                - 0x%lx\n", AttributesSetting & IMAGE_ATTRIBUTE_UEFI_IMAGE);
}

VOID
DumpFmpData (
  VOID
  )
{
  EFI_STATUS                                    Status;
  EFI_FIRMWARE_MANAGEMENT_PROTOCOL              *Fmp;
  EFI_HANDLE                                    *HandleBuffer;
  UINTN                                         NumberOfHandles;
  UINTN                                         Index;
  EFI_FIRMWARE_IMAGE_DESCRIPTOR                 *FmpImageInfoBuf;
  UINTN                                         ImageInfoSize;
  UINT32                                        FmpImageInfoDescriptorVer;
  UINT8                                         FmpImageInfoCount;
  UINTN                                         DescriptorSize;
  UINT32                                        PackageVersion;
  CHAR16                                        *PackageVersionName;
  UINT32                                        PackageVersionNameMaxLen;
  UINT64                                        AttributesSupported;
  UINT64                                        AttributesSetting;

  Print(L"############\n");
  Print(L"# FMP DATA #\n");
  Print(L"############\n");
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiFirmwareManagementProtocolGuid,
                  NULL,
                  &NumberOfHandles,
                  &HandleBuffer
                  );
  if (EFI_ERROR(Status)) {
    Print(L"FMP protocol - %r\n", EFI_NOT_FOUND);
    return;
  }

  for (Index = 0; Index < NumberOfHandles; Index++) {
    Status = gBS->HandleProtocol(
                    HandleBuffer[Index],
                    &gEfiFirmwareManagementProtocolGuid,
                    (VOID **)&Fmp
                    );
    if (EFI_ERROR(Status)) {
      continue;
    }

    ImageInfoSize = 0;
    Status = Fmp->GetImageInfo (
                    Fmp,
                    &ImageInfoSize,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL
                    );
    if (Status != EFI_BUFFER_TOO_SMALL) {
      continue;
    }

    FmpImageInfoBuf = NULL;
    FmpImageInfoBuf = AllocateZeroPool (ImageInfoSize);
    if (FmpImageInfoBuf == NULL) {
      Status = EFI_OUT_OF_RESOURCES;
      goto EXIT;
    }

    PackageVersionName = NULL;
    Status = Fmp->GetImageInfo (
                    Fmp,
                    &ImageInfoSize,               // ImageInfoSize
                    FmpImageInfoBuf,              // ImageInfo
                    &FmpImageInfoDescriptorVer,   // DescriptorVersion
                    &FmpImageInfoCount,           // DescriptorCount
                    &DescriptorSize,              // DescriptorSize
                    &PackageVersion,              // PackageVersion
                    &PackageVersionName           // PackageVersionName
                    );

    //
    // If FMP GetInformation interface failed, skip this resource
    //
    if (EFI_ERROR(Status)) {
      Print(L"FMP (%d) ImageInfo - %r\n", Index, Status);
      FreePool(FmpImageInfoBuf);
      continue;
    }

    Print(L"FMP (%d) ImageInfo:\n", Index);
    DumpFmpImageInfo(
      ImageInfoSize,               // ImageInfoSize
      FmpImageInfoBuf,             // ImageInfo
      FmpImageInfoDescriptorVer,   // DescriptorVersion
      FmpImageInfoCount,           // DescriptorCount
      DescriptorSize,              // DescriptorSize
      PackageVersion,              // PackageVersion
      PackageVersionName           // PackageVersionName
      );

    if (PackageVersionName != NULL) {
      FreePool(PackageVersionName);
    }
    FreePool(FmpImageInfoBuf);

    //
    // Get package info
    //
    PackageVersionName = NULL;
    Status = Fmp->GetPackageInfo (
                    Fmp,
                    &PackageVersion,              // PackageVersion
                    &PackageVersionName,          // PackageVersionName
                    &PackageVersionNameMaxLen,    // PackageVersionNameMaxLen
                    &AttributesSupported,         // AttributesSupported
                    &AttributesSetting            // AttributesSetting
                    );
    if (EFI_ERROR(Status)) {
      Print(L"FMP (%d) PackageInfo - %r\n", Index, Status);
    } else {
      Print(L"FMP (%d) ImageInfo:\n", Index);
      DumpFmpPackageInfo(
        PackageVersion,              // PackageVersion
        PackageVersionName,          // PackageVersionName
        PackageVersionNameMaxLen,    // PackageVersionNameMaxLen
        AttributesSupported,         // AttributesSupported
        AttributesSetting            // AttributesSetting
        );

      if (PackageVersionName != NULL) {
        FreePool(PackageVersionName);
      }
    }
  }
  Print(L"\n");

EXIT:
  FreePool(HandleBuffer);
}

VOID
DumpWindowsUxCapsule (
  IN EFI_DISPLAY_CAPSULE                *DisplayCapsule
  )
{
  DISPLAY_DISPLAY_PAYLOAD  *ImagePayload;

  ImagePayload = &DisplayCapsule->ImagePayload;
  Print(L"WindowsUxCapsule:\n");
  Print(L"  Version    - 0x%x\n", ImagePayload->Version);
  Print(L"  Checksum   - 0x%x\n", ImagePayload->Checksum);
  Print(L"  ImageType  - 0x%x\n", ImagePayload->ImageType);
  Print(L"  Mode       - 0x%x\n", ImagePayload->Mode);
  Print(L"  OffsetX    - 0x%x\n", ImagePayload->OffsetX);
  Print(L"  OffsetY    - 0x%x\n", ImagePayload->OffsetY);
}

VOID
DumpFmpCapsule(
  IN EFI_CAPSULE_HEADER                *CapsuleHeader
  )
{
  EFI_FIRMWARE_MANAGEMENT_CAPSULE_HEADER        *FmpCapsuleHeader;
  EFI_FIRMWARE_MANAGEMENT_CAPSULE_IMAGE_HEADER  *ImageHeader;
  UINTN                                         Index;
  UINT64                                        *ItemOffsetList;

  FmpCapsuleHeader = (EFI_FIRMWARE_MANAGEMENT_CAPSULE_HEADER *)((UINT8 *)CapsuleHeader + CapsuleHeader->HeaderSize);

  Print(L"FmpCapsule:\n");
  Print(L"  Version                - 0x%x\n", FmpCapsuleHeader->Version);
  Print(L"  EmbeddedDriverCount    - 0x%x\n", FmpCapsuleHeader->EmbeddedDriverCount);
  Print(L"  PayloadItemCount       - 0x%x\n", FmpCapsuleHeader->PayloadItemCount);

  ItemOffsetList = (UINT64 *)(FmpCapsuleHeader + 1);
  for (Index = 0; Index < FmpCapsuleHeader->EmbeddedDriverCount; Index++) {
    Print(L"  ItemOffsetList[%d]      - 0x%lx\n", Index, ItemOffsetList[Index]);
  }
  for (; Index < (UINTN)(FmpCapsuleHeader->EmbeddedDriverCount + FmpCapsuleHeader->PayloadItemCount); Index++) {
    Print(L"  ItemOffsetList[%d]      - 0x%lx\n", Index, ItemOffsetList[Index]);
    ImageHeader = (EFI_FIRMWARE_MANAGEMENT_CAPSULE_IMAGE_HEADER *)((UINT8 *)FmpCapsuleHeader + ItemOffsetList[Index]);

    Print(L"  ImageHeader:\n");
    Print(L"    Version                - 0x%x\n", ImageHeader->Version);
    Print(L"    UpdateImageTypeId      - %g\n", &ImageHeader->UpdateImageTypeId);
    Print(L"    UpdateImageIndex       - 0x%x\n", ImageHeader->UpdateImageIndex);
    Print(L"    UpdateImageSize        - 0x%x\n", ImageHeader->UpdateImageSize);
    Print(L"    UpdateVendorCodeSize   - 0x%x\n", ImageHeader->UpdateVendorCodeSize);
    if (ImageHeader->Version >= EFI_FIRMWARE_MANAGEMENT_CAPSULE_IMAGE_HEADER_INIT_VERSION) {
      Print(L"    UpdateHardwareInstance - 0x%lx\n", ImageHeader->UpdateHardwareInstance);
    }
  }
}

CHAR8 *
GetCapsuleName(
  IN EFI_GUID *Guid
  )
{
  EFI_STATUS                                    Status;
  ESRT_MANAGEMENT_PROTOCOL                      *EsrtProtocol;
  EFI_SYSTEM_RESOURCE_ENTRY                     EsrtEntry;

  if (CompareGuid(&gWindowsUxCapsuleGuid, Guid)) {
    return "Windows UX Capsule";
  } else if (CompareGuid(&gEfiFmpCapsuleGuid, Guid)) {
    return "UEFI FMP Capsule";
  } else {
    Status = gBS->LocateProtocol(&gEsrtManagementProtocolGuid, NULL, (VOID **)&EsrtProtocol);
    if (!EFI_ERROR(Status)) {
      Status = EsrtProtocol->GetEsrtEntry(Guid, &EsrtEntry);
      if (!EFI_ERROR(Status)) {
        return "RSRT Capsule (unknown format)";
      }
    }
  }
  return "Unknown Capsule";
}

VOID
DumpCapsuleVolumeHob(
  IN EFI_HOB_UEFI_CAPSULE                *Capsule
  )
{
  EFI_CAPSULE_HEADER          *CapsuleHeader;

  Print(L"######################\n");
  Print(L"# CAPSULE VOLUME HOB #\n");
  Print(L"######################\n");

  CapsuleHeader = (VOID *)(UINTN)Capsule->BaseAddress;
  Print(L"  CapsuleGuid              - %g (%a)\n", &CapsuleHeader->CapsuleGuid, GetCapsuleName(&CapsuleHeader->CapsuleGuid));
  Print(L"  HeaderSize               - 0x%x\n", CapsuleHeader->HeaderSize);
  Print(L"  Flags                    - 0x%x\n", CapsuleHeader->Flags);
  Print(L"    PERSIST_ACROSS_RESET   - 0x%x\n", CapsuleHeader->Flags & CAPSULE_FLAGS_PERSIST_ACROSS_RESET);
  Print(L"    POPULATE_SYSTEM_TABLE  - 0x%x\n", CapsuleHeader->Flags & CAPSULE_FLAGS_POPULATE_SYSTEM_TABLE);
  Print(L"    INITIATE_RESET         - 0x%x\n", CapsuleHeader->Flags & CAPSULE_FLAGS_INITIATE_RESET);
  Print(L"  CapsuleImageSize         - 0x%x\n", CapsuleHeader->CapsuleImageSize);

  if (CompareGuid(&gWindowsUxCapsuleGuid, &CapsuleHeader->CapsuleGuid)) {
    DumpWindowsUxCapsule ((EFI_DISPLAY_CAPSULE *)CapsuleHeader);
  } else if (CompareGuid(&gEfiFmpCapsuleGuid, &CapsuleHeader->CapsuleGuid)) {
    DumpFmpCapsule (CapsuleHeader);
  }
  Print(L"\n");
}

VOID
DumpCapsuleVolumeHobData (
  VOID
  )
{
  EFI_PEI_HOB_POINTERS        HobPointer;
  UINTN                       Index;

  HobPointer.Raw = GetHobList();
  Index = 0;
  while ((HobPointer.Raw = GetNextHob(EFI_HOB_TYPE_UEFI_CAPSULE, HobPointer.Raw)) != NULL) {
    Print(L"CapsuleVolumeHob (%d): (0x%lx - 0x%lx)\n", Index, HobPointer.Capsule->BaseAddress, HobPointer.Capsule->Length);
    DumpCapsuleVolumeHob (HobPointer.Capsule);
    HobPointer.Raw = GET_NEXT_HOB(HobPointer);
    Index++;
  }
}

VOID
DumpEsrtManagementData(
  VOID
  )
{
  EFI_STATUS                                    Status;
  ESRT_MANAGEMENT_PROTOCOL                      *EsrtProtocol;
  EFI_SYSTEM_RESOURCE_ENTRY                     Entry;
  EFI_SYSTEM_RESOURCE_ENTRY                     *EsrtEntry;
  EFI_SYSTEM_RESOURCE_TABLE                     *Esrt;
  UINTN                                         Index;

  Print(L"##############################\n");
  Print(L"# EDKII ESRT MANAGEMENT DATA #\n");
  Print(L"##############################\n");

  Status = gBS->LocateProtocol(&gEsrtManagementProtocolGuid, NULL, (VOID **)&EsrtProtocol);
  if (EFI_ERROR(Status)) {
    Print (L"EsrtManagementProtocol - %r\n", EsrtProtocol);
    return;
  }

  Status = EfiGetSystemConfigurationTable(&gEfiSystemResourceTableGuid, (VOID **)&Esrt);
  if (EFI_ERROR(Status)) {
    return;
  }

  Print(L"ESRT from EsrtManagementProtocol\n");

  EsrtEntry = (VOID *)(Esrt + 1);
  for (Index = 0; Index < Esrt->FwResourceCount; Index++) {
    Print(L"EFI_SYSTEM_RESOURCE_ENTRY (%d):\n", Index);
    Status = EsrtProtocol->GetEsrtEntry(&EsrtEntry->FwClass, &Entry);
    if (!EFI_ERROR(Status)) {
      DumpEsrtEntry(&Entry);
    }
    EsrtEntry++;
  }
  Print(L"\n");
}

VOID
DumpEsrtRepository(
  IN EFI_SYSTEM_RESOURCE_ENTRY  *EsrtRepository,
  IN UINTN                      RepositorySize
  )
{
  UINTN  EsrtNum;
  UINTN  Index;

  if (RepositorySize % sizeof(EFI_SYSTEM_RESOURCE_ENTRY) != 0) {
    Print (L"EsrtRepository Corrupt!!!\n");
    return;
  }
  EsrtNum = RepositorySize / sizeof(EFI_SYSTEM_RESOURCE_ENTRY);
  for (Index = 0; Index < EsrtNum; Index++) {
    Print(L"EFI_SYSTEM_RESOURCE_ENTRY (%d):\n", Index);
    DumpEsrtEntry(&EsrtRepository[Index]);
  }
}

VOID
DumpEsrtDriverPrivateData(
  VOID
  )
{
  EFI_STATUS                 Status;
  EFI_SYSTEM_RESOURCE_ENTRY  *EsrtRepository;
  UINTN                      RepositorySize;

  Print(L"##################################\n");
  Print(L"# EDKII ESRT DRIVER PRIVATE DATA #\n");
  Print(L"##################################\n");

  Status = GetVariable2 (
             EFI_ESRT_FMP_VARIABLE_NAME,
             &mEsrtDxeDriverGuid,
             (VOID **) &EsrtRepository,
             &RepositorySize
             );
  if (EFI_ERROR(Status)) {
    Print(L"ESRT_FMP_VARIABLE - %r\n", Status);
  } else {
    Print(L"ESRT_FMP_VARIABLE: (Size - 0x%x)\n", RepositorySize);
    DumpEsrtRepository(EsrtRepository, RepositorySize);
  }

  Status = GetVariable2 (
             EFI_ESRT_NONFMP_VARIABLE_NAME,
             &mEsrtDxeDriverGuid,
             (VOID **) &EsrtRepository,
             &RepositorySize
             );
  if (EFI_ERROR(Status)) {
    Print(L"ESRT_NONFMP_VARIABLE - %r\n", Status);
  } else {
    Print(L"ESRT_NONFMP_VARIABLE: (Size - 0x%x)\n", RepositorySize);
    DumpEsrtRepository(EsrtRepository, RepositorySize);
  }
  Print(L"\n");
}

EFI_STATUS
EFIAPI
EsrtFmpDumpEntrypoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  DumpFmpData ();

  DumpCapsuleVolumeHobData ();

  DumpEsrtAcpiData ();

  DumpEsrtManagementData ();

  DumpEsrtDriverPrivateData();

  return EFI_SUCCESS;
}
