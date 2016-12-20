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
EFIAPI
DumpAcpiFADT (
  VOID  *Table
  )
{
  EFI_ACPI_5_1_FIXED_ACPI_DESCRIPTION_TABLE                            *Fadt;
  
  Fadt = Table;
  if (Fadt == NULL) {
    return;
  }
  
  //
  // Dump Fadt table
  //
  Print (
    L"*****************************************************************************\n"
    L"*         Fixed ACPI Description Table                                      *\n"
    L"*****************************************************************************\n"
    );
    
  if (GetAcpiDumpPropertyDumpData()) {
    DumpAcpiHex (Fadt->Header.Length, Fadt);
  }
  
  if (!GetAcpiDumpPropertyDumpVerb()) {
    goto Done;
  }

  Print (
    (sizeof(UINTN) == sizeof(UINT64)) ?
    L"FADT address ............................................. 0x%016lx\n" :
    L"FADT address ............................................. 0x%08x\n",
    Fadt
    );
  
  DumpAcpiTableHeader(&(Fadt->Header));
  
  Print (
    L"  Table Contents:\n"
    );
  Print (
    L"    FACS Address ......................................... 0x%08x\n",
    Fadt->FirmwareCtrl
    );
  Print (
    L"    DSDT Address ......................................... 0x%08x\n",
    Fadt->Dsdt
    );
  if (Fadt->Header.Revision < EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE_REVISION) {
    Print (
      L"    INT_MODEL ............................................ 0x%02x\n",
      ((EFI_ACPI_1_0_FIXED_ACPI_DESCRIPTION_TABLE *)Fadt)->IntModel
      );
    switch (((EFI_ACPI_1_0_FIXED_ACPI_DESCRIPTION_TABLE *)Fadt)->IntModel) {
    case EFI_ACPI_1_0_INT_MODE_DUAL_PIC:
      Print (
        L"      Dual PIC\n"
        );
      break;
    case EFI_ACPI_1_0_INT_MODE_MULTIPLE_APIC:
      Print (
        L"      Multiple APIC\n"
        );
      break;
    default:
      break;
    }
  } else {
    Print (
      L"    PM_Profile ........................................... 0x%02x\n",
      Fadt->PreferredPmProfile
      );
    switch (Fadt->PreferredPmProfile) {
    case EFI_ACPI_2_0_PM_PROFILE_UNSPECIFIED:
      Print (
        L"      Unspecified\n"
        );
      break;
    case EFI_ACPI_2_0_PM_PROFILE_DESKTOP:
      Print (
        L"      Desktop\n"
        );
      break;
    case EFI_ACPI_2_0_PM_PROFILE_MOBILE:
      Print (
        L"      Mobile\n"
        );
      break;
    case EFI_ACPI_2_0_PM_PROFILE_WORKSTATION:
      Print (
        L"      Workstation\n"
        );
      break;
    case EFI_ACPI_2_0_PM_PROFILE_ENTERPRISE_SERVER:
      Print (
        L"      Enterprise Server\n"
        );
      break;
    case EFI_ACPI_2_0_PM_PROFILE_SOHO_SERVER:
      Print (
        L"      SOHO Server\n"
        );
      break;
    case EFI_ACPI_2_0_PM_PROFILE_APPLIANCE_PC:
      Print (
        L"      Appliance PC\n"
        );
      break;
    case EFI_ACPI_3_0_PM_PROFILE_PERFORMANCE_SERVER:
      Print (
        L"      Performance Server\n"
        );
      break;
    case EFI_ACPI_5_0_PM_PROFILE_TABLET:
      Print (
        L"      Tablet\n"
        );
      break;
    default:
      break;
    }
  }
  Print (
    L"    SCI .................................................. 0x%04x\n",
    Fadt->SciInt
    );
  Print (
    L"    SMI Command Port ..................................... 0x%08x\n",
    Fadt->SmiCmd
    );
  Print (
    L"    ACPI Enable Command .................................. 0x%02x\n",
    Fadt->AcpiEnable
    );
  Print (
    L"    ACPI Disable Command ................................. 0x%02x\n",
    Fadt->AcpiDisable
    );
  Print (
    L"    S4BIOS Request Command ............................... 0x%02x\n",
    Fadt->S4BiosReq
    );
  if (Fadt->Header.Revision >= EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE_REVISION) {
    Print (
      L"    Perf State Control Command ........................... 0x%02x\n",
      Fadt->PstateCnt
      );
  }
  Print (
    L"    PM1a Event Address ................................... 0x%08x\n",
    Fadt->Pm1aEvtBlk
    );
  Print (
    L"    PM1b Event Address ................................... 0x%08x\n",
    Fadt->Pm1bEvtBlk
    );
  Print (
    L"    PM1a Control Address ................................. 0x%08x\n",
    Fadt->Pm1aCntBlk
    );
  Print (
    L"    PM1b Control Address ................................. 0x%08x\n",
    Fadt->Pm1bCntBlk
    );
  Print (
    L"    PM2 Control Address .................................. 0x%08x\n",
    Fadt->Pm2CntBlk
    );
  Print (
    L"    PM Timer Address ..................................... 0x%08x\n",
    Fadt->PmTmrBlk
    );
  Print (
    L"    GPE0 Address ......................................... 0x%08x\n",
    Fadt->Gpe0Blk
    );
  Print (
    L"    GPE1 Address ......................................... 0x%08x\n",
    Fadt->Gpe1Blk
    );
  Print (
    L"    PM1 Event Length ..................................... 0x%02x\n",
    Fadt->Pm1EvtLen
    );
  Print (
    L"    PM1 Control Length ................................... 0x%02x\n",
    Fadt->Pm1CntLen
    );
  Print (
    L"    PM2 Control Length ................................... 0x%02x\n",
    Fadt->Pm2CntLen
    );
  Print (
    L"    PM Timer Length ...................................... 0x%02x\n",
    Fadt->PmTmrLen
    );
  Print (
    L"    GPE0 Length .......................................... 0x%02x\n",
    Fadt->Gpe0BlkLen
    );
  Print (
    L"    GPE1 Length .......................................... 0x%02x\n",
    Fadt->Gpe1BlkLen
    );
  Print (
    L"    GPE1 Base ............................................ 0x%02x\n",
    Fadt->Gpe1Base
    );
  if (Fadt->Header.Revision >= EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE_REVISION) {
    Print (
      L"    C State Control Command .............................. 0x%02x\n",
      Fadt->CstCnt
      );
  }
  Print (
    L"    C2 Hardware Latency .................................. 0x%04x\n",
    Fadt->PLvl2Lat
    );
  Print (
    L"    C3 Hardware Latency .................................. 0x%04x\n",
    Fadt->PLvl3Lat
    );
  Print (
    L"    Cache Flush Size ..................................... 0x%04x\n",
    Fadt->FlushSize
    );
  Print (
    L"    Cache Flush Stride ................................... 0x%04x\n",
    Fadt->FlushStride
    );
  Print (
    L"    Duty Cycle Bit Offset ................................ 0x%02x\n",
    Fadt->DutyOffset
    );
  Print (
    L"    Duty Cycle Bit Width ................................. 0x%02x\n",
    Fadt->DutyWidth
    );
  Print (
    L"    Day Alarm Index ...................................... 0x%02x\n",
    Fadt->DayAlrm
    );
  Print (
    L"    Month Alarm Index .................................... 0x%02x\n",
    Fadt->MonAlrm
    );
  Print (
    L"    Century Alarm Index .................................. 0x%02x\n",
    Fadt->Century
    );
  if (Fadt->Header.Revision >= EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE_REVISION) {
    Print (
      L"    IAPC Boot Flag ....................................... 0x%04x\n",
      Fadt->IaPcBootArch
      );
    Print (
      L"      LEGACY_DEVICES ..................................... 0x%04x\n",
      Fadt->IaPcBootArch & EFI_ACPI_2_0_LEGACY_DEVICES
      );
    Print (
      L"      8042 ............................................... 0x%04x\n",
      Fadt->IaPcBootArch & EFI_ACPI_2_0_8042
      );
    if (Fadt->Header.Revision >= EFI_ACPI_3_0_FIXED_ACPI_DESCRIPTION_TABLE_REVISION) {
      Print (
        L"      VGA Not Present .................................... 0x%04x\n",
        Fadt->IaPcBootArch & EFI_ACPI_3_0_VGA_NOT_PRESENT
        );
      Print (
        L"      MSI Not Present .................................... 0x%04x\n",
        Fadt->IaPcBootArch & EFI_ACPI_3_0_MSI_NOT_SUPPORTED
        );
      Print (
        L"      PCIe ASPM Controls ................................. 0x%04x\n",
        Fadt->IaPcBootArch & EFI_ACPI_3_0_PCIE_ASPM_CONTROLS
        );
      if (Fadt->Header.Revision >= EFI_ACPI_5_0_FIXED_ACPI_DESCRIPTION_TABLE_REVISION) {
        Print (
          L"      CMOS RTC Not Present ............................... 0x%04x\n",
          Fadt->IaPcBootArch & EFI_ACPI_5_0_CMOS_RTC_NOT_PRESENT
          );
      }
    }
  }
  Print (
    L"    Flags ................................................ 0x%08x\n",
    Fadt->Flags
    );
  Print (
    L"      WBINVD ............................................. 0x%08x\n",
    Fadt->Flags & EFI_ACPI_1_0_WBINVD
    );
  Print (
    L"      WBINVD_FLUSH ....................................... 0x%08x\n",
    Fadt->Flags & EFI_ACPI_1_0_WBINVD_FLUSH
    );
  Print (
    L"      PROC_C1 ............................................ 0x%08x\n",
    Fadt->Flags & EFI_ACPI_1_0_PROC_C1
    );
  Print (
    L"      P_LVL2_UP .......................................... 0x%08x\n",
    Fadt->Flags & EFI_ACPI_1_0_P_LVL2_UP
    );
  Print (
    L"      PWR_BUTTON ......................................... 0x%08x\n",
    Fadt->Flags & EFI_ACPI_1_0_PWR_BUTTON
    );
  Print (
    L"      SLP_BUTTON ......................................... 0x%08x\n",
    Fadt->Flags & EFI_ACPI_1_0_SLP_BUTTON
    );
  Print (
    L"      FIX_RTC ............................................ 0x%08x\n",
    Fadt->Flags & EFI_ACPI_1_0_FIX_RTC
    );
  Print (
    L"      RTC_S4 ............................................. 0x%08x\n",
    Fadt->Flags & EFI_ACPI_1_0_RTC_S4
    );
  Print (
    L"      TMR_VAL_EXT ........................................ 0x%08x\n",
    Fadt->Flags & EFI_ACPI_1_0_TMR_VAL_EXT
    );
  Print (
    L"      DCK_CAP ............................................ 0x%08x\n",
    Fadt->Flags & EFI_ACPI_1_0_DCK_CAP
    );
  
  if (Fadt->Header.Revision >= EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE_REVISION) {
    Print (
      L"      RESET_REG_SUP ...................................... 0x%08x\n",
      Fadt->Flags & EFI_ACPI_2_0_RESET_REG_SUP
      );
    Print (
      L"      SEALED_CASE ........................................ 0x%08x\n",
      Fadt->Flags & EFI_ACPI_2_0_SEALED_CASE
      );
    Print (
      L"      HEADLESS ........................................... 0x%08x\n",
      Fadt->Flags & EFI_ACPI_2_0_HEADLESS
      );
    Print (
      L"      CPU_SW_SLP ......................................... 0x%08x\n",
      Fadt->Flags & EFI_ACPI_2_0_CPU_SW_SLP
      );
    if (Fadt->Header.Revision >= EFI_ACPI_3_0_FIXED_ACPI_DESCRIPTION_TABLE_REVISION) {
      Print (
        L"      PCI_EXP_WAK ........................................ 0x%08x\n",
        Fadt->Flags & EFI_ACPI_3_0_PCI_EXP_WAK
        );
      Print (
        L"      USE_PLATFORM_CLOCK ................................. 0x%08x\n",
        Fadt->Flags & EFI_ACPI_3_0_USE_PLATFORM_CLOCK
        );
      Print (
        L"      S4_RTC_STS_VALID ................................... 0x%08x\n",
        Fadt->Flags & EFI_ACPI_3_0_S4_RTC_STS_VALID
        );
      Print (
        L"      REMOTE_POWER_ON_CAPABLE ............................ 0x%08x\n",
        Fadt->Flags & EFI_ACPI_3_0_REMOTE_POWER_ON_CAPABLE
        );
      Print (
        L"      FORCE_APIC_CLUSTER_MODEL ........................... 0x%08x\n",
        Fadt->Flags & EFI_ACPI_3_0_FORCE_APIC_CLUSTER_MODEL
        );
      Print (
        L"      FORCE_APIC_PHYSICAL_DESTINATION_MODE ............... 0x%08x\n",
        Fadt->Flags & EFI_ACPI_3_0_FORCE_APIC_PHYSICAL_DESTINATION_MODE
        );
      if (Fadt->Header.Revision >= EFI_ACPI_5_0_FIXED_ACPI_DESCRIPTION_TABLE_REVISION) {
        Print (
          L"      HW_REDUCED_ACPI .................................... 0x%08x\n",
          Fadt->Flags & EFI_ACPI_5_0_HW_REDUCED_ACPI
          );
        Print (
          L"      LOW_POWER_S0_IDLE_CAPABLE .......................... 0x%08x\n",
          Fadt->Flags & EFI_ACPI_5_0_LOW_POWER_S0_IDLE_CAPABLE
          );
      }
    }
  }

  if (Fadt->Header.Revision >= EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE_REVISION) {
    Print (
      L"    Reset Register Address \n"
      );
    DumpAcpiGAddressStructure ((EFI_ACPI_3_0_GENERIC_ADDRESS_STRUCTURE *)&(Fadt->ResetReg));
    Print (
      L"    Reset Register Value ................................. 0x%02x\n",
      Fadt->ResetValue
      );
    if (Fadt->Header.Revision >= EFI_ACPI_5_0_FIXED_ACPI_DESCRIPTION_TABLE_REVISION) {
      if (Fadt->MinorVersion >= EFI_ACPI_5_1_FIXED_ACPI_DESCRIPTION_TABLE_MINOR_REVISION) {
        Print (
          L"    ARM Boot Flag ........................................ 0x%02x\n",
          Fadt->ArmBootArch
          );
        Print (
          L"      PSCI_COMPLIANT ..................................... 0x%08x\n",
          Fadt->ArmBootArch & EFI_ACPI_5_1_ARM_PSCI_COMPLIANT
          );
        Print (
          L"      PSCI_USE_HVC ....................................... 0x%08x\n",
          Fadt->ArmBootArch & EFI_ACPI_5_1_ARM_PSCI_USE_HVC
          );
      }
      Print (
        L"    Minor Version ........................................ 0x%02x\n",
        Fadt->MinorVersion
        );
    }
    Print (
      L"    Extended FACS Address ................................ 0x%016lx\n",
      Fadt->XFirmwareCtrl
      );
    Print (
      L"    Extended DSDT Address ................................ 0x%016lx\n",
      Fadt->XDsdt
      );
    Print (
      L"    Extended PM1a Event Address \n"
      );
    DumpAcpiGAddressStructure ((EFI_ACPI_3_0_GENERIC_ADDRESS_STRUCTURE *)&(Fadt->XPm1aEvtBlk));
    Print (
      L"    Extended PM1b Event Address \n"
      );
    DumpAcpiGAddressStructure ((EFI_ACPI_3_0_GENERIC_ADDRESS_STRUCTURE *)&(Fadt->XPm1bEvtBlk));
    Print (
      L"    Extended PM1a Control Address \n"
      );
    DumpAcpiGAddressStructure ((EFI_ACPI_3_0_GENERIC_ADDRESS_STRUCTURE *)&(Fadt->XPm1aCntBlk));
    Print (
      L"    Extended PM1b Control Address \n"
      );
    DumpAcpiGAddressStructure ((EFI_ACPI_3_0_GENERIC_ADDRESS_STRUCTURE *)&(Fadt->XPm1bCntBlk));
    Print (
      L"    Extended PM2 Control Address \n"
      );
    DumpAcpiGAddressStructure ((EFI_ACPI_3_0_GENERIC_ADDRESS_STRUCTURE *)&(Fadt->XPm2CntBlk));
    Print (
      L"    Extended PM Timer Address \n"
      );
    DumpAcpiGAddressStructure ((EFI_ACPI_3_0_GENERIC_ADDRESS_STRUCTURE *)&(Fadt->XPmTmrBlk));
    Print (
      L"    Extended GPE0 Address \n"
      );
    DumpAcpiGAddressStructure ((EFI_ACPI_3_0_GENERIC_ADDRESS_STRUCTURE *)&(Fadt->XGpe0Blk));
    Print (
      L"    Extended GPE1 Address \n"
      );
    DumpAcpiGAddressStructure ((EFI_ACPI_3_0_GENERIC_ADDRESS_STRUCTURE *)&(Fadt->XGpe1Blk));
    if (Fadt->Header.Revision >= EFI_ACPI_5_0_FIXED_ACPI_DESCRIPTION_TABLE_REVISION) {
      Print (
        L"    Sleep Control Reg \n"
        );
      DumpAcpiGAddressStructure ((EFI_ACPI_3_0_GENERIC_ADDRESS_STRUCTURE *)&(Fadt->SleepControlReg));
      Print (
        L"    Sleep Status Reg \n"
        );
      DumpAcpiGAddressStructure ((EFI_ACPI_3_0_GENERIC_ADDRESS_STRUCTURE *)&(Fadt->SleepStatusReg));
    }
  }

Done:
  Print (         
    L"*****************************************************************************\n\n"
    );
  
  return;
}

