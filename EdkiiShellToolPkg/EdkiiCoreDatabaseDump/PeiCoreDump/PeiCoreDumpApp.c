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
#include <Library/PeCoffGetEntryPointLib.h>
#include <Library/DxeServicesLib.h>
#include <Library/HobLib.h>

#include "PeiCoreDump.h"
#include "PeiCoreDumpComm.h"

#define UNKNOWN_NAME  L"???"

EFI_GUID mPeiCoreDumpGuid = PEI_CORE_DUMP_GUID;

VOID   *mPeiCoreDatabase;
UINTN  mPeiCoreDatabaseSize;

IMAGE_STRUCT  *mImageStruct;
UINTN         mImageStructCountMax;
UINTN         mImageStructCount;

EFI_GUID  mZeroGuid;

VOID
GetPeiCoreDatabase(
  VOID
  )
{
  VOID  *Hob;

  Hob = GetFirstGuidHob(&mPeiCoreDumpGuid);
  if (Hob == NULL) {
    Print(L"PeiCoreDump Hob not found!\n");
    return;
  }
  mPeiCoreDatabase = GET_GUID_HOB_DATA(Hob);
  mPeiCoreDatabaseSize = GET_GUID_HOB_DATA_SIZE(Hob);
  return ;
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
  if (NameString != NULL) {
    StrnCpyS(mImageStruct[mImageStructCount].NameString, NAME_STRING_LENGTH + 1, NameString, NAME_STRING_LENGTH);
  }
  CopyGuid(&mImageStruct[mImageStructCount].FileGuid, Guid);

  mImageStructCount++;
}

CHAR16 *
GetImageRefName(
  IN UINTN ImageRef
  )
{
  if (ImageRef >= mImageStructCountMax) {
    return UNKNOWN_NAME;
  }

  return mImageStruct[ImageRef].NameString;
}

VOID
DumpPeiLoadedImage(
  VOID
  )
{
  PEI_CORE_IMAGE_DATABASE_STRUCTURE  *ImageStruct;

  ImageStruct = (VOID *)mPeiCoreDatabase;
  while ((UINTN)ImageStruct < (UINTN)mPeiCoreDatabase + mPeiCoreDatabaseSize) {
    if (ImageStruct->Header.Signature == PEI_CORE_IMAGE_DATABASE_SIGNATURE) {
      if (!CompareGuid(&ImageStruct->FileGuid, &mZeroGuid)) {
        Print(L"Image: %s (0x%x)\n", ImageStruct->NameString, ImageStruct->ImageBase);
        Print(L"       (Fv(%g))\n", &ImageStruct->FileGuid);
      }
    }
    ImageStruct = (VOID *)((UINTN)ImageStruct + ImageStruct->Header.Length);
  }

  return;
}

VOID
DumpPpiOnPpiDatabaseStruct(
  IN PEI_CORE_PPI_DATABASE_STRUCTURE  *PpiDatabaseStruct
  )
{
  PEI_CORE_PPI_STRUCTURE         *PpiStruct;
  PEI_CORE_PPI_STRUCTURE_PPI     *Ppi;
  PEI_CORE_PPI_STRUCTURE_NOTIFY  *Notify;
  UINTN                   Index;

  PpiStruct = (VOID *)((UINTN)PpiDatabaseStruct + sizeof(PEI_CORE_PPI_DATABASE_STRUCTURE));
  for (Index = 0; Index < PpiDatabaseStruct->PpiCount; Index++) {
    if (PpiStruct[Index].Ppi.Flags == 0) {
      continue;
    }
    Print(L"[%d] - Flags (0x%x)\n", Index, PpiStruct[Index].Ppi.Flags);
    if ((PpiStruct[Index].Ppi.Flags & EFI_PEI_PPI_DESCRIPTOR_PPI) != 0) {
      Ppi = &PpiStruct[Index].Ppi;
      Print(L"  Ppi - %a, Interface - 0x%x", GuidToName(&Ppi->Guid), Ppi->Ppi);
      if (Ppi->Ppi != 0) {
        CHAR16  *Name;
        Name = GetImageRefName((UINTN)Ppi->ImageRef);
        if (StrCmp(Name, UNKNOWN_NAME) != 0) {
          Print(L" (%s)", Name);
        }
      }
      Print(L"\n");
    } else if ((PpiStruct[Index].Ppi.Flags & EFI_PEI_PPI_DESCRIPTOR_NOTIFY_TYPES) != 0) {
      Notify = &PpiStruct[Index].Notify;
      Print(L"  Notify - %a, Interface - 0x%x", GuidToName(&Notify->Guid), Notify->Notify);
      if (Notify->Notify != 0) {
        CHAR16  *Name;
        Name = GetImageRefName((UINTN)Notify->ImageRef);
        if (StrCmp(Name, UNKNOWN_NAME) != 0) {
          Print(L" (%s)", Name);
        }
      }
      Print(L"\n");
    }
  }
}

VOID
DumpPpiDatabase(
  VOID
  )
{
  PEI_CORE_PPI_DATABASE_STRUCTURE  *PpiDatabaseStruct;

  PpiDatabaseStruct = (VOID *)mPeiCoreDatabase;
  while ((UINTN)PpiDatabaseStruct < (UINTN)mPeiCoreDatabase + mPeiCoreDatabaseSize) {
    if (PpiDatabaseStruct->Header.Signature == PEI_CORE_PPI_DATABASE_SIGNATURE) {
      Print(L"PpiListEnd            - %d\n", PpiDatabaseStruct->PpiListEnd);
      Print(L"NotifyListEnd         - %d\n", PpiDatabaseStruct->NotifyListEnd);
      Print(L"DispatchListEnd       - %d\n", PpiDatabaseStruct->DispatchListEnd);
      Print(L"LastDispatchedInstall - %d\n", PpiDatabaseStruct->LastDispatchedInstall);
      Print(L"LastDispatchedNotify  - %d\n", PpiDatabaseStruct->LastDispatchedNotify);
      Print(L"PpiCount              - %d\n", PpiDatabaseStruct->PpiCount);
      DumpPpiOnPpiDatabaseStruct(PpiDatabaseStruct);
      break;
    }
    PpiDatabaseStruct = (VOID *)((UINTN)PpiDatabaseStruct + PpiDatabaseStruct->Header.Length);
  }

  return;
}

VOID
DumpMemoryInfo(
  VOID
  )
{
  PEI_CORE_MEMORY_INFO_STRUCTURE  *MemoryInfo;

  MemoryInfo = (VOID *)mPeiCoreDatabase;
  while ((UINTN)MemoryInfo < (UINTN)mPeiCoreDatabase + mPeiCoreDatabaseSize) {
    if (MemoryInfo->Header.Signature == PEI_CORE_MEMORY_INFO_SIGNATURE) {
      Print(L"PhysicalMemoryBegin   - 0x%016lx\n", MemoryInfo->PhysicalMemoryBegin);
      Print(L"PhysicalMemoryLength  - 0x%016lx\n", MemoryInfo->PhysicalMemoryLength);
      Print(L"FreePhysicalMemoryTop - 0x%016lx\n", MemoryInfo->FreePhysicalMemoryTop);
      break;
    }
    MemoryInfo = (VOID *)((UINTN)MemoryInfo + MemoryInfo->Header.Length);
  }

  return;
}

EFI_STATUS
EFIAPI
PeiCoreDumpAppEntrypoint(
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  GetPeiCoreDatabase();

  if (mPeiCoreDatabase == NULL) {
    return EFI_SUCCESS;
  }

  InitGuid();

  //
  // Dump all image
  //
  Print(L"##################\n");
  Print(L"# IMAGE DATABASE #\n");
  Print(L"##################\n");
  DumpPeiLoadedImage();
  Print(L"\n");

  //
  // Dump handle list
  //
  Print(L"################\n");
  Print(L"# PPI DATABASE #\n");
  Print(L"################\n");
  DumpPpiDatabase();
  Print(L"\n");

  //
  // Dump handle list
  //
  Print(L"################\n");
  Print(L"# MEMORY INFO  #\n");
  Print(L"################\n");
  DumpMemoryInfo();
  Print(L"\n");

  // BUGBUG: Filter myself

  DeinitGuid();

  return EFI_SUCCESS;
}
