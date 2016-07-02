/** @file

Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef  _SMM_CHILD_DUMP_H_
#define  _SMM_CHILD_DUMP_H_

#define UNKNOWN_NAME  L"???"

EFI_GUID *
ProtocolTypeToGuid(
  IN INT32         ProtocolType
  );

UINTN
SmmChildHandlerStructureSizeByGuid(
  IN EFI_GUID  *Guid
  );

CHAR8 *
SxTypeToName(
  IN EFI_SLEEP_TYPE Type
  );

CHAR8 *
SxPhaseToName(
  IN EFI_SLEEP_PHASE Phase
  );

CHAR8 *
UsbTypeToName(
  IN EFI_USB_SMI_TYPE Type
  );

CHAR8 *
StandbyButtonPhaseToName(
  IN EFI_STANDBY_BUTTON_PHASE Phase
  );

CHAR8 *
PowerButtonPhaseToName(
  IN EFI_POWER_BUTTON_PHASE Phase
  );

CHAR8 *
IoTrapTypeToName(
  IN EFI_SMM_IO_TRAP_DISPATCH_TYPE Type
  );

#endif
