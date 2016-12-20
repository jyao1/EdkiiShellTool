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
#include <IndustryStandard/LowPowerIdleTable.h>

//
// LPIT structure
//
typedef struct {
  EFI_ACPI_DESCRIPTION_HEADER    Header;
//EFI_ACPI_LPI_DESCRIPTOR        Descriptor[];
} EFI_ACPI_LPIT_TABLE;

typedef struct {
  UINT32                                 Type;
  UINT32                                 Length;
} EFI_ACPI_LPI_DESCRIPTOR;

VOID
DumpNativeCStateLpiStateStructure (
  ACPI_LPI_NATIVE_CSTATE_DESCRIPTOR *NativeCStateLpiState
  )
{
  if (NativeCStateLpiState == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       Native C State LPI State Descriptor                               *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  NativeCStateLpiState address ........................... 0x%016lx\n" :
    L"  NativeCStateLpiState address ........................... 0x%08x\n",
    NativeCStateLpiState
    );
  Print (
    L"    Type ................................................. 0x%08x\n",
    NativeCStateLpiState->Type
    );
  Print (
    L"    Length ............................................... 0x%08x\n",
    NativeCStateLpiState->Length
    );
  Print (
    L"    Unique Id ............................................ 0x%04x\n",
    NativeCStateLpiState->UniqueId
    );
  Print (
    L"    Flags ................................................ 0x%08x\n",
    NativeCStateLpiState->Flags.Data32
    );
  Print (
    L"      Diabled ............................................ 0x%08x\n",
    NativeCStateLpiState->Flags.Bits.Disabled
    );
  Print (
    L"      Counter Unavailable ................................ 0x%08x\n",
    NativeCStateLpiState->Flags.Bits.CounterUnavailable << 1
    );
  Print (
    L"    Entry Trigger Register\n"
    );
  DumpAcpiGAddressStructure ((EFI_ACPI_3_0_GENERIC_ADDRESS_STRUCTURE *)&NativeCStateLpiState->EntryTrigger);
  Print (
    L"    Residency ............................................ 0x%08x\n",
    NativeCStateLpiState->Residency
    );
  Print (
    L"    Latency .............................................. 0x%08x\n",
    NativeCStateLpiState->Latency
    );
  Print (
    L"    Residency Counter Register\n"
    );
  DumpAcpiGAddressStructure ((EFI_ACPI_3_0_GENERIC_ADDRESS_STRUCTURE *)&NativeCStateLpiState->ResidencyCounter);
  Print (
    L"    Residency Counter Frequency .......................... 0x%016lx\n",
    NativeCStateLpiState->ResidencyCounterFrequency
    );
  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
EFIAPI
DumpAcpiLPIT (
  VOID  *Table
  )
{
  EFI_ACPI_LPIT_TABLE                            *Lpit;
  EFI_ACPI_LPI_DESCRIPTOR                        *LpiDesc;
  INTN                                           LpiLen;

  Lpit = Table;
  if (Lpit == NULL) {
    return;
  }
  
  //
  // Dump Lpit table
  //
  Print (
    L"*****************************************************************************\n"
    L"*         Low Power Idle Table                                              *\n"
    L"*****************************************************************************\n"
    );
    
  if (GetAcpiDumpPropertyDumpData()) {
    DumpAcpiHex (Lpit->Header.Length, Lpit);
  }
  
  if (!GetAcpiDumpPropertyDumpVerb()) {
    goto Done;
  }

  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"LPIT address ............................................. 0x%016lx\n" :
    L"LPIT address ............................................. 0x%08x\n",
    Lpit
    );
  
  DumpAcpiTableHeader(&(Lpit->Header));
  
  Print (
    L"  Table Contents:\n"
    );
  
  Print (
    L"\n"
    );
  
  LpiLen  = Lpit->Header.Length - sizeof(EFI_ACPI_LPIT_TABLE);
  LpiDesc = (EFI_ACPI_LPI_DESCRIPTOR *)(Lpit + 1);
  while (LpiLen > 0) {
    switch (LpiDesc->Type) {
    case ACPI_LPI_STRUCTURE_TYPE_NATIVE_CSTATE:
      DumpNativeCStateLpiStateStructure ((ACPI_LPI_NATIVE_CSTATE_DESCRIPTOR *)LpiDesc);
      break;
    default:
      break;
    }
    LpiDesc = (EFI_ACPI_LPI_DESCRIPTOR *)((UINTN)LpiDesc + LpiDesc->Length);
    LpiLen -= LpiDesc->Length;
  }

Done:
  Print (         
    L"*****************************************************************************\n\n"
    );
  
  return;
}

EFI_STATUS
EFIAPI
DumpAcpiLPITLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return RegisterDumpAcpiTable (EFI_ACPI_5_1_IO_LOW_POWER_IDLE_TABLE_STRUCTURE_SIGNATURE, DumpAcpiLPIT);
}

