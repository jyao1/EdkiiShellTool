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

#pragma pack(1)

typedef struct {
  UINT16  Type;
} EFI_ACPI_HEST_ERROR_SOURCE_STRUCT_HEADER;

#pragma pack()

VOID
DumpHestMachineCheckErrorBank (
  EFI_ACPI_4_0_IA32_ARCHITECTURE_MACHINE_CHECK_ERROR_BANK_STRUCTURE   *MachineCheckErrorBank
  )
{
  if (MachineCheckErrorBank == NULL) {
    return ;
  }

  Print (         
    L"    *************************************************************************\n"
    L"    *       Machine Check Error Bank                                        *\n"
    L"    *************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"    Machine Check Error Bank address ..................... 0x%016lx\n" :
    L"    Machine Check Error Bank address ..................... 0x%08x\n",
    MachineCheckErrorBank
    );
  Print (
    L"      Bank Number ........................................ 0x%02x\n",
    MachineCheckErrorBank->BankNumber
    );
  Print (
    L"      Clear Status On Initialization ..................... 0x%02x\n",
    MachineCheckErrorBank->ClearStatusOnInitialization
    );
  Print (
    L"      Status Data Format ................................. 0x%02x\n",
    MachineCheckErrorBank->StatusDataFormat
    );
  switch (MachineCheckErrorBank->StatusDataFormat) {
  case EFI_ACPI_4_0_IA32_ARCHITECTURE_MACHINE_CHECK_ERROR_DATA_FORMAT_IA32:
    Print (
      L"        IA32\n"
      );
    break;
  case EFI_ACPI_4_0_IA32_ARCHITECTURE_MACHINE_CHECK_ERROR_DATA_FORMAT_INTEL64:
    Print (
      L"        Intel64\n"
      );
    break;
  case EFI_ACPI_4_0_IA32_ARCHITECTURE_MACHINE_CHECK_ERROR_DATA_FORMAT_AMD64:
    Print (
      L"        AMD64\n"
      );
    break;
  default:
    break;
  }
  Print (
    L"      Control Register Msr Address ....................... 0x%04x\n",
    MachineCheckErrorBank->ControlRegisterMsrAddress
    );
  Print (
    L"      Control Init Data .................................. 0x%016lx\n",
    MachineCheckErrorBank->ControlInitData
    );
  Print (
    L"      Status Register Msr Address ........................ 0x%08x\n",
    MachineCheckErrorBank->StatusRegisterMsrAddress
    );
  Print (
    L"      Address Register Msr Address ....................... 0x%08x\n",
    MachineCheckErrorBank->AddressRegisterMsrAddress
    );
  Print (
    L"      Misc Register Msr Address .......................... 0x%08x\n",
    MachineCheckErrorBank->MiscRegisterMsrAddress
    );
 
  Print (       
    L"    *************************************************************************\n\n"
    );

  return;
}

VOID
DumpHestHardwareNotificationStructure (
  EFI_ACPI_4_0_HARDWARE_ERROR_NOTIFICATION_STRUCTURE     *NotificationStructure
  )
{
  if (NotificationStructure == NULL) {
    return ;
  }

  Print (         
    L"    *************************************************************************\n"
    L"    *       Hardware Notification Structure                                 *\n"
    L"    *************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"    Hardware Notification Structure address .............. 0x%016lx\n" :
    L"    Hardware Notification Structure address .............. 0x%08x\n",
    NotificationStructure
    );
  Print (
    L"      Type ............................................... 0x%02x\n",
    NotificationStructure->Type
    );
  switch (NotificationStructure->Type) {
  case EFI_ACPI_4_0_HARDWARE_ERROR_NOTIFICATION_POLLED:
    Print (
      L"        Polled\n"
      );
    break;
  case EFI_ACPI_4_0_HARDWARE_ERROR_NOTIFICATION_EXTERNAL_INTERRUPT:
    Print (
      L"        External Interrupt\n"
      );
    break;
  case EFI_ACPI_4_0_HARDWARE_ERROR_NOTIFICATION_LOCAL_INTERRUPT:
    Print (
      L"        Local Interrupt\n"
      );
    break;
  case EFI_ACPI_4_0_HARDWARE_ERROR_NOTIFICATION_SCI:
    Print (
      L"        SCI\n"
      );
    break;
  case EFI_ACPI_4_0_HARDWARE_ERROR_NOTIFICATION_NMI:
    Print (
      L"        NMI\n"
      );
    break;
  default:
    break;
  }
  Print (
    L"      Length ............................................. 0x%02x\n",
    NotificationStructure->Length
    );
  Print (
    L"      Configuration Write Enable ......................... 0x%04x\n",
    *(UINT16 *)&NotificationStructure->ConfigurationWriteEnable
    );
  Print (
    L"        Type ............................................. 0x%04x\n",
    NotificationStructure->ConfigurationWriteEnable.Type
    );
  Print (
    L"        Poll Interval .................................... 0x%04x\n",
    NotificationStructure->ConfigurationWriteEnable.PollInterval
    );
  Print (
    L"        Switch To Polling Threshold Value ................ 0x%04x\n",
    NotificationStructure->ConfigurationWriteEnable.SwitchToPollingThresholdValue
    );
  Print (
    L"        Switch To Polling Threshold Window ............... 0x%04x\n",
    NotificationStructure->ConfigurationWriteEnable.SwitchToPollingThresholdWindow
    );
  Print (
    L"        Error Threshold Value ............................ 0x%04x\n",
    NotificationStructure->ConfigurationWriteEnable.ErrorThresholdValue
    );
  Print (
    L"        Error Threshold Window ........................... 0x%04x\n",
    NotificationStructure->ConfigurationWriteEnable.ErrorThresholdWindow
    );
  Print (
    L"      Poll Interval ...................................... 0x%08x\n",
    NotificationStructure->PollInterval
    );
  Print (
    L"      Vector ............................................. 0x%08x\n",
    NotificationStructure->Vector
    );
  Print (
    L"      Switch To Polling Threshold Value .................. 0x%08x\n",
    NotificationStructure->SwitchToPollingThresholdValue
    );
  Print (
    L"      Switch To Polling Threshold Window ................. 0x%08x\n",
    NotificationStructure->SwitchToPollingThresholdWindow
    );
  Print (
    L"      Error Threshold Value .............................. 0x%08x\n",
    NotificationStructure->ErrorThresholdValue
    );
  Print (
    L"      Error Threshold Window ............................. 0x%08x\n",
    NotificationStructure->ErrorThresholdWindow
    );
 
  Print (       
    L"    *************************************************************************\n\n"
    );

  return;
}

VOID
DumpHestIa32MachineCheckException (
  EFI_ACPI_4_0_IA32_ARCHITECTURE_MACHINE_CHECK_EXCEPTION_STRUCTURE  *ErrorSource
  )
{
  EFI_ACPI_4_0_IA32_ARCHITECTURE_MACHINE_CHECK_ERROR_BANK_STRUCTURE   *MachineCheckErrorBank;
  UINTN                                                               Index;

  if (ErrorSource == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       IA32 Machine Check Exception Structure                            *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  IA32 Machine Check Exception Structure address ......... 0x%016lx\n" :
    L"  IA32 Machine Check Exception Structure address ......... 0x%08x\n",
    ErrorSource
    );
  Print (
    L"    Type ................................................. 0x%04x\n",
    ErrorSource->Type
    );
  Print (
    L"    Source Id ............................................ 0x%04x\n",
    ErrorSource->SourceId
    );
  Print (
    L"    Flags ................................................ 0x%02x\n",
    ErrorSource->Flags
    );
  Print (
    L"      Firmware First ..................................... 0x%02x\n",
    ErrorSource->Flags & EFI_ACPI_4_0_ERROR_SOURCE_FLAG_FIRMWARE_FIRST
    );
  Print (
    L"    Enabled .............................................. 0x%02x\n",
    ErrorSource->Enabled
    );
  Print (
    L"    Number Of Records To PreAllocate ..................... 0x%08x\n",
    ErrorSource->NumberOfRecordsToPreAllocate
    );
  Print (
    L"    Max Sections Per Record .............................. 0x%08x\n",
    ErrorSource->MaxSectionsPerRecord
    );
  Print (
    L"    Global Capability Init Data .......................... 0x%016lx\n",
    ErrorSource->GlobalCapabilityInitData
    );
  Print (
    L"    Global Control Init Data ............................. 0x%016lx\n",
    ErrorSource->GlobalControlInitData
    );
  Print (
    L"    Number Of Hardware Banks ............................. 0x%02x\n",
    ErrorSource->NumberOfHardwareBanks
    );

  MachineCheckErrorBank = (EFI_ACPI_4_0_IA32_ARCHITECTURE_MACHINE_CHECK_ERROR_BANK_STRUCTURE *)(ErrorSource + 1);
  for (Index = 0; Index < ErrorSource->NumberOfHardwareBanks; Index++) {
    DumpHestMachineCheckErrorBank (MachineCheckErrorBank);
    MachineCheckErrorBank ++;
  }

  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
DumpHestIa32CorrectedMachineCheck (
  EFI_ACPI_4_0_IA32_ARCHITECTURE_CORRECTED_MACHINE_CHECK_STRUCTURE  *ErrorSource
  )
{
  EFI_ACPI_4_0_IA32_ARCHITECTURE_MACHINE_CHECK_ERROR_BANK_STRUCTURE   *MachineCheckErrorBank;
  UINTN                                                               Index;

  if (ErrorSource == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       IA32 Corrected Machine Check Structure                            *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  IA32 Corrected Machine Check Structure address ......... 0x%016lx\n" :
    L"  IA32 Corrected Machine Check Structure address ......... 0x%08x\n",
    ErrorSource
    );
  Print (
    L"    Type ................................................. 0x%04x\n",
    ErrorSource->Type
    );
  Print (
    L"    Source Id ............................................ 0x%04x\n",
    ErrorSource->SourceId
    );
  Print (
    L"    Flags ................................................ 0x%02x\n",
    ErrorSource->Flags
    );
  Print (
    L"      Firmware First ..................................... 0x%02x\n",
    ErrorSource->Flags & EFI_ACPI_4_0_ERROR_SOURCE_FLAG_FIRMWARE_FIRST
    );
  Print (
    L"    Enabled .............................................. 0x%02x\n",
    ErrorSource->Enabled
    );
  Print (
    L"    Number Of Records To PreAllocate ..................... 0x%08x\n",
    ErrorSource->NumberOfRecordsToPreAllocate
    );
  Print (
    L"    Max Sections Per Record .............................. 0x%08x\n",
    ErrorSource->MaxSectionsPerRecord
    );

  DumpHestHardwareNotificationStructure (&ErrorSource->NotificationStructure);

  Print (
    L"    Number Of Hardware Banks ............................. 0x%02x\n",
    ErrorSource->NumberOfHardwareBanks
    );

  MachineCheckErrorBank = (EFI_ACPI_4_0_IA32_ARCHITECTURE_MACHINE_CHECK_ERROR_BANK_STRUCTURE *)(ErrorSource + 1);
  for (Index = 0; Index < ErrorSource->NumberOfHardwareBanks; Index++) {
    DumpHestMachineCheckErrorBank (MachineCheckErrorBank);
    MachineCheckErrorBank ++;
  }

  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
DumpHestIa32NmiError (
  EFI_ACPI_4_0_IA32_ARCHITECTURE_NMI_ERROR_STRUCTURE  *ErrorSource
  )
{
  if (ErrorSource == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       IA32 NMI Error Structure                                          *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  IA32 NMI Error Structure address ....................... 0x%016lx\n" :
    L"  IA32 NMI Error Structure address ....................... 0x%08x\n",
    ErrorSource
    );
  Print (
    L"    Type ................................................. 0x%04x\n",
    ErrorSource->Type
    );
  Print (
    L"    Source Id ............................................ 0x%04x\n",
    ErrorSource->SourceId
    );
  Print (
    L"    Number Of Records To PreAllocate ..................... 0x%08x\n",
    ErrorSource->NumberOfRecordsToPreAllocate
    );
  Print (
    L"    Max Sections Per Record .............................. 0x%08x\n",
    ErrorSource->MaxSectionsPerRecord
    );
  Print (
    L"    Max Raw Data Length .................................. 0x%08x\n",
    ErrorSource->MaxRawDataLength
    );

  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
DumpHestPcieRootPortAer (
  EFI_ACPI_4_0_PCI_EXPRESS_ROOT_PORT_AER_STRUCTURE  *ErrorSource
  )
{
  if (ErrorSource == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       PCI Express Root Port AER Structure                               *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  PCI Express Root Port AER Structure address ............ 0x%016lx\n" :
    L"  PCI Express Root Port AER Structure address ............ 0x%08x\n",
    ErrorSource
    );
  Print (
    L"    Type ................................................. 0x%04x\n",
    ErrorSource->Type
    );
  Print (
    L"    Source Id ............................................ 0x%04x\n",
    ErrorSource->SourceId
    );
  Print (
    L"    Flags ................................................ 0x%02x\n",
    ErrorSource->Flags
    );
  Print (
    L"      Firmware First ..................................... 0x%02x\n",
    ErrorSource->Flags & EFI_ACPI_4_0_ERROR_SOURCE_FLAG_FIRMWARE_FIRST
    );
  Print (
    L"    Enabled .............................................. 0x%02x\n",
    ErrorSource->Enabled
    );
  Print (
    L"    Number Of Records To PreAllocate ..................... 0x%08x\n",
    ErrorSource->NumberOfRecordsToPreAllocate
    );
  Print (
    L"    Max Sections Per Record .............................. 0x%08x\n",
    ErrorSource->MaxSectionsPerRecord
    );
  Print (
    L"    Bus .................................................. 0x%08x\n",
    ErrorSource->Bus
    );
  Print (
    L"    Device ............................................... 0x%04x\n",
    ErrorSource->Device
    );
  Print (
    L"    Function ............................................. 0x%04x\n",
    ErrorSource->Function
    );
  Print (
    L"    Device Control ....................................... 0x%04x\n",
    ErrorSource->DeviceControl
    );
  Print (
    L"    Uncorrectable Error Mask ............................. 0x%08x\n",
    ErrorSource->UncorrectableErrorMask
    );
  Print (
    L"    Uncorrectable Error Severity ......................... 0x%08x\n",
    ErrorSource->UncorrectableErrorSeverity
    );
  Print (
    L"    Correctable Error Mask ............................... 0x%08x\n",
    ErrorSource->CorrectableErrorMask
    );
  Print (
    L"    Advanced Error Capabilities And Control .............. 0x%08x\n",
    ErrorSource->AdvancedErrorCapabilitiesAndControl
    );
  Print (
    L"    Root Error Command ................................... 0x%08x\n",
    ErrorSource->RootErrorCommand
    );

  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
DumpHestPcieDeviceAer (
  EFI_ACPI_4_0_PCI_EXPRESS_DEVICE_AER_STRUCTURE  *ErrorSource
  )
{
  if (ErrorSource == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       PCI Express Device AER Structure                                  *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  PCI Express Device AER Structure address ............... 0x%016lx\n" :
    L"  PCI Express Device AER Structure address ............... 0x%08x\n",
    ErrorSource
    );
  Print (
    L"    Type ................................................. 0x%04x\n",
    ErrorSource->Type
    );
  Print (
    L"    Source Id ............................................ 0x%04x\n",
    ErrorSource->SourceId
    );
  Print (
    L"    Flags ................................................ 0x%02x\n",
    ErrorSource->Flags
    );
  Print (
    L"      Firmware First ..................................... 0x%02x\n",
    ErrorSource->Flags & EFI_ACPI_4_0_ERROR_SOURCE_FLAG_FIRMWARE_FIRST
    );
  Print (
    L"      Global ............................................. 0x%02x\n",
    ErrorSource->Flags & EFI_ACPI_4_0_ERROR_SOURCE_FLAG_GLOBAL
    );
  Print (
    L"    Enabled .............................................. 0x%02x\n",
    ErrorSource->Enabled
    );
  Print (
    L"    Number Of Records To PreAllocate ..................... 0x%08x\n",
    ErrorSource->NumberOfRecordsToPreAllocate
    );
  Print (
    L"    Max Sections Per Record .............................. 0x%08x\n",
    ErrorSource->MaxSectionsPerRecord
    );
  Print (
    L"    Bus .................................................. 0x%08x\n",
    ErrorSource->Bus
    );
  Print (
    L"    Device ............................................... 0x%04x\n",
    ErrorSource->Device
    );
  Print (
    L"    Function ............................................. 0x%04x\n",
    ErrorSource->Function
    );
  Print (
    L"    Device Control ....................................... 0x%04x\n",
    ErrorSource->DeviceControl
    );
  Print (
    L"    Uncorrectable Error Mask ............................. 0x%08x\n",
    ErrorSource->UncorrectableErrorMask
    );
  Print (
    L"    Uncorrectable Error Severity ......................... 0x%08x\n",
    ErrorSource->UncorrectableErrorSeverity
    );
  Print (
    L"    Correctable Error Mask ............................... 0x%08x\n",
    ErrorSource->CorrectableErrorMask
    );
  Print (
    L"    Advanced Error Capabilities And Control .............. 0x%08x\n",
    ErrorSource->AdvancedErrorCapabilitiesAndControl
    );

  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
DumpHestPcieBridgeAer (
  EFI_ACPI_4_0_PCI_EXPRESS_BRIDGE_AER_STRUCTURE  *ErrorSource
  )
{
  if (ErrorSource == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       PCI Express Bridge AER Structure                                  *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  PCI Express Bridge AER Structure address ............... 0x%016lx\n" :
    L"  PCI Express Bridge AER Structure address ............... 0x%08x\n",
    ErrorSource
    );
  Print (
    L"    Type ................................................. 0x%04x\n",
    ErrorSource->Type
    );
  Print (
    L"    Source Id ............................................ 0x%04x\n",
    ErrorSource->SourceId
    );
  Print (
    L"    Flags ................................................ 0x%02x\n",
    ErrorSource->Flags
    );
  Print (
    L"      Firmware First ..................................... 0x%02x\n",
    ErrorSource->Flags & EFI_ACPI_4_0_ERROR_SOURCE_FLAG_FIRMWARE_FIRST
    );
  Print (
    L"      Global ............................................. 0x%02x\n",
    ErrorSource->Flags & EFI_ACPI_4_0_ERROR_SOURCE_FLAG_GLOBAL
    );
  Print (
    L"    Enabled .............................................. 0x%02x\n",
    ErrorSource->Enabled
    );
  Print (
    L"    Number Of Records To PreAllocate ..................... 0x%08x\n",
    ErrorSource->NumberOfRecordsToPreAllocate
    );
  Print (
    L"    Max Sections Per Record .............................. 0x%08x\n",
    ErrorSource->MaxSectionsPerRecord
    );
  Print (
    L"    Bus .................................................. 0x%08x\n",
    ErrorSource->Bus
    );
  Print (
    L"    Device ............................................... 0x%04x\n",
    ErrorSource->Device
    );
  Print (
    L"    Function ............................................. 0x%04x\n",
    ErrorSource->Function
    );
  Print (
    L"    Device Control ....................................... 0x%04x\n",
    ErrorSource->DeviceControl
    );
  Print (
    L"    Uncorrectable Error Mask ............................. 0x%08x\n",
    ErrorSource->UncorrectableErrorMask
    );
  Print (
    L"    Uncorrectable Error Severity ......................... 0x%08x\n",
    ErrorSource->UncorrectableErrorSeverity
    );
  Print (
    L"    Correctable Error Mask ............................... 0x%08x\n",
    ErrorSource->CorrectableErrorMask
    );
  Print (
    L"    Advanced Error Capabilities And Control .............. 0x%08x\n",
    ErrorSource->AdvancedErrorCapabilitiesAndControl
    );
  Print (
    L"    Secondary Uncorrectable Error Mask ................... 0x%08x\n",
    ErrorSource->SecondaryUncorrectableErrorMask
    );
  Print (
    L"    Secondary Uncorrectable Error Severity ............... 0x%08x\n",
    ErrorSource->SecondaryUncorrectableErrorSeverity
    );
  Print (
    L"    Secondary Advanced Error Capabilities And Control .... 0x%08x\n",
    ErrorSource->SecondaryAdvancedErrorCapabilitiesAndControl
    );

  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
DumpHestGenericHardwareError (
  EFI_ACPI_4_0_GENERIC_HARDWARE_ERROR_SOURCE_STRUCTURE  *ErrorSource
  )
{
  if (ErrorSource == NULL) {
    return;
  }

  Print (         
    L"  ***************************************************************************\n"
    L"  *       Generic Hardware Error Structure                                  *\n"
    L"  ***************************************************************************\n"
    );
  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"  Generic Hardware Error Structure address ............... 0x%016lx\n" :
    L"  Generic Hardware Error Structure address ............... 0x%08x\n",
    ErrorSource
    );
  Print (
    L"    Type ................................................. 0x%04x\n",
    ErrorSource->Type
    );
  Print (
    L"    Source Id ............................................ 0x%04x\n",
    ErrorSource->SourceId
    );
  Print (
    L"    Related Source Id .................................... 0x%04x\n",
    ErrorSource->RelatedSourceId
    );
  Print (
    L"    Flags ................................................ 0x%02x\n",
    ErrorSource->Flags
    );
  Print (
    L"    Enabled .............................................. 0x%02x\n",
    ErrorSource->Enabled
    );
  Print (
    L"    Number Of Records To PreAllocate ..................... 0x%08x\n",
    ErrorSource->NumberOfRecordsToPreAllocate
    );
  Print (
    L"    Max Sections Per Record .............................. 0x%08x\n",
    ErrorSource->MaxSectionsPerRecord
    );
  Print (
    L"    Max Raw Data Length .................................. 0x%08x\n",
    ErrorSource->MaxRawDataLength
    );

  Print (
    L"    Error Status Address\n"
    );
  DumpAcpiGAddressStructure ((EFI_ACPI_3_0_GENERIC_ADDRESS_STRUCTURE *)&ErrorSource->ErrorStatusAddress);

  DumpHestHardwareNotificationStructure (&ErrorSource->NotificationStructure);

  Print (
    L"    Error Status Block Length ............................ 0x%08x\n",
    ErrorSource->ErrorStatusBlockLength
    );

  Print (       
    L"  ***************************************************************************\n\n"
    );

  return;
}

VOID
EFIAPI
DumpAcpiHEST (
  VOID  *Table
  )
{
  EFI_ACPI_4_0_HARDWARE_ERROR_SOURCE_TABLE_HEADER                            *Hest;
  EFI_ACPI_HEST_ERROR_SOURCE_STRUCT_HEADER                                   *ErrorSourceHead;
  UINTN                                                                      ErrorSourceLength;
  UINTN                                                                      Index;

  Hest = Table;
  if (Hest == NULL) {
    return;
  }
  
  //
  // Dump Hest table
  //
  Print (
    L"*****************************************************************************\n"
    L"*         Hardware Error Source Table                                       *\n"
    L"*****************************************************************************\n"
    );
    
  if (GetAcpiDumpPropertyDumpData()) {
    DumpAcpiHex (Hest->Header.Length, Hest);
  }
  
  if (!GetAcpiDumpPropertyDumpVerb()) {
    goto Done;
  }

  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"HEST address ............................................. 0x%016lx\n" :
    L"HEST address ............................................. 0x%08x\n",
    Hest
    );
  
  DumpAcpiTableHeader(&(Hest->Header));
  
  Print (
    L"  Table Contents:\n"
    );
  Print (
    L"    Error Source Count ................................... 0x%08x\n",
    Hest->ErrorSourceCount
    );

  ErrorSourceHead = (EFI_ACPI_HEST_ERROR_SOURCE_STRUCT_HEADER *)(Hest + 1);
  ErrorSourceLength = 0;
  for (Index = 0; Index < Hest->ErrorSourceCount; Index++) {
    switch (ErrorSourceHead->Type) {
    case EFI_ACPI_4_0_IA32_ARCHITECTURE_MACHINE_CHECK_EXCEPTION:
      ErrorSourceLength = sizeof(EFI_ACPI_4_0_IA32_ARCHITECTURE_MACHINE_CHECK_EXCEPTION_STRUCTURE) + 
                          ((EFI_ACPI_4_0_IA32_ARCHITECTURE_MACHINE_CHECK_EXCEPTION_STRUCTURE *)ErrorSourceHead)->NumberOfHardwareBanks * sizeof(EFI_ACPI_4_0_IA32_ARCHITECTURE_MACHINE_CHECK_ERROR_BANK_STRUCTURE);
      DumpHestIa32MachineCheckException ((EFI_ACPI_4_0_IA32_ARCHITECTURE_MACHINE_CHECK_EXCEPTION_STRUCTURE *)ErrorSourceHead);
      break;
    case EFI_ACPI_4_0_IA32_ARCHITECTURE_CORRECTED_MACHINE_CHECK:
      ErrorSourceLength = sizeof(EFI_ACPI_4_0_IA32_ARCHITECTURE_CORRECTED_MACHINE_CHECK_STRUCTURE) + 
                          ((EFI_ACPI_4_0_IA32_ARCHITECTURE_CORRECTED_MACHINE_CHECK_STRUCTURE *)ErrorSourceHead)->NumberOfHardwareBanks * sizeof(EFI_ACPI_4_0_IA32_ARCHITECTURE_MACHINE_CHECK_ERROR_BANK_STRUCTURE);
      DumpHestIa32CorrectedMachineCheck ((EFI_ACPI_4_0_IA32_ARCHITECTURE_CORRECTED_MACHINE_CHECK_STRUCTURE *)ErrorSourceHead);
      break;
    case EFI_ACPI_4_0_IA32_ARCHITECTURE_NMI_ERROR:
      ErrorSourceLength = sizeof(EFI_ACPI_4_0_IA32_ARCHITECTURE_NMI_ERROR_STRUCTURE);
      DumpHestIa32NmiError ((EFI_ACPI_4_0_IA32_ARCHITECTURE_NMI_ERROR_STRUCTURE *)ErrorSourceHead);
      break;
    case EFI_ACPI_4_0_PCI_EXPRESS_ROOT_PORT_AER:
      ErrorSourceLength = sizeof(EFI_ACPI_4_0_PCI_EXPRESS_ROOT_PORT_AER_STRUCTURE);
      DumpHestPcieRootPortAer ((EFI_ACPI_4_0_PCI_EXPRESS_ROOT_PORT_AER_STRUCTURE *)ErrorSourceHead);
      break;
    case EFI_ACPI_4_0_PCI_EXPRESS_DEVICE_AER:
      ErrorSourceLength = sizeof(EFI_ACPI_4_0_PCI_EXPRESS_DEVICE_AER_STRUCTURE);
      DumpHestPcieDeviceAer ((EFI_ACPI_4_0_PCI_EXPRESS_DEVICE_AER_STRUCTURE *)ErrorSourceHead);
      break;
    case EFI_ACPI_4_0_PCI_EXPRESS_BRIDGE_AER:
      ErrorSourceLength = sizeof(EFI_ACPI_4_0_PCI_EXPRESS_BRIDGE_AER_STRUCTURE);
      DumpHestPcieBridgeAer ((EFI_ACPI_4_0_PCI_EXPRESS_BRIDGE_AER_STRUCTURE *)ErrorSourceHead);
      break;
    case EFI_ACPI_4_0_GENERIC_HARDWARE_ERROR:
      ErrorSourceLength = sizeof(EFI_ACPI_4_0_GENERIC_HARDWARE_ERROR_SOURCE_STRUCTURE);
      DumpHestGenericHardwareError ((EFI_ACPI_4_0_GENERIC_HARDWARE_ERROR_SOURCE_STRUCTURE *)ErrorSourceHead);
      break;
    default:
      Print (
        L"      Unknown Type ....................................... 0x%02x\n",
        ErrorSourceHead->Type
        );
      goto Done;
    }
    ErrorSourceHead = (EFI_ACPI_HEST_ERROR_SOURCE_STRUCT_HEADER *)((UINTN)ErrorSourceHead + ErrorSourceLength);
  }

Done:
  Print (         
    L"*****************************************************************************\n\n"
    );
  
  return;
}

EFI_STATUS
EFIAPI
DumpAcpiHESTLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return RegisterDumpAcpiTable (EFI_ACPI_4_0_HARDWARE_ERROR_SOURCE_TABLE_SIGNATURE, DumpAcpiHEST);
}
