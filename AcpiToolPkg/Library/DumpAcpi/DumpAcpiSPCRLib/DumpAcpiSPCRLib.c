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
#include <IndustryStandard/SerialPortConsoleRedirectionTable.h>

VOID
EFIAPI
DumpAcpiSPCR (
  VOID  *Table
  )
{
  EFI_ACPI_SERIAL_PORT_CONSOLE_REDIRECTION_TABLE                            *Spcr;

  Spcr = Table;
  if (Spcr == NULL) {
    return;
  }
  
  //
  // Dump Spcr table
  //
  Print (
    L"*****************************************************************************\n"
    L"*         Serial Port Console Redirection Table                             *\n"
    L"*****************************************************************************\n"
    );
    
  if (GetAcpiDumpPropertyDumpData()) {
    DumpAcpiHex (Spcr->Header.Length, Spcr);
  }
  
  if (!GetAcpiDumpPropertyDumpVerb()) {
    goto Done;
  }

  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"SPCR address ............................................. 0x%016lx\n" :
    L"SPCR address ............................................. 0x%08x\n",
    Spcr
    );
  
  DumpAcpiTableHeader(&(Spcr->Header));
  
  Print (
    L"  Table Contents:\n"
    );
  Print (
    L"    Interface Type ....................................... 0x%02x\n",
    Spcr->InterfaceType
    );
  switch (Spcr->InterfaceType) {
  case 0:
    Print (
      L"      full 16550 interface\n"
      );
    break;
  case 1:
    Print (
      L"      full 16450 interface (must also accept writing to the 16550 FCR register)\n"
      );
    break;
  default:
    break;    
  }
  
  Print (
    L"    Base Address\n"
    );
  DumpAcpiGAddressStructure ((EFI_ACPI_3_0_GENERIC_ADDRESS_STRUCTURE *)&(Spcr->BaseAddress));

  Print (
    L"    Interrupt Type ....................................... 0x%02x\n",
    Spcr->InterruptType
    );
  Print (
    L"      PC-AT-compatible dual-8259 IRQ interrupt ........... 0x%02x\n",
    Spcr->InterruptType & 0x1
    );
  Print (
    L"      I/O APIC interrupt (Global System Interrupt) ....... 0x%02x\n",
    Spcr->InterruptType & 0x2
    );
  Print (
    L"      I/O SAPIC interrupt (Global System Interrupt) ...... 0x%02x\n",
    Spcr->InterruptType & 0x4
    );
  Print (
    L"    IRQ .................................................. 0x%02x\n",
    Spcr->Irq
    );
  Print (
    L"    Global System Interrupt .............................. 0x%08x\n",
    Spcr->GlobalSystemInterrupt
    );
  Print (
    L"    Baud Rate ............................................ 0x%02x\n",
    Spcr->BaudRate
    );
  switch (Spcr->BaudRate) {
  case 3:
    Print (
      L"      9600\n"
      );
    break;
  case 4:
    Print (
      L"      19200\n"
      );
    break;
  case 6:
    Print (
      L"      57600\n"
      );
    break;
  case 7:
    Print (
      L"      115200\n"
      );
    break;
  default:
    break;
  }
  Print (
    L"    Parity ............................................... 0x%02x\n",
    Spcr->Parity
    );
  switch (Spcr->Parity) {
  case 0:
    Print (
      L"      No parity\n"
      );
    break;
  default:
    break;
  }
  Print (
    L"    Stop Bits ............................................ 0x%02x\n",
    Spcr->StopBits
    );
  switch (Spcr->StopBits) {
  case 1:
    Print (
      L"      1 Stop bit\n"
      );
    break;
  default:
    break;
  }
  Print (
    L"    Flow Control ......................................... 0x%02x\n",
    Spcr->FlowControl
    );
  Print (
    L"      DCD required for transmit  ......................... 0x%02x\n",
    Spcr->FlowControl & 0x1
    );
  Print (
    L"      RTS/CTS hardware flow control ...................... 0x%02x\n",
    Spcr->FlowControl & 0x2
    );
  Print (
    L"      XON/XOFF software control .......................... 0x%02x\n",
    Spcr->FlowControl & 0x4
    );
  Print (
    L"    Terminal Type ........................................ 0x%02x\n",
    Spcr->TerminalType
    );
  switch (Spcr->TerminalType) {
  case 0:
    Print (
      L"      VT100\n"
      );
    break;
  case 1:
    Print (
      L"      VT100+\n"
      );
    break;
  case 2:
    Print (
      L"      VT-UTF8\n"
      );
    break;
  case 3:
    Print (
      L"      ANSI\n"
      );
    break;
  default:
    break;
  }
  Print (
    L"    PCI Device ID ........................................ 0x%04x\n",
    Spcr->PciDeviceId
    ); 
  Print (
    L"    PCI Vendor ID ........................................ 0x%04x\n",
    Spcr->PciVendorId
    ); 
  Print (
    L"    PCI Bus Bumber ....................................... 0x%02x\n",
    Spcr->PciBusNumber
    );
  Print (
    L"    PCI Device Bumber .................................... 0x%02x\n",
    Spcr->PciDeviceNumber
    );
  Print (
    L"    PCI Function Bumber .................................. 0x%02x\n",
    Spcr->PciFunctionNumber
    );
  Print (
    L"    PCI Flags ............................................ 0x%08x\n",
    Spcr->PciFlags
    );
  Print (
    L"      OS NOT suppress PNP device enumeration ............. 0x%08x\n",
    Spcr->PciFlags & 0x1
    );
  Print (
    L"    PCI Segment .......................................... 0x%02x\n",
    Spcr->PciSegment
    );

Done:
  Print (         
    L"*****************************************************************************\n\n"
    );
  
  return;
}

EFI_STATUS
EFIAPI
DumpAcpiSPCRLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return RegisterDumpAcpiTable (EFI_ACPI_2_0_SERIAL_PORT_CONSOLE_REDIRECTION_TABLE_SIGNATURE, DumpAcpiSPCR);
}
