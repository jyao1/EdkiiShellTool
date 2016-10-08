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
#include <Library/PeCoffGetEntryPointLib.h>
#include <Library/DxeServicesLib.h>
#include <Protocol/SmmCommunication.h>
#include <Guid/PiSmmCommunicationRegionTable.h>

#include <Library/SmmChildDumpLib.h>

#include "SmmCoreDump.h"
#include "SmmCoreDumpComm.h"

#define UNKNOWN_NAME  L"???"

EFI_GUID mSmmCoreDumpGuid = SMM_CORE_DUMP_GUID;

VOID   *mSmmCoreDatabase;
UINTN  mSmmCoreDatabaseSize;

IMAGE_STRUCT  *mImageStruct;
UINTN         mImageStructCountMax;
UINTN         mImageStructCount;

VOID *gSmst;

CHAR16 *
AddressToImageName(
  IN UINTN  Address
  )
{
  return NULL;
}

UINTN
AddressToImageRef(
  IN UINTN  Address
  )
{
  return (UINTN)-1;
}

VOID
GetSmmCoreDatabase(
  VOID
  )
{
  EFI_STATUS                                    Status;
  UINTN                                         CommSize;
  UINT8                                         *CommBuffer;
  EFI_SMM_COMMUNICATE_HEADER                    *CommHeader;
  SMM_CORE_DUMP_PARAMETER_GET_INFO              *CommGetInfo;
  SMM_CORE_DUMP_PARAMETER_GET_DATA_BY_OFFSET    *CommGetData;
  EFI_SMM_COMMUNICATION_PROTOCOL                *SmmCommunication;
  UINTN                                         MinimalSizeNeeded;
  EDKII_PI_SMM_COMMUNICATION_REGION_TABLE       *PiSmmCommunicationRegionTable;
  UINT32                                        Index;
  EFI_MEMORY_DESCRIPTOR                         *Entry;
  VOID                                          *Buffer;
  UINTN                                         Size;
  UINTN                                         Offset;

  Status = gBS->LocateProtocol(&gEfiSmmCommunicationProtocolGuid, NULL, (VOID **)&SmmCommunication);
  if (EFI_ERROR(Status)) {
    Print(L"SmmCoreDump: Locate SmmCommunication protocol - %r\n", Status);
    return ;
  }

  MinimalSizeNeeded = EFI_PAGE_SIZE;

  Status = EfiGetSystemConfigurationTable(
             &gEdkiiPiSmmCommunicationRegionTableGuid,
             (VOID **)&PiSmmCommunicationRegionTable
             );
  if (EFI_ERROR(Status)) {
    Print(L"SmmCoreDump: Get PiSmmCommunicationRegionTable - %r\n", Status);
    return ;
  }
  ASSERT(PiSmmCommunicationRegionTable != NULL);
  Entry = (EFI_MEMORY_DESCRIPTOR *)(PiSmmCommunicationRegionTable + 1);
  Size = 0;
  for (Index = 0; Index < PiSmmCommunicationRegionTable->NumberOfEntries; Index++) {
    if (Entry->Type == EfiConventionalMemory) {
      Size = EFI_PAGES_TO_SIZE((UINTN)Entry->NumberOfPages);
      if (Size >= MinimalSizeNeeded) {
        break;
      }
    }
    Entry = (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)Entry + PiSmmCommunicationRegionTable->DescriptorSize);
  }
  ASSERT(Index < PiSmmCommunicationRegionTable->NumberOfEntries);
  CommBuffer = (UINT8 *)(UINTN)Entry->PhysicalStart;

  //
  // Get Size
  //
  CommHeader = (EFI_SMM_COMMUNICATE_HEADER *)&CommBuffer[0];
  CopyMem(&CommHeader->HeaderGuid, &mSmmCoreDumpGuid, sizeof(mSmmCoreDumpGuid));
  CommHeader->MessageLength = sizeof(SMM_CORE_DUMP_PARAMETER_GET_INFO);

  CommGetInfo = (SMM_CORE_DUMP_PARAMETER_GET_INFO *)&CommBuffer[OFFSET_OF(EFI_SMM_COMMUNICATE_HEADER, Data)];
  CommGetInfo->Header.Command = SMM_CORE_DUMP_COMMAND_GET_INFO;
  CommGetInfo->Header.DataLength = sizeof(*CommGetInfo);
  CommGetInfo->Header.ReturnStatus = (UINT64)-1;
  CommGetInfo->DumpSize = 0;

  CommSize = sizeof(EFI_GUID) + sizeof(UINTN) + CommHeader->MessageLength;
  Status = SmmCommunication->Communicate(SmmCommunication, CommBuffer, &CommSize);
  if (EFI_ERROR(Status)) {
    Print(L"SmmCoreDump: SmmCommunication - %r\n", Status);
    return ;
  }

  if (CommGetInfo->Header.ReturnStatus != 0) {
    Print(L"SmmCoreDump: GetInfo - 0x%0x\n", CommGetInfo->Header.ReturnStatus);
    return ;
  }

  mSmmCoreDatabaseSize = (UINTN)CommGetInfo->DumpSize;

  //
  // Get Data
  //
  mSmmCoreDatabase = AllocateZeroPool(mSmmCoreDatabaseSize);
  if (mSmmCoreDatabase == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    Print(L"SmmCoreDump: AllocateZeroPool (0x%x) for dump buffer - %r\n", mSmmCoreDatabaseSize, Status);
    return ;
  }

  CommHeader = (EFI_SMM_COMMUNICATE_HEADER *)&CommBuffer[0];
  CopyMem(&CommHeader->HeaderGuid, &mSmmCoreDumpGuid, sizeof(mSmmCoreDumpGuid));
  CommHeader->MessageLength = sizeof(SMM_CORE_DUMP_PARAMETER_GET_DATA_BY_OFFSET);

  CommGetData = (SMM_CORE_DUMP_PARAMETER_GET_DATA_BY_OFFSET *)&CommBuffer[OFFSET_OF(EFI_SMM_COMMUNICATE_HEADER, Data)];
  CommGetData->Header.Command = SMM_CORE_DUMP_COMMAND_GET_DATA_BY_OFFSET;
  CommGetData->Header.DataLength = sizeof(*CommGetData);
  CommGetData->Header.ReturnStatus = (UINT64)-1;

  CommSize = sizeof(EFI_GUID) + sizeof(UINTN) + CommHeader->MessageLength;
  Buffer = (UINT8 *)CommHeader + CommSize;
  Size -= CommSize;

  CommGetData->DumpBuffer = (PHYSICAL_ADDRESS)(UINTN)Buffer;
  CommGetData->DumpOffset = 0;
  while (CommGetData->DumpOffset < mSmmCoreDatabaseSize) {
    Offset = (UINTN)CommGetData->DumpOffset;
    if (Size <= (mSmmCoreDatabaseSize - CommGetData->DumpOffset)) {
      CommGetData->DumpSize = (UINT64)Size;
    } else {
      CommGetData->DumpSize = (UINT64)(mSmmCoreDatabaseSize - CommGetData->DumpOffset);
    }
    Status = SmmCommunication->Communicate(SmmCommunication, CommBuffer, &CommSize);
    ASSERT_EFI_ERROR(Status);

    if (CommGetData->Header.ReturnStatus != 0) {
      FreePool(mSmmCoreDatabase);
      mSmmCoreDatabase = NULL;
      Print(L"SmmCoreDump: GetData - 0x%x\n", CommGetData->Header.ReturnStatus);
      return ;
    }
    CopyMem((UINT8 *)mSmmCoreDatabase + Offset, (VOID *)(UINTN)CommGetData->DumpBuffer, (UINTN)CommGetData->DumpSize);
  }

  DEBUG((EFI_D_INFO, "SmmCoreDumpSize - 0x%x\n", mSmmCoreDatabaseSize));

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
DumpSmmLoadedImage(
  VOID
  )
{
  SMM_CORE_IMAGE_DATABASE_STRUCTURE  *ImageStruct;

  mImageStructCountMax = 0;
  ImageStruct = (VOID *)mSmmCoreDatabase;
  while ((UINTN)ImageStruct < (UINTN)mSmmCoreDatabase + mSmmCoreDatabaseSize) {
    if (ImageStruct->Header.Signature == SMM_CORE_IMAGE_DATABASE_SIGNATURE) {
      mImageStructCountMax++;
    }
    ImageStruct = (VOID *)((UINTN)ImageStruct + ImageStruct->Header.Length);
  }
  mImageStruct = AllocateZeroPool(mImageStructCountMax * sizeof(IMAGE_STRUCT));
  if (mImageStruct == NULL) {
    mImageStructCountMax = 0;
    return;
  }

  ImageStruct = (VOID *)mSmmCoreDatabase;
  while ((UINTN)ImageStruct < (UINTN)mSmmCoreDatabase + mSmmCoreDatabaseSize) {
    if (ImageStruct->Header.Signature == SMM_CORE_IMAGE_DATABASE_SIGNATURE) {
      Print(L"Image - %s ", ImageStruct->NameString);
      Print(L"(0x%x - 0x%x", ImageStruct->ImageBase, ImageStruct->ImageSize);
      if (ImageStruct->EntryPoint != 0) {
        Print(L", EntryPoint:0x%x", ImageStruct->EntryPoint);
      }
      if (ImageStruct->ImageBase != ImageStruct->RealImageBase) {
        Print(L", Base:0x%x", ImageStruct->RealImageBase);
      }
      Print(L")\n");
      Print(L"       (FvFile(%g))\n", &ImageStruct->FileGuid);

      AddImageStruct((UINTN)ImageStruct->RealImageBase, ImageStruct->ImageSize, ImageStruct->ImageBase, ImageStruct->EntryPoint, ImageStruct->NameString, &ImageStruct->FileGuid);
    }

    ImageStruct = (VOID *)((UINTN)ImageStruct + ImageStruct->Header.Length);
  }

  return;
}

VOID
DumpProtocolInterfacesOnHandleStruct(
  IN SMM_CORE_HANDLE_DATABASE_STRUCTURE  *HandleStruct
  )
{
  SMM_CORE_HANDLE_PROTOCOL_STRUCTURE  *Protocol;
  UINTN                               Index;

  Protocol = (VOID *)((UINTN)HandleStruct + sizeof(SMM_CORE_HANDLE_DATABASE_STRUCTURE));
  for (Index = 0; Index < HandleStruct->ProtocolCount; Index++) {
    Print(L"  Protocol - %a, Interface - 0x%x", GuidToName(&Protocol[Index].Guid), Protocol[Index].Interface);
    if (Protocol[Index].Interface != 0) {
      CHAR16  *Name;
      Name = GetImageRefName(Protocol[Index].ImageRef);
      if (StrCmp(Name, UNKNOWN_NAME) != 0) {
        Print(L" (%s)", Name);
      }
    }
    Print(L"\n");
  }
}

VOID
DumpHandleDatabase(
  VOID
  )
{
  SMM_CORE_HANDLE_DATABASE_STRUCTURE  *HandleStruct;

  HandleStruct = (VOID *)mSmmCoreDatabase;
  while ((UINTN)HandleStruct < (UINTN)mSmmCoreDatabase + mSmmCoreDatabaseSize) {
    if (HandleStruct->Header.Signature == SMM_CORE_HANDLE_DATABASE_SIGNATURE) {
      Print(L"Handle - 0x%x\n", HandleStruct->Handle);
      DumpProtocolInterfacesOnHandleStruct(HandleStruct);
    }
    HandleStruct = (VOID *)((UINTN)HandleStruct + HandleStruct->Header.Length);
  }

  return;
}

VOID
DumpProtocolInterfacesOnProtocolStruct(
  IN SMM_CORE_PROTOCOL_DATABASE_STRUCTURE  *ProtocolStruct
  )
{
  SMM_CORE_PROTOCOL_INTERFACE_STRUCTURE  *Interface;
  UINTN                                  Index;

  Interface = (VOID *)((UINTN)ProtocolStruct + sizeof(SMM_CORE_PROTOCOL_DATABASE_STRUCTURE));
  for (Index = 0; Index < ProtocolStruct->InterfaceCount; Index++) {
    Print(L"  Interface - 0x%x", Interface[Index].Interface);
    if (Interface[Index].Interface != 0) {
      CHAR16  *Name;
      Name = GetImageRefName(Interface[Index].ImageRef);
      if (StrCmp(Name, UNKNOWN_NAME) != 0) {
        Print(L" (%s)", Name);
      }
    }
    Print(L"\n");
  }
}

VOID
DumpProtocolNotifyOnProtocolStruct(
  IN SMM_CORE_PROTOCOL_DATABASE_STRUCTURE  *ProtocolStruct
  )
{
  SMM_CORE_PROTOCOL_NOTIFY_STRUCTURE     *Notify;
  UINTN                                  Index;

  Notify = (VOID *)((UINTN)ProtocolStruct + sizeof(SMM_CORE_PROTOCOL_DATABASE_STRUCTURE) +
                    ProtocolStruct->InterfaceCount * sizeof(SMM_CORE_PROTOCOL_INTERFACE_STRUCTURE));
  for (Index = 0; Index < ProtocolStruct->NotifyCount; Index++) {
    Print(L"  Notify - 0x%x", Notify[Index].Notify);
    if (Notify[Index].Notify != 0) {
      CHAR16  *Name;
      Name = GetImageRefName(Notify[Index].ImageRef);
      if (StrCmp(Name, UNKNOWN_NAME) != 0) {
        Print(L" (%s)", Name);
      }
    }
    Print(L"\n");
  }
}

VOID
DumpProtocolDatabase(
  VOID
  )
{
  SMM_CORE_PROTOCOL_DATABASE_STRUCTURE  *ProtocolStruct;

  ProtocolStruct = (VOID *)mSmmCoreDatabase;
  while ((UINTN)ProtocolStruct < (UINTN)mSmmCoreDatabase + mSmmCoreDatabaseSize) {
    if (ProtocolStruct->Header.Signature == SMM_CORE_PROTOCOL_DATABASE_SIGNATURE) {
      Print(L"Protocol - %a\n", GuidToName(&ProtocolStruct->Guid));
      DumpProtocolInterfacesOnProtocolStruct(ProtocolStruct);
      DumpProtocolNotifyOnProtocolStruct(ProtocolStruct);
    }
    ProtocolStruct = (VOID *)((UINTN)ProtocolStruct + ProtocolStruct->Header.Length);
  }

  return;
}

VOID
DumpSmiHandler(
  VOID
  )
{
  SMM_CORE_SMI_DATABASE_STRUCTURE  *SmiStruct;
  SMM_CORE_SMI_HANDLER_STRUCTURE   *SmiHandlerStruct;
  UINTN                            Index;

  SmiStruct = (VOID *)mSmmCoreDatabase;
  while ((UINTN)SmiStruct < (UINTN)mSmmCoreDatabase + mSmmCoreDatabaseSize) {
    if ((SmiStruct->Header.Signature == SMM_CORE_SMI_DATABASE_SIGNATURE) && (!SmiStruct->IsRootHandler)) {
      SmiHandlerStruct = (VOID *)(SmiStruct + 1);
      Print(L"SmiEntry - %a\n", GuidToName(&SmiStruct->HandlerType));
      for (Index = 0; Index < SmiStruct->HandlerCount; Index++) {
        Print(L"  SmiHandle - 0x%x\n", SmiHandlerStruct[Index].SmiHandle);
        Print(L"    Handler - 0x%x", SmiHandlerStruct[Index].Handler);
        if (SmiHandlerStruct[Index].Handler != 0) {
          CHAR16  *Name;
          Name = GetImageRefName((UINTN)SmiHandlerStruct[Index].ImageRef);
          if (StrCmp(Name, UNKNOWN_NAME) != 0) {
            Print(L" (%s)", Name);
          }
        }
        Print(L"\n");
      }
    }
    SmiStruct = (VOID *)((UINTN)SmiStruct + SmiStruct->Header.Length);
  }

  return;
}

VOID
DumpRootSmiHandler(
  VOID
  )
{
  SMM_CORE_SMI_DATABASE_STRUCTURE  *SmiStruct;
  SMM_CORE_SMI_HANDLER_STRUCTURE   *SmiHandlerStruct;
  UINTN                            Index;

  SmiStruct = (VOID *)mSmmCoreDatabase;
  while ((UINTN)SmiStruct < (UINTN)mSmmCoreDatabase + mSmmCoreDatabaseSize) {
    if ((SmiStruct->Header.Signature == SMM_CORE_SMI_DATABASE_SIGNATURE) && (SmiStruct->IsRootHandler)) {
      SmiHandlerStruct = (VOID *)(SmiStruct + 1);
      for (Index = 0; Index < SmiStruct->HandlerCount; Index++) {
        Print(L"RootSmiHandle - 0x%x\n", SmiHandlerStruct[Index].SmiHandle);
        Print(L"  Handler - 0x%x", SmiHandlerStruct[Index].Handler);
        if (SmiHandlerStruct[Index].Handler != 0) {
          CHAR16  *Name;
          Name = GetImageRefName((UINTN)SmiHandlerStruct[Index].ImageRef);
          if (StrCmp(Name, UNKNOWN_NAME) != 0) {
            Print (L" (%s)", Name);
          }
        }
        Print(L"\n");
      }
    }
    SmiStruct = (VOID *)((UINTN)SmiStruct + SmiStruct->Header.Length);
  }

  return;
}

EFI_STATUS
EFIAPI
SmmCoreDumpAppEntrypoint(
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  GetSmmCoreDatabase();

  if (mSmmCoreDatabase == NULL) {
    return EFI_SUCCESS;
  }

  InitGuid();

  //
  // Dump all image
  //
  Print(L"##################\n");
  Print(L"# IMAGE DATABASE #\n");
  Print(L"##################\n");
  DumpSmmLoadedImage();
  Print(L"\n");

  //
  // Dump handle list
  //
  Print(L"###################\n");
  Print(L"# HANDLE DATABASE #\n");
  Print(L"###################\n");
  DumpHandleDatabase();
  Print(L"\n");

  //
  // Dump protocol database
  //
  Print(L"#####################\n");
  Print(L"# PROTOCOL DATABASE #\n");
  Print(L"#####################\n");
  DumpProtocolDatabase();
  Print(L"\n");

  //
  // Dump SMI Handler
  //
  Print(L"########################\n");
  Print(L"# SMI Handler DATABASE #\n");
  Print(L"########################\n");
  Print(L"# 1. GUID SMI Handler #\n");
  DumpSmiHandler();
  Print(L"# 2. ROOT SMI Handler #\n");
  DumpRootSmiHandler();
  Print(L"\n");

  // BUGBUG: Filter myself

  DumpSmmChildHandler(mSmmCoreDatabase, mSmmCoreDatabaseSize);

  DeinitGuid();

  if (mSmmCoreDatabase != NULL) {
    FreePool(mSmmCoreDatabase);
  }

  return EFI_SUCCESS;
}
