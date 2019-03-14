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
#include <IndustryStandard/Pci.h>

#define PCI_CLASS_MASS_STORAGE  0x01
#define PCI_SUB_CLASS_IDE       0x01
#define PCI_SUB_CLASS_RAID      0x04
#define PCI_SUB_CLASS_SATADPA   0x06

//
//******************************************************
// EFI_ATA_EXT_TRANSFER_PROTOCOL
//******************************************************
//
// This extended mode describes the SATA physical protocol.
// SATA physical layers can operate at different speeds. 
// These speeds are defined below. Various PATA protocols 
// and associated modes are not applicable to SATA devices.
//

typedef enum {
  EfiAtaSataTransferProtocol  
} EFI_ATA_EXT_TRANSFER_PROTOCOL;

#define  EFI_SATA_AUTO_SPEED  0
#define  EFI_SATA_GEN1_SPEED  1
#define  EFI_SATA_GEN2_SPEED  2

//*******************************************************
// EFI_IDE_CABLE_TYPE
//*******************************************************
typedef enum {
  EfiIdeCableTypeUnknown,
  EfiIdeCableType40pin,
  EfiIdeCableType80Pin,
  EfiIdeCableTypeSerial,
  EfiIdeCableTypeMaximum
} EFI_IDE_CABLE_TYPE;

//******************************************************
// EFI_ATA_MODE
//******************************************************
typedef struct {
  BOOLEAN  Valid;
  UINT32       Mode; 
} EFI_ATA_MODE;

//******************************************************
// EFI_ATA_EXTENDED_MODE
//******************************************************
typedef struct {
  EFI_ATA_EXT_TRANSFER_PROTOCOL  TransferProtocol;
  UINT32                         Mode;
} EFI_ATA_EXTENDED_MODE;

//******************************************************
// EFI_ATA_COLLECTIVE_MODE
//******************************************************
typedef struct {
  EFI_ATA_MODE           PioMode; 
  EFI_ATA_MODE           SingleWordDmaMode;
  EFI_ATA_MODE           MultiWordDmaMode;
  EFI_ATA_MODE           UdmaMode;
  UINT32                 ExtModeCount;
  EFI_ATA_EXTENDED_MODE  ExtMode[1]; 
} EFI_ATA_COLLECTIVE_MODE;

//
//*******************************************************
// EFI_ATA_IDENTIFY_DATA
//*******************************************************
//

#pragma pack(1)

typedef struct {   
  UINT16  config;             // General Configuration
  UINT16  cylinders;          // Number of Cylinders
  UINT16  reserved_2;
  UINT16  heads;              //Number of logical heads
  UINT16  vendor_data1;
  UINT16  vendor_data2;
  UINT16  sectors_per_track;
  UINT16  vendor_specific_7_9[3];
  CHAR8   SerialNo[20];       // ASCII 
  UINT16  vendor_specific_20_21[2]; 
  UINT16  ecc_bytes_available;   
  CHAR8   FirmwareVer[8];     // ASCII 
  CHAR8   ModelName[40];      // ASCII   
  UINT16  multi_sector_cmd_max_sct_cnt;
  UINT16  reserved_48;
  UINT16  capabilities;
  UINT16  reserved_50;    
  UINT16  pio_cycle_timing;   
  UINT16  reserved_52;            
  UINT16  field_validity;    
  UINT16  current_cylinders;
  UINT16  current_heads;
  UINT16  current_sectors;   
  UINT16  CurrentCapacityLsb;
  UINT16  CurrentCapacityMsb;    
  UINT16  reserved_59;    
  UINT16  user_addressable_sectors_lo;
  UINT16  user_addressable_sectors_hi;
  UINT16  reserved_62;    
  UINT16  multi_word_dma_mode;   
  UINT16  advanced_pio_modes;
  UINT16  min_multi_word_dma_cycle_time;
  UINT16  rec_multi_word_dma_cycle_time;
  UINT16  min_pio_cycle_time_without_flow_control;
  UINT16  min_pio_cycle_time_with_flow_control;
  UINT16  reserved_69_79[11];    
  UINT16  major_version_no;
  UINT16  minor_version_no;
  UINT16  command_set_supported_82; // word 82
  UINT16  command_set_supported_83; // word 83
  UINT16  command_set_feature_extn; // word 84
  UINT16  command_set_feature_enb_85; // word 85
  UINT16  command_set_feature_enb_86; // word 86
  UINT16  command_set_feature_default; // word 87
  UINT16  ultra_dma_mode; // word 88
  UINT16  reserved_89_127[39];
  UINT16  security_status;
  UINT16  vendor_data_129_159[31];
  UINT16  reserved_160_255[96];
} EFI_ATA_IDENTIFY_DATA;

#pragma pack()

//*******************************************************
// EFI_ATAPI_IDENTIFY_DATA
//*******************************************************
#pragma pack(1)
typedef struct {
    UINT16  config;             // General Configuration
    UINT16  obsolete_1;
    UINT16  specific_config;
    UINT16  obsolete_3;   
    UINT16  retired_4_5[2];
    UINT16  obsolete_6;   
    UINT16  cfa_reserved_7_8[2];
    UINT16  retired_9;
    CHAR8   SerialNo[20];       // ASCII 
    UINT16  retired_20_21[2];
    UINT16  obsolete_22;
    CHAR8   FirmwareVer[8];     // ASCII 
    CHAR8   ModelName[40];      // ASCII 
    UINT16  multi_sector_cmd_max_sct_cnt;
    UINT16  reserved_48;
    UINT16  capabilities_49;
    UINT16  capabilities_50;
    UINT16  obsolete_51_52[2];   
    UINT16  field_validity;
    UINT16  obsolete_54_58[5];
    UINT16  mutil_sector_setting;
    UINT16  user_addressable_sectors_lo;
    UINT16  user_addressable_sectors_hi;
    UINT16  obsolete_62;
    UINT16  multi_word_dma_mode;
    UINT16  advanced_pio_modes;
    UINT16  min_multi_word_dma_cycle_time;
    UINT16  rec_multi_word_dma_cycle_time;
    UINT16  min_pio_cycle_time_without_flow_control;
    UINT16  min_pio_cycle_time_with_flow_control;
    UINT16  reserved_69_74[6];
    UINT16  queue_depth;
    UINT16  reserved_76_79[4];
    UINT16  major_version_no;
    UINT16  minor_version_no;
    UINT16  cmd_set_support_82;
    UINT16  cmd_set_support_83;
    UINT16  cmd_feature_support;
    UINT16  cmd_feature_enable_85;
    UINT16  cmd_feature_enable_86;
    UINT16  cmd_feature_default;
    UINT16  ultra_dma_select;
    UINT16  time_required_for_sec_erase;
    UINT16  time_required_for_enhanced_sec_erase;
    UINT16  current_advanced_power_mgmt_value;
    UINT16  master_pwd_revison_code;
    UINT16  hardware_reset_result;
    UINT16  current_auto_acoustic_mgmt_value;
    UINT16  reserved_95_99[5];
    UINT16  max_user_lba_for_48bit_addr[4];
    UINT16  reserved_104_126[23];
    UINT16  removable_media_status_notification_support;
    UINT16  security_status;
    UINT16  vendor_data_129_159[31];
    UINT16  cfa_power_mode;
    UINT16  cfa_reserved_161_175[15];
    UINT16  current_media_serial_no[30];
    UINT16  reserved_206_254[49];
    UINT16  integrity_word;
} EFI_ATAPI_IDENTIFY_DATA;

#pragma pack()

//*******************************************************
// EFI_IDENTIFY_DATA
//*******************************************************
typedef union {
  EFI_ATA_IDENTIFY_DATA      AtaData;
  EFI_ATAPI_IDENTIFY_DATA    AtapiData;
} EFI_IDENTIFY_DATA; 

#define   EFI_ATAPI_DEVICE_IDENTIFY_DATA  0x8000

//*******************************************************
//EFI_ATA_VENDOR_ATTRIBUTE
//*******************************************************

#pragma pack(1)
typedef struct{
  UINT8  Id;
  UINT16 Flags; 
  UINT8  Current;
  UINT8  Worst;
  UINT8  Raw[6];
  UINT8  Reserv;
} EFI_ATA_VENDOR_ATTRIBUTE;
#pragma pack()

//*******************************************************
// EFI_ATA_SMART_DATA
//*******************************************************

#define  NUMBER_ATA_SMART_ATTRIBUTES 30

#pragma pack(1)
typedef struct {
  UINT16 Revsion_number;
  EFI_ATA_VENDOR_ATTRIBUTE Vendor_Attributes [NUMBER_ATA_SMART_ATTRIBUTES];
  UINT8  Offline_Data_Collection_Status;
  UINT8  Self_Test_Exec_Status;  
  UINT16 Total_Time_To_Complete_Off_Line; 
  UINT8  Vendor_Specific_366; 
  UINT8  Offline_Data_Collection_Capability;
  UINT16 Smart_Capability;
  UINT8  Errorlog_Capability;
  UINT8  Vendor_Specific_371; 
  UINT8  Short_Test_Completion_Time;
  UINT8  Extend_Test_Completion_Time;
  UINT8  Conveyance_Test_Completion_Time;
  UINT8  Reserved_375_385[11];
  UINT8  Vendor_Specific_386_510[125]; 
  UINT8  Chksum;
} EFI_ATA_SMART_DATA; 
#pragma pack()

VOID
DumpAtaIdentifyData (
  EFI_ATA_IDENTIFY_DATA   *AtaData
  );

VOID
DumpAtapiIdentifyData (
  EFI_ATAPI_IDENTIFY_DATA *AtapiData
  );

VOID
DumpAtaSmartData(
  EFI_ATA_SMART_DATA  *AtaSmartData
  );

//////////////////
// Defined data //
//////////////////

#define PCI_CLASS_MASS_STORAGE    0x01
#define PCI_SUB_CLASS_IDE         0x01


//
// IDE Registers 
//
typedef union 
{
  UINT16  Command;    /* when write */
  UINT16  Status;     /* when read */
} IDE_CMD_OR_STATUS;

typedef union {
  UINT16  Error;      /* when read */
  UINT16  Feature;    /* when write */ 
} IDE_ERROR_OR_FEATURE;

typedef union {
  UINT16 AltStatus;       /* when read */
  UINT16 DeviceControl;   /* when write */
} IDE_AltStatus_OR_DeviceControl;

//
// IDE registers set 
//
typedef struct {
  UINT16                  Data;
  IDE_ERROR_OR_FEATURE    Reg1;                  
  UINT16                  SectorCount;
  UINT16                  SectorNumber;
  UINT16                  CylinderLsb;
  UINT16                  CylinderMsb;
  UINT16                  Head;
  IDE_CMD_OR_STATUS       Reg;                   
  IDE_AltStatus_OR_DeviceControl       Alt;
  UINT16                  DriveAddress;
  UINT16                  MasterSlave;
} IDE_BASE_REGISTERS;

typedef enum {
    IdeMagnetic,                   /* ZIP Drive or LS120 Floppy Drive */
    IdeCdRom,                      /* ATAPI CDROM */
    IdeHardDisk,                   /* Hard Disk */
    Ide48bitAddressingHardDisk,    /* Hard Disk larger than 120GB */
    IdeUnknown
} IDE_DEVICE_TYPE;

typedef struct {
  UINT32                        Signature;
  EFI_PCI_IO_PROTOCOL           *PciIo;
  UINT8                         Channel;
  UINT8                         Device;
  IDE_DEVICE_TYPE               Type;
  EFI_IDENTIFY_DATA             *pIdData;
  EFI_ATA_SMART_DATA            *pSmartData;
  IDE_BASE_REGISTERS            *IoPort;
} IDE_DEV;

#define IDE_PRIMARY_OPERATING_MODE            1
#define IDE_SECONDARY_OPERATING_MODE          4

typedef struct {
  UINT16  CommandBlockBaseAddr;
  UINT16  ControlBlockBaseAddr;
  UINT16  BusMasterBaseAddr;
} IDE_REGISTERS_BASE_ADDR;

//
// bit definition 
//
#define bit(a)    (1 << (a))

//
// IDE registers bit definitions
//

// Err Reg
#define BBK_ERR     bit(7)    /* Bad block detected */
#define UNC_ERR     bit(6)    /* Uncorrectable Data */
#define MC_ERR      bit(5)    /* Media Change */
#define IDNF_ERR    bit(4)    /* ID Not Found */
#define MCR_ERR     bit(3)    /* Media Change Requested */
#define ABRT_ERR    bit(2)    /* Aborted Command */
#define TK0NF_ERR   bit(1)    /* Track 0 Not Found */
#define AMNF_ERR    bit(0)    /* Address Mark Not Found */

// Device/Head Reg
#define LBA_MODE    bit(6)
#define DEV    bit(4)
#define HS3 bit(3)
#define HS2 bit(2)
#define HS1 bit(1)
#define HS0 bit(0)
#define CHS_MODE    (0)
#define DRV0    (0)
#define DRV1    (1)
#define MST_DRV DRV0
#define SLV_DRV DRV1

//Status Reg
#define BSY     bit(7)    /* Controller Busy */ 
#define DRDY    bit(6)    /* Drive Ready */             
#define DWF     bit(5)    /* Drive Write Fault */       
#define DSC     bit(4)    /* Disk Seek Complete */      
#define DRQ     bit(3)    /* Data Request */            
#define CORR    bit(2)    /* Corrected Data */          
#define IDX     bit(1)    /* Index */                   
#define ERR     bit(0)    /* Error */                   

// Device Control Reg
#define  SRST   bit(2)    /* Software Reset */
#define  IEN_L  bit(1)    /* Interrupt Enable #*/

//
// ATA Commands Code
//
#define IDENTIFY_DRIVE_CMD              0xEC
#define ATAPI_IDENTIFY_DEVICE_CMD       0xA1
#define ATA_SMART_CMD                   0xB0
#define ATA_SMART_CMD_READDATA          0xD0

#define STALL_1_MILLI_SECOND    1000    // stall 1 ms
#define STALL_1_SECOND          1000000 // stall 1 second

// ATATIMEOUT is used for waiting time out for ATA device
#define ATATIMEOUT    1000    // 1 second 

#define MAX_28BIT_ADDRESSING_CAPACITY    0xfffffff

typedef enum {
  IdeCtrlTestCalculateModesBeforeSumbitData,
  IdeCtrlTestCalculateModesBeforeDisqualifyModes
} IDE_CTRL_INIT_TEST_CALCULATE_MODES_PHASE;

typedef struct {
  UINT32    *Mode;
  UINT32    Number;
} ATA_MODE_LIST;

STATIC  
EFI_STATUS
DiscoverIdeDevice (
  IN IDE_DEV *IdeDev
  );

STATIC
EFI_STATUS
DetectIDEController (
  IN  IDE_DEV  *IdeDev
  );
  
STATIC
EFI_STATUS
CheckPowerMode (
  IDE_DEV    *IdeDev
  );
  
STATIC
EFI_STATUS                                                         
ATAIdentify (                                                  
  IN  IDE_DEV  *IdeDev
  );
  
STATIC
EFI_STATUS
AtaAtapi6Identify (
  IN  IDE_DEV  *IdeDev
  );

STATIC
EFI_STATUS
ATAPIIdentify (                                                    
  IN  IDE_DEV  *IdeDev
  );

STATIC
UINT8
IDEReadPortB (
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT16                Port
  );

STATIC
VOID
IDEWritePortB (
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT16                Port, 
  IN  UINT8                 Data
  );
  
STATIC
VOID
IDEReadPortWMultiple (
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT16                Port, 
  IN  UINTN                 Count,
  IN  VOID                  *Buffer
  );
  
STATIC
EFI_STATUS  
WaitForBSYClear (
  IN  IDE_DEV         *IdeDev,
  IN  UINTN           TimeoutInMilliSeconds
  );

STATIC
EFI_STATUS  
DRDYReady (
  IN  IDE_DEV         *IdeDev,
  IN  UINTN           DelayInMilliSeconds
  );
  
STATIC
EFI_STATUS  
DRDYReady2 (
  IN  IDE_DEV         *IdeDev,
  IN  UINTN           DelayInMilliSeconds
  );

STATIC
EFI_STATUS  
DRQReady (
  IN  IDE_DEV         *IdeDev,
  IN  UINTN           TimeoutInMilliSeconds
  );
  
STATIC
EFI_STATUS  
DRQReady2 (
  IN  IDE_DEV         *IdeDev,
  IN  UINTN           TimeoutInMilliSeconds
  );
  
STATIC
EFI_STATUS  
DRQClear (
  IN  IDE_DEV         *IdeDev,
  IN  UINTN           TimeoutInMilliSeconds
  );
  
STATIC
EFI_STATUS  
DRQClear2 (
  IN  IDE_DEV  *IdeDev,
  IN  UINTN    TimeoutInMilliSeconds
  );
 
STATIC
EFI_STATUS
CheckErrorStatus (
  IN  IDE_DEV  *IdeDev
  );
  
STATIC
EFI_STATUS
AtaPioDataIn (
  IN  IDE_DEV         *IdeDev, 
  IN  VOID            *Buffer,
  IN  UINT32          ByteCount,
  IN  UINT8           AtaCommand,
  IN  UINT8           Head,
  IN  UINT8           Feature,
  IN  UINT8           SectorCount,
  IN  UINT8           SectorNumber,
  IN  UINT8           CylinderLsb,
  IN  UINT8           CylinderMsb
  );

EFI_STATUS
GetIdeRegistersBaseAddr (
  IN  EFI_PCI_IO_PROTOCOL         *PciIo,
  OUT IDE_REGISTERS_BASE_ADDR     *IdeRegsBaseAddr
  );

STATIC
UINT8
IsBigEndian (
  VOID
  );

STATIC
VOID
SwapSmartData (
  IN EFI_ATA_SMART_DATA *SmartData
  );

STATIC 
VOID
SwapBytes (
  IN UINT8  *Swap
  );

STATIC
UINT16
AtaReturnAttributeRawValue (
  IN UINT8              Id,
  IN EFI_ATA_SMART_DATA *SmartData
  );

VOID
AtaInfo (
  VOID
  );

BOOLEAN mSlaveDeviceExist = FALSE;
BOOLEAN mMasterDeviceExist = FALSE;

VOID
DumpAtaSmartData (
EFI_ATA_SMART_DATA *AtaSmartData
 )
{
  UINTN                    Index;
  UINT16                   Value;
  EFI_ATA_VENDOR_ATTRIBUTE *Attribute;

  if (AtaSmartData) {
    SwapSmartData(AtaSmartData);
    Print (L"\n    S.M.A.R.T.\n");
    for (Index = 0; Index < NUMBER_ATA_SMART_ATTRIBUTES; Index++) {
      Attribute = AtaSmartData->Vendor_Attributes + Index;
      if (Attribute->Id) {
        switch (Attribute->Id) {
        case 194:
          Value = AtaReturnAttributeRawValue (194, AtaSmartData);
          Print (L"      The HDD Temperature ................................ %d C\n", Value);
          break;
        default:
          break;
        }
      }
    }
  } else {
    Print (L"\n    S.M.A.R.T. can not been read.\n");
  }
}

VOID
DumpAtaIdentifyData (
  EFI_ATA_IDENTIFY_DATA   *AtaData
  )
{
  UINTN Index;
 
  Print (L"    config ............................................... 0x%04x\n", AtaData->config);
  Print (L"    cylinders ............................................ 0x%04x\n", AtaData->cylinders);
  Print (L"    heads ................................................ 0x%04x\n", AtaData->heads);
  Print (L"    vendor_data1 ......................................... 0x%04x\n", AtaData->vendor_data1);
  Print (L"    vendor_data2 ......................................... 0x%04x\n", AtaData->vendor_data2);
  Print (L"    sectors_per_track .................................... 0x%04x\n", AtaData->sectors_per_track);
  Print (L"    SerialNo ............................................. ");
  for (Index = 0; Index < 10; Index++) {
    Print (L"%c", AtaData->SerialNo[Index * 2 + 1]);
    Print (L"%c", AtaData->SerialNo[Index * 2]);
  }
  Print (L"\n");
  Print (L"    ecc_bytes_available .................................. 0x%04x\n", AtaData->ecc_bytes_available);
  Print (L"    FirmwareVer .......................................... ");
  for (Index = 0; Index < 4; Index++) {
    Print (L"%c", AtaData->FirmwareVer[Index * 2 + 1]);
    Print (L"%c", AtaData->FirmwareVer[Index * 2]);
  }
  Print (L"\n");
  Print (L"    ModelName ............................................ ");
  for (Index = 0; Index < 20; Index++) {
    Print (L"%c", AtaData->ModelName[Index * 2 + 1]);
    Print (L"%c", AtaData->ModelName[Index * 2]);
  }
  Print (L"\n");
  Print (L"    multi_sector_cmd_max_sct_cnt ......................... 0x%04x\n", AtaData->multi_sector_cmd_max_sct_cnt);
  Print (L"    capabilities ......................................... 0x%04x\n", AtaData->capabilities);
  Print (L"    pio_cycle_timing ..................................... 0x%04x\n", AtaData->pio_cycle_timing);
  Print (L"    field_validity ....................................... 0x%04x\n", AtaData->field_validity);
  Print (L"    current_cylinders .................................... 0x%04x\n", AtaData->current_cylinders);
  Print (L"    current_heads ........................................ 0x%04x\n", AtaData->current_heads);
  Print (L"    current_sectors ...................................... 0x%04x\n", AtaData->current_sectors);
  Print (L"    CurrentCapacityLsb ................................... 0x%04x\n", AtaData->CurrentCapacityLsb);
  Print (L"    CurrentCapacityMsb ................................... 0x%04x\n", AtaData->CurrentCapacityMsb);
  Print (L"    user_addressable_sectors_lo .......................... 0x%04x\n", AtaData->user_addressable_sectors_lo);
  Print (L"    user_addressable_sectors_hi .......................... 0x%04x\n", AtaData->user_addressable_sectors_hi);
  Print (L"    multi_word_dma_mode .................................. 0x%04x\n", AtaData->multi_word_dma_mode);
  Print (L"    advanced_pio_modes ................................... 0x%04x\n", AtaData->advanced_pio_modes);
  Print (L"    min_multi_word_dma_cycle_time ........................ 0x%04x\n", AtaData->min_multi_word_dma_cycle_time);
  Print (L"    rec_multi_word_dma_cycle_time ........................ 0x%04x\n", AtaData->rec_multi_word_dma_cycle_time);
  Print (L"    min_pio_cycle_time_without_flow_control .............. 0x%04x\n", AtaData->min_pio_cycle_time_without_flow_control);
  Print (L"    min_pio_cycle_time_with_flow_control ................. 0x%04x\n", AtaData->min_pio_cycle_time_with_flow_control);
  Print (L"    major_version_no ..................................... 0x%04x\n", AtaData->major_version_no);
  Print (L"    minor_version_no ..................................... 0x%04x\n", AtaData->minor_version_no);
  Print (L"    command_set_supported_82 ............................. 0x%04x\n", AtaData->command_set_supported_82);
  Print (L"    command_set_supported_83 ............................. 0x%04x\n", AtaData->command_set_supported_83);
  Print (L"    command_set_feature_extn ............................. 0x%04x\n", AtaData->command_set_feature_extn);
  Print (L"    command_set_feature_enb_85 ........................... 0x%04x\n", AtaData->command_set_feature_enb_85);
  Print (L"    command_set_feature_enb_86 ........................... 0x%04x\n", AtaData->command_set_feature_enb_86);
  Print (L"    command_set_feature_default .......................... 0x%04x\n", AtaData->command_set_feature_default);
  Print (L"    ultra_dma_mode ....................................... 0x%04x\n", AtaData->ultra_dma_mode);
  switch (AtaData->ultra_dma_mode & 0xFF00) {
  case 0x0100:
    Print (L"      UDMA mode 0\n");
    break;
  case 0x0200:
    Print (L"      UDMA mode 1\n");
    break;
  case 0x0400:
    Print (L"      UDMA mode 2\n");
    break;
  case 0x0800:
    Print (L"      UDMA mode 3\n");
    break;
  case 0x1000:
    Print (L"      UDMA mode 4\n");
    break;
  case 0x2000:
    Print (L"      UDMA mode 5\n");
    break;
  case 0x4000:
    Print (L"      UDMA mode 6\n");
    break;
  default:
    break;
  }
  Print (L"    security_status ...................................... 0x%04x\n", AtaData->security_status);

  return;
}

VOID
DumpAtapiIdentifyData (
  EFI_ATAPI_IDENTIFY_DATA *AtapiData
  )
{
  UINTN Index;
  
  Print (L"    config ............................................... 0x%04x\n", AtapiData->config);
  Print (L"    specific_config ...................................... 0x%04x\n", AtapiData->specific_config);
  Print (L"    SerialNo ............................................. ");
  for (Index = 0; Index < 20; Index++) {
    Print (L"%c", AtapiData->SerialNo[Index]);
  }
  Print (L"\n");
  Print (L"    FirmwareVer .......................................... ");
  for (Index = 0; Index < 8; Index++) {
    Print (L"%c", AtapiData->FirmwareVer[Index]);
  }
  Print (L"\n");
  Print (L"    ModelName ............................................ ");
  for (Index = 0; Index < 40; Index++) {
    Print (L"%c", AtapiData->ModelName[Index]);
  }
  Print (L"\n");
  Print (L"    multi_sector_cmd_max_sct_cnt ......................... 0x%04x\n", AtapiData->multi_sector_cmd_max_sct_cnt);
  Print (L"    capabilities_49 ...................................... 0x%04x\n", AtapiData->capabilities_49);
  Print (L"    capabilities_50 ...................................... 0x%04x\n", AtapiData->capabilities_50);
  Print (L"    field_validity ....................................... 0x%04x\n", AtapiData->field_validity);
  Print (L"    mutil_sector_setting ................................. 0x%04x\n", AtapiData->mutil_sector_setting);
  Print (L"    user_addressable_sectors_lo .......................... 0x%04x\n", AtapiData->user_addressable_sectors_lo);
  Print (L"    user_addressable_sectors_hi .......................... 0x%04x\n", AtapiData->user_addressable_sectors_hi);
  Print (L"    multi_word_dma_mode .................................. 0x%04x\n", AtapiData->multi_word_dma_mode);
  Print (L"    advanced_pio_modes ................................... 0x%04x\n", AtapiData->advanced_pio_modes);
  Print (L"    min_multi_word_dma_cycle_time ........................ 0x%04x\n", AtapiData->min_multi_word_dma_cycle_time);
  Print (L"    rec_multi_word_dma_cycle_time ........................ 0x%04x\n", AtapiData->rec_multi_word_dma_cycle_time);
  Print (L"    min_pio_cycle_time_without_flow_control .............. 0x%04x\n", AtapiData->min_pio_cycle_time_without_flow_control);
  Print (L"    min_pio_cycle_time_with_flow_control ................. 0x%04x\n", AtapiData->min_pio_cycle_time_with_flow_control);
  Print (L"    queue_depth .......................................... 0x%04x\n", AtapiData->queue_depth);
  Print (L"    major_version_no ..................................... 0x%04x\n", AtapiData->major_version_no);
  Print (L"    minor_version_no ..................................... 0x%04x\n", AtapiData->minor_version_no);
  Print (L"    cmd_set_support_82 ................................... 0x%04x\n", AtapiData->cmd_set_support_82);
  Print (L"    cmd_set_support_83 ................................... 0x%04x\n", AtapiData->cmd_set_support_83);
  Print (L"    cmd_feature_support .................................. 0x%04x\n", AtapiData->cmd_feature_support);
  Print (L"    cmd_feature_enable_85 ................................ 0x%04x\n", AtapiData->cmd_feature_enable_85);
  Print (L"    cmd_feature_enable_86 ................................ 0x%04x\n", AtapiData->cmd_feature_enable_86);
  Print (L"    cmd_feature_default .................................. 0x%04x\n", AtapiData->cmd_feature_default);
  Print (L"    ultra_dma_select ..................................... 0x%04x\n", AtapiData->ultra_dma_select);
  switch (AtapiData->ultra_dma_select & 0xFF00) {
  case 0x0100:
    Print (L"      UDMA mode 0\n");
    break;
  case 0x0200:
    Print (L"      UDMA mode 1\n");
    break;
  case 0x0400:
    Print (L"      UDMA mode 2\n");
    break;
  case 0x0800:
    Print (L"      UDMA mode 3\n");
    break;
  case 0x1000:
    Print (L"      UDMA mode 4\n");
    break;
  case 0x2000:
    Print (L"      UDMA mode 5\n");
    break;
  case 0x4000:
    Print (L"      UDMA mode 6\n");
    break;
  default:
    break;
  }
  Print (L"    time_required_for_sec_erase .......................... 0x%04x\n", AtapiData->time_required_for_sec_erase);
  Print (L"    time_required_for_enhanced_sec_erase ................. 0x%04x\n", AtapiData->time_required_for_enhanced_sec_erase);
  Print (L"    current_advanced_power_mgmt_value .................... 0x%04x\n", AtapiData->current_advanced_power_mgmt_value);
  Print (L"    master_pwd_revison_code .............................. 0x%04x\n", AtapiData->master_pwd_revison_code);
  Print (L"    hardware_reset_result ................................ 0x%04x\n", AtapiData->hardware_reset_result);
  Print (L"    current_auto_acoustic_mgmt_value ..................... 0x%04x\n", AtapiData->current_auto_acoustic_mgmt_value);
  Print (L"    max_user_lba_for_48bit_addr .......................... 0x%04x\n", AtapiData->max_user_lba_for_48bit_addr);
  Print (L"    removable_media_status_notification_support .......... 0x%04x\n", AtapiData->removable_media_status_notification_support);
  Print (L"    security_status ...................................... 0x%04x\n", AtapiData->security_status);
  Print (L"    cfa_power_mode ....................................... 0x%04x\n", AtapiData->cfa_power_mode);
  Print (L"    current_media_serial_no .............................. 0x%04x\n", AtapiData->current_media_serial_no);
  Print (L"    integrity_word ....................................... 0x%04x\n", AtapiData->integrity_word);

  return;
}

//
// functions to handle IDE devices
//

STATIC
EFI_STATUS
DiscoverIdeDevice (
  IN IDE_DEV *IdeDev
  )
{
  EFI_STATUS      Status;
  BOOLEAN         SataFlag;

  SataFlag = FALSE;
  //
  // This extra detection is for SATA disks
  //
  Status = CheckPowerMode (IdeDev);
  if (Status == EFI_SUCCESS) {
    SataFlag = TRUE;
  }
    
  //
  //If a channel has not been checked, check it now. Then set it to "checked" state
  //After this step, all devices in this channel have been checked.
  //
  Status = DetectIDEController(IdeDev);
 
  if((EFI_ERROR(Status)) && !SataFlag) {    
    return EFI_NOT_FOUND;
  }
  
  //
  // test if it is an ATA device 
  //
  Status = ATAIdentify (IdeDev);
  if (EFI_ERROR(Status)) {
    //
    // if not ATA device, test if it is an ATAPI device
    //
    Status = ATAPIIdentify (IdeDev);
    if (EFI_ERROR(Status)) {
      //
      // if not ATAPI device either, return error.
      //
      return EFI_NOT_FOUND;
    }
  }
  
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
DetectIDEController (
  IN  IDE_DEV  *IdeDev
  )
{
  EFI_STATUS    Status;  
  UINT8         ErrorReg;
  UINT8         StatusReg;
  UINT8         InitStatusReg;
  EFI_STATUS    DeviceStatus;

  //
  //  This function is called by DiscoverIdeDevice(). It is used for detect 
  //  whether the IDE device exists in the specified Channel as the specified 
  //  Device Number.
  //  There is two IDE channels: one is Primary Channel, the other is 
  //  Secondary Channel.(Channel is the logical name for the physical "Cable".) 
  //  Different channel has different register group.
  //  On each IDE channel, at most two IDE devices attach, 
  //  one is called Device 0 (Master device), the other is called Device 1 
  //  (Slave device). The devices on the same channel co-use the same register 
  //  group, so before sending out a command for a specified device via command 
  //  register, it is a must to select the current device to accept the command 
  //  by set the device number in the Head/Device Register.
  //
  
  //
  //Slave device has been detected with master device.
  //
  if ((IdeDev->Device) == 1) {
    if (mSlaveDeviceExist) {
      //
      //If master not exists but slave exists, slave have to wait a while 
      //
      if (!mMasterDeviceExist) {  
        //
        // if single slave can't be detected, add delay 4s here.
        //
      }
      
      return EFI_SUCCESS;
    } else {
      return EFI_NOT_FOUND;
    }
  }
      
  //
  // Select slave device
  //
  IDEWritePortB (
    IdeDev->PciIo,
    IdeDev->IoPort->Head, 
    (UINT8)((1 << 4) | 0xe0 )
    );
  gBS->Stall (100);

  //
  //Save the init slave status register
  //
  InitStatusReg = IDEReadPortB (IdeDev->PciIo,IdeDev->IoPort->Reg.Status);


  //
  //Select master back
  //
  IDEWritePortB (
    IdeDev->PciIo,
    IdeDev->IoPort->Head, 
    (UINT8)((0 << 4) | 0xe0 )
    );
  gBS->Stall (100);
  //
  //Send ATA Device Execut Diagnostic command. 
  //This command should work no matter DRDY is ready or not
  //
  IDEWritePortB (IdeDev->PciIo,IdeDev->IoPort->Reg.Command, 0x90);

  Status = WaitForBSYClear (IdeDev, 3500);

  ErrorReg = IDEReadPortB (IdeDev->PciIo,IdeDev->IoPort->Reg1.Error);


  //
  //Master Error register is 0x01. D0 passed, D1 passed or not present.
  //Master Error register is 0x81. D0 passed, D1 failed. Return.
  //Master Error register is other value. D0 failed, D1 passed or not present..
  //
  if (ErrorReg == 0x01) {
    mMasterDeviceExist = TRUE;
    DeviceStatus = EFI_SUCCESS;
  } else if (ErrorReg == 0x81) {
  
    mMasterDeviceExist = TRUE;
    DeviceStatus = EFI_SUCCESS;
    mSlaveDeviceExist = FALSE;
    
    return DeviceStatus;
  } else {
    mMasterDeviceExist = FALSE;
    DeviceStatus = EFI_NOT_FOUND;
  }
    
  //
  //Master Error register is not 0x81, Go on check Slave
  //

  //
  // select slave
  //
  IDEWritePortB (
    IdeDev->PciIo,
    IdeDev->IoPort->Head, 
    (UINT8)((1 << 4) | 0xe0 )
    );

  gBS->Stall (300);
  ErrorReg = IDEReadPortB (IdeDev->PciIo,IdeDev->IoPort->Reg1.Error);

  //
  //Slave Error register is not 0x01, D1 failed. Return.
  //
  if (ErrorReg != 0x01) {
    mSlaveDeviceExist = FALSE;
    return DeviceStatus;
  }

  StatusReg = IDEReadPortB (IdeDev->PciIo,IdeDev->IoPort->Reg.Status);

  //
  //Most ATAPI devices don't set DRDY bit, so test with a slow but accurate  
  //   "ATAPI TEST UNIT READY" command
  //
/*
  if (((StatusReg & DRDY) == 0) && ((InitStatusReg & DRDY) == 0)) {
    Status = AtapiTestUnitReady (IdeDev);

    //
    //Still fail, Slave doesn't exist.
    //
    if (EFI_ERROR (Status)) {
      mSlaveDeviceExist = FALSE;
      return DeviceStatus;
    }    
  }
*/
  //
  //Error reg is 0x01 and DRDY is ready, 
  //  or ATAPI test unit ready success, 
  //  or  init Slave status DRDY is ready
  //Slave exists.
  //
  mSlaveDeviceExist = TRUE;
 
  return DeviceStatus;
}

EFI_STATUS
CheckPowerMode (
  IDE_DEV    *IdeDev
  )
{
  UINT8       ErrorRegister;
  EFI_STATUS  Status;  

  //
  // Read SATA registers to detect SATA disks
  // 
  
  IDEWritePortB (
    IdeDev->PciIo,
    IdeDev->IoPort->Head, 
    (UINT8)((IdeDev->Device << 4) | 0xe0 )
    );

  //
  // Wait 31 seconds for BSY clear. BSY should be in clear state if there exists
  // a device (initial state). Normally, BSY is also in clear state if there is
  // no device
  //
  Status = WaitForBSYClear (IdeDev, 31000);
  if (EFI_ERROR(Status)) {
    return EFI_NOT_FOUND;
  } 

  //
  // select device, read error register 
  //
  IDEWritePortB (
    IdeDev->PciIo,
    IdeDev->IoPort->Head, 
    (UINT8)((IdeDev->Device << 4) | 0xe0 )
    );
  Status = DRDYReady (IdeDev,200);

  ErrorRegister = IDEReadPortB (IdeDev->PciIo,IdeDev->IoPort->Reg1.Error);
  if ((ErrorRegister == 0x01) || (ErrorRegister == 0x81)) {
    return EFI_SUCCESS;
  }  else {
    return EFI_NOT_FOUND;
  }
}

STATIC
EFI_STATUS                                                         
ATAIdentify (                                                  
  IN  IDE_DEV  *IdeDev
  )
{
  EFI_STATUS           Status;
  EFI_IDENTIFY_DATA    *AtaIdentifyPointer;
  EFI_ATA_SMART_DATA   *AtaSmartPointer;               
  UINT32               Capacity;
  UINT8                DeviceSelect;
      
  //
  //  AtaIdentifyPointer is used for accommodating returned IDENTIFY data of 
  //  the ATA Identify command
  //
  Status = gBS->AllocatePool(
                  EfiBootServicesData,
                  sizeof(EFI_IDENTIFY_DATA),
                  (VOID**)&AtaIdentifyPointer
                  );
  if (EFI_ERROR(Status)) {
    return Status;
  }
  ZeroMem(AtaIdentifyPointer,sizeof(EFI_IDENTIFY_DATA));
  
  //
  //  AtaSmartPointer is used for accommodating returned S.M.A.R.T. data of 
  //  ATA Identify command
  //
  Status = gBS->AllocatePool(
                  EfiBootServicesData,
                  sizeof(EFI_ATA_SMART_DATA),
                  (VOID**)&AtaSmartPointer
                  );
  if (EFI_ERROR(Status)) {
    gBS->FreePool (AtaIdentifyPointer);
    return Status;
  }
  ZeroMem(AtaSmartPointer,sizeof(EFI_ATA_SMART_DATA));

  //
  //  use ATA PIO Data In protocol to send ATA Identify command
  //  and receive data from device
  //
  DeviceSelect = 0;
  DeviceSelect = (UINT8)((IdeDev->Device) << 4);
  Status = AtaPioDataIn (
             IdeDev, 
             (VOID*)AtaIdentifyPointer, 
             sizeof(EFI_IDENTIFY_DATA),
             IDENTIFY_DRIVE_CMD, 
             DeviceSelect, 
             0,
             0,
             0,
             0,
             0
             );
  //
  // If ATA Identify command succeeds, then according to the received 
  // IDENTIFY data,
  // identify the device type ( ATA or not ).
  // If ATA device, fill the information in IdeDev.
  // If not ATA device, return IDE_DEVICE_ERROR
  //
  if (!EFI_ERROR(Status)) {
    
    IdeDev->pIdData = AtaIdentifyPointer;
   
    // bit 15 of pAtaIdentify->config is used to identify whether device is 
    // ATA device or ATAPI device.
    // if 0, means ATA device; if 1, means ATAPI device.
    //
    if ((AtaIdentifyPointer->AtaData.config & 0x8000) == 0x00) {
    
      //
      //Detect if support S.M.A.R.T. If yes, enable it as default
      //
      // 
      // AtaSMARTSupport(IdeDev);

      //
      //  use ATA PIO Data In protocol to send read SMART value command
      //  and receive data from device
      //
      Status = AtaPioDataIn (
                 IdeDev, 
                 (VOID*)AtaSmartPointer, 
                 sizeof(EFI_ATA_SMART_DATA),
                 ATA_SMART_CMD, 
                 DeviceSelect, 
                 ATA_SMART_CMD_READDATA,
                 1,
                 1,
                 0x4f,
                 0xc2
                 );
      if (!EFI_ERROR(Status)) {
        IdeDev->pSmartData = AtaSmartPointer;
      } else {
        Print (L"      Can't read S.M.A.R.T. \n");
        gBS->FreePool (AtaSmartPointer);
        IdeDev->pSmartData = NULL;
      }
      
      //
      // Check whether this device needs 48-bit addressing (ATAPI-6 ata device)
      //
      Status = AtaAtapi6Identify (IdeDev);
      if (!EFI_ERROR(Status)) {
        //
        // It's a disk with >120GB capacity, initialized in AtaAtapi6Identify()
        //
        return EFI_SUCCESS;
      }
      
      //
      // This is a hard disk <= 120GB capacity, treat it as normal hard disk
      //
      IdeDev->Type = IdeHardDisk;
  
      //
      // Calculate device capacity 
      //       
      Capacity = (AtaIdentifyPointer->AtaData.user_addressable_sectors_hi << 16) 
                  | AtaIdentifyPointer->AtaData.user_addressable_sectors_lo ;
      
      return EFI_SUCCESS;
    }
  }

  gBS->FreePool (AtaIdentifyPointer);
  gBS->FreePool (AtaSmartPointer);
  //
  // Make sure the pIdData will not be freed again. 
  //
  IdeDev->pIdData = NULL;
  IdeDev->pSmartData = NULL;
  
  return EFI_DEVICE_ERROR;
}   

STATIC
EFI_STATUS
AtaAtapi6Identify (
  IN  IDE_DEV  *IdeDev
  )
{
  UINT8              Index;
  EFI_LBA            TmpLba;            
  EFI_LBA            Capacity;
  EFI_IDENTIFY_DATA  *Atapi6IdentifyStruct;

  //
  // This function is called by ATAIdentify() to identity whether this disk
  // supports ATA/ATAPI6 48bit addressing, ie support >120G capacity
  //
  
  if (IdeDev->pIdData == NULL) {
    return EFI_UNSUPPORTED;
  }

  Atapi6IdentifyStruct = IdeDev->pIdData;

  if ((Atapi6IdentifyStruct->AtapiData.cmd_set_support_83 & bit(10)) == 0) {
    //
    // The device dosn't support 48 bit addressing
    //
    return EFI_UNSUPPORTED;
  }

  //
  // 48 bit address feature set is supported, get maximum capacity
  //
  Capacity = Atapi6IdentifyStruct->AtapiData.max_user_lba_for_48bit_addr[0];
  for (Index = 1; Index < 4; Index++) {
    //
    // Lower byte goes first: word[100] is the lowest word, word[103] is highest
    //
    TmpLba    = Atapi6IdentifyStruct->AtapiData.max_user_lba_for_48bit_addr[Index];
    Capacity |= LShiftU64 (TmpLba, 16 * Index);  
  }
  
  if (Capacity > MAX_28BIT_ADDRESSING_CAPACITY) {
    //
    // Capacity exceeds 120GB. 48-bit addressing is really needed
    //
    IdeDev->Type = Ide48bitAddressingHardDisk;

    return EFI_SUCCESS;
  }

  return EFI_UNSUPPORTED;
}

STATIC
EFI_STATUS
ATAPIIdentify (                                                    
  IN  IDE_DEV  *IdeDev
  )
{
  EFI_IDENTIFY_DATA     *AtapiIdentifyPointer;
  UINT8                 DeviceSelect;  
  EFI_STATUS            Status; 
 
  //
  // device select bit
  //
  DeviceSelect = 0;
  DeviceSelect = (UINT8)((IdeDev->Device) << 4);

  AtapiIdentifyPointer = AllocatePool (sizeof (EFI_IDENTIFY_DATA));
  if (AtapiIdentifyPointer == NULL) {    
    return EFI_OUT_OF_RESOURCES;
  }
  //
  // Send ATAPI Identify Command to get IDENTIFY data.
  //
  Status = AtaPioDataIn(
             IdeDev, 
             (VOID*)AtapiIdentifyPointer, 
             sizeof(EFI_IDENTIFY_DATA),
             ATAPI_IDENTIFY_DEVICE_CMD, 
             DeviceSelect, 
             0,
             0,
             0,
             0,
             0
             );
        
  if (EFI_ERROR(Status)) {    
    gBS->FreePool (AtapiIdentifyPointer);
    return EFI_DEVICE_ERROR;
  }
  
  IdeDev->pIdData = AtapiIdentifyPointer;
/*
  //
  // Send ATAPI Inquiry Packet Command to get INQUIRY data.
  //
  Status = AtapiInquiry(IdeDev);
  if (EFI_ERROR(Status)) {
    gBS->FreePool (IdeDev->pIdData);
    //
    // Make sure the pIdData will not be freed again. 
    //
    IdeDev->pIdData = NULL;
    return EFI_DEVICE_ERROR;
  }
   
  //
  // Get media removable info from INQUIRY data.
  //

  //
  // Identify device type via INQUIRY data.
  //
  switch (IdeDev->pInquiryData->peripheral_type & 0x1f) 
  {
  case 0x00: // Magnetic Disk
    IdeDev->Type = IdeMagnetic;          // device is LS120 or ZIP drive.
    break;
  
  case 0x05:  // CD-ROM
    IdeDev->Type = IdeCdRom; 
    break;
  
  case 0x01: // Tape
  case 0x04: // WORM
  case 0x07: // Optical
  default:
    IdeDev->Type = IdeUnknown;
    gBS->FreePool (IdeDev->pIdData);    
    gBS->FreePool (IdeDev->pInquiryData);
    //
    // Make sure the pIdData and pInquiryData will not be freed again. 
    //
    IdeDev->pIdData = NULL;
    IdeDev->pInquiryData = NULL;
    return EFI_DEVICE_ERROR;
  }
  */
  /* 
  IdeDev->SenseDataNumber = 6;    // original sense data numbers 
  IdeDev->SenseData = EfiLibAllocatePool (IdeDev->SenseDataNumber * sizeof(REQUEST_SENSE_DATA));
  if (IdeDev->SenseData == NULL) {
    gBS->FreePool (IdeDev->pIdData);
    gBS->FreePool (IdeDev->pInquiryData);
    //
    // Make sure the pIdData and pInquiryData will not be freed again. 
    //
    IdeDev->pIdData = NULL;
    IdeDev->pInquiryData = NULL;
    return EFI_OUT_OF_RESOURCES;
  }
  */  
  return EFI_SUCCESS;
}
/*
STATIC
VOID
AtaSMARTSupport (
  IN  IDE_DEV  *IdeDev
  )
{
  EFI_STATUS                          Status;
  BOOLEAN                             SMARTSupported;
  UINT8                               Device;
  EFI_IDENTIFY_DATA                   *TmpAtaIdentifyPointer;
  UINT8                               DeviceSelect;
  UINT8                               LBAMid;
  UINT8                               LBAHigh;

  //
  //Detect if the device supports S.M.A.R.T.
  //
  if ((IdeDev->pIdData->AtaData.command_set_supported_83 & 0xc000) != 0x4000) {
    //
    //Data in word 82 is not valid (bit15 shall be zero and bit14 shall be to one)
    //
    return;
  } else {
    if ((IdeDev->pIdData->AtaData.command_set_supported_82 & 0x0001) != 0x0001) {
      //
      //S.M.A.R.T is not supported by the device
      //
      SMARTSupported = FALSE;
    } else {
      SMARTSupported = TRUE;
    }
  }

  if (SMARTSupported) {
    Device = (UINT8)((IdeDev->Device << 4) | 0xe0);
    Status = AtaNonDataCommandIn (
               IdeDev, 
               ATA_SMART_CMD,
               Device, 
               ATA_SMART_ENABLE_OPERATION,
               0,
               0, 
               ATA_CONSTANT_4F,
               ATA_CONSTANT_C2
               );
    //
    //Detect if this feature is enabled
    //
    Status = gBS->AllocatePool(
                    EfiBootServicesData,
                    sizeof(EFI_IDENTIFY_DATA),
                    (VOID**)&TmpAtaIdentifyPointer
                    );
    if (EFI_ERROR(Status)) {
      return ;
    }

    EfiZeroMem(TmpAtaIdentifyPointer,sizeof(EFI_IDENTIFY_DATA));

    DeviceSelect = (UINT8)((IdeDev->Device) << 4);

    Status = AtaPioDataIn (
               IdeDev, 
               (VOID*)TmpAtaIdentifyPointer, 
               sizeof(EFI_IDENTIFY_DATA),
               IDENTIFY_DRIVE_CMD, 
               DeviceSelect, 
               0,
               0,
               0,
               0,
               0
               );

    if (EFI_ERROR(Status)) {
      gBS->FreePool (TmpAtaIdentifyPointer);
      return ;
    }

    //
    //Check if the feature is enabled
    //
    if ((TmpAtaIdentifyPointer->AtaData.command_set_feature_enb_85 & 0x0001) == 0x0001) {
      //
      //Read status data
      //
      AtaNonDataCommandIn (
        IdeDev, 
        ATA_SMART_CMD,
        Device, 
        ATA_SMART_RETURN_STATUS,
        0, 0, 
        ATA_CONSTANT_4F,
        ATA_CONSTANT_C2
        );
      LBAMid = IDEReadPortB (IdeDev->PciIo,IdeDev->IoPort->CylinderLsb); 
      LBAHigh = IDEReadPortB (IdeDev->PciIo,IdeDev->IoPort->CylinderMsb); 

      if ((LBAMid == 0x4f) && (LBAHigh == 0xc2)) {
        //
        //The threshold exceeded condition is not detected by the device
        //
      } else if ((LBAMid == 0xf4) && (LBAHigh == 0x2c)) {
        //
        //The threshold exceeded condition is  detected by the device
        //
      }

    }
    gBS->FreePool (TmpAtaIdentifyPointer);
  }
  
  return;
}
*/
//
// functions handle ata,atapi command
//
STATIC
EFI_STATUS
AtaPioDataIn (
  IN  IDE_DEV         *IdeDev, 
  IN  VOID            *Buffer,
  IN  UINT32          ByteCount,
  IN  UINT8           AtaCommand,
  IN  UINT8           Head,
  IN  UINT8           Feature,
  IN  UINT8           SectorCount,
  IN  UINT8           SectorNumber,
  IN  UINT8           CylinderLsb,
  IN  UINT8           CylinderMsb
  )
{
  UINTN       WordCount;
  UINTN       Increment;
  UINT16      *Buffer16;
  EFI_STATUS  Status;
  
  Status = WaitForBSYClear (IdeDev, ATATIMEOUT);
  if(EFI_ERROR(Status)){
    return EFI_DEVICE_ERROR;
  }

  //
  //  e0:1110,0000-- bit7 and bit5 are reserved bits.
  //           bit6 set means LBA mode
  //
  IDEWritePortB (
    IdeDev->PciIo,IdeDev->IoPort->Head, 
    (UINT8)((IdeDev->Device << 4) | 0xe0 | Head)
    );   

  //
  // All ATAPI device's ATA commands can be issued regardless of the 
  // state of the DRDY
  //
  if(IdeDev->Type == IdeHardDisk){
    Status = DRDYReady(IdeDev, ATATIMEOUT);
    if (EFI_ERROR(Status)) {
      return EFI_DEVICE_ERROR;
    }
  }
 
  //
  // set all the command parameters
  // Before write to all the following registers, BSY and DRQ must be 0.
  //
  Status = DRQClear2 (IdeDev, ATATIMEOUT);
  if(EFI_ERROR(Status)) {    
    return EFI_DEVICE_ERROR ;
  }
  /*
  if (AtaCommand == SET_FEATURES_CMD) {
    IDEWritePortB(IdeDev->PciIo,IdeDev->IoPort->Reg1.Feature, 0x03);
  }
  */
  IDEWritePortB (IdeDev->PciIo,IdeDev->IoPort->SectorCount, SectorCount); 
  IDEWritePortB (IdeDev->PciIo,IdeDev->IoPort->SectorNumber, SectorNumber);
  IDEWritePortB (IdeDev->PciIo,IdeDev->IoPort->CylinderLsb, CylinderLsb); 
  IDEWritePortB (IdeDev->PciIo,IdeDev->IoPort->CylinderMsb, CylinderMsb); 
  IDEWritePortB (IdeDev->PciIo,IdeDev->IoPort->Reg1.Feature, Feature);

  //
  // send command via Command Register
  //
  IDEWritePortB (IdeDev->PciIo,IdeDev->IoPort->Reg.Command, AtaCommand);
 
  Buffer16 = (UINT16 *)Buffer;
  
  //
  // According to PIO data in protocol, host can perform a series of reads to 
  // the data register after each time device set DRQ ready;
  // The data size of "a series of read" is command specific.
  // For most ATA command, data size received from device will not exceed 
  // 1 sector, hence the data size for "a series of read" can be the whole data
  // size of one command request.
  // For ATA command such as Read Sector command, the data size of one ATA 
  // command request is often larger than 1 sector, according to the 
  // Read Sector command, the data size of "a series of read" is exactly 1 
  // sector.
  // Here for simplification reason, we specify the data size for 
  // "a series of read" to 1 sector (256 words) if data size of one ATA command
  // request is larger than 256 words.
  // 
  Increment = 256;      // 256 words

  WordCount = 0;        // used to record bytes of currently transfered data

  while (WordCount < ByteCount / 2) {
    //
    // Poll DRQ bit set, data transfer can be performed only when DRQ is ready.
    //
    Status = DRQReady2(IdeDev, ATATIMEOUT);
    if (EFI_ERROR(Status)) {      
      return EFI_DEVICE_ERROR;
    } 
    
    Status = CheckErrorStatus (IdeDev);
    if(EFI_ERROR(Status)) {      
      return EFI_DEVICE_ERROR;
    }

    //
    // Get the byte count for one series of read
    //
    if ( (WordCount + Increment) > ByteCount/2) {
      Increment = ByteCount/2 - WordCount ;
    }

    IDEReadPortWMultiple (
      IdeDev->PciIo,
      IdeDev->IoPort->Data, 
      Increment, 
      Buffer16
      );

    WordCount += Increment;
    Buffer16 += Increment;
  }

  DRQClear (IdeDev, ATATIMEOUT);
  
  return CheckErrorStatus (IdeDev);
}

//
// ide port access functions
//
STATIC
UINT8
IDEReadPortB (
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT16                Port
  )
{
  UINT8         Data = 0;
  PciIo->Io.Read (PciIo,
              EfiPciIoWidthUint8,
              EFI_PCI_IO_PASS_THROUGH_BAR,
              (UINT64)Port,
              1,
              &Data
              );
  return Data;                 
}   

STATIC
VOID
IDEWritePortB (
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT16                Port, 
  IN  UINT8                 Data
  )
{
  PciIo->Io.Write (PciIo,
              EfiPciIoWidthUint8,
              EFI_PCI_IO_PASS_THROUGH_BAR,
              (UINT64)Port,
              1,
              &Data
              );  

}

STATIC
VOID
IDEReadPortWMultiple (
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT16                Port, 
  IN  UINTN                 Count,
  IN  VOID                  *Buffer
  )
{
  PciIo->Io.Read (PciIo,
              EfiPciIoWidthFifoUint16,
              EFI_PCI_IO_PASS_THROUGH_BAR,
              (UINT64)Port,
              Count,
              Buffer
              );
}

//
// functions handle with status register
//

STATIC
EFI_STATUS  
DRQClear (
  IN  IDE_DEV  *IdeDev,
  IN  UINTN    TimeoutInMilliSeconds
  )
{
  UINT32        Delay;
  UINT8         StatusRegister;
  UINT8         ErrorRegister;
  
  //
  // This function is used to poll for the DRQ bit clear in the Status 
  // Register. DRQ is cleared when the device is finished transferring data. 
  // So this function is called after data transfer is finished.
  //

  Delay = (UINT32)(((TimeoutInMilliSeconds * STALL_1_MILLI_SECOND) / 30) + 1);
  do {
    StatusRegister = IDEReadPortB(IdeDev->PciIo,IdeDev->IoPort->Reg.Status);
    //
    // wait for BSY == 0 and DRQ == 0
    //
    if ((StatusRegister & (DRQ | BSY)) == 0) {
      break;
    }
    
    if ((StatusRegister & (BSY | ERR)) == ERR) {
      ErrorRegister =  IDEReadPortB(IdeDev->PciIo,IdeDev->IoPort->Reg1.Error); 
      if ((ErrorRegister & ABRT_ERR) == ABRT_ERR) {
        return EFI_ABORTED;
      }
    }

    gBS->Stall(30);
    Delay --;
  } while (Delay);
  
  return (Delay == 0) ? EFI_TIMEOUT : EFI_SUCCESS;
}

STATIC
EFI_STATUS  
DRQClear2 (
  IN  IDE_DEV  *IdeDev,
  IN  UINTN    TimeoutInMilliSeconds
  )  
{
  UINT32      Delay;
  UINT8       AltRegister;
  UINT8       ErrorRegister;
  
  //
  // This function is used to poll for the DRQ bit clear in the Alternate 
  // Status Register. DRQ is cleared when the device is finished 
  // transferring data. So this function is called after data transfer
  // is finished.
  //

  Delay = (UINT32)(((TimeoutInMilliSeconds * STALL_1_MILLI_SECOND) / 30) + 1);
  do {
    AltRegister = IDEReadPortB (IdeDev->PciIo,IdeDev->IoPort->Alt.AltStatus);
    //
    //  wait for BSY == 0 and DRQ == 0
    //
    if ((AltRegister & (DRQ | BSY)) == 0) {
      break;
    }
    
    if ((AltRegister & (BSY | ERR)) == ERR) {
      ErrorRegister = IDEReadPortB(IdeDev->PciIo,IdeDev->IoPort->Reg1.Error); 
      if ((ErrorRegister & ABRT_ERR) == ABRT_ERR) {
        return EFI_ABORTED;
      }
    }

    gBS->Stall(30); 
    Delay --;
  } while (Delay);
  
  return (Delay == 0) ? EFI_TIMEOUT : EFI_SUCCESS;
}

STATIC
EFI_STATUS  
DRQReady (
  IN  IDE_DEV  *IdeDev,
  IN  UINTN    TimeoutInMilliSeconds
  )
{
  UINT32      Delay; 
  UINT8       StatusRegister;
  UINT8       ErrorRegister;

  //
  // This function is used to poll for the DRQ bit set in the 
  // Status Register.
  // DRQ is set when the device is ready to transfer data. So this function
  // is called after the command is sent to the device and before required 
  // data is transferred.
  // 

  Delay = (UINT32)(((TimeoutInMilliSeconds * STALL_1_MILLI_SECOND) / 30) + 1);
  do {
    //
    //  read Status Register will clear interrupt
    //
    StatusRegister = IDEReadPortB(IdeDev->PciIo,IdeDev->IoPort->Reg.Status);   
    
    //
    //  BSY==0,DRQ==1
    //
    if ((StatusRegister & (BSY | DRQ )) == DRQ) {
      break;
    }
    
    if ((StatusRegister & (BSY | ERR)) == ERR) {
      ErrorRegister =  IDEReadPortB(IdeDev->PciIo,IdeDev->IoPort->Reg1.Error); 
      if ((ErrorRegister & ABRT_ERR) == ABRT_ERR) {
        return EFI_ABORTED;
      }
    }
   
    gBS->Stall(30); 
    Delay --;
  } while (Delay);

  return (Delay == 0) ? EFI_TIMEOUT : EFI_SUCCESS;
}

STATIC
EFI_STATUS  
DRQReady2 (
  IN  IDE_DEV  *IdeDev,
  IN  UINTN    TimeoutInMilliSeconds
  )
{
  UINT32      Delay; 
  UINT8       AltRegister;
  UINT8       ErrorRegister;
  
  //
  // This function is used to poll for the DRQ bit set in the 
  // Alternate Status Register. DRQ is set when the device is ready to 
  // transfer data. So this function is called after the command 
  // is sent to the device and before required data is transferred.
  //  

  Delay = (UINT32)(((TimeoutInMilliSeconds * STALL_1_MILLI_SECOND) / 30) + 1);

  do {
    //
    //  Read Alternate Status Register will not clear interrupt status
    //
    AltRegister = IDEReadPortB(IdeDev->PciIo,IdeDev->IoPort->Alt.AltStatus); 
    //
    //BSY == 0 , DRQ == 1
    //
    if ((AltRegister & (BSY | DRQ )) == DRQ) {
      break;
    }    
    
    if ((AltRegister & (BSY | ERR)) == ERR) {
      ErrorRegister = IDEReadPortB(IdeDev->PciIo,IdeDev->IoPort->Reg1.Error); 
      if ((ErrorRegister & ABRT_ERR) == ABRT_ERR) {
        return EFI_ABORTED;
      }
    }

    gBS->Stall(30); 
    Delay --;
  } while (Delay);
  
  return (Delay == 0) ? EFI_TIMEOUT : EFI_SUCCESS;
}

STATIC
EFI_STATUS  
DRDYReady (
  IN  IDE_DEV  *IdeDev,
  IN  UINTN    DelayInMilliSeconds
  )
{
  UINT32        Delay; 
  UINT8         StatusRegister;
  UINT8       ErrorRegister;

  //
  // This function is used to poll for the DRDY bit set in the 
  // Status Register. DRDY bit is set when the device is ready 
  // to accept command. Most ATA commands must be sent after 
  // DRDY set except the ATAPI Packet Command.
  //
 
  Delay = (UINT32)(((DelayInMilliSeconds * STALL_1_MILLI_SECOND)  / 30) + 1);
  do {
    StatusRegister = IDEReadPortB(IdeDev->PciIo,IdeDev->IoPort->Reg.Status); 
    //
    //  BSY == 0 , DRDY == 1
    //
    if ((StatusRegister & (DRDY | BSY)) == DRDY) {
      break;
    }
    
    if ((StatusRegister & (BSY | ERR)) == ERR) {
      ErrorRegister =  IDEReadPortB(IdeDev->PciIo,IdeDev->IoPort->Reg1.Error); 
      if ((ErrorRegister & ABRT_ERR) == ABRT_ERR) {
        return EFI_ABORTED;
      }
    }

    gBS->Stall(30); 
    Delay --;
  } while (Delay);

  return (Delay == 0) ? EFI_TIMEOUT : EFI_SUCCESS;
}

STATIC
EFI_STATUS  
DRDYReady2 (
  IN  IDE_DEV  *IdeDev,
  IN  UINTN    DelayInMilliSeconds
  )
{
  UINT32        Delay; 
  UINT8         AltRegister;
  UINT8       ErrorRegister;

  // 
  // This function is used to poll for the DRDY bit set in the 
  // Alternate Status Register. DRDY bit is set when the device is ready 
  // to accept command. Most ATA commands must be sent after 
  // DRDY set except the ATAPI Packet Command.
  //  
  
  Delay = (UINT32)(((DelayInMilliSeconds * STALL_1_MILLI_SECOND)  / 30) + 1);
  do {
    AltRegister = IDEReadPortB(IdeDev->PciIo,IdeDev->IoPort->Alt.AltStatus); 
    //
    //  BSY == 0 , DRDY == 1
    //
    if ((AltRegister & (DRDY | BSY)) == DRDY){
      break;
    }
    
    if ((AltRegister & (BSY | ERR)) == ERR) {
      ErrorRegister = IDEReadPortB(IdeDev->PciIo,IdeDev->IoPort->Reg1.Error); 
      if ((ErrorRegister & ABRT_ERR) == ABRT_ERR) {
        return EFI_ABORTED;
      }
    }
    
    gBS->Stall(30); 
    Delay --;
  } while (Delay);
  
  return (Delay == 0) ? EFI_TIMEOUT : EFI_SUCCESS;
}

STATIC
EFI_STATUS  
WaitForBSYClear (
  IN  IDE_DEV         *IdeDev,
  IN  UINTN           TimeoutInMilliSeconds
  ) 
{
  UINT32        Delay; 
  UINT8         StatusRegister;
  
  //
  // This function is used to poll for the BSY bit clear in the 
  // Status Register. BSY is clear when the device is not busy.
  // Every command must be sent after device is not busy.
  // 
  
  Delay = (UINT32)(((TimeoutInMilliSeconds * STALL_1_MILLI_SECOND) / 30) +  1);
  do {
    StatusRegister = IDEReadPortB(IdeDev->PciIo,IdeDev->IoPort->Reg.Status); 
    if ((StatusRegister & BSY) == 0x00) {
      break;
    }    
    gBS->Stall(30);    
    Delay --;
  } while (Delay);

  return (Delay == 0) ? EFI_TIMEOUT : EFI_SUCCESS;
}

STATIC
EFI_STATUS  
WaitForBSYClear2 (
  IN  IDE_DEV  *IdeDev,
  IN  UINTN    TimeoutInMilliSeconds
  )
{
  UINT32        Delay; 
  UINT8         AltRegister;
  
  //
  // This function is used to poll for the BSY bit clear in the 
  // Alternate Status Register. BSY is clear when the device is not busy.
  // Every command must be sent after device is not busy.
  //  
  
  Delay = (UINT32)(((TimeoutInMilliSeconds * STALL_1_MILLI_SECOND) / 30) +  1);
  do {
    AltRegister = IDEReadPortB(IdeDev->PciIo,IdeDev->IoPort->Alt.AltStatus); 
    if ((AltRegister & BSY) == 0x00) {
      break;
    }

    gBS->Stall(30); 
    Delay --;
  } while (Delay);
  
  return (Delay == 0) ? EFI_TIMEOUT : EFI_SUCCESS;
}

STATIC
EFI_STATUS
CheckErrorStatus (
  IN  IDE_DEV  *IdeDev
  )
{
  UINT8   StatusRegister;

  //
  // This function is used to analyze the Status Register and print out 
  // some debug information and if there is ERR bit set in the Status
  // Register, the Error Register's value is also be parsed and print out.
  //
  StatusRegister = IDEReadPortB (IdeDev->PciIo,IdeDev->IoPort->Reg.Status); 

  if ((StatusRegister & (ERR | DWF | CORR)) == 0 ) {    
    return EFI_SUCCESS;  
  }
  
  return EFI_DEVICE_ERROR;  
}

EFI_STATUS
GetIdeRegistersBaseAddr (
  IN  EFI_PCI_IO_PROTOCOL         *PciIo,
  OUT IDE_REGISTERS_BASE_ADDR     *IdeRegsBaseAddr
  )
/*++

Routine Description:
  Get IDE IO port registers' base addresses by mode. In 'Compatibility' mode,
  use fixed addresses. In Native-PCI mode, get base addresses from BARs in
  the PCI IDE controller's Configuration Space.
  
  The steps to get IDE IO port registers' base addresses for each channel 
  as follows:

  1. Examine the Programming Interface byte of the Class Code fields in PCI IDE 
     controller's Configuration Space to determine the operating mode.
     
  2. a) In 'Compatibility' mode, use fixed addresses shown in the Table 1 below.
                 ___________________________________________
                |           | Command Block | Control Block |
                |  Channel  |   Registers   |   Registers   |
                |___________|_______________|_______________|
                |  Primary  |  1F0h - 1F7h  |  3F6h - 3F7h  |
                |___________|_______________|_______________|
                | Secondary |  170h - 177h  |  376h - 377h  |
                |___________|_______________|_______________|
        
                  Table 1. Compatibility resource mappings
        
     b) In Native-PCI mode, IDE registers are mapped into IO space using the BARs
        in IDE controller's PCI Configuration Space, shown in the Table 2 below.
               ___________________________________________________
              |           |   Command Block   |   Control Block   |
              |  Channel  |     Registers     |     Registers     |
              |___________|___________________|___________________|
              |  Primary  | BAR at offset 0x10| BAR at offset 0x14|
              |___________|___________________|___________________|
              | Secondary | BAR at offset 0x18| BAR at offset 0x1C|
              |___________|___________________|___________________|
      
                        Table 2. BARs for Register Mapping
        Note: Refer to Intel ICH4 datasheet, Control Block Offset: 03F4h for 
              primary, 0374h for secondary. So 2 bytes extra offset should be 
              added to the base addresses read from BARs.
  
  For more details, please refer to PCI IDE Controller Specification and Intel 
  ICH4 Datasheet.
  
Arguments:
  PciIo             - Pointer to the EFI_PCI_IO_PROTOCOL instance
  IdeRegsBaseAddr   - Pointer to IDE_REGISTERS_BASE_ADDR to 
                      receive IDE IO port registers' base addresses
                      
Returns:
    
--*/
{
  EFI_STATUS  Status;
  PCI_TYPE00  PciData;

  Status = PciIo->Pci.Read (
                        PciIo,
                        EfiPciIoWidthUint8,
                        0,
                        sizeof (PciData),
                        &PciData
                        );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  if ((PciData.Hdr.ClassCode[0] & IDE_PRIMARY_OPERATING_MODE) == 0) {
    IdeRegsBaseAddr[0].CommandBlockBaseAddr  = 0x1f0;
    IdeRegsBaseAddr[0].ControlBlockBaseAddr  = 0x3f6;
    IdeRegsBaseAddr[0].BusMasterBaseAddr     = 
    (UINT16)((PciData.Device.Bar[4] & 0x0000fff0));
  } else {
    //
    // The BARs should be of IO type
    //
    if ((PciData.Device.Bar[0] & 1) == 0 || 
        (PciData.Device.Bar[1] & 1) == 0) {
      return EFI_UNSUPPORTED;
    }

    IdeRegsBaseAddr[0].CommandBlockBaseAddr  =
    (UINT16) (PciData.Device.Bar[0] & 0x0000fff8);
    IdeRegsBaseAddr[0].ControlBlockBaseAddr  =
    (UINT16) ((PciData.Device.Bar[1] & 0x0000fffc) + 2);
    IdeRegsBaseAddr[0].BusMasterBaseAddr     =
    (UINT16) ((PciData.Device.Bar[4] & 0x0000fff0));
  }

  if ((PciData.Hdr.ClassCode[0] & IDE_SECONDARY_OPERATING_MODE) == 0) {
    IdeRegsBaseAddr[1].CommandBlockBaseAddr  = 0x170;
    IdeRegsBaseAddr[1].ControlBlockBaseAddr  = 0x376;
    IdeRegsBaseAddr[1].BusMasterBaseAddr     =
    (UINT16) ((PciData.Device.Bar[4] & 0x0000fff0));
  } else {
    //
    // The BARs should be of IO type
    //
    if ((PciData.Device.Bar[2] & 1) == 0 ||
        (PciData.Device.Bar[3] & 1) == 0) {
      return EFI_UNSUPPORTED;
    }

    IdeRegsBaseAddr[1].CommandBlockBaseAddr  =
    (UINT16) (PciData.Device.Bar[2] & 0x0000fff8);
    IdeRegsBaseAddr[1].ControlBlockBaseAddr  =
    (UINT16) ((PciData.Device.Bar[3] & 0x0000fffc) + 2);
    IdeRegsBaseAddr[1].BusMasterBaseAddr     =
    (UINT16) ((PciData.Device.Bar[4] & 0x0000fff0));
  }

  return EFI_SUCCESS;
}

STATIC
UINT8
IsBigEndian (
  VOID
  )
{
  UINT16 Value;
  UINT8  *Temp;
  
  Value = 0x0100;
  Temp = (UINT8*)&Value;

  return (*Temp);
}

STATIC
VOID
SwapSmartData (
  IN EFI_ATA_SMART_DATA *SmartData
  )
{
  UINTN                    Index;
  EFI_ATA_VENDOR_ATTRIBUTE *Temp;

  if(IsBigEndian()) {
    SwapBytes ((UINT8*)&(SmartData->Revsion_number));
    SwapBytes ((UINT8*)&(SmartData->Smart_Capability));
    SwapBytes ((UINT8*)&(SmartData->Total_Time_To_Complete_Off_Line));
    for (Index = 0; Index < NUMBER_ATA_SMART_ATTRIBUTES; Index++) {
      Temp = SmartData->Vendor_Attributes + Index;
      SwapBytes ((UINT8*)&(Temp->Flags));
    }
  }
  return;
}

STATIC 
VOID
SwapBytes (
  IN UINT8  *Swap
  )
{
  UINT8 Temp;
  
  Temp = *Swap;
  *Swap = *(Swap + 1);
  *(Swap + 1) = Temp;
  
  return;
}

STATIC
UINT16
AtaReturnAttributeRawValue (
  IN UINT8              Id,
  IN EFI_ATA_SMART_DATA *SmartData
  )
{
  UINTN                    Index;
  UINT16                   RawValue;
  UINT8                    Temp1;
  UINT8                    Temp2;
  EFI_ATA_VENDOR_ATTRIBUTE *Attri;

  if ((Id == 0) || (SmartData == NULL)) {
    return 0;
  }
  for (Index = 0; Index < NUMBER_ATA_SMART_ATTRIBUTES; Index++) {
    Attri = SmartData->Vendor_Attributes + Index;
    if (Attri->Id == Id) {    
      Temp1 = Attri->Raw[0];
      Temp2 = Attri->Raw[1];
      RawValue = (UINT16)(Temp1 + Temp2 * 256);
      return RawValue;
    }
  }
  return 0;
}

VOID
AtaInfo (
  VOID
  )
{
  EFI_STATUS                         Status;
  UINTN                              Index;
  UINTN                              NoHandles;
  EFI_HANDLE                         *HandleBuffer;
  EFI_DEVICE_PATH_PROTOCOL           *DevicePath;
  CHAR16                             *DevicePathStr;
  EFI_PCI_IO_PROTOCOL                *PciIo;
  EFI_IDENTIFY_DATA                  *IdentifyData; 
  EFI_ATA_SMART_DATA                 *SmartData; 
  UINT8                              Channel;
  UINT8                              Device;
  UINT8                              MaxChannel;
  UINT8                              MaxDevice;
  IDE_DEV                            IdeDev;
  UINT16                             CommandBlockBaseAddr;
  UINT16                             ControlBlockBaseAddr;
  IDE_BASE_REGISTERS                 IdeIoPortRegister;
  IDE_REGISTERS_BASE_ADDR            IdeRegsBaseAddr[2];
  PCI_TYPE00                         PciData;

  Status = gBS->LocateHandleBuffer (
                 ByProtocol,
                 &gEfiPciIoProtocolGuid,
                 NULL,
                 &NoHandles,
                 &HandleBuffer
                 );
  if (EFI_ERROR (Status) || (NoHandles == 0)) {
    Print (L"PciIo not found\n");
    return ;
  }

  for (Index = 0; Index < NoHandles; Index++) {
    PciIo = NULL;
    Status = gBS->HandleProtocol (
                   HandleBuffer[Index],
                   &gEfiPciIoProtocolGuid,
                   (VOID **)&PciIo
                   );
    if (EFI_ERROR (Status)) {
      Print (L"Get PciIo error - %r\n", Status);
      continue;
    }

    Status = PciIo->Pci.Read (
                          PciIo,
                          EfiPciIoWidthUint8,
                          0,
                          sizeof (PciData),
                          &PciData
                          );
    if (EFI_ERROR (Status)) {
      continue;
    }
    if (PciData.Hdr.ClassCode[2] != PCI_CLASS_MASS_STORAGE) {
      continue;
    }
    if (PciData.Hdr.ClassCode[1] != PCI_SUB_CLASS_IDE) {
      if (PciData.Hdr.ClassCode[1] == PCI_SUB_CLASS_RAID) {
        Print (L"Detect RAID Controller!\n");
      } else if (PciData.Hdr.ClassCode[1] == PCI_SUB_CLASS_SATADPA) {
        Print (L"Detect SATADPA Controller!\n");
      } else {
        Print (L"Detect Other IDE Controller!\n");
      }
      continue;
    }

    MaxChannel = 2;
    MaxDevice = 2;

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

    //
    // Check BusMaster Enabled?
    //
    if ((PciData.Hdr.Command & 0x4) != 0x4) {
      Print (L"  IDE Controller disabled!\n");
      continue;
    }

    //
    // send ATA command
    //
    for (Channel = 0; Channel < MaxChannel; Channel++) {
      Status = GetIdeRegistersBaseAddr (PciIo, IdeRegsBaseAddr);
      if (EFI_ERROR (Status)) {
        continue;
      }
      CommandBlockBaseAddr = IdeRegsBaseAddr[Channel].CommandBlockBaseAddr;
      ControlBlockBaseAddr = IdeRegsBaseAddr[Channel].ControlBlockBaseAddr;

      for (Device = 0; Device < MaxDevice; Device++) {
       
        SetMem (&IdeDev, sizeof (IDE_DEV),0);
  
        IdeDev.Channel = Channel;
        IdeDev.Device  = Device;
        IdeDev.PciIo   = PciIo;

        IdeDev.IoPort  = &IdeIoPortRegister;
 
        IdeIoPortRegister.Data                = CommandBlockBaseAddr;
        (*(UINT16 *) &IdeIoPortRegister.Reg1) = (UINT16) (CommandBlockBaseAddr + 0x01);
        IdeIoPortRegister.SectorCount         = (UINT16) (CommandBlockBaseAddr + 0x02);
        IdeIoPortRegister.SectorNumber        = (UINT16) (CommandBlockBaseAddr + 0x03);
        IdeIoPortRegister.CylinderLsb         = (UINT16) (CommandBlockBaseAddr + 0x04);
        IdeIoPortRegister.CylinderMsb         = (UINT16) (CommandBlockBaseAddr + 0x05);
        IdeIoPortRegister.Head                = (UINT16) (CommandBlockBaseAddr + 0x06);
        (*(UINT16 *) &IdeIoPortRegister.Reg)  = (UINT16) (CommandBlockBaseAddr + 0x07);
        (*(UINT16 *) &IdeIoPortRegister.Alt)  = ControlBlockBaseAddr;
        IdeIoPortRegister.DriveAddress        = (UINT16) (ControlBlockBaseAddr + 0x01);
        IdeIoPortRegister.MasterSlave         = (UINT16) ((Device == 0) ? 1 : 0);

        Status = DiscoverIdeDevice (&IdeDev);
        if (!EFI_ERROR(Status)) {
          IdentifyData = IdeDev.pIdData;
          SmartData = IdeDev.pSmartData;
          Print (L"  Channel - %d, Device - %d\n", Channel, Device);
          if ((IdeDev.Type == IdeHardDisk) || (IdeDev.Type == Ide48bitAddressingHardDisk)) {
            DumpAtaIdentifyData ((EFI_ATA_IDENTIFY_DATA *)IdentifyData);
            if (SmartData != NULL) {
              DumpAtaSmartData ((EFI_ATA_SMART_DATA *)SmartData);
            }
          } else {
            DumpAtapiIdentifyData ((EFI_ATAPI_IDENTIFY_DATA *)IdentifyData);
          }

          FreePool (IdeDev.pIdData);
        } else {
          Print (L"  Channel - %d, Device - %d : No device!\n", Channel, Device);
        }
      }
    }
  }
  
  return ;
}

EFI_STATUS
EFIAPI
AtaInfoEntryPoint (
  IN EFI_HANDLE                   ImageHandle,
  IN EFI_SYSTEM_TABLE             *SystemTable
  )
{
  AtaInfo ();
   
  return EFI_SUCCESS;
}
