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
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/ShellLib.h>
#include <Library/DumpAcpiTableFuncLib.h>

#include <Guid/Acpi.h>

#include "DumpACPI.h"

#define RSDP_SIGN_DEFINITION SIGNATURE_32 ('R', 'S', 'D', 'P')
 
VOID
PrintUsage (
  VOID
  );

BOOLEAN                     mIsAcpi20;

BOOLEAN                     mIsDumpData;
BOOLEAN                     mIsDumpVerb;
BOOLEAN                     mIsParseAml;

BOOLEAN                     mHasSign;
UINT32                      mTableSign;

EFI_HANDLE                  mImageHandle;

SHELL_PARAM_ITEM mParamList[] = {
  {L"-?",                      TypeFlag},
  {L"-h",                      TypeFlag},
  {L"-d",                      TypeFlag},
  {L"-v",                      TypeFlag},
  {L"-b",                      TypeFlag},
  {L"-s",                      TypeValue},
  {NULL,                       TypeMax},
  };

EFI_STATUS
EFIAPI
DumpACPIEntryPoint (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
{
  LIST_ENTRY                *ParamPackage;
  CHAR8                     Sign[4];
  CHAR16                    *SignatureName;
  EFI_STATUS                Status;

  RegisterDumpAcpiTable (EFI_ACPI_1_0_FIXED_ACPI_DESCRIPTION_TABLE_SIGNATURE, DumpAcpiFADT);
  RegisterDumpAcpiTable (EFI_ACPI_1_0_FIRMWARE_ACPI_CONTROL_STRUCTURE_SIGNATURE, DumpAcpiFACS);
  RegisterDumpAcpiTable (EFI_ACPI_1_0_DIFFERENTIATED_SYSTEM_DESCRIPTION_TABLE_SIGNATURE, DumpAcpiDSDT);
  RegisterDumpAcpiTable (EFI_ACPI_1_0_SECONDARY_SYSTEM_DESCRIPTION_TABLE_SIGNATURE, DumpAcpiSSDT);
  RegisterDumpAcpiTable (EFI_ACPI_1_0_PERSISTENT_SYSTEM_DESCRIPTION_TABLE_SIGNATURE, DumpAcpiPSDT);

  Status = ShellCommandLineParse (mParamList, &ParamPackage, NULL, TRUE);
  if (EFI_ERROR(Status)) {
    Print(L"ERROR: Incorrect command line.\n");
    return Status;
  }

  //
  // Enable tab key which can pause the output
  //
//  EnableOutputTabPause();

  mImageHandle = ImageHandle;
  
  mIsDumpData = FALSE;
  mIsDumpVerb = FALSE;
  mIsParseAml = FALSE;
  mHasSign    = FALSE;
  mTableSign  = 0;
  
  if (ParamPackage == NULL) {
    mIsDumpVerb = TRUE;
  } else {
    if (ShellCommandLineGetFlag(ParamPackage, L"-?") ||
        ShellCommandLineGetFlag(ParamPackage, L"-h")) {
      PrintUsage ();
      return EFI_SUCCESS;
    }

    if (ShellCommandLineGetFlag(ParamPackage, L"-d")) {
      mIsDumpData = TRUE;
    }
    if (ShellCommandLineGetFlag(ParamPackage, L"-v")) {
      mIsDumpVerb = TRUE;
    }
    if (ShellCommandLineGetFlag(ParamPackage, L"-b")) {
  //    EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
    }

    SignatureName = (CHAR16 *)ShellCommandLineGetValue(ParamPackage, L"-s");
    if (SignatureName != NULL) {
      mHasSign = TRUE;
      Sign[0] = (CHAR8)SignatureName[0];
      Sign[1] = (CHAR8)SignatureName[1];
      Sign[2] = (CHAR8)SignatureName[2];
      Sign[3] = (CHAR8)SignatureName[3];
      mTableSign = SIGNATURE_32 (Sign[0], Sign[1], Sign[2], Sign[3]);
    }
  }

  if ((!mIsDumpVerb) && (!mIsDumpData)) {
    mIsDumpVerb = TRUE;
  }

  SetAcpiDumpPropertyDumpData (mIsDumpData);
  SetAcpiDumpPropertyDumpVerb (mIsDumpVerb);
  
  if (mHasSign) {
    DumpAcpiTableWithSign (mTableSign);
  } else {
    DumpAcpiAllTable ();
  }
  
  return EFI_SUCCESS;
}

VOID
DumpAcpiTableWithSign (
  UINT32                                TableSign
  )
{
  EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER                            *Rsdp;
  RSDT_TABLE                            *Rsdt;
  XSDT_TABLE                            *Xsdt;
  EFI_ACPI_DESCRIPTION_HEADER                    *Table;
  EFI_ACPI_DESCRIPTION_HEADER                    *SubTable1;
  EFI_ACPI_DESCRIPTION_HEADER                    *SubTable2;
  UINTN                                 EntryCount;
  UINTN                                 Index;
  UINT32                                *RsdtEntryPtr;
  UINT64                                *XsdtEntryPtr;
  UINT64                                TempEntry;

  Rsdp      = NULL;
  Rsdt      = NULL;
  Xsdt      = NULL;
  Table     = NULL;
  SubTable1 = NULL;
  SubTable2 = NULL;
  
  //
  // Scan RSDP
  //
  ScanAcpiRSDP (&Rsdp, &mIsAcpi20);
  if (Rsdp == NULL) {
    return;
  }

  //
  // Scan RSDT
  //
  ScanAcpiRSDT (Rsdp, &Rsdt);
  
  //
  // Scan XSDT
  //
  if (mIsAcpi20) {
    ScanAcpiXSDT (Rsdp, &Xsdt);
  }

  switch (TableSign) {
  case RSDP_SIGN_DEFINITION:
    DumpAcpiRSDP (Rsdp);
    return;
  case EFI_ACPI_1_0_ROOT_SYSTEM_DESCRIPTION_TABLE_SIGNATURE:
    DumpAcpiRSDT (Rsdt);
    return;
  case EFI_ACPI_2_0_EXTENDED_SYSTEM_DESCRIPTION_TABLE_SIGNATURE:
    DumpAcpiXSDT (Xsdt);
    return;
  default:
    break;
  }
 
  //
  // Dump each table in RSDT
  //
  if ((Xsdt == NULL) && (Rsdt != NULL)) {
    EntryCount = (Rsdt->Header.Length - sizeof (EFI_ACPI_DESCRIPTION_HEADER)) / 4;
    RsdtEntryPtr = &Rsdt->Entry;    
    for (Index = 0; Index < EntryCount; Index ++, RsdtEntryPtr ++) {
      Table = (EFI_ACPI_DESCRIPTION_HEADER *)((UINTN)(*RsdtEntryPtr));
      if (Table->Signature == TableSign) {
        DumpSelectAcpiTable (Table);
      }
      if (Table->Signature == EFI_ACPI_1_0_FIXED_ACPI_DESCRIPTION_TABLE_SIGNATURE) {
        //
        // Record FACS and DSDT
        //
        ScanAcpiFADT ((EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE *)Table, (EFI_ACPI_4_0_FIRMWARE_ACPI_CONTROL_STRUCTURE **)&SubTable1, (EFI_ACPI_DESCRIPTION_HEADER **)&SubTable2);
      }
    }
  }
  
  //
  // Dump each table in XSDT
  //
  if (Xsdt != NULL) {
    EntryCount = (Xsdt->Header.Length - sizeof (EFI_ACPI_DESCRIPTION_HEADER)) / 8;
    XsdtEntryPtr = (UINT64 *)(UINTN)&Xsdt->Entry;
    CopyMem(&TempEntry, XsdtEntryPtr, sizeof(UINT64));
    for (Index = 0; Index < EntryCount; Index ++, XsdtEntryPtr ++) {
      CopyMem(&TempEntry, XsdtEntryPtr, sizeof(UINT64));
      Table = (EFI_ACPI_DESCRIPTION_HEADER *)((UINTN)TempEntry);
      if (Table->Signature == TableSign) {
        DumpSelectAcpiTable (Table);
      }
      if (Table->Signature == EFI_ACPI_1_0_FIXED_ACPI_DESCRIPTION_TABLE_SIGNATURE) {
        //
        // Record FACS and DSDT
        //
        ScanAcpiFADT ((EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE *)Table, (EFI_ACPI_4_0_FIRMWARE_ACPI_CONTROL_STRUCTURE **)&SubTable1, (EFI_ACPI_DESCRIPTION_HEADER **)&SubTable2);
      }
    }
  }

  switch (TableSign) {
  case EFI_ACPI_1_0_FIRMWARE_ACPI_CONTROL_STRUCTURE_SIGNATURE:
    DumpAcpiFACS ((EFI_ACPI_4_0_FIRMWARE_ACPI_CONTROL_STRUCTURE *)SubTable1);
    return;
  case EFI_ACPI_1_0_DIFFERENTIATED_SYSTEM_DESCRIPTION_TABLE_SIGNATURE:
    DumpAcpiDSDT ((EFI_ACPI_DESCRIPTION_HEADER *)SubTable2);
    return;
  default:
    break;
  }
  
  return;
}

VOID
DumpAcpiAllTable (
  VOID
  )
{
  EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER                            *Rsdp;
  RSDT_TABLE                            *Rsdt;
  XSDT_TABLE                            *Xsdt;
  EFI_ACPI_DESCRIPTION_HEADER                    *Table;
  EFI_ACPI_DESCRIPTION_HEADER                    *SubTable1;
  EFI_ACPI_DESCRIPTION_HEADER                    *SubTable2;
  UINTN                                 EntryCount;
  UINTN                                 Index;
  UINT32                                *RsdtEntryPtr;
  UINT64                                *XsdtEntryPtr;
  UINT64                                TempEntry;

  Rsdp = NULL;
  Rsdt = NULL;
  Xsdt = NULL;
  
  //
  // Dump RSDP
  //
  ScanAcpiRSDP (&Rsdp, &mIsAcpi20);
  if (Rsdp == NULL) {
    return;
  }
  DumpAcpiRSDP (Rsdp);

  //
  // Dump RSDT
  //
  ScanAcpiRSDT (Rsdp, &Rsdt);
  DumpAcpiRSDT (Rsdt);
  
  //
  // Dump XSDT
  //
  if (mIsAcpi20) {
    ScanAcpiXSDT (Rsdp, &Xsdt);
    DumpAcpiXSDT (Xsdt);
  }

  //
  // Dump each table in RSDT
  //
  if ((Xsdt == NULL) && (Rsdt != NULL)) {
    EntryCount = (Rsdt->Header.Length - sizeof (EFI_ACPI_DESCRIPTION_HEADER)) / 4;
    RsdtEntryPtr = &Rsdt->Entry;    
    for (Index = 0; Index < EntryCount; Index ++, RsdtEntryPtr ++) {
      Table = (EFI_ACPI_DESCRIPTION_HEADER *)((UINTN)(*RsdtEntryPtr));
      if (Table == NULL) {
        continue;
      }
      DumpSelectAcpiTable (Table);
      if (Table->Signature == EFI_ACPI_1_0_FIXED_ACPI_DESCRIPTION_TABLE_SIGNATURE) {
        //
        // Record FACS and DSDT
        //
        ScanAcpiFADT ((EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE *)Table, (EFI_ACPI_4_0_FIRMWARE_ACPI_CONTROL_STRUCTURE **)&SubTable1, (EFI_ACPI_DESCRIPTION_HEADER **)&SubTable2);
        DumpAcpiFACS ((EFI_ACPI_4_0_FIRMWARE_ACPI_CONTROL_STRUCTURE *)SubTable1);
        DumpAcpiDSDT ((EFI_ACPI_DESCRIPTION_HEADER *)SubTable2);
      }
    }
  }
  
  //
  // Dump each table in XSDT
  //
  if (Xsdt != NULL) {
    EntryCount = (Xsdt->Header.Length - sizeof (EFI_ACPI_DESCRIPTION_HEADER)) / 8;
    XsdtEntryPtr = (UINT64 *)(UINTN)&Xsdt->Entry;
    CopyMem(&TempEntry, XsdtEntryPtr, sizeof(UINT64));
    for (Index = 0; Index < EntryCount; Index ++, XsdtEntryPtr ++) {
      CopyMem(&TempEntry, XsdtEntryPtr, sizeof(UINT64));
      Table = (EFI_ACPI_DESCRIPTION_HEADER *)((UINTN)TempEntry);
      if (Table == NULL) {
        continue;
      }
      DumpSelectAcpiTable (Table);
      if (Table->Signature == EFI_ACPI_1_0_FIXED_ACPI_DESCRIPTION_TABLE_SIGNATURE) {
        //
        // Record FACS and DSDT
        //
        ScanAcpiFADT ((EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE *)Table, (EFI_ACPI_4_0_FIRMWARE_ACPI_CONTROL_STRUCTURE **)&SubTable1, (EFI_ACPI_DESCRIPTION_HEADER **)&SubTable2);
        DumpAcpiFACS ((EFI_ACPI_4_0_FIRMWARE_ACPI_CONTROL_STRUCTURE *)SubTable1);
        DumpAcpiDSDT ((EFI_ACPI_DESCRIPTION_HEADER *)SubTable2);
      }
    }
  }

  return;
}

VOID
ScanAcpiRSDP (
  EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER                            **RsdpTable,
  BOOLEAN                               *Is20Table
  )
{
  UINTN                     Index;
  EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER                *Rsdp;
  
  if ((RsdpTable == NULL) || (Is20Table == NULL)) {
    return;
  }
  Rsdp = NULL;
  *Is20Table = FALSE;
  
  //
  // scan for ACPI 1.0b table first
  //
  for (Index = 0; Index < gST->NumberOfTableEntries; Index ++) {
    if (CompareGuid (&gEfiAcpiTableGuid, &(gST->ConfigurationTable[Index].VendorGuid))) {
      Rsdp = gST->ConfigurationTable[Index].VendorTable;
      break;
    }
  }
  
  //
  // scan for ACPI 2.0b table
  //
  for (Index = 0; Index < gST->NumberOfTableEntries; Index ++) {
    if (CompareGuid (&gEfiAcpi20TableGuid, &(gST->ConfigurationTable[Index].VendorGuid))) {
      Rsdp = gST->ConfigurationTable[Index].VendorTable;
      *Is20Table = (Rsdp->Revision >= 2);
      break;
    }
  }
  
  *RsdpTable = Rsdp;

  return;
}

VOID
DumpAcpiRSDP (
  EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER                            *Rsdp
  )
{
  UINT8  *Signature;

  if (Rsdp == NULL) {
    return;
  }
  
  //
  // Dump RSDP content
  //
  Print (
    L"*****************************************************************************\n"
    L"*         Root System Description Pointer                                   *\n"
    L"*****************************************************************************\n"
    );
  if (mIsDumpData) {
    if (!mIsAcpi20) {
      DumpAcpiHex (20, Rsdp);
    } else {
      DumpAcpiHex (36, Rsdp);
    }
  }

  if (!mIsDumpVerb) {
    goto Done;
  }

  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"RSDP address ............................................. 0x%016lx\n" :
    L"RSDP address ............................................. 0x%08x\n",
    Rsdp
    );
  Signature = (UINT8 *)&Rsdp->Signature;
  Print (
    L"  Signature .............................................. '%c%c%c%c%c%c%c%c'\n",
    Signature[0],
    Signature[1],
    Signature[2],
    Signature[3],
    Signature[4],
    Signature[5],
    Signature[6],
    Signature[7]
    );
  Print (
    L"  Checksum ............................................... 0x%02x\n",
    Rsdp->Checksum
    );
  Print (
    L"  OEMID .................................................. '%c%c%c%c%c%c'\n",
    Rsdp->OemId[0],
    Rsdp->OemId[1],
    Rsdp->OemId[2],
    Rsdp->OemId[3],
    Rsdp->OemId[4],
    Rsdp->OemId[5]
    );
  if (mIsAcpi20) {
    Print (
      L"  Revision ............................................... 0x%02x\n",
      Rsdp->Revision
      );
  }
  Print (
    L"  RsdtAddress ............................................ 0x%08x\n",
    Rsdp->RsdtAddress
    );
  if (mIsAcpi20) {
    Print (
      L"  Length ................................................. 0x%08x\n",
      Rsdp->Length
      );
    Print (
      L"  XsdtAddress ............................................ 0x%016lx\n",
      Rsdp->XsdtAddress
      );
    Print (
      L"  Extended Checksum ...................................... 0x%02x\n",
      Rsdp->ExtendedChecksum
      );
  }         
  
Done:
  Print (
    L"*****************************************************************************\n\n"
    );

  return;
}

VOID
ScanAcpiRSDT (
  EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER                            *Rsdp,
  RSDT_TABLE                            **RsdtTable
  )
{
  RSDT_TABLE                *Rsdt;
  
  Rsdt = NULL;
  
  if (Rsdp != NULL) {
    Rsdt = (RSDT_TABLE*)((UINTN)Rsdp->RsdtAddress);    
  }
  if (RsdtTable != NULL) {
    *RsdtTable = Rsdt;
  }
  
  return;
}

VOID
DumpAcpiRSDT (
  RSDT_TABLE                            *Rsdt
  )
{
  UINTN                     EntryCount;
  UINTN                     Index;
  UINT32                    *EntryPtr;
  
  if (Rsdt == NULL) {
    return;
  }

  EntryCount = (Rsdt->Header.Length - sizeof (EFI_ACPI_DESCRIPTION_HEADER)) / 4;
  
  //
  // Dump RSDT content
  //
  Print (
    L"*****************************************************************************\n"
    L"*         Root System Description Table                                     *\n"
    L"*****************************************************************************\n"
    );

  if (mIsDumpData) {
    DumpAcpiHex (Rsdt->Header.Length, Rsdt);
  }

  if (!mIsDumpVerb) {
    goto Done;
  }

  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"RSDT address ............................................. 0x%016lx\n" :
    L"RSDT address ............................................. 0x%08x\n",
    Rsdt
    );
  
  DumpAcpiTableHeader (&(Rsdt->Header));
  
  Print (
    L"  Table Contents:\n"
    );
  
  EntryPtr = &Rsdt->Entry;
  for (Index = 0; Index < EntryCount; Index ++, EntryPtr ++) {
    Print (
      L"    Entry %2d: ............................................ 0x%08x\n",
      Index + 1,
      *EntryPtr
      );
  }

Done:
  Print (
    L"*****************************************************************************\n\n"
    );

  return;
}

VOID
ScanAcpiXSDT (
  EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER                            *Rsdp,
  XSDT_TABLE                            **XsdtTable
  )
{
  XSDT_TABLE                *Xsdt;
  
  Xsdt = NULL;
  
  if (Rsdp != NULL) {
    Xsdt = (XSDT_TABLE*)((UINTN)Rsdp->XsdtAddress);
  }
  if (XsdtTable != NULL) {
    *XsdtTable = Xsdt;
  }
  
  return;
}

VOID
DumpAcpiXSDT (
  XSDT_TABLE                            *Xsdt
  )
{
  UINTN                     EntryCount;
  UINTN                     Index;
  UINT64                    *EntryPtr;
  UINT64                    TempEntry;
    
  if (Xsdt == NULL) {
    return;
  }

  EntryCount = (Xsdt->Header.Length - sizeof (EFI_ACPI_DESCRIPTION_HEADER)) / 8;
  
  //
  // Dump XSDT content
  //
  Print (         
    L"*****************************************************************************\n"
    L"*         Extended System Description Table                                 *\n"
    L"*****************************************************************************\n"
    );

  if (mIsDumpData) {
    DumpAcpiHex (Xsdt->Header.Length, Xsdt);
  }

  if (!mIsDumpVerb) {
    goto Done;
  }

  Print (         
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"XSDT address ............................................. 0x%016lx\n" :
    L"XSDT address ............................................. 0x%08x\n",
    Xsdt
    );
  
  DumpAcpiTableHeader (&(Xsdt->Header));
  
  Print (         
    L"  Table Contents:\n"
    );
  
  EntryPtr = (UINT64 *)(UINTN)&Xsdt->Entry;
  for (Index = 0; Index < EntryCount; Index ++, EntryPtr ++) {
    CopyMem (&TempEntry, EntryPtr, sizeof(UINT64));
    Print (       
      L"    Entry %2d: ............................................ 0x%016lx\n",
      Index + 1,
      TempEntry
      );
  }

Done:
  Print (
    L"*****************************************************************************\n\n"
    );

  return;
}

VOID
DumpAcpiTables (
  EFI_ACPI_DESCRIPTION_HEADER                    *Table
  )
{
  UINT8                     *SignatureByte;
  
  if (Table == NULL) {
    return;
  }
  
  SignatureByte = (UINT8*)&Table->Signature;
  
  //
  // dump table
  //
  Print (         
    L"*****************************************************************************\n"
    );
  Print (         
    L"*         %c%c%c%c Table                                                        *\n",
    SignatureByte[0],
    SignatureByte[1],
    SignatureByte[2],
    SignatureByte[3]
    );
  Print (         
    L"*****************************************************************************\n"
    );

  if (mIsDumpData) {
    DumpAcpiHex (Table->Length, Table);
  }

  if (!mIsDumpVerb) {
    goto Done;
  }

  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"%c%c%c%c address ............................................. 0x%016lx\n" :
    L"%c%c%c%c address ............................................. 0x%08x\n",
    SignatureByte[0],
    SignatureByte[1],
    SignatureByte[2],
    SignatureByte[3],
    Table
    );

  DumpAcpiTableHeader (Table);

Done:  
  Print (         
    L"*****************************************************************************\n\n"
    );

  return;
}

VOID
DumpSelectAcpiTable (
  EFI_ACPI_DESCRIPTION_HEADER                    *Table
  )
{
  DUMP_ACPI_TABLE_FUNC  DumpAcpiTableFunc;

  DumpAcpiTableFunc = GetDumpAcpiTableFunc (Table->Signature);
  if (DumpAcpiTableFunc != NULL) {
    DumpAcpiTableFunc (Table);
  } else {
    DumpAcpiTables (Table);
  }
  return ;
}

//
// internal functions
//

VOID
ScanTableInRSDT (
  RSDT_TABLE            *Rsdt,
  UINT32                Signature,
  EFI_ACPI_DESCRIPTION_HEADER    **FoundTable,
  UINTN                 *Instance
  )
{
  UINTN                     Index;
  UINT32                    EntryCount;
  UINT32                    *EntryPtr;
  EFI_ACPI_DESCRIPTION_HEADER        *Table;
  
  *FoundTable = NULL;
  *Instance = 0;
  
  EntryCount = (Rsdt->Header.Length - sizeof (EFI_ACPI_DESCRIPTION_HEADER)) / 4;
  
  EntryPtr = &Rsdt->Entry;
  for (Index = 0; Index < EntryCount; Index ++, EntryPtr ++) {
    Table = (EFI_ACPI_DESCRIPTION_HEADER*)((UINTN)(*EntryPtr));
    if (Table->Signature == Signature) {
      *FoundTable = Table;
      (*Instance) ++;
    }
  }
  
  return;
}

VOID
ScanTableInXSDT (
  XSDT_TABLE            *Xsdt,
  UINT32                Signature,
  EFI_ACPI_DESCRIPTION_HEADER    **FoundTable,
  UINTN                 *Instance
  )
{
  UINTN         Index;
  UINT32        EntryCount;
  UINT64        EntryPtr;
  UINTN         BasePtr;
  
  EFI_ACPI_DESCRIPTION_HEADER    *Table;
  
  *FoundTable = NULL;
  *Instance = 0;
  
  EntryCount = (Xsdt->Header.Length - sizeof (EFI_ACPI_DESCRIPTION_HEADER)) / 8;
  
  BasePtr = (UINTN)(&(Xsdt->Entry));
  for (Index = 0; Index < EntryCount; Index ++) {
    CopyMem (&EntryPtr, (VOID *)(BasePtr + Index * sizeof(UINT64)), sizeof(UINT64));
    Table = (EFI_ACPI_DESCRIPTION_HEADER*)((UINTN)(EntryPtr));
    if (Table->Signature == Signature) {
      *FoundTable = Table;
      (*Instance) ++;
    }
  }
  
  return;
}

VOID
ScanAcpiFADT (
  EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE              *Fadt,
  EFI_ACPI_4_0_FIRMWARE_ACPI_CONTROL_STRUCTURE           **Facs,
  EFI_ACPI_DESCRIPTION_HEADER                            **Dsdt
  )
{
  BOOLEAN                  Extended;
  
  if (Fadt == NULL) {
    return;
  }

  Extended = (Fadt->Header.Revision >= EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE_REVISION);
  
  if (Facs != NULL) {
    if (!Extended) {
      *Facs = (VOID *)(UINTN)Fadt->FirmwareCtrl;
    } else {
      *Facs = (VOID *)(UINTN)Fadt->XFirmwareCtrl;
      if (*Facs == NULL) {
        *Facs = (VOID *)(UINTN)Fadt->FirmwareCtrl;
      }
    }
  }
  
  if (Dsdt != NULL) {
    if (!Extended) {
      *Dsdt = (VOID *)(UINTN)Fadt->Dsdt;
    } else {
      *Dsdt = (VOID *)(UINTN)Fadt->XDsdt;
      if (*Dsdt == NULL) {
        *Dsdt = (VOID *)(UINTN)Fadt->Dsdt;
      }
    }
  }
}

VOID
PrintUsage (
  VOID
  )
{
  Print (
    L"DumpACPI Version 0.9\n"
    L"Copyright (C) Intel Corp 2012. All rights reserved.\n"
    L"\n"
    L"Dumps ACPI1.0~ACPI5.0 Table in EFI Shell Environment.\n"
    L"\n"
    L"usage: DumpACPI [-d] [-v] [-b] [-s <SIGN>]\n"
    L"  -d    Dumps ACPI Table Raw Data.\n"
    L"  -v    Dumps ACPI Table Verbose Data.\n"
    L"  -s    Dumps ACPI Table with signature being <SIGN>.\n"
    L"        The signature should be defined value in ACPI spec.\n"
    L"        One exception is RSDP, please use RSDP instead of \'RSD PTR \'.\n"
    L"  -b    Displays one screen at a time.\n"
    L"\n"
    );
  return;
}
