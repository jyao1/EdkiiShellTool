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
#include <Library/PrintLib.h>
#include <Library/DevicePathLib.h>
#include <Protocol/DevicePath.h>

extern UINTN  Argc;
extern CHAR16 **Argv;

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

VOID
DumpBootOption (
  IN UINT16  BootIndex
  )
{
  UINTN                     OptionSize;
  EFI_LOAD_OPTION           *Option;
  CHAR16                    OptionName[sizeof("Boot####")];
  CHAR16                    *Description;
  UINTN                     DescriptionSize;
  EFI_DEVICE_PATH_PROTOCOL  *FilePathList;
  CHAR16                    *FilePathListStr;
  VOID                      *OptionalData;
  UINTN                     OptionalDataSize;
  UINTN                     Index;
  EFI_STATUS                Status;

  UnicodeSPrint (OptionName, sizeof (OptionName), L"Boot%04x", BootIndex);

  Status = GetVariable2 (
             OptionName,
             &gEfiGlobalVariableGuid,
             (void **)&Option,
             &OptionSize
             );
  Print (L"Get %s - %r\n", OptionName, Status);

  Print (L"  Attributes         - 0x%08x\n", Option->Attributes);
  Print (L"  FilePathListLength - 0x%04x\n", Option->FilePathListLength);
  Description = (VOID *)(Option + 1);
  DescriptionSize = StrSize (Description);
  Print (L"  Description        - %s\n", Description);
  FilePathList = (VOID *)((UINTN)Description + DescriptionSize);
  FilePathListStr = ConvertDevicePathToText (FilePathList, FALSE, FALSE);
  Print (L"  FilePathList       - %s\n", FilePathListStr);
  OptionalData = (VOID *)((UINTN)FilePathList + Option->FilePathListLength);
  OptionalDataSize = (UINTN)Option + OptionSize - (UINTN)OptionalData;
  Print (L"  OptionalData       - ");
  for (Index = 0; Index < OptionalDataSize; Index++) {
    Print (L"%02x ", *((UINT8 *)OptionalData + Index));
  }
  Print (L"\n");

  FreePool (Option);
}

VOID
DumpAllBootOption (
  VOID
  )
{
  EFI_STATUS  Status;
  UINTN       BufferSize;
  UINT16      *Buffer;
  UINTN       Index;

  Status = GetVariable2 (
             L"BootOrder",
             &gEfiGlobalVariableGuid,
             (void **)&Buffer,
             &BufferSize
             );
  Print (L"Get BootOrder - %r\n", Status);

  Print (L"Boot Order - ");
  for (Index = 0; Index < BufferSize/sizeof(CHAR16); Index++) {
    Print (L"0x%04x ", Buffer[Index]);
  }
  Print (L"\n");

  for (Index = 0; Index < BufferSize/sizeof(CHAR16); Index++) {
    DumpBootOption (Buffer[Index]);
  }

  FreePool (Buffer);
}

UINT16
GetNewBootIndex (
  VOID
  )
{
  EFI_STATUS  Status;
  UINTN       BufferSize;
  UINT16      *Buffer;
  UINTN       Index;
  UINTN       NewIndex;
  BOOLEAN     Match;
  UINTN       NewBufferSize;
  UINT16      *NewBuffer;

  Status = GetVariable2 (
             L"BootOrder",
             &gEfiGlobalVariableGuid,
             (void **)&Buffer,
             &BufferSize
             );
  if (EFI_ERROR(Status)) {
    NewIndex = 0;
    NewBufferSize = sizeof(UINT16);
    NewBuffer = AllocateZeroPool (NewBufferSize);
    ASSERT(NewBuffer != NULL);
    *(UINT16 *)NewBuffer = (UINT16)NewIndex;
    Status = gRT->SetVariable (
                    L"BootOrder",
                    &gEfiGlobalVariableGuid,
                    EFI_VARIABLE_NON_VOLATILE |
                      EFI_VARIABLE_BOOTSERVICE_ACCESS |
                      EFI_VARIABLE_RUNTIME_ACCESS,
                    NewBufferSize,
                    NewBuffer
                    );
    ASSERT_EFI_ERROR(Status);
    FreePool (NewBuffer);
    return (UINT16)NewIndex;
  }

  for (NewIndex = 0; NewIndex < 0xFFFF; NewIndex++) {
    Match = FALSE;
    for (Index = 0; Index < BufferSize/sizeof(CHAR16); Index++) {
      if (NewIndex == Index) {
        Match = TRUE;
      }
    }
    if (!Match) {
      NewBufferSize = BufferSize + sizeof(UINT16);
      NewBuffer = AllocateZeroPool (NewBufferSize);
      ASSERT(NewBuffer != NULL);
      *(UINT16 *)NewBuffer = (UINT16)NewIndex;
      CopyMem ((UINT8 *)NewBuffer + sizeof(UINT16), Buffer, BufferSize);
      Status = gRT->SetVariable (
                      L"BootOrder",
                      &gEfiGlobalVariableGuid,
                      EFI_VARIABLE_NON_VOLATILE |
                        EFI_VARIABLE_BOOTSERVICE_ACCESS |
                        EFI_VARIABLE_RUNTIME_ACCESS,
                      NewBufferSize,
                      NewBuffer
                      );
      ASSERT_EFI_ERROR(Status);

      FreePool (Buffer);
      FreePool (NewBuffer);
      return (UINT16)NewIndex;
    }
  }
  FreePool (Buffer);

  ASSERT(FALSE);
  return 0xFFFF;
}

VOID
AddBootOption (
  IN CHAR16  *FilePath
  )
{
  UINT16                    NewBootIndex;
  UINTN                     OptionSize;
  EFI_LOAD_OPTION           *Option;
  CHAR16                    OptionName[sizeof("Boot####")];
  UINTN                     DescriptionSize;
  EFI_DEVICE_PATH_PROTOCOL  *FilePathList;
  UINTN                     FilePathListSize;
  UINTN                     OptionalDataSize;
  EFI_STATUS                Status;

  NewBootIndex = GetNewBootIndex ();

  UnicodeSPrint (OptionName, sizeof (OptionName), L"Boot%04x", NewBootIndex);

  DescriptionSize = StrSize (FilePath);
  FilePathList = ConvertTextToDevicePath (FilePath);
  FilePathListSize = GetDevicePathSize (FilePathList);
  OptionalDataSize = 0;
  OptionSize = sizeof(EFI_LOAD_OPTION) + DescriptionSize + FilePathListSize + OptionalDataSize;
  Option = AllocateZeroPool (OptionSize);
  ASSERT(Option != NULL);

  Option->Attributes = LOAD_OPTION_ACTIVE;
  Option->FilePathListLength = (UINT16)FilePathListSize;
  CopyMem ((UINT8 *)Option + sizeof(EFI_LOAD_OPTION), FilePath, DescriptionSize);
  CopyMem ((UINT8 *)Option + sizeof(EFI_LOAD_OPTION) + DescriptionSize, FilePathList, FilePathListSize);

  Status = gRT->SetVariable (
                  OptionName,
                  &gEfiGlobalVariableGuid,
                  EFI_VARIABLE_NON_VOLATILE |
                    EFI_VARIABLE_BOOTSERVICE_ACCESS |
                    EFI_VARIABLE_RUNTIME_ACCESS,
                  OptionSize,
                  Option
                  );
  ASSERT_EFI_ERROR(Status);

  FreePool (FilePathList);

  return ;
}

/**
  Print APP usage.
**/
VOID
PrintUsage (
  VOID
  )
{
  Print(L"BootOption:  A tool to manage UEFI boot option\n");
  Print(L"  BootOption -dump\n");
  Print(L"  BootOption -add <file path>\n");
}

EFI_STATUS
EFIAPI
BootOptionEntrypoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  GetArg ();

  if ((Argc == 2) && (StrCmp (Argv[1], L"-dump") == 0)) {
    DumpAllBootOption ();
    return EFI_SUCCESS;
  }

  if ((Argc == 3) && (StrCmp (Argv[1], L"-add") == 0)) {
    AddBootOption (Argv[2]);
    return EFI_SUCCESS;
  }


  PrintUsage ();

  return EFI_SUCCESS;
}
