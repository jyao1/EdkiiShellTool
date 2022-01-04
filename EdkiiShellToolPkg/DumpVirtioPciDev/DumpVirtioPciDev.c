## @file
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

#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Protocol/PciRootBridgeIo.h>
#include <Protocol/PciIo.h>
#include <Protocol/Shell.h>

#include <IndustryStandard/Pci.h>

UINTN gPCIIO_Count = 0;
EFI_PCI_IO_PROTOCOL  *gPCIIOArray[256];

typedef struct
{
  UINT32 DeviceFeatures;
  UINT32 GuestFeatures;
  UINT32 QueueAddress;
  UINT16 QueueSize;
  UINT16 QueueSelect;
  UINT16 QueueNotify;
  UINT8  DeviceStatus;
  UINT8  ISRStatus;
} VIRTIOHDR;

typedef struct {
  UINT64 Addr;
  UINT32 Len;
  UINT16 Flags;
  UINT16 Next;
} VRING_DESC;

VOID
EFIAPI
PrintByByte(UINT8* Content, UINT32 Len) {
  UINT32 i;
  for (i = 0; i < Len; i++) {
    if (i % 16 == 0) Print (L"\n");
    Print (L"%02x ", Content[i]);
  }
  Print (L"\n");
}


EFI_STATUS 
LocatePCIIO (VOID)
{
  EFI_STATUS    Status;
  EFI_HANDLE    *PciHandleBuffer = NULL;
  UINTN         HandleIndex = 0;
  UINTN         HandleCount = 0;

  Status = gBS->LocateHandleBuffer (
    ByProtocol,
    &gEfiPciIoProtocolGuid,
    NULL,
    &HandleCount,
    &PciHandleBuffer
  );

  if (EFI_ERROR (Status)) return Status;

  gPCIIO_Count = HandleCount;
  Print (L"Find PCI I/O Protocol: %d\n", HandleCount);

  for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex ++) {
    Status = gBS->HandleProtocol (
      PciHandleBuffer[HandleIndex],
      &gEfiPciIoProtocolGuid,
      (VOID **) &(gPCIIOArray[HandleIndex])  
    );
  }

  return Status;
}

EFI_STATUS
ListPCIMessage (VOID)
{
  UINTN i = 0;
  PCI_TYPE00 Pci;
  UINTN      DescTableSize;
  VIRTIOHDR  PciVirtioHdr;

  for (i = 0; i < gPCIIO_Count; i ++) {
    gPCIIOArray[i]->Pci.Read(
                      gPCIIOArray[i], 
                      EfiPciWidthUint32,
                      0,
                      sizeof (PCI_TYPE00) / sizeof (UINT32),
                      &Pci);
    
    if ((Pci.Hdr.DeviceId >= 0x1000) &&
        (Pci.Hdr.DeviceId <= 0x103F) &&
        (Pci.Hdr.RevisionID <= 0x0000)) {
          Print (L"    Virtio PCI Device Info\n");
          Print (L"    VendorId           - 0x%04x\n", Pci.Hdr.VendorId);
          Print (L"    DeviceId           - 0x%04x\n", Pci.Hdr.DeviceId);
          Print (L"    RevisionID         - 0x%02x\n", Pci.Hdr.RevisionID);
          Print (L"    SubsystemVendorID  - 0x%04x\n", Pci.Device.SubsystemVendorID);
          Print (L"    SubsystemID        - 0x%04x\n", Pci.Device.SubsystemID);
          Print (L"##################################################\n");
          PrintByByte ((UINT8 *) &Pci, sizeof (PCI_TYPE00));
          Print (L"##################################################\n");
      
      if (Pci.Device.Bar[0] & BIT0) {
        gPCIIOArray[i]->Io.Read(
                          gPCIIOArray[i], 
                          EfiPciWidthUint32,
                          PCI_BAR_IDX0,
                          0,
                          sizeof (PciVirtioHdr) / sizeof (UINT32),
                          &PciVirtioHdr
        );
        Print (L"    Virtio Header Info\n");
        Print (L"    Device Features  - 0x%08x\n", PciVirtioHdr.DeviceFeatures);
        Print (L"    Guest Features   - 0x%08x\n", PciVirtioHdr.GuestFeatures);
        Print (L"    Queue Address    - 0x%08x\n", PciVirtioHdr.QueueAddress << EFI_PAGE_SHIFT);
        Print (L"    Queue Size       - 0x%04x\n", PciVirtioHdr.QueueSize);
        Print (L"    Queue Select     - 0x%04x\n", PciVirtioHdr.QueueSelect);
        Print (L"    Queue Notify     - 0x%04x\n", PciVirtioHdr.QueueNotify);
        Print (L"    Device Status    - 0x%02x\n", PciVirtioHdr.DeviceStatus);
        Print (L"    ISR Status       - 0x%02x\n", PciVirtioHdr.ISRStatus);
        
        Print (L"##################################################\n");
        PrintByByte ((UINT8 *) &PciVirtioHdr, sizeof (PciVirtioHdr));
        Print (L"##################################################\n");

        DescTableSize = sizeof (VRING_DESC) * PciVirtioHdr.QueueSize;
        Print (L"    Virtioqueue Descriptors Table\n");
        PrintByByte ((UINT8 *) (UINT64)(PciVirtioHdr.QueueAddress << EFI_PAGE_SHIFT), DescTableSize);
        Print (L"##################################################\n");

        Print (L"    Virtioqueue Avaliable Ring\n");
        PrintByByte ((UINT8 *) (UINT64)((PciVirtioHdr.QueueAddress << EFI_PAGE_SHIFT) + DescTableSize), EFI_PAGE_SIZE - DescTableSize);
        Print (L"##################################################\n");

        Print (L"    Virtioqueue Used Ring\n");
        PrintByByte ((UINT8 *) (UINT64)((PciVirtioHdr.QueueAddress << EFI_PAGE_SHIFT) + EFI_PAGE_SIZE), EFI_PAGE_SIZE);
        Print (L"##################################################\n");
      }
    }
  }

  return EFI_SUCCESS;
}




/**
  Main entrypoint for DumpVirtioPciDev shell application.

  @param[in]  ImageHandle     The image handle.
  @param[in]  SystemTable     The system table.

  @retval EFI_SUCCESS            Command completed successfully.
  @retval EFI_INVALID_PARAMETER  Command usage error.
  @retval EFI_OUT_OF_RESOURCES   Not enough resources were available to run the command.
  @retval EFI_ABORTED            Aborted by user.
  @retval EFI_NOT_FOUND          The specified PCD is not found.
  @retval Others                 Error status returned from gBS->LocateProtocol.
**/
EFI_STATUS
EFIAPI
DumpVirtioPciDevMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS     Status;

  Status = LocatePCIIO ();
  if (EFI_ERROR (Status)) {
    Print (L"Can not locate PCI IO devices");
    return Status;
  }

  Status = ListPCIMessage ();
  
  return Status;
}



  
