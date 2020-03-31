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

#include <Guid/PropertiesTable.h>
#include <Guid/MemoryAttributesTable.h>

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
DumpMemoryAttributesTable (
  IN EFI_MEMORY_ATTRIBUTES_TABLE                     *MemoryAttributesTable
  )
{
  UINTN                 Index;
  EFI_MEMORY_DESCRIPTOR *Entry;
  UINT64                RTDataPages;
  UINT64                RTCodePages;

  RTDataPages = 0;
  RTCodePages = 0;

  Print (L"MemoryAttributesTable:\n");
  Print (L"  Version         - 0x%08x\n", MemoryAttributesTable->Version);
  Print (L"  NumberOfEntries - 0x%08x\n", MemoryAttributesTable->NumberOfEntries);
  Print (L"  DescriptorSize  - 0x%08x\n", MemoryAttributesTable->DescriptorSize);
  Print (L"\n");
  Entry = (EFI_MEMORY_DESCRIPTOR *)(MemoryAttributesTable + 1);
  Print (L"Type       Start            End              # Pages          Attributes\n");
  for (Index = 0; Index < MemoryAttributesTable->NumberOfEntries; Index++) {
    Print(L"% -10s %016LX-%016LX %016LX %016LX\n", 
      ShortNameOfMemoryType(Entry->Type),
      Entry->PhysicalStart,
      Entry->PhysicalStart + EFI_PAGES_TO_SIZE((UINTN)Entry->NumberOfPages) - 1,
      Entry->NumberOfPages,
      Entry->Attribute
      );
    switch (Entry->Type) {
    case EfiRuntimeServicesCode:
      RTCodePages += Entry->NumberOfPages;
      break;
    case EfiRuntimeServicesData:
      RTDataPages += Entry->NumberOfPages;
      break;
    default:
      break;
    }
    Entry = NEXT_MEMORY_DESCRIPTOR (Entry, MemoryAttributesTable->DescriptorSize);
  }

  Print (L"\n");
  Print (L"  RT_Code   : %,14ld Pages (%,ld Bytes)\n", RTCodePages, MultU64x64(SIZE_4KB, RTCodePages));
  Print (L"  RT_Data   : %,14ld Pages (%,ld Bytes)\n", RTDataPages, MultU64x64(SIZE_4KB, RTDataPages));
  Print (L"              -------------- \n");
}

VOID
DumpPropertiesTable(
  IN EFI_PROPERTIES_TABLE                     *PropertiesTable
  )
{
  Print(L"PropertiesTable:\n");
  Print(L"  Version                     - 0x%08x\n", PropertiesTable->Version);
  Print(L"  Length                      - 0x%08x\n", PropertiesTable->Length);
  Print(L"  MemoryProtectionAttribute   - 0x%016lx\n", PropertiesTable->MemoryProtectionAttribute);
}

EFI_STATUS
EFIAPI
MemoryAttributesDumpEntrypoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS  Status;
  VOID        *MemoryAttributesTable;
  VOID        *PropertiesTable;
  
  Status = EfiGetSystemConfigurationTable (&gEfiMemoryAttributesTableGuid, &MemoryAttributesTable);
  if (!EFI_ERROR (Status)) {
    DumpMemoryAttributesTable(MemoryAttributesTable);
  }

  Status = EfiGetSystemConfigurationTable(&gEfiPropertiesTableGuid, &PropertiesTable);
  if (!EFI_ERROR(Status)) {
    DumpPropertiesTable(PropertiesTable);
  }

  return EFI_SUCCESS;
}
