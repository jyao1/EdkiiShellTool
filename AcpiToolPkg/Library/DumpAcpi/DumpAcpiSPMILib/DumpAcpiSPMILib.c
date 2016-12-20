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
#include <IndustryStandard/ServerProcessorManagementInterfaceTable.h>

VOID
EFIAPI
DumpAcpiSPMI (
  VOID  *Table
  )
{
  EFI_ACPI_SERVER_PROCESSOR_MANAGEMENT_INTERFACE_DESCRIPTION_TABLE                            *Spmi;

  Spmi = Table;
  if (Spmi == NULL) {
    return;
  }
  
  //
  // Dump Spmi table
  //
  Print (
    L"*****************************************************************************\n"
    L"*         Service Processor Management Interface Description Table          *\n"
    L"*****************************************************************************\n"
    );
    
  if (GetAcpiDumpPropertyDumpData()) {
    DumpAcpiHex (Spmi->Header.Length, Spmi);
  }
  
  if (!GetAcpiDumpPropertyDumpVerb()) {
    goto Done;
  }

  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"SPMI address ............................................. 0x%016lx\n" :
    L"SPMI address ............................................. 0x%08x\n",
    Spmi
    );
  
  DumpAcpiTableHeader(&(Spmi->Header));
  
  Print (
    L"  Table Contents:\n"
    );
  Print (
    L"    Interface Type ....................................... 0x%02x\n",
    Spmi->InterfaceType
    );
  switch (Spmi->InterfaceType) {
  case EFI_ACPI_SPMI_INTERFACE_TYPE_KCS:
    Print (
      L"      Keyboard Controller Style (KCS)\n"
      );
    break;
  case EFI_ACPI_SPMI_INTERFACE_TYPE_SMIC:
    Print (
      L"      Server Management Interface Chip (SMIC)\n"
      );
    break;
  case EFI_ACPI_SPMI_INTERFACE_TYPE_BT:
    Print (
      L"      Block Transfer (BT)\n"
      );
    break;
  case EFI_ACPI_SPMI_INTERFACE_TYPE_SSIF:
    Print (
      L"      SMBus System Interface (SSIF)\n"
      );
    break;
  default:
    break;    
  }
  Print (
    L"    Specification Revision ............................... 0x%04x\n",
    Spmi->SpecificationRevision
    );
  Print (
    L"    Interrupt Type ....................................... 0x%02x\n",
    Spmi->InterruptType
    );
  Print (
    L"      SCI triggered through GPE .......................... 0x%02x\n",
    Spmi->InterruptType & EFI_ACPI_SPMI_INTERRUPT_TYPE_SCI
    );
  Print (
    L"      I/O APIC/SAPIC interrupt (Global System Interrupt) . 0x%02x\n",
    Spmi->InterruptType & EFI_ACPI_SPMI_INTERRUPT_TYPE_IOAPIC
    );
  Print (
    L"    GPE .................................................. 0x%02x\n",
    Spmi->GPE
    );
  Print (
    L"    PCI Device Flag ...................................... 0x%02x\n",
    Spmi->PCIDeviceFlag
    );
  Print (
    L"      PCI Device Flag Set ................................ 0x%02x\n",
    Spmi->PCIDeviceFlag & EFI_ACPI_SPMI_PCI_DEVICE_FLAG
    );
  Print (
    L"    Global System Interrupt .............................. 0x%08x\n",
    Spmi->GlobalSystemInterrupt
    );

  Print (
    L"    Base Address\n"
    );
  DumpAcpiGAddressStructure ((EFI_ACPI_3_0_GENERIC_ADDRESS_STRUCTURE *)&(Spmi->BaseAddress));

  if (Spmi->PCIDeviceFlag & EFI_ACPI_SPMI_PCI_DEVICE_FLAG) {
    Print (
      L"    PCI Segment Group .................................... 0x%02x\n",
      Spmi->PCISegmentGroup_UID1
      );
    Print (
      L"    PCI Bus Bumber ....................................... 0x%02x\n",
      Spmi->PCIBusNumber_UID2
      );
    Print (
      L"    PCI Device Bumber .................................... 0x%02x\n",
      Spmi->PCIDeviceNumber_UID3 & 0xF
      );
    Print (
      L"    PCI Function Bumber .................................. 0x%02x\n",
      Spmi->PCIFunctionNumber_UID4 & 0x7
      );
    Print (
      L"      Interrupt Flag ..................................... 0x%02x\n",
      Spmi->PCIFunctionNumber_UID4 & 0x40
      );
  } else {
    Print (
      L"    UID byte 1 ........................................... 0x%02x\n",
      Spmi->PCISegmentGroup_UID1
      );
    Print (
      L"    UID byte 2 ........................................... 0x%02x\n",
      Spmi->PCIBusNumber_UID2
      );
    Print (
      L"    UID byte 3 ........................................... 0x%02x\n",
      Spmi->PCIDeviceNumber_UID3
      );
    Print (
      L"    UID byte 4 ........................................... 0x%02x\n",
      Spmi->PCIFunctionNumber_UID4
      );
  }

Done:
  Print (         
    L"*****************************************************************************\n\n"
    );
  
  return;
}

EFI_STATUS
EFIAPI
DumpAcpiSPMILibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return RegisterDumpAcpiTable (EFI_ACPI_2_0_SERVER_PLATFORM_MANAGEMENT_INTERFACE_SIGNATURE, DumpAcpiSPMI);
}
