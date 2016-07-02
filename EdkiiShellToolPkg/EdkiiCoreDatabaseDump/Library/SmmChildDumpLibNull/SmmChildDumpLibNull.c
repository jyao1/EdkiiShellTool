/** @file

Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <PiSmm.h>

EFI_STATUS
EFIAPI
SmmChildInit(
  VOID
  )
{
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
SmmChildDump(
  VOID
  )
{
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
SmmChildGetDataSize(
  OUT UINTN  *Size
  )
{
  *Size = 0;
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
SmmChildGetData(
  IN     UINTN  Size,
  IN OUT VOID   *Data
  )
{
  return EFI_UNSUPPORTED;
}

VOID
EFIAPI
DumpSmmChildHandler(
  IN VOID  *Data,
  IN UINTN Size
  )
{
  return;
}