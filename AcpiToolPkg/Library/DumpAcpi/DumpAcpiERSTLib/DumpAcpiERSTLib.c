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
DumpErstInstructionEntry (
  EFI_ACPI_4_0_ERST_SERIALIZATION_INSTRUCTION_ENTRY                               *InstructionEntry
  )
{
  if (InstructionEntry == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       ERST Instruction Entry                                            *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  ERST Instruction Entry address ......................... 0x%016lx\n" :
    L"  ERST Instruction Entry address ......................... 0x%08x\n",
    InstructionEntry
    );
  Print (
    L"    Serialization Action ................................. 0x%02x\n",
    InstructionEntry->SerializationAction
    );
  switch (InstructionEntry->SerializationAction) {
  case EFI_ACPI_4_0_ERST_BEGIN_WRITE_OPERATION:
    Print (
      L"      BEGIN_WRITE_OPERATION\n"
      );
    break;
  case EFI_ACPI_4_0_ERST_BEGIN_READ_OPERATION:
    Print (
      L"      BEGIN_READ_OPERATION\n"
      );
    break;
  case EFI_ACPI_4_0_ERST_BEGIN_CLEAR_OPERATION:
    Print (
      L"      BEGIN_CLEAR_OPERATION\n"
      );
    break;
  case EFI_ACPI_4_0_ERST_END_OPERATION:
    Print (
      L"      END_OPERATION\n"
      );
    break;
  case EFI_ACPI_4_0_ERST_SET_RECORD_OFFSET:
    Print (
      L"      SET_RECORD_OFFSET\n"
      );
    break;
  case EFI_ACPI_4_0_ERST_EXECUTE_OPERATION:
    Print (
      L"      EXECUTE_OPERATION\n"
      );
    break;
  case EFI_ACPI_4_0_ERST_CHECK_BUSY_STATUS:
    Print (
      L"      CHECK_BUSY_STATUS\n"
      );
    break;
  case EFI_ACPI_4_0_ERST_GET_COMMAND_STATUS:
    Print (
      L"      GET_COMMAND_STATUS\n"
      );
    break;
  case EFI_ACPI_4_0_ERST_GET_RECORD_IDENTIFIER:
    Print (
      L"      GET_RECORD_IDENTIFIER\n"
      );
    break;
  case EFI_ACPI_4_0_ERST_SET_RECORD_IDENTIFIER:
    Print (
      L"      SET_RECORD_IDENTIFIER\n"
      );
    break;
  case EFI_ACPI_4_0_ERST_GET_RECORD_COUNT:
    Print (
      L"      GET_RECORD_COUNT\n"
      );
    break;
  case EFI_ACPI_4_0_ERST_BEGIN_DUMMY_WRITE_OPERATION:
    Print (
      L"      BEGIN_DUMMY_WRITE_OPERATION\n"
      );
    break;
  case EFI_ACPI_4_0_ERST_GET_ERROR_LOG_ADDRESS_RANGE:
    Print (
      L"      GET_ERROR_LOG_ADDRESS_RANGE\n"
      );
    break;
  case EFI_ACPI_4_0_ERST_GET_ERROR_LOG_ADDRESS_RANGE_LENGTH:
    Print (
      L"      GET_ERROR_LOG_ADDRESS_RANGE_LENGTH\n"
      );
    break;
  case EFI_ACPI_4_0_ERST_GET_ERROR_LOG_ADDRESS_RANGE_ATTRIBUTES:
    Print (
      L"      GET_ERROR_LOG_ADDRESS_RANGE_ATTRIBUTES\n"
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
  case EFI_ACPI_4_0_ERST_READ_REGISTER:
    Print (
      L"      READ_REGISTER\n"
      );
    break;
  case EFI_ACPI_4_0_ERST_READ_REGISTER_VALUE:
    Print (
      L"      READ_REGISTER_VALUE\n"
      );
    break;
  case EFI_ACPI_4_0_ERST_WRITE_REGISTER:
    Print (
      L"      WRITE_REGISTER\n"
      );
    break;
  case EFI_ACPI_4_0_ERST_WRITE_REGISTER_VALUE:
    Print (
      L"      WRITE_REGISTER_VALUE\n"
      );
    break;
  case EFI_ACPI_4_0_ERST_NOOP:
    Print (
      L"      NOOP\n"
      );
    break;
  case EFI_ACPI_4_0_ERST_LOAD_VAR1:
    Print (
      L"      LOAD_VAR1\n"
      );
    break;
  case EFI_ACPI_4_0_ERST_LOAD_VAR2:
    Print (
      L"      LOAD_VAR2\n"
      );
    break;
  case EFI_ACPI_4_0_ERST_STORE_VAR1:
    Print (
      L"      STORE_VAR1\n"
      );
    break;
  case EFI_ACPI_4_0_ERST_ADD:
    Print (
      L"      ADD\n"
      );
    break;
  case EFI_ACPI_4_0_ERST_SUBTRACT:
    Print (
      L"      SUBTRACT\n"
      );
    break;
  case EFI_ACPI_4_0_ERST_ADD_VALUE:
    Print (
      L"      ADD_VALUE\n"
      );
    break;
  case EFI_ACPI_4_0_ERST_SUBTRACT_VALUE:
    Print (
      L"      SUBTRACT_VALUE\n"
      );
    break;
  case EFI_ACPI_4_0_ERST_STALL:
    Print (
      L"      STALL\n"
      );
    break;
  case EFI_ACPI_4_0_ERST_STALL_WHILE_TRUE:
    Print (
      L"      STALL_WHILE_TRUE\n"
      );
    break;
  case EFI_ACPI_4_0_ERST_SKIP_NEXT_INSTRUCTION_IF_TRUE:
    Print (
      L"      SKIP_NEXT_INSTRUCTION_IF_TRUE\n"
      );
    break;
  case EFI_ACPI_4_0_ERST_GOTO:
    Print (
      L"      GOTO\n"
      );
    break;
  case EFI_ACPI_4_0_ERST_SET_SRC_ADDRESS_BASE:
    Print (
      L"      SET_SRC_ADDRESS_BASE\n"
      );
    break;
  case EFI_ACPI_4_0_ERST_SET_DST_ADDRESS_BASE:
    Print (
      L"      SET_DST_ADDRESS_BASE\n"
      );
    break;
  case EFI_ACPI_4_0_ERST_MOVE_DATA:
    Print (
      L"      MOVE_DATA\n"
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
    InstructionEntry->Flags & EFI_ACPI_4_0_ERST_PRESERVE_REGISTER
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
DumpAcpiERST (
  VOID  *Table
  )
{
  EFI_ACPI_4_0_ERROR_RECORD_SERIALIZATION_TABLE_HEADER                            *Erst;
  EFI_ACPI_4_0_ERST_SERIALIZATION_INSTRUCTION_ENTRY                               *InstructionEntry;
  UINTN                                                                           Index;

  Erst = Table;
  if (Erst == NULL) {
    return;
  }
  
  //
  // Dump Erst table
  //
  Print (
    L"*****************************************************************************\n"
    L"*         Error Record Serialization Table                                  *\n"
    L"*****************************************************************************\n"
    );
    
  if (GetAcpiDumpPropertyDumpData()) {
    DumpAcpiHex (Erst->Header.Length, Erst);
  }
  
  if (!GetAcpiDumpPropertyDumpVerb()) {
    goto Done;
  }

  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"ERST address ............................................. 0x%016lx\n" :
    L"ERST address ............................................. 0x%08x\n",
    Erst
    );
  
  DumpAcpiTableHeader(&(Erst->Header));
  
  Print (
    L"  Table Contents:\n"
    );
  Print (
    L"    Serialization Header Size ............................ 0x%08x\n",
    Erst->SerializationHeaderSize
    );
  Print (
    L"    Instruction Entry Count .............................. 0x%08x\n",
    Erst->InstructionEntryCount
    );
  
  InstructionEntry = (EFI_ACPI_4_0_ERST_SERIALIZATION_INSTRUCTION_ENTRY *)(Erst + 1);
  for (Index = 0; Index < Erst->InstructionEntryCount; Index++) {
    DumpErstInstructionEntry (InstructionEntry);
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
DumpAcpiERSTLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return RegisterDumpAcpiTable (EFI_ACPI_4_0_ERROR_RECORD_SERIALIZATION_TABLE_SIGNATURE, DumpAcpiERST);
}
