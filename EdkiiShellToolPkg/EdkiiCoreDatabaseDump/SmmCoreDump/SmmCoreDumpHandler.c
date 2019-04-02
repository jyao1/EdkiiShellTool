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
#include <Library/SmmServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/SmmMemLib.h>
#include <Protocol/LoadedImage.h>

#include <Library/SmmChildDumpLib.h>

#include "SmmCoreDump.h"
#include "SmmCoreDumpComm.h"

//
// This structure is copied from EDKII SmmCore
//
#include "SmmCore/SmmCore.h"
#include "SmmCore/PiSmmCore.h"

extern LIST_ENTRY      *mSmmCoreHandleList;
extern LIST_ENTRY      *mSmmCoreProtocolDatabase;
extern LIST_ENTRY      *mSmmCoreRootSmiHandlerList;
extern LIST_ENTRY      *mSmmCoreSmiEntryList;

extern IMAGE_STRUCT  *mImageStruct;
extern UINTN         mImageStructCount;

EFI_GUID mSmmCoreDumpGuid = SMM_CORE_DUMP_GUID;

VOID   *mSmmCoreDatabase;
UINTN  mSmmCoreDatabaseSize;

UINTN  mSmmImageDatabaseSize;
UINTN  mSmmHandlerDatabaseSize;
UINTN  mSmmProtocolDatabaseSize;
UINTN  mSmmSmiDatabaseSize;
UINTN  mSmmRootSmiDatabaseSize;
UINTN  mSmmChildDatabaseSize;

BOOLEAN                       mSmmCoreDumpRecordingStatus;

UINTN
GetSmmImageDatabaseSize(
  VOID
  )
{
  return sizeof(SMM_CORE_IMAGE_DATABASE_STRUCTURE) * mImageStructCount;
}

UINTN
GetSmmProtocolInterfacesSizeOnHandle(
  IN IHANDLE    *IHandle
  )
{
  LIST_ENTRY           *ListEntry;
  UINTN                Size;

  Size = 0;
  ListEntry = &IHandle->Protocols;
  for (ListEntry = ListEntry->ForwardLink;
       ListEntry != &IHandle->Protocols;
       ListEntry = ListEntry->ForwardLink) {
    Size += sizeof(SMM_CORE_HANDLE_PROTOCOL_STRUCTURE);
  }
  DEBUG((EFI_D_VERBOSE, "GetSmmProtocolInterfacesSizeOnHandle(0x%x) - 0x%x\n", IHandle, Size));
  return Size;
}

UINTN
GetSmmHandlerDatabaseSize(
  VOID
  )
{
  LIST_ENTRY      *ListEntry;
  IHANDLE         *Handle;
  UINTN           Size;

  Size = 0;
  ListEntry = mSmmCoreHandleList;
  for (ListEntry = ListEntry->ForwardLink;
       ListEntry != mSmmCoreHandleList;
       ListEntry = ListEntry->ForwardLink) {
    Handle = CR(ListEntry, IHANDLE, AllHandles, EFI_HANDLE_SIGNATURE);
    Size += sizeof(SMM_CORE_HANDLE_DATABASE_STRUCTURE);
    Size += GetSmmProtocolInterfacesSizeOnHandle(Handle);
  }
  DEBUG((EFI_D_VERBOSE, "GetSmmHandlerDatabaseSize - 0x%x\n", Size));
  return Size;
}

UINTN
GetSmmProtocolInterfacesSizeOnProtocolEntry(
  IN PROTOCOL_ENTRY  *ProtocolEntry
  )
{
  LIST_ENTRY           *ListEntry;
  UINTN                Size;

  Size = 0;
  ListEntry = &ProtocolEntry->Protocols;
  for (ListEntry = ListEntry->ForwardLink;
       ListEntry != &ProtocolEntry->Protocols;
       ListEntry = ListEntry->ForwardLink) {
    Size += sizeof(SMM_CORE_PROTOCOL_INTERFACE_STRUCTURE);
  }
  DEBUG((EFI_D_VERBOSE, "GetSmmProtocolInterfacesSizeOnProtocolEntry(0x%x) - 0x%x\n", ProtocolEntry, Size));
  return Size;
}

UINTN
GetSmmProtocolNotifySizeOnProtocolEntry(
  IN PROTOCOL_ENTRY  *ProtocolEntry
  )
{
  LIST_ENTRY           *ListEntry;
  UINTN                Size;

  Size = 0;
  ListEntry = &ProtocolEntry->Notify;
  for (ListEntry = ListEntry->ForwardLink;
       ListEntry != &ProtocolEntry->Notify;
       ListEntry = ListEntry->ForwardLink) {
    Size += sizeof(SMM_CORE_PROTOCOL_NOTIFY_STRUCTURE);
  }
  DEBUG((EFI_D_VERBOSE, "GetSmmProtocolNotifySizeOnProtocolEntry(0x%x) - 0x%x\n", ProtocolEntry, Size));
  return Size;
}

UINTN
GetSmmProtocolDatabaseSize(
  VOID
  )
{
  LIST_ENTRY      *ListEntry;
  PROTOCOL_ENTRY  *ProtocolEntry;
  UINTN           Size;

  Size = 0;
  ListEntry = mSmmCoreProtocolDatabase;
  for (ListEntry = ListEntry->ForwardLink;
       ListEntry != mSmmCoreProtocolDatabase;
       ListEntry = ListEntry->ForwardLink) {
    ProtocolEntry = CR(ListEntry, PROTOCOL_ENTRY, AllEntries, PROTOCOL_ENTRY_SIGNATURE);
    if (CompareGuid(&ProtocolEntry->ProtocolID, &gEfiCallerIdGuid)) {
      // Need filter it here, because UninstallProtocol still keeps the entry
      // just leave an empty link list.
      break;
    }
    Size += sizeof(SMM_CORE_PROTOCOL_DATABASE_STRUCTURE);
    Size += GetSmmProtocolInterfacesSizeOnProtocolEntry(ProtocolEntry);
    Size += GetSmmProtocolNotifySizeOnProtocolEntry(ProtocolEntry);
  }

  DEBUG((EFI_D_VERBOSE, "GetSmmProtocolDatabaseSize - 0x%x\n", Size));
  return Size;
}

UINTN
GetSmmSmiHandlerSizeOnSmiEntry(
  IN SMI_ENTRY       *SmiEntry
  )
{
  LIST_ENTRY      *ListEntry;
  UINTN           Size;

  Size = 0;
  ListEntry = &SmiEntry->SmiHandlers;
  for (ListEntry = ListEntry->ForwardLink;
       ListEntry != &SmiEntry->SmiHandlers;
       ListEntry = ListEntry->ForwardLink) {
    Size += sizeof(SMM_CORE_SMI_HANDLER_STRUCTURE);
  }

  return Size;
}

UINTN
GetSmmSmiDatabaseSize(
  VOID
  )
{
  LIST_ENTRY      *ListEntry;
  SMI_ENTRY       *SmiEntry;
  UINTN           Size;

  Size = 0;
  ListEntry = mSmmCoreSmiEntryList;
  for (ListEntry = ListEntry->ForwardLink;
       ListEntry != mSmmCoreSmiEntryList;
       ListEntry = ListEntry->ForwardLink) {
    SmiEntry = CR(ListEntry, SMI_ENTRY, AllEntries, SMI_ENTRY_SIGNATURE);
    Size += sizeof(SMM_CORE_SMI_DATABASE_STRUCTURE);
    Size += GetSmmSmiHandlerSizeOnSmiEntry(SmiEntry);
  }
  return Size;
}

UINTN
GetSmmRootSmiDatabaseSize(
  VOID
  )
{
  LIST_ENTRY      *ListEntry;
  UINTN           Size;

  Size = 0;
  Size += sizeof(SMM_CORE_SMI_DATABASE_STRUCTURE);
  ListEntry = mSmmCoreRootSmiHandlerList;
  for (ListEntry = ListEntry->ForwardLink;
       ListEntry != mSmmCoreRootSmiHandlerList;
       ListEntry = ListEntry->ForwardLink) {
    Size += sizeof(SMM_CORE_SMI_HANDLER_STRUCTURE);
  }
  return Size;
}

UINTN
GetSmmCoreDatabaseSize(
  VOID
  )
{
  mSmmImageDatabaseSize = GetSmmImageDatabaseSize();
  mSmmHandlerDatabaseSize = GetSmmHandlerDatabaseSize();
  mSmmProtocolDatabaseSize = GetSmmProtocolDatabaseSize();
  mSmmSmiDatabaseSize = GetSmmSmiDatabaseSize();
  mSmmRootSmiDatabaseSize = GetSmmRootSmiDatabaseSize();

  SmmChildGetDataSize(&mSmmChildDatabaseSize);

  return mSmmImageDatabaseSize + mSmmHandlerDatabaseSize + mSmmProtocolDatabaseSize + mSmmSmiDatabaseSize + mSmmRootSmiDatabaseSize + mSmmChildDatabaseSize;
}

UINTN
GetSmmImageDatabaseData (
  IN VOID  *Data,
  IN UINTN ExpectedSize
  )
{
  SMM_CORE_IMAGE_DATABASE_STRUCTURE   *ImageStruct;
  UINTN                               Size;
  UINTN                               Index;

  if (ExpectedSize / sizeof(SMM_CORE_IMAGE_DATABASE_STRUCTURE) != mImageStructCount) {
    return 0;
  }

  ImageStruct = Data;
  for (Index = 0; Index < mImageStructCount; Index++) {
    ImageStruct[Index].Header.Signature = SMM_CORE_IMAGE_DATABASE_SIGNATURE;
    ImageStruct[Index].Header.Length = sizeof(SMM_CORE_IMAGE_DATABASE_STRUCTURE);
    ImageStruct[Index].Header.Revision = SMM_CORE_IMAGE_DATABASE_REVISION;
    CopyGuid(&ImageStruct[Index].FileGuid, &mImageStruct[Index].FileGuid);
    CopyMem(ImageStruct[Index].NameString, mImageStruct[Index].NameString, sizeof(ImageStruct[Index].NameString));
    ImageStruct[Index].EntryPoint = mImageStruct[Index].EntryPoint;
    ImageStruct[Index].ImageBase = mImageStruct[Index].LoadedImageBase;
    ImageStruct[Index].ImageSize = mImageStruct[Index].ImageSize;
    ImageStruct[Index].RealImageBase = mImageStruct[Index].ImageBase;
  }

  Size = sizeof(SMM_CORE_IMAGE_DATABASE_STRUCTURE) * mImageStructCount;
  return Size;
}

UINTN
GetSmmProtocolInterfacesDataOnHandle(
  IN IHANDLE    *IHandle,
  IN VOID       *Data,
  IN UINTN      MaxSize
  )
{
  SMM_CORE_HANDLE_PROTOCOL_STRUCTURE   *HandleProtocolStruct;
  LIST_ENTRY           *ListEntry;
  PROTOCOL_INTERFACE   *ProtocolInterface;
  UINTN                Size;

  HandleProtocolStruct = Data;
  Size = 0;
  ListEntry = &IHandle->Protocols;
  for (ListEntry = ListEntry->ForwardLink;
       ListEntry != &IHandle->Protocols;
       ListEntry = ListEntry->ForwardLink) {
    ProtocolInterface = CR(ListEntry, PROTOCOL_INTERFACE, Link, PROTOCOL_INTERFACE_SIGNATURE);
    if (Size >= MaxSize) {
      DEBUG((EFI_D_ERROR, "GetSmmProtocolInterfacesDataOnHandle: Size >= MaxSize\n"));
      return 0;
    }
    if (sizeof(SMM_CORE_HANDLE_PROTOCOL_STRUCTURE) > MaxSize - Size) {
      DEBUG((EFI_D_ERROR, "GetSmmProtocolInterfacesDataOnHandle: Size > MaxSize 2\n"));
      return 0;
    }
    CopyGuid(&HandleProtocolStruct->Guid, &ProtocolInterface->Protocol->ProtocolID);
    HandleProtocolStruct->Interface = (UINTN)ProtocolInterface->Interface;
    HandleProtocolStruct->ImageRef = AddressToImageRefEx((UINTN)ProtocolInterface->Interface, &ProtocolInterface->Protocol->ProtocolID);
    Size += sizeof(SMM_CORE_HANDLE_PROTOCOL_STRUCTURE);
    HandleProtocolStruct++;
  }
  DEBUG((EFI_D_VERBOSE, "GetSmmProtocolInterfacesDataOnHandle(0x%x) - 0x%x\n", IHandle, Size));
  return Size;
}

UINTN
GetSmmHandlerDatabaseData(
  IN VOID  *Data,
  IN UINTN ExpectedSize
  )
{
  SMM_CORE_HANDLE_DATABASE_STRUCTURE  *HandleStruct;
  LIST_ENTRY      *ListEntry;
  IHANDLE         *Handle;
  UINTN           Size;

  HandleStruct = Data;
  Size = 0;
  ListEntry = mSmmCoreHandleList;
  for (ListEntry = ListEntry->ForwardLink;
       ListEntry != mSmmCoreHandleList;
       ListEntry = ListEntry->ForwardLink) {
    Handle = CR(ListEntry, IHANDLE, AllHandles, EFI_HANDLE_SIGNATURE);
    if (Size >= ExpectedSize) {
      DEBUG((EFI_D_ERROR, "GetSmmHandlerDatabaseData: Size >= MaxSize (0x%x)\n", Size));
      return 0;
    }
    if (sizeof(SMM_CORE_HANDLE_DATABASE_STRUCTURE) > ExpectedSize - Size) {
      DEBUG((EFI_D_ERROR, "GetSmmHandlerDatabaseData: Size > MaxSize 2 (0x%x)\n", Size));
      return 0;
    }
    HandleStruct->Header.Signature = SMM_CORE_HANDLE_DATABASE_SIGNATURE;
    HandleStruct->Header.Length = sizeof(SMM_CORE_HANDLE_DATABASE_STRUCTURE);
    HandleStruct->Header.Revision = SMM_CORE_HANDLE_DATABASE_REVISION;
    HandleStruct->Handle = (UINTN)Handle;
    Size += sizeof(SMM_CORE_HANDLE_DATABASE_STRUCTURE);
    HandleStruct->ProtocolCount = GetSmmProtocolInterfacesDataOnHandle(Handle, (UINT8 *)HandleStruct + HandleStruct->Header.Length, ExpectedSize-Size) / sizeof(SMM_CORE_HANDLE_PROTOCOL_STRUCTURE);
    Size += HandleStruct->ProtocolCount * sizeof(SMM_CORE_HANDLE_PROTOCOL_STRUCTURE);
    HandleStruct->Header.Length += (UINT32)(HandleStruct->ProtocolCount * sizeof(SMM_CORE_HANDLE_PROTOCOL_STRUCTURE));
    HandleStruct = (VOID *)((UINTN)HandleStruct + HandleStruct->Header.Length);
  }
  DEBUG((EFI_D_VERBOSE, "GetSmmHandlerDatabaseData - 0x%x\n", Size));
  if (Size != ExpectedSize) {
    DEBUG((EFI_D_ERROR, "GetSmmHandlerDatabaseData: Size != ExpectedSize\n"));
    return 0;
  }
  return Size;
}

UINTN
GetSmmProtocolInterfacesDataOnProtocolEntry(
  IN PROTOCOL_ENTRY  *ProtocolEntry,
  IN VOID            *Data,
  IN UINTN           MaxSize
  )
{
  SMM_CORE_PROTOCOL_INTERFACE_STRUCTURE  *ProtocolInterfaceStruct;
  LIST_ENTRY           *ListEntry;
  PROTOCOL_INTERFACE   *ProtocolInterface;
  UINTN                Size;

  ProtocolInterfaceStruct = Data;
  Size = 0;
  ListEntry = &ProtocolEntry->Protocols;
  for (ListEntry = ListEntry->ForwardLink;
       ListEntry != &ProtocolEntry->Protocols;
       ListEntry = ListEntry->ForwardLink) {
    ProtocolInterface = CR(ListEntry, PROTOCOL_INTERFACE, ByProtocol, PROTOCOL_INTERFACE_SIGNATURE);
    if (Size >= MaxSize) {
      DEBUG((EFI_D_ERROR, "GetSmmProtocolInterfacesDataOnProtocolEntry: Size >= MaxSize\n"));
      return 0;
    }
    if (sizeof(SMM_CORE_PROTOCOL_INTERFACE_STRUCTURE) > MaxSize - Size) {
      DEBUG((EFI_D_ERROR, "GetSmmProtocolInterfacesDataOnProtocolEntry: Size > MaxSize 2\n"));
      return 0;
    }
    ProtocolInterfaceStruct->Interface = (UINTN)ProtocolInterface->Interface;
    ProtocolInterfaceStruct->ImageRef = AddressToImageRefEx((UINTN)ProtocolInterface->Interface, &ProtocolInterface->Protocol->ProtocolID);
    Size += sizeof(SMM_CORE_PROTOCOL_INTERFACE_STRUCTURE);
    ProtocolInterfaceStruct++;
  }
  DEBUG((EFI_D_VERBOSE, "GetSmmProtocolInterfacesDataOnProtocolEntry(0x%x) - 0x%x\n", ProtocolEntry, Size));
  return Size;
}

UINTN
GetSmmProtocolNotifyDataOnProtocolEntry(
  IN PROTOCOL_ENTRY  *ProtocolEntry,
  IN VOID            *Data,
  IN UINTN           MaxSize
  )
{
  SMM_CORE_PROTOCOL_NOTIFY_STRUCTURE   *ProtocolNotifyStruct;
  LIST_ENTRY           *ListEntry;
  PROTOCOL_NOTIFY      *ProtocolNotify;
  UINTN                Size;

  ProtocolNotifyStruct = Data;
  Size = 0;
  ListEntry = &ProtocolEntry->Notify;
  for (ListEntry = ListEntry->ForwardLink;
       ListEntry != &ProtocolEntry->Notify;
       ListEntry = ListEntry->ForwardLink) {
    ProtocolNotify = CR(ListEntry, PROTOCOL_NOTIFY, Link, PROTOCOL_NOTIFY_SIGNATURE);
    if (Size >= MaxSize) {
      DEBUG((EFI_D_ERROR, "GetSmmProtocolNotifyDataOnProtocolEntry: Size >= MaxSize\n"));
      return 0;
    }
    if (sizeof(SMM_CORE_PROTOCOL_NOTIFY_STRUCTURE) > MaxSize - Size) {
      DEBUG((EFI_D_ERROR, "GetSmmProtocolNotifyDataOnProtocolEntry: Size > MaxSize 2\n"));
      return 0;
    }
    ProtocolNotifyStruct->Notify = (UINTN)ProtocolNotify->Function;
    ProtocolNotifyStruct->ImageRef = AddressToImageRef((UINTN)ProtocolNotify->Function);
    Size += sizeof(SMM_CORE_PROTOCOL_NOTIFY_STRUCTURE);
    ProtocolNotifyStruct++;
  }
  DEBUG((EFI_D_VERBOSE, "GetSmmProtocolNotifyDataOnProtocolEntry(0x%x) - 0x%x\n", ProtocolEntry, Size));
  return Size;
}

UINTN
GetSmmProtocolDatabaseData(
  IN VOID  *Data,
  IN UINTN ExpectedSize
  )
{
  SMM_CORE_PROTOCOL_DATABASE_STRUCTURE   *ProtocolStruct;
  LIST_ENTRY      *ListEntry;
  PROTOCOL_ENTRY  *ProtocolEntry;
  UINTN           Size;

  ProtocolStruct = Data;
  Size = 0;
  ListEntry = mSmmCoreProtocolDatabase;
  for (ListEntry = ListEntry->ForwardLink;
       ListEntry != mSmmCoreProtocolDatabase;
       ListEntry = ListEntry->ForwardLink) {
    ProtocolEntry = CR(ListEntry, PROTOCOL_ENTRY, AllEntries, PROTOCOL_ENTRY_SIGNATURE);
    if (CompareGuid(&ProtocolEntry->ProtocolID, &gEfiCallerIdGuid)) {
      // Need filter it here, because UninstallProtocol still keeps the entry
      // just leave an empty link list.
      break;
    }
    if (Size >= ExpectedSize) {
      DEBUG((EFI_D_ERROR, "GetSmmProtocolDatabaseData: Size >= MaxSize\n"));
      return 0;
    }
    if (sizeof(SMM_CORE_PROTOCOL_DATABASE_STRUCTURE) > ExpectedSize - Size) {
      DEBUG((EFI_D_ERROR, "GetSmmProtocolDatabaseData: Size > MaxSize 2\n"));
      return 0;
    }
    ProtocolStruct->Header.Signature = SMM_CORE_PROTOCOL_DATABASE_SIGNATURE;
    ProtocolStruct->Header.Length = sizeof(SMM_CORE_PROTOCOL_DATABASE_STRUCTURE);
    ProtocolStruct->Header.Revision = SMM_CORE_PROTOCOL_DATABASE_REVISION;
    CopyGuid(&ProtocolStruct->Guid, &ProtocolEntry->ProtocolID);
    Size += sizeof(SMM_CORE_PROTOCOL_DATABASE_STRUCTURE);
    ProtocolStruct->InterfaceCount = GetSmmProtocolInterfacesDataOnProtocolEntry(ProtocolEntry, (UINT8 *)ProtocolStruct + ProtocolStruct->Header.Length, ExpectedSize - Size) / sizeof(SMM_CORE_PROTOCOL_INTERFACE_STRUCTURE);
    Size += ProtocolStruct->InterfaceCount * sizeof(SMM_CORE_PROTOCOL_INTERFACE_STRUCTURE);
    ProtocolStruct->Header.Length += (UINT32)(ProtocolStruct->InterfaceCount * sizeof(SMM_CORE_PROTOCOL_INTERFACE_STRUCTURE));
    ProtocolStruct->NotifyCount = GetSmmProtocolNotifyDataOnProtocolEntry(ProtocolEntry, (UINT8 *)ProtocolStruct + ProtocolStruct->Header.Length, ExpectedSize - Size) / sizeof(SMM_CORE_PROTOCOL_NOTIFY_STRUCTURE);
    Size += ProtocolStruct->NotifyCount * sizeof(SMM_CORE_PROTOCOL_NOTIFY_STRUCTURE);
    ProtocolStruct->Header.Length += (UINT32)(ProtocolStruct->NotifyCount * sizeof(SMM_CORE_PROTOCOL_NOTIFY_STRUCTURE));
    ProtocolStruct = (VOID *)((UINTN)ProtocolStruct + ProtocolStruct->Header.Length);
  }
  DEBUG((EFI_D_VERBOSE, "GetSmmProtocolDatabaseData - 0x%x\n", Size));

  if (Size != ExpectedSize) {
    DEBUG((EFI_D_ERROR, "GetSmmProtocolDatabaseData: Size != ExpectedSize\n"));
    return 0;
  }
  return Size;
}

UINTN
GetSmmSmiHandlerDataOnSmiEntry(
  IN SMI_ENTRY       *SmiEntry,
  IN VOID            *Data,
  IN UINTN           MaxSize
  )
{
  SMM_CORE_SMI_HANDLER_STRUCTURE   *SmiHandlerStruct;
  LIST_ENTRY      *ListEntry;
  SMI_HANDLER     *SmiHandler;
  UINTN           Size;

  SmiHandlerStruct = Data;
  Size = 0;
  ListEntry = &SmiEntry->SmiHandlers;
  for (ListEntry = ListEntry->ForwardLink;
       ListEntry != &SmiEntry->SmiHandlers;
       ListEntry = ListEntry->ForwardLink) {
    SmiHandler = CR(ListEntry, SMI_HANDLER, Link, SMI_HANDLER_SIGNATURE);
    if (Size >= MaxSize) {
      return 0;
    }
    if (sizeof(SMM_CORE_SMI_HANDLER_STRUCTURE) > MaxSize - Size) {
      return 0;
    }
    SmiHandlerStruct->SmiHandle = (UINTN)SmiHandler;
    SmiHandlerStruct->Handler = (UINTN)SmiHandler->Handler;
    SmiHandlerStruct->ImageRef = AddressToImageRef((UINTN)SmiHandler->Handler);
    Size += sizeof(SMM_CORE_SMI_HANDLER_STRUCTURE);
    SmiHandlerStruct++;
  }

  return Size;
}

UINTN
GetSmmSmiDatabaseData(
  IN VOID  *Data,
  IN UINTN ExpectedSize
  )
{
  SMM_CORE_SMI_DATABASE_STRUCTURE   *SmiStruct;
  LIST_ENTRY      *ListEntry;
  SMI_ENTRY       *SmiEntry;
  UINTN           Size;

  SmiStruct = Data;
  Size = 0;
  ListEntry = mSmmCoreSmiEntryList;
  for (ListEntry = ListEntry->ForwardLink;
       ListEntry != mSmmCoreSmiEntryList;
       ListEntry = ListEntry->ForwardLink) {
    SmiEntry = CR(ListEntry, SMI_ENTRY, AllEntries, SMI_ENTRY_SIGNATURE);
    if (Size >= ExpectedSize) {
      return 0;
    }
    if (sizeof(SMM_CORE_SMI_DATABASE_STRUCTURE) > ExpectedSize - Size) {
      return 0;
    }

    SmiStruct->Header.Signature = SMM_CORE_SMI_DATABASE_SIGNATURE;
    SmiStruct->Header.Length = sizeof(SMM_CORE_SMI_DATABASE_STRUCTURE);
    SmiStruct->Header.Revision = SMM_CORE_SMI_DATABASE_REVISION;
    SmiStruct->IsRootHandler = FALSE;
    CopyGuid(&SmiStruct->HandlerType, &SmiEntry->HandlerType);
    Size += sizeof(SMM_CORE_SMI_DATABASE_STRUCTURE);
    SmiStruct->HandlerCount = GetSmmSmiHandlerDataOnSmiEntry(SmiEntry, (UINT8 *)SmiStruct + SmiStruct->Header.Length, ExpectedSize - Size) / sizeof(SMM_CORE_SMI_HANDLER_STRUCTURE);
    Size += SmiStruct->HandlerCount * sizeof(SMM_CORE_SMI_HANDLER_STRUCTURE);
    SmiStruct->Header.Length += (UINT32)(SmiStruct->HandlerCount * sizeof(SMM_CORE_SMI_HANDLER_STRUCTURE));
    SmiStruct = (VOID *)((UINTN)SmiStruct + SmiStruct->Header.Length);
  }
  if (ExpectedSize != Size) {
    return 0;
  }
  return Size;
}

UINTN
GetSmmRootSmiDatabaseData(
  IN VOID  *Data,
  IN UINTN ExpectedSize
  )
{
  SMM_CORE_SMI_DATABASE_STRUCTURE   *SmiStruct;
  SMM_CORE_SMI_HANDLER_STRUCTURE    *SmiHandlerStruct;
  LIST_ENTRY      *ListEntry;
  SMI_HANDLER     *SmiHandler;
  UINTN           Size;

  Size = 0;
  SmiStruct = Data;
  SmiStruct->Header.Signature = SMM_CORE_SMI_DATABASE_SIGNATURE;
  SmiStruct->Header.Length = sizeof(SMM_CORE_SMI_DATABASE_STRUCTURE);
  SmiStruct->Header.Revision = SMM_CORE_SMI_DATABASE_REVISION;
  SmiStruct->IsRootHandler = TRUE;
  ZeroMem(&SmiStruct->HandlerType, sizeof(SmiStruct->HandlerType));
  SmiStruct->HandlerCount = 0;
  Size += sizeof(SMM_CORE_SMI_DATABASE_STRUCTURE);

  SmiHandlerStruct = (VOID *)((UINT8 *)SmiStruct + SmiStruct->Header.Length);
  ListEntry = mSmmCoreRootSmiHandlerList;
  for (ListEntry = ListEntry->ForwardLink;
       ListEntry != mSmmCoreRootSmiHandlerList;
       ListEntry = ListEntry->ForwardLink) {
    SmiHandler = CR(ListEntry, SMI_HANDLER, Link, SMI_HANDLER_SIGNATURE);
    if (Size >= ExpectedSize) {
      return 0;
    }
    if (sizeof(SMM_CORE_SMI_HANDLER_STRUCTURE) > ExpectedSize - Size) {
      return 0;
    }
    SmiHandlerStruct->SmiHandle = (UINTN)SmiHandler;
    SmiHandlerStruct->Handler = (UINTN)SmiHandler->Handler;
    SmiHandlerStruct->ImageRef = AddressToImageRef((UINTN)SmiHandler->Handler);
    SmiStruct->HandlerCount++;
    SmiStruct->Header.Length += sizeof(SMM_CORE_SMI_HANDLER_STRUCTURE);
    Size += sizeof(SMM_CORE_SMI_HANDLER_STRUCTURE);
    SmiHandlerStruct++;
  }
  if (ExpectedSize != Size) {
    return 0;
  }
  return Size;
}

EFI_STATUS
GetSmmCoreDatabaseData(
  IN VOID *Data
  )
{
  UINTN  SmmImageDatabaseSize;
  UINTN  SmmHandlerDatabaseSize;
  UINTN  SmmProtocolDatabaseSize;
  UINTN  SmmSmiDatabaseSize;
  UINTN  SmmRootSmiDatabaseSize;

  DEBUG((EFI_D_VERBOSE, "GetSmmCoreDatabaseData\n"));
  SmmImageDatabaseSize = GetSmmImageDatabaseData(Data, mSmmImageDatabaseSize);
  if (SmmImageDatabaseSize != mSmmImageDatabaseSize) {
    DEBUG((EFI_D_ERROR, "GetSmmCoreDatabaseData - SmmImageDatabaseSize mismatch!\n"));
    return EFI_INVALID_PARAMETER;
  }
  SmmHandlerDatabaseSize = GetSmmHandlerDatabaseData((UINT8 *)Data + SmmImageDatabaseSize, mSmmHandlerDatabaseSize);
  if (SmmHandlerDatabaseSize != mSmmHandlerDatabaseSize) {
    DEBUG((EFI_D_ERROR, "GetSmmCoreDatabaseData - SmmHandlerDatabaseSize mismatch!\n"));
    return EFI_INVALID_PARAMETER;
  }
  SmmProtocolDatabaseSize = GetSmmProtocolDatabaseData((UINT8 *)Data + SmmImageDatabaseSize + SmmHandlerDatabaseSize, mSmmProtocolDatabaseSize);
  if (SmmProtocolDatabaseSize != mSmmProtocolDatabaseSize) {
    DEBUG((EFI_D_ERROR, "GetSmmCoreDatabaseData - SmmProtocolDatabaseSize mismatch!\n"));
    return EFI_INVALID_PARAMETER;
  }
  SmmSmiDatabaseSize = GetSmmSmiDatabaseData((UINT8 *)Data + SmmImageDatabaseSize + SmmHandlerDatabaseSize + SmmProtocolDatabaseSize, mSmmSmiDatabaseSize);
  if (SmmSmiDatabaseSize != mSmmSmiDatabaseSize) {
    DEBUG((EFI_D_ERROR, "GetSmmCoreDatabaseData - SmmSmiDatabaseSize mismatch!\n"));
    return EFI_INVALID_PARAMETER;
  }
  SmmRootSmiDatabaseSize = GetSmmRootSmiDatabaseData((UINT8 *)Data + SmmImageDatabaseSize + SmmHandlerDatabaseSize + SmmProtocolDatabaseSize + SmmSmiDatabaseSize, mSmmRootSmiDatabaseSize);
  if (SmmRootSmiDatabaseSize != mSmmRootSmiDatabaseSize) {
    DEBUG((EFI_D_ERROR, "GetSmmCoreDatabaseData - SmmRootSmiDatabaseSize mismatch!\n"));
    return EFI_INVALID_PARAMETER;
  }

  DEBUG((EFI_D_VERBOSE, "GetSmmCoreDatabaseData - mSmmChildDatabaseSize - 0x%x\n", mSmmChildDatabaseSize));
  if (mSmmChildDatabaseSize != 0) {
    SmmChildGetData(mSmmChildDatabaseSize, (UINT8 *)Data + SmmImageDatabaseSize + SmmHandlerDatabaseSize + SmmProtocolDatabaseSize + SmmSmiDatabaseSize + mSmmRootSmiDatabaseSize);
  }

  return EFI_SUCCESS;
}

VOID
BuildSmmCoreDatabase(
  VOID
  )
{
  EFI_STATUS  Status;
  mSmmCoreDatabaseSize = GetSmmCoreDatabaseSize();
  mSmmCoreDatabase = AllocatePool(mSmmCoreDatabaseSize);
  if (mSmmCoreDatabase == NULL) {
    return;
  }
  Status = GetSmmCoreDatabaseData(mSmmCoreDatabase);
  if (EFI_ERROR(Status)) {
    FreePool(mSmmCoreDatabase);
    mSmmCoreDatabase = NULL;
  }
}

/**
  Copy SMM core dump data.

  @param DumpBuffer  The buffer to hold SMM core dump data.
  @param DumpSize    On input, dump buffer size.
                     On output, actual dump data size copied.
  @param DumpOffset  On input, dump buffer offset to copy.
                     On output, next time dump buffer offset to copy.

**/
VOID
SmmCoreDumpCopyData(
  OUT VOID      *DumpBuffer,
  IN OUT UINT64 *DumpSize,
  IN OUT UINT64 *DumpOffset
  )
{
  if (*DumpOffset >= mSmmCoreDatabaseSize) {
    *DumpOffset = mSmmCoreDatabaseSize;
    return;
  }
  if (mSmmCoreDatabaseSize - *DumpOffset < *DumpSize) {
    *DumpSize = mSmmCoreDatabaseSize - *DumpOffset;
  }

  CopyMem(
    DumpBuffer,
    (UINT8 *)mSmmCoreDatabase + *DumpOffset,
    (UINTN)*DumpSize
    );
  *DumpOffset = *DumpOffset + *DumpSize;
}

/**
  SMM core dump handler to get info.

  @param SmmCoreDumpParameterGetInfo The parameter of SMM core dump get size.

**/
VOID
SmmCoreDumpHandlerGetInfo(
  IN SMM_CORE_DUMP_PARAMETER_GET_INFO   *SmmCoreDumpParameterGetInfo
  )
{
  BOOLEAN                       SmmCoreDumpRecordingStatus;

  SmmCoreDumpRecordingStatus = mSmmCoreDumpRecordingStatus;
  mSmmCoreDumpRecordingStatus = FALSE;

  SmmCoreDumpParameterGetInfo->DumpSize = mSmmCoreDatabaseSize;
  SmmCoreDumpParameterGetInfo->Header.ReturnStatus = 0;

  mSmmCoreDumpRecordingStatus = SmmCoreDumpRecordingStatus;
}

/**
  SMM core dump handler to get data by offset.

  @param SmmCoreDumpParameterGetDataByOffset   The parameter of SMM core dump get data by offset.

**/
VOID
SmmCoreDumpHandlerGetDataByOffset(
  IN SMM_CORE_DUMP_PARAMETER_GET_DATA_BY_OFFSET     *SmmCoreDumpParameterGetDataByOffset
  )
{
  SMM_CORE_DUMP_PARAMETER_GET_DATA_BY_OFFSET    SmmCoreDumpGetDataByOffset;
  BOOLEAN                                       SmmCoreDumpRecordingStatus;

  SmmCoreDumpRecordingStatus = mSmmCoreDumpRecordingStatus;
  mSmmCoreDumpRecordingStatus = FALSE;
  
  CopyMem(&SmmCoreDumpGetDataByOffset, SmmCoreDumpParameterGetDataByOffset, sizeof(SmmCoreDumpGetDataByOffset));

  //
  // Sanity check
  //
  if (!SmmIsBufferOutsideSmmValid((UINTN)SmmCoreDumpGetDataByOffset.DumpBuffer, (UINTN)SmmCoreDumpGetDataByOffset.DumpSize)) {
    DEBUG((EFI_D_ERROR, "SmmCoreDumpHandlerGetDataByOffset: SMM core dump data in SMRAM or overflow!\n"));
    SmmCoreDumpParameterGetDataByOffset->Header.ReturnStatus = (UINT64)(INT64)(INTN)EFI_ACCESS_DENIED;
    goto Done;
  }

  SmmCoreDumpCopyData((VOID *)(UINTN)SmmCoreDumpGetDataByOffset.DumpBuffer, &SmmCoreDumpGetDataByOffset.DumpSize, &SmmCoreDumpGetDataByOffset.DumpOffset);
  CopyMem(SmmCoreDumpParameterGetDataByOffset, &SmmCoreDumpGetDataByOffset, sizeof(SmmCoreDumpGetDataByOffset));
  SmmCoreDumpParameterGetDataByOffset->Header.ReturnStatus = 0;

Done:
  mSmmCoreDumpRecordingStatus = SmmCoreDumpRecordingStatus;
}

/**
  Dispatch function for a Software SMI handler.

  Caution: This function may receive untrusted input.
  Communicate buffer and buffer size are external input, so this function will do basic validation.

  @param DispatchHandle  The unique handle assigned to this handler by SmiHandlerRegister().
  @param Context         Points to an optional handler context which was specified when the
                         handler was registered.
  @param CommBuffer      A pointer to a collection of data in memory that will
                         be conveyed from a non-SMM environment into an SMM environment.
  @param CommBufferSize  The size of the CommBuffer.

  @retval EFI_SUCCESS Command is handled successfully.
**/
EFI_STATUS
EFIAPI
SmmCoreDumpHandler(
  IN EFI_HANDLE  DispatchHandle,
  IN CONST VOID  *Context         OPTIONAL,
  IN OUT VOID    *CommBuffer      OPTIONAL,
  IN OUT UINTN   *CommBufferSize  OPTIONAL
  )
{
  SMM_CORE_DUMP_PARAMETER_HEADER           *SmmCoreDumpParameterHeader;
  UINTN                                    TempCommBufferSize;

  DEBUG((EFI_D_ERROR, "SmmCoreDumpHandler Enter\n"));

  if (mSmmCoreDatabase == NULL) {
    return EFI_SUCCESS;
  }

  //
  // If input is invalid, stop processing this SMI
  //
  if (CommBuffer == NULL || CommBufferSize == NULL) {
    return EFI_SUCCESS;
  }

  TempCommBufferSize = *CommBufferSize;

  if (TempCommBufferSize < sizeof(SMM_CORE_DUMP_PARAMETER_HEADER)) {
    DEBUG((EFI_D_ERROR, "SmmCoreDumpHandler: SMM communication buffer size invalid!\n"));
    return EFI_SUCCESS;
  }

  if (!SmmIsBufferOutsideSmmValid((UINTN)CommBuffer, TempCommBufferSize)) {
    DEBUG((EFI_D_ERROR, "SmmCoreDumpHandler: SMM communication buffer in SMRAM or overflow!\n"));
    return EFI_SUCCESS;
  }

  SmmCoreDumpParameterHeader = (SMM_CORE_DUMP_PARAMETER_HEADER *)((UINTN)CommBuffer);
  SmmCoreDumpParameterHeader->ReturnStatus = (UINT64)-1;

  switch (SmmCoreDumpParameterHeader->Command) {
  case SMM_CORE_DUMP_COMMAND_GET_INFO:
    DEBUG((EFI_D_ERROR, "SmmCoreDumpHandlerGetInfo\n"));
    if (TempCommBufferSize != sizeof(SMM_CORE_DUMP_PARAMETER_GET_INFO)) {
      DEBUG((EFI_D_ERROR, "SmmCoreDumpHandler: SMM communication buffer size invalid!\n"));
      return EFI_SUCCESS;
    }
    SmmCoreDumpHandlerGetInfo((SMM_CORE_DUMP_PARAMETER_GET_INFO *)(UINTN)CommBuffer);
    break;
  case SMM_CORE_DUMP_COMMAND_GET_DATA_BY_OFFSET:
    DEBUG((EFI_D_ERROR, "SmmCoreDumpHandlerGetDataByOffset\n"));
    if (TempCommBufferSize != sizeof(SMM_CORE_DUMP_PARAMETER_GET_DATA_BY_OFFSET)) {
      DEBUG((EFI_D_ERROR, "SmmCoreDumpHandler: SMM communication buffer size invalid!\n"));
      return EFI_SUCCESS;
    }
    SmmCoreDumpHandlerGetDataByOffset((SMM_CORE_DUMP_PARAMETER_GET_DATA_BY_OFFSET *)(UINTN)CommBuffer);
    break;
  default:
    break;
  }

  DEBUG((EFI_D_ERROR, "SmmCoreDumpHandler Exit\n"));

  return EFI_SUCCESS;
}

/**
  Register SmmCore dump handler.

**/
VOID
RegisterSmmCoreDumpHandler (
  VOID
  )
{
  EFI_STATUS    Status;
  EFI_HANDLE    DispatchHandle;

  BuildSmmCoreDatabase();

  Status = gSmst->SmiHandlerRegister (
                    SmmCoreDumpHandler,
                    &mSmmCoreDumpGuid,
                    &DispatchHandle
                    );
  ASSERT_EFI_ERROR (Status);
}
