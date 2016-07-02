/** @file

Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef  _SMM_CHILD_DUMP_COMM_H_
#define  _SMM_CHILD_DUMP_COMM_H_

#pragma pack(1)

typedef struct {
  UINT32                       Signature;
  UINT32                       Length;
  UINT32                       Revision;
} SMM_CHILD_DATABASE_COMMON_HEADER;

#define SMM_CHILD_SMI_DATABASE_SIGNATURE SIGNATURE_32 ('C','D','S','D')
#define SMM_CHILD_SMI_DATABASE_REVISION  0x0001

typedef struct {
  UINTN      SmiHandle;
  UINTN      Handler;
  UINTN      ImageRef;
} SMM_CHILD_SMI_HANDLER_STRUCTURE_HEADER;

typedef struct {
  UINTN      SmiHandle;
  UINTN      Handler;
  UINTN      ImageRef;
  EFI_SMM_SW_REGISTER_CONTEXT  Context;
} SMM_CHILD_SW_SMI_HANDLER_STRUCTURE;

typedef struct {
  UINTN      SmiHandle;
  UINTN      Handler;
  UINTN      ImageRef;
  EFI_SMM_SX_REGISTER_CONTEXT  Context;
} SMM_CHILD_SX_SMI_HANDLER_STRUCTURE;

typedef struct {
  UINTN      SmiHandle;
  UINTN      Handler;
  UINTN      ImageRef;
  EFI_SMM_PERIODIC_TIMER_REGISTER_CONTEXT  Context;
} SMM_CHILD_PERIODIC_TIMER_SMI_HANDLER_STRUCTURE;

#define SMM_CHILD_USB_REGISTER_CONTEXT_MAX_SIZE  0x40
typedef struct {
  EFI_USB_SMI_TYPE          Type;
  UINT8                     Device[SMM_CHILD_USB_REGISTER_CONTEXT_MAX_SIZE];
} SMM_CHILD_USB_REGISTER_CONTEXT;
typedef struct {
  UINTN      SmiHandle;
  UINTN      Handler;
  UINTN      ImageRef;
  SMM_CHILD_USB_REGISTER_CONTEXT  Context;
} SMM_CHILD_USB_SMI_HANDLER_STRUCTURE;

typedef struct {
  UINTN      SmiHandle;
  UINTN      Handler;
  UINTN      ImageRef;
  EFI_SMM_GPI_REGISTER_CONTEXT  Context;
} SMM_CHILD_GPI_SMI_HANDLER_STRUCTURE;

typedef struct {
  UINTN      SmiHandle;
  UINTN      Handler;
  UINTN      ImageRef;
  EFI_SMM_POWER_BUTTON_REGISTER_CONTEXT  Context;
} SMM_CHILD_POWER_BUTTON_SMI_HANDLER_STRUCTURE;

typedef struct {
  UINTN      SmiHandle;
  UINTN      Handler;
  UINTN      ImageRef;
  EFI_SMM_STANDBY_BUTTON_REGISTER_CONTEXT  Context;
} SMM_CHILD_STANDBY_BUTTON_SMI_HANDLER_STRUCTURE;

typedef struct {
  UINTN      SmiHandle;
  UINTN      Handler;
  UINTN      ImageRef;
  EFI_SMM_IO_TRAP_REGISTER_CONTEXT  Context;
} SMM_CHILD_IO_TRAP_SMI_HANDLER_STRUCTURE;

typedef struct {
  SMM_CHILD_DATABASE_COMMON_HEADER       Header;
  EFI_GUID                               Guid;
  UINTN                                  HandlerCount;
//SMM_CHILD_XXX_SMI_HANDLER_STRUCTURE    Handler[HandlerCount];
} SMM_CHILD_SMI_DATABASE_STRUCTURE;

#pragma pack()

#endif
