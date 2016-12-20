/** @file

Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Base.h>
#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiLib.h>
#include <Library/DumpAcpiTableFuncLib.h>
#include <IndustryStandard/Acpi.h>
#include <IndustryStandard/IScsiBootFirmwareTable.h>

#pragma pack (1)

typedef struct {
  UINT16                                               NICOffset;
  UINT16                                               TargetOffset;
} IBFT_CONTROL_NIC_TARGET_STRUCT;

#pragma pack ()

VOID
DumpIbftIpAddress (
  EFI_IPv6_ADDRESS  *IpAddress
  )
{
  UINTN  Index;
  Print (L"%02x", IpAddress->Addr[0]);
  for (Index = 1; Index < sizeof(EFI_IPv6_ADDRESS); Index++) {
    Print (L":%02x", IpAddress->Addr[Index]);
  }
}

VOID
DumpIbftName (
  UINTN  Length,
  UINT8  *Name
  )
{
  UINTN  Index;
  for (Index = 0; Index < Length; Index++) {
    Print (L"%c", Name[Index]);
  }
}

VOID
DumpIbftByte (
  UINTN  Length,
  UINT8  *Byte
  )
{
  UINTN  Index;
  for (Index = 0; Index < Length; Index++) {
    Print (L"%2x", Byte[Index]);
  }
}

VOID
DumpIbftStructHeader (
  VOID                                                   *Ibft,
  EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_STRUCTURE_HEADER    *IbftStructure
  )
{
  if (IbftStructure == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       IBFT Structure                                                    *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  Ibft Structure address ................................. 0x%016lx\n" :
    L"  Ibft Structure address ................................. 0x%08x\n",
    IbftStructure
    );
  Print (
    L"    Structure Id ......................................... 0x%02x\n",
    IbftStructure->StructureId
    );
  Print (
    L"    Version .............................................. 0x%02x\n",
    IbftStructure->Version
    );
  Print (
    L"    Length ............................................... 0x%04x\n",
    IbftStructure->Length
    );
  Print (
    L"    Index ................................................ 0x%02x\n",
    IbftStructure->Index
    );
  Print (
    L"    Flags ................................................ 0x%02x\n",
    IbftStructure->Flags
    );
  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
DumpIbftControlStruct (
  VOID                                                    *Ibft,
  EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_CONTROL_STRUCTURE    *IbftStructure
  )
{
  UINTN                           Count;
  UINTN                           Index;
  IBFT_CONTROL_NIC_TARGET_STRUCT  *NicTarget;

  if (IbftStructure == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       IBFT Control Structure                                            *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  Ibft Structure address ................................. 0x%016lx\n" :
    L"  Ibft Structure address ................................. 0x%08x\n",
    IbftStructure
    );
  Print (
    L"    Structure Id ......................................... 0x%02x\n",
    IbftStructure->Header.StructureId
    );
  Print (
    L"    Version .............................................. 0x%02x\n",
    IbftStructure->Header.Version
    );
  Print (
    L"    Length ............................................... 0x%04x\n",
    IbftStructure->Header.Length
    );
  Print (
    L"    Index ................................................ 0x%02x\n",
    IbftStructure->Header.Index
    );
  Print (
    L"    Flags ................................................ 0x%02x\n",
    IbftStructure->Header.Flags
    );
  Print (
    L"      Boot Failover ...................................... 0x%02x\n",
    IbftStructure->Header.Flags & EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_CONTROL_STRUCTURE_FLAG_BOOT_FAILOVER
    );
  Print (
    L"    Extensions ........................................... 0x%04x\n",
    IbftStructure->Extensions
    );
  Print (
    L"    Initiator Offset ..................................... 0x%04x\n",
    IbftStructure->InitiatorOffset
    );

  NicTarget = (IBFT_CONTROL_NIC_TARGET_STRUCT *)&IbftStructure->NIC0Offset;
  Count = ((IbftStructure->Header.Length - sizeof(EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_CONTROL_STRUCTURE)) / sizeof(IBFT_CONTROL_NIC_TARGET_STRUCT)) + 2;
  for (Index = 0; Index < Count; Index++) {
    Print (
      L"    NIC%d Offset .......................................... 0x%04x\n",
      Index,
      NicTarget->NICOffset
      );
    Print (
      L"    Target%d Offset ....................................... 0x%04x\n",
      Index,
      NicTarget->TargetOffset
      );
    NicTarget ++;
  }
  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
DumpIbftInitiatorStruct (
  VOID                                                      *Ibft,
  EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_INITIATOR_STRUCTURE    *IbftStructure
  )
{
  if (IbftStructure == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       IBFT Initiator Structure                                          *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  Ibft Structure address ................................. 0x%016lx\n" :
    L"  Ibft Structure address ................................. 0x%08x\n",
    IbftStructure
    );
  Print (
    L"    Structure Id ......................................... 0x%02x\n",
    IbftStructure->Header.StructureId
    );
  Print (
    L"    Version .............................................. 0x%02x\n",
    IbftStructure->Header.Version
    );
  Print (
    L"    Length ............................................... 0x%04x\n",
    IbftStructure->Header.Length
    );
  Print (
    L"    Index ................................................ 0x%02x\n",
    IbftStructure->Header.Index
    );
  Print (
    L"    Flags ................................................ 0x%02x\n",
    IbftStructure->Header.Flags
    );
  Print (
    L"      Block Valid ........................................ 0x%02x\n",
    IbftStructure->Header.Flags & EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_INITIATOR_STRUCTURE_FLAG_BLOCK_VALID
    );
  Print (
    L"      Firmware Boot Selected ............................. 0x%02x\n",
    IbftStructure->Header.Flags & EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_INITIATOR_STRUCTURE_FLAG_BOOT_SELECTED
    );
  Print (
    L"    iSNS Server .......................................... "
    );
  DumpIbftIpAddress (&IbftStructure->ISnsServer);
  Print (L"\n");
  Print (
    L"    SLP Server ........................................... "
    );
  DumpIbftIpAddress (&IbftStructure->SlpServer);
  Print (L"\n");
  Print (
    L"    Primary Radius Server ................................ "
    );
  DumpIbftIpAddress (&IbftStructure->PrimaryRadiusServer);
  Print (L"\n");
  Print (
    L"    Secondary Radius Server .............................. "
    );
  DumpIbftIpAddress (&IbftStructure->SecondaryRadiusServer);
  Print (L"\n");
  Print (
    L"    Initiator Name Length ................................ 0x%04x\n",
    IbftStructure->IScsiNameLength
    );
  Print (
    L"    Initiator Name Offset ................................ 0x%04x\n",
    IbftStructure->IScsiNameOffset
    );
  Print (L"      ");
  DumpIbftName (IbftStructure->IScsiNameLength, (UINT8 *)((UINTN)Ibft + IbftStructure->IScsiNameOffset));
  Print (L"\n");

  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
DumpIbftNicStruct (
  VOID                                                *Ibft,
  EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_NIC_STRUCTURE    *IbftStructure
  )
{
  if (IbftStructure == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       IBFT NIC Structure                                                *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  Ibft Structure address ................................. 0x%016lx\n" :
    L"  Ibft Structure address ................................. 0x%08x\n",
    IbftStructure
    );
  Print (
    L"    Structure Id ......................................... 0x%02x\n",
    IbftStructure->Header.StructureId
    );
  Print (
    L"    Version .............................................. 0x%02x\n",
    IbftStructure->Header.Version
    );
  Print (
    L"    Length ............................................... 0x%04x\n",
    IbftStructure->Header.Length
    );
  Print (
    L"    Index ................................................ 0x%02x\n",
    IbftStructure->Header.Index
    );
  Print (
    L"    Flags ................................................ 0x%02x\n",
    IbftStructure->Header.Flags
    );
  Print (
    L"      Block Valid ........................................ 0x%02x\n",
    IbftStructure->Header.Flags & EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_NIC_STRUCTURE_FLAG_BLOCK_VALID
    );
  Print (
    L"      Firmware Boot Selected ............................. 0x%02x\n",
    IbftStructure->Header.Flags & EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_NIC_STRUCTURE_FLAG_BOOT_SELECTED
    );
  Print (
    L"      Global ............................................. 0x%02x\n",
    IbftStructure->Header.Flags & EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_NIC_STRUCTURE_FLAG_GLOBAL
    );
  Print (
    L"    IP Address ........................................... "
    );
  DumpIbftIpAddress (&IbftStructure->Ip);
  Print (L"\n");
  Print (
    L"    Subnet Mask Prefix ................................... 0x%02x\n",
    IbftStructure->SubnetMaskPrefixLength
    );
  Print (
    L"    Origin ............................................... 0x%02x\n",
    IbftStructure->Origin
    );
  Print (
    L"    Gateway .............................................. "
    );
  DumpIbftIpAddress (&IbftStructure->Gateway);
  Print (L"\n");
  Print (
    L"    Primary DNS .......................................... "
    );
  DumpIbftIpAddress (&IbftStructure->PrimaryDns);
  Print (L"\n");
  Print (
    L"    Secondary DNS ........................................ "
    );
  DumpIbftIpAddress (&IbftStructure->SecondaryDns);
  Print (L"\n");
  Print (
    L"    DHCP ................................................. "
    );
  DumpIbftIpAddress (&IbftStructure->DhcpServer);
  Print (L"\n");
  Print (
    L"    VLAN ................................................. 0x%04x\n",
    IbftStructure->VLanTag
    );
  Print (
    L"    MAC Address .......................................... %02x:%02x:%02x:%02x:%02x:%02x\n",
    IbftStructure->Mac[0],
    IbftStructure->Mac[1],
    IbftStructure->Mac[2],
    IbftStructure->Mac[3],
    IbftStructure->Mac[4],
    IbftStructure->Mac[5]
    );
  Print (
    L"    PCI Device ........................................... %02x:%2x:%2x\n",
    IbftStructure->PciLocation & 0xFF,
    (IbftStructure->PciLocation > 8) & 0x1F,
    (IbftStructure->PciLocation > 13) & 0x7
    );
  Print (
    L"    Host Name Length ..................................... 0x%04x\n",
    IbftStructure->HostNameLength
    );
  Print (
    L"    Host Name Offset ..................................... 0x%04x\n",
    IbftStructure->HostNameOffset
    );
  Print (L"      ");
  DumpIbftName (IbftStructure->HostNameLength, (UINT8 *)((UINTN)Ibft + IbftStructure->HostNameOffset));
  Print (L"\n");

  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
DumpIbftTargetStruct (
  VOID                                                   *Ibft,
  EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_TARGET_STRUCTURE    *IbftStructure
  )
{
  if (IbftStructure == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       IBFT NIC Structure                                                *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  Ibft Structure address ................................. 0x%016lx\n" :
    L"  Ibft Structure address ................................. 0x%08x\n",
    IbftStructure
    );
  Print (
    L"    Structure Id ......................................... 0x%02x\n",
    IbftStructure->Header.StructureId
    );
  Print (
    L"    Version .............................................. 0x%02x\n",
    IbftStructure->Header.Version
    );
  Print (
    L"    Length ............................................... 0x%04x\n",
    IbftStructure->Header.Length
    );
  Print (
    L"    Index ................................................ 0x%02x\n",
    IbftStructure->Header.Index
    );
  Print (
    L"    Flags ................................................ 0x%02x\n",
    IbftStructure->Header.Flags
    );
  Print (
    L"      Block Valid ........................................ 0x%02x\n",
    IbftStructure->Header.Flags & EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_TARGET_STRUCTURE_FLAG_BLOCK_VALID
    );
  Print (
    L"      Firmware Boot Selected ............................. 0x%02x\n",
    IbftStructure->Header.Flags & EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_TARGET_STRUCTURE_FLAG_BOOT_SELECTED
    );
  Print (
    L"      Use Radius CHAP .................................... 0x%02x\n",
    IbftStructure->Header.Flags & EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_TARGET_STRUCTURE_FLAG_RADIUS_CHAP
    );
  Print (
    L"      Use Radius rCHAP ................................... 0x%02x\n",
    IbftStructure->Header.Flags & EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_TARGET_STRUCTURE_FLAG_RADIUS_RCHAP
    );
  Print (
    L"    Target IP Address .................................... "
    );
  DumpIbftIpAddress (&IbftStructure->Ip);
  Print (L"\n");
  Print (
    L"    Target IP Socket ..................................... 0x%04x\n",
    IbftStructure->Port
    );
  Print (
    L"    Target Boot LUN ...................................... %02x%02x%02x%02x%02x%02x%02x%02x\n",
    IbftStructure->BootLun[0],
    IbftStructure->BootLun[1],
    IbftStructure->BootLun[2],
    IbftStructure->BootLun[3],
    IbftStructure->BootLun[4],
    IbftStructure->BootLun[5],
    IbftStructure->BootLun[6],
    IbftStructure->BootLun[7]
    );
  Print (
    L"    CHAP Type ............................................ 0x%02x\n",
    IbftStructure->CHAPType
    );
  switch (IbftStructure->CHAPType) {
  case EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_TARGET_STRUCTURE_CHAP_TYPE_NO_CHAP:
    Print (
      L"      No CHAP\n"
      );
    break;
  case EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_TARGET_STRUCTURE_CHAP_TYPE_CHAP:
    Print (
      L"      CHAP\n"
      );
    break;
  case EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_TARGET_STRUCTURE_CHAP_TYPE_MUTUAL_CHAP:
    Print (
      L"      Mutual CHAP\n"
      );
    break;
  default:
    break;
  }
  Print (
    L"    NIC Association ...................................... 0x%02x\n",
    IbftStructure->NicIndex
    );
  Print (
    L"    Target Name Length ................................... 0x%04x\n",
    IbftStructure->IScsiNameLength
    );
  Print (
    L"    Target Name Offset ................................... 0x%04x\n",
    IbftStructure->IScsiNameOffset
    );
  Print (L"      ");
  DumpIbftName (IbftStructure->IScsiNameLength, (UINT8 *)((UINTN)Ibft + IbftStructure->IScsiNameOffset));
  Print (L"\n");
  Print (
    L"    CHAP Name Length ..................................... 0x%04x\n",
    IbftStructure->CHAPNameLength
    );
  Print (
    L"    CHAP Name Offset ..................................... 0x%04x\n",
    IbftStructure->CHAPNameOffset
    );
  Print (L"      ");
  DumpIbftName (IbftStructure->CHAPNameLength, (UINT8 *)((UINTN)Ibft + IbftStructure->CHAPNameOffset));
  Print (L"\n");
  Print (
    L"    CHAP Secret Length ................................... 0x%04x\n",
    IbftStructure->CHAPSecretLength
    );
  Print (
    L"    CHAP Secret Offset ................................... 0x%04x\n",
    IbftStructure->CHAPSecretOffset
    );
  Print (L"      ");
  DumpIbftByte (IbftStructure->CHAPSecretLength, (UINT8 *)((UINTN)Ibft + IbftStructure->CHAPSecretOffset));
  Print (L"\n");
  Print (
    L"    Reverse CHAP Name Length ............................. 0x%04x\n",
    IbftStructure->ReverseCHAPNameLength
    );
  Print (
    L"    Reverse CHAP Name Offset ............................. 0x%04x\n",
    IbftStructure->ReverseCHAPNameOffset
    );
  Print (L"      ");
  DumpIbftName (IbftStructure->ReverseCHAPNameLength, (UINT8 *)((UINTN)Ibft + IbftStructure->ReverseCHAPNameOffset));
  Print (L"\n");
  Print (
    L"    Reverse CHAP Secret Length ........................... 0x%04x\n",
    IbftStructure->ReverseCHAPSecretLength
    );
  Print (
    L"    Reverse CHAP Secret Offset ........................... 0x%04x\n",
    IbftStructure->ReverseCHAPSecretOffset
    );
  Print (L"      ");
  DumpIbftName (IbftStructure->ReverseCHAPSecretLength, (UINT8 *)((UINTN)Ibft + IbftStructure->ReverseCHAPSecretOffset));
  Print (L"\n");

  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
DumpIbftExtensionsStruct (
  VOID                                                   *Ibft,
  EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_STRUCTURE_HEADER    *IbftStructure
  )
{
  if (IbftStructure == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       IBFT Extensions Structure                                         *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  Ibft Structure address ................................. 0x%016lx\n" :
    L"  Ibft Structure address ................................. 0x%08x\n",
    IbftStructure
    );
  Print (
    L"    Structure Id ......................................... 0x%02x\n",
    IbftStructure->StructureId
    );
  Print (
    L"    Version .............................................. 0x%02x\n",
    IbftStructure->Version
    );
  Print (
    L"    Length ............................................... 0x%04x\n",
    IbftStructure->Length
    );
  Print (
    L"    Index ................................................ 0x%02x\n",
    IbftStructure->Index
    );
  Print (
    L"    Flags ................................................ 0x%02x\n",
    IbftStructure->Flags
    );
  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
EFIAPI
DumpAcpiIBFT (
  VOID  *Table
  )
{
  EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_HEADER              *Ibft;
  EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_CONTROL_STRUCTURE   *IbftStructure;
  UINTN                                                  Count;
  UINTN                                                  Index;
  IBFT_CONTROL_NIC_TARGET_STRUCT                         *NicTarget;

  Ibft = Table;
  if (Ibft == NULL) {
    return;
  }
  
  //
  // Dump Ibft table
  //
  Print (
    L"*****************************************************************************\n"
    L"*         ISCSI Boot Firmware Table                                         *\n"
    L"*****************************************************************************\n"
    );
    
  if (GetAcpiDumpPropertyDumpData()) {
    DumpAcpiHex (Ibft->Length, Ibft);
  }
  
  if (!GetAcpiDumpPropertyDumpVerb()) {
    goto Done;
  }

  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"IBFT address ............................................. 0x%016lx\n" :
    L"IBFT address ............................................. 0x%08x\n",
    Ibft
    );
  
  DumpAcpiTableHeader((EFI_ACPI_DESCRIPTION_HEADER *)Ibft);
  
  Print (
    L"  Table Contents:\n"
    );

  IbftStructure = (EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_CONTROL_STRUCTURE *)(Ibft + 1);

  DumpIbftControlStruct (Table, (EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_CONTROL_STRUCTURE *)IbftStructure);

  if (IbftStructure->InitiatorOffset != 0) {
    DumpIbftInitiatorStruct (Table, (EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_INITIATOR_STRUCTURE *)((UINTN)Table + IbftStructure->InitiatorOffset));
  }

  NicTarget = (IBFT_CONTROL_NIC_TARGET_STRUCT *)&IbftStructure->NIC0Offset;
  Count = ((IbftStructure->Header.Length - sizeof(EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_CONTROL_STRUCTURE)) / sizeof(IBFT_CONTROL_NIC_TARGET_STRUCT)) + 2;
  for (Index = 0; Index < Count; Index++) {
    if (NicTarget->NICOffset != 0) {
      DumpIbftNicStruct (Table, (EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_NIC_STRUCTURE *)((UINTN)Table + NicTarget->NICOffset));
    }
    if (NicTarget->TargetOffset != 0) {
      DumpIbftTargetStruct (Table, (EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_TARGET_STRUCTURE *)((UINTN)Table + NicTarget->TargetOffset));
    }
    NicTarget ++;
  }

  if (IbftStructure->Extensions != 0) {
    DumpIbftExtensionsStruct (Table, (EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_STRUCTURE_HEADER *)((UINTN)Table + IbftStructure->Extensions));
  }

Done:
  Print (         
    L"*****************************************************************************\n\n"
    );
  
  return;
}

EFI_STATUS
EFIAPI
DumpAcpiIBFTLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return RegisterDumpAcpiTable (EFI_ACPI_3_0_ISCSI_BOOT_FIRMWARE_TABLE_SIGNATURE, DumpAcpiIBFT);
}
