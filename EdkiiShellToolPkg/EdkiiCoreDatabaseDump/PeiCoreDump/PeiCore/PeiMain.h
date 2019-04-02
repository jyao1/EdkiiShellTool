/** @file
  Definition of Pei Core Structures and Services
  
Copyright (c) 2006 - 2015, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _PEI_MAIN_H_
#define _PEI_MAIN_H_

#include <Ppi/DxeIpl.h>
#include <Ppi/FirmwareVolume.h>
#include <Ppi/Security2.h>
#include <Library/PeCoffLib.h>

///
/// It is an FFS type extension used for PeiFindFileEx. It indicates current
/// Ffs searching is for all PEIMs can be dispatched by PeiCore.
///
#define PEI_CORE_INTERNAL_FFS_FILE_DISPATCH_TYPE   0xff

///
/// Pei Core private data structures
///
typedef union {
  EFI_PEI_PPI_DESCRIPTOR      *Ppi;
  EFI_PEI_NOTIFY_DESCRIPTOR   *Notify;
  VOID                        *Raw;
} PEI_PPI_LIST_POINTERS;

///
/// Number of PEI_PPI_LIST_POINTERS to grow by each time we run out of room
///

typedef struct {
  UINTN                 CurrentCount;
  UINTN                 MaxCount;
  UINTN                 LastDispatchedCount;
  ///
  /// MaxCount number of entries.
  ///
  PEI_PPI_LIST_POINTERS *PpiPtrs;
} PEI_PPI_LIST;

typedef struct {
  UINTN                 CurrentCount;
  UINTN                 MaxCount;
  ///
  /// MaxCount number of entries.
  ///
  PEI_PPI_LIST_POINTERS *NotifyPtrs;
} PEI_CALLBACK_NOTIFY_LIST;

typedef struct {
  UINTN                 CurrentCount;
  UINTN                 MaxCount;
  UINTN                 LastDispatchedCount;
  ///
  /// MaxCount number of entries.
  ///
  PEI_PPI_LIST_POINTERS *NotifyPtrs;
} PEI_DISPATCH_NOTIFY_LIST;

///
/// PPI database structure which contains three links:
/// PpiList, CallbackNotifyList and DispatchNotifyList.
///
typedef struct {
  ///
  /// PPI List.
  ///
  PEI_PPI_LIST              PpiList;
  ///
  /// Notify List at dispatch level.
  ///
  PEI_CALLBACK_NOTIFY_LIST  CallbackNotifyList;
  ///
  /// Notify List at callback level.
  ///
  PEI_DISPATCH_NOTIFY_LIST  DispatchNotifyList;
} PEI_PPI_DATABASE;

//
// PEI_CORE_FV_HANDE.PeimState
// Do not change these values as there is code doing math to change states.
// Look for Private->Fv[FvCount].PeimState[PeimCount]++;
//
#define PEIM_STATE_NOT_DISPATCHED         0x00
#define PEIM_STATE_DISPATCHED             0x01
#define PEIM_STATE_REGISTER_FOR_SHADOW    0x02
#define PEIM_STATE_DONE                   0x03

typedef struct {
  EFI_FIRMWARE_VOLUME_HEADER          *FvHeader;
  EFI_PEI_FIRMWARE_VOLUME_PPI         *FvPpi;
  EFI_PEI_FV_HANDLE                   FvHandle;
  UINTN                               PeimCount;
  //
  // Ponter to the buffer with the PeimCount number of Entries.
  //
  UINT8                               *PeimState;
  //
  // Ponter to the buffer with the PeimCount number of Entries.
  //
  EFI_PEI_FILE_HANDLE                 *FvFileHandles;
  BOOLEAN                             ScanFv;
  UINT32                              AuthenticationStatus;
} PEI_CORE_FV_HANDLE;

typedef struct {
  EFI_GUID                            FvFormat;
  VOID                                *FvInfo;
  UINT32                              FvInfoSize;
  UINT32                              AuthenticationStatus;
  EFI_PEI_NOTIFY_DESCRIPTOR           NotifyDescriptor;
} PEI_CORE_UNKNOW_FORMAT_FV_INFO;

#define CACHE_SETION_MAX_NUMBER       0x10
typedef struct {
  EFI_COMMON_SECTION_HEADER*          Section[CACHE_SETION_MAX_NUMBER];
  VOID*                               SectionData[CACHE_SETION_MAX_NUMBER];
  UINTN                               SectionSize[CACHE_SETION_MAX_NUMBER];
  UINT32                              AuthenticationStatus[CACHE_SETION_MAX_NUMBER];
  UINTN                               AllSectionCount;
  UINTN                               SectionIndex;
} CACHE_SECTION_DATA;

#define HOLE_MAX_NUMBER       0x3
typedef struct {
  EFI_PHYSICAL_ADDRESS               Base;
  UINTN                              Size;
  UINTN                              Offset;
  BOOLEAN                            OffsetPositive;
} HOLE_MEMORY_DATA;

///
/// Forward declaration for PEI_CORE_INSTANCE
///
typedef struct _PEI_CORE_INSTANCE  PEI_CORE_INSTANCE;


/**
  Function Pointer type for PeiCore function.
  @param SecCoreData     Points to a data structure containing SEC to PEI handoff data, such as the size
                         and location of temporary RAM, the stack location and the BFV location.
  @param PpiList         Points to a list of one or more PPI descriptors to be installed initially by the PEI core.
                         An empty PPI list consists of a single descriptor with the end-tag
                         EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST. As part of its initialization
                         phase, the PEI Foundation will add these SEC-hosted PPIs to its PPI database such
                         that both the PEI Foundation and any modules can leverage the associated service
                         calls and/or code in these early PPIs
  @param OldCoreData     Pointer to old core data that is used to initialize the
                         core's data areas.
**/
typedef
EFI_STATUS
(EFIAPI *PEICORE_FUNCTION_POINTER)(
  IN CONST  EFI_SEC_PEI_HAND_OFF    *SecCoreData,
  IN CONST  EFI_PEI_PPI_DESCRIPTOR  *PpiList,
  IN PEI_CORE_INSTANCE              *OldCoreData
  );

#define PEI_CORE_HANDLE_SIGNATURE  SIGNATURE_32('P','e','i','C')

///
/// Pei Core private data structure instance
///
struct _PEI_CORE_INSTANCE {
  UINTN                              Signature;

  ///
  /// Point to ServiceTableShadow
  ///
  EFI_PEI_SERVICES                   *Ps;
  PEI_PPI_DATABASE                   PpiData;

  ///
  /// The count of FVs which contains FFS and could be dispatched by PeiCore.
  ///
  UINTN                              FvCount;

  ///
  /// The max count of FVs which contains FFS and could be dispatched by PeiCore.
  ///
  UINTN                              MaxFvCount;

  ///
  /// Pointer to the buffer with the MaxFvCount number of entries.
  /// Each entry is for one FV which contains FFS and could be dispatched by PeiCore.
  ///
  PEI_CORE_FV_HANDLE                 *Fv;

  ///
  /// Pointer to the buffer with the MaxUnknownFvInfoCount number of entries.
  /// Each entry is for one FV which could not be dispatched by PeiCore.
  ///
  PEI_CORE_UNKNOW_FORMAT_FV_INFO     *UnknownFvInfo;
  UINTN                              MaxUnknownFvInfoCount;
  UINTN                              UnknownFvInfoCount;

  ///
  /// Pointer to the buffer FvFileHandlers in PEI_CORE_FV_HANDLE specified by CurrentPeimFvCount.
  ///
  EFI_PEI_FILE_HANDLE                *CurrentFvFileHandles;
  UINTN                              AprioriCount;
  UINTN                              CurrentPeimFvCount;
  UINTN                              CurrentPeimCount;
  EFI_PEI_FILE_HANDLE                CurrentFileHandle;
  BOOLEAN                            PeimNeedingDispatch;
  BOOLEAN                            PeimDispatchOnThisPass;
  BOOLEAN                            PeimDispatcherReenter;
  EFI_PEI_HOB_POINTERS               HobList;
  BOOLEAN                            SwitchStackSignal;
  BOOLEAN                            PeiMemoryInstalled;
  VOID                               *CpuIo;
  EFI_PEI_SECURITY2_PPI              *PrivateSecurityPpi;
  EFI_PEI_SERVICES                   ServiceTableShadow;
  EFI_PEI_PPI_DESCRIPTOR             *XipLoadFile;
  EFI_PHYSICAL_ADDRESS               PhysicalMemoryBegin;
  UINT64                             PhysicalMemoryLength;
  EFI_PHYSICAL_ADDRESS               FreePhysicalMemoryTop;
  UINTN                              HeapOffset;
  BOOLEAN                            HeapOffsetPositive;
  UINTN                              StackOffset;
  BOOLEAN                            StackOffsetPositive;
  //
  // Information for migrating memory pages allocated in pre-memory phase.
  //
  HOLE_MEMORY_DATA                   MemoryPages;
  PEICORE_FUNCTION_POINTER           ShadowedPeiCore;
  CACHE_SECTION_DATA                 CacheSection;
  //
  // For Loading modules at fixed address feature to cache the top address below which the
  // Runtime code, boot time code and PEI memory will be placed. Please note that the offset between this field
  // and  Ps should not be changed since maybe user could get this top address by using the offet to Ps.
  //
  EFI_PHYSICAL_ADDRESS               LoadModuleAtFixAddressTopAddress;
  //
  // The field is define for Loading modules at fixed address feature to tracker the PEI code
  // memory range usage. It is a bit mapped array in which every bit indicates the correspoding memory page
  // available or not.
  //
  UINT64                            *PeiCodeMemoryRangeUsageBitMap;
  //
  // This field points to the shadowed image read function
  //
  PE_COFF_LOADER_READ_FILE          ShadowedImageRead;

  UINTN                             TempPeimCount;

  //
  // Pointer to the temp buffer with the TempPeimCount number of entries.
  //
  EFI_PEI_FILE_HANDLE               *TempFileHandles;
  //
  // Pointer to the temp buffer with the TempPeimCount number of entries.
  //
  EFI_GUID                          *TempFileGuid;

  //
  // Temp Memory Range is not covered by PeiTempMem and Stack.
  // Those Memory Range will be migrated into physical memory.
  //
  HOLE_MEMORY_DATA                  HoleData[HOLE_MAX_NUMBER];
};

///
/// Pei Core Instance Data Macros
///
#define PEI_CORE_INSTANCE_FROM_PS_THIS(a) \
  CR(a, PEI_CORE_INSTANCE, Ps, PEI_CORE_HANDLE_SIGNATURE)

///
/// Union of temporarily used function pointers (to save stack space)
///
typedef union {
  PEICORE_FUNCTION_POINTER     PeiCore;
  EFI_PEIM_ENTRY_POINT2        PeimEntry;
  EFI_PEIM_NOTIFY_ENTRY_POINT  PeimNotifyEntry;
  EFI_DXE_IPL_PPI              *DxeIpl;
  EFI_PEI_PPI_DESCRIPTOR       *PpiDescriptor;
  EFI_PEI_NOTIFY_DESCRIPTOR    *NotifyDescriptor;
  VOID                         *Raw;
} PEI_CORE_TEMP_POINTERS;

typedef struct {
  CONST EFI_SEC_PEI_HAND_OFF    *SecCoreData;
  EFI_PEI_PPI_DESCRIPTOR        *PpiList;
  VOID                          *Data;
} PEI_CORE_PARAMETERS;

#endif
