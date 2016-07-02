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

LIST_ENTRY      *mSmmChildCallbackDatabase;

#define SMM_PROTOCOL_TYPE_TO_GUID {  \
          &gEfiSmmSwDispatch2ProtocolGuid,             \
		  }

EFI_GUID  *mProtocolTypeToGuid[] = SMM_PROTOCOL_TYPE_TO_GUID;

EFI_GUID *
ProtocolTypeToGuid(
  IN INT32        ProtocolType
  )
{
  if (ProtocolType < sizeof(mProtocolTypeToGuid) / sizeof(mProtocolTypeToGuid[0])) {
    return mProtocolTypeToGuid[ProtocolType];
  } else {
    return NULL;
  }
}

CHAR8 mContextNameStr[16];

CHAR8 *mSxTypeName[] = { "S0", "S1", "S2", "S3", "S4", "S5" };
CHAR8 *
SxTypeToName(
  IN EFI_SLEEP_TYPE Type
  )
{
  if (Type < sizeof(mSxTypeName) / sizeof(mSxTypeName[0])) {
    return mSxTypeName[Type];
  } else {
    AsciiSPrint(mContextNameStr, sizeof(mContextNameStr), "0x%x", Type);
    return mContextNameStr;
  }
}

CHAR8 *mSxPhaseName[] = { "Entry", "Exit" };
CHAR8 *
SxPhaseToName(
  IN EFI_SLEEP_PHASE Phase
  )
{
  if (Phase < sizeof(mSxPhaseName) / sizeof(mSxPhaseName[0])) {
    return mSxPhaseName[Phase];
  } else {
    AsciiSPrint(mContextNameStr, sizeof(mContextNameStr), "0x%x", Phase);
    return mContextNameStr;
  }
}

CHAR8 *mUsbTypeName[] = { "UsbLegacy", "UsbWake" };
CHAR8 *
UsbTypeToName(
  IN EFI_USB_SMI_TYPE Type
  )
{
  if (Type < sizeof(mUsbTypeName) / sizeof(mUsbTypeName[0])) {
    return mUsbTypeName[Type];
  } else {
    AsciiSPrint(mContextNameStr, sizeof(mContextNameStr), "0x%x", Type);
    return mContextNameStr;
  }
}

CHAR8 *mStandbyButtonPhaseName[] = { "Entry", "Exit" };
CHAR8 *
StandbyButtonPhaseToName(
  IN EFI_STANDBY_BUTTON_PHASE Phase
  )
{
  if (Phase < sizeof(mStandbyButtonPhaseName) / sizeof(mStandbyButtonPhaseName[0])) {
    return mStandbyButtonPhaseName[Phase];
  } else {
    AsciiSPrint(mContextNameStr, sizeof(mContextNameStr), "0x%x", Phase);
    return mContextNameStr;
  }
}

CHAR8 *mPowerButtonPhaseName[] = { "Entry", "Exit" };
CHAR8 *
PowerButtonPhaseToName(
  IN EFI_POWER_BUTTON_PHASE Phase
  )
{
  if (Phase < sizeof(mPowerButtonPhaseName) / sizeof(mPowerButtonPhaseName[0])) {
    return mPowerButtonPhaseName[Phase];
  } else {
    AsciiSPrint(mContextNameStr, sizeof(mContextNameStr), "0x%x", Phase);
    return mContextNameStr;
  }
}

CHAR8 *mIoTrapTypeName[] = { "WriteTrap", "ReadTrap", "ReadWriteTrap" };
CHAR8 *
IoTrapTypeToName(
  IN EFI_SMM_IO_TRAP_DISPATCH_TYPE Type
  )
{
  if (Type < sizeof(mIoTrapTypeName) / sizeof(mIoTrapTypeName[0])) {
    return mIoTrapTypeName[Type];
  } else {
    AsciiSPrint(mContextNameStr, sizeof(mContextNameStr), "0x%x", Type);
    return mContextNameStr;
  }
}

VOID
DumpChildHandlerContext(
  IN EFI_GUID  *Guid,
  IN VOID      *Context
  )
{
  EFI_SMM_SW_REGISTER_CONTEXT             *Sw;
  EFI_SMM_SX_REGISTER_CONTEXT             *Sx;
  EFI_SMM_PERIODIC_TIMER_REGISTER_CONTEXT *PeriodicTimer;
  EFI_SMM_USB_REGISTER_CONTEXT            *Usb;
  EFI_SMM_GPI_REGISTER_CONTEXT            *Gpi;
  EFI_SMM_POWER_BUTTON_REGISTER_CONTEXT   *PowerButton;
  EFI_SMM_STANDBY_BUTTON_REGISTER_CONTEXT *StandbyButton;
  EFI_SMM_IO_TRAP_REGISTER_CONTEXT        *IoTrap;

  if (CompareGuid(Guid, &gEfiSmmSwDispatch2ProtocolGuid)) {
    Sw = Context;
    DEBUG((EFI_D_INFO, " SwSmi - 0x%x,", Sw->SwSmiInputValue));
  } else if (CompareGuid(Guid, &gEfiSmmSxDispatch2ProtocolGuid)) {
    Sx = Context;
    DEBUG((EFI_D_INFO, " Type - 0x%a, Phase - 0x%a,", SxTypeToName(Sx->Type), SxPhaseToName(Sx->Phase)));
  } else if (CompareGuid(Guid, &gEfiSmmPeriodicTimerDispatch2ProtocolGuid)) {
    PeriodicTimer = Context;
    DEBUG((EFI_D_INFO, " Period - %ld, SmiTickInterval - %ld,", PeriodicTimer->Period, PeriodicTimer->SmiTickInterval));
  } else if (CompareGuid(Guid, &gEfiSmmUsbDispatch2ProtocolGuid)) {
    CHAR16  *PathStr;
    Usb = Context;
    PathStr = ConvertDevicePathToText(Usb->Device, TRUE, TRUE);
    DEBUG((EFI_D_INFO, " Type - 0x%a, Device - %s,", UsbTypeToName(Usb->Type), PathStr));
  } else if (CompareGuid(Guid, &gEfiSmmGpiDispatch2ProtocolGuid)) {
    Gpi = Context;
    DEBUG((EFI_D_INFO, " GpiNum - %ld,", Gpi->GpiNum));
  } else if (CompareGuid(Guid, &gEfiSmmStandbyButtonDispatch2ProtocolGuid)) {
    StandbyButton = Context;
    DEBUG((EFI_D_INFO, " Phase - 0x%a,", StandbyButtonPhaseToName(StandbyButton->Phase)));
  } else if (CompareGuid(Guid, &gEfiSmmPowerButtonDispatch2ProtocolGuid)) {
    PowerButton = Context;
    DEBUG((EFI_D_INFO, " Phase - 0x%a,", PowerButtonPhaseToName(PowerButton->Phase)));
  } else if (CompareGuid(Guid, &gEfiSmmIoTrapDispatch2ProtocolGuid)) {
    IoTrap = Context;
    DEBUG((EFI_D_INFO, " Address - 0x%x, Length - 0x%x, Type - %a,", IoTrap->Address, IoTrap->Length, IoTrapTypeToName(IoTrap->Type)));
  }
}

VOID
DumpSmmChildCallbackDatabase(
  VOID
  )
{
  LIST_ENTRY      *ListEntry;
  DATABASE_RECORD *Record;
  EFI_GUID        *Guid;

  ListEntry = mSmmChildCallbackDatabase;
  for (ListEntry = ListEntry->ForwardLink;
       ListEntry != mSmmChildCallbackDatabase;
       ListEntry = ListEntry->ForwardLink) {
    Record = CR(ListEntry, DATABASE_RECORD, Link, DATABASE_RECORD_SIGNATURE);
    DEBUG((EFI_D_INFO, "ChildHandle - 0x%x\n", &Record->Link));
    Guid = ProtocolTypeToGuid(0);
    if (Guid != NULL) {
      DEBUG((EFI_D_INFO, "  Guid - %g,", Guid));
      DumpChildHandlerContext(Guid, &Record->RegisterContext);
      DEBUG((EFI_D_INFO, " Callback - 0x%x", Record->DispatchFunction));
      if (Record->DispatchFunction != NULL) {
        CHAR16  *Name;
        Name = AddressToImageName((UINTN)Record->DispatchFunction);
        if (StrCmp(Name, UNKNOWN_NAME) != 0) {
          DEBUG((EFI_D_INFO, " (%s)", Name));
        }
      }
    }
    DEBUG((EFI_D_INFO, "\n"));
  }

  return;
}

VOID
GetSmmChildCallbackDatabase(
  IN EFI_HANDLE    DispatchHandle
  )
{
  LIST_ENTRY      *ListEntry;
  DATABASE_RECORD *Record;

  ListEntry = (LIST_ENTRY *)DispatchHandle;
  for (ListEntry = ListEntry->ForwardLink;
       ListEntry != (LIST_ENTRY *)DispatchHandle;
       ListEntry = ListEntry->ForwardLink) {
    Record = BASE_CR(ListEntry, DATABASE_RECORD, Link);
    if (Record->Signature != DATABASE_RECORD_SIGNATURE) {
      // BUGBUG: NT32 will load image to BS memory, but execute code in DLL.
      // Do not use mSmmChildLoadedImage->ImageBase/ImageSize
      mSmmChildCallbackDatabase = ListEntry;
      break;
    }
  }

  return;
}

EFI_STATUS
EFIAPI
DummyChildHandler(
  IN EFI_HANDLE  DispatchHandle,
  IN CONST VOID  *Context         OPTIONAL,
  IN OUT VOID    *CommBuffer      OPTIONAL,
  IN OUT UINTN   *CommBufferSize  OPTIONAL
  )
{
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SmmChildDumpInit(
  VOID
  )
{
  EFI_STATUS                                Status;
  EFI_HANDLE                                SwHandle;
  EFI_SMM_SW_DISPATCH2_PROTOCOL             *SwDispatch;
  EFI_SMM_SW_REGISTER_CONTEXT               SwContext;

  DEBUG((EFI_D_INFO, "in SmmChildDumpInit\n"));

  Status = gSmst->SmmLocateProtocol(&gEfiSmmSwDispatch2ProtocolGuid, NULL, (VOID**)&SwDispatch);
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_INFO, "Locate gEfiSmmSwDispatch2ProtocolGuid - %r\n", Status));
    goto Done;
  }

  SwContext.SwSmiInputValue = (UINTN)-1;
  Status = SwDispatch->Register(
                         SwDispatch,
                         DummyChildHandler,
                         &SwContext,
                         &SwHandle
                         );
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_INFO, "Register gEfiSmmSwDispatch2ProtocolGuid - %r\n", Status));
    goto Done;
  }

  GetSmmChildCallbackDatabase(SwHandle);
  Status = SwDispatch->UnRegister(
                         SwDispatch,
                         SwHandle
                         );
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_INFO, "UnRegister gEfiSmmSwDispatch2ProtocolGuid - %r\n", Status));
    goto Done;
  }
  if (mSmmChildCallbackDatabase == NULL) {
    DEBUG((EFI_D_INFO, "SmmChild CallbackDatabase is not found!\n"));
    goto Done;
  }
  DEBUG((EFI_D_INFO, "SmmChild CallbackDatabase is found!\n"));

Done:
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SmmEndOfDxeInSmmChildDump(
  IN CONST EFI_GUID  *Protocol,
  IN VOID            *Interface,
  IN EFI_HANDLE      Handle
  )
{
  SmmChildDumpInit();
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SmmChildInit(
  VOID
  )
{
  EFI_STATUS  Status;
  VOID        *Registration;

  Status = gSmst->SmmRegisterProtocolNotify (
                    &gEfiSmmEndOfDxeProtocolGuid,
                    SmmEndOfDxeInSmmChildDump,
                    &Registration
                    );
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SmmChildDump (
  VOID
  )
{
  DEBUG((EFI_D_INFO, "in SmmChildDump\n"));

  if (mSmmChildCallbackDatabase == NULL) {
    DEBUG((EFI_D_INFO, "SmmChild CallbackDatabase is not found!\n"));
    goto Done;
  }

  //
  // Dump child callback database
  //
  DEBUG((EFI_D_INFO, "# 3. Hardware SMI Handler #\n"));
  DumpSmmChildCallbackDatabase();
  DEBUG((EFI_D_INFO, "\n"));

  // BUGBUG: Filter myself

Done:
  return EFI_SUCCESS;
}
