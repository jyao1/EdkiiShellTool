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
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiLib.h>
#include <Library/HobLib.h>

#include <Guid/MemoryTypeInformation.h>

CHAR16 *mMemoryTypeShortName[] = {
  L"Reserved",
  L"LoaderCode",
  L"LoaderData",
  L"BS_Code",
  L"BS_Data",
  L"RT_Code",
  L"RT_Data",
  L"Available",
  L"Unusable",
  L"ACPI_Recl",
  L"ACPI_NVS",
  L"MMIO",
  L"MMIO_Port",
  L"PalCode",
  L"Persistent",
};

CHAR16 mUnknownStr[11];

CHAR16 *
ShortNameOfMemoryType(
  IN UINT32 Type
  )
{
  if (Type < sizeof(mMemoryTypeShortName) / sizeof(mMemoryTypeShortName[0])) {
    return mMemoryTypeShortName[Type];
  } else {
    UnicodeSPrint(mUnknownStr, sizeof(mUnknownStr), L"%08x", Type);
    return mUnknownStr;
  }
}

VOID
DumpMemoryTypeInformation (
  IN EFI_MEMORY_TYPE_INFORMATION                     *MemoryTypeInformation
  )
{
  UINTN                 Index;

  for (Index = 0; MemoryTypeInformation[Index].Type != EfiMaxMemoryType; Index++) {
    Print (L"  % -10s : 0x%08x\n", ShortNameOfMemoryType(MemoryTypeInformation[Index].Type), MemoryTypeInformation[Index].NumberOfPages);
  }
}

VOID
DumpMemoryTypeInfoSummary (
  IN EFI_MEMORY_TYPE_INFORMATION *CurrentMemoryTypeInformation,
  IN EFI_MEMORY_TYPE_INFORMATION *PreviousMemoryTypeInformation
  )
{
  UINTN                        Index;
  UINTN                        Index1;
  EFI_BOOT_MODE                BootMode;
  UINT32                       Previous;
  UINT32                       Current;
  UINT32                       Next;
  BOOLEAN                      MemoryTypeInformationModified;

  MemoryTypeInformationModified = FALSE;
  BootMode = GetBootModeHob();

  //
  // Use a heuristic to adjust the Memory Type Information for the next boot
  //
  Print (L"\n");
  Print (L"             (HOB)   (ConfTabl)  (Var)  \n");
  Print (L"  Memory    Previous  Current    Next   \n");
  Print (L"   Type      Pages     Pages     Pages  \n");
  Print (L"==========  ========  ========  ========\n");

  for (Index = 0; PreviousMemoryTypeInformation[Index].Type != EfiMaxMemoryType; Index++) {

    for (Index1 = 0; CurrentMemoryTypeInformation[Index1].Type != EfiMaxMemoryType; Index1++) {
      if (PreviousMemoryTypeInformation[Index].Type == CurrentMemoryTypeInformation[Index1].Type) {
        break;
      }
    }
    if (CurrentMemoryTypeInformation[Index1].Type == EfiMaxMemoryType) {
      continue;
    }

    //
    // Previous is the number of pages pre-allocated
    // Current is the number of pages actually needed
    //
    Previous = PreviousMemoryTypeInformation[Index].NumberOfPages;
    Current = CurrentMemoryTypeInformation[Index1].NumberOfPages;
    Next = Previous;

    //
    // Inconsistent Memory Reserved across bootings may lead to S4 fail
    // Write next varible to 125% * current when the pre-allocated memory is:
    //  1. More than 150% of needed memory and boot mode is BOOT_WITH_DEFAULT_SETTING
    //  2. Less than the needed memory
    //
    if ((Current + (Current >> 1)) < Previous) {
      if (BootMode == BOOT_WITH_DEFAULT_SETTINGS) {
        Next = Current + (Current >> 2);
      }
    } else if (Current > Previous) {
      Next = Current + (Current >> 2);
    }
    if (Next > 0 && Next < 4) {
      Next = 4;
    }

    if (Next != Previous) {
      PreviousMemoryTypeInformation[Index].NumberOfPages = Next;
      MemoryTypeInformationModified = TRUE;
    }

    Print (L"%-10s  %08x  %08x  %08x\n", ShortNameOfMemoryType(PreviousMemoryTypeInformation[Index].Type), Previous, Current, Next);
  }
  Print(L"\n");

  if (MemoryTypeInformationModified) {
    Print (L"MemoryTypeInformation - Modified. RESET Needed!!!\n");
  } else {
    Print (L"MemoryTypeInformation - Unmodified.\n");
  }
  Print(L"\n");
}

EFI_STATUS
EFIAPI
MemoryTypeInfoEntrypoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS              Status;
  EFI_HOB_GUID_TYPE       *GuidHob;
  VOID                    *CurrentMemoryTypeInformation;
  VOID                    *PreviousMemoryTypeInformation;
  VOID                    *VariableMemoryTypeInformation;
  
  CurrentMemoryTypeInformation = NULL;
  PreviousMemoryTypeInformation = NULL;

  Status = EfiGetSystemConfigurationTable (&gEfiMemoryTypeInformationGuid, &CurrentMemoryTypeInformation);
  if (!EFI_ERROR (Status)) {
    //Print (L"MemoryTypeInfo in UEFI Configuration Table (Current, actually needed):\n");
    //DumpMemoryTypeInformation (CurrentMemoryTypeInformation);
  }

  GuidHob = GetFirstGuidHob (&gEfiMemoryTypeInformationGuid);
  if (GuidHob != NULL) {
    PreviousMemoryTypeInformation = GET_GUID_HOB_DATA(GuidHob);
    //Print (L"MemoryTypeInfo in Hob (Previous, preallocated):\n");
    //DumpMemoryTypeInformation (PreviousMemoryTypeInformation);
  }

  Status = GetVariable2 (
             EFI_MEMORY_TYPE_INFORMATION_VARIABLE_NAME,
             &gEfiMemoryTypeInformationGuid,
             &VariableMemoryTypeInformation,
             NULL
             );
  if (!EFI_ERROR(Status)) {
    //Print (L"MemoryTypeInfo in Variable (Next):\n");
    //DumpMemoryTypeInformation (VariableMemoryTypeInformation);
  }

  if ((CurrentMemoryTypeInformation != NULL) && (PreviousMemoryTypeInformation != NULL)) {
    DumpMemoryTypeInfoSummary(CurrentMemoryTypeInformation, PreviousMemoryTypeInformation);
  }

  return EFI_SUCCESS;
}
