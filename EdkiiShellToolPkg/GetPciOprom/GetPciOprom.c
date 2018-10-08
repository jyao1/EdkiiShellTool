/** @file
  Application to get PCI OPROM.

  Copyright (c) 2018, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Uefi.h>
#include <PiDxe.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/ShellLib.h>
#include <Library/PrintLib.h>
#include <Library/MemoryAllocationLib.h>

#include <Protocol/PciIo.h>
#include <Protocol/Decompress.h>
#include <IndustryStandard/Pci.h>

#define MAJOR_VERSION   1
#define MINOR_VERSION   0

CHAR16 mPciOpromFileName[16];

CONST SHELL_PARAM_ITEM GetPciOpromParamList[] = {
  {L"-?", TypeFlag},
  {L"-h", TypeFlag},
  {L"-H", TypeFlag},
  {L"-v", TypeFlag},
  {L"-V", TypeFlag},
  {L"-f", TypeFlag},
  {L"-F", TypeFlag},
  {L"-i", TypeFlag},
  {L"-I", TypeFlag},
  {NULL, TypeMax}
  };

/**
   Display current version.
**/
VOID
ShowVersion (
  )
{
  ShellPrintEx (-1, -1, L"GetPciOprom Version %d.%02d\n", MAJOR_VERSION, MINOR_VERSION);
}

/**
   Display Usage and Help information.
**/
VOID
ShowHelp (
  )
{
  ShellPrintEx (-1, -1, L"Get PCI OPROM.\n");
  ShellPrintEx (-1, -1, L"GetPciOprom [-h] [-v] [-f Bus Dev Func] [-i VendorId DeviceId]\n");
  ShellPrintEx (-1, -1, L"  -h      Show help information.\n");
  ShellPrintEx (-1, -1, L"  -v      Show version information.\n");
  ShellPrintEx (-1, -1, L"  -f      Get PCI OPROM by Bus, Dev and Func (hexadecimal format).\n");
  ShellPrintEx (-1, -1, L"  -i      Get PCI OPROM by VendorId and DeviceId (hexadecimal format).\n");
  ShellPrintEx (-1, -1, L"  -i      Get PCI OPROM by VendorId and DeviceId (hexadecimal format).\n");
  ShellPrintEx (-1, -1, L"Dump PCI OPROM information for all PCI devices if no -f/-i option is specified.\n");
  ShellPrintEx (-1, -1, L"Dump PCI OPROM information and save PCI OPROM to file for the specified PCI device\n");
  ShellPrintEx (-1, -1, L"if -f/-i option is specified.\n");
  ShellPrintEx (-1, -1, L"  PciOpromAll.bin is for whole PCI OPROM image\n");
  ShellPrintEx (-1, -1, L"  PciOpromLegacy.bin is for legacy OPROM entry\n");
  ShellPrintEx (-1, -1, L"  PciOpromXXX.efi is for EFI OPROM entry, XXX is the ARCH name, for example:\n");
  ShellPrintEx (-1, -1, L"    PciOpromX64.efi is for EFI X64 OPROM entry\n");
  ShellPrintEx (-1, -1, L"    PciOpromIA32.efi is for EFI IA32 OPROM entry\n");
}

typedef struct {
  UINT16  Type;
  CHAR16  *TypeName;
} TYPE_INFO;

TYPE_INFO mMachineTypeInfo[] = {
  {IMAGE_FILE_MACHINE_I386,             L"IA32"},
  {IMAGE_FILE_MACHINE_EBC,              L"EBC"},
  {IMAGE_FILE_MACHINE_X64,              L"X64"},
  {IMAGE_FILE_MACHINE_ARMTHUMB_MIXED,   L"ARM"},
  {IMAGE_FILE_MACHINE_ARM64,            L"AARCH64"}
};

/**
 Return MachineType name.

 @param MachineType The machine type.

 @return MachineType name.
**/
CHAR16 *
GetMachineTypeName (
  UINT16 MachineType
  )
{
  UINTN  Index;

  for (Index = 0; Index < sizeof(mMachineTypeInfo)/sizeof(mMachineTypeInfo[0]); Index++) {
    if (mMachineTypeInfo[Index].Type == MachineType) {
      return mMachineTypeInfo[Index].TypeName;
    }
  }

  return L"<Unknown>";
}

TYPE_INFO mSubSystemInfo[] = {
  {EFI_IMAGE_SUBSYSTEM_EFI_APPLICATION,         L"EFI_APPLICATION"},
  {EFI_IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER, L"EFI_BOOT_SERVICE_DRIVER"},
  {EFI_IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER,      L"EFI_RUNTIME_DRIVER"}
};

/**
 Return SubSystem name.

 @param SubSystem   The subsystem.

 @return SubSystem name.
**/
CHAR16 *
GetSubSystemName (
  UINT16 SubSystem
  )
{
  UINTN  Index;

  for (Index = 0; Index < sizeof(mSubSystemInfo)/sizeof(mSubSystemInfo[0]); Index++) {
    if (mSubSystemInfo[Index].Type == SubSystem) {
      return mSubSystemInfo[Index].TypeName;
    }
  }

  return L"<Unknown>";
}

TYPE_INFO mCodeTypeInfo[] = {
  {PCI_CODE_TYPE_PCAT_IMAGE,    L"LEGACY"},
  {PCI_CODE_TYPE_EFI_IMAGE,     L"EFI"}
};

/**
 Return CodeType name.

 @param CodeType    The code type.

 @return CodeType name.
**/
CHAR16 *
GetCodeTypeName (
  UINT16 CodeType
  )
{
  UINTN  Index;

  for (Index = 0; Index < sizeof(mCodeTypeInfo)/sizeof(mCodeTypeInfo[0]); Index++) {
    if (mCodeTypeInfo[Index].Type == CodeType) {
      return mCodeTypeInfo[Index].TypeName;
    }
  }

  return L"<Unknown>";
}

/**
  Save PCI OPROM to file.

  @param[in] FileName   File name.
  @param[in] Buffer     Buffer pointer.
  @param[in] BufferSize Buffer size.

  @retval EFI_SUCCESS   Save PCI OPROM to file successfully.
  @retval Others        Failed to save PCI OPROM to file.

**/
EFI_STATUS
SavePciOpromToFile (
  IN CONST CHAR16       *FileName,
  IN VOID               *Buffer,
  IN UINTN              BufferSize
  )
{
  EFI_STATUS            Status;
  SHELL_FILE_HANDLE     FileHandle;

  Status = ShellOpenFileByName (FileName, &FileHandle, EFI_FILE_MODE_WRITE | EFI_FILE_MODE_READ, 0);
  if (!EFI_ERROR (Status)) {
    //
    // Delete existing file.
    //
    Status = ShellDeleteFile (&FileHandle);
    if (EFI_ERROR (Status)) {
      ShellPrintEx (-1, -1, NULL, L"%H%a%N: Cannot delete file - '%H%s%N'\n", __FUNCTION__, FileName);
      return Status;
    }
  } else if (Status == EFI_NOT_FOUND) {
    //
    // Good when file doesn't exist.
    //
  } else {
    //
    // Otherwise it's bad.
    //
    ShellPrintEx (-1, -1, NULL, L"%H%a%N: Cannot open file - '%H%s%N'\n", __FUNCTION__, FileName);
    return Status;
  }

  Status = ShellOpenFileByName (FileName, &FileHandle, EFI_FILE_MODE_CREATE | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_READ, 0);
  if (EFI_ERROR (Status)) {
    ShellPrintEx (-1, -1, NULL, L"%H%a%N: Cannot create file - '%H%s%N'\n", __FUNCTION__, FileName);
    return Status;
  }

  return ShellWriteFile (FileHandle, &BufferSize, Buffer);
}

/**
  Process PCI ROM image.

  Dump PCI OPROM information and save PCI OPROM to file if SAVE is TRUE.

  @param[in] RomImage       ROM image buffer pointer.
  @param[in] RomSize        ROM image buffer size.
  @param[in] Save           Save PCI OPROM to file or not.

**/
VOID
ProcessPciRomImage (
  IN VOID       *RomImage,
  IN UINTN      RomSize,
  IN BOOLEAN    Save
  )
{
  EFI_STATUS                        Status;
  UINT8                             *RomEntry;
  UINTN                             RomOffset;
  EFI_PCI_EXPANSION_ROM_HEADER      *EfiRomHeader;
  EFI_LEGACY_EXPANSION_ROM_HEADER   *LegacyRomHeader;
  PCI_3_0_DATA_STRUCTURE            *Pcir;
  UINTN                             Index;
  UINT16                            *DeviceIdList;
  VOID                              *Image;
  UINT32                            ImageSize;
  VOID                              *Destination;
  UINT32                            DestinationSize;
  VOID                              *Scratch;
  UINT32                            ScratchSize;
  EFI_DECOMPRESS_PROTOCOL           *Decompress;

  if (RomImage == NULL) {
    ShellPrintEx (-1, -1, L"This PCI device has no OPROM.\n");
    return;
  }

  if (Save) {
    SavePciOpromToFile (L"PciOpromAll.bin", RomImage, RomSize);
    ShellPrintEx (-1, -1, L"Saved PciOpromAll.bin with size 0x%x\n", RomSize);
  }

  RomEntry = (UINT8 *) RomImage;
  ASSERT (((EFI_PCI_EXPANSION_ROM_HEADER *) RomEntry)->Signature == PCI_EXPANSION_ROM_HEADER_SIGNATURE);

  Index = 0;
  do {
    RomOffset = (UINTN) RomEntry - (UINTN) RomImage;
    if (RomOffset >= RomSize) {
      break;
    }
    EfiRomHeader = (EFI_PCI_EXPANSION_ROM_HEADER *) RomEntry;
    if (EfiRomHeader->Signature != PCI_EXPANSION_ROM_HEADER_SIGNATURE) {
      RomEntry += 512;
      continue;
    }

    Pcir = (PCI_3_0_DATA_STRUCTURE *) (RomEntry + EfiRomHeader->PcirOffset);
    ASSERT (Pcir->Signature == PCI_DATA_STRUCTURE_SIGNATURE);

    ShellPrintEx (-1, -1, L"OPROM (0x%x) (Offset = 0x%x)\n", Index, RomOffset);
    ShellPrintEx (-1, -1, L"  PCI_DATA_STRUCTURE:\n");
    ShellPrintEx (-1, -1, L"    Signature             = 0x%08x\n", Pcir->Signature);
    ShellPrintEx (-1, -1, L"    VendorId              = 0x%04x\n", Pcir->VendorId);
    ShellPrintEx (-1, -1, L"    DeviceId              = 0x%04x\n", Pcir->DeviceId);
    if (Pcir->Revision >= 3) {
      if (Pcir->DeviceListOffset != 0) {
        DeviceIdList = (UINT16 *) ((UINTN) Pcir + Pcir->DeviceListOffset);
        while (*DeviceIdList != 0) {
          ShellPrintEx (-1, -1, L"      DeviceIdList = 0x%x\n", *DeviceIdList);
          DeviceIdList++;
        }
      }
    }
    ShellPrintEx (-1, -1, L"    Revision              = 0x%x\n", Pcir->Revision);
    ShellPrintEx (-1, -1, L"    ClassCode             = 0x%06x\n", (*(UINT32 *) Pcir->ClassCode) & 0x00FFFFFF);
    ShellPrintEx (-1, -1, L"    ImageLength           = 0x%x\n", Pcir->ImageLength);
    if (Pcir->Revision >= 3) {
      ShellPrintEx (-1, -1, L"    MaxRuntimeImageLength = 0x%x\n", Pcir->MaxRuntimeImageLength);
    }
    ShellPrintEx (-1, -1, L"    Indicator             = 0x%x\n", Pcir->Indicator);
    ShellPrintEx (-1, -1, L"    CodeType              = 0x%x (%s)\n", Pcir->CodeType, GetCodeTypeName (Pcir->CodeType));

    if (Pcir->CodeType == PCI_CODE_TYPE_PCAT_IMAGE) {
      //
      // It is a LEGACY OPROM entry.
      //
      LegacyRomHeader = (EFI_LEGACY_EXPANSION_ROM_HEADER *) EfiRomHeader;
      ShellPrintEx (-1, -1, L"  EFI_LEGACY_EXPANSION_ROM_HEADER:\n");
      ShellPrintEx (-1, -1, L"    Signature            = 0x%08x\n", LegacyRomHeader->Signature);
      ShellPrintEx (-1, -1, L"    Size512              = 0x%x\n", LegacyRomHeader->Size512);
      ShellPrintEx (-1, -1, L"    InitEntryPoint       = 0x%06x\n", (*(UINT32 *) LegacyRomHeader->InitEntryPoint) & 0x00FFFFFF);
      ShellPrintEx (-1, -1, L"    PcirOffset           = 0x%x\n", LegacyRomHeader->PcirOffset);
      if (Save) {
        SavePciOpromToFile (L"PciOpromLegacy.bin", RomEntry, Pcir->ImageLength * 512);
        ShellPrintEx (-1, -1, L"Saved PciOpromLegacy.bin with size 0x%x\n", Pcir->ImageLength * 512);
      }
    } else if (Pcir->CodeType == PCI_CODE_TYPE_EFI_IMAGE) {
      //
      // It is a EFI OPROM entry.
      //
      ShellPrintEx (-1, -1, L"  EFI_PCI_EXPANSION_ROM_HEADER:\n");
      ShellPrintEx (-1, -1, L"    Signature            = 0x%08x\n", LegacyRomHeader->Signature);
      ShellPrintEx (-1, -1, L"    InitializationSize   = 0x%x\n", EfiRomHeader->InitializationSize);
      ShellPrintEx (-1, -1, L"    EfiSubsystem         = 0x%x (%s)\n", EfiRomHeader->EfiSubsystem, GetSubSystemName (EfiRomHeader->EfiSubsystem));
      ShellPrintEx (-1, -1, L"    EfiMachineType       = 0x%x (%s)\n", EfiRomHeader->EfiMachineType, GetMachineTypeName (EfiRomHeader->EfiMachineType));
      ShellPrintEx (-1, -1, L"    CompressionType      = 0x%x\n", EfiRomHeader->CompressionType);
      ShellPrintEx (-1, -1, L"    EfiImageHeaderOffset = 0x%x\n", EfiRomHeader->EfiImageHeaderOffset);
      ShellPrintEx (-1, -1, L"    PcirOffset           = 0x%x\n", EfiRomHeader->PcirOffset);

      UnicodeSPrint (mPciOpromFileName, sizeof (mPciOpromFileName), L"PciOprom%s.efi", GetMachineTypeName (EfiRomHeader->EfiMachineType));

      Image = (VOID *) ((UINTN) EfiRomHeader + EfiRomHeader->EfiImageHeaderOffset);
      ImageSize = (UINT32) (EfiRomHeader->InitializationSize * 512 - EfiRomHeader->EfiImageHeaderOffset);

      if (Save) {
        if (EfiRomHeader->CompressionType != EFI_PCI_EXPANSION_ROM_HEADER_COMPRESSED) {
          //
          // Uncompressed.
          //
          SavePciOpromToFile (mPciOpromFileName, Image, ImageSize);
          ShellPrintEx (-1, -1, L"Saved %s with size 0x%x\n", mPciOpromFileName, ImageSize);
        } else {
          //
          // Compressed.
          //
          Status = gBS->LocateProtocol (&gEfiDecompressProtocolGuid, NULL, (VOID **) &Decompress);
          ASSERT_EFI_ERROR (Status);
          Status = Decompress->GetInfo (
                                 Decompress,
                                 Image,
                                 ImageSize,
                                 &DestinationSize,
                                 &ScratchSize
                                 );
          ASSERT_EFI_ERROR (Status);
          Destination = AllocatePool (DestinationSize);
          ASSERT (Destination != NULL);
          Scratch = AllocatePool (ScratchSize);
          ASSERT (Scratch != NULL);
          Status = Decompress->Decompress (
                                 Decompress,
                                 Image,
                                 ImageSize,
                                 Destination,
                                 DestinationSize,
                                 Scratch,
                                 ScratchSize
                                 );
          ASSERT_EFI_ERROR (Status);
          SavePciOpromToFile (mPciOpromFileName, Destination, DestinationSize);
          ShellPrintEx (-1, -1, L"Saved %s with size 0x%x\n", mPciOpromFileName, DestinationSize);
          FreePool (Scratch);
          FreePool (Destination);
        }
      }
    }

    RomEntry += Pcir->ImageLength * 512;
    Index++;
  } while (((Pcir->Indicator & 0x80) == 0x00) && (RomOffset < RomSize));
}

/**
  Get PCI OPROM.

  Dump PCI OPROM information for all PCI devices if Bus is 0x100, Device is 0x20,
  Function is 0x8, VendorId is 0xFFFF and DeviceId is 0xFFFF, otherwise dump
  PCI OPROM information and save PCI OPROM to file for the specific PCI device.

  @param[in] Bus          PCI bus, 0x100 to match any PCI bus.
  @param[in] Device       PCI device, 0x20 to match any PCI device.
  @param[in] Function     PCI function, 0x8 to match any PCI function.
  @param[in] VendorId     PCI vendor ID, 0xFFFF to match any PCI vendor ID.
  @param[in] DeviceId     PCI devcie ID, 0xFFFF to match any PCI devcie ID.

  @retval EFI_SUCCESS     Get PCI OPROM successfully.
  @retval EFI_NOT_FOUND   No any or the specified PCI device is not found.

**/
EFI_STATUS
GetPciOprom (
  IN UINTN      Bus,
  IN UINTN      Device,
  IN UINTN      Function,
  IN UINTN      VendorId,
  IN UINTN      DeviceId
  )
{
  EFI_STATUS            Status;
  UINTN                 PciIoHandleCount;
  EFI_HANDLE            *PciIoHandleBuffer;
  UINTN                 Index;
  EFI_PCI_IO_PROTOCOL   *PciIo;
  UINT16                PciVendorId;
  UINT16                PciDeviceId;
  UINTN                 PciSegment;
  UINTN                 PciBus;
  UINTN                 PciDevice;
  UINTN                 PciFunction;
  BOOLEAN               Found;

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiPciIoProtocolGuid,
                  NULL,
                  &PciIoHandleCount,
                  &PciIoHandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    ShellPrintEx (-1, -1, L"%a: %EError. %NNo any PCI device is found.\n", __FUNCTION__);
    return Status;
  }

  Found = FALSE;
  for (Index = 0; Index < PciIoHandleCount; Index++) {
    Status = gBS->HandleProtocol (
                    PciIoHandleBuffer[Index],
                    &gEfiPciIoProtocolGuid,
                    (VOID**) &PciIo
                    );
    ASSERT_EFI_ERROR (Status);

    Status = PciIo->GetLocation (PciIo, &PciSegment, &PciBus, &PciDevice, &PciFunction);
    ASSERT_EFI_ERROR (Status);

    Status = PciIo->Pci.Read (PciIo, EfiPciIoWidthUint16, PCI_VENDOR_ID_OFFSET, 1, &PciVendorId);
    ASSERT_EFI_ERROR (Status);

    Status = PciIo->Pci.Read (PciIo, EfiPciIoWidthUint16, PCI_DEVICE_ID_OFFSET, 1, &PciDeviceId);
    ASSERT_EFI_ERROR (Status);

    if (((Bus      == 0x100)  || (Bus      == PciBus)) &&
        ((Device   == 0x20)   || (Device   == PciDevice)) &&
        ((Function == 0x8)    || (Function == PciFunction)) &&
        ((VendorId == 0xFFFF) || (VendorId == PciVendorId)) &&
        ((DeviceId == 0xFFFF) || (DeviceId == PciDeviceId))) {
      Found = TRUE;
      ShellPrintEx (
        -1,
        -1,
        L"\nPCI device (B%02xD%02xF%02x) (%04x:%04x)\n",
        PciBus,
        PciDevice,
        PciFunction,
        PciVendorId,
        PciDeviceId
        );
      if ((Bus      == 0x100) &&
          (Device   == 0x20) &&
          (Function == 0x8) &&
          (VendorId == 0xFFFF) &&
          (DeviceId == 0xFFFF)) {
        //
        // It is to dump PCI OPROM information for all PCI devices.
        //
        ProcessPciRomImage (PciIo->RomImage, (UINTN) PciIo->RomSize, FALSE);
      } else {
        //
        // It is to dump PCI OPROM information and save PCI OPROM to file
        // for the specific PCI device.
        //
        ProcessPciRomImage (PciIo->RomImage, (UINTN) PciIo->RomSize, TRUE);
        break;
      }
    }
  }

  gBS->FreePool (PciIoHandleBuffer);

  if (Found) {
    return EFI_SUCCESS;
  } else {
    ShellPrintEx (-1, -1, L"%a: %EError. %NThe specified PCI device is not found.\n", __FUNCTION__);
    return EFI_NOT_FOUND;
  }
}

/**
  Main entrypoint for GetPciOprom shell application.

  @param[in] ImageHandle    The image handle.
  @param[in] SystemTable    The system table.

  @retval EFI_SUCCESS            Get PCI OPROM successfully.
  @retval EFI_INVALID_PARAMETER  Command usage error.
  @retval EFI_NOT_FOUND          No any or the specified PCI device is not found.

**/
EFI_STATUS
EFIAPI
GetPciOpromMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS            Status;
  LIST_ENTRY            *ParamPackage;
  CHAR16                *ParamProblem;
  CONST CHAR16          *CmdLineArg;
  UINTN                 Bus;
  UINTN                 Device;
  UINTN                 Function;
  UINTN                 VendorId;
  UINTN                 DeviceId;

  ParamPackage = NULL;
  Bus = 0x100;
  Device = 0x20;
  Function = 0x8;
  VendorId = 0xFFFF;
  DeviceId = 0xFFFF;

  //
  // initialize the shell lib (we must be in non-auto-init...)
  //
  Status = ShellInitialize ();
  ASSERT_EFI_ERROR (Status);

  //
  // parse the command line
  //
  Status = ShellCommandLineParse (GetPciOpromParamList, &ParamPackage, &ParamProblem, TRUE);
  if (EFI_ERROR (Status)) {
    if (ParamProblem != NULL) {
      ShellPrintEx (-1, -1, L"%a: %EError. %NThe argument '%B%s%N' is invalid.\n", __FUNCTION__, ParamProblem);
      FreePool (ParamProblem);
    } else {
      ShellPrintEx (-1, -1, L"%a: %EError. %NThe input parameters are not recognized.\n", __FUNCTION__);
    }
    Status = EFI_INVALID_PARAMETER;
  } else {
    if (ShellCommandLineGetCount (ParamPackage) > 4) {
      ShellPrintEx (-1, -1, L"%a: %EError. %NToo many arguments specified.\n", __FUNCTION__);
      Status = EFI_INVALID_PARAMETER;
    } else if (ShellCommandLineGetFlag (ParamPackage, L"-?")  ||
               ShellCommandLineGetFlag (ParamPackage, L"-h")  ||
               ShellCommandLineGetFlag (ParamPackage, L"-H")) {
      //
      // check for "-?" help information.
      //
      ShowHelp ();
      goto Done;
    } else if (ShellCommandLineGetFlag (ParamPackage, L"-v") ||
               ShellCommandLineGetFlag (ParamPackage, L"-V")) {
      //
      // check for "-v" for version inforamtion.
      //
      ShowVersion ();
      goto Done;
    } else if (ShellCommandLineGetFlag (ParamPackage, L"-f")  ||
               ShellCommandLineGetFlag (ParamPackage, L"-F")) {
      if (ShellCommandLineGetCount (ParamPackage) != 4) {
        ShellPrintEx (-1, -1, L"%a: %EError. %NIncorrect parameter count for -f/-F.\n", __FUNCTION__);
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      }
      CmdLineArg = ShellCommandLineGetRawValue (ParamPackage, 1);
      if (CmdLineArg != NULL) {
        Bus = StrDecimalToUintn (CmdLineArg);
        if (Bus >= 0x100) {
          ShellPrintEx (-1, -1, L"%a: %EError. %NInvalid Bus value: 0x%x.\n", __FUNCTION__, Bus);
          Status = EFI_INVALID_PARAMETER;
          goto Done;
        }
      }
      CmdLineArg = ShellCommandLineGetRawValue (ParamPackage, 2);
      if (CmdLineArg != NULL) {
        Device = StrDecimalToUintn (CmdLineArg);
        if (Device >= 0x20) {
          ShellPrintEx (-1, -1, L"%a: %EError. %NInvalid Device value: 0x%x.\n", __FUNCTION__, Device);
          Status = EFI_INVALID_PARAMETER;
          goto Done;
        }
      }
      CmdLineArg = ShellCommandLineGetRawValue (ParamPackage, 3);
      if (CmdLineArg != NULL) {
        Function = StrDecimalToUintn (CmdLineArg);
        if (Function >= 0x8) {
          ShellPrintEx (-1, -1, L"%a: %EError. %NInvalid Function value: 0x%x.\n", __FUNCTION__, Function);
          Status = EFI_INVALID_PARAMETER;
          goto Done;
        }
      }
    } else if (ShellCommandLineGetFlag (ParamPackage, L"-i")  ||
               ShellCommandLineGetFlag (ParamPackage, L"-I")) {
      if (ShellCommandLineGetCount (ParamPackage) != 3) {
        ShellPrintEx (-1, -1, L"%a: %EError. %NIncorrect parameter count for -i/-I.\n", __FUNCTION__);
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      }
      CmdLineArg = ShellCommandLineGetRawValue (ParamPackage, 1);
      if (CmdLineArg != NULL) {
        VendorId = StrHexToUintn (CmdLineArg);
        if (VendorId >= 0xFFFF) {
          ShellPrintEx (-1, -1, L"%a: %EError. %NInvalid VendorId value: 0x%x.\n", __FUNCTION__, VendorId);
          Status = EFI_INVALID_PARAMETER;
          goto Done;
        }
      }
      CmdLineArg = ShellCommandLineGetRawValue (ParamPackage, 2);
      if (CmdLineArg != NULL) {
        DeviceId = StrHexToUintn (CmdLineArg);
        if (DeviceId >= 0xFFFF) {
          ShellPrintEx (-1, -1, L"%a: %EError. %NInvalid DeviceId value: 0x%x.\n", __FUNCTION__, DeviceId);
          Status = EFI_INVALID_PARAMETER;
          goto Done;
        }
      }
    }
  }

  //
  // Additional check for the input parameter.
  //
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  Status = GetPciOprom (Bus, Device, Function, VendorId, DeviceId);

Done:
  //
  // Free the command line package map to ShellCommandLineParse.
  //
  if (ParamPackage != NULL) {
    ShellCommandLineFreeVarList (ParamPackage);
  }

  return Status;
}

