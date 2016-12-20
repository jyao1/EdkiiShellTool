/** @file

Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _DUMP_ACPI_TABLE_FUNC_LIB_H_
#define _DUMP_ACPI_TABLE_FUNC_LIB_H_

#include <Uefi.h>
#include <IndustryStandard/Acpi.h>

typedef
VOID
(EFIAPI *DUMP_ACPI_TABLE_FUNC) (
  VOID                      *Table
  );

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
  );

/**
  Return DumpAcpiTable function.

  @param[in]  Signature               ACPI table signature

  @return DumpAcpiTable function
**/
DUMP_ACPI_TABLE_FUNC
EFIAPI
GetDumpAcpiTableFunc (
  IN  UINT32                Signature
  );

VOID
DumpAcpiTableHeader (
  EFI_ACPI_DESCRIPTION_HEADER                    *Header
  );

VOID
DumpAcpiGAddressStructure (
  EFI_ACPI_3_0_GENERIC_ADDRESS_STRUCTURE                    *GAddress
  );

VOID
DumpAcpiHex (
  IN UINTN BufferSize,
  IN VOID  *Buffer
  );

VOID
SetAcpiDumpPropertyDumpData (
  IN BOOLEAN                     IsDumpData
  );

VOID
SetAcpiDumpPropertyDumpVerb (
  IN BOOLEAN                     IsDumpVerb
  );

BOOLEAN
GetAcpiDumpPropertyDumpData (
  VOID
  );

BOOLEAN
GetAcpiDumpPropertyDumpVerb (
  VOID
  );

#endif
