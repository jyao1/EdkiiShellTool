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

VOID
EFIAPI
DumpAcpiGTDT (
  VOID  *Table
  )
{
  EFI_ACPI_5_1_GENERIC_TIMER_DESCRIPTION_TABLE                            *Gtdt;

  Gtdt = Table;
  if (Gtdt == NULL) {
    return;
  }
  
  //
  // Dump Gtdt table
  //
  Print (
    L"*****************************************************************************\n"
    L"*         Generic Timer Description Table                                   *\n"
    L"*****************************************************************************\n"
    );
    
  if (GetAcpiDumpPropertyDumpData()) {
    DumpAcpiHex (Gtdt->Header.Length, Gtdt);
  }
  
  if (!GetAcpiDumpPropertyDumpVerb()) {
    goto Done;
  }

  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"GTDT address ............................................. 0x%016lx\n" :
    L"GTDT address ............................................. 0x%08x\n",
    Gtdt
    );
  
  DumpAcpiTableHeader(&(Gtdt->Header));
  
  Print (
    L"  Table Contents:\n"
    );
  if (Gtdt->Header.Length >= sizeof(EFI_ACPI_5_1_GENERIC_TIMER_DESCRIPTION_TABLE)) {
    Print (
      L"    Count Control Base Physical Address .................. 0x%016lx\n",
      Gtdt->CntControlBasePhysicalAddress
      );
  } else {
    Print (
      L"    Physical Address ..................................... 0x%016lx\n",
      Gtdt->CntControlBasePhysicalAddress
      );
    Print (
      L"    Global Flags ......................................... 0x%08x\n",
      ((EFI_ACPI_5_0_GENERIC_TIMER_DESCRIPTION_TABLE *)Gtdt)->GlobalFlags
      );
    Print (
      L"      Memory Mapped Block Present ........................ 0x%08x\n",
      ((EFI_ACPI_5_0_GENERIC_TIMER_DESCRIPTION_TABLE *)Gtdt)->GlobalFlags & EFI_ACPI_5_0_GTDT_GLOBAL_FLAG_MEMORY_MAPPED_BLOCK_PRESENT
      );
    Print (
      L"      Interrupt Mode ..................................... 0x%08x\n",
      ((EFI_ACPI_5_0_GENERIC_TIMER_DESCRIPTION_TABLE *)Gtdt)->GlobalFlags & EFI_ACPI_5_0_GTDT_GLOBAL_FLAG_INTERRUPT_MODE
      );
  }
  Print (
    L"    Secure PL1 Timer GSIV ................................ 0x%08x\n",
    Gtdt->SecurePL1TimerGSIV
    );
  Print (
    L"    Secure PL1 Timer Flags ............................... 0x%08x\n",
    Gtdt->SecurePL1TimerFlags
    );
  Print (
    L"      Timer Interrupt Mode ............................... 0x%08x\n",
    Gtdt->SecurePL1TimerFlags & EFI_ACPI_5_0_GTDT_TIMER_FLAG_TIMER_INTERRUPT_MODE
    );
  Print (
    L"      Timer Interrupt Polarity ........................... 0x%08x\n",
    Gtdt->SecurePL1TimerFlags & EFI_ACPI_5_0_GTDT_TIMER_FLAG_TIMER_INTERRUPT_POLARITY
    );
  Print (
    L"    Non-Secure PL1 Timer GSIV ............................ 0x%08x\n",
    Gtdt->NonSecurePL1TimerGSIV
    );
  Print (
    L"    Non-Secure PL1 Timer Flags ........................... 0x%08x\n",
    Gtdt->NonSecurePL1TimerFlags
    );
  Print (
    L"      Timer Interrupt Mode ............................... 0x%08x\n",
    Gtdt->NonSecurePL1TimerFlags & EFI_ACPI_5_0_GTDT_TIMER_FLAG_TIMER_INTERRUPT_MODE
    );
  Print (
    L"      Timer Interrupt Polarity ........................... 0x%08x\n",
    Gtdt->NonSecurePL1TimerFlags & EFI_ACPI_5_0_GTDT_TIMER_FLAG_TIMER_INTERRUPT_POLARITY
    );
  Print (
    L"    Virtual Timer GSIV ................................... 0x%08x\n",
    Gtdt->VirtualTimerGSIV
    );
  Print (
    L"    Virtual Timer Flags .................................. 0x%08x\n",
    Gtdt->VirtualTimerFlags
    );
  Print (
    L"      Timer Interrupt Mode ............................... 0x%08x\n",
    Gtdt->VirtualTimerFlags & EFI_ACPI_5_0_GTDT_TIMER_FLAG_TIMER_INTERRUPT_MODE
    );
  Print (
    L"      Timer Interrupt Polarity ........................... 0x%08x\n",
    Gtdt->VirtualTimerFlags & EFI_ACPI_5_0_GTDT_TIMER_FLAG_TIMER_INTERRUPT_POLARITY
    );
  Print (
    L"    Non-Secure PL2 Timer GSIV ............................ 0x%08x\n",
    Gtdt->NonSecurePL2TimerGSIV
    );
  Print (
    L"    Non-Secure PL2 Timer Flags ........................... 0x%08x\n",
    Gtdt->NonSecurePL2TimerFlags
    );
  Print (
    L"      Timer Interrupt Mode ............................... 0x%08x\n",
    Gtdt->NonSecurePL2TimerFlags & EFI_ACPI_5_0_GTDT_TIMER_FLAG_TIMER_INTERRUPT_MODE
    );
  Print (
    L"      Timer Interrupt Polarity ........................... 0x%08x\n",
    Gtdt->NonSecurePL2TimerFlags & EFI_ACPI_5_0_GTDT_TIMER_FLAG_TIMER_INTERRUPT_POLARITY
    );
  if (Gtdt->Header.Length >= sizeof(EFI_ACPI_5_1_GENERIC_TIMER_DESCRIPTION_TABLE)) {
    Print (
      L"    Count Read Base Physical Address .................... 0x%016lx\n",
      Gtdt->CntReadBasePhysicalAddress
      );
    Print (
      L"    Platform Timer Count ................................ 0x%08x\n",
      Gtdt->PlatformTimerCount
      );
    Print (
      L"    Platform Timer Offset ............................... 0x%08x\n",
      Gtdt->PlatformTimerOffset
      );
    // TBD -- Dump PlatformTimer table
  }

Done:
  Print (         
    L"*****************************************************************************\n\n"
    );
  
  return;
}

EFI_STATUS
EFIAPI
DumpAcpiGTDTLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return RegisterDumpAcpiTable (EFI_ACPI_5_0_GENERIC_TIMER_DESCRIPTION_TABLE_SIGNATURE, DumpAcpiGTDT);
}
