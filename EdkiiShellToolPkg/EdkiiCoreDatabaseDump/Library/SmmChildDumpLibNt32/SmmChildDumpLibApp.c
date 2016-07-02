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

VOID
PrintChildHandlerContext(
  IN EFI_GUID  *Guid,
  IN VOID      *Context
  )
{
  EFI_SMM_SW_REGISTER_CONTEXT             *Sw;
  EFI_SMM_SX_REGISTER_CONTEXT             *Sx;
  EFI_SMM_PERIODIC_TIMER_REGISTER_CONTEXT *PeriodicTimer;
  SMM_CHILD_USB_REGISTER_CONTEXT          *Usb;
  EFI_SMM_GPI_REGISTER_CONTEXT            *Gpi;
  EFI_SMM_POWER_BUTTON_REGISTER_CONTEXT   *PowerButton;
  EFI_SMM_STANDBY_BUTTON_REGISTER_CONTEXT *StandbyButton;
  EFI_SMM_IO_TRAP_REGISTER_CONTEXT        *IoTrap;

  if (CompareGuid(Guid, &gEfiSmmSwDispatch2ProtocolGuid)) {
    Sw = Context;
    Print(L" SwSmi - 0x%x", Sw->SwSmiInputValue);
  } else if (CompareGuid(Guid, &gEfiSmmSxDispatch2ProtocolGuid)) {
    Sx = Context;
    Print(L" Type - 0x%a, Phase - 0x%a", SxTypeToName(Sx->Type), SxPhaseToName(Sx->Phase));
  } else if (CompareGuid(Guid, &gEfiSmmPeriodicTimerDispatch2ProtocolGuid)) {
    PeriodicTimer = Context;
    Print(L" Period - %ld, SmiTickInterval - %ld", PeriodicTimer->Period, PeriodicTimer->SmiTickInterval);
  } else if (CompareGuid(Guid, &gEfiSmmUsbDispatch2ProtocolGuid)) {
    CHAR16  *PathStr;
    Usb = Context;
    PathStr = L"Unknown";
    if (IsDevicePathValid((EFI_DEVICE_PATH_PROTOCOL *)Usb->Device, sizeof(Usb->Device))) {
      PathStr = ConvertDevicePathToText((EFI_DEVICE_PATH_PROTOCOL *)Usb->Device, TRUE, TRUE);
    }
    Print(L" Type - 0x%a, Device - %s", UsbTypeToName(Usb->Type), PathStr);
  } else if (CompareGuid(Guid, &gEfiSmmGpiDispatch2ProtocolGuid)) {
    Gpi = Context;
    Print(L" GpiNum - %ld", Gpi->GpiNum);
  } else if (CompareGuid(Guid, &gEfiSmmStandbyButtonDispatch2ProtocolGuid)) {
    StandbyButton = Context;
    Print(L" Phase - 0x%a", StandbyButtonPhaseToName(StandbyButton->Phase));
  } else if (CompareGuid(Guid, &gEfiSmmPowerButtonDispatch2ProtocolGuid)) {
    PowerButton = Context;
    Print(L" Phase - 0x%a", PowerButtonPhaseToName(PowerButton->Phase));
  } else if (CompareGuid(Guid, &gEfiSmmIoTrapDispatch2ProtocolGuid)) {
    IoTrap = Context;
    Print(L" Address - 0x%x, Length - 0x%x, Type - %a", IoTrap->Address, IoTrap->Length, IoTrapTypeToName(IoTrap->Type));
  }
}

VOID
EFIAPI
DumpSmmChildHandler(
  IN VOID  *Data,
  IN UINTN Size
  )
{
  SMM_CHILD_SMI_DATABASE_STRUCTURE         *SmiDatabase;
  SMM_CHILD_SMI_HANDLER_STRUCTURE_HEADER   *SmiStruct;
  UINTN                                    Index;
  UINTN                                    StructSize;

  Print(L"# 3. Hardware SMI Handler #\n");
  SmiDatabase = (VOID *)Data;
  while ((UINTN)SmiDatabase < (UINTN)Data + Size) {
    if (SmiDatabase->Header.Signature == SMM_CHILD_SMI_DATABASE_SIGNATURE) {
      SmiStruct = (VOID *)(SmiDatabase + 1);
      StructSize = SmmChildHandlerStructureSizeByGuid(&SmiDatabase->Guid);
      if (StructSize == 0) {
        SmiDatabase = (VOID *)((UINTN)SmiDatabase + SmiDatabase->Header.Length);
        continue;
      }
      Print(L"ChildHandle - %a\n", GuidToName(&SmiDatabase->Guid));
      for (Index = 0; Index < SmiDatabase->HandlerCount; Index++) {
        Print(L"  SmiHandle - 0x%x,", SmiStruct->SmiHandle);
        PrintChildHandlerContext(&SmiDatabase->Guid, SmiStruct + 1);
        Print(L"\n");
        Print(L"    Handler - 0x%x", SmiStruct->Handler);
        if (SmiStruct->Handler != 0) {
          CHAR16  *Name;
          Name = GetImageRefName((UINTN)SmiStruct->ImageRef);
          if (StrCmp(Name, UNKNOWN_NAME) != 0) {
            Print(L" (%s)", Name);
          }
        }
        Print(L"\n");
        SmiStruct = (VOID *)((UINTN)SmiStruct + StructSize);
      }
    }
    SmiDatabase = (VOID *)((UINTN)SmiDatabase + SmiDatabase->Header.Length);
  }

  return;
}