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
#include <Library/DevicePathLib.h>
#include <Library/DxeServicesLib.h>
#include <Protocol/LoadedImage.h>

#include "SmmProfile.h"

#define UNKNOWN_NAME  L"???"

#define NAME_STRING_LENGTH  35
typedef struct {
  EFI_GUID FileGuid;
  UINTN    EntryPoint;
  UINTN    ImageBase;
  UINTN    ImageSize;
  UINTN    LoadedImageBase;
  CHAR16   NameString[NAME_STRING_LENGTH + 1];
} IMAGE_STRUCT;

CHAR16 mNameString[NAME_STRING_LENGTH + 1];

IMAGE_STRUCT  *mImageStruct;
UINTN         mImageStructCountMax;
UINTN         mImageStructCount;

EFI_GUID  mSmmProfileGuid = SMM_PROFILE_GUID;

EFI_MEMORY_DESCRIPTOR *mMemoryMap;
UINTN                 mMemoryMapSize;
UINTN                 mDescriptorSize;

CHAR16 *mMemoryTypeShortName[] = {
  L"Reserved",
  L"LoaderCode",
  L"LoaderData",
  L"BS_Code",
  L"BS_Data",
  L"RT_Code",
  L"RT_Data",
  L"Available",
  L"Unusable",
  L"ACPI_Recl",
  L"ACPI_NVS",
  L"MMIO",
  L"MMIO_Port",
  L"PalCode",
  L"Persistent",
};

CHAR16 mUnknownStr[11];

CHAR16 *
ShortNameOfMemoryType(
  IN UINT32 Type
  )
{
  if (Type < sizeof(mMemoryTypeShortName) / sizeof(mMemoryTypeShortName[0])) {
    return mMemoryTypeShortName[Type];
  } else {
    UnicodeSPrint(mUnknownStr, sizeof(mUnknownStr), L"%08x", Type);
    return mUnknownStr;
  }
}

/** 
  Get a human readable name for an image.
  The following methods will be tried orderly:
    2. FFS UI section
    3. Image GUID

  @param[in]  LoadedImage LoadedImage protocol.
  @param[out] Guid        Guid of the FFS

  @post The resulting Unicode name string is stored in the mNameString global array.

**/
CHAR16 *
GetDriverNameString (
 IN  EFI_LOADED_IMAGE_PROTOCOL  *LoadedImage,
 OUT EFI_GUID                   *Guid
 )
{
  EFI_STATUS                  Status;
  CHAR16                      *NameString;
  UINTN                       StringSize;
  EFI_GUID                    *FileName;

  FileName = NULL;
  if ((DevicePathType(LoadedImage->FilePath) == MEDIA_DEVICE_PATH) &&
      (DevicePathSubType(LoadedImage->FilePath) == MEDIA_PIWG_FW_FILE_DP)) {
    FileName = &((MEDIA_FW_VOL_FILEPATH_DEVICE_PATH *)LoadedImage->FilePath)->FvFileName;
  }
  if (FileName != NULL) {
    CopyGuid(Guid, FileName);
  } else {
    ZeroMem(Guid, sizeof(EFI_GUID));
  }

  if (FileName != NULL) {
    //
    // Try to get the image's FFS UI section by image GUID
    //
    NameString = NULL;
    StringSize = 0;
    Status = GetSectionFromAnyFv (
               FileName,
               EFI_SECTION_USER_INTERFACE,
               0,
               (VOID **) &NameString,
               &StringSize
               );
    if (!EFI_ERROR (Status)) {
      //
      // Method 2: Get the name string from FFS UI section
      //
      StrnCpyS (mNameString, NAME_STRING_LENGTH + 1, NameString, NAME_STRING_LENGTH);
      mNameString[NAME_STRING_LENGTH] = 0;
      FreePool (NameString);
      return mNameString;
    }

    //
    // Method 3: Get the name string from image GUID
    //
    UnicodeSPrint (mNameString, sizeof (mNameString), L"%g", FileName);
    return mNameString;
  }

  if ((DevicePathType(LoadedImage->FilePath) == MEDIA_DEVICE_PATH) &&
    (DevicePathSubType(LoadedImage->FilePath) == MEDIA_FILEPATH_DP)) {
    return ((FILEPATH_DEVICE_PATH *)LoadedImage->FilePath)->PathName;
  }
  return NULL;
}

VOID
AddImageStruct(
  IN UINTN     ImageBase,
  IN UINTN     ImageSize,
  IN UINTN     LoadedImageBase,
  IN UINTN     EntryPoint,
  IN CHAR16    *NameString,
  IN EFI_GUID  *Guid
  )
{
  if (mImageStructCount >= mImageStructCountMax) {
    ASSERT(FALSE);
    return;
  }

  mImageStruct[mImageStructCount].ImageBase = ImageBase;
  mImageStruct[mImageStructCount].ImageSize = ImageSize;
  mImageStruct[mImageStructCount].LoadedImageBase = LoadedImageBase;
  mImageStruct[mImageStructCount].EntryPoint = EntryPoint;
  if (NameString != NULL) {
    StrnCpyS(mImageStruct[mImageStructCount].NameString, NAME_STRING_LENGTH + 1, NameString, NAME_STRING_LENGTH);
  }
  CopyGuid(&mImageStruct[mImageStructCount].FileGuid, Guid);

  mImageStructCount++;
}

CHAR16 *
AddressToImageName(
  IN UINTN  Address
  )
{
  UINTN  Index;

  for (Index = 0; Index < mImageStructCount; Index++) {
    if ((Address >= mImageStruct[Index].ImageBase) &&
      (Address < mImageStruct[Index].ImageBase + mImageStruct[Index].ImageSize)) {
      return mImageStruct[Index].NameString;
    }
  }
  return UNKNOWN_NAME;
}

VOID
GetLoadedImage(
  VOID
  )
{
  EFI_STATUS                 Status;
  UINTN                      NoHandles;
  EFI_HANDLE                 *HandleBuffer;
  UINTN                      Index;
  EFI_LOADED_IMAGE_PROTOCOL  *LoadedImage;
  CHAR16                     *PathStr;
  CHAR16                     *NameStr;
  UINTN                      EntryPoint;
  UINTN                      RealImageBase;
  EFI_GUID                   Guid;

  Status = gBS->LocateHandleBuffer(
                  ByProtocol,
                  &gEfiLoadedImageProtocolGuid,
                  NULL,
                  &NoHandles,
                  &HandleBuffer
                  );
  if (EFI_ERROR(Status)) {
    return;
  }

  mImageStructCountMax = NoHandles;
  mImageStruct = AllocateZeroPool(mImageStructCountMax * sizeof(IMAGE_STRUCT));
  if (mImageStruct == NULL) {
    goto Done;
  }

  for (Index = 0; Index < NoHandles; Index++) {
    Status = gBS->HandleProtocol(
                    HandleBuffer[Index],
                    &gEfiLoadedImageProtocolGuid,
                    (VOID **)&LoadedImage
                    );
    if (EFI_ERROR(Status)) {
      continue;
    }

    PathStr = ConvertDevicePathToText(LoadedImage->FilePath, TRUE, TRUE);
    NameStr = GetDriverNameString(LoadedImage, &Guid);
    DEBUG((EFI_D_INFO, "Image: %s ", NameStr));

    EntryPoint = 0;
    RealImageBase = (UINTN)LoadedImage->ImageBase;

    DEBUG((EFI_D_INFO, "(0x%x - 0x%x", LoadedImage->ImageBase, (UINTN)LoadedImage->ImageSize));
    if (EntryPoint != 0) {
      DEBUG((EFI_D_INFO, ", EntryPoint:0x%x", EntryPoint));
    }
    if (RealImageBase != (UINTN)LoadedImage->ImageBase) {
      DEBUG((EFI_D_INFO, ", Base:0x%x", RealImageBase));
    }
    DEBUG((EFI_D_INFO, ")\n"));
    DEBUG((EFI_D_INFO, "       (%s)\n", PathStr));

    AddImageStruct(RealImageBase, (UINTN)LoadedImage->ImageSize, (UINTN)LoadedImage->ImageBase, EntryPoint, NameStr, &Guid);

  }

Done:
  gBS->FreePool(HandleBuffer);
  return;
}

VOID
RecrodMemoryMap(
  VOID
  )
{
  EFI_STATUS            Status;
  UINTN                 MapKey;
  UINTN                 MemoryMapSize;
  EFI_MEMORY_DESCRIPTOR *MemoryMap;
  UINTN                 DescriptorSize;
  UINT32                DescriptorVersion;

  MemoryMapSize = 0;
  MemoryMap = NULL;
  Status = gBS->GetMemoryMap (
               &MemoryMapSize,
               MemoryMap,
               &MapKey,
               &DescriptorSize,
               &DescriptorVersion
               );
  ASSERT (Status == EFI_BUFFER_TOO_SMALL);

  do {
    Status = gBS->AllocatePool (EfiBootServicesData, MemoryMapSize, (VOID **)&MemoryMap);
    ASSERT (MemoryMap != NULL);
  
    Status = gBS->GetMemoryMap (
                 &MemoryMapSize,
                 MemoryMap,
                 &MapKey,
                 &DescriptorSize,
                 &DescriptorVersion
                 );
    if (EFI_ERROR (Status)) {
      gBS->FreePool (MemoryMap);
    }
  } while (Status == EFI_BUFFER_TOO_SMALL);

  mMemoryMap = MemoryMap;
  mMemoryMapSize = MemoryMapSize;
  mDescriptorSize = DescriptorSize;
}

EFI_MEMORY_TYPE
GetMemoryTypeFromAddress(
  IN UINT64  Address
  )
{
  UINTN                 MemoryMapSize;
  EFI_MEMORY_DESCRIPTOR *MemoryMap;
  UINTN                 MemoryMapEntryCount;
  UINTN                 DescriptorSize;
  UINTN                 Index;

  MemoryMap = mMemoryMap;
  MemoryMapSize = mMemoryMapSize;
  DescriptorSize = mDescriptorSize;
  MemoryMapEntryCount = MemoryMapSize / DescriptorSize;
  for (Index = 0; Index < MemoryMapEntryCount; Index++) {
    if ((Address >= MemoryMap->PhysicalStart) &&
        (Address < MemoryMap->PhysicalStart + LShiftU64(MemoryMap->NumberOfPages, EFI_PAGE_SHIFT))) {
      return MemoryMap->Type;
    }
    MemoryMap = NEXT_MEMORY_DESCRIPTOR(MemoryMap, DescriptorSize);
  }

  //
  // BUGBUG: Assume MMIO, should get GCD to check.
  //
  return EfiMemoryMappedIO;
}

VOID
DumpSmmProfileHeader (
  IN SMM_PROFILE_HEADER      *SmmProfileHeader
  )
{
  Print(L"  HeaderSize     - 0x%016lx\n", SmmProfileHeader->HeaderSize);
  Print(L"  MaxDataEntries - 0x%016lx\n", SmmProfileHeader->MaxDataEntries);
  Print(L"  MaxDataSize    - 0x%016lx\n", SmmProfileHeader->MaxDataSize);
  Print(L"  CurDataEntries - 0x%016lx\n", SmmProfileHeader->CurDataEntries);
  Print(L"  CurDataSize    - 0x%016lx\n", SmmProfileHeader->CurDataSize);
  Print(L"  TsegStart      - 0x%016lx\n", SmmProfileHeader->TsegStart);
  Print(L"  TsegSize       - 0x%016lx\n", SmmProfileHeader->TsegSize);
  Print(L"  NumSmis        - 0x%016lx\n", SmmProfileHeader->NumSmis);
  Print(L"  NumCpus        - 0x%016lx\n", SmmProfileHeader->NumCpus);
}

VOID
DumpSmmProfileEntry (
  IN SMM_PROFILE_ENTRY      *SmmProfileEntry
  )
{
  CHAR16           *NameString;
  EFI_MEMORY_TYPE  MemoryType;

  Print(L"  SmiNum         - 0x%016lx\n", SmmProfileEntry->SmiNum);
  Print(L"  CpuNum         - 0x%016lx\n", SmmProfileEntry->CpuNum);
  Print(L"  ApicId         - 0x%016lx\n", SmmProfileEntry->ApicId);
  Print(L"  ErrorCode      - 0x%016lx\n", SmmProfileEntry->ErrorCode);
  Print(L"  Instruction    - 0x%016lx", SmmProfileEntry->Instruction);
  NameString = AddressToImageName((UINTN)SmmProfileEntry->Instruction);
  if (NameString != NULL) {
    Print(L" (%s)", NameString);
  }
  Print(L"\n");
  Print(L"  Address        - 0x%016lx", SmmProfileEntry->Address);
  MemoryType = GetMemoryTypeFromAddress(SmmProfileEntry->Address);
  NameString = ShortNameOfMemoryType(MemoryType);
  if (NameString != NULL) {
    Print(L" (%s)", NameString);
  }
  Print(L"\n");
  Print(L"  SmiCmd         - 0x%016lx\n", SmmProfileEntry->SmiCmd);
}

VOID
DumpSmmProfileInformation (
  IN SMM_PROFILE_HEADER      *SmmProfileBase
  )
{
  SMM_PROFILE_ENTRY      *SmmProfileEntry;
  UINTN                  Index;

  Print(L"SMM_PROFILE_HEADER\n");
  DumpSmmProfileHeader(SmmProfileBase);
  SmmProfileEntry = (SMM_PROFILE_ENTRY *)(UINTN)(SmmProfileBase + 1);

  for (Index = 0; Index < (UINTN)SmmProfileBase->CurDataEntries; Index++) {
    Print(L"SMM_PROFILE_ENTRY[%d]\n", Index);
    DumpSmmProfileEntry(&SmmProfileEntry[Index]);
  }
  return;
}

EFI_STATUS
EFIAPI
SmmProfileDumpEntrypoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS              Status;
  UINTN                   Size;
  SMM_PROFILE_HEADER      *SmmProfileBase;

  GetLoadedImage();
  RecrodMemoryMap();

  Size = sizeof(SmmProfileBase);
  Status = gRT->GetVariable (
                  SMM_PROFILE_NAME,
                  &mSmmProfileGuid,
                  NULL,
                  &Size,
                  &SmmProfileBase
                  );
  if (!EFI_ERROR(Status)) {
    DumpSmmProfileInformation (SmmProfileBase);
  } else {
    Print (L"SmmProfile not found!\n");
  }

  if (mMemoryMap != NULL) {
    FreePool(mMemoryMap);
  }
  if (mImageStruct != NULL) {
    FreePool(mImageStruct);
  }

  return EFI_SUCCESS;
}
