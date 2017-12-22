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
#include <Protocol/ShellParameters.h>

UINTN  Argc;
CHAR16 **Argv;

/**

  This function parse application ARG.

  @return Status
**/
EFI_STATUS
GetArg (
  VOID
  )
{
  EFI_STATUS                    Status;
  EFI_SHELL_PARAMETERS_PROTOCOL *ShellParameters;

  Status = gBS->HandleProtocol (
                  gImageHandle,
                  &gEfiShellParametersProtocolGuid,
                  (VOID**)&ShellParameters
                  );
  if (EFI_ERROR(Status)) {
    return Status;
  }

  Argc = ShellParameters->Argc;
  Argv = ShellParameters->Argv;
  return EFI_SUCCESS;
}

VOID
PrintHelp (
  VOID
  )
{
  Print (L"GcdSet CAP|ATT SET|ADD|DEL <BaseAddress> <Length> <Attributes>\n");
}

EFI_STATUS
EFIAPI
InitializeGcd (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:
  Test GCD Services

Arguments:
  ImageHandle     The image handle. 
  SystemTable     The system table.

Returns:
  EFI_SUCCESS             - Command completed successfully
  EFI_INVALID_PARAMETER   - Command usage error

--*/
{
  UINT64                           BaseAddress;
  UINT64                           Length;
  UINT64                           Attributes;
  EFI_GCD_MEMORY_SPACE_DESCRIPTOR  Descriptor;
  EFI_STATUS                       Status;

  GetArg ();

  if (Argc != 6) {
    PrintHelp ();
    return EFI_SUCCESS;
  }

  BaseAddress = StrHexToUint64(Argv[3]);
  Print (L"BaseAddress - 0x%lx\n", BaseAddress);
  Length = StrHexToUint64(Argv[4]);
  Print (L"Length - 0x%lx\n", Length);

  Status = gDS->GetMemorySpaceDescriptor (BaseAddress, &Descriptor);
  if (EFI_ERROR(Status)) {
    Print (L"GetMemorySpaceDescriptor - %r\n", Status);
    return EFI_SUCCESS;
  }

  Attributes = StrHexToUint64(Argv[5]);
  Print (L"Attributes - 0x%lx\n", Attributes);
  if (StrCmp (Argv[1], L"CAP") == 0) {
    if (StrCmp (Argv[2], L"SET") == 0) {
    } else if (StrCmp (Argv[2], L"ADD") == 0) {
      Attributes = Attributes | Descriptor.Capabilities;
    } else if (StrCmp (Argv[2], L"DEL") == 0) {
      Attributes = (~Attributes) & Descriptor.Capabilities;
    } else {
      Print (L"Unknown Sub-Operation - %s\n", Argv[2]);
      return EFI_SUCCESS;
    }
    Print (L"Final Attributes - 0x%lx\n", Attributes);
    Status = gDS->SetMemorySpaceCapabilities (
                    BaseAddress,
                    Length,
                    Attributes
                    );
    Print (L"SetMemorySpaceCapabilities - %r\n", Status);
  } else if (StrCmp (Argv[1], L"ATT") == 0) {
    if (StrCmp (Argv[2], L"SET") == 0) {
    } else if (StrCmp (Argv[2], L"ADD") == 0) {
      Attributes = Attributes | Descriptor.Attributes;
    } else if (StrCmp (Argv[2], L"DEL") == 0) {
      Attributes = (~Attributes) & Descriptor.Attributes;
    } else {
      Print (L"Unknown Sub-Operation - %s\n", Argv[2]);
      return EFI_SUCCESS;
    }
    Print (L"Final Attributes - 0x%lx\n", Attributes);
    Status = gDS->SetMemorySpaceAttributes (
                    BaseAddress,
                    Length,
                    Attributes
                    );
    Print (L"SetMemorySpaceAttributes - %r\n", Status);
  } else {
    Print (L"Unknown Operation - %s\n", Argv[1]);
  }
  return EFI_SUCCESS;
}
