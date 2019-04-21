/** @file

Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
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
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Protocol/ShellParameters.h>
#include <Protocol/LoadedImage.h>
#include <Register/Cpuid.h>
#include <Register/Msr.h>
#include <Register/Microcode.h>

UINTN  Argc;
CHAR16 **Argv;

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

EFI_FILE_HANDLE
LibOpenRoot (
  IN EFI_HANDLE                   DeviceHandle
  )
{
  EFI_STATUS                      Status;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *Volume;
  EFI_FILE_HANDLE                 File;

  File = NULL;

  //
  // Handle the file system interface to the device
  //
  Status = gBS->HandleProtocol (
                DeviceHandle,
                &gEfiSimpleFileSystemProtocolGuid,
                (VOID *) &Volume
                );

  //
  // Open the root directory of the volume
  //
  if (!EFI_ERROR (Status)) {
    Status = Volume->OpenVolume (
                      Volume,
                      &File
                      );
  }
  //
  // Done
  //
  return EFI_ERROR (Status) ? NULL : File;
}

EFI_STATUS
GetImageFromFile (
  IN    CHAR16     *FileName,
  OUT   UINT8      **ImageBuffer,
  OUT   UINTN      *FileSize
  )
{
  EFI_HANDLE            DeviceHandle;
  EFI_FILE_HANDLE       Handle;
  UINT8                 *Buffer;
  UINTN                 ImageSize;
  EFI_STATUS            Status;
  EFI_FILE_INFO         *Info;
  UINTN                 BufferSize;
  EFI_FILE_HANDLE       RootHandle;
  EFI_LOADED_IMAGE_PROTOCOL *LoadedImage;

  Status = gBS->HandleProtocol (
                  gImageHandle,
                  &gEfiLoadedImageProtocolGuid,
                  (VOID**)&LoadedImage
                  );
  if (EFI_ERROR(Status)) {
    return Status;
  }
  DeviceHandle = LoadedImage->DeviceHandle;
  
  RootHandle = LibOpenRoot (DeviceHandle);
  if (RootHandle == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = RootHandle->Open (RootHandle, &Handle, FileName, EFI_FILE_MODE_READ, 0);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  RootHandle->Close (RootHandle);

  BufferSize = SIZE_OF_EFI_FILE_INFO + 1024;
  Status = gBS->AllocatePool (EfiBootServicesData,BufferSize, (VOID **)&Info);
  ASSERT_EFI_ERROR (Status);
  Status = Handle->GetInfo (
                     Handle, 
                     &gEfiFileInfoGuid, 
                     &BufferSize, 
                     Info
                     );
  if (EFI_ERROR(Status)) {
    return Status;
  }

  *FileSize = (UINTN)Info->FileSize;
  ImageSize = *FileSize;
  Status = gBS->AllocatePool (EfiBootServicesData,ImageSize, (VOID **)&Buffer);
  ASSERT_EFI_ERROR (Status);
  Status = Handle->Read (
                     Handle,
                     &ImageSize,
                     Buffer
                     );
  if (EFI_ERROR(Status)) {
    return Status;
  }

  Handle->Close (Handle);
  *ImageBuffer = Buffer;

  return EFI_SUCCESS;
}

/**

  This function dump raw data.

  @param  Data  raw data
  @param  Size  raw data size

**/
VOID
InternalDumpData (
  IN UINT8  *Data,
  IN UINTN  Size
  )
{
  UINTN  Index;
  for (Index = 0; Index < Size; Index++) {
    Print (L"%02x", (UINTN)Data[Index]);
  }
}

/**

  This function dump raw data with colume format.

  @param  Data  raw data
  @param  Size  raw data size

**/
VOID
InternalDumpHex (
  IN UINT8  *Data,
  IN UINTN  Size
  )
{
  UINTN   Index;
  UINTN   Count;
  UINTN   Left;

#define COLUME_SIZE  (16 * 2)

  Count = Size / COLUME_SIZE;
  Left  = Size % COLUME_SIZE;
  for (Index = 0; Index < Count; Index++) {
    Print (L"%04x: ", Index * COLUME_SIZE);
    InternalDumpData (Data + Index * COLUME_SIZE, COLUME_SIZE);
    Print (L"\n");
  }

  if (Left != 0) {
    Print (L"%04x: ", Index * COLUME_SIZE);
    InternalDumpData (Data + Index * COLUME_SIZE, Left);
    Print (L"\n");
  }
}

VOID
DumpCpuid (
  VOID
  )
{
  UINT32  Eax;

  AsmCpuid (CPUID_VERSION_INFO, &Eax, NULL, NULL, NULL);
  Print (L"CPUID[1].EAX - 0x%08x\n", Eax);
}

VOID
DumpDebugInterface (
  VOID
  )
{
  MSR_IA32_DEBUG_INTERFACE_REGISTER  Msr;
  CPUID_VERSION_INFO_ECX             Ecx;

  AsmCpuid (CPUID_VERSION_INFO, NULL, NULL, &Ecx.Uint32, NULL);

  if (Ecx.Bits.SDBG == 0) {
    Print (L"MSR_IA32_DEBUG_INTERFACE_REGISTER - unsupported\n");
    return ;
  }

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_DEBUG_INTERFACE);
  Print (L"MSR_IA32_DEBUG_INTERFACE_REGISTER - 0x%016lx\n", Msr.Uint64);
  Print (L"  Enable        - 0x%x\n", Msr.Bits.Enable);
  Print (L"  Lock          - 0x%x\n", Msr.Bits.Lock);
  Print (L"  DebugOccurred - 0x%x\n", Msr.Bits.DebugOccurred);
}

VOID
DumpPlatformId (
  VOID
  )
{
  MSR_IA32_PLATFORM_ID_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_PLATFORM_ID);
  Print (L"MSR_IA32_PLATFORM_ID - 0x%016lx\n", Msr.Uint64);
  Print (L"  PlatformId - 0x%x\n", Msr.Bits.PlatformId);
}

VOID
DumpMicrocodeVersion (
  VOID
  )
{
  MSR_IA32_BIOS_SIGN_ID_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_BIOS_SIGN_ID);
  Print (L"MSR_IA32_BIOS_SIGN_ID - 0x%016lx\n", Msr.Uint64);
  Print (L"  MicrocodeUpdateSignature - 0x%08x\n", Msr.Bits.MicrocodeUpdateSignature);
}

VOID
TriggerMicrocodeUpdate (
  IN VOID *Buffer
  )
{
  AsmWriteMsr64 (MSR_IA32_BIOS_UPDT_TRIG, (UINTN)Buffer + sizeof(CPU_MICROCODE_HEADER));
}

VOID
PrintHelp (
  VOID
  )
{
  Print (L"PatchMicrocode -D\n");
  Print (L"  Dump System Information\n");
  Print (L"PatchMicrocode -D <MCU>\n");
  Print (L"  Dump MCU file Information\n");
  Print (L"PatchMicrocode -P <MCU>\n");
  Print (L"  Patch MCU file\n");
}

EFI_STATUS
EFIAPI
PatchMicrocodeEntrypoint (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS  Status;
  UINT8       *ImageBuffer;
  UINTN       FileSize;

  Status = GetArg();
  if (EFI_ERROR(Status)) {
    Print(L"Not UEFI shell...\n");
    return Status;
  }
  
  if (Argc < 2 || Argc > 3) {
    return EFI_INVALID_PARAMETER;
  }

  if (Argc == 2) {
    if (StrCmp (Argv[1], L"-D") == 0) {
      DumpCpuid ();
      DumpDebugInterface ();
      DumpPlatformId ();
      DumpMicrocodeVersion ();
      return EFI_SUCCESS;
    }
  }

  if (Argc == 3) {
    if (StrCmp (Argv[1], L"-P") == 0) {
      GetImageFromFile (Argv[2], &ImageBuffer, &FileSize);
      InternalDumpHex (ImageBuffer, 0x100);
      TriggerMicrocodeUpdate (ImageBuffer);
      return EFI_SUCCESS;
    }
  }

  PrintHelp ();
  return EFI_SUCCESS;
}
