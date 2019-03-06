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
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Guid/GlobalVariable.h>
#include <Guid/Performance.h>

#define NON_EXIST_NAME  L"U.N.Owen"
#define MAX_BUFFER_SIZE 1024

#define COUNT 1000

EFI_STATUS
EFIAPI
GetVariablePerfEntry (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE*  SystemTable
  )
{
  UINT64                    StartTsc;
  UINT64                    EndTsc;
  UINTN                     Index;
  PERFORMANCE_PROPERTY      *PerformanceProperty;
  EFI_STATUS                Status;
  EFI_TPL                   OldTpl;
  UINT8                     DataBuffer[MAX_BUFFER_SIZE];
  UINTN                     BufferSize;

  OldTpl = gBS->RaiseTPL (TPL_HIGH_LEVEL);
  StartTsc = AsmReadTsc ();

  BufferSize = sizeof (DataBuffer);
  BufferSize = 0;

  for (Index = 0; Index < COUNT; Index++) {
    Status = gRT->GetVariable (
                    NON_EXIST_NAME,
                    &gEfiGlobalVariableGuid,
                    NULL,
                    &BufferSize,
                    DataBuffer
                    );
  }
  EndTsc = AsmReadTsc ();
  gBS->RestoreTPL (OldTpl);
  
  Status = EfiGetSystemConfigurationTable (&gPerformanceProtocolGuid, (VOID **)&PerformanceProperty);
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

  Print (L"%d GetVariable - %ld tick\n", COUNT, EndTsc - StartTsc);
  Print (L"GetVariable - %ld us\n", DivU64x64Remainder (MultU64x64 (DivU64x32(EndTsc - StartTsc, COUNT), 1000 * 1000), PerformanceProperty->Frequency, NULL));

  return EFI_SUCCESS;
}