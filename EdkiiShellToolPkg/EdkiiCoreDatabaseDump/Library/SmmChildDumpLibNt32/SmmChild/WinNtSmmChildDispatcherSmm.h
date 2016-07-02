/** @file

Copyright (c) 2006 - 2007, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

  WinNtGop.h

Abstract:

  Private data for the Gop driver that is bound to the WinNt Thunk protocol


**/
#ifndef _WIN_NT_SMM_CHILD_DISPATCHER_SMM_H_
#define _WIN_NT_SMM_CHILD_DISPATCHER_SMM_H_

#include <PiDxe.h>

#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include <Library/PcdLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/SmmServicesTableLib.h>

//
// "DATABASE" RECORD
// Linked list data structures
//
#define DATABASE_RECORD_SIGNATURE SIGNATURE_32 ('D', 'B', 'R', 'C')

typedef struct {
  UINT32                        Signature;
  LIST_ENTRY                    Link;

  //
  // Register Context
  //
  EFI_HANDLE                    DispatchHandle;
  EFI_SMM_HANDLER_ENTRY_POINT2  DispatchFunction;
  EFI_SMM_SW_REGISTER_CONTEXT   RegisterContext;
} DATABASE_RECORD;

#define DATABASE_RECORD_FROM_LINK(_record)  CR (_record, DATABASE_RECORD, Link, DATABASE_RECORD_SIGNATURE)

#define  MAXIMUM_SWI_VALUE 0xFF

//
// Create private data for the protocols that we'll publish
//
typedef struct {
  LIST_ENTRY                     CallbackDatabase;
  EFI_HANDLE                     SmiHandle;
  EFI_HANDLE                     SmmHandle;
  EFI_SMM_SW_DISPATCH2_PROTOCOL  SmmSwDispatch2Protocols;
} PRIVATE_DATA;

#endif
