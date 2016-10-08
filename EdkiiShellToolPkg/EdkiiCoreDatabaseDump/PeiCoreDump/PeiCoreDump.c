/** @file

Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <PiPei.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/PeCoffGetEntryPointLib.h>
#include <Ppi/EndOfPeiPhase.h>

#include <Ppi/StatusCode.h>
#include <Ppi/ReportStatusCodeHandler.h>

#include "PeiCoreDump.h"

//
// This structure is copied from EDKII PeiCore
//
#include "PeiCore/PeiMain.h"

typedef struct {
  EFI_GUID   *Guid;
  UINTN      FuncOffset;
} PPI_FUNC_STRUCT;

PPI_FUNC_STRUCT  mPpiFuncStruct[] = {
  { &gEfiPeiStatusCodePpiGuid, OFFSET_OF(EFI_PEI_PROGRESS_CODE_PPI, ReportStatusCode) },
  { &gEfiPeiRscHandlerPpiGuid, OFFSET_OF(EFI_PEI_RSC_HANDLER_PPI, Register) },
};

CHAR16 mNameString[NAME_STRING_LENGTH + 1];

IMAGE_STRUCT  *mImageStruct;
UINTN         mImageStructCountMax;
UINTN         mImageStructCount;

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
  if (NameString != NULL) {
    StrnCpyS(mImageStruct[mImageStructCount].NameString, NAME_STRING_LENGTH + 1, NameString, NAME_STRING_LENGTH);
  }
  CopyGuid(&mImageStruct[mImageStructCount].FileGuid, Guid);

  mImageStructCount++;
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
  for (Index = 0; Index < sizeof(mPpiFuncStruct) / sizeof(mPpiFuncStruct[0]); Index++) {
    if (CompareGuid(Protocol, mPpiFuncStruct[Index].Guid)) {
      return AddressToImageRef(*(UINTN *)(Address + mPpiFuncStruct[Index].FuncOffset));
    }
  }
  return (UINTN)-1;
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
  IN EFI_GUID  *Ppi
  )
{
  CHAR16  *Name;
  UINTN   Index;
  Name = AddressToImageName(Address);
  if (StrCmp(Name, UNKNOWN_NAME) != 0) {
    return Name;
  }
  for (Index = 0; Index < sizeof(mPpiFuncStruct) / sizeof(mPpiFuncStruct[0]); Index++) {
    if (CompareGuid(Ppi, mPpiFuncStruct[Index].Guid)) {
      return AddressToImageName(*(UINTN *)(Address + mPpiFuncStruct[Index].FuncOffset));
    }
  }
  return UNKNOWN_NAME;
}

CHAR16 *
GetDriverNameString(
  IN EFI_PEI_SERVICES     **PeiServices,
  IN EFI_PEI_FILE_HANDLE  FileHandle,
  OUT EFI_GUID            *Guid,
  OUT UINTN               *ImageBase
  )
{
  EFI_STATUS                  Status;
  CHAR8                       *PdbFileName;
  CHAR16                      *NameString;
  VOID                        *ImageData;
  EFI_FV_FILE_INFO            FileInfo;

  NameString = NULL;
  Status = (*PeiServices)->FfsFindSectionData((CONST EFI_PEI_SERVICES **)PeiServices, EFI_SECTION_PE32, FileHandle, &ImageData);
  if (EFI_ERROR(Status)) {
    Status = (*PeiServices)->FfsFindSectionData((CONST EFI_PEI_SERVICES **)PeiServices, EFI_SECTION_TE, FileHandle, &ImageData);
  }
  if (!EFI_ERROR(Status)) {
    if (ImageBase != NULL) {
      *ImageBase = (UINTN)ImageData;
    }
    //
    // Method 1: Get the name string from image PDB
    //
    PdbFileName = PeCoffLoaderGetPdbPointer(ImageData);
    if (PdbFileName != NULL) {
      GetShortPdbFileName(PdbFileName, mNameString);
      NameString = mNameString;
    }
  }

  //
  // Try to get the image's FFS UI section by image GUID
  //
  if (NameString == NULL) {
    Status = (*PeiServices)->FfsFindSectionData((CONST EFI_PEI_SERVICES **)PeiServices, EFI_SECTION_USER_INTERFACE, FileHandle, (VOID **)&NameString);
    if (!EFI_ERROR(Status)) {
      //
      // Method 2: Get the name string from FFS UI section
      //
      StrnCpyS(mNameString, NAME_STRING_LENGTH + 1, NameString, NAME_STRING_LENGTH);
      mNameString[NAME_STRING_LENGTH] = 0;
      NameString = mNameString;
    }
  }

  //
  // Method 3: Get the name string from FFS GUID
  //
  Status = (*PeiServices)->FfsGetFileInfo(FileHandle, &FileInfo);
  if (!EFI_ERROR(Status)) {
    if (Guid != NULL) {
      CopyGuid(Guid, &FileInfo.FileName);
    }
    if (NameString == NULL) {
      UnicodeSPrint(mNameString, sizeof(mNameString), L"%g", &FileInfo.FileName);
      NameString = mNameString;
    }
  }

  return NameString;
}

VOID
DumpImageData(
  IN PEI_CORE_INSTANCE   *PrivateData
  )
{
  EFI_PEI_SERVICES    **PeiServices;
  UINTN               FvIndex;
  PEI_CORE_FV_HANDLE  *Fv;
  UINTN               PeimIndex;
  EFI_PEI_FILE_HANDLE *FvFileHandles;
  EFI_FV_INFO         VolumeInfo;
  EFI_STATUS          Status;
  EFI_GUID            Guid;
  UINTN               ImageBase;
  CHAR16              *NameString;

  PeiServices = &PrivateData->Ps;

  mImageStructCountMax = PrivateData->FvCount * PcdGet32(PcdPeiCoreMaxPeimPerFv);
  mImageStruct = AllocateZeroPool(mImageStructCountMax * sizeof(IMAGE_STRUCT));
  if (mImageStruct == NULL) {
    return;
  }

  Fv = PrivateData->Fv;
  for (FvIndex = 0; FvIndex < PrivateData->FvCount; FvIndex++) {
    DEBUG((EFI_D_INFO, "FV[%d] - 0x%x (0x%x)", FvIndex, Fv[FvIndex].FvHandle, Fv[FvIndex].FvHeader));

    Status = (*PeiServices)->FfsGetVolumeInfo(Fv[FvIndex].FvHandle, &VolumeInfo);
    if (!EFI_ERROR(Status)) {
      DEBUG((EFI_D_INFO, "  (%a-%a)", GuidToName(&VolumeInfo.FvName), GuidToName(&VolumeInfo.FvFormat)));
    }
    DEBUG((EFI_D_INFO, "\n"));

    FvFileHandles = Fv[FvIndex].FvFileHandles;
    for (PeimIndex = 0; PeimIndex < PcdGet32(PcdPeiCoreMaxPeimPerFv); PeimIndex++) {
      if (FvFileHandles[PeimIndex] == NULL) {
        break;
      }
      DEBUG((EFI_D_INFO, "  FileHandle[%d] - 0x%x (0x%x)", PeimIndex, FvFileHandles[PeimIndex], Fv[FvIndex].PeimState[PeimIndex]));

      ImageBase = 0;
      ZeroMem(&Guid, sizeof(EFI_GUID));
      NameString = GetDriverNameString(PeiServices, FvFileHandles[PeimIndex], &Guid, &ImageBase);
      if (NameString != NULL) {
        DEBUG((EFI_D_INFO, " %S", NameString));
      }
      if (ImageBase != 0) {
        DEBUG((EFI_D_INFO, " (0x%x)", ImageBase));
      }
      DEBUG((EFI_D_INFO, "\n"));
      DEBUG((EFI_D_INFO, "    (Fv(%g))\n", &Guid));
      AddImageStruct(ImageBase, 0, ImageBase, NameString, &Guid);
    }
  }
}

VOID
DumpPpiData(
  IN PEI_PPI_DATABASE  *PpiData
  )
{
  INTN                        Index;
  PEI_PPI_LIST_POINTERS       *PpiListPtrs;
  EFI_PEI_PPI_DESCRIPTOR      *Ppi;
  EFI_PEI_NOTIFY_DESCRIPTOR   *Notify;

  DEBUG((EFI_D_INFO, "PpiListEnd            - %d\n", PpiData->PpiListEnd));
  DEBUG((EFI_D_INFO, "NotifyListEnd         - %d\n", PpiData->NotifyListEnd));
  DEBUG((EFI_D_INFO, "DispatchListEnd       - %d\n", PpiData->DispatchListEnd));
  DEBUG((EFI_D_INFO, "LastDispatchedInstall - %d\n", PpiData->NotifyListEnd));
  DEBUG((EFI_D_INFO, "LastDispatchedNotify  - %d\n", PpiData->NotifyListEnd));
  DEBUG((EFI_D_INFO, "PpiListPtrs           - %d\n", PpiData->PpiListPtrs));

  PpiListPtrs = PpiData->PpiListPtrs;
  for (Index = 0;
       Index < (INTN)PcdGet32(PcdPeiCoreMaxPpiSupported);
       Index++) {
    if (PpiListPtrs[Index].Ppi == NULL) {
      continue;
    }
    DEBUG((EFI_D_INFO, "[%d] - Flags (0x%x)\n", Index, PpiListPtrs[Index].Ppi->Flags));
    if ((PpiListPtrs[Index].Ppi->Flags & EFI_PEI_PPI_DESCRIPTOR_PPI) != 0) {
      Ppi = PpiListPtrs[Index].Ppi;
      DEBUG((EFI_D_INFO, "  Ppi - %a, Interface - 0x%x", GuidToName(Ppi->Guid), Ppi->Ppi));
      if (Ppi->Ppi != NULL) {
        CHAR16  *Name;
        Name = AddressToImageNameEx((UINTN)Ppi->Ppi, Ppi->Guid);
        if (StrCmp(Name, UNKNOWN_NAME) != 0) {
          DEBUG((EFI_D_INFO, " (%s)", Name));
        }
      }
      DEBUG((EFI_D_INFO, "\n"));
    } else if ((PpiListPtrs[Index].Ppi->Flags & EFI_PEI_PPI_DESCRIPTOR_NOTIFY_TYPES) != 0) {
      Notify = PpiListPtrs[Index].Notify;
      DEBUG((EFI_D_INFO, "  Notify - %a, Interface - 0x%x", GuidToName(Notify->Guid), Notify->Notify));
      if (Notify->Notify != NULL) {
        CHAR16  *Name;
        Name = AddressToImageName((UINTN)Notify->Notify);
        if (StrCmp(Name, UNKNOWN_NAME) != 0) {
          DEBUG((EFI_D_INFO, " (%s)", Name));
        }
      }
      DEBUG((EFI_D_INFO, "\n"));
    }
  }
}

EFI_STATUS
EFIAPI
PeiCoreDump (
  IN      EFI_PEI_SERVICES        **PeiServices
  )
{
  PEI_CORE_INSTANCE   *PrivateData;

  PrivateData = PEI_CORE_INSTANCE_FROM_PS_THIS(PeiServices);

  //
  // Dump all image
  //
  DEBUG((EFI_D_INFO, "##################\n"));
  DEBUG((EFI_D_INFO, "# IMAGE DATABASE #\n"));
  DEBUG((EFI_D_INFO, "##################\n"));
  DumpImageData(PrivateData);
  DEBUG((EFI_D_INFO, "\n"));

  //
  // Dump ppi database
  //
  DEBUG((EFI_D_INFO, "################\n"));
  DEBUG((EFI_D_INFO, "# PPI DATABASE #\n"));
  DEBUG((EFI_D_INFO, "################\n"));
  DumpPpiData(&PrivateData->PpiData);
  DEBUG((EFI_D_INFO, "\n"));

  // BUGBUG: Filter myself

  BuildPeiCoreDatabase(PrivateData);

  if (mImageStruct != NULL) {
    FreePool(mImageStruct);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
PeiCoreDumpEndOfPeiCallback(
  IN      EFI_PEI_SERVICES        **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR    *NotifyDescriptor,
  IN VOID                         *Ppi
  )
{
  PeiCoreDump(PeiServices);
  return EFI_SUCCESS;
}

GLOBAL_REMOVE_IF_UNREFERENCED EFI_PEI_NOTIFY_DESCRIPTOR mNotifyList = {
  (EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gEfiEndOfPeiSignalPpiGuid,
  PeiCoreDumpEndOfPeiCallback
};

EFI_STATUS
EFIAPI
PeiCoreDumpEntrypoint(
  IN       EFI_PEI_FILE_HANDLE       FileHandle,
  IN CONST EFI_PEI_SERVICES          **PeiServices
  )
{
  EFI_STATUS  Status;

  Status = (*PeiServices)->NotifyPpi(PeiServices, &mNotifyList);
  ASSERT_EFI_ERROR(Status);

  return EFI_SUCCESS;
}