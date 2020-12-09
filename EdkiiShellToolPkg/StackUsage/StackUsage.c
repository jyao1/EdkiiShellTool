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
#include <Library/DxeServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/HobLib.h>
#include <Library/DebugLib.h>

#define STACK_USAGE_SIGNATURE  0x5A5A5A5AA5A5A5A5ull

UINT8 *gStackBase;
UINTN gStackSize;

extern UINTN  Argc;
extern CHAR16 **Argv;

EFI_GUID mZeroGuid;

EFI_STATUS
GetArg (
  VOID
  );

EFI_STATUS
GetStackInfo (
  VOID
  )
{
  EFI_STATUS                  Status;
  VOID                        *HobList;
  EFI_PEI_HOB_POINTERS        Hob;
  EFI_HOB_MEMORY_ALLOCATION   *MemoryHob;
  UINT64                      UsedLength;
  
  //
  // Get Hob list
  //
  Status = EfiGetSystemConfigurationTable (&gEfiHobListGuid, &HobList);
  if (EFI_ERROR (Status)) {
    Print (L"HOB List not found\n");
    return EFI_NOT_FOUND;
  }

  for (Hob.Raw = HobList; !END_OF_HOB_LIST (Hob); Hob.Raw = GET_NEXT_HOB (Hob)) {
    if (GET_HOB_TYPE(Hob) == EFI_HOB_TYPE_MEMORY_ALLOCATION) {

      MemoryHob = Hob.MemoryAllocation;

      if (!CompareGuid(&mZeroGuid, &MemoryHob->AllocDescriptor.Name)) {
        if (CompareGuid(&gEfiHobMemoryAllocStackGuid, &MemoryHob->AllocDescriptor.Name)) {
          Print(
            L"  Stack: Address=0x%lx  Length=0x%lx (%dKB)\n",
            MemoryHob->AllocDescriptor.MemoryBaseAddress,
            MemoryHob->AllocDescriptor.MemoryLength,
            MemoryHob->AllocDescriptor.MemoryLength / SIZE_1KB
            );
          gStackBase = (UINT8 *)(UINTN)MemoryHob->AllocDescriptor.MemoryBaseAddress;
          gStackSize = (UINTN)MemoryHob->AllocDescriptor.MemoryLength;
          UsedLength = MemoryHob->AllocDescriptor.MemoryBaseAddress + MemoryHob->AllocDescriptor.MemoryLength - (UINT64)(UINTN)&Status;
          Print(
            L"  Current Stack: Address=0x%lx UsedLength=0x%lx (%dKB)\n",
            (UINT64)(UINTN)&Status,
            UsedLength,
            (UsedLength + SIZE_1KB - 1) / SIZE_1KB
            );
          return EFI_SUCCESS;
        }
      }
    }
  }
  Print (L"Stack Hob not found\n");
  return EFI_NOT_FOUND;
}

VOID
StackUsageCheckBegin (
  VOID
  )
{
  EFI_STATUS                  Status;
  UINT64                      BeginAddress;

  GetStackInfo ();

  BeginAddress = (UINT64)(UINTN)&Status & ~0x7ull;
  Status = gRT->SetVariable (
                  L"StackUsageStartPoint",
                  &gEfiCallerIdGuid,
                  EFI_VARIABLE_BOOTSERVICE_ACCESS,
                  sizeof(UINT64),
                  &BeginAddress
                  );
  if (EFI_ERROR (Status)) {
    Print (L"StackUsageBegin fail - fail to set variable\n");
    return ;
  }
  ASSERT (BeginAddress >= (UINTN)gStackBase);
  if (BeginAddress - (UINTN)gStackBase <= SIZE_4KB) {
    Print (L"StackUsageBegin fail - too small space\n");
    return ;
  }
  Print (
    L"  Tag [0x%lx - 0x%lx]\n",
    (UINT64)(UINTN)gStackBase,
    (UINT64)(UINTN)gStackBase + BeginAddress- (UINTN)gStackBase - SIZE_1KB
    );
  SetMem64 (
    gStackBase,
    (UINTN)BeginAddress- (UINTN)gStackBase - SIZE_1KB,
    STACK_USAGE_SIGNATURE
    );
}

VOID
StackUsageCheckEnd (
  VOID
  )
{
  UINT64                      *Ptr;
  EFI_STATUS                  Status;
  UINT64                      EndAddress;
  UINT64                      BeginAddress;
  UINTN                       DataLength;

  GetStackInfo ();

  EndAddress = 0;
  for (Ptr = (VOID *)gStackBase; (UINTN)Ptr < (UINTN)(gStackBase + gStackSize); Ptr+= 1) {
    if (*Ptr != STACK_USAGE_SIGNATURE) {
      EndAddress = (UINTN)Ptr;
      break;
    }
  }
  if (EndAddress == 0) {
    Print (L"StackUsageEnd fail - full space\n");
    return ;
  }

  DataLength = sizeof(UINT64);
  Status = gRT->GetVariable (
                  L"StackUsageStartPoint",
                  &gEfiCallerIdGuid,
                  NULL,
                  &DataLength,
                  &BeginAddress
                  );
  if (EFI_ERROR (Status)) {
    Print (L"StackUsageEnd fail - fail to get variable\n");
    return ;
  }
  Print (L"  BeginAddress - 0x%lx, EndAddress - 0x%lx\n", BeginAddress, EndAddress);
  Print (
    L"  Stack Used - 0x%lx (%dKB)\n",
    BeginAddress - EndAddress,
    (BeginAddress - EndAddress + SIZE_1KB - 1) / SIZE_1KB
    );
}

/**
  Print APP usage.
**/
VOID
PrintUsage (
  VOID
  )
{
  Print(L"StackUsage:  A tool to check stack usage\n");
  Print(L"  StackUsage [-info|-begin|-end]\n");
  Print(L"Parameter:\n");
  Print(L"  -info : Display stack allocation and current stack usage\n");
  Print(L"  -begin: Begin to check stack usage\n");
  Print(L"  -end  : End checking stack usage and display info\n");
  Print(L"Usage:\n");
  Print(L"  Step 1: > StackUage.efi -begin\n");
  Print(L"  Step 2: > TestApp.efi\n");
  Print(L"  Step 3: > StackUage.efi -end\n");
}

EFI_STATUS
EFIAPI
InitializeStackUsage (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  GetArg ();

  if (Argc == 2) {
    if (StrCmp (Argv[1], L"-h") == 0) {
      PrintUsage ();
    } else if (StrCmp (Argv[1], L"-info") == 0) {
      GetStackInfo ();
    } else if (StrCmp (Argv[1], L"-begin") == 0) {
      StackUsageCheckBegin ();
    } else if (StrCmp (Argv[1], L"-end") == 0) {
      StackUsageCheckEnd ();
    } else {
      PrintUsage ();
    }
  } else {
    PrintUsage ();
  }

  return EFI_SUCCESS;
}
