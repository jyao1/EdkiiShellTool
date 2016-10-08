/** @file

Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <PiDxe.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiLib.h>
#include <Library/DevicePathLib.h>
#include <Library/PeCoffGetEntryPointLib.h>
#include <Library/DxeServicesLib.h>
#include <Protocol/LoadedImage.h>

#include <Protocol/DevicePathUtilities.h>
#include <Protocol/DevicePathToText.h>
#include <Protocol/DevicePathFromText.h>
#include <Protocol/PlatformDriverOverride.h>
#include <Protocol/BusSpecificDriverOverride.h>
#include <Protocol/DriverDiagnostics2.h>
#include <Protocol/ComponentName2.h>
#include <Protocol/ServiceBinding.h>
#include <Protocol/PlatformToDriverConfiguration.h>
#include <Protocol/DriverFamilyOverride.h>
#include <Protocol/DriverHealth.h>
#include <Protocol/AdapterInformation.h>
#include <Protocol/SimpleTextInEx.h>
#include <Protocol/SimplePointer.h>
#include <Protocol/AbsolutePointer.h>
#include <Protocol/SerialIo.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/LoadFile.h>
#include <Protocol/LoadFile2.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/DiskIo.h>
#include <Protocol/DiskIo2.h>
#include <Protocol/BlockIo.h>
#include <Protocol/BlockIo2.h>
#include <Protocol/BlockIoCrypto.h>
#include <Protocol/EraseBlock.h>
#include <Protocol/AtaPassThru.h>
#include <Protocol/StorageSecurityCommand.h>
#include <Protocol/NvmExpressPassthru.h>
#include <Protocol/SdMmcPassThru.h>
#include <Protocol/RamDisk.h>
#include <Protocol/PciRootBridgeIo.h>
#include <Protocol/PciIo.h>
#include <Protocol/ScsiIo.h>
#include <Protocol/ScsiPassThruExt.h>
#include <Protocol/IScsiInitiatorName.h>
#include <Protocol/Usb2HostController.h>
#include <Protocol/UsbIo.h>
#include <Protocol/UsbFunctionIo.h>
#include <Protocol/DebugSupport.h>
#include <Protocol/DebugPort.h>
#include <Protocol/Decompress.h>
#include <Protocol/AcpiTable.h>
#include <Protocol/UnicodeCollation.h>
#include <Protocol/RegularExpressionProtocol.h>
#include <Protocol/Ebc.h>
#include <Protocol/FirmwareManagement.h>
#include <Protocol/HiiFont.h>
#include <Protocol/HiiString.h>
#include <Protocol/HiiImage.h>
#include <Protocol/HiiImageEx.h>
#include <Protocol/ImageDecoder.h>
#include <Protocol/HiiDatabase.h>
#include <Protocol/HiiConfigKeyword.h>
#include <Protocol/HiiConfigRouting.h>
#include <Protocol/HiiConfigAccess.h>
#include <Protocol/FormBrowser2.h>
#include <Protocol/Hash2.h>
#include <Protocol/Kms.h>
#include <Protocol/Pkcs7Verify.h>
#include <Protocol/Rng.h>
#include <Protocol/SmartCardReader.h>
#include <Protocol/SmartCardEdge.h>
#include <Protocol/Timestamp.h>

#include "DxeCoreDump.h"

//
// This structure is copied from EDKII DxeCore
//
#include "DxeCore/DxeCore.h"
#include "DxeCore/DxeMain.h"
#include "DxeCore/Handle.h"
#include "DxeCore/Event.h"

#define UNKNOWN_NAME  L"???"

#define NAME_STRING_LENGTH  36
typedef struct {
  UINTN   ImageBase;
  UINTN   ImageSize;
  CHAR16  NameString[NAME_STRING_LENGTH + 1];
} IMAGE_STRUCT;

typedef struct {
  EFI_GUID   *Guid;
  UINTN      FuncOffset;
} PROTOCOL_FUNC_STRUCT;

PROTOCOL_FUNC_STRUCT  mProtocolFuncStruct[] = {
  // 9. Protocols - Device Path Protocol
  { &gEfiDevicePathUtilitiesProtocolGuid, OFFSET_OF(EFI_DEVICE_PATH_UTILITIES_PROTOCOL, GetDevicePathSize) },
  { &gEfiDevicePathToTextProtocolGuid, OFFSET_OF(EFI_DEVICE_PATH_TO_TEXT_PROTOCOL, ConvertDeviceNodeToText) },
  { &gEfiDevicePathFromTextProtocolGuid, OFFSET_OF(EFI_DEVICE_PATH_FROM_TEXT_PROTOCOL, ConvertTextToDeviceNode) },
  // 10. Protocols - UEFI Driver Model
  { &gEfiDriverBindingProtocolGuid, OFFSET_OF(EFI_DRIVER_BINDING_PROTOCOL, Supported) },
  { &gEfiPlatformDriverOverrideProtocolGuid, OFFSET_OF(EFI_PLATFORM_DRIVER_OVERRIDE_PROTOCOL, GetDriver) },
  { &gEfiBusSpecificDriverOverrideProtocolGuid, OFFSET_OF(EFI_BUS_SPECIFIC_DRIVER_OVERRIDE_PROTOCOL, GetDriver) },
  { &gEfiDriverDiagnostics2ProtocolGuid, OFFSET_OF(EFI_DRIVER_DIAGNOSTICS2_PROTOCOL, RunDiagnostics) },
  { &gEfiComponentName2ProtocolGuid, OFFSET_OF(EFI_COMPONENT_NAME2_PROTOCOL, GetDriverName) },
  { &gEfiPlatformToDriverConfigurationProtocolGuid, OFFSET_OF(EFI_PLATFORM_TO_DRIVER_CONFIGURATION_PROTOCOL, Query) },
  { &gEfiDriverFamilyOverrideProtocolGuid, OFFSET_OF(EFI_DRIVER_FAMILY_OVERRIDE_PROTOCOL, GetVersion) },
  { &gEfiDriverHealthProtocolGuid, OFFSET_OF(EFI_DRIVER_HEALTH_PROTOCOL, GetHealthStatus) },
  { &gEfiAdapterInformationProtocolGuid, OFFSET_OF(EFI_ADAPTER_INFORMATION_PROTOCOL, GetInformation) },
  // 11. Protocols - Console Support
  { &gEfiSimpleTextInputExProtocolGuid, OFFSET_OF(EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL, Reset) },
  { &gEfiSimpleTextInProtocolGuid, OFFSET_OF(EFI_SIMPLE_TEXT_INPUT_PROTOCOL, Reset) },
  { &gEfiSimpleTextOutProtocolGuid, OFFSET_OF(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL, Reset) },
  { &gEfiSimplePointerProtocolGuid, OFFSET_OF(EFI_SIMPLE_POINTER_PROTOCOL, Reset) },
  { &gEfiAbsolutePointerProtocolGuid, OFFSET_OF(EFI_ABSOLUTE_POINTER_PROTOCOL, Reset) },
  { &gEfiSerialIoProtocolGuid, OFFSET_OF(EFI_SERIAL_IO_PROTOCOL, Reset) },
  { &gEfiGraphicsOutputProtocolGuid, OFFSET_OF(EFI_GRAPHICS_OUTPUT_PROTOCOL, QueryMode) },
  // 12. Protocols - Media Access
  { &gEfiLoadFileProtocolGuid, OFFSET_OF(EFI_LOAD_FILE_PROTOCOL, LoadFile) },
  { &gEfiLoadFile2ProtocolGuid, OFFSET_OF(EFI_LOAD_FILE_PROTOCOL, LoadFile) },
  { &gEfiSimpleFileSystemProtocolGuid, OFFSET_OF(EFI_SIMPLE_FILE_SYSTEM_PROTOCOL, OpenVolume) },
  { &gEfiDiskIoProtocolGuid, OFFSET_OF(EFI_DISK_IO_PROTOCOL, ReadDisk) },
  { &gEfiDiskIo2ProtocolGuid, OFFSET_OF(EFI_DISK_IO2_PROTOCOL, Cancel) },
  { &gEfiBlockIoProtocolGuid, OFFSET_OF(EFI_BLOCK_IO_PROTOCOL, Reset) },
  { &gEfiBlockIo2ProtocolGuid, OFFSET_OF(EFI_BLOCK_IO2_PROTOCOL, Reset) },
  { &gEfiBlockIoCryptoProtocolGuid, OFFSET_OF(EFI_BLOCK_IO_CRYPTO_PROTOCOL, Reset) },
  { &gEfiEraseBlockProtocolGuid, OFFSET_OF(EFI_ERASE_BLOCK_PROTOCOL, EraseBlocks) },
  { &gEfiAtaPassThruProtocolGuid, OFFSET_OF(EFI_ATA_PASS_THRU_PROTOCOL, PassThru) },
  { &gEfiStorageSecurityCommandProtocolGuid, OFFSET_OF(EFI_STORAGE_SECURITY_COMMAND_PROTOCOL, ReceiveData) },
  { &gEfiNvmExpressPassThruProtocolGuid, OFFSET_OF(EFI_NVM_EXPRESS_PASS_THRU_PROTOCOL, PassThru) },
  { &gEfiSdMmcPassThruProtocolGuid, OFFSET_OF(EFI_SD_MMC_PASS_THRU_PROTOCOL, PassThru) },
  { &gEfiRamDiskProtocolGuid, OFFSET_OF(EFI_RAM_DISK_PROTOCOL, Register) },
  // 13. Protocols - PCI Bus Support
  { &gEfiPciRootBridgeIoProtocolGuid, OFFSET_OF(EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL, PollMem) },
  { &gEfiPciIoProtocolGuid, OFFSET_OF(EFI_PCI_IO_PROTOCOL, PollMem) },
  // 14. Protocols - SCSI Driver Models and Bus Support
  { &gEfiScsiIoProtocolGuid, OFFSET_OF(EFI_SCSI_IO_PROTOCOL, GetDeviceType) },
  { &gEfiExtScsiPassThruProtocolGuid, OFFSET_OF(EFI_EXT_SCSI_PASS_THRU_PROTOCOL, PassThru) },
  // 15. Protocols - iSCSI Boot
  { &gEfiIScsiInitiatorNameProtocolGuid, OFFSET_OF(EFI_ISCSI_INITIATOR_NAME_PROTOCOL, Get) },
  // 16. Protocols - USB Support
  { &gEfiUsb2HcProtocolGuid, OFFSET_OF(EFI_USB2_HC_PROTOCOL, GetCapability) },
  { &gEfiUsbIoProtocolGuid, OFFSET_OF(EFI_USB_IO_PROTOCOL, UsbControlTransfer) },
  { &gEfiUsbFunctionIoProtocolGuid, OFFSET_OF(EFI_USBFN_IO_PROTOCOL, DetectPort) },
  // 17. Protocols - Debugger Support
  { &gEfiDebugSupportProtocolGuid, OFFSET_OF(EFI_DEBUG_SUPPORT_PROTOCOL, GetMaximumProcessorIndex) },
  { &gEfiDebugPortProtocolGuid, OFFSET_OF(EFI_DEBUGPORT_PROTOCOL, Reset) },
  // 18. Protocols - Compression Algorithm Specification
  { &gEfiDecompressProtocolGuid, OFFSET_OF(EFI_DECOMPRESS_PROTOCOL, GetInfo) },
  // 19. Protocols - ACPI Protocols
  { &gEfiAcpiTableProtocolGuid, OFFSET_OF(EFI_ACPI_TABLE_PROTOCOL, InstallAcpiTable) },
  // 20. Protocols - String Services
  { &gEfiUnicodeCollationProtocolGuid, OFFSET_OF(EFI_UNICODE_COLLATION_PROTOCOL, StriColl) },
  { &gEfiUnicodeCollation2ProtocolGuid, OFFSET_OF(EFI_UNICODE_COLLATION_PROTOCOL, StriColl) },
  { &gEfiRegularExpressionProtocolGuid, OFFSET_OF(EFI_REGULAR_EXPRESSION_PROTOCOL, MatchString) },
  // 21. EFI Byte Code Virtual Machine
  { &gEfiEbcProtocolGuid, OFFSET_OF(EFI_EBC_PROTOCOL, CreateThunk) },
  // 22. Firmware Update and Reporting
  { &gEfiFirmwareManagementProtocolGuid, OFFSET_OF(EFI_FIRMWARE_MANAGEMENT_PROTOCOL, GetImageInfo) },
  // 32. HII Protocols
  { &gEfiHiiFontProtocolGuid, OFFSET_OF(EFI_HII_FONT_PROTOCOL, StringToImage) },
//{ &gEfiHiiFontExProtocolGuid, OFFSET_OF(EFI_HII_FONT_EX_PROTOCOL, StringToImageEx) },
  { &gEfiHiiStringProtocolGuid, OFFSET_OF(EFI_HII_STRING_PROTOCOL, NewString) },
  { &gEfiHiiImageProtocolGuid, OFFSET_OF(EFI_HII_IMAGE_PROTOCOL, NewImage) },
  { &gEfiHiiImageExProtocolGuid, OFFSET_OF(EFI_HII_IMAGE_EX_PROTOCOL, NewImageEx) },
  { &gEfiHiiImageDecoderProtocolGuid, OFFSET_OF(EFI_HII_IMAGE_DECODER_PROTOCOL, GetImageDecoderName) },
//{ &gEfiHiiFontGlyphGeneratorProtocolGuid, OFFSET_OF(EFI_HII_FONT_GLYPH_GENERATOR_PROTOCOL, GenerateGlyph) },
  { &gEfiHiiDatabaseProtocolGuid, OFFSET_OF(EFI_HII_DATABASE_PROTOCOL, NewPackageList) },
  // 33. HII Configuration Processing and Browser Protocol
  { &gEfiConfigKeywordHandlerProtocolGuid, OFFSET_OF(EFI_CONFIG_KEYWORD_HANDLER_PROTOCOL, SetData) },
  { &gEfiHiiConfigRoutingProtocolGuid, OFFSET_OF(EFI_HII_CONFIG_ROUTING_PROTOCOL, ExtractConfig) },
  { &gEfiHiiConfigAccessProtocolGuid, OFFSET_OF(EFI_HII_CONFIG_ACCESS_PROTOCOL, ExtractConfig) },
  { &gEfiFormBrowser2ProtocolGuid, OFFSET_OF(EFI_FORM_BROWSER2_PROTOCOL, SendForm) },
  // 35. Secure Technologies
  { &gEfiHash2ServiceBindingProtocolGuid, OFFSET_OF(EFI_SERVICE_BINDING_PROTOCOL, CreateChild) },
  { &gEfiHash2ProtocolGuid, OFFSET_OF(EFI_HASH2_PROTOCOL, GetHashSize) },
  { &gEfiKmsProtocolGuid, OFFSET_OF(EFI_KMS_PROTOCOL, GetServiceStatus) },
  { &gEfiPkcs7VerifyProtocolGuid, OFFSET_OF(EFI_PKCS7_VERIFY_PROTOCOL, VerifyBuffer) },
  { &gEfiRngProtocolGuid, OFFSET_OF(EFI_RNG_PROTOCOL, GetInfo) },
  { &gEfiSmartCardReaderProtocolGuid, OFFSET_OF(EFI_SMART_CARD_READER_PROTOCOL, SCardConnect) },
  { &gEfiSmartCardEdgeProtocolGuid, OFFSET_OF(EFI_SMART_CARD_EDGE_PROTOCOL, GetContext) },
  // 36. Protocols - Timestamp Protocol
  { &gEfiTimestampProtocolGuid, OFFSET_OF(EFI_TIMESTAMP_PROTOCOL, GetTimestamp) },
};

MEDIA_FW_VOL_FILEPATH_DEVICE_PATH  mDxeCoreFilePath = {
  { MEDIA_DEVICE_PATH, MEDIA_PIWG_FW_FILE_DP, {sizeof(MEDIA_FW_VOL_FILEPATH_DEVICE_PATH), 0} },
  DXE_CORE_GUID,
};

EFI_LOADED_IMAGE_PROTOCOL  *mDxeCoreLoadedImage;

LIST_ENTRY      *mDxeCoreHandleList;
LIST_ENTRY      *mDxeCoreProtocolDatabase;

LIST_ENTRY      *mDxeCoreEventSignalQueue;
LIST_ENTRY      *mDxeCoreEfiTimerList;

CHAR16 mNameString[NAME_STRING_LENGTH + 1];

IMAGE_STRUCT  *mImageStruct;
UINTN         mImageStructCountMax;
UINTN         mImageStructCount;

/** 
  Get the file name portion of the Pdb File Name.
  
  The portion of the Pdb File Name between the last backslash and
  either a following period or the end of the string is converted
  to Unicode and copied into UnicodeBuffer.  The name is truncated,
  if necessary, to ensure that UnicodeBuffer is not overrun.
  
  @param[in]  PdbFileName     Pdb file name.
  @param[out] UnicodeBuffer   The resultant Unicode File Name.
  
**/
VOID
GetShortPdbFileName (
  IN  CHAR8     *PdbFileName,
  OUT CHAR16    *UnicodeBuffer
  )
{
  UINTN IndexA;     // Current work location within an ASCII string.
  UINTN IndexU;     // Current work location within a Unicode string.
  UINTN StartIndex;
  UINTN EndIndex;

  ZeroMem (UnicodeBuffer, (NAME_STRING_LENGTH + 1) * sizeof (CHAR16));

  if (PdbFileName == NULL) {
    StrnCpyS (UnicodeBuffer, NAME_STRING_LENGTH + 1, L" ", 1);
  } else {
    StartIndex = 0;
    for (EndIndex = 0; PdbFileName[EndIndex] != 0; EndIndex++);
    for (IndexA = 0; PdbFileName[IndexA] != 0; IndexA++) {
      if (PdbFileName[IndexA] == '\\') {
        StartIndex = IndexA + 1;
      }

      if (PdbFileName[IndexA] == '.') {
        EndIndex = IndexA;
      }
    }

    IndexU = 0;
    for (IndexA = StartIndex; IndexA < EndIndex; IndexA++) {
      UnicodeBuffer[IndexU] = (CHAR16) PdbFileName[IndexA];
      IndexU++;
      if (IndexU >= NAME_STRING_LENGTH) {
        UnicodeBuffer[NAME_STRING_LENGTH] = 0;
        break;
      }
    }
  }
}

/** 
  Get a human readable name for an image.
  The following methods will be tried orderly:
    1. Image PDB
    2. FFS UI section
    3. Image GUID

    @param[in]  LoadedImage LoadedImage protocol.

  @post The resulting Unicode name string is stored in the mNameString global array.

**/
CHAR16 *
GetDriverNameString (
 IN EFI_LOADED_IMAGE_PROTOCOL  *LoadedImage
 )
{
  EFI_STATUS                  Status;
  CHAR8                       *PdbFileName;
  CHAR16                      *NameString;
  UINTN                       StringSize;
  EFI_GUID                    *FileName;

  //
  // Method 1: Get the name string from image PDB
  //
  if (LoadedImage->ImageBase != 0) {
    PdbFileName = PeCoffLoaderGetPdbPointer ((VOID *) (UINTN)LoadedImage->ImageBase);
    if (PdbFileName != NULL) {
      GetShortPdbFileName (PdbFileName, mNameString);
      return mNameString;
    }
  }

  FileName = NULL;
  if ((DevicePathType(LoadedImage->FilePath) == MEDIA_DEVICE_PATH) &&
      (DevicePathSubType(LoadedImage->FilePath) == MEDIA_PIWG_FW_FILE_DP)) {
    FileName = &((MEDIA_FW_VOL_FILEPATH_DEVICE_PATH *)LoadedImage->FilePath)->FvFileName;
  }

  if (FileName != NULL) {
    //
    // Try to get the image's FFS UI section by image GUID
    //
    NameString = NULL;
    StringSize = 0;
    Status = GetSectionFromAnyFv (
               FileName,
               EFI_SECTION_USER_INTERFACE,
               0,
               (VOID **) &NameString,
               &StringSize
               );
    if (!EFI_ERROR (Status)) {
      //
      // Method 2: Get the name string from FFS UI section
      //
      StrnCpyS (mNameString, NAME_STRING_LENGTH + 1, NameString, NAME_STRING_LENGTH);
      mNameString[NAME_STRING_LENGTH] = 0;
      FreePool (NameString);
      return mNameString;
    }

    //
    // Method 3: Get the name string from image GUID
    //
    UnicodeSPrint (mNameString, sizeof (mNameString), L"%g", FileName);
    return mNameString;
  }

  if ((DevicePathType(LoadedImage->FilePath) == MEDIA_DEVICE_PATH) &&
    (DevicePathSubType(LoadedImage->FilePath) == MEDIA_FILEPATH_DP)) {
    return ((FILEPATH_DEVICE_PATH *)LoadedImage->FilePath)->PathName;
  }
  return NULL;
}

/**
  Retrieves and returns a pointer to the entry point to a PE/COFF image that has been loaded
  into system memory with the PE/COFF Loader Library functions.

  Retrieves the entry point to the PE/COFF image specified by Pe32Data and returns this entry
  point in EntryPoint.  If the entry point could not be retrieved from the PE/COFF image, then
  return RETURN_INVALID_PARAMETER.  Otherwise return RETURN_SUCCESS.
  If Pe32Data is NULL, then ASSERT().
  If EntryPoint is NULL, then ASSERT().

  @param  Pe32Data                  The pointer to the PE/COFF image that is loaded in system memory.
  @param  EntryPoint                The pointer to entry point to the PE/COFF image to return.

  @retval RETURN_SUCCESS            EntryPoint was returned.
  @retval RETURN_INVALID_PARAMETER  The entry point could not be found in the PE/COFF image.

**/
RETURN_STATUS
InternalPeCoffGetEntryPoint (
  IN  VOID  *Pe32Data,
  OUT VOID  **EntryPoint
  )
{
  EFI_IMAGE_DOS_HEADER                  *DosHdr;
  EFI_IMAGE_OPTIONAL_HEADER_PTR_UNION   Hdr;

  ASSERT (Pe32Data   != NULL);
  ASSERT (EntryPoint != NULL);

  DosHdr = (EFI_IMAGE_DOS_HEADER *) Pe32Data;
  if (DosHdr->e_magic == EFI_IMAGE_DOS_SIGNATURE) {
    //
    // DOS image header is present, so read the PE header after the DOS image header.
    //
    Hdr.Pe32 = (EFI_IMAGE_NT_HEADERS32 *) ((UINTN) Pe32Data + (UINTN) ((DosHdr->e_lfanew) & 0x0ffff));
  } else {
    //
    // DOS image header is not present, so PE header is at the image base.
    //
    Hdr.Pe32 = (EFI_IMAGE_NT_HEADERS32 *) Pe32Data;
  }

  //
  // Calculate the entry point relative to the start of the image.
  // AddressOfEntryPoint is common for PE32 & PE32+
  //
  if (Hdr.Te->Signature == EFI_TE_IMAGE_HEADER_SIGNATURE) {
    *EntryPoint = (VOID *) ((UINTN) Pe32Data + (UINTN) (Hdr.Te->AddressOfEntryPoint & 0x0ffffffff) + sizeof (EFI_TE_IMAGE_HEADER) - Hdr.Te->StrippedSize);
    return RETURN_SUCCESS;
  } else if (Hdr.Pe32->Signature == EFI_IMAGE_NT_SIGNATURE) {
    *EntryPoint = (VOID *) ((UINTN) Pe32Data + (UINTN) (Hdr.Pe32->OptionalHeader.AddressOfEntryPoint & 0x0ffffffff));
    return RETURN_SUCCESS;
  }

  return RETURN_UNSUPPORTED;
}

VOID
AddImageStruct(
  IN UINTN  ImageBase,
  IN UINTN  ImageSize,
  IN CHAR16 *NameString
  )
{
  if (mImageStructCount >= mImageStructCountMax) {
    ASSERT(FALSE);
    return;
  }

  mImageStruct[mImageStructCount].ImageBase = ImageBase;
  mImageStruct[mImageStructCount].ImageSize = ImageSize;
  if (NameString != NULL) {
    StrnCpyS(mImageStruct[mImageStructCount].NameString, NAME_STRING_LENGTH + 1, NameString, NAME_STRING_LENGTH);
  }

  mImageStructCount++;
}

CHAR16 *
AddressToImageName(
  IN UINTN  Address
  )
{
  UINTN  Index;

  for (Index = 0; Index < mImageStructCount; Index++) {
    if ((Address >= mImageStruct[Index].ImageBase) &&
        (Address < mImageStruct[Index].ImageBase + mImageStruct[Index].ImageSize)) {
      return mImageStruct[Index].NameString;
    }
  }
  return UNKNOWN_NAME;
}

CHAR16 *
AddressToImageNameEx(
  IN UINTN     Address,
  IN EFI_GUID  *Protocol
  )
{
  CHAR16  *Name;
  UINTN   Index;
  Name = AddressToImageName(Address);
  if (StrCmp(Name, UNKNOWN_NAME) != 0) {
    return Name;
  }
  for (Index = 0; Index < sizeof(mProtocolFuncStruct) / sizeof(mProtocolFuncStruct[0]); Index++) {
    if (CompareGuid(Protocol, mProtocolFuncStruct[Index].Guid)) {
      return AddressToImageName(*(UINTN *)(Address + mProtocolFuncStruct[Index].FuncOffset));
    }
  }
  return UNKNOWN_NAME;
}

VOID
GetLoadedImage(
  VOID
  )
{
  EFI_STATUS                 Status;
  UINTN                      NoHandles;
  EFI_HANDLE                 *HandleBuffer;
  UINTN                      Index;
  EFI_LOADED_IMAGE_PROTOCOL  *LoadedImage;
  CHAR16                     *PathStr;
  CHAR16                     *NameStr;
  LOADED_IMAGE_PRIVATE_DATA  *LoadedImagePrivate;
  UINTN                      EntryPoint;
  VOID                       *EntryPointInImage;
  UINTN                      RealImageBase;

  Status = gBS->LocateHandleBuffer(
                  ByProtocol,
                  &gEfiLoadedImageProtocolGuid,
                  NULL,
                  &NoHandles,
                  &HandleBuffer
                  );
  if (EFI_ERROR(Status)) {
    return;
  }

  mImageStructCountMax = NoHandles;
  mImageStruct = AllocateZeroPool(mImageStructCountMax * sizeof(IMAGE_STRUCT));
  if (mImageStruct == NULL) {
    goto Done;
  }

  for (Index = 0; Index < NoHandles; Index++) {
    Status = gBS->HandleProtocol(
                    HandleBuffer[Index],
                    &gEfiLoadedImageProtocolGuid,
                    (VOID **)&LoadedImage
                    );
    if (EFI_ERROR(Status)) {
      continue;
    }
    PathStr = ConvertDevicePathToText(LoadedImage->FilePath, TRUE, TRUE);
    NameStr = GetDriverNameString(LoadedImage);
    Print(L"Image: %s ", NameStr);

    EntryPoint = 0;
    RealImageBase = (UINTN)LoadedImage->ImageBase;
    LoadedImagePrivate = BASE_CR(LoadedImage, LOADED_IMAGE_PRIVATE_DATA, Info);
    if (LoadedImagePrivate->Signature == LOADED_IMAGE_PRIVATE_DATA_SIGNATURE) {
      EntryPoint = (UINTN)LoadedImagePrivate->EntryPoint;
      if ((EntryPoint != 0) && ((EntryPoint < (UINTN)LoadedImage->ImageBase) || (EntryPoint >= ((UINTN)LoadedImage->ImageBase + (UINTN)LoadedImage->ImageSize)))) {
        //
        // If the EntryPoint is not in the range of image buffer, it should come from emulation environment.
        // So patch ImageBuffer here to align the EntryPoint.
        //
        Status = InternalPeCoffGetEntryPoint(LoadedImage->ImageBase, &EntryPointInImage);
        ASSERT_EFI_ERROR(Status);
        RealImageBase = (UINTN)LoadedImage->ImageBase + EntryPoint - (UINTN)EntryPointInImage;
      }
    }
    Print(L"(0x%x - 0x%x", LoadedImage->ImageBase, (UINTN)LoadedImage->ImageSize);
    if (EntryPoint != 0) {
      Print(L", EntryPoint:0x%x", EntryPoint);
    }
    if (RealImageBase != (UINTN)LoadedImage->ImageBase) {
      Print(L", Base:0x%x", RealImageBase);
    }
    Print(L")\n");
    Print(L"       (%s)\n", PathStr);

    AddImageStruct(RealImageBase, (UINTN)LoadedImage->ImageSize, NameStr);

    if (CompareMem(&mDxeCoreFilePath, LoadedImage->FilePath, sizeof(mDxeCoreFilePath)) == 0) {
      mDxeCoreLoadedImage = LoadedImage;
    }
  }

Done:
  gBS->FreePool(HandleBuffer);
  return;
}

VOID
DumpProtocolInterfacesOnHandle(
  IN IHANDLE    *IHandle
  )
{
  LIST_ENTRY           *ListEntry;
  PROTOCOL_INTERFACE   *ProtocolInterface;
  CHAR16               *PathStr;

  ListEntry = &IHandle->Protocols;
  for (ListEntry = ListEntry->ForwardLink;
       ListEntry != &IHandle->Protocols;
       ListEntry = ListEntry->ForwardLink) {
    ProtocolInterface = CR(ListEntry, PROTOCOL_INTERFACE, Link, PROTOCOL_INTERFACE_SIGNATURE);
    Print(L"  Protocol - %a, Interface - 0x%x", GuidToName(&ProtocolInterface->Protocol->ProtocolID), ProtocolInterface->Interface);
    if (ProtocolInterface->Interface != NULL) {
      CHAR16  *Name;
      Name = AddressToImageNameEx((UINTN)ProtocolInterface->Interface, &ProtocolInterface->Protocol->ProtocolID);
      if (StrCmp(Name, UNKNOWN_NAME) != 0) {
        Print(L" (%s)", Name);
      }
    }
    Print(L"\n");
    if (CompareGuid(&ProtocolInterface->Protocol->ProtocolID, &gEfiDevicePathProtocolGuid)) {
      PathStr = ConvertDevicePathToText(ProtocolInterface->Interface, TRUE, TRUE);
      Print(L"    (%s)\n", PathStr);
    }
    if (CompareGuid(&ProtocolInterface->Protocol->ProtocolID, &gEfiLoadedImageProtocolGuid)) {
      PathStr = ConvertDevicePathToText(((EFI_LOADED_IMAGE_PROTOCOL *)ProtocolInterface->Interface)->FilePath, TRUE, TRUE);
      Print(L"    (%s)\n", PathStr);
    }
    if (CompareGuid(&ProtocolInterface->Protocol->ProtocolID, &gEfiLoadedImageDevicePathProtocolGuid)) {
      PathStr = ConvertDevicePathToText(ProtocolInterface->Interface, TRUE, TRUE);
      Print(L"    (%s)\n", PathStr);
    }
  }
}

VOID
DumpHandleList(
  VOID
  )
{
  LIST_ENTRY      *ListEntry;
  IHANDLE         *Handle;

  ListEntry = mDxeCoreHandleList;
  for (ListEntry = ListEntry->ForwardLink;
       ListEntry != mDxeCoreHandleList;
       ListEntry = ListEntry->ForwardLink) {
    Handle = CR(ListEntry, IHANDLE, AllHandles, EFI_HANDLE_SIGNATURE);
    Print(L"Handle - 0x%x\n", Handle);
    DumpProtocolInterfacesOnHandle(Handle);
  }

  return;
}

VOID
GetDxeCoreHandleList(
  IN IHANDLE    *IHandle
  )
{
  LIST_ENTRY      *ListEntry;
  IHANDLE         *Handle;

  ListEntry = &IHandle->AllHandles;
  for (ListEntry = ListEntry->ForwardLink;
       ListEntry != &IHandle->AllHandles;
       ListEntry = ListEntry->ForwardLink) {
    Handle = BASE_CR(ListEntry, IHANDLE, AllHandles);
    if (Handle->Signature != EFI_HANDLE_SIGNATURE) {
      // BUGBUG: NT32 will load image to BS memory, but execute code in DLL.
      // Do not use mDxeCoreLoadedImage->ImageBase/ImageSize
      mDxeCoreHandleList = ListEntry;
      break;
    }
  }

  return;
}

VOID
GetDxeCoreProtocolDatabase(
  IN IHANDLE    *IHandle
  )
{
  LIST_ENTRY      *ListEntry;
  IHANDLE         *Handle;
  PROTOCOL_ENTRY  *IProtocolEntry;
  PROTOCOL_ENTRY  *ProtocolEntry;

  IProtocolEntry = NULL;
  ListEntry = &IHandle->AllHandles;
  for (ListEntry = ListEntry->ForwardLink;
       ListEntry != &IHandle->AllHandles;
       ListEntry = ListEntry->ForwardLink) {
    Handle = BASE_CR(ListEntry, IHANDLE, AllHandles);
    if (Handle->Signature == EFI_HANDLE_SIGNATURE) {
      LIST_ENTRY           *ProtocolListEntry;
      PROTOCOL_INTERFACE   *ProtocolInterface;

      ProtocolListEntry = &Handle->Protocols;
      for (ProtocolListEntry = ProtocolListEntry->ForwardLink;
           ProtocolListEntry != &Handle->Protocols;
           ProtocolListEntry = ProtocolListEntry->ForwardLink) {
        ProtocolInterface = BASE_CR(ProtocolListEntry, PROTOCOL_INTERFACE, Link);
        if (ProtocolInterface->Signature == PROTOCOL_INTERFACE_SIGNATURE) {
          IProtocolEntry = ProtocolInterface->Protocol;
        }
        if (IProtocolEntry != NULL) {
          break;
        }
      }
      if (IProtocolEntry != NULL) {
        break;
      }
    }
  }

  if (IProtocolEntry == NULL) {
    return;
  }

  ListEntry = &IProtocolEntry->AllEntries;
  for (ListEntry = ListEntry->ForwardLink;
       ListEntry != &IProtocolEntry->AllEntries;
       ListEntry = ListEntry->ForwardLink) {
    ProtocolEntry = BASE_CR(ListEntry, PROTOCOL_ENTRY, AllEntries);
    if (ProtocolEntry->Signature != PROTOCOL_ENTRY_SIGNATURE) {
      // BUGBUG: NT32 will load image to BS memory, but execute code in DLL.
      // Do not use mDxeCoreLoadedImage->ImageBase/ImageSize
      mDxeCoreProtocolDatabase = ListEntry;
      break;
    }
  }

  return;
}

VOID
DumpProtocolInterfacesOnProtocolEntry(
  IN PROTOCOL_ENTRY  *ProtocolEntry
  )
{
  LIST_ENTRY           *ListEntry;
  PROTOCOL_INTERFACE   *ProtocolInterface;
  CHAR16               *PathStr;

  ListEntry = &ProtocolEntry->Protocols;
  for (ListEntry = ListEntry->ForwardLink;
       ListEntry != &ProtocolEntry->Protocols;
       ListEntry = ListEntry->ForwardLink) {
    ProtocolInterface = CR(ListEntry, PROTOCOL_INTERFACE, ByProtocol, PROTOCOL_INTERFACE_SIGNATURE);
    Print(L"  Interface - 0x%x", ProtocolInterface->Interface);
    if (ProtocolInterface->Interface != NULL) {
      CHAR16  *Name;
      Name = AddressToImageNameEx((UINTN)ProtocolInterface->Interface, &ProtocolInterface->Protocol->ProtocolID);
      if (StrCmp(Name, UNKNOWN_NAME) != 0) {
        Print(L" (%s)", Name);
      }
    }
    Print(L"\n");
    if (CompareGuid(&ProtocolInterface->Protocol->ProtocolID, &gEfiDevicePathProtocolGuid)) {
      PathStr = ConvertDevicePathToText(ProtocolInterface->Interface, TRUE, TRUE);
      Print(L"    (%s)\n", PathStr);
    }
    if (CompareGuid(&ProtocolInterface->Protocol->ProtocolID, &gEfiLoadedImageProtocolGuid)) {
      PathStr = ConvertDevicePathToText(((EFI_LOADED_IMAGE_PROTOCOL *)ProtocolInterface->Interface)->FilePath, TRUE, TRUE);
      Print(L"    (%s)\n", PathStr);
    }
    if (CompareGuid(&ProtocolInterface->Protocol->ProtocolID, &gEfiLoadedImageDevicePathProtocolGuid)) {
      PathStr = ConvertDevicePathToText(ProtocolInterface->Interface, TRUE, TRUE);
      Print(L"    (%s)\n", PathStr);
    }
  }
}

VOID
DumpProtocolNotifyOnProtocolEntry(
  IN PROTOCOL_ENTRY  *ProtocolEntry
  )
{
  LIST_ENTRY           *ListEntry;
  PROTOCOL_NOTIFY      *ProtocolNotify;
  IEVENT               *IEvent;
  CHAR8                *EventGroupStr;

  ListEntry = &ProtocolEntry->Notify;
  for (ListEntry = ListEntry->ForwardLink;
       ListEntry != &ProtocolEntry->Notify;
       ListEntry = ListEntry->ForwardLink) {
    ProtocolNotify = CR(ListEntry, PROTOCOL_NOTIFY, Link, PROTOCOL_NOTIFY_SIGNATURE);
    IEvent = (IEVENT *)ProtocolNotify->Event;
    ASSERT(IEvent->Signature == EVENT_SIGNATURE);
    EventGroupStr = GuidToName(&IEvent->EventGroup);
    Print(L"  Event - 0x%x", IEvent);
    if (AsciiStrCmp (EventGroupStr, "ZeroGuid") != 0) {
      Print(L" (%a)", EventGroupStr);
    }
    if ((IEvent->Timer.TriggerTime != 0) || (IEvent->Timer.Period != 0)) {
      Print(L" (TiggerTime:%ld, Period:%ld)", IEvent->Timer.TriggerTime, IEvent->Timer.Period);
    }
    Print(L"\n");
    Print(L"    Type - 0x%x, TPL - 0x%x, Notify - 0x%x", IEvent->Type, IEvent->NotifyTpl, IEvent->NotifyFunction);
    if (IEvent->NotifyFunction != NULL) {
      CHAR16  *Name;
      Name = AddressToImageName((UINTN)IEvent->NotifyFunction);
      if (StrCmp(Name, UNKNOWN_NAME) != 0) {
        Print(L" (%s)", Name);
      }
    }
    Print(L"\n");
  }
}

VOID
DumpProtocolDatabase(
  VOID
  )
{
  LIST_ENTRY      *ListEntry;
  PROTOCOL_ENTRY  *ProtocolEntry;

  ListEntry = mDxeCoreProtocolDatabase;
  for (ListEntry = ListEntry->ForwardLink;
       ListEntry != mDxeCoreProtocolDatabase;
       ListEntry = ListEntry->ForwardLink) {
    ProtocolEntry = CR(ListEntry, PROTOCOL_ENTRY, AllEntries, PROTOCOL_ENTRY_SIGNATURE);
    Print(L"Protocol - %a\n", GuidToName(&ProtocolEntry->ProtocolID));
    DumpProtocolInterfacesOnProtocolEntry(ProtocolEntry);
    DumpProtocolNotifyOnProtocolEntry(ProtocolEntry);
  }

  return;
}

VOID
GetDxeCoreEventSignalQueue(
  IN IEVENT *IEvent
  )
{
  LIST_ENTRY      *ListEntry;
  IEVENT          *Event;

  ListEntry = &IEvent->SignalLink;
  for (ListEntry = ListEntry->ForwardLink;
       ListEntry != &IEvent->SignalLink;
       ListEntry = ListEntry->ForwardLink) {
    Event = BASE_CR(ListEntry, IEVENT, SignalLink);
    if (Event->Signature != EVENT_SIGNATURE) {
      // BUGBUG: NT32 will load image to BS memory, but execute code in DLL.
      // Do not use mDxeCoreLoadedImage->ImageBase/ImageSize
      mDxeCoreEventSignalQueue = ListEntry;
      break;
    }
  }

  return;
}

VOID
GetDxeCoreTimerList(
  IN IEVENT *IEvent
  )
{
  LIST_ENTRY      *ListEntry;
  IEVENT          *Event;

  ListEntry = &IEvent->Timer.Link;
  for (ListEntry = ListEntry->ForwardLink;
       ListEntry != &IEvent->Timer.Link;
       ListEntry = ListEntry->ForwardLink) {
    Event = BASE_CR(ListEntry, IEVENT, Timer.Link);
    if (Event->Signature != EVENT_SIGNATURE) {
      // BUGBUG: NT32 will load image to BS memory, but execute code in DLL.
      // Do not use mDxeCoreLoadedImage->ImageBase/ImageSize
      mDxeCoreEfiTimerList = ListEntry;
      break;
    }
  }

  return;
}

VOID
DumpEventSignalQueue(
  VOID
  )
{
  LIST_ENTRY      *ListEntry;
  IEVENT          *Event;
  CHAR8           *EventGroupStr;

  ListEntry = mDxeCoreEventSignalQueue;
  for (ListEntry = ListEntry->BackLink;
       ListEntry != mDxeCoreEventSignalQueue;
       ListEntry = ListEntry->BackLink) {
    Event = CR(ListEntry, IEVENT, SignalLink, EVENT_SIGNATURE);
    Print(L"Event - 0x%x", Event);
    EventGroupStr = GuidToName(&Event->EventGroup);
    if (AsciiStrCmp(EventGroupStr, "ZeroGuid") != 0) {
      Print(L" (%a)", EventGroupStr);
    }
    if ((Event->Timer.TriggerTime != 0) || (Event->Timer.Period != 0)) {
      Print(L" (TiggerTime:%ld, Period:%ld)", Event->Timer.TriggerTime, Event->Timer.Period);
    }
    Print(L"\n");
    Print(L"  Type - 0x%x, TPL - 0x%x, Notify - 0x%x", Event->Type, Event->NotifyTpl, Event->NotifyFunction);
    if (Event->NotifyFunction != NULL) {
      CHAR16  *Name;
      Name = AddressToImageName((UINTN)Event->NotifyFunction);
      if (StrCmp(Name, UNKNOWN_NAME) != 0) {
        Print(L" (%s)", Name);
      }
    }
    Print(L"\n");
  }

  return;
}

VOID
DumpEventTimerList(
  VOID
  )
{
  LIST_ENTRY      *ListEntry;
  IEVENT          *Event;
  CHAR8           *EventGroupStr;

  ListEntry = mDxeCoreEfiTimerList;
  for (ListEntry = ListEntry->BackLink;
       ListEntry != mDxeCoreEfiTimerList;
       ListEntry = ListEntry->BackLink) {
    Event = CR(ListEntry, IEVENT, Timer.Link, EVENT_SIGNATURE);
    Print(L"Event - 0x%x", Event);
    EventGroupStr = GuidToName(&Event->EventGroup);
    if (AsciiStrCmp(EventGroupStr, "ZeroGuid") != 0) {
      Print(L" (%a)", EventGroupStr);
    }
    if ((Event->Timer.TriggerTime != 0) || (Event->Timer.Period != 0)) {
      Print(L" (TiggerTime:%ld, Period:%ld)", Event->Timer.TriggerTime, Event->Timer.Period);
    }
    Print(L"\n");
    Print(L"  Type - 0x%x, TPL - 0x%x, Notify - 0x%x", Event->Type, Event->NotifyTpl, Event->NotifyFunction);
    if (Event->NotifyFunction != NULL) {
      CHAR16  *Name;
      Name = AddressToImageName((UINTN)Event->NotifyFunction);
      if (StrCmp(Name, UNKNOWN_NAME) != 0) {
        Print(L" (%s)", Name);
      }
    }
    Print(L"\n");
  }

  return;
}

VOID
EFIAPI
DummyFunc (
  EFI_EVENT                               Event,
  VOID                                    *Context
  )
{
}

EFI_STATUS
EFIAPI
DxeCoreDumpEntrypoint(
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS  Status;
  EFI_EVENT   DummyEvent;

  InitGuid();

  //
  // Dump all image
  //
  Print(L"##################\n");
  Print(L"# IMAGE DATABASE #\n");
  Print(L"##################\n");
  GetLoadedImage();
  if (mDxeCoreLoadedImage == NULL) {
    Print(L"DxeCore is not found!\n");
    goto Done;
  }
  Print(L"\n");

  GetDxeCoreHandleList((IHANDLE *)ImageHandle);
  GetDxeCoreProtocolDatabase((IHANDLE *)ImageHandle);
  if (mDxeCoreHandleList == NULL) {
    Print(L"DxeCore gHandleList is not found!\n");
    goto Done;
  }
  if (mDxeCoreProtocolDatabase == NULL) {
    Print(L"DxeCore mProtocolDatabase is not found!\n");
  }

  //
  // Dump handle list
  //
  Print(L"###################\n");
  Print(L"# HANDLE DATABASE #\n");
  Print(L"###################\n");
  DumpHandleList();
  Print(L"\n");

  //
  // Dump protocol database
  //
  Print(L"#####################\n");
  Print(L"# PROTOCOL DATABASE #\n");
  Print(L"#####################\n");
  DumpProtocolDatabase();
  Print(L"\n");

  //
  // Dump event signal queue
  //

  Print(L"##################\n");
  Print(L"# EVENT DATABASE #\n");
  Print(L"##################\n");
  // Create Dummy Event to get gEventSignalQueue
  Print(L"# 1. NOTIFY_SIGNAL #\n");
  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  DummyFunc,
                  NULL,
                  &gEfiEventReadyToBootGuid,
                  &DummyEvent
                  );
  if (!EFI_ERROR (Status)) {
    GetDxeCoreEventSignalQueue((IEVENT *)DummyEvent);
    gBS->CloseEvent(DummyEvent);
    if (mDxeCoreEventSignalQueue == NULL) {
      Print(L"DxeCore gEventSignalQueue is not found!\n");
      goto Done;
    }
    DumpEventSignalQueue();
  }
  Print(L"# 2. TIMER #\n");
  Status = gBS->CreateEvent (
                  EVT_TIMER,
                  TPL_NOTIFY,
                  DummyFunc,
                  NULL,
                  &DummyEvent
                  );
  if (!EFI_ERROR(Status)) {
    Status = gBS->SetTimer (
                    DummyEvent,
                    TimerPeriodic,
                    10 * 1000 * 1000
                    );
    if (!EFI_ERROR(Status)) {
      GetDxeCoreTimerList((IEVENT *)DummyEvent);
    }
    gBS->CloseEvent(DummyEvent);
    if (mDxeCoreEfiTimerList == NULL) {
      Print(L"DxeCore mEfiTimerList is not found!\n");
      goto Done;
    }
    DumpEventTimerList();
  }
  Print(L"\n");

  // BUGBUG: Filter myself

Done:
  if (mImageStruct != NULL) {
    FreePool(mImageStruct);
  }

  DeinitGuid();

  return EFI_SUCCESS;
}
