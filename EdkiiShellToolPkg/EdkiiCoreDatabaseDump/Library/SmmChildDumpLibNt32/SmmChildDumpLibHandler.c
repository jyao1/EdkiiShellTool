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

#include <Protocol/SmmSwDispatch2.h>
#include <Protocol/SmmSxDispatch2.h>
#include <Protocol/SmmPeriodicTimerDispatch2.h>
#include <Protocol/SmmUsbDispatch2.h>
#include <Protocol/SmmGpiDispatch2.h>
#include <Protocol/SmmStandbyButtonDispatch2.h>
#include <Protocol/SmmPowerButtonDispatch2.h>
#include <Protocol/SmmIoTrapDispatch2.h>

#include <Library/SmmChildDumpLib.h>
//
// This structure is copied from SmiChildDispatcher
//
#include "SmmChild/WinNtSmmChildDispatcherSmm.h"

#include "SmmChildDump.h"
#include "SmmChildDumpComm.h"

extern LIST_ENTRY      *mSmmChildCallbackDatabase;

extern EFI_GUID  *mProtocolTypeToGuid[];

UINTN  mSmmChildSwCallbackDatabaseSize;
UINTN  mSmmChildSxCallbackDatabaseSize;
UINTN  mSmmChildPeriodicTimerCallbackDatabaseSize;
UINTN  mSmmChildUsbCallbackDatabaseSize;
UINTN  mSmmChildGpiCallbackDatabaseSize;
UINTN  mSmmChildPowerButtonCallbackDatabaseSize;
UINTN  mSmmChildStandbyButtonCallbackDatabaseSize;
UINTN  mSmmChildIoTrapCallbackDatabaseSize;

typedef struct {
  EFI_GUID               *Guid;
  UINTN                  Size;
} SMM_CHILD_HANDLER_STRUCTURE_SIZE_BY_TYPE;

SMM_CHILD_HANDLER_STRUCTURE_SIZE_BY_TYPE mSmmChildHandlerStructureSize[] = {
  { &gEfiSmmSwDispatch2ProtocolGuid, sizeof(SMM_CHILD_SW_SMI_HANDLER_STRUCTURE)},
  { &gEfiSmmSxDispatch2ProtocolGuid, sizeof(SMM_CHILD_SX_SMI_HANDLER_STRUCTURE)},
  { &gEfiSmmPeriodicTimerDispatch2ProtocolGuid, sizeof(SMM_CHILD_PERIODIC_TIMER_SMI_HANDLER_STRUCTURE)},
  { &gEfiSmmUsbDispatch2ProtocolGuid, sizeof(SMM_CHILD_USB_SMI_HANDLER_STRUCTURE)},
  { &gEfiSmmGpiDispatch2ProtocolGuid, sizeof(SMM_CHILD_GPI_SMI_HANDLER_STRUCTURE)},
  { &gEfiSmmPowerButtonDispatch2ProtocolGuid, sizeof(SMM_CHILD_POWER_BUTTON_SMI_HANDLER_STRUCTURE)},
  { &gEfiSmmStandbyButtonDispatch2ProtocolGuid, sizeof(SMM_CHILD_STANDBY_BUTTON_SMI_HANDLER_STRUCTURE)},
  { &gEfiSmmIoTrapDispatch2ProtocolGuid, sizeof(SMM_CHILD_IO_TRAP_SMI_HANDLER_STRUCTURE) },
};

UINTN
SmmChildHandlerStructureSizeByGuid(
  IN EFI_GUID  *Guid
  )
{
  UINTN     Index;

  for (Index = 0; Index < sizeof(mSmmChildHandlerStructureSize) / sizeof(mSmmChildHandlerStructureSize[0]); Index++) {
    if (CompareGuid(Guid, mSmmChildHandlerStructureSize[Index].Guid)) {
      return mSmmChildHandlerStructureSize[Index].Size;
    }
  }

  return 0;
}

UINTN
SmmChildHandlerStructureSizeByType(
  IN INT32  ProtocolType
  )
{
  EFI_GUID  *Guid;

  Guid = ProtocolTypeToGuid(ProtocolType);
  if (Guid != NULL) {
    return SmmChildHandlerStructureSizeByGuid(Guid);
  }
  return 0;
}

UINTN
GetSmmChildCallbackDatabaseSize(
  IN INT32  ProtocolType
  )
{
  LIST_ENTRY      *ListEntry;
  //DATABASE_RECORD *Record;
  UINTN           Count;
  UINTN           Size;

  Count = 0;
  ListEntry = mSmmChildCallbackDatabase;
  for (ListEntry = ListEntry->ForwardLink;
       ListEntry != mSmmChildCallbackDatabase;
       ListEntry = ListEntry->ForwardLink) {
    //Record = CR(ListEntry, DATABASE_RECORD, Link, DATABASE_RECORD_SIGNATURE);
    //if (Record->ProtocolType == ProtocolType) {
      Count++;
    //}
  }

  Size = sizeof(SMM_CHILD_SMI_DATABASE_STRUCTURE) + Count * SmmChildHandlerStructureSizeByType(ProtocolType);
  DEBUG((EFI_D_VERBOSE, "count - %d, StructSize - 0x%x\n", Count, SmmChildHandlerStructureSizeByType(ProtocolType)));
  DEBUG((EFI_D_VERBOSE, "GetSmmChildCallbackDatabaseSize(%d) - 0x%x\n", ProtocolType, Size));
  return Size;
}

UINTN
GetSmmChildSwCallbackDatabaseSize(
  VOID
  )
{
  UINTN Size;
  Size = GetSmmChildCallbackDatabaseSize(0);
  return Size;
}

UINTN
GetSmmChildSxCallbackDatabaseSize(
  VOID
  )
{
  return 0;
}

UINTN
GetSmmChildPeriodicTimerCallbackDatabaseSize(
  VOID
  )
{
  return 0;
}

UINTN
GetSmmChildUsbCallbackDatabaseSize(
  VOID
  )
{
  return 0;
}

UINTN
GetSmmChildGpiCallbackDatabaseSize(
  VOID
  )
{
  return 0;
}

UINTN
GetSmmChildPowerButtonCallbackDatabaseSize(
  VOID
  )
{
  return 0;
}

UINTN
GetSmmChildStandbyButtonCallbackDatabaseSize(
  VOID
  )
{
  return 0;
}

UINTN
GetSmmChildIoTrapCallbackDatabaseSize(
  VOID
  )
{
  return 0;
}

EFI_STATUS
EFIAPI
SmmChildGetDataSize(
  OUT UINTN  *Size
  )
{
  mSmmChildSwCallbackDatabaseSize = GetSmmChildSwCallbackDatabaseSize();
  mSmmChildSxCallbackDatabaseSize = GetSmmChildSxCallbackDatabaseSize();
  mSmmChildPeriodicTimerCallbackDatabaseSize = GetSmmChildPeriodicTimerCallbackDatabaseSize();
  mSmmChildUsbCallbackDatabaseSize = GetSmmChildUsbCallbackDatabaseSize();
  mSmmChildGpiCallbackDatabaseSize = GetSmmChildGpiCallbackDatabaseSize();
  mSmmChildPowerButtonCallbackDatabaseSize = GetSmmChildPowerButtonCallbackDatabaseSize();
  mSmmChildStandbyButtonCallbackDatabaseSize = GetSmmChildStandbyButtonCallbackDatabaseSize();
  mSmmChildIoTrapCallbackDatabaseSize = GetSmmChildIoTrapCallbackDatabaseSize();

  DEBUG((EFI_D_INFO, "SmmChildGetDataSize\n"));
  DEBUG((EFI_D_VERBOSE, "  SmmChildSwCallbackDatabaseSize - 0x%x\n", mSmmChildSwCallbackDatabaseSize));
  DEBUG((EFI_D_VERBOSE, "  SmmChildSxCallbackDatabaseSize - 0x%x\n", mSmmChildSxCallbackDatabaseSize));
  DEBUG((EFI_D_VERBOSE, "  SmmChildPeriodicTimerCallbackDatabaseSize - 0x%x\n", mSmmChildPeriodicTimerCallbackDatabaseSize));
  DEBUG((EFI_D_VERBOSE, "  SmmChildUsbCallbackDatabaseSize - 0x%x\n", mSmmChildUsbCallbackDatabaseSize));
  DEBUG((EFI_D_VERBOSE, "  SmmChildGpiCallbackDatabaseSize - 0x%x\n", mSmmChildGpiCallbackDatabaseSize));
  DEBUG((EFI_D_VERBOSE, "  SmmChildPowerButtonCallbackDatabaseSize - 0x%x\n", mSmmChildPowerButtonCallbackDatabaseSize));
  DEBUG((EFI_D_VERBOSE, "  SmmChildStandbyButtonCallbackDatabaseSize - 0x%x\n", mSmmChildStandbyButtonCallbackDatabaseSize));
  DEBUG((EFI_D_VERBOSE, "  SmmChildIoTrapCallbackDatabaseSize - 0x%x\n", mSmmChildIoTrapCallbackDatabaseSize));

  *Size = mSmmChildSwCallbackDatabaseSize +
          mSmmChildSxCallbackDatabaseSize + 
          mSmmChildPeriodicTimerCallbackDatabaseSize + 
          mSmmChildUsbCallbackDatabaseSize +
          mSmmChildGpiCallbackDatabaseSize +
          mSmmChildPowerButtonCallbackDatabaseSize +
          mSmmChildStandbyButtonCallbackDatabaseSize +
          mSmmChildIoTrapCallbackDatabaseSize;
  DEBUG((EFI_D_VERBOSE, "*Size - 0x%x\n", *Size));
  return EFI_SUCCESS;
}

VOID
GetSmmChildCallbackContextData(
  IN EFI_GUID  *Guid,
  IN VOID      *Context,
  IN OUT VOID  *Data
  )
{
  EFI_SMM_USB_REGISTER_CONTEXT            *Usb;
  EFI_SMM_USB_REGISTER_CONTEXT            *UsbStruct;
  UINTN                                   DevicePathSize;

  if (CompareGuid(Guid, &gEfiSmmSwDispatch2ProtocolGuid)) {
    CopyMem(Data, Context, sizeof(EFI_SMM_SW_REGISTER_CONTEXT));
  } else if (CompareGuid(Guid, &gEfiSmmSxDispatch2ProtocolGuid)) {
    CopyMem(Data, Context, sizeof(EFI_SMM_SX_REGISTER_CONTEXT));
  } else if (CompareGuid(Guid, &gEfiSmmPeriodicTimerDispatch2ProtocolGuid)) {
    CopyMem(Data, Context, sizeof(EFI_SMM_PERIODIC_TIMER_REGISTER_CONTEXT));
  } else if (CompareGuid(Guid, &gEfiSmmUsbDispatch2ProtocolGuid)) {
    Usb = Context;
    UsbStruct = Data;

    UsbStruct->Type = Usb->Type;
    ZeroMem(UsbStruct->Device, sizeof(UsbStruct->Device));
    if (IsDevicePathValid(Usb->Device, sizeof(UsbStruct->Device))) {
      DevicePathSize = GetDevicePathSize(Usb->Device);
      if (DevicePathSize <= sizeof(UsbStruct->Device)) {
        CopyMem(UsbStruct->Device, Usb->Device, DevicePathSize);
      }
    }
  } else if (CompareGuid(Guid, &gEfiSmmGpiDispatch2ProtocolGuid)) {
    CopyMem(Data, Context, sizeof(EFI_SMM_GPI_REGISTER_CONTEXT));
  } else if (CompareGuid(Guid, &gEfiSmmStandbyButtonDispatch2ProtocolGuid)) {
    CopyMem(Data, Context, sizeof(EFI_SMM_STANDBY_BUTTON_REGISTER_CONTEXT));
  } else if (CompareGuid(Guid, &gEfiSmmPowerButtonDispatch2ProtocolGuid)) {
    CopyMem(Data, Context, sizeof(EFI_SMM_POWER_BUTTON_REGISTER_CONTEXT));
  } else if (CompareGuid(Guid, &gEfiSmmIoTrapDispatch2ProtocolGuid)) {
    CopyMem (Data, Context, sizeof(EFI_SMM_IO_TRAP_REGISTER_CONTEXT));
  }
}

UINTN
GetSmmChildCallbackDatabaseData(
  IN INT32                  ProtocolType,
  IN VOID                   *Data,
  IN UINTN                  ExpectedSize
  )
{
  SMM_CHILD_SMI_DATABASE_STRUCTURE         *SmiDatabase;
  SMM_CHILD_SMI_HANDLER_STRUCTURE_HEADER   *SmiStruct;
  LIST_ENTRY      *ListEntry;
  DATABASE_RECORD *Record;
  UINTN           StructSize;
  UINTN           Size;
  EFI_GUID        *Guid;

  StructSize = SmmChildHandlerStructureSizeByType(ProtocolType);
  Guid = ProtocolTypeToGuid(ProtocolType);

  if (ExpectedSize < sizeof(SMM_CHILD_SMI_DATABASE_STRUCTURE)) {
    return 0;
  }

  SmiDatabase = Data;
  SmiDatabase->Header.Signature = SMM_CHILD_SMI_DATABASE_SIGNATURE;
  SmiDatabase->Header.Revision = SMM_CHILD_SMI_DATABASE_REVISION;
  if (Guid != NULL) {
    CopyGuid(&SmiDatabase->Guid, Guid);
  } else {
    return 0;
  }
  SmiDatabase->HandlerCount = 0;
  SmiStruct = (VOID *)(SmiDatabase + 1);
  Size = sizeof(SMM_CHILD_SMI_DATABASE_STRUCTURE);

  ListEntry = mSmmChildCallbackDatabase;
  for (ListEntry = ListEntry->ForwardLink;
       ListEntry != mSmmChildCallbackDatabase;
       ListEntry = ListEntry->ForwardLink) {
    Record = CR(ListEntry, DATABASE_RECORD, Link, DATABASE_RECORD_SIGNATURE);
    //if (Record->ProtocolType == ProtocolType) {
      if (Size >= ExpectedSize) {
        DEBUG((EFI_D_ERROR, "GetSmmChildCallbackDatabaseData: Size >= MaxSize (0x%x)\n", Size));
        return 0;
      }
      if (StructSize > ExpectedSize - Size) {
        DEBUG((EFI_D_ERROR, "GetSmmChildCallbackDatabaseData: Size > MaxSize 2 (0x%x)\n", Size));
        return 0;
      }
      SmiStruct->SmiHandle = (UINTN)ListEntry;
      if (Guid != NULL) {
        SmiStruct->Handler = (UINTN)Record->DispatchFunction;
        SmiStruct->ImageRef = AddressToImageRef((UINTN)Record->DispatchFunction);
        GetSmmChildCallbackContextData(Guid, &Record->RegisterContext, (SmiStruct + 1));
      }
      SmiStruct = (VOID *)((UINTN)SmiStruct + StructSize);
      SmiDatabase->HandlerCount++;
      Size += StructSize;
    //}
  }
  DEBUG((EFI_D_VERBOSE, "GetSmmChildCallbackDatabaseData(%d) - 0x%x\n", ProtocolType, Size));
  SmiDatabase->Header.Length = (UINT32)Size;
  if (Size != ExpectedSize) {
    DEBUG((EFI_D_ERROR, "GetSmmChildCallbackDatabaseData(%d): Size != ExpectedSize\n", ProtocolType));
    return 0;
  }
  return Size;
}

UINTN
GetSmmChildSwCallbackDatabaseData(
  IN VOID  *Data,
  IN UINTN ExpectedSize
  )
{
  UINTN  Size;
  Size = GetSmmChildCallbackDatabaseData(0, Data, ExpectedSize);
  return Size;
}

UINTN
GetSmmChildSxCallbackDatabaseData(
  IN VOID  *Data,
  IN UINTN ExpectedSize
  )
{
  return 0;
}

UINTN
GetSmmChildPeriodicTimerCallbackDatabaseData(
  IN VOID  *Data,
  IN UINTN ExpectedSize
  )
{
  return 0;
}

UINTN
GetSmmChildUsbCallbackDatabaseData(
  IN VOID  *Data,
  IN UINTN ExpectedSize
  )
{
  return 0;
}

UINTN
GetSmmChildGpiCallbackDatabaseData(
  IN VOID  *Data,
  IN UINTN ExpectedSize
  )
{
  return 0;
}

UINTN
GetSmmChildPowerButtonCallbackDatabaseData(
  IN VOID  *Data,
  IN UINTN ExpectedSize
  )
{
  return 0;
}

UINTN
GetSmmChildStandbyButtonCallbackDatabaseData(
  IN VOID  *Data,
  IN UINTN ExpectedSize
  )
{
  return 0;
}

UINTN
GetSmmChildIoTrapCallbackDatabaseData(
  IN VOID  *Data,
  IN UINTN ExpectedSize
  )
{
  return 0;
}

EFI_STATUS
EFIAPI
SmmChildGetData(
  IN     UINTN  Size,
  IN OUT VOID   *Data
  )
{
  UINTN  SmmChildSwCallbackDatabaseSize;
  UINTN  SmmChildSxCallbackDatabaseSize;
  UINTN  SmmChildPeriodicTimerCallbackDatabaseSize;
  UINTN  SmmChildUsbCallbackDatabaseSize;
  UINTN  SmmChildGpiCallbackDatabaseSize;
  UINTN  SmmChildPowerButtonCallbackDatabaseSize;
  UINTN  SmmChildStandbyButtonCallbackDatabaseSize;
  UINTN  SmmChildIoTrapCallbackDatabaseSize;

  DEBUG((EFI_D_INFO, "SmmChildGetData\n"));

  SmmChildSwCallbackDatabaseSize = GetSmmChildSwCallbackDatabaseData(Data, mSmmChildSwCallbackDatabaseSize);
  if (SmmChildSwCallbackDatabaseSize != mSmmChildSwCallbackDatabaseSize) {
    return EFI_INVALID_PARAMETER;
  }
  DEBUG((EFI_D_VERBOSE, "SmmChildSwCallbackDatabaseSize - 0x%x\n", SmmChildSwCallbackDatabaseSize));
  Data = (UINT8 *)Data + mSmmChildSwCallbackDatabaseSize;

  SmmChildSxCallbackDatabaseSize = GetSmmChildSxCallbackDatabaseData(Data, mSmmChildSxCallbackDatabaseSize);
  if (SmmChildSxCallbackDatabaseSize != mSmmChildSxCallbackDatabaseSize) {
    return EFI_INVALID_PARAMETER;
  }
  DEBUG((EFI_D_VERBOSE, "SmmChildSxCallbackDatabaseSize - 0x%x\n", SmmChildSxCallbackDatabaseSize));
  Data = (UINT8 *)Data + mSmmChildSxCallbackDatabaseSize;

  SmmChildPeriodicTimerCallbackDatabaseSize = GetSmmChildPeriodicTimerCallbackDatabaseData(Data, mSmmChildPeriodicTimerCallbackDatabaseSize);
  if (SmmChildPeriodicTimerCallbackDatabaseSize != mSmmChildPeriodicTimerCallbackDatabaseSize) {
    return EFI_INVALID_PARAMETER;
  }
  DEBUG((EFI_D_VERBOSE, "SmmChildPeriodicTimerCallbackDatabaseSize - 0x%x\n", SmmChildPeriodicTimerCallbackDatabaseSize));
  Data = (UINT8 *)Data + mSmmChildPeriodicTimerCallbackDatabaseSize;

  SmmChildUsbCallbackDatabaseSize = GetSmmChildUsbCallbackDatabaseData(Data, mSmmChildUsbCallbackDatabaseSize);
  if (SmmChildUsbCallbackDatabaseSize != mSmmChildUsbCallbackDatabaseSize) {
    return EFI_INVALID_PARAMETER;
  }
  DEBUG((EFI_D_VERBOSE, "SmmChildUsbCallbackDatabaseSize - 0x%x\n", SmmChildUsbCallbackDatabaseSize));
  Data = (UINT8 *)Data + mSmmChildUsbCallbackDatabaseSize;

  SmmChildGpiCallbackDatabaseSize = GetSmmChildGpiCallbackDatabaseData(Data, mSmmChildGpiCallbackDatabaseSize);
  if (SmmChildGpiCallbackDatabaseSize != mSmmChildGpiCallbackDatabaseSize) {
    return EFI_INVALID_PARAMETER;
  }
  DEBUG((EFI_D_VERBOSE, "SmmChildGpiCallbackDatabaseSize - 0x%x\n", SmmChildGpiCallbackDatabaseSize));
  Data = (UINT8 *)Data + mSmmChildGpiCallbackDatabaseSize;

  SmmChildPowerButtonCallbackDatabaseSize = GetSmmChildPowerButtonCallbackDatabaseData(Data, mSmmChildPowerButtonCallbackDatabaseSize);
  if (SmmChildPowerButtonCallbackDatabaseSize != mSmmChildPowerButtonCallbackDatabaseSize) {
    return EFI_INVALID_PARAMETER;
  }
  DEBUG((EFI_D_VERBOSE, "SmmChildPowerButtonCallbackDatabaseSize - 0x%x\n", SmmChildPowerButtonCallbackDatabaseSize));
  Data = (UINT8 *)Data + mSmmChildPowerButtonCallbackDatabaseSize;

  SmmChildStandbyButtonCallbackDatabaseSize = GetSmmChildStandbyButtonCallbackDatabaseData(Data, mSmmChildStandbyButtonCallbackDatabaseSize);
  if (SmmChildStandbyButtonCallbackDatabaseSize != mSmmChildStandbyButtonCallbackDatabaseSize) {
    return EFI_INVALID_PARAMETER;
  }
  DEBUG((EFI_D_VERBOSE, "SmmChildStandbyButtonCallbackDatabaseSize - 0x%x\n", SmmChildStandbyButtonCallbackDatabaseSize));
  Data = (UINT8 *)Data + mSmmChildStandbyButtonCallbackDatabaseSize;

  SmmChildIoTrapCallbackDatabaseSize = GetSmmChildIoTrapCallbackDatabaseData(Data, mSmmChildIoTrapCallbackDatabaseSize);
  if (SmmChildIoTrapCallbackDatabaseSize != mSmmChildIoTrapCallbackDatabaseSize) {
    return EFI_INVALID_PARAMETER;
  }
  DEBUG((EFI_D_VERBOSE, "SmmChildIoTrapCallbackDatabaseSize - 0x%x\n", SmmChildIoTrapCallbackDatabaseSize));
  Data = (UINT8 *)Data + mSmmChildIoTrapCallbackDatabaseSize;

  return EFI_SUCCESS;
}
