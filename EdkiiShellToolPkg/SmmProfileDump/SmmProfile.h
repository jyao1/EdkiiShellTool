/** @file

Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __SMM_PROFILE_H__
#define __SMM_PROFILE_H__

#define SMM_PROFILE_NAME            L"SmmProfileData"
#define SMM_PROFILE_GUID            {0xA3FF0EF5, 0x0C28, 0x42f5, { 0xB5, 0x44, 0x8C, 0x7D, 0xE1, 0xE8, 0x00, 0x14 }}

typedef struct {
  UINT64  HeaderSize;
  UINT64  MaxDataEntries;
  UINT64  MaxDataSize;
  UINT64  CurDataEntries;
  UINT64  CurDataSize;
  UINT64  TsegStart;
  UINT64  TsegSize;
  UINT64  NumSmis;
  UINT64  NumCpus;
} SMM_PROFILE_HEADER;

typedef struct {
  UINT64  SmiNum;
  UINT64  CpuNum;
  UINT64  ApicId;
  UINT64  ErrorCode;
  UINT64  Instruction;
  UINT64  Address;
  UINT64  SmiCmd;
} SMM_PROFILE_ENTRY;

#endif