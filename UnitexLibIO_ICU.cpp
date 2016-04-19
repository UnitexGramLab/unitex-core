/*
 * Unitex
 *
 * Copyright (C) 2001-2016 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
 *
 */

/*
 * File created and contributed by Gilles Vollant (Ergonotics SAS) 
 * as part of an UNITEX optimization and reliability effort
 *
 * additional information: http://www.ergonotics.com/unitex-contribution/
 * https://github.com/ergonotics/JNI-for-Unitex-2.1
 * contact : unitex-contribution@ergonotics.com
 *
 */


#include "UnitexLibIO_ICU.h"
#include <string.h>
#include <stdlib.h>


// uima c++ users can replace UnicodeString by UnicodeStringRef 
/**
 * Write an Unitex file content (to system filesystem or filespace)
 * it write from two buffer (prefix and suffix). This is useful for writing both header and footer (or BOM and text...)
 */
  UNITEX_FUNC int UNITEX_CALL WriteUnicodeUnitexFile(const char*filename, icu::UnicodeString const& uString)
  {    
    UChar uBom = 0xfeff;

    const UChar * uBuffer = uString.getBuffer();
    int32_t uLength = uString.length();

    bool result = WriteUnitexFile(filename, &uBom, sizeof(UChar), uBuffer, uLength * sizeof(UChar)) == 0;

    return result;
  }
 

#define GetUtf8_Size(ch)  \
  (((((unsigned char)(ch)) & ((unsigned char)0x80))==((unsigned char)0x00)) ? 1 : \
  (((((unsigned char)(ch)) & ((unsigned char)0xe0))==((unsigned char)0xc0)) ? 2 : \
  (((((unsigned char)(ch)) & ((unsigned char)0xf0))==((unsigned char)0xe0)) ? 3 : \
  (((((unsigned char)(ch)) & ((unsigned char)0xf8))==((unsigned char)0xf0)) ? 4 : \
  (((((unsigned char)(ch)) & ((unsigned char)0xfc))==((unsigned char)0xf8)) ? 5 : \
  (((((unsigned char)(ch)) & ((unsigned char)0xfe))==((unsigned char)0xfc)) ? 6 : 001))))))


#define GetUtf8_Mask(ch)  \
  (((((unsigned char)(ch)) & ((unsigned char)0x80))==((unsigned char)0x00)) ? ((unsigned char)0x7f) : \
  (((((unsigned char)(ch)) & ((unsigned char)0xe0))==((unsigned char)0xc0)) ? ((unsigned char)0x1f) : \
  (((((unsigned char)(ch)) & ((unsigned char)0xf0))==((unsigned char)0xe0)) ? ((unsigned char)0x0f) : \
  (((((unsigned char)(ch)) & ((unsigned char)0xf8))==((unsigned char)0xf0)) ? ((unsigned char)0x07) : \
  (((((unsigned char)(ch)) & ((unsigned char)0xfc))==((unsigned char)0xf8)) ? ((unsigned char)0x03) : \
  (((((unsigned char)(ch)) & ((unsigned char)0xfe))==((unsigned char)0xfc)) ? ((unsigned char)0x01) : 0))))))

  static size_t unpack_utf8_string(UChar*write_content_walk_buf,size_t nb_unichar_alloc_walk,size_t * p_size_this_string_written,
    const unsigned char*src_walk,size_t buf_size)
  {
    size_t size_this_string_written=0;
    size_t nb_pack_read=0;
    for (;;)
    {
      if ((src_walk==NULL) || (buf_size==0))
      {        
        if (p_size_this_string_written!=NULL)
          *p_size_this_string_written = size_this_string_written;
        return 0;
      }
      unsigned char ch = *(src_walk++);
      buf_size--;
      nb_pack_read++;

      UChar c;



      if ((ch&0x80) == 0)
      {
        c=ch;
      }
      else
      {
        c=ch & GetUtf8_Mask(ch);
        int nbbyte=GetUtf8_Size(ch);
        if (((int)buf_size)+1 < nbbyte)
        {
          if (p_size_this_string_written!=NULL)
            *p_size_this_string_written = size_this_string_written;
          return 0;
        }

        for(;;)
        {
          nbbyte--;
          if (nbbyte==0)
            break;

          c = (c<<6) | ( (*(src_walk++)) & 0x3F);
          buf_size--;
          nb_pack_read++;
        }
      }

      if ((write_content_walk_buf!=NULL) && (size_this_string_written<nb_unichar_alloc_walk))
        *(write_content_walk_buf + size_this_string_written)=c;
      size_this_string_written++;

      if (c==0)
      {
        if (p_size_this_string_written!=NULL)
          *p_size_this_string_written = size_this_string_written;
        return nb_pack_read;
      }
    }
  }

  /// <summary>
  /// Gets the whole contents of a (virtual) file into a Unicode string.
  /// </summary>
  /// <param name='fileName'>The file name.</param>
  /// <param name='uString'>The Unicode string where to store the file contents.</param>
  /// <returns>1 if ok, 0 if failed.</returns>
  UNITEX_FUNC int UNITEX_CALL GetUnicodeStringFromUnitexFile(const char* filename, UnicodeString& uString)
  {
    uString.remove();

    UNITEXFILEMAPPED* pFileHandle;
    const void* buffer = NULL;
    size_t bufferSize = 0;
    GetUnitexFileReadBuffer(filename, &pFileHandle, &buffer, &bufferSize);

    if (pFileHandle != NULL) {
      if (bufferSize > 0) {
        const unsigned char* bufchar= (const unsigned char*) buffer;
        size_t size_bom = 0;
        bool is_utf16_native_endianess = false;
        bool is_utf16_swap_endianess = false;

        if (bufferSize > 1) {
          UChar UTF16Bom = *((const UChar*)buffer);

          if (UTF16Bom == 0xfeff)
          {
            // native endian
            is_utf16_native_endianess = true;
            size_bom = 2;
          }

          if (UTF16Bom == 0xfffe)
          {
            // reverse endian
            is_utf16_swap_endianess = true;
            size_bom = 2;
          }
        }


        if (bufferSize > 2) {
          if (((*(bufchar)) == 0xef) && ((*(bufchar + 1)) == 0xbb) && ((*(bufchar + 2)) == 0xbf))
          {
            size_bom = 3;
          }
        }

        if (is_utf16_native_endianess)
        {
          const UChar* uBuffer = (const UChar*)(bufchar + size_bom);
          size_t uSize = (bufferSize - size_bom) / U_SIZEOF_UCHAR;
          uString.setTo(uBuffer, uSize);
        }
        else if (is_utf16_swap_endianess)
        {
          unsigned char* returnedUTF16buffer = new unsigned char [bufferSize];
          if (returnedUTF16buffer != NULL)
          {
            for (size_t i = 0; i<bufferSize; i += 2)
            {
              unsigned char c1 = *(bufchar + i);
              unsigned char c2 = *(bufchar + i + 1);
              *(returnedUTF16buffer + i) = c2;
              *(returnedUTF16buffer + i + 1) = c1;
            }
            const UChar* uBuffer = (const UChar*)(returnedUTF16buffer + size_bom);
            size_t uSize = (bufferSize - size_bom) / U_SIZEOF_UCHAR;
            uString.setTo(uBuffer, uSize);
            delete [] returnedUTF16buffer;
          }
        }
        else
        {
          size_t len_buf_UChar = bufferSize+1+1;
          UChar* stringUChar = new UChar[len_buf_UChar + 1];
        

          size_t nb_written = 0;
          unpack_utf8_string(stringUChar,len_buf_UChar,&nb_written,bufchar + size_bom, bufferSize - size_bom);
    

          uString.setTo((const UChar*)stringUChar, nb_written);
          delete [] stringUChar;
        }
      }
      CloseUnitexFileReadBuffer(pFileHandle, buffer, bufferSize);
      return 1;
    }
    
    return 0;
  }



