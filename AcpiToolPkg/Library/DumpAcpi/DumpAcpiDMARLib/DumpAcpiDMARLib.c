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
#include <IndustryStandard/DMARemappingReportingTable.h>

VOID
DumpDmarDeviceScopeEntry (
  EFI_ACPI_DMAR_DEVICE_SCOPE_STRUCTURE_HEADER     *DmarDeviceScopeEntry
  )
{
  UINTN   PciPathNumber;
  UINTN   PciPathIndex;
  EFI_ACPI_DMAR_PCI_PATH  *PciPath;

  if (DmarDeviceScopeEntry == NULL) {
    return;
  }

  Print (         
    L"    *************************************************************************\n"
    L"    *       DMA-Remapping Device Scope Entry Structure                      *\n"
    L"    *************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"    DMAR Device Scope Entry address ...................... 0x%016lx\n" :
    L"    DMAR Device Scope Entry address ...................... 0x%08x\n",
    DmarDeviceScopeEntry
    );
  Print (
    L"      Device Scope Entry Type ............................ 0x%02x\n",
    DmarDeviceScopeEntry->Type
    );
  switch (DmarDeviceScopeEntry->Type) {
  case EFI_ACPI_DEVICE_SCOPE_ENTRY_TYPE_PCI_ENDPOINT:
    Print (
      L"        PCI Endpoint Device\n"
      );
    break;
  case EFI_ACPI_DEVICE_SCOPE_ENTRY_TYPE_PCI_BRIDGE:
    Print (
      L"        PCI Sub-hierachy\n"
      );
    break;
  case EFI_ACPI_DEVICE_SCOPE_ENTRY_TYPE_IOAPIC:
    Print (
      L"        IOAPIC\n"
      );
    break;
  case EFI_ACPI_DEVICE_SCOPE_ENTRY_TYPE_MSI_CAPABLE_HPET:
    Print (
      L"        MSI Capable HPET\n"
      );
    break;
  case EFI_ACPI_DEVICE_SCOPE_ENTRY_TYPE_ACPI_NAMESPACE_DEVICE:
    Print (
      L"        ACPI Namespace Device\n"
      );
    break;
  default:
    break;
  }
  Print (
    L"      Length ............................................. 0x%02x\n",
    DmarDeviceScopeEntry->Length
    );
  Print (
    L"      Enumeration ID ..................................... 0x%02x\n",
    DmarDeviceScopeEntry->EnumerationId
    );
  Print (
    L"      Starting Bus Number ................................ 0x%02x\n",
    DmarDeviceScopeEntry->StartBusNumber
    );

  PciPathNumber = (DmarDeviceScopeEntry->Length - sizeof(EFI_ACPI_DMAR_DEVICE_SCOPE_STRUCTURE_HEADER)) / sizeof(EFI_ACPI_DMAR_PCI_PATH);
  PciPath = (EFI_ACPI_DMAR_PCI_PATH *)(DmarDeviceScopeEntry + 1);
  for (PciPathIndex = 0; PciPathIndex < PciPathNumber; PciPathIndex++) {
    Print (
      L"      Device ............................................. 0x%02x\n",
      PciPath[PciPathIndex].Device
      );
    Print (
      L"      Function ........................................... 0x%02x\n",
      PciPath[PciPathIndex].Function
      );
  }
 
  Print (       
    L"    *************************************************************************\n\n"
    );

  return;
}

VOID
DumpDmarAndd (
  EFI_ACPI_DMAR_ANDD_HEADER *Andd
  )
{
  if (Andd == NULL) {
    return;
  }

  Print (
    L"  ***************************************************************************\n"
    L"  *       ACPI Name-space Device Declaration Structure                      *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  ANDD address ........................................... 0x%016lx\n" :
    L"  ANDD address ........................................... 0x%08x\n",
    Andd
    );
  Print (
    L"    Type ................................................. 0x%04x\n",
    Andd->Header.Type
    );
  Print (
    L"    Length ............................................... 0x%04x\n",
    Andd->Header.Length
    );
  Print (
    L"    ACPI Device Number ................................... 0x%02x\n",
    Andd->AcpiDeviceNumber
    );
  Print (
    L"    ACPI Object Name ..................................... '%a'\n",
    (Andd + 1)
    );

  Print (
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
DumpDmarRhsa (
  EFI_ACPI_DMAR_RHSA_HEADER *Rhsa
  )
{
  if (Rhsa == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       Remapping Hardware Status Affinity Structure                      *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  RHSA address ........................................... 0x%016lx\n" :
    L"  RHSA address ........................................... 0x%08x\n",
    Rhsa
    );
  Print (
    L"    Type ................................................. 0x%04x\n",
    Rhsa->Header.Type
    );
  Print (
    L"    Length ............................................... 0x%04x\n",
    Rhsa->Header.Length
    );
  Print (
    L"    Register Base Address ................................ 0x%016lx\n",
    Rhsa->RegisterBaseAddress
    );
  Print (
    L"    Proximity Domain ..................................... 0x%08x\n",
    Rhsa->ProximityDomain
    );

  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
DumpDmarAtsr (
  EFI_ACPI_DMAR_ATSR_HEADER *Atsr
  )
{
  EFI_ACPI_DMAR_DEVICE_SCOPE_STRUCTURE_HEADER       *DmarDeviceScopeEntry;
  INTN                                    AtsrLen;

  if (Atsr == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       Root Port ATS Capability Reporting Structure                      *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  ATSR address ........................................... 0x%016lx\n" :
    L"  ATSR address ........................................... 0x%08x\n",
    Atsr
    );
  Print (
    L"    Type ................................................. 0x%04x\n",
    Atsr->Header.Type
    );
  Print (
    L"    Length ............................................... 0x%04x\n",
    Atsr->Header.Length
    );
  Print (
    L"    Flags ................................................ 0x%02x\n",
    Atsr->Flags
    );
  Print (
    L"      ALL_PORTS .......................................... 0x%02x\n",
    Atsr->Flags & EFI_ACPI_DMAR_ATSR_FLAGS_ALL_PORTS
    );
  Print (
    L"    Segment Number ....................................... 0x%04x\n",
    Atsr->SegmentNumber
    );

  AtsrLen  = Atsr->Header.Length - sizeof(EFI_ACPI_DMAR_ATSR_HEADER);
  DmarDeviceScopeEntry = (EFI_ACPI_DMAR_DEVICE_SCOPE_STRUCTURE_HEADER *)(Atsr + 1);
  while (AtsrLen > 0) {
    DumpDmarDeviceScopeEntry (DmarDeviceScopeEntry);
    AtsrLen -= DmarDeviceScopeEntry->Length;
    DmarDeviceScopeEntry = (EFI_ACPI_DMAR_DEVICE_SCOPE_STRUCTURE_HEADER *)((UINTN)DmarDeviceScopeEntry + DmarDeviceScopeEntry->Length);
  }

  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
DumpDmarRmrr (
  EFI_ACPI_DMAR_RMRR_HEADER *Rmrr
  )
{
  EFI_ACPI_DMAR_DEVICE_SCOPE_STRUCTURE_HEADER       *DmarDeviceScopeEntry;
  INTN                                    RmrrLen;

  if (Rmrr == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       Reserved Memory Region Reporting Structure                        *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  RMRR address ........................................... 0x%016lx\n" :
    L"  RMRR address ........................................... 0x%08x\n",
    Rmrr
    );
  Print (
    L"    Type ................................................. 0x%04x\n",
    Rmrr->Header.Type
    );
  Print (
    L"    Length ............................................... 0x%04x\n",
    Rmrr->Header.Length
    );
  Print (
    L"    Segment Number ....................................... 0x%04x\n",
    Rmrr->SegmentNumber
    );
  Print (
    L"    Reserved Memory Region Base Address .................. 0x%016lx\n",
    Rmrr->ReservedMemoryRegionBaseAddress
    );
  Print (
    L"    Reserved Memory Region Limit Address ................. 0x%016lx\n",
    Rmrr->ReservedMemoryRegionLimitAddress
    );

  RmrrLen  = Rmrr->Header.Length - sizeof(EFI_ACPI_DMAR_RMRR_HEADER);
  DmarDeviceScopeEntry = (EFI_ACPI_DMAR_DEVICE_SCOPE_STRUCTURE_HEADER *)(Rmrr + 1);
  while (RmrrLen > 0) {
    DumpDmarDeviceScopeEntry (DmarDeviceScopeEntry);
    RmrrLen -= DmarDeviceScopeEntry->Length;
    DmarDeviceScopeEntry = (EFI_ACPI_DMAR_DEVICE_SCOPE_STRUCTURE_HEADER *)((UINTN)DmarDeviceScopeEntry + DmarDeviceScopeEntry->Length);
  }

  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
DumpDmarDrhd (
  EFI_ACPI_DMAR_DRHD_HEADER *Drhd
  )
{
  EFI_ACPI_DMAR_DEVICE_SCOPE_STRUCTURE_HEADER       *DmarDeviceScopeEntry;
  INTN                                    DrhdLen;

  if (Drhd == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       DMA-Remapping Hardware Definition Structure                       *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  DRHD address ........................................... 0x%016lx\n" :
    L"  DRHD address ........................................... 0x%08x\n",
    Drhd
    );
  Print (
    L"    Type ................................................. 0x%04x\n",
    Drhd->Header.Type
    );
  Print (
    L"    Length ............................................... 0x%04x\n",
    Drhd->Header.Length
    );
  Print (
    L"    Flags ................................................ 0x%02x\n",
    Drhd->Flags
    );
  Print (
    L"      INCLUDE_PCI_ALL .................................... 0x%02x\n",
    Drhd->Flags & EFI_ACPI_DMAR_DRHD_FLAGS_INCLUDE_PCI_ALL
    );
  Print (
    L"    Segment Number ....................................... 0x%04x\n",
    Drhd->SegmentNumber
    );
  Print (
    L"    Register Base Address ................................ 0x%016lx\n",
    Drhd->RegisterBaseAddress
    );

  DrhdLen  = Drhd->Header.Length - sizeof(EFI_ACPI_DMAR_DRHD_HEADER);
  DmarDeviceScopeEntry = (EFI_ACPI_DMAR_DEVICE_SCOPE_STRUCTURE_HEADER *)(Drhd + 1);
  while (DrhdLen > 0) {
    DumpDmarDeviceScopeEntry (DmarDeviceScopeEntry);
    DrhdLen -= DmarDeviceScopeEntry->Length;
    DmarDeviceScopeEntry = (EFI_ACPI_DMAR_DEVICE_SCOPE_STRUCTURE_HEADER *)((UINTN)DmarDeviceScopeEntry + DmarDeviceScopeEntry->Length);
  }

  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
EFIAPI
DumpAcpiDMAR (
  VOID  *Table
  )
{
  EFI_ACPI_DMAR_HEADER            *Dmar;
  EFI_ACPI_DMAR_STRUCTURE_HEADER *DmarHeader;
  INTN                  DmarLen;

  Dmar = Table;
  if (Dmar == NULL) {
    return;
  }
  
  //
  // Dump Dmar table
  //
  Print (
    L"*****************************************************************************\n"
    L"*         DMAR Table                                                        *\n"
    L"*****************************************************************************\n"
    );
    
  if (GetAcpiDumpPropertyDumpData()) {
    DumpAcpiHex (Dmar->Header.Length, Dmar);
  }
  
  if (!GetAcpiDumpPropertyDumpVerb()) {
    goto Done;
  }

  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"DMAR address ............................................. 0x%016lx\n" :
    L"DMAR address ............................................. 0x%08x\n",
    Dmar
    );
  
  DumpAcpiTableHeader(&(Dmar->Header));
  
  Print (
    L"  Table Contents:\n"
    );
  Print (
    L"    Host Address Width ................................... 0x%02x\n",
    Dmar->HostAddressWidth
    );
  Print (
    L"    Flags ................................................ 0x%02x\n",
    Dmar->Flags
    );
  Print (
    L"      INTR_REMAP ......................................... 0x%02x\n",
    Dmar->Flags & EFI_ACPI_DMAR_FLAGS_INTR_REMAP
    );
  Print (
    L"      X2APIC_OPT_OUT_SET ................................. 0x%02x\n",
    Dmar->Flags & EFI_ACPI_DMAR_FLAGS_X2APIC_OPT_OUT
    );

  DmarLen  = Dmar->Header.Length - sizeof(EFI_ACPI_DMAR_HEADER);
  DmarHeader = (EFI_ACPI_DMAR_STRUCTURE_HEADER *)(Dmar + 1);
  while (DmarLen > 0) {
    switch (DmarHeader->Type) {
    case EFI_ACPI_DMAR_TYPE_DRHD:
      DumpDmarDrhd ((EFI_ACPI_DMAR_DRHD_HEADER *)DmarHeader);
      break;
    case EFI_ACPI_DMAR_TYPE_RMRR:
      DumpDmarRmrr ((EFI_ACPI_DMAR_RMRR_HEADER *)DmarHeader);
      break;
    case EFI_ACPI_DMAR_TYPE_ATSR:
      DumpDmarAtsr ((EFI_ACPI_DMAR_ATSR_HEADER *)DmarHeader);
      break;
    case EFI_ACPI_DMAR_TYPE_RHSA:
      DumpDmarRhsa ((EFI_ACPI_DMAR_RHSA_HEADER *)DmarHeader);
      break;
    case EFI_ACPI_DMAR_TYPE_ANDD:
      DumpDmarAndd ((EFI_ACPI_DMAR_ANDD_HEADER *)DmarHeader);
      break;
    default:
      break;
    }
    DmarLen -= DmarHeader->Length;
    DmarHeader = (EFI_ACPI_DMAR_STRUCTURE_HEADER *)((UINTN)DmarHeader + DmarHeader->Length);
  }
 
Done:
  Print (         
    L"*****************************************************************************\n\n"
    );
  
  return;
}

EFI_STATUS
EFIAPI
DumpAcpiDMARLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return RegisterDumpAcpiTable (EFI_ACPI_4_0_DMA_REMAPPING_TABLE_SIGNATURE, DumpAcpiDMAR);
}
