/** @file

Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef  _SMM_CORE_DUMP_COMM_H_
#define  _SMM_CORE_DUMP_COMM_H_

#pragma pack(1)

#define SMM_CORE_NAME_STRING  36

typedef struct {
  UINT32                       Signature;
  UINT32                       Length;
  UINT32                       Revision;
} SMM_CORE_DATABASE_COMMON_HEADER;

#define SMM_CORE_IMAGE_DATABASE_SIGNATURE SIGNATURE_32 ('S','C','I','D')
#define SMM_CORE_IMAGE_DATABASE_REVISION  0x0001

typedef struct {
  SMM_CORE_DATABASE_COMMON_HEADER     Header;
  EFI_GUID                            FileGuid;
  CHAR16                              NameString[SMM_CORE_NAME_STRING];
  UINTN                               EntryPoint;
  UINTN                               ImageBase;
  UINTN                               ImageSize;
  UINTN                               RealImageBase;
} SMM_CORE_IMAGE_DATABASE_STRUCTURE;

#define SMM_CORE_HANDLE_DATABASE_SIGNATURE SIGNATURE_32 ('S','C','H','D')
#define SMM_CORE_HANDLE_DATABASE_REVISION  0x0001

typedef struct {
  EFI_GUID   Guid;
  UINTN      Interface;
  UINTN      ImageRef;
} SMM_CORE_HANDLE_PROTOCOL_STRUCTURE;

typedef struct {
  SMM_CORE_DATABASE_COMMON_HEADER     Header;
  UINTN                               Handle;
  UINTN                               ProtocolCount;
//SMM_CORE_HANDLE_PROTOCOL_STRUCTURE  Protocol[ProtocolCount];
} SMM_CORE_HANDLE_DATABASE_STRUCTURE;

#define SMM_CORE_PROTOCOL_DATABASE_SIGNATURE SIGNATURE_32 ('S','C','P','D')
#define SMM_CORE_PROTOCOL_DATABASE_REVISION  0x0001

typedef struct {
  UINTN      Interface;
  UINTN      ImageRef;
} SMM_CORE_PROTOCOL_INTERFACE_STRUCTURE;

typedef struct {
  UINTN      Notify;
  UINTN      ImageRef;
} SMM_CORE_PROTOCOL_NOTIFY_STRUCTURE;

typedef struct {
  SMM_CORE_DATABASE_COMMON_HEADER        Header;
  EFI_GUID                               Guid;
  UINTN                                  InterfaceCount;
  UINTN                                  NotifyCount;
//SMM_CORE_PROTOCOL_INTERFACE_STRUCTURE  Interface[InterfaceCount];
//SMM_CORE_PROTOCOL_NOTIFY_STRUCTURE     Notify[NotifyCount];
} SMM_CORE_PROTOCOL_DATABASE_STRUCTURE;

#define SMM_CORE_SMI_DATABASE_SIGNATURE SIGNATURE_32 ('S','C','S','D')
#define SMM_CORE_SMI_DATABASE_REVISION  0x0001

typedef struct {
  UINTN      SmiHandle;
  UINTN      Handler;
  UINTN      ImageRef;
} SMM_CORE_SMI_HANDLER_STRUCTURE;

typedef struct {
  SMM_CORE_DATABASE_COMMON_HEADER     Header;
  UINT8                               IsRootHandler;
  UINT8                               Reserved[3];
  EFI_GUID                            HandlerType;
  UINTN                               HandlerCount;
//SMM_CORE_SMI_HANDLER_STRUCTURE      Handler[HandlerCount];
} SMM_CORE_SMI_DATABASE_STRUCTURE;

//
// Layout:
// +-------------------------------------+
// | SMM_CORE_IMAGE_DATABASE_STRUCTURE    |
// +-------------------------------------+
// | SMM_CORE_HANDLE_DATABASE_STRUCTURE   |
// +-------------------------------------+
// | SMM_CORE_PROTOCOL_DATABASE_STRUCTURE |
// +-------------------------------------+
// | SMM_CORE_SMI_DATABASE_STRUCTURE      |
// +-------------------------------------+
//



//
// SMM_CORE dump command
//
#define SMM_CORE_DUMP_COMMAND_GET_INFO           0x1
#define SMM_CORE_DUMP_COMMAND_GET_DATA_BY_OFFSET 0x2

typedef struct {
  UINT32                            Command;
  UINT32                            DataLength;
  UINT64                            ReturnStatus;
} SMM_CORE_DUMP_PARAMETER_HEADER;

typedef struct {
  SMM_CORE_DUMP_PARAMETER_HEADER    Header;
  UINT64                            DumpSize;
} SMM_CORE_DUMP_PARAMETER_GET_INFO;

typedef struct {
  SMM_CORE_DUMP_PARAMETER_HEADER    Header;
  //
  // On input, dump buffer size.
  // On output, actual dump data size copied.
  //
  UINT64                            DumpSize;
  PHYSICAL_ADDRESS                  DumpBuffer;
  //
  // On input, dump buffer offset to copy.
  // On output, next time dump buffer offset to copy.
  //
  UINT64                            DumpOffset;
} SMM_CORE_DUMP_PARAMETER_GET_DATA_BY_OFFSET;

#define SMM_CORE_DUMP_GUID {0x49174342, 0x7108, 0x409b, {0x8b, 0xbe, 0x65, 0xfd, 0xa8, 0x53, 0x89, 0xf5}}

#pragma pack()

#endif
