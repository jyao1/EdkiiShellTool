/** @file
  The internal header file includes the common header files, defines
  internal structure and functions used by SmmCore module.

  Copyright (c) 2009 - 2015, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials are licensed and made available 
  under the terms and conditions of the BSD License which accompanies this 
  distribution.  The full text of the license may be found at        
  http://opensource.org/licenses/bsd-license.php                                            

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

**/

#ifndef _SMM_CORE_H_
#define _SMM_CORE_H_

#include <Protocol/FirmwareVolume2.h>

//
// Structure for recording the state of an SMM Driver
//
#define EFI_SMM_DRIVER_ENTRY_SIGNATURE SIGNATURE_32('s', 'd','r','v')

typedef struct {
  UINTN                           Signature;
  LIST_ENTRY                      Link;             // mDriverList

  LIST_ENTRY                      ScheduledLink;    // mScheduledQueue

  EFI_HANDLE                      FvHandle;
  EFI_GUID                        FileName;
  EFI_DEVICE_PATH_PROTOCOL        *FvFileDevicePath;
  EFI_FIRMWARE_VOLUME2_PROTOCOL   *Fv;

  VOID                            *Depex;
  UINTN                           DepexSize;

  BOOLEAN                         Before;
  BOOLEAN                         After;
  EFI_GUID                        BeforeAfterGuid;

  BOOLEAN                         Dependent;
  BOOLEAN                         Scheduled;
  BOOLEAN                         Initialized;
  BOOLEAN                         DepexProtocolError;

  EFI_HANDLE                      ImageHandle;
  EFI_LOADED_IMAGE_PROTOCOL       *LoadedImage;
  //
  // Image EntryPoint in SMRAM
  //
  PHYSICAL_ADDRESS                ImageEntryPoint;
  //
  // Image Buffer in SMRAM  
  //
  PHYSICAL_ADDRESS                ImageBuffer;
  //
  // Image Page Number
  //
  UINTN                           NumberOfPage;
  EFI_HANDLE                      SmmImageHandle;
  EFI_LOADED_IMAGE_PROTOCOL       SmmLoadedImage;
} EFI_SMM_DRIVER_ENTRY;

#define EFI_HANDLE_SIGNATURE            SIGNATURE_32('h','n','d','l')

///
/// IHANDLE - contains a list of protocol handles
///
typedef struct {
  UINTN               Signature;
  /// All handles list of IHANDLE
  LIST_ENTRY          AllHandles;
  /// List of PROTOCOL_INTERFACE's for this handle
  LIST_ENTRY          Protocols;
  UINTN               LocateRequest;
} IHANDLE;

#define ASSERT_IS_HANDLE(a)  ASSERT((a)->Signature == EFI_HANDLE_SIGNATURE)

#define PROTOCOL_ENTRY_SIGNATURE        SIGNATURE_32('p','r','t','e')

///
/// PROTOCOL_ENTRY - each different protocol has 1 entry in the protocol
/// database.  Each handler that supports this protocol is listed, along
/// with a list of registered notifies.
///
typedef struct {
  UINTN               Signature;
  /// Link Entry inserted to mProtocolDatabase
  LIST_ENTRY          AllEntries;
  /// ID of the protocol
  EFI_GUID            ProtocolID;
  /// All protocol interfaces
  LIST_ENTRY          Protocols;
  /// Registerd notification handlers
  LIST_ENTRY          Notify;
} PROTOCOL_ENTRY;

#define PROTOCOL_INTERFACE_SIGNATURE  SIGNATURE_32('p','i','f','c')

///
/// PROTOCOL_INTERFACE - each protocol installed on a handle is tracked
/// with a protocol interface structure
///
typedef struct {
  UINTN                       Signature;
  /// Link on IHANDLE.Protocols
  LIST_ENTRY                  Link;
  /// Back pointer
  IHANDLE                     *Handle;
  /// Link on PROTOCOL_ENTRY.Protocols
  LIST_ENTRY                  ByProtocol;
  /// The protocol ID
  PROTOCOL_ENTRY              *Protocol;
  /// The interface value
  VOID                        *Interface;
} PROTOCOL_INTERFACE;

#define PROTOCOL_NOTIFY_SIGNATURE       SIGNATURE_32('p','r','t','n')

///
/// PROTOCOL_NOTIFY - used for each register notification for a protocol
///
typedef struct {
  UINTN               Signature;
  PROTOCOL_ENTRY      *Protocol;
  /// All notifications for this protocol
  LIST_ENTRY          Link;
  /// Notification function
  EFI_SMM_NOTIFY_FN   Function;
  /// Last position notified
  LIST_ENTRY          *Position;
} PROTOCOL_NOTIFY;

#endif
