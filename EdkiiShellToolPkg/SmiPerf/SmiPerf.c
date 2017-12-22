/** @file

Copyright (c) 2017, Intel Corporation. All rights reserved.<BR>
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
#include <Library/DxeServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Guid/Performance.h>

#define COUNT 1000

EFI_STATUS
EFIAPI
InitializeSmiPerf (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  UINT64                    StartTsc;
  UINT64                    EndTsc;
  UINTN                     Index;
  PERFORMANCE_PROPERTY      *PerformanceProperty;
  EFI_STATUS                Status;
  EFI_TPL                   OldTpl;

  OldTpl = gBS->RaiseTPL (TPL_HIGH_LEVEL);
  StartTsc = AsmReadTsc ();
  for (Index = 0; Index < COUNT; Index++) {
    IoWrite8 (0xB2, 0xFF);
  }
  EndTsc = AsmReadTsc ();
  gBS->RestoreTPL (OldTpl);
  
  Status = EfiGetSystemConfigurationTable (&gPerformanceProtocolGuid, &PerformanceProperty);
  if (EFI_ERROR (Status)) {
    Print (L"PERFORMANCE_PROPERTY not found!\n");
    return EFI_NOT_FOUND;
  } else {
    Print (L"PERFORMANCE_PROPERTY\n");
    Print (L"  Revision        - 0x%x\n", PerformanceProperty->Revision);
    Print (L"  TimerStartValue - %ld\n", PerformanceProperty->TimerStartValue);
    Print (L"  TimerEndValue   - %ld\n", PerformanceProperty->TimerEndValue);
    Print (L"  Frequency       - %ld\n", PerformanceProperty->Frequency);
  }

  Print (L"%d SMI - %ld tick\n", COUNT, EndTsc - StartTsc);
  //
  // 1 SMI = (EndTsc - StartTsc)/COUNT * 1000 * 1000 / Frequency uS
  //       = 341115 * 1000 * 1000 / 1616844000
  //
  Print (L"SMI - %ld us\n", DivU64x64Remainder (MultU64x64 (DivU64x32(EndTsc - StartTsc, COUNT), 1000 * 1000), PerformanceProperty->Frequency, NULL));

  return EFI_SUCCESS;
}
