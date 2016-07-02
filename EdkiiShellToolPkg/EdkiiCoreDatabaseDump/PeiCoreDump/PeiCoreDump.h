/** @file

Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef  _PEI_CORE_DUMP_H_
#define  _PEI_CORE_DUMP_H_

#define UNKNOWN_NAME  L"???"

#define NAME_STRING_LENGTH  35
typedef struct {
  EFI_GUID FileGuid;
  UINTN    EntryPoint;
  UINTN    ImageBase;
  UINTN    ImageSize;
  UINTN    LoadedImageBase;
  CHAR16   NameString[NAME_STRING_LENGTH + 1];
} IMAGE_STRUCT;

VOID
BuildPeiCoreDatabase(
  IN VOID   *PrivateData
  );

UINTN
AddressToImageRef(
  IN UINTN  Address
  );

UINTN
AddressToImageRefEx(
  IN UINTN     Address,
  IN EFI_GUID  *Protocol
  );

VOID
InitGuid(
  VOID
  );

VOID
DeinitGuid(
  VOID
  );

CHAR8 *
GuidToName(
  IN EFI_GUID  *Guid
  );

EFI_STATUS
ReadFileToBuffer(
  IN  CHAR16                               *FileName,
  OUT UINTN                                *BufferSize,
  OUT VOID                                 **Buffer
  );

CHAR8 *
AsciiStrGetNewTokenLine(
  IN CHAR8                       *String,
  IN CHAR8                       *CharSet
  );

CHAR8 *
AsciiStrGetNextTokenLine(
  IN CHAR8                       *CharSet
  );

CHAR8 *
AsciiStrGetNewTokenField(
  IN CHAR8                       *String,
  IN CHAR8                       *CharSet
  );

CHAR8 *
AsciiStrGetNextTokenField(
  IN CHAR8                       *CharSet
  );

BOOLEAN
StrEndWith(
  IN CHAR16                       *Str,
  IN CHAR16                       *SubStr
  );

EFI_STATUS
AsciiStrToGuid(
  IN  CHAR8    *Str,
  OUT EFI_GUID *Guid
  );

#endif
