/** @file

Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Uefi.h>
#include <Library\BaseMemoryLib.h>
#include <Library\UefiLib.h>
#include <Library\DumpAcpiTableFuncLib.h>

typedef struct {
  UINT32                Signature;
  DUMP_ACPI_TABLE_FUNC  DumpAcpiTableFunc;
} DUMP_ACPI_TABLE_FUNC_STRUCT;

#define DUMP_ACPI_TABLE_FUNC_MAX_NUMBER 0x100

DUMP_ACPI_TABLE_FUNC_STRUCT  mDumpAcpiTableFuncStruct[DUMP_ACPI_TABLE_FUNC_MAX_NUMBER];
UINTN                        mDumpAcpiTableFuncStructNum;

BOOLEAN                     mInternalIsAcpiDumpData = FALSE;
BOOLEAN                     mInternalIsAcpiDumpVerb = TRUE;

/**
  Register DumpAcpiTable function.

  @param[in]  Signature               ACPI table signature
  @param[in]  DumpAcpiTableFunc       Dump ACPI table function

  @return Register status
**/
EFI_STATUS
EFIAPI
RegisterDumpAcpiTable (
  IN  UINT32                Signature,
  IN  DUMP_ACPI_TABLE_FUNC  DumpAcpiTableFunc
  )
{
  DUMP_ACPI_TABLE_FUNC  Func;

  Func = GetDumpAcpiTableFunc (Signature);
  if (Func != NULL) {
    return EFI_ALREADY_STARTED;
  }

  if (mDumpAcpiTableFuncStructNum >= DUMP_ACPI_TABLE_FUNC_MAX_NUMBER) {
    return EFI_OUT_OF_RESOURCES;
  }

  mDumpAcpiTableFuncStruct[mDumpAcpiTableFuncStructNum].Signature = Signature;
  mDumpAcpiTableFuncStruct[mDumpAcpiTableFuncStructNum].DumpAcpiTableFunc = DumpAcpiTableFunc;
  mDumpAcpiTableFuncStructNum ++;

  return EFI_SUCCESS;
}

/**
  Return DumpAcpiTable function.

  @param[in]  Signature               ACPI table signature

  @return DumpAcpiTable function
**/
DUMP_ACPI_TABLE_FUNC
EFIAPI
GetDumpAcpiTableFunc (
  IN  UINT32                Signature
  )
{
  UINTN  Index;

  for (Index = 0; Index < mDumpAcpiTableFuncStructNum; Index++) {
    if (mDumpAcpiTableFuncStruct[Index].Signature == Signature) {
      return mDumpAcpiTableFuncStruct[Index].DumpAcpiTableFunc;
    }
  }

  return NULL;
}

VOID
DumpAcpiTableHeader (
  EFI_ACPI_DESCRIPTION_HEADER                    *Header
  )
{
  UINT8               *Signature;
  UINT8               *OemTableId;
  UINT8               *CreatorId;
  
  if (Header == NULL) {
    return;
  }
  
  Print (
    L"  Table Header:\n"
    );
  Signature = (UINT8*)&Header->Signature;
  Print (
    L"    Signature ............................................ '%c%c%c%c'\n",
    Signature[0],
    Signature[1],
    Signature[2],
    Signature[3]
    );
  Print (
    L"    Length ............................................... 0x%08x\n",
    Header->Length
    );
  Print (
    L"    Revision ............................................. 0x%02x\n",
    Header->Revision
    );
  Print (
    L"    Checksum ............................................. 0x%02x\n",
    Header->Checksum
    );
  Print (
    L"    OEMID ................................................ '%c%c%c%c%c%c'\n",
    Header->OemId[0],
    Header->OemId[1],
    Header->OemId[2],
    Header->OemId[3],
    Header->OemId[4],
    Header->OemId[5]
    );
  OemTableId = (UINT8 *)&Header->OemTableId;
  Print (
    L"    OEM Table ID ......................................... '%c%c%c%c%c%c%c%c'\n",
    OemTableId[0],
    OemTableId[1],
    OemTableId[2],
    OemTableId[3],
    OemTableId[4],
    OemTableId[5],
    OemTableId[6],
    OemTableId[7]
    );
  Print (
    L"    OEM Revision ......................................... 0x%08x\n",
    Header->OemRevision
    );
  CreatorId = (UINT8 *)&Header->CreatorId;
  Print (
    L"    Creator ID ........................................... '%c%c%c%c'\n",
    CreatorId[0],
    CreatorId[1],
    CreatorId[2],
    CreatorId[3]
    );
  Print (
    L"    Creator Revision ..................................... 0x%08x\n",
    Header->CreatorRevision
    );

  return;
}

VOID
DumpAcpiGAddressStructure (
  EFI_ACPI_3_0_GENERIC_ADDRESS_STRUCTURE                    *GAddress
  )
{
  UINT64               Address;
  
  if (GAddress == NULL) {
    return;
  }
  
  Print (
    L"      Address Space ID ................................... 0x%02x\n",
    GAddress->AddressSpaceId
    );
  switch (GAddress->AddressSpaceId) {
  case EFI_ACPI_3_0_SYSTEM_MEMORY:
    Print (
      L"        System Memory\n"
      );
    break;
  case EFI_ACPI_3_0_SYSTEM_IO:
    Print (
      L"        System I/O\n"
      );
    break;
  case EFI_ACPI_3_0_PCI_CONFIGURATION_SPACE:
    Print (
      L"        PCI Configuration Space\n"
      );
    break;
  case EFI_ACPI_3_0_EMBEDDED_CONTROLLER:
    Print (
      L"        Embedded Controller\n"
      );
    break;
  case EFI_ACPI_3_0_SMBUS:
    Print (
      L"        SMBus\n"
      );
    break;
  case EFI_ACPI_3_0_FUNCTIONAL_FIXED_HARDWARE:
    Print (
      L"        Functional Fixed Hardware\n"
      );
    break;
  default:
    break;
  }
  Print (
    L"      Rigister Bit Width ................................. 0x%02x\n",
    GAddress->RegisterBitWidth
    );
  Print (
    L"      Rigister Bit Offset ................................ 0x%02x\n",
    GAddress->RegisterBitOffset
    );
  Print (
    L"      Access Size ........................................ 0x%02x\n",
    GAddress->AccessSize
    );
  switch (GAddress->AccessSize) {
  case EFI_ACPI_3_0_UNDEFINED:
    Print (
      L"        Undefined\n"
      );
    break;
  case EFI_ACPI_3_0_BYTE:
    Print (
      L"        Byte access\n"
      );
    break;
  case EFI_ACPI_3_0_WORD:
    Print (
      L"        Word access\n"
      );
    break;
  case EFI_ACPI_3_0_DWORD:
    Print (
      L"        Dword access\n"
      );
    break;
  case EFI_ACPI_3_0_QWORD:
    Print (
      L"        Qword access\n"
      );
    break;
  default:
    break;
  }

  CopyMem (&Address, &GAddress->Address, sizeof(UINT64));
  Print (
    L"      Address ............................................ 0x%016lx\n",
    Address
    );

  return;
}

VOID
DumpAcpiHex (
  IN UINTN BufferSize,
  IN VOID  *Buffer
  )
/*++

Routine Description:

  Dump hex data

Arguments:

  Buffer     - Buffer address
  BufferSize - Buffer size

Returns:

  None

--*/
{
  UINTN  Index;
  UINTN  IndexJ;
  UINTN  Left;
#define COL_SIZE  16

  Left = BufferSize % COL_SIZE;
  for (Index = 0; Index < BufferSize/COL_SIZE; Index++) {
    Print (L"      %04x: ", Index * COL_SIZE);
    for (IndexJ = 0; IndexJ < COL_SIZE; IndexJ++) {
      Print (L"%02x ", *((UINT8 *)Buffer + Index * COL_SIZE + IndexJ));
    }
    Print (L"\n");
  }
  if (Left != 0) {
    Print (L"      %04x: ", Index * COL_SIZE);
    for (IndexJ = 0; IndexJ < Left; IndexJ++) {
      Print (L"%02x ", *((UINT8 *)Buffer + Index * COL_SIZE + IndexJ));
    }
    Print (L"\n");
  }
}

VOID
SetAcpiDumpPropertyDumpData (
  IN BOOLEAN                     IsDumpData
  )
{
  mInternalIsAcpiDumpData = IsDumpData;
}

VOID
SetAcpiDumpPropertyDumpVerb (
  IN BOOLEAN                     IsDumpVerb
  )
{
  mInternalIsAcpiDumpVerb = IsDumpVerb;
}

BOOLEAN
GetAcpiDumpPropertyDumpData (
  VOID
  )
{
  return mInternalIsAcpiDumpData;
}

BOOLEAN
GetAcpiDumpPropertyDumpVerb (
  VOID
  )
{
  return mInternalIsAcpiDumpVerb;
}
