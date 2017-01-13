## @file AcpiToolPkg.dsc
# 
# Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
# This program and the accompanying materials are licensed and made available under
# the terms and conditions of the BSD License that accompanies this distribution.
# The full text of the license may be found at
# http://opensource.org/licenses/bsd-license.php.
#
# THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
# WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
##

[Defines]
  PLATFORM_NAME                  = AcpiToolPkg
  PLATFORM_GUID                  = F7F1457C-E455-4b28-8390-41F9922BEE25
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/AcpiToolPkg
  SUPPORTED_ARCHITECTURES        = IA32|X64|AARCH64
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT

[LibraryClasses]
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  DxeServicesTableLib|MdePkg/Library/DxeServicesTableLib/DxeServicesTableLib.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  PciLib|MdePkg/Library/BasePciLibCf8/BasePciLibCf8.inf
  PciCf8Lib|MdePkg/Library/BasePciCf8Lib/BasePciCf8Lib.inf
  IoLib|MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf
  SynchronizationLib|MdePkg/Library/BaseSynchronizationLib/BaseSynchronizationLib.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  PeCoffLib|MdePkg/Library/BasePeCoffLib/BasePeCoffLib.inf
  PeCoffExtraActionLib|MdePkg/Library/BasePeCoffExtraActionLibNull/BasePeCoffExtraActionLibNull.inf
  CacheMaintenanceLib|MdePkg/Library/BaseCacheMaintenanceLib/BaseCacheMaintenanceLib.inf
  DxeServicesLib|MdePkg/Library/DxeServicesLib/DxeServicesLib.inf
  HobLib|MdePkg/Library/DxeHobLib/DxeHobLib.inf
  DumpAcpiTableFuncLib|AcpiToolPkg/Library/DumpAcpi/DumpAcpiTableFuncLib/DumpAcpiTableFuncLib.inf

  UefiDriverEntryPoint|MdePkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.inf
  DebugLib|IntelFrameworkModulePkg/Library/PeiDxeDebugLibReportStatusCode/PeiDxeDebugLibReportStatusCode.inf
  ReportStatusCodeLib|MdeModulePkg/Library/DxeReportStatusCodeLib/DxeReportStatusCodeLib.inf
  DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
  
[LibraryClasses.common.DXE_SMM_DRIVER]
  HobLib|MdePkg/Library/DxeHobLib/DxeHobLib.inf
  MemoryAllocationLib|MdePkg/Library/SmmMemoryAllocationLib/SmmMemoryAllocationLib.inf
  SmmServicesTableLib|MdePkg/Library/SmmServicesTableLib/SmmServicesTableLib.inf
  SmmMemLib|MdePkg/Library/SmmMemLib/SmmMemLib.inf

[LibraryClasses.common.UEFI_APPLICATION]
  UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
  ShellLib|ShellPkg/Library/UefiShellLib/UefiShellLib.inf
  FileHandleLib|MdePkg/Library/UefiFileHandleLib/UefiFileHandleLib.inf
  ShellCEntryLib|ShellPkg/Library/UefiShellCEntryLib/UefiShellCEntryLib.inf
  UefiHiiServicesLib|MdeModulePkg/Library/UefiHiiServicesLib/UefiHiiServicesLib.inf
  HiiLib|MdeModulePkg/Library/UefiHiiLib/UefiHiiLib.inf
  DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
  SortLib|MdeModulePkg/Library/UefiSortLib/UefiSortLib.inf

[PcdsFixedAtBuild.common]
  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x1f
  gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x80080046
  gEfiMdePkgTokenSpaceGuid.PcdReportStatusCodePropertyMask|0x07

[Components]
  AcpiToolPkg/DumpAcpi/DumpACPI.inf {
    <LibraryClasses>
# ACPI Defined
      NULL|AcpiToolPkg/Library/DumpAcpi/DumpAcpiBERTLib/DumpAcpiBERTLib.inf  #TBD
      NULL|AcpiToolPkg/Library/DumpAcpi/DumpAcpiBGRTLib/DumpAcpiBGRTLib.inf
      NULL|AcpiToolPkg/Library/DumpAcpi/DumpAcpiCPEPLib/DumpAcpiCPEPLib.inf
      NULL|AcpiToolPkg/Library/DumpAcpi/DumpAcpiECDTLib/DumpAcpiECDTLib.inf
      NULL|AcpiToolPkg/Library/DumpAcpi/DumpAcpiEINJLib/DumpAcpiEINJLib.inf
      NULL|AcpiToolPkg/Library/DumpAcpi/DumpAcpiERSTLib/DumpAcpiERSTLib.inf
      NULL|AcpiToolPkg/Library/DumpAcpi/DumpAcpiFPDTLib/DumpAcpiFPDTLib.inf
      NULL|AcpiToolPkg/Library/DumpAcpi/DumpAcpiGTDTLib/DumpAcpiGTDTLib.inf
      NULL|AcpiToolPkg/Library/DumpAcpi/DumpAcpiHESTLib/DumpAcpiHESTLib.inf
      NULL|AcpiToolPkg/Library/DumpAcpi/DumpAcpiMADTLib/DumpAcpiMADTLib.inf
      NULL|AcpiToolPkg/Library/DumpAcpi/DumpAcpiMPSTLib/DumpAcpiMPSTLib.inf
      NULL|AcpiToolPkg/Library/DumpAcpi/DumpAcpiMSCTLib/DumpAcpiMSCTLib.inf
      NULL|AcpiToolPkg/Library/DumpAcpi/DumpAcpiNFITLib/DumpAcpiNFITLib.inf
      NULL|AcpiToolPkg/Library/DumpAcpi/DumpAcpiPMTTLib/DumpAcpiPMTTLib.inf
      NULL|AcpiToolPkg/Library/DumpAcpi/DumpAcpiRASFLib/DumpAcpiRASFLib.inf
      NULL|AcpiToolPkg/Library/DumpAcpi/DumpAcpiSBSTLib/DumpAcpiSBSTLib.inf
      NULL|AcpiToolPkg/Library/DumpAcpi/DumpAcpiSRATLib/DumpAcpiSRATLib.inf
      NULL|AcpiToolPkg/Library/DumpAcpi/DumpAcpiSLITLib/DumpAcpiSLITLib.inf
# ACPI Reserved
      NULL|AcpiToolPkg/Library/DumpAcpi/DumpAcpiBOOTLib/DumpAcpiBOOTLib.inf  #TBD
      NULL|AcpiToolPkg/Library/DumpAcpi/DumpAcpiCSRTLib/DumpAcpiCSRTLib.inf  #TBD
      NULL|AcpiToolPkg/Library/DumpAcpi/DumpAcpiDBGPLib/DumpAcpiDBGPLib.inf
      NULL|AcpiToolPkg/Library/DumpAcpi/DumpAcpiDBG2Lib/DumpAcpiDBG2Lib.inf
      NULL|AcpiToolPkg/Library/DumpAcpi/DumpAcpiDMARLib/DumpAcpiDMARLib.inf
      NULL|AcpiToolPkg/Library/DumpAcpi/DumpAcpiETDTLib/DumpAcpiETDTLib.inf
      NULL|AcpiToolPkg/Library/DumpAcpi/DumpAcpiHPETLib/DumpAcpiHPETLib.inf
      NULL|AcpiToolPkg/Library/DumpAcpi/DumpAcpiIBFTLib/DumpAcpiIBFTLib.inf
      NULL|AcpiToolPkg/Library/DumpAcpi/DumpAcpiIVRSLib/DumpAcpiIVRSLib.inf  #TBD
      NULL|AcpiToolPkg/Library/DumpAcpi/DumpAcpiLPITLib/DumpAcpiLPITLib.inf
      NULL|AcpiToolPkg/Library/DumpAcpi/DumpAcpiMCFGLib/DumpAcpiMCFGLib.inf
      NULL|AcpiToolPkg/Library/DumpAcpi/DumpAcpiMCHILib/DumpAcpiMCHILib.inf  #TBD
      NULL|AcpiToolPkg/Library/DumpAcpi/DumpAcpiMSDMLib/DumpAcpiMSDMLib.inf
      NULL|AcpiToolPkg/Library/DumpAcpi/DumpAcpiSLICLib/DumpAcpiSLICLib.inf
      NULL|AcpiToolPkg/Library/DumpAcpi/DumpAcpiSPCRLib/DumpAcpiSPCRLib.inf
      NULL|AcpiToolPkg/Library/DumpAcpi/DumpAcpiSPMILib/DumpAcpiSPMILib.inf
      NULL|AcpiToolPkg/Library/DumpAcpi/DumpAcpiTCPALib/DumpAcpiTCPALib.inf
      NULL|AcpiToolPkg/Library/DumpAcpi/DumpAcpiTPM2Lib/DumpAcpiTPM2Lib.inf
      NULL|AcpiToolPkg/Library/DumpAcpi/DumpAcpiUEFILib/DumpAcpiUEFILib.inf
      NULL|AcpiToolPkg/Library/DumpAcpi/DumpAcpiWAETLib/DumpAcpiWAETLib.inf  #TBD
      NULL|AcpiToolPkg/Library/DumpAcpi/DumpAcpiWDATLib/DumpAcpiWDATLib.inf
      NULL|AcpiToolPkg/Library/DumpAcpi/DumpAcpiWDRTLib/DumpAcpiWDRTLib.inf
      NULL|AcpiToolPkg/Library/DumpAcpi/DumpAcpiWPBTLib/DumpAcpiWPBTLib.inf  #TBD
      NULL|AcpiToolPkg/Library/DumpAcpi/DumpAcpiWSPTLib/DumpAcpiWSPTLib.inf
#Other defined
      NULL|AcpiToolPkg/Library/DumpAcpi/DumpAcpiASFLib/DumpAcpiASFLib.inf
      NULL|AcpiToolPkg/Library/DumpAcpi/DumpAcpiWDDTLib/DumpAcpiWDDTLib.inf
  }

