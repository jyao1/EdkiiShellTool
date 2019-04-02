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
#include <Library/HobLib.h>

#include "PeiCoreDump.h"
#include "PeiCoreDumpComm.h"

//
// This structure is copied from EDKII PeiCore
//
#include "PeiCore/PeiMain.h"

EFI_GUID mPeiCoreDumpGuid = PEI_CORE_DUMP_GUID;

extern IMAGE_STRUCT  *mImageStruct;
extern UINTN         mImageStructCountMax;
extern UINTN         mImageStructCount;

VOID
BuildImageDatabaseData(
  IN PEI_CORE_INSTANCE   *PrivateData,
  IN VOID                *Data
  )
{
  PEI_CORE_IMAGE_DATABASE_STRUCTURE  *ImageStruct;
  UINTN                              Index;
  UINTN                              TotalPeimCount;

  ImageStruct = Data;
  for (Index = 0; Index < mImageStructCount; Index++) {
    ImageStruct[Index].Header.Signature = PEI_CORE_IMAGE_DATABASE_SIGNATURE;
    ImageStruct[Index].Header.Length = sizeof(PEI_CORE_IMAGE_DATABASE_STRUCTURE);
    ImageStruct[Index].Header.Revision = PEI_CORE_IMAGE_DATABASE_REVISION;
    CopyGuid(&ImageStruct[Index].FileGuid, &mImageStruct[Index].FileGuid);
    CopyMem(ImageStruct[Index].NameString, mImageStruct[Index].NameString, sizeof(ImageStruct[Index].NameString));
    ImageStruct[Index].EntryPoint = mImageStruct[Index].EntryPoint;
    ImageStruct[Index].ImageBase = mImageStruct[Index].LoadedImageBase;
    ImageStruct[Index].ImageSize = mImageStruct[Index].ImageSize;
    ImageStruct[Index].RealImageBase = mImageStruct[Index].ImageBase;
  }

  TotalPeimCount = CalcTotalPeimCount (PrivateData);
  for (; Index < TotalPeimCount; Index++) {
    ZeroMem(&ImageStruct[Index], sizeof(PEI_CORE_IMAGE_DATABASE_STRUCTURE));
    ImageStruct[Index].Header.Signature = PEI_CORE_IMAGE_DATABASE_SIGNATURE;
    ImageStruct[Index].Header.Length = sizeof(PEI_CORE_IMAGE_DATABASE_STRUCTURE);
    ImageStruct[Index].Header.Revision = PEI_CORE_IMAGE_DATABASE_REVISION;
  }
}

VOID
BuildPpiData (
  IN PEI_PPI_LIST_POINTERS       *PpiListPtrs,
  IN UINTN                       CurrentCount,
  IN OUT PEI_CORE_PPI_STRUCTURE  *PpiStructure
  )
{
  UINTN                            Index;
  EFI_PEI_PPI_DESCRIPTOR           *Ppi;
  EFI_PEI_NOTIFY_DESCRIPTOR        *Notify;

  for (Index = 0; Index < CurrentCount; Index++) {
    if (PpiListPtrs[Index].Ppi == NULL) {
      continue;
    }
    PpiStructure[Index].Ppi.Flags = PpiListPtrs[Index].Ppi->Flags;
    if ((PpiListPtrs[Index].Ppi->Flags & EFI_PEI_PPI_DESCRIPTOR_PPI) != 0) {
      Ppi = PpiListPtrs[Index].Ppi;
      CopyGuid(&PpiStructure[Index].Ppi.Guid, Ppi->Guid);
      PpiStructure[Index].Ppi.Ppi = (UINTN)Ppi->Ppi;
      PpiStructure[Index].Ppi.ImageRef = AddressToImageRefEx((UINTN)Ppi->Ppi, Ppi->Guid);
    } else if ((PpiListPtrs[Index].Ppi->Flags & EFI_PEI_PPI_DESCRIPTOR_NOTIFY_TYPES) != 0) {
      Notify = PpiListPtrs[Index].Notify;
      CopyGuid(&PpiStructure[Index].Notify.Guid, Notify->Guid);
      PpiStructure[Index].Notify.Notify = (UINTN)Notify->Notify;
      PpiStructure[Index].Notify.ImageRef = AddressToImageRef((UINTN)Notify->Notify);
    }
  }
}

VOID
BuildPpiDatabaseData(
  IN PEI_CORE_INSTANCE   *PrivateData,
  IN VOID                *Data
  )
{
  PEI_PPI_DATABASE                 *PpiData;
  PEI_CORE_PPI_DATABASE_STRUCTURE  *PpiDatabase;
  PEI_CORE_PPI_STRUCTURE           *PpiStructure;
  UINTN                            Index;
  UINTN                            TotalPpiDataCount;

  PpiData = &PrivateData->PpiData;

  TotalPpiDataCount = CalcTotalPpiDataCount (PrivateData);

  PpiDatabase = Data;
  PpiDatabase->Header.Signature = PEI_CORE_PPI_DATABASE_SIGNATURE;
  PpiDatabase->Header.Length = (UINT32)(sizeof(PEI_CORE_PPI_DATABASE_STRUCTURE) + TotalPpiDataCount * sizeof(PEI_CORE_PPI_STRUCTURE));
  PpiDatabase->Header.Revision = PEI_CORE_PPI_DATABASE_REVISION;
  PpiDatabase->PpiListCurrentCount = PpiData->PpiList.CurrentCount;
  PpiDatabase->PpiListMaxCount = PpiData->PpiList.MaxCount;
  PpiDatabase->PpiListLastDispatchedCount = PpiData->PpiList.LastDispatchedCount;
  PpiDatabase->CallbackNotifyListCurrentCount = PpiData->CallbackNotifyList.CurrentCount;
  PpiDatabase->CallbackNotifyListMaxCount = PpiData->CallbackNotifyList.MaxCount;
  PpiDatabase->DispatchNotifyListCurrentCount = PpiData->DispatchNotifyList.CurrentCount;
  PpiDatabase->DispatchNotifyListMaxCount = PpiData->DispatchNotifyList.MaxCount;
  PpiDatabase->DispatchNotifyListLastDispatchedCount = PpiData->DispatchNotifyList.LastDispatchedCount;
  PpiStructure = (VOID *)(PpiDatabase + 1);

  ZeroMem(PpiStructure, TotalPpiDataCount * sizeof(PEI_CORE_PPI_STRUCTURE));

  Index = 0;
  BuildPpiData (PpiData->PpiList.PpiPtrs, PpiData->PpiList.CurrentCount, &PpiStructure[Index]);
  Index += PpiData->PpiList.CurrentCount;
  BuildPpiData (PpiData->CallbackNotifyList.NotifyPtrs, PpiData->CallbackNotifyList.CurrentCount, &PpiStructure[Index]);
  Index += PpiData->CallbackNotifyList.CurrentCount;
  BuildPpiData (PpiData->DispatchNotifyList.NotifyPtrs, PpiData->DispatchNotifyList.CurrentCount, &PpiStructure[Index]);

  return;
}

VOID
BuildMemoryInfoData(
  IN PEI_CORE_INSTANCE   *PrivateData,
  IN VOID                *Data
  )
{
  PEI_CORE_MEMORY_INFO_STRUCTURE  *MemoryInfo;

  MemoryInfo = Data;
  MemoryInfo->Header.Signature = PEI_CORE_MEMORY_INFO_SIGNATURE;
  MemoryInfo->Header.Length    = sizeof(PEI_CORE_MEMORY_INFO_STRUCTURE);
  MemoryInfo->Header.Revision  = PEI_CORE_MEMORY_INFO_REVISION;
  MemoryInfo->PhysicalMemoryBegin   = PrivateData->PhysicalMemoryBegin;
  MemoryInfo->PhysicalMemoryLength  = PrivateData->PhysicalMemoryLength;
  MemoryInfo->FreePhysicalMemoryTop = PrivateData->FreePhysicalMemoryTop;
}

VOID
BuildPeiCoreDatabase(
  IN VOID   *PrivateData
  )
{
  UINTN             ImageDatabaseSize;
  UINTN             PpiDatabaseSize;
  UINTN             MemoryInfoSize;
  VOID              *Data;
  EFI_PEI_SERVICES  **PeiServices;
  EFI_HOB_GUID_TYPE *Hob;
  EFI_STATUS        Status;
  UINTN             TotalPpiDataCount;
  UINTN             TotalPeimCount;

  PeiServices = &((PEI_CORE_INSTANCE *)PrivateData)->Ps;

  TotalPpiDataCount = CalcTotalPpiDataCount (PrivateData);
  TotalPeimCount = CalcTotalPeimCount (PrivateData);

  ImageDatabaseSize = TotalPeimCount * sizeof(PEI_CORE_IMAGE_DATABASE_STRUCTURE);
  PpiDatabaseSize = sizeof(PEI_CORE_PPI_DATABASE_STRUCTURE) + TotalPpiDataCount * sizeof(PEI_CORE_PPI_STRUCTURE);
  MemoryInfoSize = sizeof(PEI_CORE_MEMORY_INFO_STRUCTURE);

  Status = (*PeiServices)->CreateHob(
                            (CONST EFI_PEI_SERVICES **)PeiServices,
                            EFI_HOB_TYPE_GUID_EXTENSION,
                            (UINT16)(sizeof(EFI_HOB_GUID_TYPE) + ImageDatabaseSize + PpiDatabaseSize + MemoryInfoSize),
                            (VOID **)&Hob
                            );
  if (EFI_ERROR(Status)) {
    return;
  }
  CopyGuid(&Hob->Name, &mPeiCoreDumpGuid);
  Data = Hob + 1;

  BuildImageDatabaseData(PrivateData, Data);
  BuildPpiDatabaseData(PrivateData, (VOID *)((UINTN)Data + ImageDatabaseSize));
  BuildMemoryInfoData(PrivateData, (VOID *)((UINTN)Data + ImageDatabaseSize + PpiDatabaseSize));
}