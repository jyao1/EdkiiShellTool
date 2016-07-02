/** @file
  Support file for string handling.

  Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials                          
  are licensed and made available under the terms and conditions of the BSD License         
  which accompanies this distribution.  The full text of the license may be found at        
  http://opensource.org/licenses/bsd-license.php                                            

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

**/

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>

#define ASCII_IS_HYPHEN(a)               ((a) == '-')
#define ASCII_IS_NULL(a)                 ((a) == '\0')

/**
  Convert a unicode char from lower case to upper case,
  if it is in [a, z].

  @param Chr The unicode char

  @return The uppper case of unicode char, if it is in [a, z].
  @return The original char, if it is not in [a, z]
**/
STATIC
CHAR16
UnicodeToUpper (
  IN      CHAR16                    Chr
  )
{
  return (Chr >= L'a' && Chr <= L'z') ? Chr - (L'a' - L'A') : Chr;
}

/**
  Compare the Unicode string pointed by String to the string pointed by String2.

  @param String   String to process
  @param String2  The other string to process

  @return a positive integer if String is lexicall greater than String2; Zero if 
  the two strings are identical; and a negative interger if String is lexically 
  less than String2.
**/
STATIC
INTN
StriCmp (
  IN CHAR16   *String,
  IN CHAR16   *String2
  )
{
  while ((*String != L'\0') &&
         (UnicodeToUpper (*String) == UnicodeToUpper (*String2))) {
    String++;
    String2++;
  }

  return UnicodeToUpper (*String) - UnicodeToUpper (*String2);
}

/**
  Verify if the string is end with the sub string.
  This function uses case-insensitive comparison.

  @param Str    The full string.
  @param SubStr The sub string.

  @retval TRUE  The string is end with the sub string.
  @retval FALSE The string is not end with the sub string.
**/
BOOLEAN
StrEndWith (
  IN CHAR16                       *Str,
  IN CHAR16                       *SubStr
  )
{
  CHAR16  *Temp;

  if ((Str == NULL) || (SubStr == NULL) || (StrLen(Str) < StrLen(SubStr))) {
    return FALSE;
  }

  Temp = Str + StrLen(Str) - StrLen(SubStr);

  //
  // Compare
  //
  if (StriCmp (Temp, SubStr) == 0) {
    return TRUE;
  } else {
    return FALSE;
  }
}

/**
  Converts a list of string to a specified buffer.

  @param Buf             The output buffer that contains the string.
  @param BufferLength    The length of the buffer
  @param Str             The input string that contains the hex number

  @retval EFI_SUCCESS    The string was successfully converted to the buffer.

**/
EFI_STATUS
AsciiStrToBuf (
  OUT UINT8    *Buf,
  IN  UINTN    BufferLength,
  IN  CHAR8    *Str
  )
{
  UINTN       Index;
  UINTN       StrLength;
  UINT8       Digit;
  UINT8       Byte;

  Digit = 0;

  //
  // Two hex char make up one byte
  //
  StrLength = BufferLength * 2;

  for(Index = 0; Index < StrLength; Index++, Str++) {

    if ((*Str >= 'a') && (*Str <= 'f')) {
      Digit = (UINT8) (*Str - 'a' + 0x0A);
    } else if ((*Str >= 'A') && (*Str <= 'F')) {
      Digit = (UINT8) (*Str - 'A' + 0x0A);
    } else if ((*Str >= '0') && (*Str <= '9')) {
      Digit = (UINT8) (*Str - '0');
    } else {
      return EFI_INVALID_PARAMETER;
    }

    //
    // For odd characters, write the upper nibble for each buffer byte,
    // and for even characters, the lower nibble.
    //
    if ((Index & 1) == 0) {
      Byte = (UINT8) (Digit << 4);
    } else {
      Byte = Buf[Index / 2];
      Byte &= 0xF0;
      Byte = (UINT8) (Byte | Digit);
    }

    Buf[Index / 2] = Byte;
  }

  return EFI_SUCCESS;
}

/**
  Converts a string to GUID value.
  Guid Format is xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx

  @param Str              The registry format GUID string that contains the GUID value.
  @param Guid             A pointer to the converted GUID value.

  @retval EFI_SUCCESS     The GUID string was successfully converted to the GUID value.
  @retval EFI_UNSUPPORTED The input string is not in registry format.
  @return others          Some error occurred when converting part of GUID value.

**/
EFI_STATUS
AsciiStrToGuid (
  IN  CHAR8    *Str,
  OUT EFI_GUID *Guid
  )
{
  //
  // Get the first UINT32 data
  //
  Guid->Data1 = (UINT32) AsciiStrHexToUint64  (Str);
  while (!ASCII_IS_HYPHEN (*Str) && !ASCII_IS_NULL (*Str)) {
    Str ++;
  }
  
  if (ASCII_IS_HYPHEN (*Str)) {
    Str++;
  } else {
    return EFI_UNSUPPORTED;
  }
  
  //
  // Get the second UINT16 data
  //
  Guid->Data2 = (UINT16) AsciiStrHexToUint64  (Str);
  while (!ASCII_IS_HYPHEN (*Str) && !ASCII_IS_NULL (*Str)) {
    Str ++;
  }

  if (ASCII_IS_HYPHEN (*Str)) {
    Str++;
  } else {
    return EFI_UNSUPPORTED;
  }
  
  //
  // Get the third UINT16 data
  //
  Guid->Data3 = (UINT16) AsciiStrHexToUint64  (Str);
  while (!ASCII_IS_HYPHEN (*Str) && !ASCII_IS_NULL (*Str)) {
    Str ++;
  }

  if (ASCII_IS_HYPHEN (*Str)) {
    Str++;
  } else {
    return EFI_UNSUPPORTED;
  }

  //
  // Get the following 8 bytes data
  //  
  AsciiStrToBuf (&Guid->Data4[0], 2, Str);
  //
  // Skip 2 byte hex chars
  //
  Str += 2 * 2;

  if (ASCII_IS_HYPHEN (*Str)) {
    Str++;
  } else {
    return EFI_UNSUPPORTED;
  }
  AsciiStrToBuf (&Guid->Data4[2], 6, Str);

  return EFI_SUCCESS;
}

CHAR8  *mAsciiLineBuffer          = NULL;
CHAR8  *mAsciiFieldBuffer         = NULL;

/**
  Find the first substring.

  @param String    The string to be tokenized.
  @param CharSet   A set of char to be used as delimiters.

  @return the index of char which is not in delimiters.
**/
UINTN
AsciiStrSpn (
  IN CHAR8                       *String,
  IN CHAR8                       *CharSet
  )
{
  UINTN   Count;
  CHAR8  *Str1;
  CHAR8  *Str2;

  Count = 0;

  for (Str1 = String; *Str1 != '\0'; Str1 ++) {
    for (Str2 = CharSet; *Str2 != '\0'; Str2 ++) {
      if (*Str1 == *Str2) {
        break;
      }
    }

    if (*Str2 == '\0') {
      return Count;
    }

    Count ++;
  }

  return Count;
}

/**
  Searches a string for the first occurrence of a character contained in a
  specified buffer.

  @param String    The string to be tokenized.
  @param CharSet   A set of char to be used as delimiters.

  @return the first occurrence of a char in delimiters.
**/
CHAR8 *
AsciiStrBrk (
  IN CHAR8                       *String,
  IN CHAR8                       *CharSet
  )
{
  CHAR8  *Str1;
  CHAR8  *Str2;

  for (Str1 = String; *Str1 != '\0'; Str1 ++) {
    for (Str2 = CharSet; *Str2 != '\0'; Str2 ++) {
      if (*Str1 == *Str2) {
        return (CHAR8 *) Str1;
      }
    }
  }

  return NULL;
}

/**
  Find the next token after one or more specified characters.

  NOTE: This function is used to parse a line of buffer.

  @param String    The string to be tokenized.
  @param CharSet   A set of char to be used as delimiters.

  @return the next token in the string.
**/
CHAR8 *
AsciiStrTokenLine (
  IN CHAR8                       *String OPTIONAL,
  IN CHAR8                       *CharSet
  )
{
  CHAR8  *Begin;
  CHAR8  *End;

  Begin = (String == NULL) ? mAsciiLineBuffer : String;
  if (Begin == NULL) {
    return NULL;
  }

  Begin += AsciiStrSpn (Begin, CharSet);
  if (*Begin == '\0') {
    mAsciiLineBuffer = NULL;
    return NULL;
  }

  End = AsciiStrBrk (Begin, CharSet);
  if ((End != NULL) && (*End != '\0')) {
    *End = '\0';
    End ++;
  }

  mAsciiLineBuffer = End;
  return Begin;
}

/**
  Find the next token after one specificed characters.

  NOTE: This function is used to parse a field in a line of buffer.

  @param String    The string to be tokenized.
  @param CharSet   A set of char to be used as delimiters.

  @return the next token in the string.
**/
CHAR8 *
AsciiStrTokenField (
  IN CHAR8                       *String OPTIONAL,
  IN CHAR8                       *CharSet
  )
{
  CHAR8  *Begin;
  CHAR8  *End;


  Begin = (String == NULL) ? mAsciiFieldBuffer : String;
  if (Begin == NULL) {
    return NULL;
  }

  if (*Begin == L'\0') {
    mAsciiFieldBuffer = NULL;
    return NULL;
  }

  End = AsciiStrBrk (Begin, CharSet);
  if ((End != NULL) && (*End != '\0')) {
    *End = '\0';
    End ++;
  }

  mAsciiFieldBuffer = End;
  return Begin;
}

/**
  This function returns the new token in the input string.
  It should be followed by AsciiStrGetNextTokenLine() to get next token.

  NOTE: This function is used to parse a line of buffer.

  @param String    The string to be tokenized.
  @param CharSet   A set of char to be used as delimiters.

  @return the new token in the string.
**/
CHAR8 *
AsciiStrGetNewTokenLine (
  IN CHAR8                       *String,
  IN CHAR8                       *CharSet
  )
{
  CHAR8 *Buf;
  Buf = AsciiStrTokenLine (String, CharSet);
  while (Buf != NULL) {
    if (AsciiStrCmp (Buf, "") == 0) {
      Buf = AsciiStrTokenLine (NULL, CharSet);
      continue;
    } else {
      break;
    }
  }
  return Buf;
}

/**
  This function returns the next token in the input string.
  It should follow AsciiStrGetNewTokenLine().

  NOTE: This function is used to parse a field in a line of buffer.

  @param CharSet   A set of char to be used as delimiters.

  @return the next token in the string.
**/
CHAR8 *
AsciiStrGetNextTokenLine (
  IN CHAR8                       *CharSet
  )
{
  CHAR8 *Buf;
  Buf = AsciiStrTokenLine (NULL, CharSet);
  while (Buf != NULL) {
    if (AsciiStrCmp (Buf, "") == 0) {
      Buf = AsciiStrTokenLine (NULL, CharSet);
      continue;
    } else {
      break;
    }
  }
  return Buf;
}

/**
  This function returns the new token in the input string.
  It should be followed by AsciiStrGetNextTokenField() to get next token.

  NOTE: This function is used to parse a field in a line of buffer.

  @param String    The string to be tokenized.
  @param CharSet   A set of char to be used as delimiters.

  @return the new token in the string.
**/
CHAR8 *
AsciiStrGetNewTokenField (
  IN CHAR8                       *String,
  IN CHAR8                       *CharSet
  )
{
  CHAR8 *Buf;
  Buf = AsciiStrTokenField (String, CharSet);
  while (Buf != NULL) {
    if (AsciiStrCmp (Buf, "") == 0) {
      Buf = AsciiStrTokenField (NULL, CharSet);
      continue;
    } else {
      break;
    }
  }
  return Buf;
}

/**
  This function returns the next token in the input string.
  It should follow AsciiStrGetNewTokenField().

  NOTE: This function is used to parse a line of buffer.

  @param CharSet   A set of char to be used as delimiters.

  @return the next token in the string.
**/
CHAR8 *
AsciiStrGetNextTokenField (
  IN CHAR8                       *CharSet
  )
{
  CHAR8 *Buf;
  Buf = AsciiStrTokenField (NULL, CharSet);
  while (Buf != NULL) {
    if (AsciiStrCmp (Buf, "") == 0) {
      Buf = AsciiStrTokenField (NULL, CharSet);
      continue;
    } else {
      break;
    }
  }
  return Buf;
}
