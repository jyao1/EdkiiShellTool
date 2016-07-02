/** @file

Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <PiDxe.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>

CHAR16 *mGcdMemoryTypeShortName[] = {
  L" NE   ",
  L" RSVD ",
  L" SYS  ",
  L" MMIO ",
  L" PERS ",
  L" RELI ",
};

CHAR16 mUnknownStr[11];

CHAR16 *
ShortNameOfGcdMemoryType(
  IN UINT32 Type
  )
{
  if (Type < sizeof(mGcdMemoryTypeShortName) / sizeof(mGcdMemoryTypeShortName[0])) {
    return mGcdMemoryTypeShortName[Type];
  } else {
    return L" ???? ";
  }
}

CHAR16 *mGcdIoTypeShortName[] = {
  L" NE   ",
  L" RSVD ",
  L" SYS  ",
};

CHAR16 *
ShortNameOfGcdIoType(
  IN UINT32 Type
  )
{
  if (Type < sizeof(mGcdIoTypeShortName) / sizeof(mGcdIoTypeShortName[0])) {
    return mGcdIoTypeShortName[Type];
  }
  else {
    return L" ???? ";
  }
}

CHAR16
EFIAPI
InternalAsciiToUpper (
  IN      CHAR16                     Chr
  )
{
  return (UINT16) ((Chr >= L'a' && Chr <= L'z') ? Chr - (L'a' - L'A') : Chr);
}

/**
  Performs a case insensitive comparison of two Null-terminated ASCII strings,
  and returns the difference between the first mismatched ASCII characters.

  This function performs a case insensitive comparison of the Null-terminated
  ASCII string FirstString to the Null-terminated ASCII string SecondString. If
  FirstString is identical to SecondString, then 0 is returned. Otherwise, the
  value returned is the first mismatched lower case ASCII character in
  SecondString subtracted from the first mismatched lower case ASCII character
  in FirstString.

  If FirstString is NULL, then ASSERT().
  If SecondString is NULL, then ASSERT().
  If PcdMaximumAsciiStringLength is not zero and FirstString contains more than
  PcdMaximumAsciiStringLength ASCII characters, not including the Null-terminator,
  then ASSERT().
  If PcdMaximumAsciiStringLength is not zero and SecondString contains more
  than PcdMaximumAsciiStringLength ASCII characters, not including the
  Null-terminator, then ASSERT().

  @param  FirstString   A pointer to a Null-terminated ASCII string.
  @param  SecondString  A pointer to a Null-terminated ASCII string.

  @retval ==0    FirstString is identical to SecondString using case insensitive
                 comparisons.
  @retval !=0    FirstString is not identical to SecondString using case
                 insensitive comparisons.

**/
INTN
EFIAPI
StriCmp (
  IN      CONST CHAR16               *FirstString,
  IN      CONST CHAR16               *SecondString
  )
{
  CHAR16  UpperFirstString;
  CHAR16  UpperSecondString;

  UpperFirstString  = InternalAsciiToUpper (*FirstString);
  UpperSecondString = InternalAsciiToUpper (*SecondString);
  while ((*FirstString != L'\0') && (UpperFirstString == UpperSecondString)) {
    FirstString++;
    SecondString++;
    UpperFirstString  = InternalAsciiToUpper (*FirstString);
    UpperSecondString = InternalAsciiToUpper (*SecondString);
  }

  return UpperFirstString - UpperSecondString;
}

EFI_ALLOCATE_TYPE
ParseAllocateType (
  IN CHAR16  *String
  )
/*++

Routine Description:

  Parse allocate type

Arguments:

  String  - date

Returns:

  Allocate type

--*/
{
  CHAR16  *Ptr;

  for (Ptr = String; *Ptr != 0; Ptr++) {
    if (*Ptr >= L'a' && *Ptr <= L'z') {
      *Ptr = (CHAR16) (*Ptr - L'a' + L'A');
    }
  }

  if (StriCmp (String, L"ANYUP") == 0) {
    return EfiGcdAllocateAnySearchBottomUp;
  } else if (StriCmp (String, L"MAXUP") == 0) {
    return EfiGcdAllocateMaxAddressSearchBottomUp;
  } else if (StriCmp (String, L"AT") == 0) {
    return EfiGcdAllocateAddress;
  } else if (StriCmp (String, L"ANYDN") == 0) {
    return EfiGcdAllocateAnySearchTopDown;
  } else if (StriCmp (String, L"MAXDN") == 0) {
    return EfiGcdAllocateMaxAddressSearchTopDown;
  }

  return MaxAllocateType;
}

EFI_GCD_MEMORY_TYPE
ParseGcdMemoryType (
  IN CHAR16  *String
  )
/*++

Routine Description:

  Parse GCD memory type

Arguments:

  String  - GCD memory type

Returns:

  Return GCD memory type

--*/
{
  CHAR16  *Ptr;

  for (Ptr = String; *Ptr != 0; Ptr++) {
    if (*Ptr >= L'a' && *Ptr <= L'z') {
      *Ptr = (CHAR16) (*Ptr - L'a' + L'A');
    }
  }

  if (StriCmp (String, L"NE") == 0) {
    return EfiGcdMemoryTypeNonExistent;
  } else if (StriCmp (String, L"RSVD") == 0) {
    return EfiGcdMemoryTypeReserved;
  } else if (StriCmp (String, L"SYS") == 0) {
    return EfiGcdMemoryTypeSystemMemory;
  } else if (StriCmp (String, L"MMIO") == 0) {
    return EfiGcdMemoryTypeMemoryMappedIo;
  } else if (StriCmp (String, L"PERS") == 0) {
    return EfiGcdMemoryTypeMemoryMappedIo;
  } else if (StriCmp (String, L"RELI") == 0) {
    return EfiGcdMemoryTypeMemoryMappedIo;
  }

  return EfiGcdMemoryTypeMaximum;
}

EFI_GCD_IO_TYPE
ParseGcdIoType (
  IN CHAR16  *String
  )
/*++

Routine Description:

  Parse GCDIO type

Arguments:

  String  - Type

Returns:

  Return GCDIO type

--*/
{
  CHAR16  *Ptr;

  for (Ptr = String; *Ptr != 0; Ptr++) {
    if (*Ptr >= L'a' && *Ptr <= L'z') {
      *Ptr = (CHAR16) (*Ptr - L'a' + L'A');
    }
  }

  if (StriCmp (String, L"NE") == 0) {
    return EfiGcdIoTypeNonExistent;
  } else if (StriCmp (String, L"RSVD") == 0) {
    return EfiGcdIoTypeReserved;
  } else if (StriCmp (String, L"IO") == 0) {
    return EfiGcdIoTypeIo;
  }

  return EfiGcdIoTypeMaximum;
}

VOID
PrintBitMask (
  IN UINT64 Bit,
  IN UINT64 Capabilities,
  IN UINT64 Attributes
  )
/*++

Routine Description:

  Print bit mask

Arguments:

  Bit           - Bit
  Capabilities  - Capabilities
  Attributes    - Attributes

Returns:

  None

--*/
{
  if ((Capabilities & Bit) != 0) {
    if ((Attributes & Bit) != 0) {
      Print (L"1");
    } else {
      Print (L"0");
    }
  } else {
    Print (L"-");
  }
}

VOID
PrintMemoryDescriptorHeader (
  VOID
  )
/*++

Routine Description:

  Print Memory descriptor header

Arguments:

  None

Returns:

  None

--*/
{
  if (sizeof(UINT64) == sizeof(UINTN)) {
    Print (L"                                              U                                      \n");
    Print (L"                                       RRMNXRWCWWWU                                  \n");
    Print (L"Base Address     End Address      Type TORVPPPEBTCC Image            Device          \n");
    Print (L"================ ================ ==== ============ ================ ================\n");
  } else {
    Print (L"                                              U                      \n");
    Print (L"                                       RRMNXRWCWWWU                  \n");
    Print (L"Base Address     End Address      Type TORVPPPEBTCC Image    Device  \n");
    Print (L"================ ================ ==== ============ ======== ========\n");
  }
}

VOID
PrintMemoryDescriptor (
  EFI_GCD_MEMORY_SPACE_DESCRIPTOR  *MemoryDescriptor
  )
/*++

Routine Description:

  Print memory descriptor

Arguments:

  MemoryDescriptor  - Pointer to a gcd memory space desciptor

Returns:

  None

--*/
{
  Print (
    L"%016lx-%016lx",
    MemoryDescriptor->BaseAddress,
    MemoryDescriptor->BaseAddress + MemoryDescriptor->Length - 1
    );

  Print (ShortNameOfGcdMemoryType(MemoryDescriptor->GcdMemoryType));

  if (MemoryDescriptor->GcdMemoryType != EfiGcdMemoryTypeNonExistent) {
    PrintBitMask (EFI_MEMORY_RUNTIME, MemoryDescriptor->Capabilities, MemoryDescriptor->Attributes);
    PrintBitMask (EFI_MEMORY_RO, MemoryDescriptor->Capabilities, MemoryDescriptor->Attributes);
    PrintBitMask (EFI_MEMORY_MORE_RELIABLE, MemoryDescriptor->Capabilities, MemoryDescriptor->Attributes);
    PrintBitMask (EFI_MEMORY_NV, MemoryDescriptor->Capabilities, MemoryDescriptor->Attributes);
    PrintBitMask (EFI_MEMORY_XP, MemoryDescriptor->Capabilities, MemoryDescriptor->Attributes);
    PrintBitMask (EFI_MEMORY_RP, MemoryDescriptor->Capabilities, MemoryDescriptor->Attributes);
    PrintBitMask (EFI_MEMORY_WP, MemoryDescriptor->Capabilities, MemoryDescriptor->Attributes);
    PrintBitMask (EFI_MEMORY_UCE, MemoryDescriptor->Capabilities, MemoryDescriptor->Attributes);
    PrintBitMask (EFI_MEMORY_WB, MemoryDescriptor->Capabilities, MemoryDescriptor->Attributes);
    PrintBitMask (EFI_MEMORY_WT, MemoryDescriptor->Capabilities, MemoryDescriptor->Attributes);
    PrintBitMask (EFI_MEMORY_WC, MemoryDescriptor->Capabilities, MemoryDescriptor->Attributes);
    PrintBitMask (EFI_MEMORY_UC, MemoryDescriptor->Capabilities, MemoryDescriptor->Attributes);
  } else {
    Print (L"            ");
  }

  if (sizeof(UINT64) == sizeof(UINTN)) {
    if (MemoryDescriptor->ImageHandle != NULL) {
      Print (L" %016lx", (UINT64)(UINTN)MemoryDescriptor->ImageHandle);
      if (MemoryDescriptor->DeviceHandle != NULL) {
        Print (L" %016lx", (UINT64)(UINTN)MemoryDescriptor->ImageHandle);
      }
    }
  } else {
    if (MemoryDescriptor->ImageHandle != NULL) {
      Print (L" %08x", MemoryDescriptor->ImageHandle);
      if (MemoryDescriptor->DeviceHandle != NULL) {
        Print (L" %08x", MemoryDescriptor->ImageHandle);
      }
    }
  }

  Print (L"\n");
}

VOID
PrintIoDescriptorHeader (
  VOID
  )
/*++

Routine Description:

  Print IOI descriptor header

Arguments:

  None

Returns:

  None

--*/
{
  Print (L"Base Address     End Address      Type Image    Device  \n");
  Print (L"================ ================ ==== ======== ========\n");
}

VOID
PrintIoDescriptor (
  EFI_GCD_IO_SPACE_DESCRIPTOR  *IoDescriptor
  )
/*++

Routine Description:

  Print IO descriptor

Arguments:

  IoDescriptor  - A pointer to the gcd IO space descriptor

Returns:

  None

--*/
{
  Print (
    L"%016lx-%016lx",
    IoDescriptor->BaseAddress,
    IoDescriptor->BaseAddress + IoDescriptor->Length - 1
    );

  Print (ShortNameOfGcdIoType (IoDescriptor->GcdIoType));

  if (IoDescriptor->ImageHandle != NULL) {
    Print (L" %08x", IoDescriptor->ImageHandle);
    if (IoDescriptor->DeviceHandle != NULL) {
      Print (L" %08x", IoDescriptor->ImageHandle);
    }
  }

  Print (L"\n");
}

EFI_STATUS
EFIAPI
InitializeGcd (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:
  Test GCD Services

Arguments:
  ImageHandle     The image handle. 
  SystemTable     The system table.

Returns:
  EFI_SUCCESS             - Command completed successfully
  EFI_INVALID_PARAMETER   - Command usage error

--*/
{
  EFI_STATUS                      Status;
  EFI_GCD_MEMORY_SPACE_DESCRIPTOR *MemoryMap;
  EFI_GCD_IO_SPACE_DESCRIPTOR     *IoMap;
  UINTN                           NumberOfDescriptors;
  UINTN                           Index;

  Print(L"GCD MEM:\n");
  NumberOfDescriptors = 0;
  MemoryMap           = NULL;
  Status = gDS->GetMemorySpaceMap (
                  &NumberOfDescriptors,
                  &MemoryMap
                  );
  if (!EFI_ERROR (Status)) {
    PrintMemoryDescriptorHeader ();
    for (Index = 0; Index < NumberOfDescriptors; Index++) {
      PrintMemoryDescriptor (&MemoryMap[Index]);
    }
  }

  Print(L"GCD IO:\n");
  NumberOfDescriptors = 0;
  IoMap               = NULL;
  Status = gDS->GetIoSpaceMap (
                  &NumberOfDescriptors,
                  &IoMap
                  );
  if (!EFI_ERROR (Status)) {
    PrintIoDescriptorHeader ();
    for (Index = 0; Index < NumberOfDescriptors; Index++) {
      PrintIoDescriptor (&IoMap[Index]);
    }
  }

  return EFI_SUCCESS;
}
