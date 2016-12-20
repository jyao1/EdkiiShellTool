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
#include <IndustryStandard/DebugPort2Table.h>

VOID
DumpDbg2DeviceInfoStruct (
  EFI_ACPI_DBG2_DEBUG_DEVICE_INFORMATION_STRUCT *Dbg2DebugDeviceInfo
  )
{
  UINTN                                   Index;
  EFI_ACPI_3_0_GENERIC_ADDRESS_STRUCTURE  *GAddress;
  UINT32                                  *AddressSize;

  if (Dbg2DebugDeviceInfo == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       Dbg2 Device Information Structure                                 *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  Dbg2 Device Information address ........................ 0x%016lx\n" :
    L"  Dbg2 Device Information address ........................ 0x%08x\n",
    Dbg2DebugDeviceInfo
    );
  Print (
    L"    Revision ............................................. 0x%02x\n",
    Dbg2DebugDeviceInfo->Revision
    );
  Print (
    L"    Length ............................................... 0x%04x\n",
    Dbg2DebugDeviceInfo->Length
    );
  Print (
    L"    Number of Generic Address Registers .................. 0x%02x\n",
    Dbg2DebugDeviceInfo->NumberofGenericAddressRegisters
    );
  Print (
    L"    NameSpace String Length .............................. 0x%04x\n",
    Dbg2DebugDeviceInfo->NameSpaceStringLength
    );
  Print (
    L"    NameSpace String Offset .............................. 0x%04x\n",
    Dbg2DebugDeviceInfo->NameSpaceStringOffset
    );
  if (Dbg2DebugDeviceInfo->NameSpaceStringOffset != 0) {
    Print (
      L"      %a\n",
      (UINT8 *)Dbg2DebugDeviceInfo + Dbg2DebugDeviceInfo->NameSpaceStringOffset
      );
  }
  Print (
    L"    Oem Data Length ...................................... 0x%04x\n",
    Dbg2DebugDeviceInfo->OemDataLength
    );
  Print (
    L"    Oem Data Offset ...................................... 0x%04x\n",
    Dbg2DebugDeviceInfo->OemDataOffset
    );
  if (Dbg2DebugDeviceInfo->OemDataOffset != 0) {
    Print (
      L"    Oem Data:\n"
      );
    DumpAcpiHex (
      Dbg2DebugDeviceInfo->OemDataLength,
      (UINT8 *)Dbg2DebugDeviceInfo + Dbg2DebugDeviceInfo->OemDataOffset
      );
    Print (
      L"\n"
      );
  }
  Print (
    L"    Port Type ............................................ 0x%04x\n",
    Dbg2DebugDeviceInfo->PortType
    );
  switch (Dbg2DebugDeviceInfo->PortType) {
  case EFI_ACPI_DBG2_PORT_TYPE_SERIAL:
    Print (
      L"      Serial\n"
      );
    break;
  case EFI_ACPI_DBG2_PORT_TYPE_1394:
    Print (
      L"      1394\n"
      );
    break;
  case EFI_ACPI_DBG2_PORT_TYPE_USB:
    Print (
      L"      USB\n"
      );
    break;
  case EFI_ACPI_DBG2_PORT_TYPE_NET:
    Print (
      L"      NET\n"
      );
    break;
  default:
    break;
  }
  Print (
    L"    Port Subtype ......................................... 0x%04x\n",
    Dbg2DebugDeviceInfo->PortSubtype
    );
  switch (Dbg2DebugDeviceInfo->PortType) {
  case EFI_ACPI_DBG2_PORT_TYPE_SERIAL:
    switch (Dbg2DebugDeviceInfo->PortSubtype) {
    case EFI_ACPI_DBG2_PORT_SUBTYPE_SERIAL_FULL_16550:
      Print (
        L"      full 16550 interface\n"
        );
      break;
    case EFI_ACPI_DBG2_PORT_SUBTYPE_SERIAL_16550_SUBSET_COMPATIBLE_WITH_MS_DBGP_SPEC:
      Print (
        L"      16550 subset interface compatible with Microsoft Debug Port Specification\n"
        );
      break;
    default:
      break;
    }
    break;
  case EFI_ACPI_DBG2_PORT_TYPE_1394:
    switch (Dbg2DebugDeviceInfo->PortSubtype) {
    case EFI_ACPI_DBG2_PORT_SUBTYPE_1394_STANDARD:
      Print (
        L"      IEEE1394 Standard\n"
        );
      break;
    default:
      break;
    }
    break;
  case EFI_ACPI_DBG2_PORT_TYPE_USB:
    switch (Dbg2DebugDeviceInfo->PortSubtype) {
    case EFI_ACPI_DBG2_PORT_SUBTYPE_USB_XHCI:
      Print (
        L"      XHCI compatible\n"
        );
      break;
    case EFI_ACPI_DBG2_PORT_SUBTYPE_USB_EHCI:
      Print (
        L"      EHCI compatible\n"
        );
      break;
    default:
      break;
    }
    break;
  case EFI_ACPI_DBG2_PORT_TYPE_NET:
    Print (
      L"      Vendor ID: %04x\n",
	  Dbg2DebugDeviceInfo->PortSubtype
      );
    break;
  default:
    break;
  }
  Print (
    L"    Base Address Register Offset ......................... 0x%04x\n",
    Dbg2DebugDeviceInfo->BaseAddressRegisterOffset
    );
  GAddress = (EFI_ACPI_3_0_GENERIC_ADDRESS_STRUCTURE *)((UINTN)Dbg2DebugDeviceInfo + Dbg2DebugDeviceInfo->BaseAddressRegisterOffset);
  for (Index = 0; Index < Dbg2DebugDeviceInfo->NumberofGenericAddressRegisters; Index++) {
    Print (
      L"    Base Address Register(%d)\n",
      Index
      );
    DumpAcpiGAddressStructure (GAddress);
    GAddress ++;
  }
  Print (
    L"    Address Size Offset .................................. 0x%04x\n",
    Dbg2DebugDeviceInfo->AddressSizeOffset
    );
  AddressSize = (UINT32 *)((UINTN)Dbg2DebugDeviceInfo + Dbg2DebugDeviceInfo->AddressSizeOffset);
  for (Index = 0; Index < Dbg2DebugDeviceInfo->NumberofGenericAddressRegisters; Index++) {
    Print (
      L"    Address Size(%d): 0x%08x\n",
      Index,
      *AddressSize
      );
    AddressSize ++;
  }

  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
EFIAPI
DumpAcpiDBG2 (
  VOID  *Table
  )
{
  EFI_ACPI_DEBUG_PORT_2_DESCRIPTION_TABLE                          *Dbg2;
  EFI_ACPI_DBG2_DEBUG_DEVICE_INFORMATION_STRUCT                    *Dbg2DebugDeviceInfo;
  UINTN                                                            Index;

  Dbg2 = Table;
  if (Dbg2 == NULL) {
    return;
  }
  
  //
  // Dump Dbgp table
  //
  Print (
    L"*****************************************************************************\n"
    L"*         Debug Port 2 Table                                                *\n"
    L"*****************************************************************************\n"
    );
    
  if (GetAcpiDumpPropertyDumpData()) {
    DumpAcpiHex (Dbg2->Header.Length, Dbg2);
  }
  
  if (!GetAcpiDumpPropertyDumpVerb()) {
    goto Done;
  }

  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"DBG2 address ............................................. 0x%016lx\n" :
    L"DBG2 address ............................................. 0x%08x\n",
    Dbg2
    );
  
  DumpAcpiTableHeader(&(Dbg2->Header));
  
  Print (
    L"  Table Contents:\n"
    );
  Print (
    L"    Number Dbg Device Info ............................... 0x%08x\n",
    Dbg2->NumberDbgDeviceInfo
    );
  Print (
    L"    Offset Dbg Device Info ............................... 0x%08x\n",
    Dbg2->OffsetDbgDeviceInfo
    );
  Dbg2DebugDeviceInfo = (EFI_ACPI_DBG2_DEBUG_DEVICE_INFORMATION_STRUCT *)((UINTN)Table + Dbg2->OffsetDbgDeviceInfo);
  for (Index = 0; Index < Dbg2->NumberDbgDeviceInfo; Index++) {
    DumpDbg2DeviceInfoStruct (Dbg2DebugDeviceInfo);
    Dbg2DebugDeviceInfo = (EFI_ACPI_DBG2_DEBUG_DEVICE_INFORMATION_STRUCT *)((UINTN)Dbg2DebugDeviceInfo + Dbg2DebugDeviceInfo->Length);
  }

Done:
  Print (         
    L"*****************************************************************************\n\n"
    );
  
  return;
}

EFI_STATUS
EFIAPI
DumpAcpiDBG2LibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return RegisterDumpAcpiTable (EFI_ACPI_5_0_DEBUG_PORT_2_TABLE_SIGNATURE, DumpAcpiDBG2);
}
