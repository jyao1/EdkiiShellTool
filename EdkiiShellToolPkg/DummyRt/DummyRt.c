/** @file
  Copyright (c) 2018 - 2019, Intel Corporation. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

VOID *mRtData;
VOID *mRtCode;

/**
  Main entry for this driver/library.

  @param[in] ImageHandle  Image handle this driver.
  @param[in] SystemTable  Pointer to SystemTable.

**/
EFI_STATUS
EFIAPI
TestEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  Print (L"RT .code - 0x%p\n", TestEntryPoint);
  Print (L"RT .data - 0x%p\n", &mRtData);

  gBS->AllocatePool (EfiRuntimeServicesCode, 1, &mRtCode);
  gBS->AllocatePool (EfiRuntimeServicesData, 1, &mRtData);
  Print (L"RT Code - 0x%p\n", mRtCode);
  Print (L"RT Data - 0x%p\n", mRtData);
  return EFI_SUCCESS;
}
