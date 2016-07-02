/** @file
  Esrt management implementation head file.

Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _DXE_ESRT_IMPL_H_
#define _DXE_ESRT_IMPL_H_

//
// Name of  Variable for Non-FMP ESRT Repository
// 
#define EFI_ESRT_NONFMP_VARIABLE_NAME    L"EsrtNonFmp"

//
// Name of Variable for FMP
// 
#define EFI_ESRT_FMP_VARIABLE_NAME       L"EsrtFmp"

//
// Attribute of Cached ESRT entry
//
#define ESRT_FROM_FMP                    0x00000001
#define ESRT_FROM_NONFMP                 0x00000002


//
// Driver GUID
//
#define ESRT_DXE_DRIVER_GUID  { 0x999BD818, 0x7DF7, 0x4A9A,{ 0xA5, 0x02, 0x9B, 0x75, 0x03, 0x3E, 0x6A, 0x0F } }


#endif // #ifndef _EFI_ESRT_IMPL_H_

