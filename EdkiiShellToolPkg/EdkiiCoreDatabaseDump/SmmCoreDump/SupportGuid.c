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
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiLib.h>

#include "SmmCoreDump.h"

#define GUID_STRING_LENGTH_MAX  64
typedef struct {
  EFI_GUID   Guid;
  CHAR8      Str[GUID_STRING_LENGTH_MAX];
} EFI_GUID_STRING;

EFI_GUID_STRING  *mGuidString;
UINTN            mGuidStringCountMax;
UINTN            mGuidStringCount;

VOID
AddGuidName(
  IN CHAR8    *GuidStr,
  IN CHAR8    *NameStr
  )
{
  EFI_GUID    Guid;
  EFI_STATUS  Status;

  if (mGuidStringCount >= mGuidStringCountMax) {
    ASSERT(FALSE);
    return;
  }

  Status = AsciiStrToGuid(GuidStr, &Guid);
  if (EFI_ERROR(Status)) {
    return;
  }

  if (AsciiStrCmp(GuidStr, "00000000-0000-0000-0000-000000000000") == 0) {
    CopyGuid(&mGuidString[mGuidStringCount].Guid, &Guid);
    AsciiStrnCpyS(mGuidString[mGuidStringCount].Str, sizeof(mGuidString[mGuidStringCount].Str), "ZeroGuid", sizeof(mGuidString[mGuidStringCount].Str) - 1);
  } else {
    CopyGuid(&mGuidString[mGuidStringCount].Guid, &Guid);
    AsciiStrnCpyS(mGuidString[mGuidStringCount].Str, sizeof(mGuidString[mGuidStringCount].Str), NameStr, sizeof(mGuidString[mGuidStringCount].Str) - 1);
  }

  mGuidStringCount++;
}

VOID
InitGuid(
  VOID
  )
{
  EFI_STATUS  Status;
  UINTN       BufferSize;
  VOID        *Buffer;
  CHAR8       *LineBuffer;
  CHAR8       *GuidStr;
  CHAR8       *NameStr;
  UINTN       Index;

  Status = ReadFileToBuffer(L"Guid.xref", &BufferSize, &Buffer);
  if (EFI_ERROR(Status)) {
    return;
  }

  mGuidStringCountMax = 0;
  for (Index = 0; Index < BufferSize; Index++) {
    if (*((CHAR8 *)Buffer + Index) == '\n') {
      mGuidStringCountMax++;
    }
  }
  mGuidStringCountMax++;
  mGuidString = AllocateZeroPool(mGuidStringCountMax * sizeof(EFI_GUID_STRING));
  if (mGuidString == NULL) {
    return;
  }

  LineBuffer = AsciiStrGetNewTokenLine(Buffer, "\n\r");
  while (LineBuffer != NULL) {
    GuidStr = AsciiStrGetNewTokenField(LineBuffer, " ");
    NameStr = AsciiStrGetNextTokenField(" ");
    if (GuidStr != NULL && NameStr != NULL) {
      AddGuidName(GuidStr, NameStr);
    }
    LineBuffer = AsciiStrGetNextTokenLine("\n\r");
  }

  FreePool(Buffer);

  return;
}

CHAR8 mGuidName[sizeof("12345678-1234-1234-1234-1234567890AB")];

CHAR8 *
GuidToName(
  IN EFI_GUID  *Guid
  )
{
  UINTN  Index;

  for (Index = 0; Index < mGuidStringCount; Index++) {
    if (CompareGuid(&mGuidString[Index].Guid, Guid)) {
      return mGuidString[Index].Str;
    }
  }

  AsciiSPrint(mGuidName, sizeof(mGuidName), "%g", Guid);

  return mGuidName;
}

VOID
DeinitGuid(
  VOID
  )
{
  if (mGuidString != NULL) {
    FreePool(mGuidString);
  }
}