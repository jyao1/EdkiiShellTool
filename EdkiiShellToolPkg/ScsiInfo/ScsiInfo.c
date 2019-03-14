/** @file

Copyright (c) 2017, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the gBSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/gBSd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE gBSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <PiDxe.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/DevicePathLib.h>
#include <Protocol/PciIo.h>
#include <Protocol/ScsiPassThru.h>

//
// SCSI command OP Code
//
//
// Commands for all device types
//
#define EFI_SCSI_OP_CHANGE_DEFINITION 0x40
#define EFI_SCSI_OP_COMPARE           0x39
#define EFI_SCSI_OP_COPY              0x18
#define EFI_SCSI_OP_COPY_VERIFY       0x3a
#define EFI_SCSI_OP_INQUIRY           0x12
#define EFI_SCSI_OP_LOG_SELECT        0x4c
#define EFI_SCSI_OP_LOG_SENSE         0x4d
#define EFI_SCSI_OP_MODE_SEL6         0x15
#define EFI_SCSI_OP_MODE_SEL10        0x55
#define EFI_SCSI_OP_MODE_SEN6         0x1a
#define EFI_SCSI_OP_MODE_SEN10        0x5a
#define EFI_SCSI_OP_READ_BUFFER       0x3c
#define EFI_SCSI_OP_REQUEST_SENSE     0x03
#define EFI_SCSI_OP_SEND_DIAG         0x1d
#define EFI_SCSI_OP_TEST_UNIT_READY   0x00
#define EFI_SCSI_OP_WRITE_BUFF        0x3b

//
// Commands unique to Direct Access Devices
//
#define EFI_SCSI_OP_COMPARE         0x39
#define EFI_SCSI_OP_FORMAT          0x04
#define EFI_SCSI_OP_LOCK_UN_CACHE   0x36
#define EFI_SCSI_OP_PREFETCH        0x34
#define EFI_SCSI_OP_MEDIA_REMOVAL   0x1e
#define EFI_SCSI_OP_READ6           0x08
#define EFI_SCSI_OP_READ10          0x28
#define EFI_SCSI_OP_READ_CAPACITY   0x25
#define EFI_SCSI_OP_READ_DEFECT     0x37
#define EFI_SCSI_OP_READ_LONG       0x3e
#define EFI_SCSI_OP_REASSIGN_BLK    0x07
#define EFI_SCSI_OP_RECEIVE_DIAG    0x1c
#define EFI_SCSI_OP_RELEASE         0x17
#define EFI_SCSI_OP_REZERO          0x01
#define EFI_SCSI_OP_SEARCH_DATA_E   0x31
#define EFI_SCSI_OP_SEARCH_DATA_H   0x30
#define EFI_SCSI_OP_SEARCH_DATA_L   0x32
#define EFI_SCSI_OP_SEEK6           0x0b
#define EFI_SCSI_OP_SEEK10          0x2b
#define EFI_SCSI_OP_SEND_DIAG       0x1d
#define EFI_SCSI_OP_SET_LIMIT       0x33
#define EFI_SCSI_OP_START_STOP_UNIT 0x1b
#define EFI_SCSI_OP_SYNC_CACHE      0x35
#define EFI_SCSI_OP_VERIFY          0x2f
#define EFI_SCSI_OP_WRITE6          0x0a
#define EFI_SCSI_OP_WRITE10         0x2a
#define EFI_SCSI_OP_WRITE_VERIFY    0x2e
#define EFI_SCSI_OP_WRITE_LONG      0x3f
#define EFI_SCSI_OP_WRITE_SAME      0x41

//
// Commands unique to Sequential Access Devices
//
#define EFI_SCSI_OP_ERASE             0x19
#define EFI_SCSI_OP_LOAD_UNLOAD       0x1b
#define EFI_SCSI_OP_LOCATE            0x2b
#define EFI_SCSI_OP_READ_BLOCK_LIMIT  0x05
#define EFI_SCSI_OP_READ_POS          0x34
#define EFI_SCSI_OP_READ_REVERSE      0x0f
#define EFI_SCSI_OP_RECOVER_BUF_DATA  0x14
#define EFI_SCSI_OP_RESERVE_UNIT      0x16
#define EFI_SCSI_OP_REWIND            0x01
#define EFI_SCSI_OP_SPACE             0x11
#define EFI_SCSI_OP_VERIFY_TAPE       0x13
#define EFI_SCSI_OP_WRITE_FILEMARK    0x10

//
// Commands unique to Printer Devices
//
#define EFI_SCSI_OP_PRINT       0x0a
#define EFI_SCSI_OP_SLEW_PRINT  0x0b
#define EFI_SCSI_OP_STOP_PRINT  0x1b
#define EFI_SCSI_OP_SYNC_BUFF   0x10

//
// Commands unique to Processor Devices
//
#define EFI_SCSI_OP_RECEIVE 0x08
#define EFI_SCSI_OP_SEND    0x0a

//
// Commands unique to Write-Once Devices
//
#define EFI_SCSI_OP_MEDIUM_SCAN     0x38
#define EFI_SCSI_OP_SEARCH_DAT_E10  0x31
#define EFI_SCSI_OP_SEARCH_DAT_E12  0xb1
#define EFI_SCSI_OP_SEARCH_DAT_H10  0x30
#define EFI_SCSI_OP_SEARCH_DAT_H12  0xb0
#define EFI_SCSI_OP_SEARCH_DAT_L10  0x32
#define EFI_SCSI_OP_SEARCH_DAT_L12  0xb2
#define EFI_SCSI_OP_SET_LIMIT10     0x33
#define EFI_SCSI_OP_SET_LIMIT12     0xb3
#define EFI_SCSI_OP_VERIFY10        0x2f
#define EFI_SCSI_OP_VERIFY12        0xaf
#define EFI_SCSI_OP_WRITE12         0xaa
#define EFI_SCSI_OP_WRITE_VERIFY10  0x2e
#define EFI_SCSI_OP_WRITE_VERIFY12  0xae

//
// Commands unique to CD-ROM Devices
//
#define EFI_SCSI_OP_PLAY_AUD_10       0x45
#define EFI_SCSI_OP_PLAY_AUD_12       0xa5
#define EFI_SCSI_OP_PLAY_AUD_MSF      0x47
#define EFI_SCSI_OP_PLAY_AUD_TKIN     0x48
#define EFI_SCSI_OP_PLAY_TK_REL10     0x49
#define EFI_SCSI_OP_PLAY_TK_REL12     0xa9
#define EFI_SCSI_OP_READ_CD_CAPACITY  0x25
#define EFI_SCSI_OP_READ_HEADER       0x44
#define EFI_SCSI_OP_READ_SUB_CHANNEL  0x42
#define EFI_SCSI_OP_READ_TOC          0x43

//
// Commands unique to Scanner Devices
//
#define EFI_SCSI_OP_GET_DATABUFF_STAT 0x34
#define EFI_SCSI_OP_GET_WINDOW        0x25
#define EFI_SCSI_OP_OBJECT_POS        0x31
#define EFI_SCSI_OP_SCAN              0x1b
#define EFI_SCSI_OP_SET_WINDOW        0x24

//
// Commands unique to Optical Memory Devices
//
#define EFI_SCSI_OP_UPDATE_BLOCK  0x3d

//
// Commands unique to Medium Changer Devices
//
#define EFI_SCSI_OP_EXCHANGE_MEDIUM   0xa6
#define EFI_SCSI_OP_INIT_ELEMENT_STAT 0x07
#define EFI_SCSI_OP_POS_TO_ELEMENT    0x2b
#define EFI_SCSI_OP_REQUEST_VE_ADDR   0xb5
#define EFI_SCSI_OP_SEND_VOL_TAG      0xb6

//
// Commands unique to Communition Devices
//
#define EFI_SCSI_OP_GET_MESSAGE6    0x08
#define EFI_SCSI_OP_GET_MESSAGE10   0x28
#define EFI_SCSI_OP_GET_MESSAGE12   0xa8
#define EFI_SCSI_OP_SEND_MESSAGE6   0x0a
#define EFI_SCSI_OP_SEND_MESSAGE10  0x2a
#define EFI_SCSI_OP_SEND_MESSAGE12  0xaa

//
// SCSI Data Transfer Direction
//
#define EFI_SCSI_DATA_IN  0
#define EFI_SCSI_DATA_OUT 1

//
// Peripheral Device Type Definitions
//
#define EFI_SCSI_TYPE_DISK          0x00  // Disk device
#define EFI_SCSI_TYPE_TAPE          0x01  // Tape device
#define EFI_SCSI_TYPE_PRINTER       0x02  // Printer
#define EFI_SCSI_TYPE_PROCESSOR     0x03  // Processor
#define EFI_SCSI_TYPE_WORM          0x04  // Write-once read-multiple
#define EFI_SCSI_TYPE_CDROM         0x05  // CD-ROM device
#define EFI_SCSI_TYPE_SCANNER       0x06  // Scanner device
#define EFI_SCSI_TYPE_OPTICAL       0x07  // Optical memory device
#define EFI_SCSI_TYPE_MEDIUMCHANGER 0x08  // Medium Changer device
#define EFI_SCSI_TYPE_COMMUNICATION 0x09  // Communications device
#define EFI_SCSI_TYPE_RESERVED_LOW  0x0A  // Reserved (low)
#define EFI_SCSI_TYPE_RESERVED_HIGH 0x1E  // Reserved (high)
#define EFI_SCSI_TYPE_UNKNOWN       0x1F  // Unknown or no device type

#pragma pack(1)
//
// Data structures for scsi command use
//
typedef struct {
  UINT8 Peripheral_Type : 5;
  UINT8 Peripheral_Qualifier : 3;
  UINT8 DeviceType_Modifier : 7;  // Reserved_1
  UINT8 RMB : 1;
  UINT8 Version;
  UINT8 Response_Data_Format : 4;
  UINT8 HiSup : 1;
  UINT8 NormACA : 1;
  UINT8 TrmTsk : 1;
  UINT8 AERC : 1;
  UINT8 Addnl_Length;
  UINT8 Protect : 1;
  UINT8 Reserved_5 : 2;
  UINT8 _3PC : 1;
  UINT8 TPGS : 2;
  UINT8 ACC : 1;
  UINT8 SCCS : 1;
  UINT8 Addr16 : 1;
  UINT8 Addr32 : 1;
  UINT8 ACKREQQ: 1;
  UINT8 MChngr : 1;
  UINT8 MultiP : 1;
  UINT8 VS_1 : 1;
  UINT8 EncServ : 1;
  UINT8 BQue : 1;
  UINT8 VS_2 : 1;
  UINT8 CmdQue : 1;
  UINT8 TranDis : 1;
  UINT8 Linked : 1;
  UINT8 Sync : 1;
  UINT8 WBus16 : 1;
  UINT8 WBus32 : 1;
  UINT8 RelAdr : 1;
  CHAR8 Vendor_Indentification[8];
  CHAR8 Product_Indentification[16];
  CHAR8 Product_RevisionLevel[4];
  UINT8 Vendor_Specific[20];
  UINT8 IUS : 1;
  UINT8 QAS : 1;
  UINT8 Clocking : 2;
  UINT8 Reserved_56 : 4;
  UINT8 Reserved_57;
  UINT16 Vendor_Descriptor[8];
  UINT8 Reserved_74_95[22];
} EFI_SCSI_INQUIRY_DATA;

#pragma pack()

VOID
DumpScsiInquiryData (
  EFI_SCSI_INQUIRY_DATA   *ScsiData
  )
{
  UINTN Index;
 
  Print (L"    Peripheral_Type ...................................... 0x%02x\n", ScsiData->Peripheral_Type);
  switch (ScsiData->Peripheral_Type) {
  case 0:
    Print (L"      Direct-access device (e.g., magnetic disk)\n");
    break;
  case 1:
    Print (L"      Sequential-access device (e.g., magnetic tape)\n");
    break;
  case 2:
    Print (L"      Printer device\n");
    break;
  case 3:
    Print (L"      Processor device\n");
    break;
  case 4:
    Print (L"      Write-once device (e.g., some optical disks)\n");
    break;
  case 5:
    Print (L"      CD/DVD device\n");
    break;
  case 6:
    Print (L"      Scanner device\n");
    break;
  case 7:
    Print (L"      Optical memory device (e.g., some optical disks)\n");
    break;
  case 8:
    Print (L"      Medium changer device (e.g., jukeboxes)\n");
    break;
  case 9:
    Print (L"      Communications device\n");
    break;
  case 0xA:
  case 0xB:
    Print (L"      Graphic arts pre-press devices\n");
    break;
  case 0xC:
    Print (L"      Storage array controller device (e.g., RAID)\n");
    break;
  case 0xD:
    Print (L"      Enclosure services device\n");
    break;
  case 0xE:
    Print (L"      Simplified direct-access device (e.g., magnetic disk)\n");
    break;
  case 0xF:
    Print (L"      Optical card reader/writer device\n");
    break;
  case 0x11:
    Print (L"      Object-based Storage Device\n");
    break;
  case 0x12:
    Print (L"      Automation/Drive Interface\n");
    break;
  case 0x1E:
    Print (L"      Well known logical unit\n");
    break;
  case 0x1F:
    Print (L"      Unknown or no device type\n");
    break;
  default:
    break;
  }
  Print (L"    Peripheral_Qualifier ................................. 0x%02x\n", ScsiData->Peripheral_Qualifier);
  Print (L"    DeviceType_Modifier .................................. 0x%02x\n", ScsiData->DeviceType_Modifier);
  Print (L"    RMB .................................................. 0x%02x\n", ScsiData->RMB);
  Print (L"    Version .............................................. 0x%02x\n", ScsiData->Version);
  switch (ScsiData->Version) {
  case 1:
    Print (L"      (SCSI-1)\n");
    break;
  case 2:
    Print (L"      (SCSI-2)\n");
    break;
  case 3:
    Print (L"      (SPC)\n");
    break;
  case 4:
    Print (L"      (SPC-2)\n");
    break;
  case 5:
    Print (L"      (SPC-3)\n");
    break;
  default:
    break;
  }
  Print (L"    Response_Data_Format ................................. 0x%02x\n", ScsiData->Response_Data_Format);
  Print (L"    HiSup ................................................ 0x%02x\n", ScsiData->HiSup);
  Print (L"    NormACA .............................................. 0x%02x\n", ScsiData->NormACA);
  Print (L"    TrmTsk ............................................... 0x%02x\n", ScsiData->TrmTsk);
  Print (L"    AERC ................................................. 0x%02x\n", ScsiData->AERC);
  Print (L"    Addnl_Length ......................................... 0x%02x\n", ScsiData->Addnl_Length);
  Print (L"    Protect .............................................. 0x%02x\n", ScsiData->Protect);
  Print (L"    3PC .................................................. 0x%02x\n", ScsiData->_3PC);
  Print (L"    TPGS ................................................. 0x%02x\n", ScsiData->TPGS);
  Print (L"    ACC .................................................. 0x%02x\n", ScsiData->ACC);
  Print (L"    SCCS ................................................. 0x%02x\n", ScsiData->SCCS);
  Print (L"    Addr16 ............................................... 0x%02x\n", ScsiData->Addr16);
  Print (L"    Addr32 ............................................... 0x%02x\n", ScsiData->Addr32);
  Print (L"    ACKREQQ .............................................. 0x%02x\n", ScsiData->ACKREQQ);
  Print (L"    MChngr ............................................... 0x%02x\n", ScsiData->MChngr);
  Print (L"    MultiP ............................................... 0x%02x\n", ScsiData->MultiP);
  Print (L"    VS ................................................... 0x%02x\n", ScsiData->VS_1);
  Print (L"    EncServ .............................................. 0x%02x\n", ScsiData->EncServ);
  Print (L"    BQue ................................................. 0x%02x\n", ScsiData->BQue);
  Print (L"    VS ................................................... 0x%02x\n", ScsiData->VS_2);
  Print (L"    CmdQue ............................................... 0x%02x\n", ScsiData->CmdQue);
  Print (L"    TranDis .............................................. 0x%02x\n", ScsiData->TranDis);
  Print (L"    Linked ............................................... 0x%02x\n", ScsiData->Linked);
  Print (L"    Sync ................................................. 0x%02x\n", ScsiData->Sync);
  Print (L"    WBus16 ............................................... 0x%02x\n", ScsiData->WBus16);
  Print (L"    WBus32 ............................................... 0x%02x\n", ScsiData->WBus32);
  Print (L"    RelAdr ............................................... 0x%02x\n", ScsiData->RelAdr);
  Print (L"    Vendor_Indentification ............................... ");
  for (Index = 0; Index < 8; Index++) {
    Print (L"%c", ScsiData->Vendor_Indentification[Index]);
  }
  Print (L"\n");
  Print (L"    Product_Indentification .............................. ");
  for (Index = 0; Index < 16; Index++) {
    Print (L"%c", ScsiData->Product_Indentification[Index]);
  }
  Print (L"\n");
  Print (L"    Product_RevisionLevel ................................ ");
  for (Index = 0; Index < 4; Index++) {
    Print (L"%c", ScsiData->Product_RevisionLevel[Index]);
  }
  Print (L"\n");
  Print (L"    IUS .................................................. 0x%02x\n", ScsiData->IUS);
  Print (L"    QAS .................................................. 0x%02x\n", ScsiData->QAS);
  Print (L"    Clocking ............................................. 0x%02x\n", ScsiData->Clocking);
  for (Index = 0; Index < 8; Index++) {
    Print (L"    Vendor_Descriptor .................................... 0x%04x\n", ScsiData->Vendor_Descriptor[Index]);
  }
  Print (L"\n");
  
  return;
}

VOID
ScsiInfo (
  VOID
  )
{
  EFI_STATUS                             Status;
  UINTN                                  Index;
  UINTN                                  NoHandles;
  EFI_HANDLE                             *HandleBuffer;
  EFI_DEVICE_PATH_PROTOCOL               *DevicePath;
  CHAR16                                 *DevicePathStr;
  EFI_SCSI_PASS_THRU_PROTOCOL            *ScsiPassThru;
  EFI_SCSI_INQUIRY_DATA                  InquiryData; 
  UINT32                                 Target;
  UINT64                                 Lun;
  EFI_SCSI_PASS_THRU_SCSI_REQUEST_PACKET Packet;
  UINT8                                  Cdb[6];

  Status = gBS->LocateHandleBuffer (
                 ByProtocol,
                 &gEfiScsiPassThruProtocolGuid,
                 NULL,
                 &NoHandles,
                 &HandleBuffer
                 );
  if (EFI_ERROR (Status) || (NoHandles == 0)) {
    Print (L"ScsiPassThru not found\n");
    return ;
  }

  for (Index = 0; Index < NoHandles; Index++) {
    ScsiPassThru = NULL;
    Status = gBS->HandleProtocol (
                   HandleBuffer[Index],
                   &gEfiScsiPassThruProtocolGuid,
                   (VOID **)&ScsiPassThru
                   );
    if (EFI_ERROR (Status)) {
      Print (L"Get ScsiPassThru error - %r\n", Status);
      break;
    }

    DevicePath = NULL;
    Status = gBS->HandleProtocol (
                   HandleBuffer[Index],
                   &gEfiDevicePathProtocolGuid,
                   (VOID **)&DevicePath
                   );
    if (EFI_ERROR (Status)) {
      Print (L"Get Device Path error - %r\n", Status);
    } else {
      DevicePathStr = ConvertDevicePathToText(DevicePath, TRUE, TRUE);
      if (DevicePathStr != NULL) {
        Print (L"Device Path - %s\n", DevicePathStr);
        FreePool (DevicePathStr);
      }
    }

    Target = 0xFFFFFFFF;
    Lun = 0;
    while (TRUE) {
      Status = ScsiPassThru->GetNextDevice (ScsiPassThru, &Target, &Lun);
      if (EFI_ERROR (Status)) {
        //
        // Test Over
        //
        break;
      }

      Print (L"  Target - %d, Lun - %ld\n", Target, Lun);

      //
      // Initialize the Request Packet.
      //
      SetMem (&Packet, sizeof (EFI_SCSI_PASS_THRU_SCSI_REQUEST_PACKET), 0);
      SetMem (Cdb, 6, 0);
      SetMem (&InquiryData, sizeof (EFI_SCSI_INQUIRY_DATA), 0);
  
      // Set to OP_INQUIRY.
      Cdb[0] = EFI_SCSI_OP_INQUIRY;
      Cdb[1] = (UINT8)(((UINT8)Lun << 5) & 0xE0); // Compatibility support
      Cdb[4] = sizeof (EFI_SCSI_INQUIRY_DATA);

      Packet.Timeout = 0;
      Packet.Cdb = Cdb;
      Packet.CdbLength = 6;
      Packet.DataBuffer = &InquiryData;
      Packet.TransferLength = sizeof (EFI_SCSI_INQUIRY_DATA);
      Packet.DataDirection = 0;

      //
      // send SCSI command
      //
      Status = ScsiPassThru->PassThru (ScsiPassThru, Target, Lun, &Packet, NULL);
      if (EFI_ERROR (Status)) {
//        Print (L" Send Packet error - %r\n", Status);
      } else {
        DumpScsiInquiryData (&InquiryData);
      }
    }
  }
  
  return ;
}

EFI_STATUS
EFIAPI
ScsiInfoEntryPoint (
  IN EFI_HANDLE                   ImageHandle,
  IN EFI_SYSTEM_TABLE             *SystemTable
  )
{
  ScsiInfo ();
   
  return EFI_SUCCESS;
}
