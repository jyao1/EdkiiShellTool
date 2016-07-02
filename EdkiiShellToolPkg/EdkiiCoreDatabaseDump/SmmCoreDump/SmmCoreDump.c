/** @file

Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <PiSmm.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/SmmServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiLib.h>
#include <Library/DevicePathLib.h>
#include <Library/PeCoffGetEntryPointLib.h>
#include <Library/DxeServicesLib.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/SmmAccess2.h>
#include <Protocol/SmmReadyToLock.h>
#include <Protocol/SmmEndOfDxe.h>

#include <Protocol/SmmStatusCode.h>
#include <Protocol/SmmCpu.h>
#include <Protocol/SmmCpuIo2.h>
#include <Protocol/SmmPciRootBridgeIo.h>
#include <Protocol/SmmSwDispatch2.h>
#include <Protocol/SmmSxDispatch2.h>
#include <Protocol/SmmPeriodicTimerDispatch2.h>
#include <Protocol/SmmUsbDispatch2.h>
#include <Protocol/SmmGpiDispatch2.h>
#include <Protocol/SmmStandbyButtonDispatch2.h>
#include <Protocol/SmmPowerButtonDispatch2.h>
#include <Protocol/SmmIoTrapDispatch2.h>
#include <Protocol/SmmReportStatusCodeHandler.h>

#include <Library/SmmChildDumpLib.h>
#include "SmmCoreDump.h"

//
// This structure is copied from EDKII SmmCore
//
#include "SmmCore/SmmCore.h"
#include "SmmCore/PiSmmCore.h"
#include "SmmCore/Smi.h"

typedef struct {
  EFI_GUID   *Guid;
  UINTN      FuncOffset;
} PROTOCOL_FUNC_STRUCT;

PROTOCOL_FUNC_STRUCT  mProtocolFuncStruct[] = {
  // 4 SMM Protocols
  { &gEfiSmmStatusCodeProtocolGuid, OFFSET_OF(EFI_SMM_STATUS_CODE_PROTOCOL, ReportStatusCode) },
  { &gEfiSmmCpuProtocolGuid, OFFSET_OF(EFI_SMM_CPU_PROTOCOL, ReadSaveState) },
  { &gEfiSmmCpuIo2ProtocolGuid, OFFSET_OF(EFI_SMM_CPU_IO2_PROTOCOL, Mem) },
  { &gEfiSmmPciRootBridgeIoProtocolGuid, OFFSET_OF(EFI_SMM_PCI_ROOT_BRIDGE_IO_PROTOCOL, PollMem) },
  // 6. SMM Child Dispatch Protocols
  { &gEfiSmmSwDispatch2ProtocolGuid, OFFSET_OF(EFI_SMM_SW_DISPATCH2_PROTOCOL, Register) },
  { &gEfiSmmSxDispatch2ProtocolGuid, OFFSET_OF(EFI_SMM_SX_DISPATCH2_PROTOCOL, Register) },
  { &gEfiSmmPeriodicTimerDispatch2ProtocolGuid, OFFSET_OF(EFI_SMM_PERIODIC_TIMER_DISPATCH2_PROTOCOL, Register) },
  { &gEfiSmmUsbDispatch2ProtocolGuid, OFFSET_OF(EFI_SMM_USB_DISPATCH2_PROTOCOL, Register) },
  { &gEfiSmmGpiDispatch2ProtocolGuid, OFFSET_OF(EFI_SMM_GPI_DISPATCH2_PROTOCOL, Register) },
  { &gEfiSmmStandbyButtonDispatch2ProtocolGuid, OFFSET_OF(EFI_SMM_STANDBY_BUTTON_DISPATCH2_PROTOCOL, Register) },
  { &gEfiSmmPowerButtonDispatch2ProtocolGuid, OFFSET_OF(EFI_SMM_GPI_DISPATCH2_PROTOCOL, Register) },
  { &gEfiSmmIoTrapDispatch2ProtocolGuid, OFFSET_OF(EFI_SMM_IO_TRAP_DISPATCH2_PROTOCOL, Register) },
  // Vol 3
  { &gEfiSmmRscHandlerProtocolGuid, OFFSET_OF(EFI_SMM_RSC_HANDLER_PROTOCOL, Register) },
};

MEDIA_FW_VOL_FILEPATH_DEVICE_PATH  mSmmCoreFilePath = {
  { MEDIA_DEVICE_PATH, MEDIA_PIWG_FW_FILE_DP, {sizeof(MEDIA_FW_VOL_FILEPATH_DEVICE_PATH), 0} },
  SMM_CORE_GUID,
};

EFI_LOADED_IMAGE_PROTOCOL  *mSmmCoreLoadedImage;

LIST_ENTRY      *mSmmCoreHandleList;
LIST_ENTRY      *mSmmCoreProtocolDatabase;
LIST_ENTRY      *mSmmCoreRootSmiHandlerList;
LIST_ENTRY      *mSmmCoreSmiEntryList;

CHAR16 mNameString[NAME_STRING_LENGTH + 1];

IMAGE_STRUCT  *mImageStruct;
UINTN         mImageStructCountMax;
UINTN         mImageStructCount;


EFI_SMRAM_DESCRIPTOR *mSmmCoreDumpInternalSmramRanges;
UINTN                mSmmCoreDumpInternalSmramCount;

BOOLEAN
EFIAPI
IsSmram(
  IN EFI_PHYSICAL_ADDRESS  Buffer,
  IN UINT64                Length
  )
{
  UINTN  Index;

  for (Index = 0; Index < mSmmCoreDumpInternalSmramCount; Index++) {
    if (((Buffer >= mSmmCoreDumpInternalSmramRanges[Index].CpuStart) && (Buffer < mSmmCoreDumpInternalSmramRanges[Index].CpuStart + mSmmCoreDumpInternalSmramRanges[Index].PhysicalSize)) ||
      ((mSmmCoreDumpInternalSmramRanges[Index].CpuStart >= Buffer) && (mSmmCoreDumpInternalSmramRanges[Index].CpuStart < Buffer + Length))) {
      return TRUE;
    }
  }

  return FALSE;
}

EFI_STATUS
ReadFileToBuffer(
  IN  CHAR16                               *FileName,
  OUT UINTN                                *BufferSize,
  OUT VOID                                 **Buffer
  )
{
  return EFI_UNSUPPORTED;
}

/** 
  Get the file name portion of the Pdb File Name.
  
  The portion of the Pdb File Name between the last backslash and
  either a following period or the end of the string is converted
  to Unicode and copied into UnicodeBuffer.  The name is truncated,
  if necessary, to ensure that UnicodeBuffer is not overrun.
  
  @param[in]  PdbFileName     Pdb file name.
  @param[out] UnicodeBuffer   The resultant Unicode File Name.
  
**/
VOID
GetShortPdbFileName (
  IN  CHAR8     *PdbFileName,
  OUT CHAR16    *UnicodeBuffer
  )
{
  UINTN IndexA;     // Current work location within an ASCII string.
  UINTN IndexU;     // Current work location within a Unicode string.
  UINTN StartIndex;
  UINTN EndIndex;

  ZeroMem (UnicodeBuffer, (NAME_STRING_LENGTH + 1) * sizeof (CHAR16));

  if (PdbFileName == NULL) {
    StrnCpyS (UnicodeBuffer, NAME_STRING_LENGTH + 1, L" ", 1);
  } else {
    StartIndex = 0;
    for (EndIndex = 0; PdbFileName[EndIndex] != 0; EndIndex++);
    for (IndexA = 0; PdbFileName[IndexA] != 0; IndexA++) {
      if (PdbFileName[IndexA] == '\\') {
        StartIndex = IndexA + 1;
      }

      if (PdbFileName[IndexA] == '.') {
        EndIndex = IndexA;
      }
    }

    IndexU = 0;
    for (IndexA = StartIndex; IndexA < EndIndex; IndexA++) {
      UnicodeBuffer[IndexU] = (CHAR16) PdbFileName[IndexA];
      IndexU++;
      if (IndexU >= NAME_STRING_LENGTH) {
        UnicodeBuffer[NAME_STRING_LENGTH] = 0;
        break;
      }
    }
  }
}

/** 
  Get a human readable name for an image.
  The following methods will be tried orderly:
    1. Image PDB
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
  CHAR8                       *PdbFileName;
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

  //
  // Method 1: Get the name string from image PDB
  //
  if (LoadedImage->ImageBase != 0) {
    PdbFileName = PeCoffLoaderGetPdbPointer ((VOID *) (UINTN)LoadedImage->ImageBase);
    if (PdbFileName != NULL) {
      GetShortPdbFileName (PdbFileName, mNameString);
      return mNameString;
    }
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

/**
  Retrieves and returns a pointer to the entry point to a PE/COFF image that has been loaded
  into system memory with the PE/COFF Loader Library functions.

  Retrieves the entry point to the PE/COFF image specified by Pe32Data and returns this entry
  point in EntryPoint.  If the entry point could not be retrieved from the PE/COFF image, then
  return RETURN_INVALID_PARAMETER.  Otherwise return RETURN_SUCCESS.
  If Pe32Data is NULL, then ASSERT().
  If EntryPoint is NULL, then ASSERT().

  @param  Pe32Data                  The pointer to the PE/COFF image that is loaded in system memory.
  @param  EntryPoint                The pointer to entry point to the PE/COFF image to return.

  @retval RETURN_SUCCESS            EntryPoint was returned.
  @retval RETURN_INVALID_PARAMETER  The entry point could not be found in the PE/COFF image.

**/
RETURN_STATUS
InternalPeCoffGetEntryPoint (
  IN  VOID  *Pe32Data,
  OUT VOID  **EntryPoint
  )
{
  EFI_IMAGE_DOS_HEADER                  *DosHdr;
  EFI_IMAGE_OPTIONAL_HEADER_PTR_UNION   Hdr;

  ASSERT (Pe32Data   != NULL);
  ASSERT (EntryPoint != NULL);

  DosHdr = (EFI_IMAGE_DOS_HEADER *) Pe32Data;
  if (DosHdr->e_magic == EFI_IMAGE_DOS_SIGNATURE) {
    //
    // DOS image header is present, so read the PE header after the DOS image header.
    //
    Hdr.Pe32 = (EFI_IMAGE_NT_HEADERS32 *) ((UINTN) Pe32Data + (UINTN) ((DosHdr->e_lfanew) & 0x0ffff));
  } else {
    //
    // DOS image header is not present, so PE header is at the image base.
    //
    Hdr.Pe32 = (EFI_IMAGE_NT_HEADERS32 *) Pe32Data;
  }

  //
  // Calculate the entry point relative to the start of the image.
  // AddressOfEntryPoint is common for PE32 & PE32+
  //
  if (Hdr.Te->Signature == EFI_TE_IMAGE_HEADER_SIGNATURE) {
    *EntryPoint = (VOID *) ((UINTN) Pe32Data + (UINTN) (Hdr.Te->AddressOfEntryPoint & 0x0ffffffff) + sizeof (EFI_TE_IMAGE_HEADER) - Hdr.Te->StrippedSize);
    return RETURN_SUCCESS;
  } else if (Hdr.Pe32->Signature == EFI_IMAGE_NT_SIGNATURE) {
    *EntryPoint = (VOID *) ((UINTN) Pe32Data + (UINTN) (Hdr.Pe32->OptionalHeader.AddressOfEntryPoint & 0x0ffffffff));
    return RETURN_SUCCESS;
  }

  return RETURN_UNSUPPORTED;
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
  StrnCpyS (mImageStruct[mImageStructCount].NameString, NAME_STRING_LENGTH + 1, NameString, NAME_STRING_LENGTH);
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

CHAR16 *
AddressToImageNameEx(
  IN UINTN     Address,
  IN EFI_GUID  *Protocol
  )
{
  CHAR16  *Name;
  UINTN   Index;
  Name = AddressToImageName(Address);
  if (StrCmp(Name, UNKNOWN_NAME) != 0) {
    return Name;
  }
  for (Index = 0; Index < sizeof(mProtocolFuncStruct) / sizeof(mProtocolFuncStruct[0]); Index++) {
    if (CompareGuid(Protocol, mProtocolFuncStruct[Index].Guid)) {
      return AddressToImageName(*(UINTN *)(Address + mProtocolFuncStruct[Index].FuncOffset));
    }
  }
  return UNKNOWN_NAME;
}

UINTN
AddressToImageRef(
  IN UINTN  Address
  )
{
  UINTN  Index;

  for (Index = 0; Index < mImageStructCount; Index++) {
    if ((Address >= mImageStruct[Index].ImageBase) &&
        (Address < mImageStruct[Index].ImageBase + mImageStruct[Index].ImageSize)) {
      return Index;
    }
  }
  return (UINTN)-1;
}

UINTN
AddressToImageRefEx(
  IN UINTN     Address,
  IN EFI_GUID  *Protocol
  )
{
  UINTN   ImageRef;
  UINTN   Index;
  ImageRef = AddressToImageRef(Address);
  if (ImageRef != (UINTN)-1) {
    return ImageRef;
  }
  for (Index = 0; Index < sizeof(mProtocolFuncStruct) / sizeof(mProtocolFuncStruct[0]); Index++) {
    if (CompareGuid(Protocol, mProtocolFuncStruct[Index].Guid)) {
      return AddressToImageRef(*(UINTN *)(Address + mProtocolFuncStruct[Index].FuncOffset));
    }
  }
  return (UINTN)-1;
}

VOID
GetSmmLoadedImage(
  VOID
  )
{
  EFI_STATUS                 Status;
  UINTN                      NoHandles;
  UINTN                      HandleBufferSize;
  EFI_HANDLE                 *HandleBuffer;
  UINTN                      Index;
  EFI_LOADED_IMAGE_PROTOCOL  *LoadedImage;
  CHAR16                     *PathStr;
  CHAR16                     *NameStr;
  EFI_SMM_DRIVER_ENTRY       *LoadedImagePrivate;
  UINTN                      EntryPoint;
  VOID                       *EntryPointInImage;
  UINTN                      RealImageBase;
  EFI_GUID                   Guid;

  HandleBufferSize = 0;
  HandleBuffer = NULL;
  Status = gSmst->SmmLocateHandle(
                    ByProtocol,
                    &gEfiLoadedImageProtocolGuid,
                    NULL,
                    &HandleBufferSize,
                    HandleBuffer
                    );
  if (Status != EFI_BUFFER_TOO_SMALL) {
    return;
  }
  HandleBuffer = AllocateZeroPool (HandleBufferSize);
  if (HandleBuffer == NULL) {
    return;
  }
  Status = gSmst->SmmLocateHandle(
                    ByProtocol,
                    &gEfiLoadedImageProtocolGuid,
                    NULL,
                    &HandleBufferSize,
                    HandleBuffer
                    );
  if (EFI_ERROR(Status)) {
    return;
  }

  NoHandles = HandleBufferSize/sizeof(EFI_HANDLE);
  mImageStructCountMax = NoHandles;
  mImageStruct = AllocateZeroPool(mImageStructCountMax * sizeof(IMAGE_STRUCT));
  if (mImageStruct == NULL) {
    goto Done;
  }

  for (Index = 0; Index < NoHandles; Index++) {
    Status = gSmst->SmmHandleProtocol(
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
    LoadedImagePrivate = BASE_CR(LoadedImage, EFI_SMM_DRIVER_ENTRY, SmmLoadedImage);
    if (LoadedImagePrivate->Signature == EFI_SMM_DRIVER_ENTRY_SIGNATURE) {
      EntryPoint = (UINTN)LoadedImagePrivate->ImageEntryPoint;
      if ((EntryPoint != 0) && ((EntryPoint < (UINTN)LoadedImage->ImageBase) || (EntryPoint >= ((UINTN)LoadedImage->ImageBase + (UINTN)LoadedImage->ImageSize)))) {
        //
        // If the EntryPoint is not in the range of image buffer, it should come from emulation environment.
        // So patch ImageBuffer here to align the EntryPoint.
        //
        Status = InternalPeCoffGetEntryPoint(LoadedImage->ImageBase, &EntryPointInImage);
        ASSERT_EFI_ERROR(Status);
        RealImageBase = (UINTN)LoadedImage->ImageBase + EntryPoint - (UINTN)EntryPointInImage;
      }
    }
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

    if (CompareMem(&mSmmCoreFilePath, LoadedImage->FilePath, sizeof(mSmmCoreFilePath)) == 0) {
      mSmmCoreLoadedImage = LoadedImage;
    }
  }

Done:
  FreePool(HandleBuffer);
  return;
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

    if (!IsSmram((UINT64)(UINTN)LoadedImage->ImageBase, LoadedImage->ImageSize)) {
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

    if (CompareMem(&mSmmCoreFilePath, LoadedImage->FilePath, sizeof(mSmmCoreFilePath)) == 0) {
      mSmmCoreLoadedImage = LoadedImage;
    }
  }

Done:
  gBS->FreePool(HandleBuffer);
  return;
}

VOID
DumpProtocolInterfacesOnHandle(
  IN IHANDLE    *IHandle
  )
{
  LIST_ENTRY           *ListEntry;
  PROTOCOL_INTERFACE   *ProtocolInterface;
  CHAR16               *PathStr;

  ListEntry = &IHandle->Protocols;
  for (ListEntry = ListEntry->ForwardLink;
       ListEntry != &IHandle->Protocols;
       ListEntry = ListEntry->ForwardLink) {
    ProtocolInterface = CR(ListEntry, PROTOCOL_INTERFACE, Link, PROTOCOL_INTERFACE_SIGNATURE);
    DEBUG((EFI_D_INFO, "  Protocol - %a, Interface - 0x%x", GuidToName(&ProtocolInterface->Protocol->ProtocolID), ProtocolInterface->Interface));
    if (ProtocolInterface->Interface != NULL) {
      CHAR16  *Name;
      Name = AddressToImageNameEx((UINTN)ProtocolInterface->Interface, &ProtocolInterface->Protocol->ProtocolID);
      if (StrCmp(Name, UNKNOWN_NAME) != 0) {
        DEBUG((EFI_D_INFO, " (%s)", Name));
      }
    }
    DEBUG((EFI_D_INFO, "\n"));
    if (CompareGuid(&ProtocolInterface->Protocol->ProtocolID, &gEfiDevicePathProtocolGuid)) {
      if (IsDevicePathValid(ProtocolInterface->Interface, SIZE_4KB)) {
        PathStr = ConvertDevicePathToText(ProtocolInterface->Interface, TRUE, TRUE);
        DEBUG((EFI_D_INFO, "    (%s)\n", PathStr));
      } else {
        DEBUG((EFI_D_INFO, "    (INVALID DEVICE PATH)\n"));
      }
    }
    if (CompareGuid(&ProtocolInterface->Protocol->ProtocolID, &gEfiLoadedImageProtocolGuid)) {
      PathStr = ConvertDevicePathToText(((EFI_LOADED_IMAGE_PROTOCOL *)ProtocolInterface->Interface)->FilePath, TRUE, TRUE);
      DEBUG((EFI_D_INFO, "    (%s)\n", PathStr));
    }
    if (CompareGuid(&ProtocolInterface->Protocol->ProtocolID, &gEfiLoadedImageDevicePathProtocolGuid)) {
      PathStr = ConvertDevicePathToText(ProtocolInterface->Interface, TRUE, TRUE);
      DEBUG((EFI_D_INFO, "    (%s)\n", PathStr));
    }
  }
}

VOID
DumpHandleList(
  VOID
  )
{
  LIST_ENTRY      *ListEntry;
  IHANDLE         *Handle;

  ListEntry = mSmmCoreHandleList;
  for (ListEntry = ListEntry->ForwardLink;
       ListEntry != mSmmCoreHandleList;
       ListEntry = ListEntry->ForwardLink) {
    Handle = CR(ListEntry, IHANDLE, AllHandles, EFI_HANDLE_SIGNATURE);
    DEBUG((EFI_D_INFO, "Handle - 0x%x\n", Handle));
    DumpProtocolInterfacesOnHandle(Handle);
  }

  return;
}

VOID
GetSmmCoreHandleList(
  IN IHANDLE    *IHandle
  )
{
  LIST_ENTRY      *ListEntry;
  IHANDLE         *Handle;

  ListEntry = &IHandle->AllHandles;
  for (ListEntry = ListEntry->ForwardLink;
       ListEntry != &IHandle->AllHandles;
       ListEntry = ListEntry->ForwardLink) {
    Handle = BASE_CR(ListEntry, IHANDLE, AllHandles);
    if (Handle->Signature != EFI_HANDLE_SIGNATURE) {
      // BUGBUG: NT32 will load image to BS memory, but execute code in DLL.
      // Do not use mSmmCoreLoadedImage->ImageBase/ImageSize
      mSmmCoreHandleList = ListEntry;
      break;
    }
  }

  return;
}

VOID
GetSmmCoreProtocolDatabase(
  IN IHANDLE    *IHandle
  )
{
  LIST_ENTRY      *ListEntry;
  IHANDLE         *Handle;
  PROTOCOL_ENTRY  *IProtocolEntry;
  PROTOCOL_ENTRY  *ProtocolEntry;

  IProtocolEntry = NULL;
  ListEntry = &IHandle->AllHandles;
  for (ListEntry = ListEntry->ForwardLink;
       ListEntry != &IHandle->AllHandles;
       ListEntry = ListEntry->ForwardLink) {
    Handle = BASE_CR(ListEntry, IHANDLE, AllHandles);
    if (Handle->Signature == EFI_HANDLE_SIGNATURE) {
      LIST_ENTRY           *ProtocolListEntry;
      PROTOCOL_INTERFACE   *ProtocolInterface;

      ProtocolListEntry = &Handle->Protocols;
      for (ProtocolListEntry = ProtocolListEntry->ForwardLink;
           ProtocolListEntry != &Handle->Protocols;
           ProtocolListEntry = ProtocolListEntry->ForwardLink) {
        ProtocolInterface = BASE_CR(ProtocolListEntry, PROTOCOL_INTERFACE, Link);
        if (ProtocolInterface->Signature == PROTOCOL_INTERFACE_SIGNATURE) {
          IProtocolEntry = ProtocolInterface->Protocol;
        }
        if (IProtocolEntry != NULL) {
          break;
        }
      }
      if (IProtocolEntry != NULL) {
        break;
      }
    }
  }

  if (IProtocolEntry == NULL) {
    return;
  }

  ListEntry = &IProtocolEntry->AllEntries;
  for (ListEntry = ListEntry->ForwardLink;
       ListEntry != &IProtocolEntry->AllEntries;
       ListEntry = ListEntry->ForwardLink) {
    ProtocolEntry = BASE_CR(ListEntry, PROTOCOL_ENTRY, AllEntries);
    if (ProtocolEntry->Signature != PROTOCOL_ENTRY_SIGNATURE) {
      // BUGBUG: NT32 will load image to BS memory, but execute code in DLL.
      // Do not use mSmmCoreLoadedImage->ImageBase/ImageSize
      mSmmCoreProtocolDatabase = ListEntry;
      break;
    }
  }

  return;
}

VOID
DumpProtocolInterfacesOnProtocolEntry(
  IN PROTOCOL_ENTRY  *ProtocolEntry
  )
{
  LIST_ENTRY           *ListEntry;
  PROTOCOL_INTERFACE   *ProtocolInterface;
  CHAR16               *PathStr;

  ListEntry = &ProtocolEntry->Protocols;
  for (ListEntry = ListEntry->ForwardLink;
       ListEntry != &ProtocolEntry->Protocols;
       ListEntry = ListEntry->ForwardLink) {
    ProtocolInterface = CR(ListEntry, PROTOCOL_INTERFACE, ByProtocol, PROTOCOL_INTERFACE_SIGNATURE);
    DEBUG((EFI_D_INFO, "  Interface - 0x%x", ProtocolInterface->Interface));
    if (ProtocolInterface->Interface != NULL) {
      CHAR16  *Name;
      Name = AddressToImageNameEx((UINTN)ProtocolInterface->Interface, &ProtocolInterface->Protocol->ProtocolID);
      if (StrCmp(Name, UNKNOWN_NAME) != 0) {
        DEBUG((EFI_D_INFO, " (%s)", Name));
      }
    }
    DEBUG((EFI_D_INFO, "\n"));
    if (CompareGuid(&ProtocolInterface->Protocol->ProtocolID, &gEfiDevicePathProtocolGuid)) {
      if (IsDevicePathValid(ProtocolInterface->Interface, SIZE_4KB)) {
        PathStr = ConvertDevicePathToText(ProtocolInterface->Interface, TRUE, TRUE);
        DEBUG((EFI_D_INFO, "    (%s)\n", PathStr));
      } else {
        DEBUG((EFI_D_INFO, "    (INVALID DEVICE PATH)\n"));
      }
    }
    if (CompareGuid(&ProtocolInterface->Protocol->ProtocolID, &gEfiLoadedImageProtocolGuid)) {
      PathStr = ConvertDevicePathToText(((EFI_LOADED_IMAGE_PROTOCOL *)ProtocolInterface->Interface)->FilePath, TRUE, TRUE);
      DEBUG((EFI_D_INFO, "    (%s)\n", PathStr));
    }
    if (CompareGuid(&ProtocolInterface->Protocol->ProtocolID, &gEfiLoadedImageDevicePathProtocolGuid)) {
      PathStr = ConvertDevicePathToText(ProtocolInterface->Interface, TRUE, TRUE);
      DEBUG((EFI_D_INFO, "    (%s)\n", PathStr));
    }
  }
}

VOID
DumpProtocolNotifyOnProtocolEntry(
  IN PROTOCOL_ENTRY  *ProtocolEntry
  )
{
  LIST_ENTRY           *ListEntry;
  PROTOCOL_NOTIFY      *ProtocolNotify;

  ListEntry = &ProtocolEntry->Notify;
  for (ListEntry = ListEntry->ForwardLink;
       ListEntry != &ProtocolEntry->Notify;
       ListEntry = ListEntry->ForwardLink) {
    ProtocolNotify = CR(ListEntry, PROTOCOL_NOTIFY, Link, PROTOCOL_NOTIFY_SIGNATURE);
    DEBUG((EFI_D_INFO, "  Notify - 0x%x", ProtocolNotify->Function));
    if (ProtocolNotify->Function != NULL) {
      CHAR16  *Name;
      Name = AddressToImageName((UINTN)ProtocolNotify->Function);
      if (StrCmp(Name, UNKNOWN_NAME) != 0) {
        DEBUG((EFI_D_INFO, " (%s)", Name));
      }
    }
    DEBUG((EFI_D_INFO, "\n"));
  }
}

VOID
DumpProtocolDatabase(
  VOID
  )
{
  LIST_ENTRY      *ListEntry;
  PROTOCOL_ENTRY  *ProtocolEntry;

  ListEntry = mSmmCoreProtocolDatabase;
  for (ListEntry = ListEntry->ForwardLink;
       ListEntry != mSmmCoreProtocolDatabase;
       ListEntry = ListEntry->ForwardLink) {
    ProtocolEntry = CR(ListEntry, PROTOCOL_ENTRY, AllEntries, PROTOCOL_ENTRY_SIGNATURE);
    if (CompareGuid (&ProtocolEntry->ProtocolID, &gEfiCallerIdGuid)) {
      // Need filter it here, because UninstallProtocol still keeps the entry
      // just leave an empty link list.
      break;
    }
    DEBUG((EFI_D_INFO, "Protocol - %a\n", GuidToName(&ProtocolEntry->ProtocolID)));
    DumpProtocolInterfacesOnProtocolEntry(ProtocolEntry);
    DumpProtocolNotifyOnProtocolEntry(ProtocolEntry);
  }

  return;
}

EFI_STATUS
EFIAPI
GetSmram (
  VOID
  )
{
  EFI_STATUS                    Status;
  EFI_SMM_ACCESS2_PROTOCOL      *SmmAccess;
  UINTN                         Size;

  //
  // Get SMRAM information
  //
  Status = gBS->LocateProtocol(&gEfiSmmAccess2ProtocolGuid, NULL, (VOID **)&SmmAccess);
  ASSERT_EFI_ERROR(Status);

  Size = 0;
  Status = SmmAccess->GetCapabilities(SmmAccess, &Size, NULL);
  ASSERT(Status == EFI_BUFFER_TOO_SMALL);

  mSmmCoreDumpInternalSmramRanges = AllocatePool(Size);
  ASSERT(mSmmCoreDumpInternalSmramRanges != NULL);

  Status = SmmAccess->GetCapabilities(SmmAccess, &Size, mSmmCoreDumpInternalSmramRanges);
  ASSERT_EFI_ERROR(Status);

  mSmmCoreDumpInternalSmramCount = Size / sizeof(EFI_SMRAM_DESCRIPTOR);

  {
    UINTN  Index;

    for (Index = 0; Index < mSmmCoreDumpInternalSmramCount; Index++) {
      DEBUG((EFI_D_INFO, "SMRAM: 0x%lx - 0x%lx\n", mSmmCoreDumpInternalSmramRanges[Index].CpuStart, mSmmCoreDumpInternalSmramRanges[Index].PhysicalSize));
    }
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
DummyHandler(
  IN EFI_HANDLE  DispatchHandle,
  IN CONST VOID  *Context         OPTIONAL,
  IN OUT VOID    *CommBuffer      OPTIONAL,
  IN OUT UINTN   *CommBufferSize  OPTIONAL
  )
{
  return EFI_SUCCESS;
}

VOID
GetSmmCoreSmiEntryList(
  IN SMI_HANDLER    *SmiHandler
  )
{
  LIST_ENTRY      *ListEntry;
  SMI_ENTRY       *SmiEntry;
  SMI_ENTRY       *Entry;

  SmiEntry = SmiHandler->SmiEntry;

  ListEntry = &SmiEntry->AllEntries;
  for (ListEntry = ListEntry->ForwardLink;
       ListEntry != &SmiEntry->AllEntries;
       ListEntry = ListEntry->ForwardLink) {
    Entry = BASE_CR(ListEntry, SMI_ENTRY, AllEntries);
    if (Entry->Signature != SMI_ENTRY_SIGNATURE) {
      // BUGBUG: NT32 will load image to BS memory, but execute code in DLL.
      // Do not use mSmmCoreLoadedImage->ImageBase/ImageSize
      mSmmCoreSmiEntryList = ListEntry;
      break;
    }
  }

  return;
}

VOID
DumpSmiHandlerOnSmiEntry(
  IN SMI_ENTRY       *SmiEntry
  )
{
  LIST_ENTRY      *ListEntry;
  SMI_HANDLER     *SmiHandler;

  ListEntry = &SmiEntry->SmiHandlers;
  for (ListEntry = ListEntry->ForwardLink;
       ListEntry != &SmiEntry->SmiHandlers;
       ListEntry = ListEntry->ForwardLink) {
    SmiHandler = CR(ListEntry, SMI_HANDLER, Link, SMI_HANDLER_SIGNATURE);
    DEBUG((EFI_D_INFO, "  SmiHandle - 0x%x\n", SmiHandler));
    DEBUG((EFI_D_INFO, "    Handler - 0x%x", SmiHandler->Handler));
    if (SmiHandler->Handler != NULL) {
      CHAR16  *Name;
      Name = AddressToImageName((UINTN)SmiHandler->Handler);
      if (StrCmp(Name, UNKNOWN_NAME) != 0) {
        DEBUG((EFI_D_INFO, " (%s)", Name));
      }
    }
    DEBUG((EFI_D_INFO, "\n"));
  }

  return;
}

VOID
DumpSmiEntryList(
  VOID
  )
{
  LIST_ENTRY      *ListEntry;
  SMI_ENTRY       *SmiEntry;

  ListEntry = mSmmCoreSmiEntryList;
  for (ListEntry = ListEntry->ForwardLink;
       ListEntry != mSmmCoreSmiEntryList;
       ListEntry = ListEntry->ForwardLink) {
    SmiEntry = CR(ListEntry, SMI_ENTRY, AllEntries, SMI_ENTRY_SIGNATURE);
    DEBUG((EFI_D_INFO, "SmiEntry - %a\n", GuidToName(&SmiEntry->HandlerType)));
    DumpSmiHandlerOnSmiEntry(SmiEntry);
  }

  return;
}

VOID
GetSmmCoreRootSmiHandlerList(
  IN SMI_HANDLER    *SmiHandler
  )
{
  LIST_ENTRY      *ListEntry;
  SMI_HANDLER     *Handler;

  ListEntry = &SmiHandler->Link;
  for (ListEntry = ListEntry->ForwardLink;
       ListEntry != &SmiHandler->Link;
       ListEntry = ListEntry->ForwardLink) {
    Handler = BASE_CR(ListEntry, SMI_HANDLER, Link);
    if (Handler->Signature != SMI_HANDLER_SIGNATURE) {
      // BUGBUG: NT32 will load image to BS memory, but execute code in DLL.
      // Do not use mSmmCoreLoadedImage->ImageBase/ImageSize
      mSmmCoreRootSmiHandlerList = ListEntry;
      break;
    }
  }

  return;
}

VOID
DumpRootSmiHandlerList(
  VOID
  )
{
  LIST_ENTRY      *ListEntry;
  SMI_HANDLER     *SmiHandler;

  ListEntry = mSmmCoreRootSmiHandlerList;
  for (ListEntry = ListEntry->ForwardLink;
       ListEntry != mSmmCoreRootSmiHandlerList;
       ListEntry = ListEntry->ForwardLink) {
    SmiHandler = CR(ListEntry, SMI_HANDLER, Link, SMI_HANDLER_SIGNATURE);
    DEBUG((EFI_D_INFO, "RootSmiHandle - 0x%x\n", SmiHandler));
    DEBUG((EFI_D_INFO, "  Handler - 0x%x", SmiHandler->Handler));
    if (SmiHandler->Handler != NULL) {
      CHAR16  *Name;
      Name = AddressToImageName((UINTN)SmiHandler->Handler);
      if (StrCmp(Name, UNKNOWN_NAME) != 0) {
        DEBUG((EFI_D_INFO, " (%s)", Name));
      }
    }
    DEBUG((EFI_D_INFO, "\n"));
  }

  return;
}

EFI_STATUS
EFIAPI
SmmCoreDump (
  VOID
  )
{
  EFI_STATUS           Status;
  EFI_HANDLE           SmmHandle;
  EFI_HANDLE           SmiHandle;

  GetSmram();

  //
  // Dump all image
  //
  DEBUG((EFI_D_INFO, "##################\n"));
  DEBUG((EFI_D_INFO, "# IMAGE DATABASE #\n"));
  DEBUG((EFI_D_INFO, "##################\n"));
  GetSmmLoadedImage ();
  if (mSmmCoreLoadedImage == NULL) {
    GetLoadedImage();
  }
  if (mSmmCoreLoadedImage == NULL) {
    DEBUG((EFI_D_INFO, "SmmCore is not found!\n"));
    goto Done;
  }
  DEBUG((EFI_D_INFO, "\n"));

  SmmHandle = NULL;
  Status = gSmst->SmmInstallProtocolInterface(
                    &SmmHandle,
                    &gEfiCallerIdGuid,
                    EFI_NATIVE_INTERFACE,
                    NULL
                    );
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  GetSmmCoreHandleList((IHANDLE *)SmmHandle);
  GetSmmCoreProtocolDatabase((IHANDLE *)SmmHandle);
  Status = gSmst->SmmUninstallProtocolInterface(
                    SmmHandle,
                    &gEfiCallerIdGuid,
                    NULL
                    );
  if (EFI_ERROR(Status)) {
    goto Done;
  }
  if (mSmmCoreHandleList == NULL) {
    DEBUG((EFI_D_INFO, "SmmCore gHandleList is not found!\n"));
    goto Done;
  }
  if (mSmmCoreProtocolDatabase == NULL) {
    DEBUG((EFI_D_INFO, "SmmCore mProtocolDatabase is not found!\n"));
  }

  //
  // Dump handle list
  //
  DEBUG((EFI_D_INFO, "###################\n"));
  DEBUG((EFI_D_INFO, "# HANDLE DATABASE #\n"));
  DEBUG((EFI_D_INFO, "###################\n"));
  DumpHandleList();
  DEBUG((EFI_D_INFO, "\n"));

  //
  // Dump protocol database
  //
  DEBUG((EFI_D_INFO, "#####################\n"));
  DEBUG((EFI_D_INFO, "# PROTOCOL DATABASE #\n"));
  DEBUG((EFI_D_INFO, "#####################\n"));
  DumpProtocolDatabase();
  DEBUG((EFI_D_INFO, "\n"));

  //
  // Dump SMI Handler
  //
  DEBUG((EFI_D_INFO, "########################\n"));
  DEBUG((EFI_D_INFO, "# SMI Handler DATABASE #\n"));
  DEBUG((EFI_D_INFO, "########################\n"));
  DEBUG((EFI_D_INFO, "# 1. GUID SMI Handler #\n"));
  SmiHandle = NULL;
  Status = gSmst->SmiHandlerRegister(
                    DummyHandler,
                    &gEfiCallerIdGuid,
                    &SmiHandle
                    );
  DEBUG((EFI_D_INFO, "\n"));
  if (!EFI_ERROR(Status)) {
    GetSmmCoreSmiEntryList((SMI_HANDLER *)SmiHandle);
    gSmst->SmiHandlerUnRegister(SmiHandle);
    if (mSmmCoreSmiEntryList != NULL) {
      DumpSmiEntryList();
    }
  }
  DEBUG((EFI_D_INFO, "# 2. ROOT SMI Handler #\n"));
  SmiHandle = NULL;
  Status = gSmst->SmiHandlerRegister(
                    DummyHandler,
                    NULL,
                    &SmiHandle
                    );
  if (!EFI_ERROR(Status)) {
    GetSmmCoreRootSmiHandlerList((SMI_HANDLER *)SmiHandle);
    gSmst->SmiHandlerUnRegister(SmiHandle);
    if (mSmmCoreRootSmiHandlerList != NULL) {
      DumpRootSmiHandlerList();
    }
  }
  DEBUG((EFI_D_INFO, "\n"));

  DEBUG((EFI_D_INFO, "SmmChildDump ...\n"));
  SmmChildDump();

  // BUGBUG: Filter myself
  RegisterSmmCoreDumpHandler();

Done:
  if (mImageStruct != NULL) {
    FreePool(mImageStruct);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SmmReadyToLockInSmmCoreDump (
  IN CONST EFI_GUID  *Protocol,
  IN VOID            *Interface,
  IN EFI_HANDLE      Handle
  )
{
  SmmCoreDump();
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SmmCoreDumpEntrypoint(
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS  Status;
  VOID        *Registration;

  Status = gSmst->SmmRegisterProtocolNotify (
                    &gEfiSmmReadyToLockProtocolGuid,
                    SmmReadyToLockInSmmCoreDump,
                    &Registration
                    );
  ASSERT_EFI_ERROR (Status);

  DEBUG((EFI_D_INFO, "SmmChildInit ...\n"));
  SmmChildInit();

  return EFI_SUCCESS;
}