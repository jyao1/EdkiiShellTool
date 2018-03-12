/** @file

Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __IOMMU_MAP_H__
#define __IOMMU_MAP_H__

#include <Uefi.h>
#include <Protocol/IoMmu.h>

#define MAP_HANDLE_INFO_SIGNATURE  SIGNATURE_32 ('H', 'M', 'A', 'P')

typedef struct {
  UINT32                                    Signature;
  LIST_ENTRY                                Link;
  EFI_HANDLE                                Handle;
  UINT64                                    IoMmuAccess;
} MAP_HANDLE_INFO;

#define MAP_INFO_SIGNATURE  SIGNATURE_32 ('D', 'M', 'A', 'P')
typedef struct {
  UINT32                                    Signature;
  LIST_ENTRY                                Link;
  EDKII_IOMMU_OPERATION                     Operation;
  UINTN                                     NumberOfBytes;
  UINTN                                     NumberOfPages;
  EFI_PHYSICAL_ADDRESS                      HostAddress;
  EFI_PHYSICAL_ADDRESS                      DeviceAddress;
  LIST_ENTRY                                HandleList;
} MAP_INFO;
#define MAP_INFO_FROM_LINK(a) CR (a, MAP_INFO, Link, MAP_INFO_SIGNATURE)

#endif
