/** @file

Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Base.h>
#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/PrintLib.h>
#include <Library/DumpAcpiTableFuncLib.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/DevicePath.h>
#include <Protocol/SimpleFileSystem.h>

#include "DumpACPI.h"

VOID
EFIAPI
DumpAcpiPSDT (
  VOID  *Table
  )
{
  EFI_ACPI_DESCRIPTION_HEADER                            *Psdt;
  EFI_LOADED_IMAGE_PROTOCOL             *Image;
  EFI_DEVICE_PATH_PROTOCOL              *DevicePath;
  EFI_HANDLE                            DeviceHandle;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL       *SimpleFileSystem;
  EFI_FILE                              *Root;
  EFI_FILE                              *FileHandle;
  CHAR16                                *PsdtFileName = L"Psdt_%d.aml";
  static UINTN                          FileIndex = 0;
  CHAR16                                FileName[MAX_FILE_NAME_LEN];
  UINTN                                 BufferSize;
  VOID                                  *Buffer;
  EFI_STATUS                            Status;
  
  Psdt = Table;
  if (Psdt == NULL) {
    return;
  }
  
  //
  // Dump Psdt table
  //
  Print (         
    L"*****************************************************************************\n"
    L"*         Persistent System Description Table                               *\n"
    L"*****************************************************************************\n"
    );

  if (GetAcpiDumpPropertyDumpData()) {
    DumpAcpiHex (Psdt->Length, Psdt);
  }

  if (!GetAcpiDumpPropertyDumpVerb()) {
    goto Done;
  }

  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"PSDT address ............................................. 0x%016lx\n" :
    L"PSDT address ............................................. 0x%08x\n",
    Psdt
    );
  
  DumpAcpiTableHeader (Psdt);
 
Done:
  Print (       
    L"*****************************************************************************\n\n"
    );
 
  if (!mIsDumpData) {
    return;
  }
 
  FileIndex++;

  FileName[0] = 0;
  UnicodeSPrint (FileName, sizeof(FileName), PsdtFileName, FileIndex);
    
  //
  // Dump PSDT table to Psdt.aml, use windows AcpiDump.exe to read the information.
  // This can be update by porting AcpiDump to Shell.
  //
  Status = gBS->HandleProtocol (
                 mImageHandle,
                 &gEfiLoadedImageProtocolGuid,
                 (VOID **)&Image
                 );
  if (EFI_ERROR(Status)) {
    Print (L"Error: HandleProtocol LoadedImage ! - %r\n", Status);
    FreePool (FileName);
    return;
  }
  Status = gBS->HandleProtocol (
                 Image->DeviceHandle,
                 &gEfiDevicePathProtocolGuid,
                 (VOID **)&DevicePath
                 );
  if (EFI_ERROR(Status)) {
    Print (L"Error: HandleProtocol DevicePath ! - %r\n", Status);
    FreePool (FileName);
    return;
  }
  Status = gBS->LocateDevicePath ( 
                 &gEfiSimpleFileSystemProtocolGuid,
                 &DevicePath,
                 &DeviceHandle
                 );
  if (EFI_ERROR (Status)) {
    Print (L"Error: LocateDevicePath SimpleFileSystem ! - %r\n", Status);
    FreePool (FileName);
    return;
  }
  
  Status = gBS->HandleProtocol (
                 DeviceHandle, 
                 &gEfiSimpleFileSystemProtocolGuid,
                 (VOID*)&SimpleFileSystem
                 );
  if (EFI_ERROR (Status)) {
    Print (L"Error: HandleProtocol SimpleFileSystem ! - %r\n", Status);
    FreePool (FileName);
    return;
  }
  Status = SimpleFileSystem->OpenVolume (
                               SimpleFileSystem,
                               &Root
                               );
  if (EFI_ERROR (Status)) {
    Print (L"Error: SimpleFileSystem->OpenVolume() ! - %r\n", Status);
    FreePool (FileName);
    return;
  }
  Status = Root->Open (
                   Root,
                   &FileHandle,
                   FileName,
                   EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE,
                   0
                   );
  if (EFI_ERROR (Status)) {
    Print (L"Error: Root->Open() ! - %r\n", Status);
    FreePool (FileName);
    return;
  }
  
  Buffer = Psdt;
  BufferSize = Psdt->Length;
  Status = FileHandle->Write (FileHandle, &BufferSize, Buffer);
  if (EFI_ERROR (Status)) {
    Print (L"Error: FileHandle->Write() ! - %r\n", Status);
    FileHandle->Close (FileHandle);
    Root->Close (Root);
    FreePool (FileName);
    return;
  }

  FileHandle->Close (FileHandle);
  Root->Close (Root);
  //
  // Dump PSDT to Psdt.aml finished.
  //

  return;
}
