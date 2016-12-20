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
#include <IndustryStandard/AlertStandardFormatTable.h>

#pragma pack(1)

typedef struct {
  EFI_ACPI_DESCRIPTION_HEADER    Header;
} ASF_TABLE;

#pragma pack()

VOID
DumpAsfInfo (
  EFI_ACPI_ASF_INFO                            *AsfInfo
  )
{
  if (AsfInfo == NULL) {
    return;
  }
  
  Print (         
    L"  ***************************************************************************\n"
    L"  *       ASF_INFO                                                          *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  ASF_INFO address ....................................... 0x%016lx\n" :
    L"  ASF_INFO address ....................................... 0x%08x\n",
    AsfInfo
    ); 
  Print (
    L"      Type ............................................... 0x%02x\n",
    AsfInfo->RecordHeader.Type
    ); 
  Print (
    L"      Record Length ...................................... 0x%02x\n",
    AsfInfo->RecordHeader.RecordLength
    ); 
  Print (
    L"      Minumum Watchdog Reset Value ....................... 0x%02x\n",
    AsfInfo->MinWatchDogResetValue
    ); 
  Print (
    L"      Minumum Asf Sensor Interpoll Wait Time ............. 0x%02x\n",
    AsfInfo->MinPollingInterval
    );     
  Print (
    L"      System ID .......................................... 0x%04x\n",
    AsfInfo->SystemID
    ); 
  Print (
    L"      IANA Manufacture ID ................................ 0x%08x\n",
    AsfInfo->IANAManufactureID
    ); 
  Print (
    L"      Feature Flags ...................................... 0x%02x\n",
    AsfInfo->FeatureFlags
    ); 
    
  Print (         
    L"  ***************************************************************************\n\n"
    );
  return;
}

VOID
DumpAsfAlert (
  EFI_ACPI_ASF_ALRT                            *AsfAlert
  )
{
  EFI_ACPI_ASF_ALERTDATA                        *AsfAlertDataEntry;
  UINTN                                Index;

  if (AsfAlert == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       ASF_ALERT                                                         *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  ASF_ALERT address ...................................... 0x%016lx\n" :
    L"  ASF_ALERT address ...................................... 0x%08x\n",
    AsfAlert
    );  
  Print (
    L"      Type ............................................... 0x%02x\n",
    AsfAlert->RecordHeader.Type
    ); 
  Print (
    L"      Record Length ...................................... 0x%02x\n",
    AsfAlert->RecordHeader.RecordLength
    ); 
  Print (
    L"      Assertion Event Bit Mask ........................... 0x%02x\n",
    AsfAlert->AssertionEventBitMask
    );    
  Print (
    L"      De-Assertion Event Bit Mask ........................ 0x%02x\n",
    AsfAlert->DeassertionEventBitMask
    ); 
  Print (
    L"      Number Of Alerts ................................... 0x%02x\n",
    AsfAlert->NumberOfAlerts
    );    
  Print (
    L"      Array Element Length ............................... 0x%02x\n",
    AsfAlert->ArrayElementLength
    ); 
  Print (
    L"      Device Array\n"
    ); 
  AsfAlertDataEntry = (EFI_ACPI_ASF_ALERTDATA*)(AsfAlert + 1);
  
  for (Index = 0; Index < AsfAlert->NumberOfAlerts; Index ++, AsfAlertDataEntry ++) {
    Print (L"        Alert Data %d:\n", Index); 
    Print (
      L"          Device Address ................................. 0x%02x\n",
      AsfAlertDataEntry->DeviceAddress
      ); 
    Print (
      L"          Command ........................................ 0x%02x\n",
      AsfAlertDataEntry->Command
      );     
    Print (
      L"          Data Mask ...................................... 0x%02x\n",
      AsfAlertDataEntry->DataMask
      ); 
    Print (
      L"          Compare Value .................................. 0x%02x\n",
      AsfAlertDataEntry->CompareValue
      ); 
    Print (
      L"          Event Sensor Type .............................. 0x%02x\n",
      AsfAlertDataEntry->EventSenseType
      ); 
    Print (
      L"          Event Type ..................................... 0x%02x\n",
      AsfAlertDataEntry->EventType
      ); 
    Print (
      L"          Event Offset ................................... 0x%02x\n",
      AsfAlertDataEntry->EventOffset
      ); 
    Print (
      L"          Event Source Type .............................. 0x%02x\n",
      AsfAlertDataEntry->EventSourceType
      ); 
    Print (
      L"          Event Severity ................................. 0x%02x\n",
      AsfAlertDataEntry->EventSeverity
      ); 
    Print (
      L"          Sensor Number .................................. 0x%02x\n",
      AsfAlertDataEntry->SensorNumber
      );     
    Print (
      L"          Entity ......................................... 0x%02x\n",
      AsfAlertDataEntry->Entity
      ); 
    Print (
      L"          Entity Instance ................................ 0x%02x\n",
      AsfAlertDataEntry->EntityInstance
      );  
  }    

  Print (         
    L"  ***************************************************************************\n\n"
    );
  return;
}

VOID
DumpAsfRctl (
  EFI_ACPI_ASF_RCTL                            *AsfRctl
  )
{
  EFI_ACPI_ASF_CONTROLDATA                     *AsfRctlDataEntry;
  UINTN                               Index;

  if (AsfRctl == NULL) {
    return;
  }
  
  Print (         
    L"  ***************************************************************************\n"
    L"  *       ASF_RCTL                                                          *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  ASF_RCTL address ....................................... 0x%016lx\n" :
    L"  ASF_RCTL address ....................................... 0x%08x\n",
    AsfRctl
    );   
  Print (
    L"      Type ............................................... 0x%02x\n",
    AsfRctl->RecordHeader.Type
    ); 
  Print (
    L"      Record Length ...................................... 0x%02x\n",
    AsfRctl->RecordHeader.RecordLength
    ); 
  Print (
    L"      Number Of Controls ................................. 0x%02x\n",
    AsfRctl->NumberOfControls
    );    
  Print (
    L"      Array Element Length ............................... 0x%02x\n",
    AsfRctl->ArrayElementLength
    ); 
  Print (
    L"      Control Array\n"
    ); 
  AsfRctlDataEntry = (EFI_ACPI_ASF_CONTROLDATA*)(AsfRctl + 1);
  
  for (Index = 0; Index < AsfRctl->NumberOfControls; Index ++, AsfRctlDataEntry ++) {
    Print (L"        Control Data %d:\n", Index); 
    Print (
      L"          Function ....................................... 0x%02x\n",
      AsfRctlDataEntry->Function
      ); 
    Print (
      L"          Device Address ................................. 0x%02x\n",
      AsfRctlDataEntry->DeviceAddress
      ); 
    Print (
      L"          Command ........................................ 0x%02x\n",
      AsfRctlDataEntry->Command
      );     
    Print (
      L"          Data Value ..................................... 0x%02x\n",
      AsfRctlDataEntry->DataValue
      ); 
  }    

  Print (         
    L"  ***************************************************************************\n\n"
    );
  return;
}

VOID
DumpAsfRmcp (
  EFI_ACPI_ASF_RMCP                            *AsfRmcp
  )
{
  if (AsfRmcp == NULL) {
    return;
  }
  
  Print (         
    L"  ***************************************************************************\n"
    L"  *       ASF_RMCP                                                          *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  ASF_RMCP address ....................................... 0x%016lx\n" :
    L"  ASF_RMCP address ....................................... 0x%08x\n",
    AsfRmcp
    );    
  Print (
    L"      Type ............................................... 0x%02x\n",
    AsfRmcp->RecordHeader.Type
    ); 
  Print (
    L"      Record Length ...................................... 0x%02x\n",
    AsfRmcp->RecordHeader.RecordLength
    ); 

  Print (
    L"      RMCP Boot Options Capabilites\n"
    );     
  Print (
    L"        RMCP System Firmware Capabilities Bit Mask byte1 . 0x%02x\n",
    AsfRmcp->RemoteControlCapabilities[0]
    ); 
  Print (
    L"        RMCP System Firmware Capabilities Bit Mask byte2 . 0x%02x\n",
    AsfRmcp->RemoteControlCapabilities[1]
    );
  Print (
    L"        RMCP System Firmware Capabilities Bit Mask byte3 . 0x%02x\n",
    AsfRmcp->RemoteControlCapabilities[2]
    );
  Print (
    L"        RMCP System Firmware Capabilities Bit Mask byte4 . 0x%02x\n",
    AsfRmcp->RemoteControlCapabilities[3]
    );
  Print (
    L"        RMCP Special Commands Bit Mask byte1 ............. 0x%02x\n",
    AsfRmcp->RemoteControlCapabilities[4]
    );
  Print (
    L"        RMCP Special Commands Bit Mask byte2 ............. 0x%02x\n",
    AsfRmcp->RemoteControlCapabilities[5]
    );
  Print (
    L"        RMCP System Capabilites Bit Mask ................. 0x%02x\n",
    AsfRmcp->RemoteControlCapabilities[6]
    );
  Print (
    L"      RCMP Boot Options Completion Code .................. 0x%02x\n",
    AsfRmcp->RMCPCompletionCode
    ); 
  Print (
    L"      RCMP IANA Enterprise ID ............................ %08x\n",
    AsfRmcp->RMCPIANA
    ); 
  Print (
    L"      RCMP Special Command ............................... 0x%02x\n",
    AsfRmcp->RMCPSpecialCommand
    );
  Print (
    L"      RCMP Special Command Parameter Byte 1 .............. 0x%02x\n",
    AsfRmcp->RMCPSpecialCommandParameter[0]
    );
  Print (
    L"      RCMP Special Command Parameter Byte 2 .............. 0x%02x\n",
    AsfRmcp->RMCPSpecialCommandParameter[1]
    );
  Print (
    L"      RCMP Boot Options Bit Mask Byte 1 .................. 0x%02x\n",
    AsfRmcp->RMCPBootOptions[0]
    );
  Print (
    L"      RCMP Boot Options Bit Mask Byte 2 .................. 0x%02x\n",
    AsfRmcp->RMCPBootOptions[1]
    );
  Print (
    L"      RCMP OEM Parameter Byte 1 .......................... 0x%02x\n",
    AsfRmcp->RMCPOEMParameters[0]
    );
  Print (
    L"      RCMP OEM Parameter Byte 2 .......................... 0x%02x\n",
    AsfRmcp->RMCPOEMParameters[1]
    );
   
  Print (         
    L"  ***************************************************************************\n\n"
    );
  return;
}

VOID
DumpAsfAddr (
  EFI_ACPI_ASF_ADDR                            *AsfAddr
  )
{
  UINT8                               *SMBusAddressEntry;
  UINTN                               Index;
  
  if (AsfAddr == NULL) {
    return;
  }
  
  Print (         
    L"  ***************************************************************************\n"
    L"  *       ASF_ADDR                                                          *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  ASF_ADDR address ....................................... 0x%016lx\n" :
    L"  ASF_ADDR address ....................................... 0x%08x\n",
    AsfAddr
    );     
  Print (
    L"      Type ............................................... 0x%02x\n",
    AsfAddr->RecordHeader.Type
    ); 
  Print (
    L"      Record Length ...................................... 0x%02x\n",
    AsfAddr->RecordHeader.RecordLength
    ); 
  Print (
    L"      SEEPROM Address .................................... 0x%02x\n",
    AsfAddr->SEEPROMAddress
    );    
  Print (
    L"      Number Of Devices .................................. 0x%02x\n",
    AsfAddr->NumberOfDevices
    ); 
  Print (
    L"      Fixed SMBus Address\n"
    ); 
  SMBusAddressEntry = (UINT8*)(AsfAddr + 1);
  
  for (Index = 0; Index < AsfAddr->NumberOfDevices; Index ++) {
    Print (
      L"        Device %d ......................................... 0x%02x\n",
      Index,
      SMBusAddressEntry[Index]
      ); 
  }    

  Print (         
    L"  ***************************************************************************\n\n"
    );
  return;
}

VOID
EFIAPI
DumpAcpiASFT (
  VOID  *Table
  )
{
  ASF_TABLE                 *Asft;
  UINTN                     RecordLength;
  EFI_ACPI_ASF_RECORD_HEADER         *AsfRecordEntry;
  EFI_ACPI_ASF_INFO                  *AsfInfo;  
  EFI_ACPI_ASF_ALRT                 *AsfAlert;   
  EFI_ACPI_ASF_RCTL                  *AsfRctl;   
  EFI_ACPI_ASF_RMCP                  *AsfRmcp;         
  EFI_ACPI_ASF_ADDR                  *AsfAddr;
  BOOLEAN                   IsLastRecord; 
  
  Asft = Table;
  if (Asft == NULL) {
    return;
  }
  RecordLength = 0;
  //
  // Dump ASF table
  //
  Print (
    L"*****************************************************************************\n"
    L"*         Alert Standard Format Table                                       *\n"
    L"*****************************************************************************\n"
    );

  if (GetAcpiDumpPropertyDumpData()) {
    DumpAcpiHex (Asft->Header.Length, Asft);
  }

  if (!GetAcpiDumpPropertyDumpVerb()) {
    goto Done;
  }

  Print (         
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"ASF! address ............................................. 0x%016lx\n" :
    L"ASF! address ............................................. 0x%08x\n",
    Asft
    );
  
  DumpAcpiTableHeader (&(Asft->Header));
  
  Print (         
    L"  Table Contents:\n"
    );
  
  AsfRecordEntry = (EFI_ACPI_ASF_RECORD_HEADER*)(Asft + 1);
  
  IsLastRecord = FALSE;
  
  while (!IsLastRecord) {
    RecordLength = AsfRecordEntry->RecordLength;
    switch (AsfRecordEntry->Type & 0x7F) {
    case 0x0:
      //
      //EFI_ACPI_ASF_INFO
      //
      AsfInfo = (EFI_ACPI_ASF_INFO*)((UINTN)(AsfRecordEntry));
      DumpAsfInfo (AsfInfo);
      break;
    case 0x1:
      //
      //EFI_ACPI_ASF_ALRT
      //
      AsfAlert = (EFI_ACPI_ASF_ALRT*)((UINTN)(AsfRecordEntry));
      DumpAsfAlert (AsfAlert);
      break;
    case 0x2:
      //
      //ASF_RTCL
      //
      AsfRctl = (EFI_ACPI_ASF_RCTL*)((UINTN)(AsfRecordEntry));
      DumpAsfRctl (AsfRctl);
      break;
    case 0x3:
      //
      //EFI_ACPI_ASF_RMCP
      //
      AsfRmcp = (EFI_ACPI_ASF_RMCP*)((UINTN)(AsfRecordEntry));
      DumpAsfRmcp (AsfRmcp);
      break;
    case 0x4:
      //
      //EFI_ACPI_ASF_ADDR
      //
      AsfAddr = (EFI_ACPI_ASF_ADDR*)((UINTN)(AsfRecordEntry));
      DumpAsfAddr (AsfAddr);
      break;
    default:
      break;
    }
    
    //
    // Is this the last record?
    //
    if ((AsfRecordEntry->Type & 0x80) == 0x80) {
      IsLastRecord = TRUE;
    }
    
    //
    //Move to next record
    //
    AsfRecordEntry = (EFI_ACPI_ASF_RECORD_HEADER *)((UINT8 *)AsfRecordEntry + RecordLength);
  }

Done:
  Print (
    L"*****************************************************************************\n\n"
    );
  
  return;
}

EFI_STATUS
EFIAPI
DumpAcpiASFLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return RegisterDumpAcpiTable (EFI_ACPI_ASF_DESCRIPTION_TABLE_SIGNATURE, DumpAcpiASFT);
}
