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
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>

UINTN  Argc;
CHAR16 **Argv;

EFI_GUID mZeroGuid;

EFI_STATUS
GetArg (
  VOID
  );

EFI_STATUS
ReadFileToBuffer (
  IN  CHAR16                               *FileName,
  OUT UINTN                                *BufferSize,
  OUT VOID                                 **Buffer
  );

/**
  Print APP usage.
**/
VOID
PrintUsage (
  VOID
  )
{
  Print(L"DbxEnroll:  A tool to enroll the revocation list from https://uefi.org/revocationlistfile\n");
  Print(L"  DbxEnroll -f <revocation file>\n");
}

EFI_STATUS
EFIAPI
InitializeDbxEnroll (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS  Status;
  UINTN       BufferSize;
  VOID        *Buffer;

  GetArg ();

  if ((Argc == 3) && (StrCmp (Argv[1], L"-f") == 0)) {
    Status = ReadFileToBuffer(Argv[2], &BufferSize, &Buffer);
    if (EFI_ERROR(Status)) {
      Print (L"ReadFile error - %s\n", Argv[2]);
      return Status;
    }
    Status = gRT->SetVariable (
                    L"dbx",
                    &gEfiImageSecurityDatabaseGuid,
                    EFI_VARIABLE_NON_VOLATILE |
                      EFI_VARIABLE_BOOTSERVICE_ACCESS |
                      EFI_VARIABLE_RUNTIME_ACCESS |
                      EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS |
                      EFI_VARIABLE_APPEND_WRITE,
                    BufferSize,
                    Buffer
                    );
    Print (L"SetVariable - %r\n", Status);
    return Status;
  } else {
    PrintUsage ();
  }

  return EFI_SUCCESS;
}
