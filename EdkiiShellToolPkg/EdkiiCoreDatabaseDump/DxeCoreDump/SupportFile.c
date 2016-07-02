/** @file
  Support file for file handling.

  Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials                          
  are licensed and made available under the terms and conditions of the BSD License         
  which accompanies this distribution.  The full text of the license may be found at        
  http://opensource.org/licenses/bsd-license.php                                            

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

**/

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>

#include <Protocol/LoadedImage.h>
#include <Protocol/SimpleFileSystem.h>
#include <Guid/FileInfo.h>

#include "DxeCoreDump.h"

/**
  Return File System Volume containing this shell application.

  @return File System Volume containing this shell application.
**/
EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *
GetMyVol (
  VOID
  )
{
  EFI_STATUS                        Status;
  EFI_LOADED_IMAGE_PROTOCOL         *LoadedImage;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL   *Vol;

  Status = gBS->HandleProtocol (
                  gImageHandle,
                  &gEfiLoadedImageProtocolGuid,
                  (VOID **)&LoadedImage
                  );
  ASSERT_EFI_ERROR (Status);
  
  Status = gBS->HandleProtocol (
                  LoadedImage->DeviceHandle,
                  &gEfiSimpleFileSystemProtocolGuid,
                  (VOID **)&Vol
                  );
  if (!EFI_ERROR (Status)) {
    return Vol;
  }

  return NULL;  
}

/**
  Read a file from this volume.

  @param Vol             File System Volume
  @param FileName        The file to be read.
  @param BufferSize      The file buffer size
  @param Buffer          The file buffer

  @retval EFI_SUCCESS    Read file successfully
  @retval EFI_NOT_FOUND  File not found
**/
EFI_STATUS
ReadFileFromVol (
  IN  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL   *Vol,
  IN  CHAR16                            *FileName,
  OUT UINTN                             *BufferSize,
  OUT VOID                              **Buffer
  )
{
  EFI_STATUS                        Status;
  EFI_FILE_HANDLE                   RootDir;
  EFI_FILE_HANDLE                   Handle;
  UINTN                             FileInfoSize;
  EFI_FILE_INFO                     *FileInfo;
  UINTN                             TempBufferSize;
  VOID                              *TempBuffer;

  //
  // Open the root directory
  //
  Status = Vol->OpenVolume (Vol, &RootDir);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Open the file
  //
  Status = RootDir->Open (
                      RootDir,
                      &Handle,
                      FileName,
                      EFI_FILE_MODE_READ,
                      0
                      );
  if (EFI_ERROR (Status)) {
    RootDir->Close (RootDir);
    return Status;
  }

  RootDir->Close (RootDir);

  //
  // Get the file information
  //
  FileInfoSize = sizeof(EFI_FILE_INFO) + 1024;

  FileInfo = AllocateZeroPool (FileInfoSize);
  if (FileInfo == NULL) {
    Handle->Close (Handle);
    return Status;
  }

  Status = Handle->GetInfo (
                     Handle,
                     &gEfiFileInfoGuid,
                     &FileInfoSize,
                     FileInfo
                     );
  if (EFI_ERROR (Status)) {
    Handle->Close (Handle);
    gBS->FreePool (FileInfo);
    return Status;
  }

  //
  // Allocate buffer for the file data. The last CHAR16 is for L'\0'
  //
  TempBufferSize = (UINTN) FileInfo->FileSize + sizeof(CHAR16);
  TempBuffer = AllocateZeroPool (TempBufferSize);
  if (TempBuffer == NULL) {
    Handle->Close (Handle);
    gBS->FreePool (FileInfo);
    return Status;
  }

  gBS->FreePool (FileInfo);

  //
  // Read the file data to the buffer
  //
  Status = Handle->Read (
                     Handle,
                     &TempBufferSize,
                     TempBuffer
                     );
  if (EFI_ERROR (Status)) {
    Handle->Close (Handle);
    gBS->FreePool (TempBuffer);
    return Status;
  }

  Handle->Close (Handle);

  *BufferSize = TempBufferSize;
  *Buffer     = TempBuffer;
  return EFI_SUCCESS;
}

/**
  Read a file.
  If ScanFs is FLASE, it will use this Vol as default Fs.
  If ScanFs is TRUE, it will scan all FS and check the file.
    If there is only one file match the name, it will be read.
    If there is more than one file match the name, it will return Error.

  @param ThisVol         File System Volume
  @param FileName        The file to be read.
  @param BufferSize      The file buffer size
  @param Buffer          The file buffer
  @param ScanFs          Need Scan all FS

  @retval EFI_SUCCESS    Read file successfully
  @retval EFI_NOT_FOUND  File not found
  @retval EFI_NO_MAPPING There is duplicated files found
**/
EFI_STATUS
ReadFileToBufferEx (
  IN OUT EFI_SIMPLE_FILE_SYSTEM_PROTOCOL   **ThisVol,
  IN  CHAR16                               *FileName,
  OUT UINTN                                *BufferSize,
  OUT VOID                                 **Buffer,
  IN  BOOLEAN                              ScanFs
  )
{
  EFI_STATUS                        Status;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL   *Vol;
  UINTN                             TempBufferSize;
  VOID                              *TempBuffer;
  UINTN                             NoHandles;
  EFI_HANDLE                        *HandleBuffer;
  UINTN                             Index;

  //
  // Check parameters
  //
  if ((FileName == NULL) || (Buffer == NULL) || (ThisVol == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // not scan fs
  //
  if (!ScanFs) {
    if (*ThisVol == NULL) {
      *ThisVol = GetMyVol ();
      if (*ThisVol == NULL) {
        return EFI_INVALID_PARAMETER;
      }
    }
    //
    // Read file directly from Vol
    //
    return ReadFileFromVol (*ThisVol, FileName, BufferSize, Buffer);
  }

  //
  // need scan fs
  //

  //
  // Get all Vol handle
  //
  Status = gBS->LocateHandleBuffer (
                   ByProtocol,
                   &gEfiSimpleFileSystemProtocolGuid,
                   NULL,
                   &NoHandles,
                   &HandleBuffer
                   );
  if (EFI_ERROR (Status) && (NoHandles == 0)) {
    return EFI_NOT_FOUND;
  }
  
  //
  // Walk through each Vol
  //
  *ThisVol = NULL;
  *BufferSize = 0;
  *Buffer     = NULL;
  for (Index = 0; Index < NoHandles; Index++) {
    Status = gBS->HandleProtocol (
                    HandleBuffer[Index],
                    &gEfiSimpleFileSystemProtocolGuid,
                    (VOID **)&Vol
                    );
    if (EFI_ERROR(Status)) {
      continue;
    }

    Status = ReadFileFromVol (Vol, FileName, &TempBufferSize, &TempBuffer);
    if (!EFI_ERROR (Status)) {
      //
      // Read file OK, check duplication
      //
      if (*ThisVol != NULL) {
        //
        // Find the duplicated file
        //
        gBS->FreePool (TempBuffer);
        gBS->FreePool (*Buffer);
        Print (L"Duplicated FileName found!\n");
        return EFI_NO_MAPPING;
      } else {
        //
        // Record value
        //
        *ThisVol = Vol;
        *BufferSize = TempBufferSize;
        *Buffer     = TempBuffer;
      }
    }
  }

  //
  // Scan Fs done
  //
  if (*ThisVol == NULL) {
    return EFI_NOT_FOUND;
  }

  //
  // Done
  //
  return EFI_SUCCESS;
}

EFI_STATUS
ReadFileToBuffer (
  IN  CHAR16                               *FileName,
  OUT UINTN                                *BufferSize,
  OUT VOID                                 **Buffer
  )
{
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL   *Vol;
  Vol = NULL;
  return ReadFileToBufferEx(&Vol, FileName, BufferSize, Buffer, FALSE);
}

/**
  Get file name under this dir with index.

  @param Vol             File System Volume
  @param DirName         The dir to be read.
  @param FileSuffixName  The file name pattern under this dir
  @param Index           The file index under this dir

  @return File Name which match the pattern and index.
**/
CHAR16 *
GetFileNameUnderDir (
  IN  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL   *Vol,
  IN  CHAR16                            *DirName,
  IN  CHAR16                            *FileSuffixName,
  IN OUT UINTN                          *Index
  )
{
  EFI_STATUS                        Status;
  EFI_FILE_HANDLE                   RootDir;
  EFI_FILE_HANDLE                   Handle;
  UINTN                             FileInfoSize;
  EFI_FILE_INFO                     *FileInfo;
  VOID                              *TempName;
  UINTN                             FileIndex;

  //
  // Open the root directory
  //
  Status = Vol->OpenVolume (Vol, &RootDir);
  if (EFI_ERROR (Status)) {
    return NULL;
  }

  //
  // Open the file
  //
  Status = RootDir->Open (
                      RootDir,
                      &Handle,
                      DirName,
                      EFI_FILE_MODE_READ,
                      EFI_FILE_DIRECTORY
                      );
  if (EFI_ERROR (Status)) {
    RootDir->Close (RootDir);
    return NULL;
  }
  RootDir->Close (RootDir);

  //
  // Set Dir Position
  //
  Status = Handle->SetPosition (Handle, 0);
  if (EFI_ERROR (Status)) {
    Handle->Close (Handle);
    return NULL;
  }

  //
  // Get the file information
  //
  FileInfoSize = sizeof(EFI_FILE_INFO) + 1024;

  FileInfo = AllocateZeroPool (FileInfoSize);
  if (FileInfo == NULL) {
    Handle->Close (Handle);
    return NULL;
  }

  //
  // Walk through each file in the directory
  //
  FileIndex = 0;
  TempName = NULL;
  while (TRUE) {
    //
    // Read a file entry
    //
    FileInfoSize = sizeof(EFI_FILE_INFO) + 1024;

    Status = Handle->Read (
                       Handle,
                       &FileInfoSize,
                       FileInfo
                       );
    if (EFI_ERROR (Status) || (FileInfoSize == 0)) {
      break;
    }

    if ((FileInfo->Attribute & EFI_FILE_DIRECTORY) == 0) {
      //
      // This is a file
      //

      //
      // Only deal with the key file
      //
      if (!StrEndWith (FileInfo->FileName, FileSuffixName)) {
        continue;
      }

      if (FileIndex == *Index) {
        TempName = AllocateCopyPool (StrSize (FileInfo->FileName), FileInfo->FileName);
        *Index = *Index + 1;
        break;
      }
      FileIndex ++;
    }
  }

  //
  // Free resources
  //
  gBS->FreePool (FileInfo);
  Handle->Close (Handle);

  return TempName;
}
