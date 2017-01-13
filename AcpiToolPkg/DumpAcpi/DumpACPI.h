/** @file

Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _DUMP_ACPI_H_
#define _DUMP_ACPI_H_

#include <Base.h>
#include <Uefi.h>
#include <IndustryStandard/Acpi.h>

#ifndef MAX_FILE_NAME_LEN
#define MAX_FILE_NAME_LEN  0x1000
#endif

extern BOOLEAN                     mIsAcpi20;
extern BOOLEAN                     mIsDumpData;
extern BOOLEAN                     mIsDumpVerb;
extern EFI_HANDLE                  mImageHandle;

#pragma pack(1)

typedef struct {
  EFI_ACPI_DESCRIPTION_HEADER    Header;
  UINT32                         Entry;
} RSDT_TABLE;

typedef struct {
  EFI_ACPI_DESCRIPTION_HEADER    Header;
  UINT64                         Entry;
} XSDT_TABLE;

#pragma pack()

VOID
ScanTableInRSDT (
  RSDT_TABLE            *Rsdt,
  UINT32                Signature,
  EFI_ACPI_DESCRIPTION_HEADER    **FoundTable,
  UINTN                 *Instance
  );

VOID
ScanTableInXSDT (
  XSDT_TABLE            *Xsdt,
  UINT32                Signature,
  EFI_ACPI_DESCRIPTION_HEADER    **FoundTable,
  UINTN                 *Instance
  );

VOID
ScanAcpiFADT (
  EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE                            *Fadt,
  EFI_ACPI_4_0_FIRMWARE_ACPI_CONTROL_STRUCTURE                            **Facs,
  EFI_ACPI_DESCRIPTION_HEADER                            **Dsdt
  );

VOID
DumpAcpiAllTable (
  VOID
  );

VOID
DumpAcpiTableWithSign (
  UINT32                                TableSign
  );

VOID
ScanAcpiRSDP (
  EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER                            **RsdpTable,
  BOOLEAN                               *Is20Table
  );
 
VOID
DumpAcpiRSDP (
  EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER                            *Rsdp
  );
   
VOID
ScanAcpiRSDT (
  EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER                            *Rsdp,
  RSDT_TABLE                            **Rsdt
  );
 
VOID
DumpAcpiRSDT (
  RSDT_TABLE                            *Rsdt
  );

VOID
ScanAcpiXSDT (
  EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER                            *Rsdp,
  XSDT_TABLE                            **Xsdt
  );

VOID
DumpAcpiXSDT (
  XSDT_TABLE                            *Xsdt
  );

VOID
DumpAcpiTables (
  EFI_ACPI_DESCRIPTION_HEADER                    *Table
  );

VOID
DumpSelectAcpiTable (
  EFI_ACPI_DESCRIPTION_HEADER                    *Table
  );

VOID
EFIAPI
DumpAcpiFADT (
  VOID  *Table
  );

VOID
EFIAPI
DumpAcpiFACS (
  VOID  *Table
  );

VOID
EFIAPI
DumpAcpiDSDT (
  VOID  *Table
  );

VOID
EFIAPI
DumpAcpiSSDT (
  VOID  *Table
  );

VOID
EFIAPI
DumpAcpiPSDT (
  VOID  *Table
  );

#endif
