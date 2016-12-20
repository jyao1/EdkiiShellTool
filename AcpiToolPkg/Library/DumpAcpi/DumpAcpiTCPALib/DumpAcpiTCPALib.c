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
#include <IndustryStandard/TcpaAcpi.h>
#include <IndustryStandard/UefiTcgPlatform.h>

#pragma pack(1)

#define EFI_TCG_ACPI_TABLE_REVISION_1_0  1
#define EFI_TCG_ACPI_TABLE_REVISION_2_0  2

typedef struct {
  EFI_ACPI_DESCRIPTION_HEADER       Header;
  UINT16                            Reserved;
  UINT32                            Laml;
  UINT64                            Lasa;
} EFI_TCG_ACPI_1_0_TABLE;

typedef struct {
  TPM_VERSION     VersionInfo;
  UINT32          PCRIndex;
  UINT32          EventType;
} TCPA_PCR_EVENT_STRUCT;

typedef struct {
  UINT32          PCRValueLen; 
  UINT8           PCRValue[1];
} TCPA_PCR_EVENT_STRUCT_PCR_VALUE;

typedef struct {
  UINT32          EventSize;
  UINT8           Event[1];
} TCPA_PCR_EVENT_STRUCT_EVENT;

typedef struct {
  UINT32          PCRIndex;
  UINT32          EventType;
  UINT8           Digest[20];
} TCPA_V2_PCR_EVENT_STRUCT;

typedef struct {
  UINT32          EventId;
  UINT32          EventDataSize;
  UINT8           EventData[1];
} TCPA_PCR_EVENT_STRUCT_EVENT_PLATFORM_SPECIFIC;

#pragma pack()

VOID
DumpTcpaPcrEventStructPcrValue (
  UINT32                                TcpaPcrEventStructPcrValueLength,
  VOID                                  *TcpaPcrEventStructPcrValue
  )
{
  if (TcpaPcrEventStructPcrValue == NULL) {
    return;
  }
  
  Print (
    L"    PCR Value Length ..................................... 0x%08x\n",
    TcpaPcrEventStructPcrValueLength
    );
  Print (
    L"    PCR Value:\n"
    );
  DumpAcpiHex (TcpaPcrEventStructPcrValueLength, TcpaPcrEventStructPcrValue);
  Print (
    L"\n"
    );

  return;
}

VOID
DumpTcpaPcrEventStructEventUnknown (
  TCPA_PCR_EVENT_STRUCT_EVENT           *TcpaPcrEventStructEvent
  )
{
  if (TcpaPcrEventStructEvent == NULL) {
    return;
  }
  
  Print (
    L"    Event Size ........................................... 0x%08x\n",
    TcpaPcrEventStructEvent->EventSize
    );
  Print (
    L"    Event Data:\n"
    );
  DumpAcpiHex (TcpaPcrEventStructEvent->EventSize, &TcpaPcrEventStructEvent->Event[0]);
  Print (
    L"\n"
    );

  return;
}

VOID
DumpTcpaPcrEventStructEventPlatformSpecific (
  TCPA_PCR_EVENT_STRUCT_EVENT           *TcpaPcrEventStructEvent
  )
{
  TCPA_PCR_EVENT_STRUCT_EVENT_PLATFORM_SPECIFIC *TcpaPcrEventStructEventPlatformSpecific;

  if (TcpaPcrEventStructEvent == NULL) {
    return;
  }
  
  DumpTcpaPcrEventStructEventUnknown (TcpaPcrEventStructEvent);

  TcpaPcrEventStructEventPlatformSpecific = (TCPA_PCR_EVENT_STRUCT_EVENT_PLATFORM_SPECIFIC *)&TcpaPcrEventStructEvent->Event[0];
  
  Print (
    L"    Platform Specific Event:\n"
    ); 
  Print (
    L"      Event Id ........................................... 0x%08x\n",
    TcpaPcrEventStructEventPlatformSpecific->EventId
    );
  switch (TcpaPcrEventStructEventPlatformSpecific->EventId) {
  case 1:
    Print (
      L"        SMBIOS Structure\n"
      );
    break;
  case 2:
    Print (
      L"        BIS Certificate\n"
      );
    break;
  case 3:
    Print (
      L"        POST BIOS ROM Strings\n"
      );
    break;
  case 4:
    Print (
      L"        ESCD\n"
      );
    break;
  case 5:
    Print (
      L"        CMOS\n"
      );
    break;
  case 6:
    Print (
      L"        NVRAM\n"
      );
    break;
  case 7:
    Print (
      L"        Option ROM Execute\n"
      );
    break;
  case 8:
    Print (
      L"        Option ROM Configuration\n"
      );
    break;
  case 0xA:
    Print (
      L"        Option ROM Microcode Update\n"
      );
    break;
  case 0xB:
    Print (
      L"        S-CRTM Version String\n"
      );
    break;
  case 0xC:
    Print (
      L"        S-CRTM Contents\n"
      );
    break;
  case 0xD:
    Print (
      L"        POST Contents\n"
      );
    break;
  case 0xE:
    Print (
      L"        Host Platform Manufacturer Table of Devices\n"
      );
    break;
  default:
    break;
  }
  Print (
    L"      Event Data Size .................................... 0x%08x\n",
    TcpaPcrEventStructEventPlatformSpecific->EventDataSize
    );
  Print (
    L"      Event Data:\n"
    );
  DumpAcpiHex (TcpaPcrEventStructEventPlatformSpecific->EventDataSize, &TcpaPcrEventStructEventPlatformSpecific->EventData[0]);
  Print (
    L"\n"
    );

  return;
}

VOID
DumpTcpaPcrEvent (
  UINT32                                Version,
  VOID                                  *TcpaPcrEvent,
  UINT32                                *TcpaPcrEventSize
  )
{
  TCPA_PCR_EVENT_STRUCT_PCR_VALUE       *TcpaPcrEventStructPcrValue;
  TCPA_PCR_EVENT_STRUCT_EVENT           *TcpaPcrEventStructEvent;
  UINT32                                EventType;
  TCPA_PCR_EVENT_STRUCT                 *Tcpav1PcrEvent;
  TCPA_V2_PCR_EVENT_STRUCT              *Tcpav2PcrEvent;
  
  if ((TcpaPcrEvent == NULL) || (TcpaPcrEventSize == NULL)) {
    return;
  }

  Tcpav1PcrEvent             = NULL;
  Tcpav2PcrEvent             = NULL;
  TcpaPcrEventStructEvent    = NULL;
  TcpaPcrEventStructPcrValue = NULL;
  EventType                  = 0xFFFFFFFF;

  if (Version < 2) {
    Tcpav1PcrEvent = TcpaPcrEvent;
    if ((Tcpav1PcrEvent->VersionInfo.major != 1) || (Tcpav1PcrEvent->VersionInfo.minor == 0)) {
      *TcpaPcrEventSize = 0;
      return;
    }

    // calculate Version, PCR Index, EventType
    *TcpaPcrEventSize = sizeof(TCPA_PCR_EVENT_STRUCT);
    // add PCR Value
    TcpaPcrEventStructPcrValue = (TCPA_PCR_EVENT_STRUCT_PCR_VALUE *)((UINT8 *)TcpaPcrEvent + *TcpaPcrEventSize);
    *TcpaPcrEventSize += sizeof(UINT32) + TcpaPcrEventStructPcrValue->PCRValueLen;
    // add Event
    TcpaPcrEventStructEvent = (TCPA_PCR_EVENT_STRUCT_EVENT *)((UINT8 *)TcpaPcrEvent + *TcpaPcrEventSize);
    *TcpaPcrEventSize += sizeof(UINT32) + TcpaPcrEventStructEvent->EventSize;

    EventType = Tcpav1PcrEvent->EventType;

    if (Tcpav1PcrEvent->PCRIndex == 0xFFFFFFFF) {
      *TcpaPcrEventSize = 0;
      return ;
    }

  } else {
    Tcpav2PcrEvent = TcpaPcrEvent;

    // calculate PCR Index, EventType, Digest
    *TcpaPcrEventSize = sizeof(TCPA_V2_PCR_EVENT_STRUCT);
    // add Event
    TcpaPcrEventStructEvent = (TCPA_PCR_EVENT_STRUCT_EVENT *)((UINT8 *)TcpaPcrEvent + *TcpaPcrEventSize);
    *TcpaPcrEventSize += sizeof(UINT32) + TcpaPcrEventStructEvent->EventSize;

    EventType = Tcpav2PcrEvent->EventType;

    if (Tcpav2PcrEvent->PCRIndex == 0xFFFFFFFF) {
      *TcpaPcrEventSize = 0;
      return ;
    }
  }
  
  Print (         
    L"  ***************************************************************************\n"
    ); 
  switch (EventType) {
  case 0:
    if (Version < 2) {
      Print (         
        L"  *       PCR EVENT CODE CERT                                               *\n"
        ); 
    } else {
      Print (         
        L"  *       PCR EVENT PREBOOT CERT                                            *\n"
        ); 
    }
    break;
  case EV_POST_CODE:
    if (Version < 2) {
      Print (         
        L"  *       PCR EVENT CODE NO CERT                                            *\n"
        ); 
    } else {
      Print (         
        L"  *       PCR EVENT POST CODE                                               *\n"
        ); 
    }
    break;
  case 2:
    if (Version < 2) {
      Print (         
        L"  *       PCR EVENT XML CONFIG                                              *\n"
        ); 
    } else {
      Print (         
        L"  *       PCR EVENT Unknown                                                 *\n"
        ); 
    }
    break;
  case 3:
    Print (         
      L"  *       PCR EVENT NO ACTION                                               *\n"
      ); 
    break;
  case EV_SEPARATOR:
    Print (         
      L"  *       PCR EVENT SEPARATOR                                               *\n"
      ); 
    break;
  case 5:
    Print (         
      L"  *       PCR EVENT ACTION                                                  *\n"
      ); 
    break;
  case 6:
    if (Version < 2) {
      Print (         
        L"  *       PCR EVENT PLATFORM SPECIFIC                                       *\n"
        ); 
    } else {
      Print (         
        L"  *       PCR EVENT TAG                                                     *\n"
        ); 
    }
    break;
  case EV_S_CRTM_CONTENTS:
    Print (         
      L"  *       PCR EVENT S-CRTM CONTENTS                                         *\n"
      ); 
    break;
  case EV_S_CRTM_VERSION:
    Print (         
      L"  *       PCR EVENT S-CRTM VERION                                           *\n"
      ); 
    break;
  case 9:
    Print (         
      L"  *       PCR EVENT CPU MICROCODE                                           *\n"
      ); 
    break;
  case 0xA:
    Print (         
      L"  *       PCR EVENT PLATFORM CONFIG FLAGS                                   *\n"
      ); 
    break;
  case 0xB:
    Print (         
      L"  *       PCR EVENT TABLE OF DEVICES                                        *\n"
      ); 
    break;
  case 0xC:
    Print (         
      L"  *       PCR EVENT COMPACT HASH                                            *\n"
      ); 
    break;
  case 0xD:
    Print (         
      L"  *       PCR EVENT IPL                                                     *\n"
      ); 
    break;
  case 0xE:
    Print (         
      L"  *       PCR EVENT IPL PARTITION DATA                                      *\n"
      ); 
    break;
  case 0xF:
    Print (         
      L"  *       PCR EVENT NON-HOST CODE                                           *\n"
      ); 
    break;
  case 0x10:
    Print (         
      L"  *       PCR EVENT NON-HOST CONFIG                                         *\n"
      ); 
    break;
  case 0x11:
    Print (         
      L"  *       PCR EVENT NON_HOST INFO                                           *\n"
      ); 
    break;
  case EV_EFI_VARIABLE_DRIVER_CONFIG:
    Print (         
      L"  *       PCR EVENT UEFI VARIABLE DRIVER CONFIG                             *\n"
      ); 
    break;
  case EV_EFI_VARIABLE_BOOT:
    Print (         
      L"  *       PCR EVENT UEFI VARIABLE BOOT                                      *\n"
      ); 
    break;
  case EV_EFI_BOOT_SERVICES_APPLICATION:
    Print (         
      L"  *       PCR EVENT UEFI BOOT SERVICES APPLICATION                          *\n"
      ); 
    break;
  case EV_EFI_BOOT_SERVICES_DRIVER:
    Print (         
      L"  *       PCR EVENT UEFI BOOT SERVICES DRIVER                               *\n"
      ); 
    break;
  case EV_EFI_RUNTIME_SERVICES_DRIVER:
    Print (         
      L"  *       PCR EVENT UEFI RUNTIME SERVICES DRIVER                            *\n"
      ); 
    break;
  case EV_EFI_GPT_EVENT:
    Print (         
      L"  *       PCR EVENT UEFI GPT EVENT                                          *\n"
      ); 
    break;
  case EV_EFI_ACTION:
    Print (         
      L"  *       PCR EVENT UEFI ACTION                                             *\n"
      ); 
    break;
  case EV_EFI_PLATFORM_FIRMWARE_BLOB:
    Print (         
      L"  *       PCR EVENT UEFI PLATFORM FIRMWARE BLOB                             *\n"
      ); 
    break;
  case EV_EFI_HANDOFF_TABLES:
    Print (         
      L"  *       PCR EVENT UEFI HANDOFF TABLES                                     *\n"
      ); 
    break;
  default:
    if (EventType < EV_EFI_EVENT_BASE) {
      Print (         
        L"  *       PCR EVENT Unknown                                                 *\n"
        ); 
    } else {
      Print (         
        L"  *       PCR EVENT UEFI Unknown                                            *\n"
        ); 
    }
    break;
  }
  Print (         
    L"  ***************************************************************************\n"
    ); 

  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  PCR EVENT address ...................................... 0x%016lx\n" :
    L"  PCR EVENT address ...................................... 0x%08x\n",
    TcpaPcrEvent
    );
  if (Version < 2) {
    Print (
      L"    VersionInfo .......................................... 0x%08x\n"
      L"      Version ............................................ %d.%d\n"
      L"      Revision ........................................... %d.%d\n",
      *(UINT32 *)&Tcpav1PcrEvent->VersionInfo,
      Tcpav1PcrEvent->VersionInfo.major,
      Tcpav1PcrEvent->VersionInfo.minor,
      Tcpav1PcrEvent->VersionInfo.revMajor,
      Tcpav1PcrEvent->VersionInfo.revMinor
      );
    Print (
      L"    PCR Index ............................................ 0x%08x\n",
      Tcpav1PcrEvent->PCRIndex
      );
  } else {
    Print (
      L"    PCR Index ............................................ 0x%08x\n",
      Tcpav2PcrEvent->PCRIndex
      );
  }
  Print (
    L"    PCR Event Type ....................................... 0x%08x\n",
    EventType
    );
  
  if (Version < 2) {
    DumpTcpaPcrEventStructPcrValue (TcpaPcrEventStructPcrValue->PCRValueLen, &TcpaPcrEventStructPcrValue->PCRValue[0]);
  } else {
    DumpTcpaPcrEventStructPcrValue (20, &Tcpav2PcrEvent->Digest[0]);
  }
  
  switch (EventType) {
  case 6:
    if (Version < 2) {
      DumpTcpaPcrEventStructEventPlatformSpecific (TcpaPcrEventStructEvent);
    } else {
      DumpTcpaPcrEventStructEventUnknown (TcpaPcrEventStructEvent);
    }
    break;
  default:
    DumpTcpaPcrEventStructEventUnknown (TcpaPcrEventStructEvent);
    break;
  }
  
  Print (         
    L"  ***************************************************************************\n\n"
    ); 

  return;
}

VOID
DumpTcpaLogArea (
  UINT32                                Version,
  VOID                                  *Laml,
  UINT64                                Lasa
  )
{
  VOID                                  *PcrEvent;
  UINT32                                PcrEventStructSize;

  if (Laml == 0) {
    return;
  }

  PcrEvent = Laml;
  while ((UINTN)PcrEvent < (UINTN)((UINTN)Laml + (UINTN)Lasa)) {
    DumpTcpaPcrEvent (Version, PcrEvent, &PcrEventStructSize);
    if (PcrEventStructSize == 0) {
      break;
    }
    PcrEvent = (TCPA_PCR_EVENT_STRUCT *)((UINTN)PcrEvent + (UINTN)PcrEventStructSize);
  }
}

VOID
DumpAcpiTcpav1 (
  EFI_TCG_ACPI_1_0_TABLE                         *Tcpav1
  )
{
  if (Tcpav1 == NULL) {
    return;
  }

  Print (
    L"    Log Area Maximum Length .............................. 0x%08x\n",
    Tcpav1->Laml
    );
  Print (
    L"    Log Area Start Address ............................... 0x%016lx\n",
    Tcpav1->Lasa
    );
 
  if (GetAcpiDumpPropertyDumpData()) {
    Print (
      L"    Log Area Data :\n"
      );
    DumpAcpiHex (Tcpav1->Laml, (VOID *)(UINTN)Tcpav1->Lasa);
  }
  
  if (GetAcpiDumpPropertyDumpVerb()) {
    DumpTcpaLogArea (1, (VOID *)(UINTN)Tcpav1->Lasa, (UINT64)Tcpav1->Laml);
  }

  return;
}

VOID
DumpAcpiTcpav2client (
  EFI_TCG_CLIENT_ACPI_TABLE                   *Tcpav2client
  )
{
  if (Tcpav2client == NULL) {
    return;
  }

  Print (
    L"    Platform Class ....................................... 0x%04x\n",
    Tcpav2client->PlatformClass
    );
    Print (
      L"      PC Client Platform\n"
      );

  Print (
    L"    Log Area Maximum Length .............................. 0x%08x\n",
    Tcpav2client->Laml
    );
  Print (
    L"    Log Area Start Address ............................... 0x%016lx\n",
    Tcpav2client->Lasa
    );
 
  if (GetAcpiDumpPropertyDumpData()) {
    Print (
      L"    Log Area Data :\n"
      );
    DumpAcpiHex (Tcpav2client->Laml, (VOID *)(UINTN)Tcpav2client->Lasa);
  }
  
  if (GetAcpiDumpPropertyDumpVerb()) {
    DumpTcpaLogArea (2, (VOID *)(UINTN)Tcpav2client->Lasa, (UINT64)Tcpav2client->Laml);
  }

  return;
}

VOID
DumpAcpiTcpav2server (
  EFI_TCG_SERVER_ACPI_TABLE                   *Tcpav2server
  )
{
  if (Tcpav2server == NULL) {
    return;
  }

  Print (
    L"    Platform Class ....................................... 0x%04x\n",
    Tcpav2server->PlatformClass
    );
    Print (
      L"      Server Platform\n"
      );

  Print (
    L"    Specification Revision ............................... 0x%04x\n",
    Tcpav2server->SpecRev
    );
  Print (
    L"    Device Flags ......................................... 0x%02x\n",
    Tcpav2server->DeviceFlags
    );
  Print (
    L"      PCI Device Flag .................................... 0x%02x\n",
    Tcpav2server->DeviceFlags & 0x1
    );
  Print (
    L"      TPM Bus is PNP ..................................... 0x%02x\n",
    Tcpav2server->DeviceFlags & 0x2
    );
  Print (
    L"      TPM Configuration Address Valid .................... 0x%02x\n",
    Tcpav2server->DeviceFlags & 0x4
    );
  Print (
    L"    Interrupt Flags ...................................... 0x%02x\n",
    Tcpav2server->InterruptFlags
    );
  Print (
    L"      Interrupt Mode ..................................... 0x%02x\n",
    Tcpav2server->InterruptFlags & 0x1
    );
  switch (Tcpav2server->InterruptFlags & 0x1) {
  case 0x0:
    Print (
     L"        Level-Triggered\n"
     );
    break;
  case 0x1:
    Print (
     L"        Edge-Triggered\n"
     );
    break;
  default:
    break;
  }
  Print (
    L"      Interrupt Polarity ................................. 0x%02x\n",
    Tcpav2server->InterruptFlags & 0x2
    );
  switch (Tcpav2server->InterruptFlags & 0x2) {
  case 0x0:
    Print (
     L"        Active-High\n"
     );
    break;
  case 0x2:
    Print (
     L"        Active-Low\n"
     );
    break;
  default:
    break;
  }
  Print (
    L"      SCI Triggered Through GPE .......................... 0x%02x\n",
    Tcpav2server->InterruptFlags & 0x4
    );
  Print (
    L"      IO APIC/SAPIC Interrupt ............................ 0x%02x\n",
    Tcpav2server->InterruptFlags & 0x8
    );
  Print (
    L"    GPE .................................................. 0x%02x\n",
    Tcpav2server->Gpe
    );
  Print (
    L"    Global System Interrupt .............................. 0x%08x\n",
    Tcpav2server->GlobalSysInt
    );
  Print (
    L"    Base Address\n"
    );
  DumpAcpiGAddressStructure ((EFI_ACPI_3_0_GENERIC_ADDRESS_STRUCTURE *)&(Tcpav2server->BaseAddress));
  Print (
    L"    Configuration Address\n"
    );
  DumpAcpiGAddressStructure ((EFI_ACPI_3_0_GENERIC_ADDRESS_STRUCTURE *)&(Tcpav2server->ConfigAddress));
  Print (
    L"    PCI Segment Group Number ............................. 0x%02x\n",
    Tcpav2server->PciSegNum
    );
  Print (
    L"    PCI Bus Number ....................................... 0x%02x\n",
    Tcpav2server->PciBusNum
    );
  Print (
    L"    PCI Device Number .................................... 0x%02x\n",
    Tcpav2server->PciDevNum & 0x1F
    );
  Print (
    L"    PCI Function Number .................................. 0x%02x\n",
    Tcpav2server->PciFuncNum & 0x7
    );

  Print (
    L"    Log Area Maximum Length .............................. 0x%016lx\n",
    Tcpav2server->Laml
    );
  Print (
    L"    Log Area Start Address ............................... 0x%016lx\n",
    Tcpav2server->Lasa
    );
 
  if (GetAcpiDumpPropertyDumpData()) {
    Print (
      L"    Log Area Data :\n"
      );
    DumpAcpiHex ((UINTN)Tcpav2server->Laml, (VOID *)(UINTN)Tcpav2server->Lasa);
  }
  
  if (GetAcpiDumpPropertyDumpVerb()) {
    DumpTcpaLogArea (2, (VOID *)(UINTN)Tcpav2server->Lasa, (UINT64)Tcpav2server->Laml);
  }

  return;
}

VOID
EFIAPI
DumpAcpiTCPA (
  VOID  *Table
  )
{
  EFI_TCG_ACPI_1_0_TABLE                            *Tcpa;

  Tcpa = Table;
  if (Tcpa == NULL) {
    return;
  }
  
  //
  // Dump Tcpa table
  //
  Print (
    L"*****************************************************************************\n"
    L"*         Trusted Computing Platform Alliance Capabilities Table            *\n"
    L"*****************************************************************************\n"
    );
    
  if (GetAcpiDumpPropertyDumpData()) {
    DumpAcpiHex (Tcpa->Header.Length, Tcpa);
  }
  
  if (!GetAcpiDumpPropertyDumpVerb()) {
    goto Done;
  }

  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"TCPA address ............................................. 0x%016lx\n" :
    L"TCPA address ............................................. 0x%08x\n",
    Tcpa
    );
  
  DumpAcpiTableHeader(&(Tcpa->Header));
  
  Print (
    L"  Table Contents:\n"
    );

  if (Tcpa->Header.Revision < EFI_TCG_ACPI_TABLE_REVISION_2_0) {
    DumpAcpiTcpav1 ((EFI_TCG_ACPI_1_0_TABLE *)Tcpa);
  } else {
    switch (((EFI_TCG_CLIENT_ACPI_TABLE *)Tcpa)->PlatformClass) {
    case TCG_PLATFORM_TYPE_CLIENT:
      DumpAcpiTcpav2client ((EFI_TCG_CLIENT_ACPI_TABLE *)Tcpa);
      break;
    case TCG_PLATFORM_TYPE_SERVER:
      DumpAcpiTcpav2server ((EFI_TCG_SERVER_ACPI_TABLE *)Tcpa);
      break;
    default:
      break;
    }
  }

Done:
  Print (         
    L"*****************************************************************************\n\n"
    );
  
  return;
}

EFI_STATUS
EFIAPI
DumpAcpiTCPALibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return RegisterDumpAcpiTable (EFI_ACPI_3_0_TRUSTED_COMPUTING_PLATFORM_ALLIANCE_CAPABILITIES_TABLE_SIGNATURE, DumpAcpiTCPA);
}
