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
#include <Library/UefiLib.h>

#include <IndustryStandard/Hsti.h>
#include <IndustryStandard/Acpi.h>
#include <IndustryStandard/WindowsSmmSecurityMitigationTable.h>

#include <Guid/Acpi.h>

#include <Protocol/AdapterInformation.h>

VOID
DumpHsti (
  IN VOID                     *HstiData
  )
{
  ADAPTER_INFO_PLATFORM_SECURITY *Hsti;
  UINT8                          *SecurityFeatures;
  CHAR16                         *ErrorString;
  UINTN                          Index;
  CHAR16                         ErrorChar;

  Hsti = HstiData;
  Print (L"HSTI\n");
  Print (L"  Version                     - 0x%08x\n", Hsti->Version);
  Print (L"  Role                        - 0x%08x\n", Hsti->Role);
  Print (L"  ImplementationID            - %S\n", Hsti->ImplementationID);
  Print (L"  SecurityFeaturesSize        - 0x%08x\n", Hsti->SecurityFeaturesSize);

  SecurityFeatures = (UINT8 *)(Hsti + 1);
  Print (L"  SecurityFeaturesRequired    - ");
  for (Index = 0; Index < Hsti->SecurityFeaturesSize; Index++) {
    Print (L"%02x ", SecurityFeatures[Index]);
  }
  Print (L"\n");

  SecurityFeatures = (UINT8 *)(SecurityFeatures + Hsti->SecurityFeaturesSize);
  Print (L"  SecurityFeaturesImplemented - ");
  for (Index = 0; Index < Hsti->SecurityFeaturesSize; Index++) {
    Print (L"%02x ", SecurityFeatures[Index]);
  }
  Print (L"\n");

  SecurityFeatures = (UINT8 *)(SecurityFeatures + Hsti->SecurityFeaturesSize);
  Print (L"  SecurityFeaturesVerified    - ");
  for (Index = 0; Index < Hsti->SecurityFeaturesSize; Index++) {
    Print (L"%02x ", SecurityFeatures[Index]);
  }
  Print (L"\n");

  ErrorString = (CHAR16 *)(SecurityFeatures + Hsti->SecurityFeaturesSize);
  Print (L"  ErrorString                 - \"");
  CopyMem (&ErrorChar, ErrorString, sizeof(ErrorChar));
  for (; ErrorChar != 0;) {
    if (ErrorChar == L'\r') {
      Print (L"\\r");
    } else if (ErrorChar == L'\n') {
      Print (L"\\n");
    } else {
      Print (L"%c", ErrorChar);
    }
    ErrorString++;
    CopyMem (&ErrorChar, ErrorString, sizeof(ErrorChar));
  }
  Print (L"\"\n");
}

VOID
DumpHstiData (
  IN UINT32                   Role OPTIONAL,
  IN CHAR16                   *ImplementationID OPTIONAL
  )
{
  EFI_STATUS                        Status;
  EFI_ADAPTER_INFORMATION_PROTOCOL  *Aip;
  UINTN                             NoHandles;
  EFI_HANDLE                        *Handles;
  UINTN                             Index;
  EFI_GUID                          *InfoTypesBuffer;
  UINTN                             InfoTypesBufferCount;
  UINTN                             InfoTypesIndex;
  EFI_ADAPTER_INFORMATION_PROTOCOL  *AipCandidate;
  VOID                              *InformationBlock;
  UINTN                             InformationBlockSize;
  ADAPTER_INFO_PLATFORM_SECURITY    *Hsti;

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiAdapterInformationProtocolGuid,
                  NULL,
                  &NoHandles,
                  &Handles
                  );
  if (EFI_ERROR (Status)) {
    return ;
  }

  Hsti = NULL;
  Aip = NULL;
  InformationBlock = NULL;
  InformationBlockSize = 0;
  for (Index = 0; Index < NoHandles; Index++) {
    Status = gBS->HandleProtocol (
                    Handles[Index],
                    &gEfiAdapterInformationProtocolGuid,
                    (VOID **)&Aip
                    );
    if (EFI_ERROR (Status)) {
      continue;
    }

    //
    // Check AIP
    //
    Status = Aip->GetSupportedTypes (
                    Aip,
                    &InfoTypesBuffer,
                    &InfoTypesBufferCount
                    );
    if (EFI_ERROR (Status)) {
      continue;
    }

    AipCandidate = NULL;
    for (InfoTypesIndex = 0; InfoTypesIndex < InfoTypesBufferCount; InfoTypesIndex++) {
      if (CompareGuid (&InfoTypesBuffer[InfoTypesIndex], &gAdapterInfoPlatformSecurityGuid)) {
        AipCandidate = Aip;
        break;
      }
    }
    FreePool (InfoTypesBuffer);

    if (AipCandidate == NULL) {
      continue;
    }

    //
    // Check HSTI Role
    //
    Aip = AipCandidate;
    Status = Aip->GetInformation (
                    Aip,
                    &gAdapterInfoPlatformSecurityGuid,
                    &InformationBlock,
                    &InformationBlockSize
                    );
    if (EFI_ERROR (Status)) {
      continue;
    }

    Hsti = InformationBlock;

    if ((Role == 0) ||
        ((Hsti->Role == Role) && 
         ((ImplementationID == NULL) || (StrCmp (ImplementationID, Hsti->ImplementationID) == 0)))) {
      DumpHsti (Hsti);
    }
    FreePool (InformationBlock);
  }
  FreePool (Handles);

  if (Hsti == NULL) {
    Print(L"HSTI - %r\n", EFI_NOT_FOUND);
  }
}

VOID
DumpAcpiTableHeader (
  EFI_ACPI_DESCRIPTION_HEADER                    *Header
  )
{
  UINT8               *Signature;
  UINT8               *OemTableId;
  UINT8               *CreatorId;
  
  Print (L"  Table Header:\n");
  Signature = (UINT8*)&Header->Signature;
  Print (L"    Signature                         - '%c%c%c%c'\n", Signature[0], Signature[1], Signature[2], Signature[3]);
  Print (L"    Length                            - 0x%08x\n", Header->Length);
  Print (L"    Revision                          - 0x%02x\n", Header->Revision);
  Print (L"    Checksum                          - 0x%02x\n", Header->Checksum);
  Print (L"    OEMID                             - '%c%c%c%c%c%c'\n", Header->OemId[0], Header->OemId[1], Header->OemId[2], Header->OemId[3], Header->OemId[4], Header->OemId[5]);
  OemTableId = (UINT8 *)&Header->OemTableId;
  Print (L"    OEM Table ID                      - '%c%c%c%c%c%c%c%c'\n", OemTableId[0], OemTableId[1], OemTableId[2], OemTableId[3], OemTableId[4], OemTableId[5], OemTableId[6], OemTableId[7]);
  Print (L"    OEM Revision                      - 0x%08x\n", Header->OemRevision);
  CreatorId = (UINT8 *)&Header->CreatorId;
  Print (L"    Creator ID                        - '%c%c%c%c'\n", CreatorId[0], CreatorId[1], CreatorId[2], CreatorId[3]);
  Print (L"    Creator Revision                  - 0x%08x\n", Header->CreatorRevision);

  return;
}

VOID
DumpWsmt(
  IN EFI_ACPI_WSMT_TABLE *Wsmt
  )
{
  Print(L"WSMT\n");
  DumpAcpiTableHeader (&Wsmt->Header);
  Print(L"  ProtectionFlags                     - 0x%08x\n", Wsmt->ProtectionFlags);
  Print(L"    FIXED_COMM_BUFFERS                - 0x%08x\n", Wsmt->ProtectionFlags & EFI_WSMT_PROTECTION_FLAGS_FIXED_COMM_BUFFERS);
  Print(L"    COMM_BUFFER_NESTED_PTR_PROTECTION - 0x%08x\n", Wsmt->ProtectionFlags & EFI_WSMT_PROTECTION_FLAGS_COMM_BUFFER_NESTED_PTR_PROTECTION);
  Print(L"    SYSTEM_RESOURCE_PROTECTION        - 0x%08x\n", Wsmt->ProtectionFlags & EFI_WSMT_PROTECTION_FLAGS_SYSTEM_RESOURCE_PROTECTION);
}

/**

  This function scan ACPI table in RSDT.

  @param Rsdt      ACPI RSDT
  @param Signature ACPI table signature

  @return ACPI table

**/
VOID *
ScanTableInRSDT (
  IN EFI_ACPI_DESCRIPTION_HEADER    *Rsdt,
  IN UINT32                         Signature
  )
{
  UINTN                              Index;
  UINT32                             EntryCount;
  UINT32                             *EntryPtr;
  EFI_ACPI_DESCRIPTION_HEADER        *Table;

  if (Rsdt == NULL) {
    return NULL;
  }

  EntryCount = (Rsdt->Length - sizeof (EFI_ACPI_DESCRIPTION_HEADER)) / sizeof(UINT32);
  
  EntryPtr = (UINT32 *)(Rsdt + 1);
  for (Index = 0; Index < EntryCount; Index ++, EntryPtr ++) {
    Table = (EFI_ACPI_DESCRIPTION_HEADER *)((UINTN)(*EntryPtr));
    if (Table->Signature == Signature) {
      return Table;
    }
  }
  
  return NULL;
}

/**

  This function scan ACPI table in XSDT.

  @param Xsdt      ACPI XSDT
  @param Signature ACPI table signature

  @return ACPI table

**/
VOID *
ScanTableInXSDT (
  IN EFI_ACPI_DESCRIPTION_HEADER    *Xsdt,
  IN UINT32                         Signature
  )
{
  UINTN                          Index;
  UINT32                         EntryCount;
  UINT64                         EntryPtr;
  UINTN                          BasePtr;
  EFI_ACPI_DESCRIPTION_HEADER    *Table;

  if (Xsdt == NULL) {
    return NULL;
  }

  EntryCount = (Xsdt->Length - sizeof (EFI_ACPI_DESCRIPTION_HEADER)) / sizeof(UINT64);
  
  BasePtr = (UINTN)(Xsdt + 1);
  for (Index = 0; Index < EntryCount; Index ++) {
    CopyMem (&EntryPtr, (VOID *)(BasePtr + Index * sizeof(UINT64)), sizeof(UINT64));
    Table = (EFI_ACPI_DESCRIPTION_HEADER *)((UINTN)(EntryPtr));
    if (Table->Signature == Signature) {
      return Table;
    }
  }
  
  return NULL;
}

/**
  To find table in Acpi tables.

  @param AcpiTableGuid   The guid used to find ACPI table in UEFI ConfigurationTable.
  @param Signature       ACPI table signature
  
  @return  Acpi table pointer.
**/
VOID  *
FindAcpiTableByAcpiGuid (
  IN EFI_GUID  *AcpiTableGuid,
  IN UINT32    Signature
  )
{
  EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER  *Rsdp;
  EFI_ACPI_DESCRIPTION_HEADER                   *Rsdt;
  EFI_ACPI_DESCRIPTION_HEADER                   *Xsdt;
  VOID                                          *Table;
  UINTN                                         Index;

  Rsdp  = NULL;
  //
  // found ACPI table RSD_PTR from system table
  //
  for (Index = 0; Index < gST->NumberOfTableEntries; Index++) {
    if (CompareGuid (&(gST->ConfigurationTable[Index].VendorGuid), AcpiTableGuid)) {
      //
      // A match was found.
      //
      Rsdp = gST->ConfigurationTable[Index].VendorTable;
      break;
    }
  }

  if (Rsdp == NULL) {
    return NULL;
  }

  //
  // Search XSDT
  //
  if (Rsdp->Revision >= EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER_REVISION) {
    Xsdt = (EFI_ACPI_DESCRIPTION_HEADER *)(UINTN) Rsdp->XsdtAddress;
    Table = ScanTableInXSDT (Xsdt, Signature);
    if (Table != NULL) {
      return Table;
    }
  }

  //
  // Search RSDT
  //
  Rsdt = (EFI_ACPI_DESCRIPTION_HEADER *)(UINTN) Rsdp->RsdtAddress;
  Table = ScanTableInRSDT (Rsdt, Signature);
  if (Table != NULL) {
    return Table;
  }

  return NULL;
}

/**
  To find table in Acpi tables.
 
  @param Signature ACPI table signature

  @return  Acpi table pointer.
**/
VOID  *
FindAcpiTable (
  IN UINT32    Signature
  )
{
  VOID *Table;

  Table = FindAcpiTableByAcpiGuid (&gEfiAcpi20TableGuid, Signature);
  if (Table != NULL) {
    return Table;
  }

  return FindAcpiTableByAcpiGuid (&gEfiAcpi10TableGuid, Signature);
}

VOID
DumpWsmtData (
  VOID
  )
{
  VOID *Wsmt;

  Wsmt = FindAcpiTable (EFI_ACPI_WINDOWS_SMM_SECURITY_MITIGATION_TABLE_SIGNATURE);
  if (Wsmt == NULL) {
    Print(L"WSMT - %r\n", EFI_NOT_FOUND);
    return;
  }
  DumpWsmt (Wsmt);
}

EFI_STATUS
EFIAPI
HstiWsmtDumpEntrypoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  DumpHstiData (0, NULL);

  DumpWsmtData ();

  return EFI_SUCCESS;
}
