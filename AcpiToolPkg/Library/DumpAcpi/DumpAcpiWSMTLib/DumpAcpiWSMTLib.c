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
#include <IndustryStandard/WindowsSmmSecurityMitigationTable.h>

VOID
EFIAPI
DumpAcpiWSMT (
  VOID  *Table
  )
{
  EFI_ACPI_WSMT_TABLE                            *Wsmt;

  Wsmt = Table;
  if (Wsmt == NULL) {
    return;
  }
  
  //
  // Dump Wsmt table
  //
  Print (
    L"*****************************************************************************\n"
    L"*         Windows SMM Security Mitigation Table                             *\n"
    L"*****************************************************************************\n"
    );
    
  if (GetAcpiDumpPropertyDumpData()) {
    DumpAcpiHex (Wsmt->Header.Length, Wsmt);
  }
  
  if (!GetAcpiDumpPropertyDumpVerb()) {
    goto Done;
  }

  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"WSMT address ............................................. 0x%016lx\n" :
    L"WSMT address ............................................. 0x%08x\n",
    Wsmt
    );
  
  DumpAcpiTableHeader(&(Wsmt->Header));
  
  Print (
    L"  Table Contents:\n"
    );
  Print (
    L"    Protection Flags ..................................... 0x%08x\n",
    Wsmt->ProtectionFlags
    );
  Print(
    L"      FIXED_COMM_BUFFERS ................................. 0x%08x\n",
    Wsmt->ProtectionFlags & EFI_WSMT_PROTECTION_FLAGS_FIXED_COMM_BUFFERS
    );
  Print(
    L"      COMM_BUFFER_NESTED_PTR_PROTECTION .................. 0x%08x\n",
    Wsmt->ProtectionFlags & EFI_WSMT_PROTECTION_FLAGS_COMM_BUFFER_NESTED_PTR_PROTECTION
    );
  Print(
    L"      SYSTEM_RESOURCE_PROTECTION ......................... 0x%08x\n",
    Wsmt->ProtectionFlags & EFI_WSMT_PROTECTION_FLAGS_SYSTEM_RESOURCE_PROTECTION
    );

Done:
  Print (         
    L"*****************************************************************************\n\n"
    );

  return;
}

EFI_STATUS
EFIAPI
DumpAcpiWSMTLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return RegisterDumpAcpiTable (EFI_ACPI_WINDOWS_SMM_SECURITY_MITIGATION_TABLE_SIGNATURE, DumpAcpiWSMT);
}
