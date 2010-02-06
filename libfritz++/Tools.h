/*
 * libfritz++
 *
 * Copyright (C) 2007-2008 Joachim Wilke <vdr@joachim-wilke.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */


#ifndef FRITZTOOLS_H_
#define FRITZTOOLS_H_

#include <iconv.h>
#include <stdexcept>
#include <string>
#include <PThread++.h>
#include "Listener.h"

#define I18N_NOOP(x) x

namespace fritz{

typedef unsigned char uchar;

// When handling strings that might contain UTF-8 characters, it may be necessary
// to process a "symbol" that consists of several actual character bytes. The
// following functions allow transparently accessing a "char *" string without
// having to worry about what character set is actually used.

int Utf8CharLen(const char *s);
    ///< Returns the number of character bytes at the beginning of the given
    ///< string that form a UTF-8 symbol.
uint Utf8CharGet(const char *s, int Length = 0);
    ///< Returns the UTF-8 symbol at the beginning of the given string.
    ///< Length can be given from a previous call to Utf8CharLen() to avoid calculating
    ///< it again. If no Length is given, Utf8CharLen() will be called.
int Utf8CharSet(uint c, char *s = NULL);
    ///< Converts the given UTF-8 symbol to a sequence of character bytes and copies
    ///< them to the given string. Returns the number of bytes written. If no string
    ///< is given, only the number of bytes is returned and nothing is copied.
int Utf8SymChars(const char *s, int Symbols);
    ///< Returns the number of character bytes at the beginning of the given
    ///< string that form at most the given number of UTF-8 symbols.
int Utf8StrLen(const char *s);
    ///< Returns the number of UTF-8 symbols formed by the given string of
    ///< character bytes.
char *Utf8Strn0Cpy(char *Dest, const char *Src, int n);
    ///< Copies at most n character bytes from Src to Dst, making sure that the
    ///< resulting copy ends with a complete UTF-8 symbol. The copy is guaranteed
    ///< to be zero terminated.
    ///< Returns a pointer to Dest.
int Utf8ToArray(const char *s, uint *a, int Size);
    ///< Converts the given character bytes (including the terminating 0) into an
    ///< array of UTF-8 symbols of the given Size. Returns the number of symbols
    ///< in the array (without the terminating 0).
int Utf8FromArray(const uint *a, char *s, int Size, int Max = -1);
    ///< Converts the given array of UTF-8 symbols (including the terminating 0)
    ///< into a sequence of character bytes of at most Size length. Returns the
    ///< number of character bytes written (without the terminating 0).
    ///< If Max is given, only that many symbols will be converted.
    ///< The resulting string is always zero-terminated if Size is big enough.

// When allocating buffer space, make sure we reserve enough space to hold
// a string in UTF-8 representation:

#define Utf8BufSize(s) ((s) * 4)

// The following macros automatically use the correct versions of the character
// class functions:

#define Utf8to(conv, c) (cCharSetConv::SystemCharacterTable() ? to##conv(c) : tow##conv(c))
#define Utf8is(ccls, c) (cCharSetConv::SystemCharacterTable() ? is##ccls(c) : isw##ccls(c))

class CharSetConv {
private:
  iconv_t cd;
  char *result;
  size_t length;
  static char *systemCharacterTable;
public:
  CharSetConv(const char *FromCode = NULL, const char *ToCode = NULL);
     ///< Sets up a character set converter to convert from FromCode to ToCode.
     ///< If FromCode is NULL, the previously set systemCharacterTable is used.
     ///< If ToCode is NULL, "UTF-8" is used.
  ~CharSetConv();
  const char *Convert(const char *From, char *To = NULL, size_t ToLength = 0);
     ///< Converts the given Text from FromCode to ToCode (as set in the constructor).
     ///< If To is given, it is used to copy at most ToLength bytes of the result
     ///< (including the terminating 0) into that buffer. If To is not given,
     ///< the result is copied into a dynamically allocated buffer and is valid as
     ///< long as this object lives, or until the next call to Convert(). The
     ///< return value always points to the result if the conversion was successful
     ///< (even if a fixed size To buffer was given and the result didn't fit into
     ///< it). If the string could not be converted, the result points to the
     ///< original From string.
  static void DetectCharset();
  static const char *SystemCharacterTable(void) { return systemCharacterTable; }
  static void SetSystemCharacterTable(const char *CharacterTable);
  };

class Tools
{
public:
	Tools();
	virtual ~Tools();
	static bool MatchesMsnFilter(const std::string &number);
	static std::string NormalizeNumber(std::string number);
	static int CompareNormalized(std::string number1, std::string number2);
	static bool GetLocationSettings();
	static void GetSipSettings();
	static std::string Tokenize(const std::string &buffer, const char delimiter, size_t pos);
};

}

#endif /*FRITZTOOLS_H_*/
