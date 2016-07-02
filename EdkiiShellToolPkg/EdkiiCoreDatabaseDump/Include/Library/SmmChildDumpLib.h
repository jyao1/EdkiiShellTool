/** @file

Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef  _SMM_CHILD_DUMP_LIB_H_
#define  _SMM_CHILD_DUMP_LIB_H_

//
// Below APIs are called by consumer SMM driver
//

EFI_STATUS
EFIAPI
SmmChildInit(
  VOID
  );

EFI_STATUS
EFIAPI
SmmChildDump(
  VOID
  );

EFI_STATUS
EFIAPI
SmmChildGetDataSize(
  OUT UINTN  *Size
  );

EFI_STATUS
EFIAPI
SmmChildGetData(
  IN     UINTN  Size,
  IN OUT VOID   *Data
  );

//
// Below APIs are called by consumer APP
//

VOID
EFIAPI
DumpSmmChildHandler(
  IN VOID  *Data,
  IN UINTN Size
  );

//
// Below APIs are library consumer provided functions in SMM
//

CHAR16 *
AddressToImageName(
  IN UINTN Address
  );

UINTN
AddressToImageRef(
  IN UINTN  Address
  );

//
// Below APIs are library consumer provided functions in APP
//

CHAR16 *
GetImageRefName(
  IN UINTN ImageRef
  );

CHAR8 *
GuidToName(
  IN EFI_GUID *Guid
  );

#endif
