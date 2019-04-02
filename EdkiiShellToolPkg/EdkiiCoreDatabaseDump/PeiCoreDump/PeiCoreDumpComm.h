/** @file

Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef  _PEI_CORE_DUMP_PRIVATE_H_
#define  _EPI_CORE_DUMP_PRIVATE_H_

#pragma pack(1)

#define PEI_CORE_NAME_STRING  36

typedef struct {
  UINT32                       Signature;
  UINT32                       Length;
  UINT32                       Revision;
} PEI_CORE_DATABASE_COMMON_HEADER;

#define PEI_CORE_IMAGE_DATABASE_SIGNATURE SIGNATURE_32 ('P','C','I','D')
#define PEI_CORE_IMAGE_DATABASE_REVISION  0x0001

typedef struct {
  PEI_CORE_DATABASE_COMMON_HEADER     Header;
  EFI_GUID                            FileGuid;
  CHAR16                              NameString[PEI_CORE_NAME_STRING];
  UINT64                              EntryPoint;
  UINT64                              ImageBase;
  UINT64                              ImageSize;
  UINT64                              RealImageBase;
} PEI_CORE_IMAGE_DATABASE_STRUCTURE;

#define PEI_CORE_PPI_DATABASE_SIGNATURE SIGNATURE_32 ('P','C','P','D')
#define PEI_CORE_PPI_DATABASE_REVISION  0x0001

typedef struct {
  UINT64     Flags;
  EFI_GUID   Guid;
  UINT64     Ppi;
  UINT64     ImageRef;
} PEI_CORE_PPI_STRUCTURE_PPI;

typedef struct {
  UINT64     Flags;
  EFI_GUID   Guid;
  UINT64     Notify;
  UINT64     ImageRef;
} PEI_CORE_PPI_STRUCTURE_NOTIFY;

typedef union {
  PEI_CORE_PPI_STRUCTURE_PPI     Ppi;
  PEI_CORE_PPI_STRUCTURE_NOTIFY  Notify;
} PEI_CORE_PPI_STRUCTURE;

typedef struct {
  PEI_CORE_DATABASE_COMMON_HEADER     Header;
  UINT64                              PpiListCurrentCount;
  UINT64                              PpiListMaxCount;
  UINT64                              PpiListLastDispatchedCount;
  UINT64                              CallbackNotifyListCurrentCount;
  UINT64                              CallbackNotifyListMaxCount;
  UINT64                              DispatchNotifyListCurrentCount;
  UINT64                              DispatchNotifyListMaxCount;
  UINT64                              DispatchNotifyListLastDispatchedCount;
//PEI_PPI_LIST_POINTERS               PpiListPpiPtrs[];
//PEI_PPI_LIST_POINTERS               CallbackNotifyListNotifyPtrs[];
//PEI_PPI_LIST_POINTERS               DispatchNotifyListNotifyPtrs[];
} PEI_CORE_PPI_DATABASE_STRUCTURE;

#define PEI_CORE_MEMORY_INFO_SIGNATURE SIGNATURE_32 ('P','C','M','I')
#define PEI_CORE_MEMORY_INFO_REVISION  0x0001

typedef struct {
  PEI_CORE_DATABASE_COMMON_HEADER     Header;
  EFI_PHYSICAL_ADDRESS                PhysicalMemoryBegin;
  UINT64                              PhysicalMemoryLength;
  EFI_PHYSICAL_ADDRESS                FreePhysicalMemoryTop;
} PEI_CORE_MEMORY_INFO_STRUCTURE;

//
// Layout:
// +-----------------------------------+
// | PEI_CORE_IMAGE_DATABASE_STRUCTURE |
// +-----------------------------------+
// | PEI_CORE_PPI_DATABASE_STRUCTURE   |
// +-----------------------------------+
// | PEI_CORE_MEMORY_INFO_STRUCTURE    |
// +-----------------------------------+
//

#define PEI_CORE_DUMP_GUID {0x122065da, 0xb058, 0x4fbc, {0xb2, 0x33, 0x34, 0x4f, 0xfc, 0xa9, 0x93, 0xb5}}

#pragma pack()

#endif
