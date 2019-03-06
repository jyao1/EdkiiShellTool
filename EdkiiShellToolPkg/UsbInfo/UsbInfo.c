/** @file

Copyright (c) 2017, Intel Corporation. All rights reserved.<BR>
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
#include <Library/DxeServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/DevicePathLib.h>
#include <Protocol/PciIo.h>
#include <Protocol/Usb2HostController.h>
#include <Protocol/UsbIo.h>

//
// UsbHc info
//
#define PCI_CLASSC_BASE_CLASS_SERIAL        0x0c
#define PCI_CLASSC_SUBCLASS_SERIAL_USB      0x03
#define PCI_CLASSC_PI_UHCI                  0x00
#define PCI_CLASSC_PI_OHCI                  0x10
#define PCI_CLASSC_PI_EHCI                  0x20
#define PCI_CLASSC_PI_XHCI                  0x30
#define PCI_CLASSC_PI_NO_PI                 0x80
#define PCI_CLASSC_PI_USB_DEVICE            0xFE

#define PCI_CLASSC                          0x09

#pragma pack(1)

typedef struct
{
  UINT8    PI;
  UINT8    SubClassCode;
  UINT8    BaseCode;
} USB_CLASSC;

#pragma pack()

//
// UsbIo info
//
#define CLASS_HID                     3
#define SUBCLASS_BOOT_INTERFACE       1
#define PROTOCOL_KEYBOARD             1
#define PROTOCOL_MOUSE                2

#define CLASS_MASS_STORAGE            8
#define SUBCLASS_RBC                  1
#define SUBCLASS_SFF8020_MMC2         2
#define SUBCLASS_QIC157               3
#define SUBCLASS_UFI                  4
#define SUBCLASS_SFF8070              5
#define SUBCLASS_SCSI                 6
#define PROTOCOL_CBI0                 0
#define PROTOCOL_CBI1                 1
#define PROTOCOL_BOT                  0x50

#define CLASS_HUB                     9 
#define SUBCLASS_HUB                  0
#define PROTOCOL_HUB                  0

VOID
UsbHcGetDescriptors (
  IN EFI_USB2_HC_PROTOCOL            *UsbHc,
  IN EFI_PCI_IO_PROTOCOL             *PciIo
  )
{
  EFI_STATUS                    Status;
  USB_CLASSC                    UsbClassCReg;
  
  Status = PciIo->Pci.Read (
                        PciIo,
                        EfiPciIoWidthUint8,
                        PCI_CLASSC,
                        sizeof(USB_CLASSC) / sizeof(UINT8),
                        &UsbClassCReg
                        );
  if (EFI_ERROR(Status)) {
    Print (L"  PCI read error - %r\n", Status);
    return ;
  }
  
  if ((UsbClassCReg.BaseCode == PCI_CLASSC_BASE_CLASS_SERIAL) 
      && (UsbClassCReg.SubClassCode == PCI_CLASSC_SUBCLASS_SERIAL_USB)) {
    switch (UsbClassCReg.PI) {
    case PCI_CLASSC_PI_UHCI :
      Print (L"  Universal Host Controller Interface\n");
      break;
    case PCI_CLASSC_PI_OHCI :
      Print (L"  Open Host Controller Interface\n");
      break;
    case PCI_CLASSC_PI_EHCI :
      Print (L"  Enhanced Host Controller Interface\n");
      break;
    case PCI_CLASSC_PI_XHCI :
      Print (L"  Extensible Host Controller Interface\n");
      break;
    case PCI_CLASSC_PI_NO_PI :
      Print (L"  USB with no specific PI\n");
      break;
    case PCI_CLASSC_PI_USB_DEVICE :
      Print (L"  USB Device\n");
      break;
    default:
      break;
    }
  }
}

VOID
PrintDeviceDesc (
  IN EFI_USB_DEVICE_DESCRIPTOR       DeviceDesc
  )
{
  Print (L"  Device Descriptor:\n");                        
  Print (L"    Length ............................................... 0x%02x\n", DeviceDesc.Length);
  Print (L"    DescriptorType ....................................... 0x%02x\n", DeviceDesc.DescriptorType);
  Print (L"    BcdUSB ............................................... 0x%04x\n", DeviceDesc.BcdUSB);  
  Print (L"    DeviceClass .......................................... 0x%02x\n", DeviceDesc.DeviceClass);
  Print (L"    DeviceSubClass ....................................... 0x%02x\n", DeviceDesc.DeviceSubClass);
  Print (L"    DeviceProtocol ....................................... 0x%02x\n", DeviceDesc.DeviceProtocol);
  Print (L"    MaxPacketSize0 ....................................... 0x%02x\n", DeviceDesc.MaxPacketSize0);
  Print (L"    IdVendor ............................................. 0x%04x\n", DeviceDesc.IdVendor);
  Print (L"    IdProduct ............................................ 0x%04x\n", DeviceDesc.IdProduct);
  Print (L"    BcdDevice ............................................ 0x%04x\n", DeviceDesc.BcdDevice);
  Print (L"    StrManufacturer ...................................... 0x%02x\n", DeviceDesc.StrManufacturer);
  Print (L"    StrProduct ........................................... 0x%02x\n", DeviceDesc.StrProduct);
  Print (L"    StrSerialNumber ...................................... 0x%02x\n", DeviceDesc.StrSerialNumber);
  Print (L"    NumConfigurations .................................... 0x%02x\n", DeviceDesc.NumConfigurations);
}

VOID
PrintConfigDesc (
  IN EFI_USB_CONFIG_DESCRIPTOR       ConfigDesc
  )
{
  Print (L"  Config Descriptor:\n");                        
  Print (L"    Length ............................................... 0x%02x\n", ConfigDesc.Length);
  Print (L"    DescriptorType ....................................... 0x%02x\n", ConfigDesc.DescriptorType);
  Print (L"    TotalLength .......................................... 0x%04x\n", ConfigDesc.TotalLength);
  Print (L"    NumInterfaces ........................................ 0x%02x\n", ConfigDesc.NumInterfaces);
  Print (L"    ConfigurationValue ................................... 0x%02x\n", ConfigDesc.ConfigurationValue);
  Print (L"    Configuration ........................................ 0x%02x\n", ConfigDesc.Configuration);
  Print (L"    Attributes ........................................... 0x%02x\n", ConfigDesc.Attributes);
  Print (L"    MaxPower ............................................. 0x%02x\n", ConfigDesc.MaxPower);
}

VOID
PrintInterfaceDesc (
  IN EFI_USB_INTERFACE_DESCRIPTOR       InterfaceDesc
  )
{
  Print (L"  Interface Descriptor:\n");                        
  Print (L"    Length ............................................... 0x%02x\n", InterfaceDesc.Length);
  Print (L"    DescriptorType ....................................... 0x%02x\n", InterfaceDesc.DescriptorType);
  Print (L"    InterfaceNumber ...................................... 0x%02x\n", InterfaceDesc.InterfaceNumber);
  Print (L"    AlternateSetting ..................................... 0x%02x\n", InterfaceDesc.AlternateSetting);
  Print (L"    NumEndpoints ......................................... 0x%02x\n", InterfaceDesc.NumEndpoints);
  Print (L"    InterfaceClass ....................................... 0x%02x\n", InterfaceDesc.InterfaceClass);
  switch (InterfaceDesc.InterfaceClass) {
  case CLASS_HID :
    Print (L"      CLASS HID\n");
    break;
  case CLASS_MASS_STORAGE :
    Print (L"      CLASS MASS STORAGE\n");
    break;
  case CLASS_HUB :
    Print (L"      CLASS HUB\n");
    break;
  default:
    break;
  }
  Print (L"    InterfaceSubClass .................................... 0x%02x\n", InterfaceDesc.InterfaceSubClass);
  switch (InterfaceDesc.InterfaceClass) {
  case CLASS_HID :
    switch (InterfaceDesc.InterfaceSubClass) {
    case SUBCLASS_BOOT_INTERFACE :
      Print (L"      SUBCLASS BOOT INTERFASE\n");
      break;
    default:
      break;
    }
    break;
  case CLASS_MASS_STORAGE :
    switch (InterfaceDesc.InterfaceSubClass) {
    case SUBCLASS_RBC :
      Print (L"      SUBCLASS RBC\n");
      break;
    case SUBCLASS_SFF8020_MMC2 :
      Print (L"      SUBCLASS SFF8020_MMC2\n");
      break;
    case SUBCLASS_QIC157 :
      Print (L"      SUBCLASS QIC157\n");
      break;
    case SUBCLASS_UFI :
      Print (L"      SUBCLASS UFI\n");
      break;
    case SUBCLASS_SFF8070 :
      Print (L"      SUBCLASS SFF8070\n");
      break;
    case SUBCLASS_SCSI :
      Print (L"      SUBCLASS SCSI\n");
      break;
    default:
      break;
    }
    break;
  case CLASS_HUB :
    switch (InterfaceDesc.InterfaceSubClass) {
    case SUBCLASS_HUB :
      Print (L"      SUBCLASS HUB\n");
      break;
    default:
      break;
    }
    break;
  default:
    break;
  }
  Print (L"    InterfaceProtocol .................................... 0x%02x\n", InterfaceDesc.InterfaceProtocol);
  switch (InterfaceDesc.InterfaceClass) {
  case CLASS_HID :
    switch (InterfaceDesc.InterfaceSubClass) {
    case SUBCLASS_BOOT_INTERFACE :
      switch (InterfaceDesc.InterfaceProtocol) {
      case PROTOCOL_KEYBOARD :
        Print (L"      PROTOCOL KEYBOARD\n");
        break;
      case PROTOCOL_MOUSE :
        Print (L"      PROTOCOL MOUSE\n");
        break;
      default:
        break;
      }
      break;
    default:
      break;
    }
    break;
  case CLASS_MASS_STORAGE :
    switch (InterfaceDesc.InterfaceSubClass) {
    case SUBCLASS_RBC :
    case SUBCLASS_SFF8020_MMC2 :
    case SUBCLASS_QIC157 :
    case SUBCLASS_UFI :
    case SUBCLASS_SFF8070 :
    case SUBCLASS_SCSI :
    default:
      switch (InterfaceDesc.InterfaceProtocol) {
      case PROTOCOL_CBI0 :
        Print (L"      PROTOCOL CBI0\n");
        break;
      case PROTOCOL_CBI1 :
        Print (L"      PROTOCOL CBI1\n");
        break;
      case PROTOCOL_BOT :
        Print (L"      PROTOCOL BOT\n");
        break;
      default:
        break;
      }
      break;
    }
    break;
  case CLASS_HUB :
    switch (InterfaceDesc.InterfaceSubClass) {
    case SUBCLASS_HUB :
      switch (InterfaceDesc.InterfaceProtocol) {
      case PROTOCOL_HUB :
        Print (L"      PROTOCOL HUB\n");
        break;
      default:
	break;
      }
      break;
    default:
      break;
    }
    break;
  default:
    break;
  };
  Print (L"    Interface ............................................ 0x%02x\n", InterfaceDesc.Interface);
}

VOID
PrintEndpointDesc (
  IN UINT8                             Index,
  IN EFI_USB_ENDPOINT_DESCRIPTOR       EndpointDesc
  )
{
  Print (L"  Endpoint#%d Descriptor:\n", Index);                        
  Print (L"    Length ............................................... 0x%02x\n", EndpointDesc.Length);
  Print (L"    DescriptorType ....................................... 0x%02x\n", EndpointDesc.DescriptorType);
  Print (L"    EndpointAddress ...................................... 0x%02x\n", EndpointDesc.EndpointAddress);
  Print (L"    Attributes ........................................... 0x%02x\n", EndpointDesc.Attributes);
  Print (L"    MaxPacketSize ........................................ 0x%04x\n", EndpointDesc.MaxPacketSize);
  Print (L"    Interval ............................................. 0x%02x\n", EndpointDesc.Interval);
}

VOID
UsbIoGetDescriptors (
  IN EFI_USB_IO_PROTOCOL             *UsbIo
  )
{
  EFI_STATUS                    Status;
  UINT8                         Index;
  EFI_USB_DEVICE_DESCRIPTOR     DeviceDesc;
  EFI_USB_CONFIG_DESCRIPTOR     ConfigDesc;
  EFI_USB_INTERFACE_DESCRIPTOR  InterfaceDesc;
  EFI_USB_ENDPOINT_DESCRIPTOR   EndpointDesc;
    
  //
  // Get Device Descriptor
  //
  Status = UsbIo->UsbGetDeviceDescriptor(UsbIo, &DeviceDesc);
  if (EFI_ERROR(Status)) {
    Print (L"  Get DeviceDescriptor Error.\n");
  } else {
    PrintDeviceDesc (DeviceDesc);
  }
    
  //
  // Get Config Descriptor
  //
  Status = UsbIo->UsbGetConfigDescriptor(UsbIo, &ConfigDesc);
  if (EFI_ERROR(Status)) {
    Print (L"  Get ConfigDescriptor Error.\n");
  } else {
    PrintConfigDesc (ConfigDesc);
  }
    
  //
  // Get Interface Descriptor
  //
  Status = UsbIo->UsbGetInterfaceDescriptor(UsbIo, &InterfaceDesc);
  if (EFI_ERROR(Status)) {
    Print (L"  Get InterfaceDescriptor Error.\n");
  } else {
    PrintInterfaceDesc (InterfaceDesc);
  }
    
  //
  // Get Endpoint Descriptor
  //
  for (Index = 0; Index < InterfaceDesc.NumEndpoints; Index++) {
    Status = UsbIo->UsbGetEndpointDescriptor (UsbIo, Index, &EndpointDesc);
    if (EFI_ERROR(Status)) {
      Print (L"  Get Endpoint#%d Descriptor Error.\n", Index);
    } else {
      PrintEndpointDesc (Index, EndpointDesc);
    }  
  }
}

VOID
UsbHcInfo (
  VOID
  )
{
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  CHAR16                    *DevicePathStr;
  UINTN                     Index;
  EFI_USB2_HC_PROTOCOL      *UsbHc;
  EFI_PCI_IO_PROTOCOL       *PciIo;
  UINTN                     NoHandles;
  EFI_HANDLE                *HandleBuffer;

  //
  // Get the UsbHc
  //
  Status = gBS->LocateHandleBuffer (
                 ByProtocol, 
                 &gEfiUsb2HcProtocolGuid,
                 NULL,
                 &NoHandles,
                 &HandleBuffer
                 );
  if (EFI_ERROR(Status)) {
    Print (L"LocateHandle UsbHc Failed - %r\n", Status);
    NoHandles = 0;
  }
            
  for (Index = 0; Index < NoHandles; Index++) {
    //
    // Get the UsbHc Interface
    //
    Status = gBS->HandleProtocol (
                   HandleBuffer[Index],
                   &gEfiUsb2HcProtocolGuid,
                   (VOID **)&UsbHc
                   );
    if (EFI_ERROR(Status)) {
      Print (L"HandleProtocol UsbHc Failed - %r\n", Status);
      continue;
    }
    
    //
    // Get the PciIo Interface
    //
    Status = gBS->HandleProtocol (
                   HandleBuffer[Index],
                   &gEfiPciIoProtocolGuid,
                   (VOID **)&PciIo
                   );
    if (EFI_ERROR(Status)) {
      Print (L"HandleProtocol PciIo Failed - %r\n", Status);
      continue;
    }
    
    //
    // Get DevicePath
    //
    Status = gBS->HandleProtocol (
                   HandleBuffer[Index],
                   &gEfiDevicePathProtocolGuid,
                   (VOID **)&DevicePath
                   );
    if (!EFI_ERROR(Status)) {
      DevicePathStr = ConvertDevicePathToText(DevicePath, TRUE, TRUE);
      if (DevicePathStr != NULL) {
        Print (L"UsbHc Device : %s\n", DevicePathStr);
        FreePool (DevicePathStr);
      }
    }
    
    //
    // Get the Descriptors
    //
    UsbHcGetDescriptors (UsbHc, PciIo);
  }

  Print (L"\n");
}

VOID
UsbIoInfo (
  VOID
  )
{
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  CHAR16                    *DevicePathStr;
  UINTN                     Index;
  EFI_USB_IO_PROTOCOL       *UsbIo;
  UINTN                     NoHandles;
  EFI_HANDLE                *HandleBuffer;

  //
  // Get the UsbIo
  //
  Status = gBS->LocateHandleBuffer (
                 ByProtocol, 
                 &gEfiUsbIoProtocolGuid,
                 NULL,
                 &NoHandles,
                 &HandleBuffer
                 );
  if (EFI_ERROR(Status)) {
    Print (L"LocateHandle UsbIo Failed - %r\n", Status);
    NoHandles = 0;
  }

  for (Index = 0; Index < NoHandles; Index++) {
    //
    // Get the UsbIo Interface
    //
    Status = gBS->HandleProtocol (
                   HandleBuffer[Index],
                   &gEfiUsbIoProtocolGuid,
                   (VOID **)&UsbIo
                   );
    if (EFI_ERROR(Status)) {
      Print (L"HandleProtocol UsbIo Failed - %r\n", Status);
      continue;
    }
    
    //
    // Get DevicePath
    //
    Status = gBS->HandleProtocol (
                   HandleBuffer[Index],
                   &gEfiDevicePathProtocolGuid,
                   (VOID **)&DevicePath
                   );
    if (!EFI_ERROR(Status)) {
      DevicePathStr = ConvertDevicePathToText(DevicePath, TRUE, TRUE);
      if (DevicePathStr != NULL) {
        Print (L"UsbIo Device : %s\n", DevicePathStr);
        FreePool (DevicePathStr);
      }
    }
    
    //
    // Get the Descriptors
    //
    UsbIoGetDescriptors (UsbIo);
  }
    
  Print (L"\n");
}

EFI_STATUS
EFIAPI
UsbInfoEntryPoint (
  IN EFI_HANDLE                   ImageHandle,
  IN EFI_SYSTEM_TABLE             *SystemTable
  )
{
  UsbHcInfo ();
  UsbIoInfo ();
  
  return EFI_SUCCESS;
}
