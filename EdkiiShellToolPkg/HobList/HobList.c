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
#include <Library/DxeServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/HobLib.h>
#include <Guid/HobList.h>
#include <Guid/MemoryAllocationHob.h>

/*
#define EFI_RESOURCE_ATTRIBUTE_PRESENT                  0x00000001
#define EFI_RESOURCE_ATTRIBUTE_INITIALIZED              0x00000002
#define EFI_RESOURCE_ATTRIBUTE_TESTED                   0x00000004
#define EFI_RESOURCE_ATTRIBUTE_READ_PROTECTED           0x00000080
#define EFI_RESOURCE_ATTRIBUTE_WRITE_PROTECTED          0x00000100
#define EFI_RESOURCE_ATTRIBUTE_EXECUTION_PROTECTED      0x00000200
#define EFI_RESOURCE_ATTRIBUTE_PERSISTENT               0x00800000
#define EFI_RESOURCE_ATTRIBUTE_MORE_RELIABLE            0x02000000
#define EFI_RESOURCE_ATTRIBUTE_SINGLE_BIT_ECC           0x00000008
#define EFI_RESOURCE_ATTRIBUTE_MULTIPLE_BIT_ECC         0x00000010
#define EFI_RESOURCE_ATTRIBUTE_ECC_RESERVED_1           0x00000020
#define EFI_RESOURCE_ATTRIBUTE_ECC_RESERVED_2           0x00000040
#define EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE              0x00000400
#define EFI_RESOURCE_ATTRIBUTE_WRITE_COMBINEABLE        0x00000800
#define EFI_RESOURCE_ATTRIBUTE_WRITE_THROUGH_CACHEABLE  0x00001000
#define EFI_RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE     0x00002000
#define EFI_RESOURCE_ATTRIBUTE_16_BIT_IO                0x00004000
#define EFI_RESOURCE_ATTRIBUTE_32_BIT_IO                0x00008000
#define EFI_RESOURCE_ATTRIBUTE_64_BIT_IO                0x00010000
#define EFI_RESOURCE_ATTRIBUTE_UNCACHED_EXPORTED        0x00020000
#define EFI_RESOURCE_ATTRIBUTE_READ_ONLY_PROTECTED      0x00040000
#define EFI_RESOURCE_ATTRIBUTE_READ_PROTECTABLE         0x00100000
#define EFI_RESOURCE_ATTRIBUTE_WRITE_PROTECTABLE        0x00200000
#define EFI_RESOURCE_ATTRIBUTE_EXECUTION_PROTECTABLE    0x00400000
#define EFI_RESOURCE_ATTRIBUTE_READ_ONLY_PROTECTABLE    0x00800000
#define EFI_RESOURCE_ATTRIBUTE_PERSISTABLE              0x01000000
*/

#define MEMORY_ATTRIBUTE_MASK (EFI_RESOURCE_ATTRIBUTE_PRESENT | \
                               EFI_RESOURCE_ATTRIBUTE_INITIALIZED | \
                               EFI_RESOURCE_ATTRIBUTE_TESTED | \
                               EFI_RESOURCE_ATTRIBUTE_16_BIT_IO | \
                               EFI_RESOURCE_ATTRIBUTE_32_BIT_IO | \
                               EFI_RESOURCE_ATTRIBUTE_64_BIT_IO \
                               )

#define TESTED_MEMORY_ATTRIBUTES      (EFI_RESOURCE_ATTRIBUTE_PRESENT | EFI_RESOURCE_ATTRIBUTE_INITIALIZED | EFI_RESOURCE_ATTRIBUTE_TESTED)

#define INITIALIZED_MEMORY_ATTRIBUTES (EFI_RESOURCE_ATTRIBUTE_PRESENT | EFI_RESOURCE_ATTRIBUTE_INITIALIZED)

#define PRESENT_MEMORY_ATTRIBUTES     (EFI_RESOURCE_ATTRIBUTE_PRESENT)

EFI_GUID mZeroGuid;

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
ShortNameOfMemoryType (
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

CHAR16 *mResourceTypeShortName[] = {
  L"Mem",
  L"MMIO",
  L"I/O",
  L"FD",
  L"MM Port I/O",
  L"Reserved Mem",
  L"Reserved I/O",
};

CHAR16 *
ShortNameOfResourceType (
  IN UINT32 Type
  )
{
  if (Type < sizeof(mResourceTypeShortName) / sizeof(mResourceTypeShortName[0])) {
    return mResourceTypeShortName[Type];
  } else {
    UnicodeSPrint(mUnknownStr, sizeof(mUnknownStr), L"%08x", Type);
    return mUnknownStr;
  }
}

EFI_STATUS
EFIAPI
InitializeHobList (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
/*++

Routine Description:

  Displays memory map.

Arguments:

  ImageHandle     The image handle. 
  SystemTable     The system table.

Returns:

  EFI_SUCCESS             - Command completed successfully
  EFI_INVALID_PARAMETER   - Command usage error
  EFI_NOT_FOUND           - Memory map not found

--*/
{
  EFI_STATUS                  Status;
  VOID                        *HobList;
  EFI_PEI_HOB_POINTERS        Hob;
  EFI_HOB_RESOURCE_DESCRIPTOR *ResourceHob;
  EFI_HOB_MEMORY_ALLOCATION   *MemoryHob;
  EFI_HOB_FIRMWARE_VOLUME     *FirmwareVolumeHob;
  EFI_HOB_FIRMWARE_VOLUME2    *FirmwareVolume2Hob;
  EFI_HOB_UEFI_CAPSULE        *CapsuleHob;
  EFI_HOB_CPU                 *CpuHob;
  EFI_HOB_HANDOFF_INFO_TABLE  *PhitHob;

  //
  // Get Hob list
  //
  Status = EfiGetSystemConfigurationTable (&gEfiHobListGuid, &HobList);
  if (EFI_ERROR (Status)) {
    Print (L"HOB List not found\n");
    return EFI_NOT_FOUND;
  }

  Print(L"PHIT HOB\n");
  PhitHob = HobList;
  ASSERT(GET_HOB_TYPE(HobList) == EFI_HOB_TYPE_HANDOFF);
  Print(L"  PhitHob             - 0x%x\n", PhitHob);
  Print(L"  BootMode            - 0x%x\n", PhitHob->BootMode);
  Print(L"  EfiMemoryTop        - 0x%016lx\n", PhitHob->EfiMemoryTop);
  Print(L"  EfiMemoryBottom     - 0x%016lx\n", PhitHob->EfiMemoryBottom);
  Print(L"  EfiFreeMemoryTop    - 0x%016lx\n", PhitHob->EfiFreeMemoryTop);
  Print(L"  EfiFreeMemoryBottom - 0x%016lx\n", PhitHob->EfiFreeMemoryBottom);
  Print(L"  EfiEndOfHobList     - 0x%lx\n", PhitHob->EfiEndOfHobList);

  Print(L"CPU HOBs\n");
  for (Hob.Raw = HobList; !END_OF_HOB_LIST(Hob); Hob.Raw = GET_NEXT_HOB(Hob)) {
    if (GET_HOB_TYPE(Hob) == EFI_HOB_TYPE_CPU) {
      CpuHob = Hob.Cpu;
      Print(
        L"  SizeOfMemorySpace=%x  SizeOfIoSpace=%x\n",
        CpuHob->SizeOfMemorySpace,
        CpuHob->SizeOfIoSpace
        );
    }
  }

  Print (L"Resource Descriptor HOBs\n");
  for (Hob.Raw = HobList; !END_OF_HOB_LIST (Hob); Hob.Raw = GET_NEXT_HOB (Hob)) {

    if (GET_HOB_TYPE (Hob) == EFI_HOB_TYPE_RESOURCE_DESCRIPTOR) {

      ResourceHob = Hob.ResourceDescriptor;

      Print (
        L"  BA=%016lx  L=%016lx  Attr=%016lx  ",
        ResourceHob->PhysicalStart,
        ResourceHob->ResourceLength,
        ResourceHob->ResourceAttribute
        );

      Print (ShortNameOfResourceType(ResourceHob->ResourceType));
      switch (ResourceHob->ResourceType) {
      case EFI_RESOURCE_SYSTEM_MEMORY:
        if ((ResourceHob->ResourceAttribute & EFI_RESOURCE_ATTRIBUTE_PERSISTENT) != 0) {
          Print(L" (Persistent)");
        } else if ((ResourceHob->ResourceAttribute & EFI_RESOURCE_ATTRIBUTE_MORE_RELIABLE) != 0) {
          Print(L" (MoreReliable)");
        } else if ((ResourceHob->ResourceAttribute & MEMORY_ATTRIBUTE_MASK) == TESTED_MEMORY_ATTRIBUTES) {
          Print (L" (Tested)");
        } else if ((ResourceHob->ResourceAttribute & MEMORY_ATTRIBUTE_MASK) == INITIALIZED_MEMORY_ATTRIBUTES) {
          Print (L" (Init)");
        } else if ((ResourceHob->ResourceAttribute & MEMORY_ATTRIBUTE_MASK) == PRESENT_MEMORY_ATTRIBUTES) {
          Print (L" (Present)");
        } else {
          Print (L" (Unknown)");
        }
        break;
      default:
        break;
      }
      Print (L"\n");
    }
  }

  Print (L"FV HOBs\n");
  for (Hob.Raw = HobList; !END_OF_HOB_LIST (Hob); Hob.Raw = GET_NEXT_HOB (Hob)) {
    if (GET_HOB_TYPE (Hob) == EFI_HOB_TYPE_FV) {

      FirmwareVolumeHob = Hob.FirmwareVolume;

      Print (
        L"  BA=%016lx  L=%016lx\n",
        FirmwareVolumeHob->BaseAddress,
        FirmwareVolumeHob->Length
        );
    }
  }

  Print (L"FV2 HOBs\n");
  for (Hob.Raw = HobList; !END_OF_HOB_LIST (Hob); Hob.Raw = GET_NEXT_HOB (Hob)) {
    if (GET_HOB_TYPE (Hob) == EFI_HOB_TYPE_FV2) {

      FirmwareVolume2Hob = Hob.FirmwareVolume2;

      Print (
        L"  BA=%016lx  L=%016lx  Fv={%g}  File={%g}\n",
        FirmwareVolume2Hob->BaseAddress,
        FirmwareVolume2Hob->Length,
        &FirmwareVolume2Hob->FvName,
        &FirmwareVolume2Hob->FileName
        );
    }
  }

  Print (L"Memory Allocation HOBs\n");
  for (Hob.Raw = HobList; !END_OF_HOB_LIST (Hob); Hob.Raw = GET_NEXT_HOB (Hob)) {
    if (GET_HOB_TYPE(Hob) == EFI_HOB_TYPE_MEMORY_ALLOCATION) {

      MemoryHob = Hob.MemoryAllocation;

      Print(
        L"  BA=%016lx  L=%016lx  ",
        MemoryHob->AllocDescriptor.MemoryBaseAddress,
        MemoryHob->AllocDescriptor.MemoryLength
        );
      Print(ShortNameOfMemoryType(MemoryHob->AllocDescriptor.MemoryType));
      if (!CompareGuid(&mZeroGuid, &MemoryHob->AllocDescriptor.Name)) {
        if (CompareGuid(&gEfiHobMemoryAllocStackGuid, &MemoryHob->AllocDescriptor.Name)) {
          Print(
            L"  {Stack}",
            &MemoryHob->AllocDescriptor.Name
            );
        } else if (CompareGuid(&gEfiHobMemoryAllocBspStoreGuid, &MemoryHob->AllocDescriptor.Name)) {
          Print(
            L"  {BspStore}",
            &MemoryHob->AllocDescriptor.Name
            );
        } else if (CompareGuid(&gEfiHobMemoryAllocModuleGuid, &MemoryHob->AllocDescriptor.Name)) {
          Print(
            L"  {Module=%g,Entry=0x%lx}",
            &((EFI_HOB_MEMORY_ALLOCATION_MODULE *)MemoryHob)->ModuleName,
            ((EFI_HOB_MEMORY_ALLOCATION_MODULE *)MemoryHob)->EntryPoint
            );
        } else {
          Print (
            L"  {%g}",
            &MemoryHob->AllocDescriptor.Name
            );
        }
      }
      Print (L"\n");
    }
  }

  Print(L"Capsule HOBs\n");
  for (Hob.Raw = HobList; !END_OF_HOB_LIST(Hob); Hob.Raw = GET_NEXT_HOB(Hob)) {
    if (GET_HOB_TYPE(Hob) == EFI_HOB_TYPE_UEFI_CAPSULE) {
      CapsuleHob = Hob.Capsule;
      Print(
        L"  BA=%016lx  L=%016lx\n",
        CapsuleHob->BaseAddress,
        CapsuleHob->Length
        );
    }
  }

  return EFI_SUCCESS;
}
