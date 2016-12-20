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
DumpEinjInstructionEntry (
  EFI_ACPI_4_0_EINJ_INJECTION_INSTRUCTION_ENTRY                               *InstructionEntry
  )
{
  if (InstructionEntry == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       EINJ Instruction Entry                                            *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  EINJ Instruction Entry address ......................... 0x%016lx\n" :
    L"  EINJ Instruction Entry address ......................... 0x%08x\n",
    InstructionEntry
    );
  Print (
    L"    Injection Action ..................................... 0x%02x\n",
    InstructionEntry->InjectionAction
    );
  switch (InstructionEntry->InjectionAction) {
  case EFI_ACPI_4_0_EINJ_BEGIN_INJECTION_OPERATION:
    Print (
      L"      BEGIN_INJECTION_OPERATION\n"
      );
    break;
  case EFI_ACPI_4_0_EINJ_GET_TRIGGER_ERROR_ACTION_TABLE:
    Print (
      L"      GET_TRIGGER_ERROR_ACTION_TABLE\n"
      );
    break;
  case EFI_ACPI_4_0_EINJ_SET_ERROR_TYPE:
    Print (
      L"      SET_ERROR_TYPE\n"
      );
    break;
  case EFI_ACPI_4_0_EINJ_GET_ERROR_TYPE:
    Print (
      L"      GET_ERROR_TYPE\n"
      );
    break;
  case EFI_ACPI_4_0_EINJ_END_OPERATION:
    Print (
      L"      END_OPERATION\n"
      );
    break;
  case EFI_ACPI_4_0_EINJ_EXECUTE_OPERATION:
    Print (
      L"      EXECUTE_OPERATION\n"
      );
    break;
  case EFI_ACPI_4_0_EINJ_CHECK_BUSY_STATUS:
    Print (
      L"      CHECK_BUSY_STATUS\n"
      );
    break;
  case EFI_ACPI_4_0_EINJ_GET_COMMAND_STATUS:
    Print (
      L"      GET_COMMAND_STATUS\n"
      );
    break;
  case EFI_ACPI_4_0_EINJ_TRIGGER_ERROR:
    Print (
      L"      TRIGGER_ERROR\n"
      );
    break;
  default:
    break;
  }
  Print (
    L"    Instruction .......................................... 0x%02x\n",
    InstructionEntry->Instruction
    );
  switch (InstructionEntry->Instruction) {
  case EFI_ACPI_4_0_EINJ_READ_REGISTER:
    Print (
      L"      READ_REGISTER\n"
      );
    break;
  case EFI_ACPI_4_0_EINJ_READ_REGISTER_VALUE:
    Print (
      L"      READ_REGISTER_VALUE\n"
      );
    break;
  case EFI_ACPI_4_0_EINJ_WRITE_REGISTER:
    Print (
      L"      WRITE_REGISTER\n"
      );
    break;
  case EFI_ACPI_4_0_EINJ_WRITE_REGISTER_VALUE:
    Print (
      L"      WRITE_REGISTER_VALUE\n"
      );
    break;
  case EFI_ACPI_4_0_EINJ_NOOP:
    Print (
      L"      NOOP\n"
      );
    break;
  default:
    break;
  }
  Print (
    L"    Flags ................................................ 0x%02x\n",
    InstructionEntry->Flags
    );
  Print (
    L"      PRESERVE_REGISTER .................................. 0x%02x\n",
    InstructionEntry->Flags & EFI_ACPI_4_0_EINJ_PRESERVE_REGISTER
    );
  Print (
    L"    Register Region\n"
    );
  DumpAcpiGAddressStructure ((EFI_ACPI_3_0_GENERIC_ADDRESS_STRUCTURE *)&InstructionEntry->RegisterRegion);
  Print (
    L"    Value ................................................ 0x%016lx\n",
    InstructionEntry->Value
    );
  Print (
    L"    Mask ................................................. 0x%016lx\n",
    InstructionEntry->Mask
    );

  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
EFIAPI
DumpAcpiEINJ (
  VOID  *Table
  )
{
  EFI_ACPI_4_0_ERROR_INJECTION_TABLE_HEADER                            *Einj;
  EFI_ACPI_4_0_EINJ_INJECTION_INSTRUCTION_ENTRY                        *InstructionEntry;
  UINTN                                                                Index;

  Einj = Table;
  if (Einj == NULL) {
    return;
  }
  
  //
  // Dump Einj table
  //
  Print (
    L"*****************************************************************************\n"
    L"*         Error Record Serialization Table                                  *\n"
    L"*****************************************************************************\n"
    );
    
  if (GetAcpiDumpPropertyDumpData()) {
    DumpAcpiHex (Einj->Header.Length, Einj);
  }
  
  if (!GetAcpiDumpPropertyDumpVerb()) {
    goto Done;
  }

  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"EINJ address ............................................. 0x%016lx\n" :
    L"EINJ address ............................................. 0x%08x\n",
    Einj
    );
  
  DumpAcpiTableHeader(&(Einj->Header));
  
  Print (
    L"  Table Contents:\n"
    );
  Print (
    L"    Injection Header Size ................................ 0x%08x\n",
    Einj->InjectionHeaderSize
    );
  Print (
    L"    Injection Flags ...................................... 0x%02x\n",
    Einj->InjectionFlags
    );
  Print (
    L"    Injection Entry Count ................................ 0x%08x\n",
    Einj->InjectionEntryCount
    );
  
  InstructionEntry = (EFI_ACPI_4_0_EINJ_INJECTION_INSTRUCTION_ENTRY *)(Einj + 1);
  for (Index = 0; Index < Einj->InjectionEntryCount; Index++) {
    DumpEinjInstructionEntry (InstructionEntry);
    InstructionEntry ++;
  }

Done:
  Print (         
    L"*****************************************************************************\n\n"
    );
  
  return;
}

EFI_STATUS
EFIAPI
DumpAcpiEINJLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return RegisterDumpAcpiTable (EFI_ACPI_4_0_ERROR_INJECTION_TABLE_SIGNATURE, DumpAcpiEINJ);
}
