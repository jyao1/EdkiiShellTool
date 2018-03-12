/** @file

Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __PEI_PMR_H__
#define __PEI_PMR_H__

#include <Uefi.h>

#define VTD_INFO_GUID { \
  0x222f5e30, 0x5cd, 0x49c6, { 0x8a, 0xc, 0x36, 0xd6, 0x58, 0x41, 0xe0, 0x82 } \
  }

#define DMA_BUFFER_INFO_GUID { \
  0x7b624ec7, 0xfb67, 0x4f9c, { 0xb6, 0xb0, 0x4d, 0xfa, 0x9c, 0x88, 0x20, 0x39 } \
  }

typedef struct {
  UINT32                            DmaBufferBase;
  UINT32                            DmaBufferSize;
  UINT32                            DmaBufferCurrentTop;
  UINT32                            DmaBufferCurrentBottom;
} DMA_BUFFER_INFO_32;

typedef struct {
  UINT64                            DmaBufferBase;
  UINT64                            DmaBufferSize;
  UINT64                            DmaBufferCurrentTop;
  UINT64                            DmaBufferCurrentBottom;
} DMA_BUFFER_INFO_64;

#endif
