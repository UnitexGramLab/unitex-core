 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "Unicode.h"
#include "Error.h"


/* This array is a bit array used to define characters that are letters */
char tab_is_letter[8192];


/**
 * We define here the unicode NULL character and the unicode
 * empty string.
 */
unichar U_NULL='\0';
unichar* U_EMPTY=&U_NULL;

/**
 * Here are defined the default encodings.
 */
Encoding STDIN_ENC=UTF8;
Encoding STDOUT_ENC=UTF8;
Encoding STDERR_ENC=UTF8;
Encoding FILE_ENC=UTF16_LE;


/* ------------------- Some aliases, mainly for default UTF16-LE use ------------------- */

FILE* u_fopen(char* name,char* MODE) {
return u_fopen(FILE_ENC,name,MODE);
}

int u_fempty(char* name) {
return u_fempty(FILE_ENC,name);
}

int u_fgetc_raw(FILE* f) {
return u_fgetc_raw(FILE_ENC,f);
}

int u_fgetc_UTF16LE(FILE* f) {
return u_fgetc(UTF16_LE,f);
}

int u_fgetc_UTF16BE(FILE* f) {
return u_fgetc(UTF16_BE,f);
}

int u_fgetc_UTF8(FILE* f) {
return u_fgetc(UTF8,f);
}

int u_fgetc(FILE* f) {
return u_fgetc(FILE_ENC,f);
}

int u_fgetc_CR(FILE* f) {
return u_fgetc_CR(FILE_ENC,f);
}

int u_fread_raw(unichar* t,int N,FILE* f) {
return u_fread_raw(FILE_ENC,t,N,f);
}

int u_fread(unichar* t,int N,FILE* f) {
return u_fread(FILE_ENC,t,N,f);
}

int u_fputc_raw(unichar c,FILE* f) {
return u_fputc_raw(FILE_ENC,c,f);
}

int u_fputc_UTF16LE(unichar c,FILE* f) {
return u_fputc(UTF16_LE,c,f);
}

int u_fputc_UTF16BE(unichar c,FILE* f) {
return u_fputc(UTF16_BE,c,f);
}

int u_fputc_UTF8(unichar c,FILE* f) {
return u_fputc(UTF8,c,f);
}

int u_fputc(unichar c,FILE* f) {
return u_fputc(FILE_ENC,c,f);
}

int u_ungetc(unichar c,FILE* f) {
return u_ungetc(FILE_ENC,c,f);
}

int u_fwrite_raw(unichar* t,int N,FILE* f) {
return u_fwrite_raw(FILE_ENC,t,N,f);
}

int u_fwrite(unichar* t,int N,FILE* f) {
return u_fwrite(FILE_ENC,t,N,f);
}

int u_fgets(unichar* s,FILE* f) {
return u_fgets(FILE_ENC,s,f);
}

int u_fgets(unichar* s,int size,FILE* f) {
return u_fgets(FILE_ENC,s,size,f);
}

int u_fgets2(unichar* s,FILE* f) {
return u_fgets2(FILE_ENC,s,f);
}

/**
 * By default, messages printed to the standard output are UTF8-encoded.
 */
int u_printf(char* format,...) {
va_list list;
va_start(list,format);
int n=u_vfprintf(STDOUT_ENC,stdout,format,list);
va_end(list);
return n;
}

int u_fprintf(FILE* f,char* format,...) {
va_list list;
va_start(list,format);
int n=u_vfprintf(FILE_ENC,f,format,list);
va_end(list);
return n;
}

void u_fprints(Encoding,unichar*,FILE*);
void u_fprints(unichar* s,FILE* f) {
u_fprints(FILE_ENC,s,f);
}

void u_fprints(Encoding,char*,FILE*);
void u_fprints(char* s,FILE* f) {
u_fprints(FILE_ENC,s,f);
}

int u_scanf(char* format,...) {
va_list list;
va_start(list,format);
int n=u_vfscanf(STDIN_ENC,stdin,format,list);
va_end(list);
return n;
}

int u_fscanf(FILE* f,char* format,...) {
va_list list;
va_start(list,format);
int n=u_vfscanf(FILE_ENC,f,format,list);
va_end(list);
return n;
}

/* ------------------- File functions ------------------- */

/**
 * Opens a file in binary mode for unicode I/O and returns the 
 * file in case of success; NULL otherwise. If you try to open a file
 * in READ mode, NULL will be returned if the file does not exist or
 * if it does not start with the byte order mark, in the case of a UTF16
 * file. In this last case, an error message will be printed to indicate
 * that the file is not a UTF16 one.
 * 
 * 'MODE' should be U_READ, U_WRITE or U_APPEND
 */
FILE* u_fopen(Encoding encoding,char* name,char* MODE) {
FILE* f;
if (!strcmp(MODE,U_APPEND)) {
   /* If we are in APPEND mode, we check first if the file allready exists */
   f=fopen(name,U_READ);
   if (f!=NULL) {
      /* If the file exists, we close it and reopen it in APPEND mode */
      fclose(f);
      return fopen(name,U_APPEND);
   }
   /* If the file does not exists, we are in WRITE mode */
   f=fopen(name,U_WRITE);
   if (f==NULL) return NULL;
   /* As the file is new, we must insert the byte order char if we are in
    * UTF16. */
   if (encoding==UTF16_LE) {
      u_fputc(U_BYTE_ORDER_MARK,f);
   } else if (encoding==UTF16_BE) {
      u_fputc_UTF16BE(U_BYTE_ORDER_MARK,f);
   }
   return f;
}
f=fopen(name,MODE);
int c;
if (f==NULL) return NULL;
/* If the file is opened in READ mode and if we are in UTF16,
 * we check the presence of the byte order mark. */
if (!strcmp(MODE,U_READ)) {
   if (encoding==UTF16_LE) {
      c=u_fgetc_UTF16LE(f);
      if (c!=U_BYTE_ORDER_MARK) {
         error("u_fopen error: %s is not a UTF16-LE text file\n",name);
         fclose(f);
         return NULL;
      }
      return f;
   }
   if (encoding==UTF16_BE) {
      c=u_fgetc_UTF16BE(f);
      if (c!=U_BYTE_ORDER_MARK) {
         error("u_fopen error: %s is not a UTF16-BE text file\n",name);
         fclose(f);
         return NULL;
      }
      return f;
   }
   return f;
}
/* If the file is opened in WRITE mode, we insert the 0xFEFF unicode char */
if (!strcmp(MODE,U_WRITE)) {
   u_fputc(U_BYTE_ORDER_MARK,f);
}
return f;
}


/**
 * Closes a UTF16 file.
 */
int u_fclose(FILE* f) {
return fclose(f);
}


/**
 * This function creates an empty Unicode file that just contains the
 * byte order mark. It returns 0 if it fails; 1 otherwise.
 */
int u_fempty(Encoding encoding,char* name) {
FILE* f=u_fopen(encoding,name,U_WRITE);
if (f==NULL) {
   return 0;
}
u_fclose(f);
return 1;
}


/**
 * This function tests if the given file name correspond to a UTF16 file.
 */
int u_is_UTF16(char* name) {
FILE* f=fopen(name,U_READ);
if (f==NULL) {
   /* If the file does not exist */
   return FILE_DOES_NOT_EXIST;
}
int c=u_fgetc_UTF16LE(f);
fclose(f);
if (c==U_BYTE_ORDER_MARK) {
   return UTF16_LITTLE_ENDIAN_FILE;
}
if (c==U_NOT_A_CHAR) {
   return UTF16_BIG_ENDIAN_FILE;
}
return NOT_A_UTF16_FILE;
}


/**
 * UTF16-LE version of fgetc. It does not read a char after reading 0x0D.
 * It returns EOF if it cannot read a UTF16-LE character. Moreover, it
 * prints an error message if it can read just one byte.
 */
int u_fgetc_UTF16LE_raw(FILE* f) {
int c;
unsigned char a,b;
if (!fread(&b,1,1,f)) return EOF;
if (!fread(&a,1,1,f)) {
   error("Alignment error: odd number of characters in a UTF16 file\n");
   return EOF;
}
c=a*256+b;
return c;
}


/**
 * UTF16-BE version of fgetc. It does not read a char after reading 0x0D.
 * It returns EOF if it cannot read a UTF16-BE character. Moreover, it
 * prints an error message if it can read just one byte.
 */
int u_fgetc_UTF16BE_raw(FILE* f) {
int c;
unsigned char a,b;
if (!fread(&b,1,1,f)) return EOF;
if (!fread(&a,1,1,f)) {
   error("Alignment error: odd number of characters in a UTF16 file\n");
   return EOF;
}
c=b*256+a;
return c;
}


/**
 * Reads an UTF8 encoded character from the given file and returns its
 * unicode number. Returns EOF if the end of file has been reached.
 * Prints an error and returns '?' if the end of file is found while reading a
 * compound character, or if there is an encoding error.
 * 
 * IMPORTANT: This function allows reading characters > 65536, so if
 *            it is used only for 16 bits unicode, the caller
 *            must check that the value is not greater than expected.
 */
int u_fgetc_UTF8_raw(FILE* f) {
unsigned char c;
if (!fread(&c,1,1,f)) return EOF;
if (c<=0x7F) {
   /* Case of a 1 byte character 0XXX XXXX */
   return c;
}
/* Case of a character encoded on several bytes */
int number_of_bytes;
unsigned int value;
if ((c&0xE0)==0xC0) {
   /* 2 bytes 110X XXXX*/
   value=c&31;
   number_of_bytes=2;
}
else if ((c&0xF0)==0xE0) {
   /* 3 bytes 1110X XXXX */
   value=c&15;
   number_of_bytes=3;
}
else if ((c&0xF8)==0xF0) {
   /* 4 bytes 1111 0XXX */
   value=c&7;
   number_of_bytes=4;
}
else if ((c&0xFC)==0xF8) {
   /* 5 bytes 1111 10XX */
   value=c&3;
   number_of_bytes=5;
}
else if ((c&0xFE)==0xFC) {
   /* 6 bytes 1111 110X */
   value=c&1;
   number_of_bytes=6;
}
else {
   error("Encoding error in first byte of a unicode sequence\n");
   return '?';
}
/* If there are several bytes, we read them and compute the unicode
 * number of the character */
for (int i=0;i<number_of_bytes-1;i++) {
   if (!fread(&c,1,1,f)) return EOF;
   /* Following bytes should be of the form 10XX XXXX */
   if ((c&0xC0)!=0x80) {
      error("Encoding error in byte %d of a %d byte unicode sequence\n",i+2,number_of_bytes);
      return '?';
   }
   value=(value<<6)|(c&0x3F);
}
return value;
}


/**
 * Unicode version of fgetc. It does not read a char after reading 0x0D.
 * In UTF16:
 * - It returns EOF if it cannot read a well-formed character. Moreover, it
 *   prints an error message if it can read just one byte.
 * 
 * In UTF8:
 * - It returns EOF at the end of file or '?' if it cannot read a well-formed
 *   character. In that case, it prints an error message.
 */
int u_fgetc_raw(Encoding encoding,FILE* f) {
switch(encoding) {
   case UTF16_LE: return u_fgetc_UTF16LE_raw(f);
   case UTF16_BE: return u_fgetc_UTF16BE_raw(f);
   case UTF8: return u_fgetc_UTF8_raw(f);
}
return EOF;
}
   

/**
 * Unicode version of fgetc. This function reads Windows-style end-of-lines,
 * i.e. if it reads a '\r', it skips the '\n' that is supposed to come after.
 *
 * See u_fgetc_raw for returned values.
 */
int u_fgetc(Encoding encoding,FILE *f) {
int c=u_fgetc_raw(encoding,f);
if (c==0x0D) {
   /* If we read a '\r', we try to skip the '\n' */
   if (EOF==u_fgetc_raw(encoding,f)) return EOF;
   return '\n';
}
return c;
}


/**
 * A version of u_fgetc that returns \n whatever it reads \n, \r or \r\n.
 */
int u_fgetc_CR(Encoding encoding,FILE* f) {
int c=u_fgetc_raw(encoding,f);
if (c==EOF) {
   return EOF;
}
if (c==0x0A) {
   return '\n';
}
if (c==0x0D) {
   if (encoding==UTF8) {
      /* In UTF8, we know that if we look for a \n, it will take 1 byte. This
       * is a trick in order to avoid reading a character made of several bytes
       * that we should put back to the file. */
      c=fgetc(f);
   } else {
      c=u_fgetc_raw(encoding,f);
   }
   if (c==EOF) {
      return '\n';
   }
   if (c!=0x0A) {
      /* If there is no 0x0A after 0x0D, we put back the character */
      switch(encoding) {
         case UTF16_LE:
         case UTF16_BE: {
            u_ungetc_raw(encoding,c,f);
            break;
         }
         case UTF8: {
            ungetc((char)c,f);
            break;
         }
      }
   }
   return '\n';
}
return c;
}


/**
 * Reads N characters and stores them in 't', that is supposed to be large enough.
 * Returns the number of characters read.
 * This function reads raw chars, since it does not convert \r\n into \n.
 *
 * WARNING: this function will be deprecated
 */
int u_fread_raw(Encoding encoding,unichar* t,int N,FILE* f) {
int i,c;
for (i=0;i<N;i++) {
   c=u_fgetc_raw(encoding,f);
   if (c==EOF) return i;
   t[i]=(unichar)c; 
}
return i;
}


/**
 * Reads N characters and stores them in 't', that is supposed to be large enough.
 * Returns the number of characters read. This function converts \r\n into \n.
 */
int u_fread(Encoding encoding,unichar* t,int N,FILE* f) {
int i,c;
for (i=0;i<N;i++) {
   c=u_fgetc_CR(encoding,f);
   if (c==EOF) return i;
   t[i]=(unichar)c; 
}
return i;
}


/**
 * UTF16-LE version of fputc. It does not put a 0xOA after a 0x0D.
 * Returns 1 in case of success; 0 otherwise.
 */
int u_fputc_UTF16LE_raw(unichar c,FILE *f) {
unsigned char a,b;
int ret;
a=(unsigned char)(c/256);
b=(unsigned char)(c%256);
ret=fwrite(&b,1,1,f);
ret=ret && fwrite(&a,1,1,f);
return ret;
}


/**
 * UTF16-BE version of fputc. It does not put a 0xOA after a 0x0D.
 * Returns 1 in case of success; 0 otherwise.
 */
int u_fputc_UTF16BE_raw(unichar c,FILE *f) {
unsigned char a,b;
int ret;
a=(unsigned char)(c/256);
b=(unsigned char)(c%256);
ret=fwrite(&a,1,1,f);
ret=ret && fwrite(&b,1,1,f);
return ret;
}


/**
 * A version of putc that does not prints \r\n when you want it to print a \n.
 * Returns 1 in case of success; 0 otherwise.
 */
int fputc_raw(unsigned char c,FILE *f) {
return fwrite(&c,1,1,f);
}


/**
 * This function writes a 2-bytes unicode character in the given file
 * encoding it in UTF8. It does not put a 0xOA after a 0x0D. 
 * Returns 0 if an error occurs; 1 otherwise.
 * 
 * NOTE: as it takes a unichar, this function cannot be used for writing
 *       a unicode character > 0xFFFF
 */
int u_fputc_UTF8_raw(unichar c,FILE *f) {
if (c<=0x7F) {
   return (fputc_raw(c,f)==EOF)?0:1;
}
unsigned char a;
unsigned char b;
if (c<=0x7FF) {
   a=(unsigned char) (0xC0 | (c>>6));
   b=(unsigned char) (0x80 | (c & 0x3F));
   if (fputc_raw(a,f)==EOF) return 0;
   return (fputc_raw(b,f)==EOF)?0:1;
}
unsigned char C;
a=(unsigned char) (0xE0 | (c>>12));
//b=(unsigned char) (0x80 | ((c-(c&12))>>6));   //$CD:20021119 old
b=(unsigned char) (0x80 | ((c>>6)&0x3F));       //$CD:20021119
C=(unsigned char) (0x80 | (c&0x3F));
if (fputc_raw(a,f)==EOF) return 0;
if (fputc_raw(b,f)==EOF) return 0;
if (fputc_raw(C,f)==EOF) return 0;
return 1;
}


/**
 * Unicode version of fputc. Returns 0 if an error occurs; 1 otherwise.
 */
int u_fputc_raw(Encoding encoding,unichar c,FILE* f) {
switch(encoding) {
   case UTF16_LE: return u_fputc_UTF16LE_raw(c,f);
   case UTF16_BE: return u_fputc_UTF16BE_raw(c,f);
   case UTF8: return u_fputc_UTF8_raw(c,f);
}
return 0;
}


/**
 * Unicode version of fputc that saves '\n' as '\r\n'.
 * Returns 0 if an error occurs; 1 otherwise.
 */
int u_fputc(Encoding encoding,unichar c,FILE* f) {
if (c=='\n') {
   if (!u_fputc_raw(encoding,0x0D,f)) return 0;
}
return u_fputc_raw(encoding,c,f);
}


/**
 * UTF16 version of ungetc. In fact, we just rewind 2 bytes before in 'f'.
 * At the opposite of the real ungetc, it does not push back the given 
 * character to the stream.
 * 
 * Returns 1 in case of success; 0 otherwise.
 */
int u_ungetc_UTF16_raw(unichar c,FILE *f) {
return (fseek(f,-2,SEEK_CUR)==0)?1:0;
}


/**
 * UTF16-LE version of ungetc. In fact, we just rewind 2 bytes before in 'f'.
 * At the opposite of the real ungetc, it does not push back the given 
 * character to the stream.
 * 
 * Returns 1 in case of success; 0 otherwise.
 */
int u_ungetc_UTF16LE_raw(unichar c,FILE *f) {
return u_ungetc_UTF16_raw(c,f);
}


/**
 * UTF16-BE version of ungetc. In fact, we just rewind 2 bytes before in 'f'.
 * At the opposite of the real ungetc, it does not push back the given 
 * character to the stream.
 * 
 * Returns 1 in case of success; 0 otherwise.
 */
int u_ungetc_UTF16BE_raw(unichar c,FILE *f) {
return u_ungetc_UTF16_raw(c,f);
}


/**
 * UTF8 version of ungetc. In fact, we just rewind 'n' bytes before in 'f',
 * where 'n' is the length of the UTF8 represention of 'c'.
 * At the opposite of the real ungetc, it does not push back the given 
 * character to the stream.
 * 
 * Returns 1 in case of success; 0 otherwise.
 */
int u_ungetc_UTF8_raw(unichar c,FILE *f) {
int number_of_bytes;
if (c<=0x7F) {
   /* Case of a 1 byte character 0XXX XXXX */
   number_of_bytes=-1;
}
else if (c<=0x7FF) {
   /* 2 bytes 110X XXXX*/
   number_of_bytes=-2;
}
else if (c<=0xFFFF) {
   /* 3 bytes 1110X XXXX */
   number_of_bytes=-3;
}
return (fseek(f,number_of_bytes,SEEK_CUR)==0)?1:0;
}


/**
 * Unicode version of ungetc. In fact, we just rewind 'n' bytes before in 'f',
 * where 'n' is the length of the represention of 'c' in the given encoding.
 * At the opposite of the real ungetc, it does not push back the given 
 * character to the stream.
 * 
 * Returns 1 in case of success; 0 otherwise.
 */
int u_ungetc_raw(Encoding encoding,unichar c,FILE *f) {
switch (encoding) {
   case UTF16_LE: return u_ungetc_UTF16LE_raw(c,f);
   case UTF16_BE: return u_ungetc_UTF16BE_raw(c,f);
   case UTF8: return u_ungetc_UTF8_raw(c,f);
}
return 0;
}


/**
 * Unicode version of ungetc. In fact, we just rewind 'n' bytes before in 'f',
 * where 'n' is the length of the represention of 'c' in the given encoding.
 * If 'c' is '\n', then we also ungetc a '\r'.
 * At the opposite of the real ungetc, it does not push back the given 
 * character to the stream.
 * 
 * Returns 1 in case of success; 0 otherwise.
 */
int u_ungetc(Encoding encoding,unichar c,FILE *f) {
if (c=='\n') {
   if (!u_ungetc_raw(encoding,c,f)) return 0;
   if (!u_ungetc_raw(encoding,'\r',f)) return 0;
   return 1;
}
return u_ungetc_raw(encoding,c,f);
}


/**
 * Writes N characters from t. Returns the number of characters written.
 * It does not write '\r\n' for '\n'.
 */
int u_fwrite_raw(Encoding encoding,unichar* t,int N,FILE* f) {
for (int i=0;i<N;i++) {
   if (!u_fputc_raw(encoding,t[i],f)) return i;
}
return N;
}


/**
 * Writes N characters from t. Returns the number of characters written.
 * It writes '\r\n' for '\n'.
 */
int u_fwrite(Encoding encoding,unichar* t,int N,FILE* f) {
for (int i=0;i<N;i++) {
   if (!u_fputc(encoding,t[i],f)) return i;
}
return N;
}


/**
 * Prints a char string into a file. Characters are promoted to unicode
 * and encoded according to the given encoding.
 */
void u_fprints_char(Encoding encoding,char* s,FILE* f) {
int i=0;
while (s[i]!='\0')
   u_fputc(encoding,(unichar)((unsigned char)s[i++]),f);
}


/**
 * Reads from the file 'f' until it finds the end of line '\n' or
 * the end of file. The characters read are written in 'line'. The
 * function returns EOF if the current position in the file is at the
 * end of file; otherwise, it returns the number of characters read, possibly
 * 0 if there is an empty line.
 * 
 * NOTE: there is no overflow control!
 */
int u_fgets(Encoding encoding,unichar* line,FILE* f) {
int c;
int i=0;
while ((c=u_fgetc(encoding,f))!=EOF && c!='\n') {
   line[i++]=(unichar)c;
}
if (i==0 && c==EOF) {
   /* If we are at the end of file */
   return EOF;
}
line[i]='\0';
return i;
}


/**
 * Reads a complete line or at most (size-1) unichars if the line is too long.
 * If the line is not too long, the '\n' is put before the final '\0'.
 * The function skips all '\r' and the resulting buffer is always '\0' ended.
 * It returns the length of the string:
 * - EOF : means that it was the end of file
 * - 0 : means that we have read an empty line ended by '\n'
 * - (len ==(size-1)) and (line[len-1] != '\n') : means that the line was too long for the buffer
 * - (0 < len < size-1) : means that we have read a complete line (str[len-1]=='\n', unless EOF)
 * 
 * Author: Olivier Blanc
 * Modified by Sébastien Paumier
 */

int u_fgets(Encoding encoding,unichar* line,int size,FILE* f) {
int i=0;
int c;
while ((i < (size-1)) && ((c=u_fgetc(encoding,f))!=EOF)) {
   line[i++]=(unichar)c;
   if (c=='\n') break;
}
if (i==0 && c!='\n') return EOF;
line[i]=0;
return i;
}


/**
 * This function acts exactly as 'u_fgets' does, except that 
 * it stops at an end of line if and only if it is not protected by
 * a backslash. Backslashe that are not immediately before a '\n' are taken
 * as normal characters. The function returns the length of 'line'.
 * NOTE: this is an approximation, since we cannot represent a single line
 * ended by a backslash.
 * 
 * Example:
 * 
 * abc\de\
 * ef
 * 
 * will lead to a string like: a b c \ d e \n e f
 */
int u_fgets2(Encoding encoding,unichar* line,FILE* f) {
int pos,length;
if (EOF==(pos=u_fgets(encoding,line,f))) {
   /* If we are at the end of file, then we return EOF */
   return EOF;
}
if (pos==0) {
   /* If we have read an empty line, there is nothing more to do */
   return 0;
}
length=pos;
/* Otherwise, we check if the line we have just read is ended by a backslash.
 * In that case, we add a \n to it and we read another line. */
while (line[length-1]=='\\') {
   /* We try to read another line. We try to store it at &(line[length]),
    * because, if we can read such a line, we will have to replace the
    * backslash by a \n */
   pos=u_fgets(encoding,&(line[length]),f);
   if (pos==EOF) {
      /* If we cannot read another line, we return the current length */
      return length;
   }
   /* Otherwise, we put a \n before the line we have just read, and we update the length */
   line[length-1]='\n';
   length=length+pos;
}
return length;
}


/**
 * Unicode version of fprintf. It supports all the printf format options. 
 * It also supports:
 * - %C for printing a unicode character
 * - %S for printing a unicode string
 * - %R for printing the reversed of a unicode string
 * 
 * Author: Sébastien Paumier
 * Original version with format option restrictions: Olivier Blanc
 */
int u_vfprintf(Encoding encoding,FILE* f,char* format,va_list list) {
int n_printed=0;
int i;
double d;
char c;
void* p;
unichar uc;
char* s;
unichar* us;
while (*format) {
   if (*format=='%') {
      /* If we have a special sequence introduced by '%' */
      format++;
      switch (*format) {
         /* If we have %% we must print a '%' */
         case '%': u_fputc(encoding,'%',f); n_printed++; break;
         
         /* If we have %c or %C we must print an unicode character */
         case 'c': /* We intercept %c here, because of the \n that always
                      must be encoded as \r\n */
         case 'C': {
            uc=(unichar)va_arg(list,int);
            u_fputc(encoding,uc,f);
            n_printed++; 
            break;
         }
         
         case 'H': {
            /* If we have a '%H', it means that we have to print HTML things */
            format++;
            if (*format=='S') {
               /* If we have to print a HTML string */
               us=va_arg(list,unichar*);
               if (us==NULL) {
                  u_fprints(encoding,"(null)",f);
                  n_printed=n_printed+6; 
               } else {
                  unichar html[4096];
                  int l=htmlize(us,html);
                  u_fprints(encoding,html,f);
                  n_printed=n_printed+l;
               }
            } else if (*format=='R') {
               /* If we have to print a HTML reversed string */
               us=va_arg(list,unichar*);
               if (us==NULL) {
                  u_fprints(encoding,"(null)",f);
                  n_printed=n_printed+6; 
               } else {
                  unichar reversed[4096];
                  mirror(us,reversed);
                  unichar html[4096];
                  int l=htmlize(reversed,html);
                  u_fprints(encoding,html,f);
                  n_printed=n_printed+l;
               }
            } else fatal_error("Invalid format option %%H%c\n",*format);
            break;
         }

         /* If we have %S we must print an unicode string */
         case 'S': {
            us=va_arg(list,unichar*);
            if (us==NULL) {
               u_fprints(encoding,"(null)",f);
               n_printed=n_printed+6;
            } else {
               u_fprints(encoding,us,f);
               n_printed=n_printed+u_strlen(us);
            }
            break;
         }

         /* If we have %R we must print a reversed unicode string */
         case 'R': {
            us=va_arg(list,unichar*);
            if (us==NULL) {
               /* We don't want to print ")llun(" when the string to reverse is NULL */
               u_fprints(encoding,"(null)",f);
               n_printed=n_printed+6;
               break;
            }
            unichar reversed[4096];
            n_printed=n_printed+mirror(us,reversed);
            u_fprints(encoding,reversed,f);
            break;
         }
         
         /* If we have %n, we must store the number of characters that have
          * allready been printed into the given int*. */
         case 'n': {
            int *res=va_arg(list,int*);
            *res=n_printed;
            break;
         }
         
         /* If we have '%???', we let sprintf do the job */
         default: {
            /* We get back on the '%' */
            format--;
            int z=0;
            char format2[64];
            char result[4096];
            do {
               format2[z++]=*format;
               format++;
            } while (format2[z-1]!='\0' && !strchr("diouxXeEfgcsp",format2[z-1]));
            /* We get back one character */
            format--;
            if (format2[z-1]=='\0') {
               fatal_error("Invalid format option in u_printf:\n%s\n",format2);
            }
            format2[z]='\0';
            switch (format2[z-1]) {
               case 'd': case 'i': case 'o': case 'u': case 'x': case 'X': {
                  i=va_arg(list,int);
                  n_printed=n_printed+sprintf(result,format2,i);
                  break;
               }
               case 'e':  case 'E': case 'f':  case 'g': {
                  d=va_arg(list,double);
                  n_printed=n_printed+sprintf(result,format2,d);
                  break;
               }
               case 'c': {
                  c=va_arg(list,int);
                  n_printed=n_printed+sprintf(result,format2,c);
                  break;
               }
               case 's': {
                  s=va_arg(list,char*);
                  n_printed=n_printed+sprintf(result,format2,s);
                  break;
               }
               case 'p': {
                  p=va_arg(list,void*);
                  n_printed=n_printed+sprintf(result,format2,p);
                  break;
               }
            }
            u_fprints_char(encoding,result,f);
            break;
         }
      }
   } else {
      /* If we have a normal character, we print it */
      u_fputc(encoding,(unsigned char)*format,f);
      n_printed++;
   }
   format++;
}
return n_printed;
}


/**
 * Unicode version of fprintf. See u_vfprintf for supported format options.
 * 
 * Author: Olivier Blanc
 * Modified by Sébastien Paumier
 */
int u_fprintf(Encoding encoding,FILE* f,char* format,...) {
va_list list;
va_start(list,format);
int n=u_vfprintf(encoding,f,format,list);
va_end(list);
return n;
}


/**
 * This function prints a message into a unicode string and returns the
 * size of the result. Note that if 'dest' is NULL, the function will
 * only count the size of the result without actually building it.
 * See u_fprintf for supported format options.
 * 
 * Author: Olivier Blanc
 */
int u_sprintf(unichar* dest,char* format,...) {
va_list list;
va_start(list,format);
int n=u_vsprintf(dest,format,list);
va_end(list);
return n;
}


/**
 * Unicode version of sprintf. It supports all the printf format options. 
 * It also supports:
 * - %C for printing a unicode character
 * - %S for printing a unicode string
 * - %HS for printing a unicode string in HTML (see ize)
 * - %R for printing the reversed of a unicode string
 * - %HR for printing the reversed of a unicode string in HTML (see ize)
 * 
 * Author: Sébastien Paumier
 * Original version with format option restrictions: Olivier Blanc
 */
int u_vsprintf(unichar* dest,char* format,va_list list) {
int n_printed=0;
int i;
double d;
char c;
void* p;
unichar uc;
char* s;
unichar* us;
while (*format) {
   if (*format=='%') {
      /* If we have a special sequence introduced by '%' */
      format++;
      switch (*format) {
         /* If we have %% we must print a '%' */
         case '%': if (dest) dest[n_printed]='%'; n_printed++; break;
         
         /* If we have %C we must print an unicode character */
         case 'C': {
            uc=(unichar)va_arg(list,int);
            if (dest) dest[n_printed]=uc;
            n_printed++;
            break;
         }
         
         case 'H': {
            /* If we have a '%H', it means that we have to print HTML things */
            format++;
            if (*format=='S') {
               /* If we have to print a HTML string */
               us=va_arg(list,unichar*);
               if (us==NULL) {
                  if (dest) u_strcpy(&(dest[n_printed]),"(null)");
                  n_printed=n_printed+6; 
               } else {
                  unichar html[4096];
                  int l=htmlize(us,html);
                  if (dest) u_strcpy(&(dest[n_printed]),html);
                  n_printed=n_printed+l;
               }
            } else if (*format=='R') {
               /* If we have to print a HTML reversed string */
               us=va_arg(list,unichar*);
               if (us==NULL) {
                  if (dest) u_strcpy(&(dest[n_printed]),"(null)");
                  n_printed=n_printed+6; 
               } else {
                  unichar reversed[4096];
                  mirror(us,reversed);
                  unichar html[4096];
                  int l=htmlize(reversed,html);
                  if (dest) u_strcpy(&(dest[n_printed]),html);
                  n_printed=n_printed+l;
               }
            } else fatal_error("Invalid format option %%H%c\n",*format);
            break;
         }
         
         /* If we have %S we must print an unicode string */
         case 'S': {
            us=va_arg(list,unichar*);
            if (us==NULL) {
               if (dest) u_strcpy(&(dest[n_printed]),"(null)");
               n_printed=n_printed+6; 
            } else {
               if (dest) u_strcpy(&(dest[n_printed]),us);
               n_printed=n_printed+u_strlen(us);
            }
            break;
         }

         /* If we have %R we must print a reversed unicode string */
         case 'R': {
            us=va_arg(list,unichar*);
            if (us==NULL) {
               /* We don't want to print ")llun(" when the string to reverse is NULL */
               if (dest) u_strcpy(&(dest[n_printed]),"(null)");
               n_printed=n_printed+6; 
               break;
            }
            unichar reversed[4096];
            int old=n_printed;
            n_printed=n_printed+mirror(us,reversed);
            if (dest) u_strcpy(&(dest[old]),reversed);
            break;
         }
         
         /* If we have %n, we must store the number of characters that have
          * allready been printed into the given int*. */
         case 'n': {
            int *res=va_arg(list,int*);
            *res=n_printed;
            break;
         }
         
         /* If we have '%???', we let sprintf do the job */
         default: {
            /* We get back on the '%' */
            format--;
            int z=0;
            char format2[64];
            char result[4096];
            do {
               format2[z++]=*format;
               format++;
            } while (format2[z-1]!='\0' && !strchr("diouxXeEfgcsp",format2[z-1]));
            /* We get back one character */
            format--;
            if (format2[z-1]=='\0') {
               fatal_error("Invalid format option in u_printf:\n%s\n",format2);
            }
            format2[z]='\0';
            int n_printed_old=n_printed;
            switch (format2[z-1]) {
               case 'd': case 'i': case 'o': case 'u': case 'x': case 'X': {
                  i=va_arg(list,int);
                  n_printed=n_printed+sprintf(result,format2,i);
                  break;
               }
               case 'e':  case 'E': case 'f':  case 'g': {
                  d=va_arg(list,double);
                  n_printed=n_printed+sprintf(result,format2,d);
                  break;
               }
               case 'c': {
                  c=va_arg(list,int);
                  n_printed=n_printed+sprintf(result,format2,c);
                  break;
               }
               case 's': {
                  s=va_arg(list,char*);
                  n_printed=n_printed+sprintf(result,format2,s);
                  break;
               }
               case 'p': {
                  p=va_arg(list,void*);
                  n_printed=n_printed+sprintf(result,format2,p);
                  break;
               }
            }
            if (dest) u_strcpy(&(dest[n_printed_old]),result);
            break;
         }
      }
   } else {
      /* If we have a normal character, we print it */
      if (dest) dest[n_printed]=(unsigned char)*format;
      n_printed++;
   }
   format++;
}
if (dest) dest[n_printed]='\0';
return n_printed;
}


/**
 * Returns a non-zero value if 'c' is an hexadecimal digit; 0 otherwise.
 */
int u_is_hexa_digit(unichar c) {
return (c>='0' && c<='9') || (c>='a' && c<='f') || (c>='A' && c<='F');
}


/**
 * Returns a non-zero value if 'c' is a separator; 0 otherwise.
 */
int is_separator(unichar c) {
return (c==' ') || (c=='\t') || (c=='\r') || (c=='\n');   
}


/**
 * Unicode version of fscanf. The supported format options are:
 * %c : normal character
 * %C : unicode character
 * %s : normal string
 * %S : unicode string
 * %d : decimal integer of the form 45 -45 or +45
 * %x : hexadecimal integer of the form a9 -B75 or +f8CE
 * 
 * The separators are space, tabulation '\r' and '\n'. They are skipped in
 * input stream BUT NOT IN FORMAT STRING!!! So, if we do:
 * 
 *    u_fscanf(f,"%d %s",&i,s);
 * 
 * it will match only if there is a space after the integer. For instance, you can use this
 * feature to eat EOL. If we want to an integer and the EOL sequences that follows,
 * just use:
 * 
 *    u_fscanf(f,"%d\r\n",&i);
 * 
 * Note 1: if this function is applied on stdin, it may have read one character in advance.
 * Note 2: if yout type u_fscanf(f,"%d\n",&i); the function will skip any separator that is
 *         not '\n' after the integer, so that the line " 45   \t   \n" will be entirely read.
 * 
 * Author: Sébastien Paumier
 */
int u_vfscanf(Encoding encoding,FILE* f,char* format,va_list list) {
int c;
int *i;
unichar *uc;
char *ch;
int n_variables=0;
static int stdin_ch=-1;
while (*format) {
   /* First, we get the current character */
   if (f==stdin) {
      /* If we read from the input stream, we may have to use the 1-char buffer */
      if (stdin_ch!=-1) {
         c=stdin_ch;
         stdin_ch=-1;
      } else {
         /* If we have no character in the 1-char buffer, we take one from the file */
         c=u_fgetc_raw(encoding,f);
      }
   } else {
      /* If we have to take one from the file */
      c=u_fgetc_raw(encoding,f);
   }
   if (c==EOF) {
      if (n_variables==0) {
         /* If the EOF occurs before the first conversion, we return EOF */
         return EOF;
      }
      /* Otherwise, we return the number of variables that have allready been read */
      return n_variables;
   }
   /* Then we deal with the separators. If the current format char is a separator,
    * we have two cases: */
   if (is_separator(*format)) {
      if (c==*format) {
         /* 1) the format char is the same than the input char => we go on */
         format++;
         continue;
      } else {
         /* 2) the format is for instance a '\t' and we have a current input 
          *    separator that is not a '\t' => we skip all separators that are not '\t' */
         while ((c=u_fgetc_raw(encoding,f))!=EOF && is_separator(c) && c!=*format);
         /* Subcase 1: EOF */
         if (c==EOF) return (n_variables==0)?EOF:n_variables;
         /* Subcase 2: we found the correct separator */
         if (c==*format) {
            format++;
            continue;
         }
         /* Subcase 3: we found a character that is not the expected separator => we fail */
         return n_variables;
      }
   }
   /* Now we must deal with an input separator when the current format character
    * is not a separator */
   while (c!=EOF && is_separator(c)) {
      c=u_fgetc_raw(encoding,f);
   }
   /* Again, we may have reached the EOF */
   if (c==EOF) {
      if (n_variables==0) {
         /* If the EOF occurs before the first conversion, we return EOF */
         return EOF;
      }
      /* Otherwise, we return the number of variables that have allready been read */
      return n_variables;
   }
   if (*format=='%') {
      /* If we have a special sequence introduced by '%' */
      format++;
      switch (*format) {
         /* If we have %% we must read a '%' */
         case '%': {
            if (c!='%') return n_variables;
            break;
         }
         
         /* If we have %c we must read a normal character */
         case 'c': {
            ch=va_arg(list,char*);
            *ch=(char)c;
            n_variables++;
            break;
         }

         /* If we have %C we must read a unicode character */
         case 'C': {
            uc=va_arg(list,unichar*);
            *uc=c;
            n_variables++;
            break;
         }
         
         /* If we have %s we must read a normal string */
         case 's': {
            ch=va_arg(list,char*);
            int pos=0;
            do {
               ch[pos++]=(char)c;
            } while ((c=u_fgetc_raw(encoding,f))!=EOF && !is_separator(c));
            ch[pos]='\0';
            if (c!=EOF) {
               /* If we have read a separator, we put it back in the file, for
                * the case where the user would like to read it with another read function */
               if (f==stdin) {
                  stdin_ch=c;
               }
               else {
                  u_ungetc_raw(encoding,c,f);
               }
            }
            n_variables++;
            break;
         }

         /* If we have %S we must read a unicode string */
         case 'S': {
            uc=va_arg(list,unichar*);
            int pos=0;
            do {
               uc[pos++]=c;
            } while ((c=u_fgetc_raw(encoding,f))!=EOF && !is_separator(c));
            uc[pos]='\0';
            if (c!=EOF) {
               /* If we have read a separator, we put it back in the file, for
                * the case where the user would like to read it with another read function */
               if (f==stdin) {
                  stdin_ch=c;
               }
               else {
                  u_ungetc_raw(encoding,c,f);
               }
            }
            n_variables++;
            break;
         }
         
         /* If we have %d we must read a decimal integer, eventually preceeded by '+' or '-' */
         case 'd': {
            i=va_arg(list,int*);
            int multiplier=1;
            if (c=='+' || c=='-') {
               /* If we have a sign, we must read the next character */
               if (c=='-') multiplier=-1;
               c=u_fgetc_raw(encoding,f);
               if (c==EOF || c<'0' || c>'9') {
                  /* If we have reached the EOF or if we have a non digit character */
                  return n_variables;
               }
            } else if (c<'0' || c>'9') {
               /* If we have a character that neither a sign nor a digit, we fail */
               return n_variables;
            }
            *i=0;
            do {
               *i=(*i)*10+(unichar)c-'0';
            } while ((c=u_fgetc_raw(encoding,f))!=EOF && c>='0' && c<='9');
            *i=(*i)*multiplier;
            if (c!=EOF) {
               /* If we have read a non digit, we put it back in the file, for
                * the case where the user would like to read it with another read function */
               if (f==stdin) {
                  stdin_ch=c;
               }
               else {
                  u_ungetc_raw(encoding,c,f);
               }
            }
            n_variables++;
            break;
         }

         /* If we have %x we must read an hexadecimal integer, eventually preceeded by '+' or '-' */
         case 'x': {
            i=va_arg(list,int*);
            int multiplier=1;
            if (c=='+' || c=='-') {
               /* If we have a sign, we must read the next character */
               if (c=='-') multiplier=-1;
               c=u_fgetc_raw(encoding,f);
               if (c==EOF || !u_is_hexa_digit(c)) {
                  /* If we have reached the EOF or if we have a non hexa digit character */
                  return n_variables;
               }
            } else if (!u_is_hexa_digit(c)) {
               /* If we have a character that neither a sign nor a digit, we fail */
               return n_variables;
            }
            *i=0;
            do {
               if (c>='0' && c<='9') c=c-'0';
               else if (c>='a' && c<='f') c=c-'a'+10;
               else c=c-'A'+10;
               *i=(*i)*16+c;
            } while ((c=u_fgetc_raw(encoding,f))!=EOF && u_is_hexa_digit(c));
            *i=(*i)*multiplier;
            if (c!=EOF) {
               /* If we have read a non digit, we put it back in the file, for
                * the case where the user would like to read it with another read function */
               if (f==stdin) {
                  stdin_ch=c;
               }
               else {
                  u_ungetc_raw(encoding,c,f);
               }
            }
            n_variables++;
            break;
         }
      }
   } else {
      /* If we have a normal character, we must read it */
      if (c!=*format) return n_variables;
   }
   format++;
}
return n_variables;
}


/**
 * Unicode version of fscanf. See u_vfscanf for supported format options.
 * 
 * Author: Sébastien Paumier
 */
int u_fscanf(Encoding encoding,FILE* f,char* format,...) {
va_list list;
va_start(list,format);
int n=u_vfscanf(encoding,f,format,list);
va_end(list);
return n;
}


/**
 * Unicode version of sscanf. See u_vfscanf for supported format options.
 * 
 * Author: Sébastien Paumier
 */
int u_vsscanf(unichar* s,char* format,va_list list) {
int c;
int *i;
unichar *uc;
char *ch;
int n_variables=0;
int pos=0;
while (*format) {
   /* First, we skip the separators, but only if the current format character is
    * not a separator */
   if (!is_separator(*format)) {
      while (is_separator(s[pos])) pos++;
      if (s[pos]=='\0') {
         if (n_variables==0) {
            /* If the EOF occurs before the first conversion, we return EOF */
            return EOF;
         }
         /* Otherwise, we return the number of variables that have allready been read */
         return n_variables;
      }
   } else {
      /* If we have a separator in the format string, we must look exactly for it */
      while (s[pos]!='\0' && is_separator(s[pos]) && s[pos]!=*format) pos++;
      if (s[pos]=='\0' || s[pos]!=*format) {
         /* If we have not found the wanted separator, we fail */
         return (n_variables==0)?EOF:n_variables;
      }
      format++;
      continue;
   }
   if (*format=='%') {
      /* If we have a special sequence introduced by '%' */
      format++;
      switch (*format) {
         /* If we have %% we must read a '%' */
         case '%': {
            if (s[pos]!='%') return n_variables;
            break;
         }
         
         /* If we have %c we must read a normal character */
         case 'c': {
            ch=va_arg(list,char*);
            *ch=(char)s[pos++];
            n_variables++;
            break;
         }

         /* If we have %C we must read a unicode character */
         case 'C': {
            uc=va_arg(list,unichar*);
            *uc=s[pos++];
            n_variables++;
            break;
         }
         
         /* If we have %s we must read a normal string */
         case 's': {
            ch=va_arg(list,char*);
            int pos2=0;
            do {
               ch[pos2++]=(char)s[pos++];
            } while (s[pos]!='\0' && !is_separator(s[pos]));
            ch[pos2]='\0';
            if (s[pos]!='\0') {
               /* If we have read a separator, we have nothing to do */
            }
            n_variables++;
            break;
         }

         /* If we have %S we must read a unicode string */
         case 'S': {
            uc=va_arg(list,unichar*);
            int pos2=0;
            do {
               uc[pos2++]=s[pos++];
            } while (s[pos]!='\0' && !is_separator(s[pos]));
            uc[pos2]='\0';
            if (s[pos]!='\0') {
               /* If we have read a separator, we have nothing to do */
            }
            n_variables++;
            break;
         }
         
         /* If we have %d we must read a decimal integer, eventually preceeded by '+' or '-' */
         case 'd': {
            i=va_arg(list,int*);
            int multiplier=1;
            if (s[pos]=='+' || s[pos]=='-') {
               /* If we have a sign, we must read the next character */
               if (s[pos]=='-') multiplier=-1;
               pos++;
               if (s[pos]=='\0' || s[pos]<'0' || s[pos]>'9') {
                  /* If we have reached the EOF or if we have a non digit character */
                  return n_variables;
               }
            } else if (s[pos]<'0' || s[pos]>'9') {
               /* If we have a character that neither a sign nor a digit, we fail */
               return n_variables;
            }
            *i=0;
            do {
               *i=(*i)*10+(unichar)s[pos++]-'0';
            } while (s[pos]!='\0' && s[pos]>='0' && s[pos]<='9');
            *i=(*i)*multiplier;
            if (s[pos]!='\0') {
               /* If we have read a non digit, we have nothing to do */
            }
            n_variables++;
            break;
         }

         /* If we have %x we must read an hexadecimal integer, eventually preceeded by '+' or '-' */
         case 'x': {
            i=va_arg(list,int*);
            int multiplier=1;
            if (s[pos]=='+' || s[pos]=='-') {
               /* If we have a sign, we must read the next character */
               if (s[pos]=='-') multiplier=-1;
               pos++;
               if (s[pos]=='\0' || !u_is_hexa_digit(s[pos])) {
                  /* If we have reached the EOF or if we have a non hexa digit character */
                  return n_variables;
               }
            } else if (!u_is_hexa_digit(s[pos])) {
               /* If we have a character that neither a sign nor a digit, we fail */
               return n_variables;
            }
            *i=0;
            do {
               c=s[pos++];
               if (c>='0' && c<='9') c=c-'0';
               else if (c>='a' && c<='f') c=c-'a'+10;
               else c=c-'A'+10;
               *i=(*i)*16+c;
            } while (s[pos]!='\0' && u_is_hexa_digit(s[pos]));
            *i=(*i)*multiplier;
            if (s[pos]!='\0') {
               /* If we have read a non digit, we have nothing to do */
            }
            n_variables++;
            break;
         }
      }
   } else {
      /* If we have a normal character, we must read it */
      if (s[pos++]!=*format) return n_variables;
   }
   format++;
}
return n_variables;
}


/**
 * Unicode version of sscanf. See u_vfscanf for supported format options.
 * 
 * Author: Sébastien Paumier
 */
int u_sscanf(unichar* input,char* format,...) {
va_list list;
va_start(list,format);
int n=u_vsscanf(input,format,list);
va_end(list);
return n;
}


/**
 * Prints an unicode string into a file.
 */
void u_fprints(Encoding encoding,unichar* s,FILE* f) {
int i=0;
if (s==NULL) {
   return;
}
while (s[i]!='\0') {
   u_fputc(encoding,s[i++],f);
}
}


/**
 * Prints a char string into a file.
 */
void u_fprints(Encoding encoding,char* s,FILE* f) {
int i=0;
if (s==NULL) {
   return;
}
while (s[i]!='\0') {
   u_fputc(encoding,(unichar)s[i++],f);
}
}


/* ------------------- String functions ------------------- */

/**
 * Unicode version of strlen.
 */
int u_strlen(const unichar* s) {
register int i=0;
while (s[i++]);
return (i-1);
}


/**
 * Unicode version of strcpy.
 */
unichar* u_strcpy(unichar* dest,const unichar* src) {
register unichar c;
do {
   c=*src++;
   *dest++=c;
} while (c!='\0');
return dest;
}


/**
 * Unicode version of strcpy that takes a non unicode source string.
 */
unichar* u_strcpy(unichar* dest,const char* src) {
register unichar c;
do {
   c=*src++;
   *dest++=c;
} while (c!='\0');
return dest;
}


/**
 * Unicode version of strcat.
 */
unichar* u_strcat(unichar* dest,unichar* src) {
unichar *s1=dest;
const unichar *s2=src;
register unichar c;
/* First we go at the end of the destination string */
do {
	c=*s1++;
} while (c!=(unichar)'\0');
s1-=2;
/* And we concatenate the 'src' string */
do {
	c=*s2++;
	*++s1=c;
} while (c!=(unichar)'\0');
return dest;
}


/**
 * Unicode version of strcat.
 */
unichar* u_strcat(unichar* dest,char* src) {
int i,j=0;
i=u_strlen(dest);
while ((dest[i++]=(unichar)((unsigned char)src[j++]))!=0);
return dest;
}


/**
 * Unicode version of strcmp that tolerates NULL strings.
 */
int u_strcmp(const unichar* a, const unichar* b) {
if (a==NULL) {
   if (b==NULL) return 0;
   return 1;
}
if (b==NULL) return -1;
register const unichar *a_p=a;
register const unichar *b_p=b;
register unichar a_c;
register unichar b_c;
do {
   a_c=(unichar)*a_p++;
   b_c=(unichar)*b_p++;
   if (a_c=='\0') return a_c-b_c;
} while (a_c==b_c);
return a_c-b_c;
}


/**
 * Unicode version of strcmp that tolerates NULL strings.
 */
int u_strcmp(const unichar* a, const char* b) {
if (a==NULL) {
   if (b==NULL) return 0;
   return 1;
}
if (b==NULL) return -1;
register const unichar *a_p=a;
register const unsigned char *b_p=(const unsigned char*)b;
register unichar a_c;
register unsigned char b_c;
do {
   a_c=(unichar)*a_p++;
   b_c=(unsigned char)*b_p++;
   if (a_c=='\0') return a_c-b_c;
} while (a_c==b_c);
return a_c-b_c;
}


/**
 * Unicode version of strdup.
 * This function returns an allocated string that is a copy of the given one.
 * 
 * Author: Olivier Blanc
 */
unichar* u_strdup(const unichar* str) {
if (str==NULL) return NULL;
unichar* res=(unichar*)malloc((u_strlen(str)+1)*sizeof(unichar));
if (res==NULL) {fatal_error("Not enough memory in u_strdup\n");}
for (int i=0;(res[i]=str[i])!=0;i++);
return res;
}


/**
 * Unicode version of strdup.
 * This version returns an allocated string that is a copy of the
 * n first bytes of the given one.
 * 
 * Author: Olivier Blanc
 */
unichar* u_strdup(const unichar* str,int n) {
if (str==NULL) return NULL;
if (n<=0) {fatal_error("Invalid length in u_strdup\n");}
int length=u_strlen(str);
if (length<n) {
   n=length;
}
unichar* res=(unichar*)malloc((n+1)*sizeof(unichar));
if (res==NULL) {fatal_error("Not enough memory in u_strdup\n");}
for (int i=0;i<n;i++) {
   res[i]=str[i];
}
res[n]='\0';
return res;
}


/**
 * Unicode version of strdup.
 * This function returns an allocated string that is a copy of the given one.
 * 
 * Author: Olivier Blanc
 */
unichar* u_strdup(const char* str) {
if (str==NULL) return NULL;
unichar* res=(unichar*)malloc((strlen(str)+1)*sizeof(unichar));
if (res==NULL) {fatal_error("Not enough memory in u_strdup\n");}
for (int i=0;(res[i]=str[i])!=0;i++);
return res;
}


/**
 * Unicode version of strchr.
 * This function returns a pointer on the first occurrence of 'c' in 's', or
 * NULL if not found.
 * 
 * Author: Olivier Blanc
 */
unichar* u_strchr(const unichar* s,unichar c) {
if (s==NULL) return NULL;
while (*s) {
   if (*s==c) {
      return (unichar*)s;
   }
   s++;
}
return NULL;
}


/**
 * A version of strchr that looks for a unicode character in a normal string.
 * We do this instead of calling the original strchr with a cast of 'c' to 'char',
 * because such a cast would cause invalid matches.
 * 
 * Author: Olivier Blanc
 * Modified by Sébastien Paumier
 */
char* u_strchr(char* s,unichar c) {
if (s==NULL) return NULL;
while (*s) {
   if (c==(unichar)*s) return s;
   s++;
}
return NULL;
}


/**
 * Unicode version of strpbrk.
 * This function returns a pointer on the first occurrence of any delimiter 's', or
 * NULL if not found.
 * 
 * Author: Olivier Blanc
 */
unichar* u_strpbrk(const unichar* s,unichar* delimiters) {
if (s==NULL) return NULL;
while (*s) {
   if (u_strchr(delimiters,*s)) return (unichar*)s;
   s++;
}
return NULL;
}


/**
 * Unicode version of strpbrk.
 * This function returns a pointer on the first occurrence of any delimiter 's', or
 * NULL if not found.
 * 
 * Author: Olivier Blanc
 */
unichar* u_strpbrk(const unichar* s,char* delimiters) {
if (s==NULL) return NULL;
while (*s) {
   if (u_strchr(delimiters,*s)) return (unichar*)s;
   s++;
}
return NULL;
}


/**
 * Converts the unichar* src into a char* dest.
 * dest is encoded in latin-1 (iso-8859-1) and non-convertible characters are skipped.
 * 
 * Author: Sébastien Paumier
 * Modified by Sébastian Nagel
 */
void u_to_char(char *dest,unichar *src) {
register unichar c;
do {
   c=*src++;
   if (c<=0xFF) *dest++ = (char)c;
} while (c!='\0');
}


/**
 * Puts a copy of 'src' into 'dst', replacing:
 * - multi-spaces by non-breakable spaces. Spaces at the beginning of the string
 *   are also converted, even if there is only one
 * - '< ' by "&lt;"
 * - '> ' by "&gt;"
 * - '& ' by "&amp;"
 * 
 * Note that 'dst' is supposed to be large enough.
 * The function returns the length of 'dst'.
 */
int htmlize(unichar* src,unichar* dst) {
if (src==NULL) {
   fatal_error("NULL error in ize\n");
}
int i=0;
int pos=0;
while (src[i]!='\0') {
   if (src[i]==' ') {
      /* If we find a space, we look if we are at the beginning of the line,
       * or at the end, or on the first character of a space sequence. */
      if (i==0 || src[i+1]=='\0' || src[i+1]==' ') {
         /* Here, we have to print "&nbsp;" as many times as needed */
         while (src[i]==' ') {
            u_strcpy(&dst[pos],"&nbsp;");
            pos=pos+6;
            i++;
         }
      } else {
         /* Otherwise, we put a single normal space */
         dst[pos++]=' ';
         i++;
      }
   } else if (src[i]=='<') {
      u_strcpy(&dst[pos],"&lt;");
      pos=pos+4;
      i++;
   } else if (src[i]=='>') {
      u_strcpy(&dst[pos],"&gt;");
      pos=pos+4;
      i++;
   } else if (src[i]=='&') {
      u_strcpy(&dst[pos],"&amp;");
      pos=pos+5;
      i++;
   } else {
      dst[pos++]=src[i++];
   }
}
dst[pos]='\0';
return pos;
}


/**
 * Copies the mirror of 'src' into 'dst'. Returns the length of the strings.
 */
int mirror(unichar* src,unichar* dst) {
if (src==NULL) {
   fatal_error("NULL error in mirror\n");
}
int l=u_strlen(src)-1;
int pos=0;
while (l>=0) {
   dst[pos++]=src[l--];
}
dst[pos]='\0';
return pos;
}


/**
 * Replaces 's' by its mirror and returns the length of the string,
 * or -1 if the string is NULL.
 */
int mirror(unichar* s) {
if (s==NULL) return -1;
int length=u_strlen(s);
int a=0;
int b=length-1;
unichar tmp;
while (a<b) {
   tmp=s[a];
   s[a]=s[b];
   s[b]=tmp;
   a++;
   b--;
}
return length;
}


/**
 * Returns the length of the longuest prefix common to the strings 'a' and 'b'.
 */
int get_longuest_prefix(unichar* a,unichar* b) {
if (a==NULL || b==NULL) {
   return 0;
}
int i=0;
while (a[i]==b[i] && a[i]!='\0') i++;
return i;
}


/**
 * This function returns a hash code for a unicode string.
 */
unsigned int hash_unichar(unichar* s) {
if (s==NULL) {
   return 0;
}
unsigned int code=0;
int i=0;
while (s[i]!='\0') {
   code=code*31+s[i];
   i++;
}
return code;
}



//
// unicode version of isdigit
//
int u_is_digit(unichar c) {
return (c>='0' && c<='9');
}


//
// returns true if c is a basic latin letter
//
int u_is_basic_latin_letter(unichar c) {
return ((c>='a' && c<='z') || (c>='A' && c<='Z'));
}


//
// returns true if c is a latin-1 supplement letter
//
int u_is_latin1_supplement_letter(unichar c) {
return (c>=0xC0 && c<=0xFF && c!=0xD7 && c!=0xF7);
}


//
// returns true if c is a latin extended-A letter
//
int u_is_latin_extendedA_letter(unichar c) {
return (c>=0x0100 && c<=0x017F);
}


//
// returns true if c is a latin extended-B letter
//
int u_is_latin_extendedB_letter(unichar c) {
return (c>=0x0180 && c<=0x0233 && c!=0x0220 && c!=0x221);
}


//
// returns true if c is in the IPA extensions letters
//
int u_is_IPA_extensions_letter(unichar c) {
return (c>=0x0250 && c<=0x02AD);
}


//
// returns true if c is a greek letter
//
int u_is_greek_letter(unichar c) {
return (c>=0x0386 && c<=0x03F5 && c!=0x0387 && c!=0x038B
        && c!=0x038D && c!=0x03A2 && c!=0x03CF && c!=0x03D8 && c!=0x03D9);
}

//------Beginning of Hyungue's inserts--------------

//
//	return true if c is a korean syllalbe
//
int u_is_korea_syllabe_letter(unichar c)
{
	return( (c >= 0xac00) && (c<= 0xd7af));
}
//
//	return true if c is a korean ideograme
//

int u_is_CJK_Unified_Ideographs(unichar c)
{
	return( (c>= 0x4e00) && (c <= 0x9fff));
}
int u_is_cjk_compatibility_ideographs(unichar c)
{
	return( (c>= 0xf900) && (c <= 0xfaff));	
}
//
//	return true if c is a character of the alphabet coreen
//	when charcters of this zone exit in the korean text
//	these is symbols
//
int u_is_Hangul_Compatility_Jamo(unichar c)
{
	return( (c>= 0x3130) && (c <= 0x318f));	
}
//
//	return true
//	these charcters of this zone can not existe in the korean text
//
int u_is_Hangul_Jamo(unichar c)
{
	return( (c>= 0x1100) && (c <= 0x11FF));
}
//------End of Hyungue's inserts--------------

//
// returns true if c is a cyrillic letter
//
int u_is_cyrillic_letter(unichar c) {
return (c>=0x0400 && c<=0x4F9 && (c<0x0482 || c>0x048B) && c!=0x04C5 && c!=0x04C6
        && c!=0x04C9 && c!=0x04CA && c!=0x04CD && c!=0x04CE && c!=0x04CF && c!=0x04F6
        && c!=0x04F7);
}


//
// returns true if c is an armenian letter
//
int u_is_armenian_letter(unichar c) {
return (c>=0x0531 && c<=0x587 && (c<0x0557 || c>0x0560));
}


//
// returns true if c is an hebrew letter
//
int u_is_hebrew_letter(unichar c) {
return (c>=0x05D0 && c<=0x05EA) || (c==0x05F0 || c==0x05F1 || c==0x05F2);
}


//
// returns true if c is an arabic letter
//
int u_is_arabic_letter(unichar c) {
return ((c>=0x0621 && c<=0x063A) || (c>=0x0641 && c<=0x64A)
        || (c>=0x0671 && c<=0x06D3) || c==0x06D5
        || (c>=0x06FA && c<=0x06FC));
}


//
// returns true if c is a syriac letter
//
int u_is_syriac_letter(unichar c) {
return (c>=0x0710 && c<=0x072C);
}


//
// returns true if c is a thaana letter
//
int u_is_thaana_letter(unichar c) {
return (c>=0x0780 && c<=0x07A5);
}


//
// returns true if c is a devanagari letter
//
int u_is_devanagari_letter(unichar c) {
return ((c>=0x0905 && c<=0x0939) || (c>=0x093C && c<=0x094D)
        || (c>=0x0950 && c<=0x0954) || (c>=0x0958 && c<=0x0970));
}


//
// returns true if c is a bengali letter
//
int u_is_bengali_letter(unichar c) {
return (c>=0x0985 && c<=0x09B9 && c!=0x098D && c!=0x098E
        && c!=0x0991 && c!=0x0992 && c!=0x09B1 && c!=0x09B3
        && c!=0x09B4 && c!=0x09B5) ||
       (c>=0x09BE && c<=0x09CC && c!=0x09C5 && c!=0x09C6
        && c!=0x09C9 && c!=0x09CA) ||
       (c>=0x09DC && c<=0x09E3 && c!=0x09DE) ||
       (c==0x09F0 || c==0x09F1);
}


//
// returns true if c is a gurmukhi letter
//
int u_is_gurmukhi_letter(unichar c) {
return (c>=0x0A05 && c<=0x0A0A) ||
       (c==0x0A0F || c==0x0A10) ||
       (c>=0x0A13 && c<=0x0A39 && c!=0x0A29 && c!=0x0A31
        && c!=0x0A34 && c!=0x0A37) ||
       (c>=0x0A3E && c<=0x0A42) ||
       (c==0x0A47 || c==0x0A48) ||
       (c>=0x0A4B && c<=0x0A4D) ||
       (c>=0x0A59 && c<=0x0A5E && c!=0x0A5D) ||
       (c>=0x0A70 && c<=0x0A74);
}


//
// returns true if c is a gujarati letter
//
int u_is_gujarati_letter(unichar c) {
return (c>=0x0A85 && c<=0x0ACC && c!=0x0A8C && c!=0x0A8E
        && c!=0x0A92 && c!=0x0AA9 && c!=0x0AB1 && c!=0x0AB4
        && c!=0x0ABA && c!=0x0ABB && c!=0x0AC6 && c!=0x0ACA);
}


//
// returns true if c is an oriya letter
//
int u_is_oriya_letter(unichar c) {
return (c>=0x0B05 && c<=0x0B39 && c!=0x0B0D && c!=0x0B0E
        && c!=0x0B11 && c!=0x0B12 && c!=0x0B29 && c!=0x0B31
        && c!=0x0B34 && c!=0x0B35) ||
       (c>=0x0B3E && c<=0x0B43) ||
       (c==0x0B47 || c==0x0B48 || c==0x0B4B || c==0x0B4C) ||
       (c>=0x0B5C && c<=0x0B61 && c!=0x0B5E);
}


//
// returns true if c is a tamil letter
//
int u_is_tamil_letter(unichar c) {
return (c>=0x0B85 && c<=0x0BCC && c!=0x0B8B && c!=0x0B8C
        && c!=0x0B8D && c!=0x0B91 && c!=0x0B96 && c!=0x0B97
        && c!=0x0B98 && c!=0x0B9B && c!=0x0B9D && c!=0x0BA0
        && c!=0x0BA1 && c!=0x0BA2 && c!=0x0BA5 && c!=0x0BA6
        && c!=0x0BA7 && c!=0x0BAB && c!=0x0BAC && c!=0x0BAD
        && c!=0x0BB6 && c!=0x0BBA && c!=0x0BBB && c!=0x0BBC
        && c!=0x0BBD && c!=0x0BC3 && c!=0x0BC4 && c!=0x0BC5
        && c!=0x0BC9);
}


//
// returns true if c is a telugu letter
//
int u_is_telugu_letter(unichar c) {
return (c>=0x0C05 && c<=0x0C4C && c!=0x0C0D && c!=0x0C11
        && c!=0x0C29 && c!=0x0C34 && c!=0x0C3A && c!=0x0C3B
        && c!=0x0C3C && c!=0x0C3D && c!=0x0C45 && c!=0x0C49);
}


//
// returns true if c is a kannada letter
//
int u_is_kannada_letter(unichar c) {
return (c>=0x0C85 && c<=0x0CCC && c!=0x0C8D && c!=0x0C91
        && c!=0x0CA9 && c!=0x0CB4 && c!=0x0CBA && c!=0x0CBB
        && c!=0x0CBC && c!=0x0CBD && c!=0x0CC5 && c!=0x0CC9) ||
       (c==0x0CDE || c==0x0CE0 || c==0x0CE1); 
}


//
// returns true if c is a malayalam letter
//
int u_is_malayalam_letter(unichar c) {
return (c>=0x0D05 && c<=0x0D4C && c!=0x0D0D && c!=0x0D11
        && c!=0x0D29 && c!=0x0D3A && c!=0x0D3B && c!=0x0D44
        && c!=0x0D3C && c!=0x0D3D && c!=0x0D45 && c!=0x0D49)||
       (c==0x0D60 || c==0x0D61);
}


//
// returns true if c is a sinhala letter
//
int u_is_sinhala_letter(unichar c) {
return (c>=0x0D85 && c<=0x0DC6 && c!=0x0D97 && c!=0x0D98
        && c!=0x0D99 && c!=0x0DB2 && c!=0x0DBC && c!=0x0DBE
        && c!=0x0DBF && c!=0x0D) ||
       (c>=0x0DCF && c<=0x0DDF && c!=0x0DD5 && c!=0x0DD7) ||
       (c==0x0DF2 || c==0x0DF3);
}


// returns true if c is a thai letter
//
int u_is_thai_letter(unichar c) {
return (c>=0x0E01 && c<=0x0E39 && c!=0x0E3F) ||
       (c>=0x0E40 && c<=0x0E4B);
}

// returns true if c is a greek extended letter
//																
int u_is_greek_extended_letter(unichar c) {							//$CD:20021115
return (c>=0x1F00 && c<=0x1F15) || (c>=0x1F18 && c<=0x1F1D) ||		//$CD:20021115
       (c>=0x1F20 && c<=0x1F45) || (c>=0x1F48 && c<=0x1F4D) ||		//$CD:20021115
       (c>=0x1F50 && c<=0x1F57) || 									//$CD:20021115
        c==0x1F59 || c==0x1F5B || c==0x1F5D || 						//$CD:20021115
       (c>=0x1F5F && c<=0x1F7D) || (c>=0x1F80 && c<=0x1FB4) ||		//$CD:20021115
       (c>=0x1FB6 && c<=0x1FBC) || (c>=0x1FC2 && c<=0x1FC4) ||		//$CD:20021115
       (c>=0x1FC6 && c<=0x1FCC) || (c>=0x1FD0 && c<=0x1FD3) ||		//$CD:20021115
       (c>=0x1FD6 && c<=0x1FDB) || (c>=0x1FE0 && c<=0x1FEC) ||		//$CD:20021115
       (c>=0x1FF2 && c<=0x1FF4) || (c>=0x1FF6 && c<=0x1FFC);		//$CD:20021115
}																	//$CD:20021115


//
// returns true if c is a letter in a naive way
//
int u_is_letter_internal(unichar c) {
return u_is_basic_latin_letter(c)
       || u_is_latin1_supplement_letter(c)
       || u_is_latin_extendedA_letter(c)
       || u_is_latin_extendedB_letter(c)
       || u_is_IPA_extensions_letter(c)
       || u_is_greek_letter(c)
       || u_is_cyrillic_letter(c)
       || u_is_armenian_letter(c)
       || u_is_hebrew_letter(c)
       || u_is_arabic_letter(c)
       || u_is_thaana_letter(c)
       || u_is_devanagari_letter(c)
       || u_is_bengali_letter(c)
       || u_is_gurmukhi_letter(c)
       || u_is_gujarati_letter(c)
       || u_is_oriya_letter(c)
       || u_is_tamil_letter(c)
       || u_is_telugu_letter(c)
       || u_is_kannada_letter(c)
       || u_is_malayalam_letter(c)
       || u_is_sinhala_letter(c)
       || u_is_thai_letter(c)
       || u_is_greek_extended_letter(c)	//$CD:20021115
//---------Beginning of Hyungue's inserts--------
       || u_is_korea_syllabe_letter(c)
	   || u_is_CJK_Unified_Ideographs(c)
	   || u_is_cjk_compatibility_ideographs(c)
//---------End of Hyungue's inserts--------
       ;
}


/**
 * Initializes the array : bit i = 1 if i is a letter, 0 otherwise.
 */
char init_unicode_table() {
int i;
for (i=0;i<8192;i++) tab_is_letter[i]=0;
for (i=0;i<=0xFFFF;i++) {
   if (u_is_letter_internal((unichar)i)) {
      tab_is_letter[i/8]=(char)(tab_is_letter[i/8]|(1<<(i%8)));
   }
}
return 1;
}


// this line is used to initialize automatically the unicode table
char foo=(char)(init_unicode_table()/*+make_CR()*/);


/**
 * Returns a non zero value if 'c' is a letter looking up at the unicode table;
 * 0 otherwise.
 */
int u_is_letter(unichar c) {
return (tab_is_letter[c/8]&(1<<(c%8)));
}


/**
 * Reads an integer from the string 'str'. If 'next' is not NULL,
 * it will contains a pointer to the first character that follows the
 * integer.
 * 
 * Author: Olivier Blanc
 */
int u_parse_int(unichar* str,unichar* *next) {
int res=0;
while (u_is_digit(*str)) {
   res=res*10+(*str-'0');
   str++;
}
if (next) {
   *next=str;
}
return res;
}





/* S.N.
   functions for unicode case conversion

u_tolower
u_toupper

Convert a unichar to uppercase/lowercase unichar using
the unicode case folding table:
  http://www.unicode.org/Public/UNIDATA/CaseFolding.txt

Case folding is done only for "common" and "simple" case
folding, i.e. only single characters to single characters.  
Foldings like á -> A' (A+accute) or (I -> i, I -> ï)
are excluded, see
  http://www.unicode.org/Public/UNIDATA/SpecialCasing.txt
for many examples from various languages.

To use the alphabet file is not possible, because of
multiple mappings (A -> a or à in French), see commentary
on "turn_portuguese_sequence_to_lowercase" in Alphabet.cpp

The case folding tables are implemented by a switch statement which
will be (hopefully) optimized by gcc to a table lookup or (even not
bad) a b-tree.  
Of course, it could be also hardcoded as a sparse array with first and
second 8 bits of unichar as dimensions. Because scripts are allocated
in unicode blocks this will be memory efficient.

Switch statements are autogenerated from 
  http://www.unicode.org/Public/UNIDATA/CaseFolding.txt
using this small perl script:

% cat CaseFolding.txt \
  | perl -ne 'do { next LINE if ($1 > 0xffff || $2 > 0xffff); 
                   print "    case 0x", lc($1), ": r=0x", lc($2), 
                     "; break; // ", $3, "\n" }
                if /([0-9A-F]+);\s*[CS];\s*([0-9A-F]+);\s*#\s*(.+)/' \
  > tolower_tab

resp.

% cat CaseFolding.txt \
  | perl -ne 'do { next LINE if (hex($1) > 0xffff || hex($2) > 0xffff); next if $h{$2}++; 
                   print "    case 0x", lc($2), ": r=0x", lc($1), 
                      "; break; // ", $3, "\n" }
                if /([0-9A-F]+);\s*[CS];\s*([0-9A-F]+);\s*#\s*(.+)/' \
  > toupper_tab

For the uppercase table duplicate values in the switch statement are skipped.

Since Unicode code points > 0xffff can't be used with Unitex,
they are skipped from the table.
*************************************************************/

unichar u_toupper (unichar c) {
  unichar r;
  switch (c) {
      /* begin of autogenerated code */
    case 0x0061: r=0x0041; break; // LATIN CAPITAL LETTER A
    case 0x0062: r=0x0042; break; // LATIN CAPITAL LETTER B
    case 0x0063: r=0x0043; break; // LATIN CAPITAL LETTER C
    case 0x0064: r=0x0044; break; // LATIN CAPITAL LETTER D
    case 0x0065: r=0x0045; break; // LATIN CAPITAL LETTER E
    case 0x0066: r=0x0046; break; // LATIN CAPITAL LETTER F
    case 0x0067: r=0x0047; break; // LATIN CAPITAL LETTER G
    case 0x0068: r=0x0048; break; // LATIN CAPITAL LETTER H
    case 0x0069: r=0x0049; break; // LATIN CAPITAL LETTER I
    case 0x006a: r=0x004a; break; // LATIN CAPITAL LETTER J
    case 0x006b: r=0x004b; break; // LATIN CAPITAL LETTER K
    case 0x006c: r=0x004c; break; // LATIN CAPITAL LETTER L
    case 0x006d: r=0x004d; break; // LATIN CAPITAL LETTER M
    case 0x006e: r=0x004e; break; // LATIN CAPITAL LETTER N
    case 0x006f: r=0x004f; break; // LATIN CAPITAL LETTER O
    case 0x0070: r=0x0050; break; // LATIN CAPITAL LETTER P
    case 0x0071: r=0x0051; break; // LATIN CAPITAL LETTER Q
    case 0x0072: r=0x0052; break; // LATIN CAPITAL LETTER R
    case 0x0073: r=0x0053; break; // LATIN CAPITAL LETTER S
    case 0x0074: r=0x0054; break; // LATIN CAPITAL LETTER T
    case 0x0075: r=0x0055; break; // LATIN CAPITAL LETTER U
    case 0x0076: r=0x0056; break; // LATIN CAPITAL LETTER V
    case 0x0077: r=0x0057; break; // LATIN CAPITAL LETTER W
    case 0x0078: r=0x0058; break; // LATIN CAPITAL LETTER X
    case 0x0079: r=0x0059; break; // LATIN CAPITAL LETTER Y
    case 0x007a: r=0x005a; break; // LATIN CAPITAL LETTER Z
    case 0x03bc: r=0x00b5; break; // MICRO SIGN
    case 0x00e0: r=0x00c0; break; // LATIN CAPITAL LETTER A WITH GRAVE
    case 0x00e1: r=0x00c1; break; // LATIN CAPITAL LETTER A WITH ACUTE
    case 0x00e2: r=0x00c2; break; // LATIN CAPITAL LETTER A WITH CIRCUMFLEX
    case 0x00e3: r=0x00c3; break; // LATIN CAPITAL LETTER A WITH TILDE
    case 0x00e4: r=0x00c4; break; // LATIN CAPITAL LETTER A WITH DIAERESIS
    case 0x00e5: r=0x00c5; break; // LATIN CAPITAL LETTER A WITH RING ABOVE
    case 0x00e6: r=0x00c6; break; // LATIN CAPITAL LETTER AE
    case 0x00e7: r=0x00c7; break; // LATIN CAPITAL LETTER C WITH CEDILLA
    case 0x00e8: r=0x00c8; break; // LATIN CAPITAL LETTER E WITH GRAVE
    case 0x00e9: r=0x00c9; break; // LATIN CAPITAL LETTER E WITH ACUTE
    case 0x00ea: r=0x00ca; break; // LATIN CAPITAL LETTER E WITH CIRCUMFLEX
    case 0x00eb: r=0x00cb; break; // LATIN CAPITAL LETTER E WITH DIAERESIS
    case 0x00ec: r=0x00cc; break; // LATIN CAPITAL LETTER I WITH GRAVE
    case 0x00ed: r=0x00cd; break; // LATIN CAPITAL LETTER I WITH ACUTE
    case 0x00ee: r=0x00ce; break; // LATIN CAPITAL LETTER I WITH CIRCUMFLEX
    case 0x00ef: r=0x00cf; break; // LATIN CAPITAL LETTER I WITH DIAERESIS
    case 0x00f0: r=0x00d0; break; // LATIN CAPITAL LETTER ETH
    case 0x00f1: r=0x00d1; break; // LATIN CAPITAL LETTER N WITH TILDE
    case 0x00f2: r=0x00d2; break; // LATIN CAPITAL LETTER O WITH GRAVE
    case 0x00f3: r=0x00d3; break; // LATIN CAPITAL LETTER O WITH ACUTE
    case 0x00f4: r=0x00d4; break; // LATIN CAPITAL LETTER O WITH CIRCUMFLEX
    case 0x00f5: r=0x00d5; break; // LATIN CAPITAL LETTER O WITH TILDE
    case 0x00f6: r=0x00d6; break; // LATIN CAPITAL LETTER O WITH DIAERESIS
    case 0x00f8: r=0x00d8; break; // LATIN CAPITAL LETTER O WITH STROKE
    case 0x00f9: r=0x00d9; break; // LATIN CAPITAL LETTER U WITH GRAVE
    case 0x00fa: r=0x00da; break; // LATIN CAPITAL LETTER U WITH ACUTE
    case 0x00fb: r=0x00db; break; // LATIN CAPITAL LETTER U WITH CIRCUMFLEX
    case 0x00fc: r=0x00dc; break; // LATIN CAPITAL LETTER U WITH DIAERESIS
    case 0x00fd: r=0x00dd; break; // LATIN CAPITAL LETTER Y WITH ACUTE
    case 0x00fe: r=0x00de; break; // LATIN CAPITAL LETTER THORN
    case 0x0101: r=0x0100; break; // LATIN CAPITAL LETTER A WITH MACRON
    case 0x0103: r=0x0102; break; // LATIN CAPITAL LETTER A WITH BREVE
    case 0x0105: r=0x0104; break; // LATIN CAPITAL LETTER A WITH OGONEK
    case 0x0107: r=0x0106; break; // LATIN CAPITAL LETTER C WITH ACUTE
    case 0x0109: r=0x0108; break; // LATIN CAPITAL LETTER C WITH CIRCUMFLEX
    case 0x010b: r=0x010a; break; // LATIN CAPITAL LETTER C WITH DOT ABOVE
    case 0x010d: r=0x010c; break; // LATIN CAPITAL LETTER C WITH CARON
    case 0x010f: r=0x010e; break; // LATIN CAPITAL LETTER D WITH CARON
    case 0x0111: r=0x0110; break; // LATIN CAPITAL LETTER D WITH STROKE
    case 0x0113: r=0x0112; break; // LATIN CAPITAL LETTER E WITH MACRON
    case 0x0115: r=0x0114; break; // LATIN CAPITAL LETTER E WITH BREVE
    case 0x0117: r=0x0116; break; // LATIN CAPITAL LETTER E WITH DOT ABOVE
    case 0x0119: r=0x0118; break; // LATIN CAPITAL LETTER E WITH OGONEK
    case 0x011b: r=0x011a; break; // LATIN CAPITAL LETTER E WITH CARON
    case 0x011d: r=0x011c; break; // LATIN CAPITAL LETTER G WITH CIRCUMFLEX
    case 0x011f: r=0x011e; break; // LATIN CAPITAL LETTER G WITH BREVE
    case 0x0121: r=0x0120; break; // LATIN CAPITAL LETTER G WITH DOT ABOVE
    case 0x0123: r=0x0122; break; // LATIN CAPITAL LETTER G WITH CEDILLA
    case 0x0125: r=0x0124; break; // LATIN CAPITAL LETTER H WITH CIRCUMFLEX
    case 0x0127: r=0x0126; break; // LATIN CAPITAL LETTER H WITH STROKE
    case 0x0129: r=0x0128; break; // LATIN CAPITAL LETTER I WITH TILDE
    case 0x012b: r=0x012a; break; // LATIN CAPITAL LETTER I WITH MACRON
    case 0x012d: r=0x012c; break; // LATIN CAPITAL LETTER I WITH BREVE
    case 0x012f: r=0x012e; break; // LATIN CAPITAL LETTER I WITH OGONEK
    case 0x0133: r=0x0132; break; // LATIN CAPITAL LIGATURE IJ
    case 0x0135: r=0x0134; break; // LATIN CAPITAL LETTER J WITH CIRCUMFLEX
    case 0x0137: r=0x0136; break; // LATIN CAPITAL LETTER K WITH CEDILLA
    case 0x013a: r=0x0139; break; // LATIN CAPITAL LETTER L WITH ACUTE
    case 0x013c: r=0x013b; break; // LATIN CAPITAL LETTER L WITH CEDILLA
    case 0x013e: r=0x013d; break; // LATIN CAPITAL LETTER L WITH CARON
    case 0x0140: r=0x013f; break; // LATIN CAPITAL LETTER L WITH MIDDLE DOT
    case 0x0142: r=0x0141; break; // LATIN CAPITAL LETTER L WITH STROKE
    case 0x0144: r=0x0143; break; // LATIN CAPITAL LETTER N WITH ACUTE
    case 0x0146: r=0x0145; break; // LATIN CAPITAL LETTER N WITH CEDILLA
    case 0x0148: r=0x0147; break; // LATIN CAPITAL LETTER N WITH CARON
    case 0x014b: r=0x014a; break; // LATIN CAPITAL LETTER ENG
    case 0x014d: r=0x014c; break; // LATIN CAPITAL LETTER O WITH MACRON
    case 0x014f: r=0x014e; break; // LATIN CAPITAL LETTER O WITH BREVE
    case 0x0151: r=0x0150; break; // LATIN CAPITAL LETTER O WITH DOUBLE ACUTE
    case 0x0153: r=0x0152; break; // LATIN CAPITAL LIGATURE OE
    case 0x0155: r=0x0154; break; // LATIN CAPITAL LETTER R WITH ACUTE
    case 0x0157: r=0x0156; break; // LATIN CAPITAL LETTER R WITH CEDILLA
    case 0x0159: r=0x0158; break; // LATIN CAPITAL LETTER R WITH CARON
    case 0x015b: r=0x015a; break; // LATIN CAPITAL LETTER S WITH ACUTE
    case 0x015d: r=0x015c; break; // LATIN CAPITAL LETTER S WITH CIRCUMFLEX
    case 0x015f: r=0x015e; break; // LATIN CAPITAL LETTER S WITH CEDILLA
    case 0x0161: r=0x0160; break; // LATIN CAPITAL LETTER S WITH CARON
    case 0x0163: r=0x0162; break; // LATIN CAPITAL LETTER T WITH CEDILLA
    case 0x0165: r=0x0164; break; // LATIN CAPITAL LETTER T WITH CARON
    case 0x0167: r=0x0166; break; // LATIN CAPITAL LETTER T WITH STROKE
    case 0x0169: r=0x0168; break; // LATIN CAPITAL LETTER U WITH TILDE
    case 0x016b: r=0x016a; break; // LATIN CAPITAL LETTER U WITH MACRON
    case 0x016d: r=0x016c; break; // LATIN CAPITAL LETTER U WITH BREVE
    case 0x016f: r=0x016e; break; // LATIN CAPITAL LETTER U WITH RING ABOVE
    case 0x0171: r=0x0170; break; // LATIN CAPITAL LETTER U WITH DOUBLE ACUTE
    case 0x0173: r=0x0172; break; // LATIN CAPITAL LETTER U WITH OGONEK
    case 0x0175: r=0x0174; break; // LATIN CAPITAL LETTER W WITH CIRCUMFLEX
    case 0x0177: r=0x0176; break; // LATIN CAPITAL LETTER Y WITH CIRCUMFLEX
    case 0x00ff: r=0x0178; break; // LATIN CAPITAL LETTER Y WITH DIAERESIS
    case 0x017a: r=0x0179; break; // LATIN CAPITAL LETTER Z WITH ACUTE
    case 0x017c: r=0x017b; break; // LATIN CAPITAL LETTER Z WITH DOT ABOVE
    case 0x017e: r=0x017d; break; // LATIN CAPITAL LETTER Z WITH CARON
    case 0x0253: r=0x0181; break; // LATIN CAPITAL LETTER B WITH HOOK
    case 0x0183: r=0x0182; break; // LATIN CAPITAL LETTER B WITH TOPBAR
    case 0x0185: r=0x0184; break; // LATIN CAPITAL LETTER TONE SIX
    case 0x0254: r=0x0186; break; // LATIN CAPITAL LETTER OPEN O
    case 0x0188: r=0x0187; break; // LATIN CAPITAL LETTER C WITH HOOK
    case 0x0256: r=0x0189; break; // LATIN CAPITAL LETTER AFRICAN D
    case 0x0257: r=0x018a; break; // LATIN CAPITAL LETTER D WITH HOOK
    case 0x018c: r=0x018b; break; // LATIN CAPITAL LETTER D WITH TOPBAR
    case 0x01dd: r=0x018e; break; // LATIN CAPITAL LETTER REVERSED E
    case 0x0259: r=0x018f; break; // LATIN CAPITAL LETTER SCHWA
    case 0x025b: r=0x0190; break; // LATIN CAPITAL LETTER OPEN E
    case 0x0192: r=0x0191; break; // LATIN CAPITAL LETTER F WITH HOOK
    case 0x0260: r=0x0193; break; // LATIN CAPITAL LETTER G WITH HOOK
    case 0x0263: r=0x0194; break; // LATIN CAPITAL LETTER GAMMA
    case 0x0269: r=0x0196; break; // LATIN CAPITAL LETTER IOTA
    case 0x0268: r=0x0197; break; // LATIN CAPITAL LETTER I WITH STROKE
    case 0x0199: r=0x0198; break; // LATIN CAPITAL LETTER K WITH HOOK
    case 0x026f: r=0x019c; break; // LATIN CAPITAL LETTER TURNED M
    case 0x0272: r=0x019d; break; // LATIN CAPITAL LETTER N WITH LEFT HOOK
    case 0x0275: r=0x019f; break; // LATIN CAPITAL LETTER O WITH MIDDLE TILDE
    case 0x01a1: r=0x01a0; break; // LATIN CAPITAL LETTER O WITH HORN
    case 0x01a3: r=0x01a2; break; // LATIN CAPITAL LETTER OI
    case 0x01a5: r=0x01a4; break; // LATIN CAPITAL LETTER P WITH HOOK
    case 0x0280: r=0x01a6; break; // LATIN LETTER YR
    case 0x01a8: r=0x01a7; break; // LATIN CAPITAL LETTER TONE TWO
    case 0x0283: r=0x01a9; break; // LATIN CAPITAL LETTER ESH
    case 0x01ad: r=0x01ac; break; // LATIN CAPITAL LETTER T WITH HOOK
    case 0x0288: r=0x01ae; break; // LATIN CAPITAL LETTER T WITH RETROFLEX HOOK
    case 0x01b0: r=0x01af; break; // LATIN CAPITAL LETTER U WITH HORN
    case 0x028a: r=0x01b1; break; // LATIN CAPITAL LETTER UPSILON
    case 0x028b: r=0x01b2; break; // LATIN CAPITAL LETTER V WITH HOOK
    case 0x01b4: r=0x01b3; break; // LATIN CAPITAL LETTER Y WITH HOOK
    case 0x01b6: r=0x01b5; break; // LATIN CAPITAL LETTER Z WITH STROKE
    case 0x0292: r=0x01b7; break; // LATIN CAPITAL LETTER EZH
    case 0x01b9: r=0x01b8; break; // LATIN CAPITAL LETTER EZH REVERSED
    case 0x01bd: r=0x01bc; break; // LATIN CAPITAL LETTER TONE FIVE
    case 0x01c6: r=0x01c4; break; // LATIN CAPITAL LETTER DZ WITH CARON
    case 0x01c9: r=0x01c7; break; // LATIN CAPITAL LETTER LJ
    case 0x01cc: r=0x01ca; break; // LATIN CAPITAL LETTER NJ
    case 0x01ce: r=0x01cd; break; // LATIN CAPITAL LETTER A WITH CARON
    case 0x01d0: r=0x01cf; break; // LATIN CAPITAL LETTER I WITH CARON
    case 0x01d2: r=0x01d1; break; // LATIN CAPITAL LETTER O WITH CARON
    case 0x01d4: r=0x01d3; break; // LATIN CAPITAL LETTER U WITH CARON
    case 0x01d6: r=0x01d5; break; // LATIN CAPITAL LETTER U WITH DIAERESIS AND MACRON
    case 0x01d8: r=0x01d7; break; // LATIN CAPITAL LETTER U WITH DIAERESIS AND ACUTE
    case 0x01da: r=0x01d9; break; // LATIN CAPITAL LETTER U WITH DIAERESIS AND CARON
    case 0x01dc: r=0x01db; break; // LATIN CAPITAL LETTER U WITH DIAERESIS AND GRAVE
    case 0x01df: r=0x01de; break; // LATIN CAPITAL LETTER A WITH DIAERESIS AND MACRON
    case 0x01e1: r=0x01e0; break; // LATIN CAPITAL LETTER A WITH DOT ABOVE AND MACRON
    case 0x01e3: r=0x01e2; break; // LATIN CAPITAL LETTER AE WITH MACRON
    case 0x01eb: r=0x01ea; break; // LATIN CAPITAL LETTER O WITH OGONEK
    case 0x01ed: r=0x01ec; break; // LATIN CAPITAL LETTER O WITH OGONEK AND MACRON
    case 0x01ef: r=0x01ee; break; // LATIN CAPITAL LETTER EZH WITH CARON
    case 0x01f3: r=0x01f1; break; // LATIN CAPITAL LETTER DZ
    case 0x01f5: r=0x01f4; break; // LATIN CAPITAL LETTER G WITH ACUTE
    case 0x0195: r=0x01f6; break; // LATIN CAPITAL LETTER HWAIR
    case 0x01bf: r=0x01f7; break; // LATIN CAPITAL LETTER WYNN
    case 0x01f9: r=0x01f8; break; // LATIN CAPITAL LETTER N WITH GRAVE
    case 0x01fb: r=0x01fa; break; // LATIN CAPITAL LETTER A WITH RING ABOVE AND ACUTE
    case 0x01fd: r=0x01fc; break; // LATIN CAPITAL LETTER AE WITH ACUTE
    case 0x01ff: r=0x01fe; break; // LATIN CAPITAL LETTER O WITH STROKE AND ACUTE
    case 0x0201: r=0x0200; break; // LATIN CAPITAL LETTER A WITH DOUBLE GRAVE
    case 0x0203: r=0x0202; break; // LATIN CAPITAL LETTER A WITH INVERTED BREVE
    case 0x0205: r=0x0204; break; // LATIN CAPITAL LETTER E WITH DOUBLE GRAVE
    case 0x0207: r=0x0206; break; // LATIN CAPITAL LETTER E WITH INVERTED BREVE
    case 0x0209: r=0x0208; break; // LATIN CAPITAL LETTER I WITH DOUBLE GRAVE
    case 0x020b: r=0x020a; break; // LATIN CAPITAL LETTER I WITH INVERTED BREVE
    case 0x020d: r=0x020c; break; // LATIN CAPITAL LETTER O WITH DOUBLE GRAVE
    case 0x020f: r=0x020e; break; // LATIN CAPITAL LETTER O WITH INVERTED BREVE
    case 0x0211: r=0x0210; break; // LATIN CAPITAL LETTER R WITH DOUBLE GRAVE
    case 0x0213: r=0x0212; break; // LATIN CAPITAL LETTER R WITH INVERTED BREVE
    case 0x0215: r=0x0214; break; // LATIN CAPITAL LETTER U WITH DOUBLE GRAVE
    case 0x0217: r=0x0216; break; // LATIN CAPITAL LETTER U WITH INVERTED BREVE
    case 0x0219: r=0x0218; break; // LATIN CAPITAL LETTER S WITH COMMA BELOW
    case 0x021b: r=0x021a; break; // LATIN CAPITAL LETTER T WITH COMMA BELOW
    case 0x021d: r=0x021c; break; // LATIN CAPITAL LETTER YOGH
    case 0x021f: r=0x021e; break; // LATIN CAPITAL LETTER H WITH CARON
    case 0x019e: r=0x0220; break; // LATIN CAPITAL LETTER N WITH LONG RIGHT LEG
    case 0x0223: r=0x0222; break; // LATIN CAPITAL LETTER OU
    case 0x0225: r=0x0224; break; // LATIN CAPITAL LETTER Z WITH HOOK
    case 0x0227: r=0x0226; break; // LATIN CAPITAL LETTER A WITH DOT ABOVE
    case 0x0229: r=0x0228; break; // LATIN CAPITAL LETTER E WITH CEDILLA
    case 0x022b: r=0x022a; break; // LATIN CAPITAL LETTER O WITH DIAERESIS AND MACRON
    case 0x022d: r=0x022c; break; // LATIN CAPITAL LETTER O WITH TILDE AND MACRON
    case 0x022f: r=0x022e; break; // LATIN CAPITAL LETTER O WITH DOT ABOVE
    case 0x0231: r=0x0230; break; // LATIN CAPITAL LETTER O WITH DOT ABOVE AND MACRON
    case 0x0233: r=0x0232; break; // LATIN CAPITAL LETTER Y WITH MACRON
    case 0x023c: r=0x023b; break; // LATIN CAPITAL LETTER C WITH STROKE
    case 0x019a: r=0x023d; break; // LATIN CAPITAL LETTER L WITH BAR
    case 0x0294: r=0x0241; break; // LATIN CAPITAL LETTER GLOTTAL STOP
    case 0x03b9: r=0x0345; break; // COMBINING GREEK YPOGEGRAMMENI
    case 0x03ac: r=0x0386; break; // GREEK CAPITAL LETTER ALPHA WITH TONOS
    case 0x03ad: r=0x0388; break; // GREEK CAPITAL LETTER EPSILON WITH TONOS
    case 0x03ae: r=0x0389; break; // GREEK CAPITAL LETTER ETA WITH TONOS
    case 0x03af: r=0x038a; break; // GREEK CAPITAL LETTER IOTA WITH TONOS
    case 0x03cc: r=0x038c; break; // GREEK CAPITAL LETTER OMICRON WITH TONOS
    case 0x03cd: r=0x038e; break; // GREEK CAPITAL LETTER UPSILON WITH TONOS
    case 0x03ce: r=0x038f; break; // GREEK CAPITAL LETTER OMEGA WITH TONOS
    case 0x03b1: r=0x0391; break; // GREEK CAPITAL LETTER ALPHA
    case 0x03b2: r=0x0392; break; // GREEK CAPITAL LETTER BETA
    case 0x03b3: r=0x0393; break; // GREEK CAPITAL LETTER GAMMA
    case 0x03b4: r=0x0394; break; // GREEK CAPITAL LETTER DELTA
    case 0x03b5: r=0x0395; break; // GREEK CAPITAL LETTER EPSILON
    case 0x03b6: r=0x0396; break; // GREEK CAPITAL LETTER ZETA
    case 0x03b7: r=0x0397; break; // GREEK CAPITAL LETTER ETA
    case 0x03b8: r=0x0398; break; // GREEK CAPITAL LETTER THETA
    case 0x03ba: r=0x039a; break; // GREEK CAPITAL LETTER KAPPA
    case 0x03bb: r=0x039b; break; // GREEK CAPITAL LETTER LAMDA
    case 0x03bd: r=0x039d; break; // GREEK CAPITAL LETTER NU
    case 0x03be: r=0x039e; break; // GREEK CAPITAL LETTER XI
    case 0x03bf: r=0x039f; break; // GREEK CAPITAL LETTER OMICRON
    case 0x03c0: r=0x03a0; break; // GREEK CAPITAL LETTER PI
    case 0x03c1: r=0x03a1; break; // GREEK CAPITAL LETTER RHO
    case 0x03c3: r=0x03a3; break; // GREEK CAPITAL LETTER SIGMA
    case 0x03c4: r=0x03a4; break; // GREEK CAPITAL LETTER TAU
    case 0x03c5: r=0x03a5; break; // GREEK CAPITAL LETTER UPSILON
    case 0x03c6: r=0x03a6; break; // GREEK CAPITAL LETTER PHI
    case 0x03c7: r=0x03a7; break; // GREEK CAPITAL LETTER CHI
    case 0x03c8: r=0x03a8; break; // GREEK CAPITAL LETTER PSI
    case 0x03c9: r=0x03a9; break; // GREEK CAPITAL LETTER OMEGA
    case 0x03ca: r=0x03aa; break; // GREEK CAPITAL LETTER IOTA WITH DIALYTIKA
    case 0x03cb: r=0x03ab; break; // GREEK CAPITAL LETTER UPSILON WITH DIALYTIKA
    case 0x03d9: r=0x03d8; break; // GREEK LETTER ARCHAIC KOPPA
    case 0x03db: r=0x03da; break; // GREEK LETTER STIGMA
    case 0x03dd: r=0x03dc; break; // GREEK LETTER DIGAMMA
    case 0x03df: r=0x03de; break; // GREEK LETTER KOPPA
    case 0x03e1: r=0x03e0; break; // GREEK LETTER SAMPI
    case 0x03e3: r=0x03e2; break; // COPTIC CAPITAL LETTER SHEI
    case 0x03eb: r=0x03ea; break; // COPTIC CAPITAL LETTER GANGIA
    case 0x03ed: r=0x03ec; break; // COPTIC CAPITAL LETTER SHIMA
    case 0x03ef: r=0x03ee; break; // COPTIC CAPITAL LETTER DEI
    case 0x03f8: r=0x03f7; break; // GREEK CAPITAL LETTER SHO
    case 0x03f2: r=0x03f9; break; // GREEK CAPITAL LUNATE SIGMA SYMBOL
    case 0x03fb: r=0x03fa; break; // GREEK CAPITAL LETTER SAN
    case 0x0450: r=0x0400; break; // CYRILLIC CAPITAL LETTER IE WITH GRAVE
    case 0x0451: r=0x0401; break; // CYRILLIC CAPITAL LETTER IO
    case 0x0452: r=0x0402; break; // CYRILLIC CAPITAL LETTER DJE
    case 0x0453: r=0x0403; break; // CYRILLIC CAPITAL LETTER GJE
    case 0x0454: r=0x0404; break; // CYRILLIC CAPITAL LETTER UKRAINIAN IE
    case 0x0455: r=0x0405; break; // CYRILLIC CAPITAL LETTER DZE
    case 0x0456: r=0x0406; break; // CYRILLIC CAPITAL LETTER BYELORUSSIAN-UKRAINIAN I
    case 0x0457: r=0x0407; break; // CYRILLIC CAPITAL LETTER YI
    case 0x0458: r=0x0408; break; // CYRILLIC CAPITAL LETTER JE
    case 0x0459: r=0x0409; break; // CYRILLIC CAPITAL LETTER LJE
    case 0x045a: r=0x040a; break; // CYRILLIC CAPITAL LETTER NJE
    case 0x045b: r=0x040b; break; // CYRILLIC CAPITAL LETTER TSHE
    case 0x045c: r=0x040c; break; // CYRILLIC CAPITAL LETTER KJE
    case 0x045d: r=0x040d; break; // CYRILLIC CAPITAL LETTER I WITH GRAVE
    case 0x045e: r=0x040e; break; // CYRILLIC CAPITAL LETTER SHORT U
    case 0x045f: r=0x040f; break; // CYRILLIC CAPITAL LETTER DZHE
    case 0x0430: r=0x0410; break; // CYRILLIC CAPITAL LETTER A
    case 0x0431: r=0x0411; break; // CYRILLIC CAPITAL LETTER BE
    case 0x0432: r=0x0412; break; // CYRILLIC CAPITAL LETTER VE
    case 0x0433: r=0x0413; break; // CYRILLIC CAPITAL LETTER GHE
    case 0x0434: r=0x0414; break; // CYRILLIC CAPITAL LETTER DE
    case 0x0435: r=0x0415; break; // CYRILLIC CAPITAL LETTER IE
    case 0x0436: r=0x0416; break; // CYRILLIC CAPITAL LETTER ZHE
    case 0x0437: r=0x0417; break; // CYRILLIC CAPITAL LETTER ZE
    case 0x0438: r=0x0418; break; // CYRILLIC CAPITAL LETTER I
    case 0x0439: r=0x0419; break; // CYRILLIC CAPITAL LETTER SHORT I
    case 0x043a: r=0x041a; break; // CYRILLIC CAPITAL LETTER KA
    case 0x043b: r=0x041b; break; // CYRILLIC CAPITAL LETTER EL
    case 0x043c: r=0x041c; break; // CYRILLIC CAPITAL LETTER EM
    case 0x043d: r=0x041d; break; // CYRILLIC CAPITAL LETTER EN
    case 0x043e: r=0x041e; break; // CYRILLIC CAPITAL LETTER O
    case 0x043f: r=0x041f; break; // CYRILLIC CAPITAL LETTER PE
    case 0x0440: r=0x0420; break; // CYRILLIC CAPITAL LETTER ER
    case 0x0441: r=0x0421; break; // CYRILLIC CAPITAL LETTER ES
    case 0x0442: r=0x0422; break; // CYRILLIC CAPITAL LETTER TE
    case 0x0443: r=0x0423; break; // CYRILLIC CAPITAL LETTER U
    case 0x0444: r=0x0424; break; // CYRILLIC CAPITAL LETTER EF
    case 0x0445: r=0x0425; break; // CYRILLIC CAPITAL LETTER HA
    case 0x0446: r=0x0426; break; // CYRILLIC CAPITAL LETTER TSE
    case 0x0447: r=0x0427; break; // CYRILLIC CAPITAL LETTER CHE
    case 0x0448: r=0x0428; break; // CYRILLIC CAPITAL LETTER SHA
    case 0x0449: r=0x0429; break; // CYRILLIC CAPITAL LETTER SHCHA
    case 0x044a: r=0x042a; break; // CYRILLIC CAPITAL LETTER HARD SIGN
    case 0x044b: r=0x042b; break; // CYRILLIC CAPITAL LETTER YERU
    case 0x044c: r=0x042c; break; // CYRILLIC CAPITAL LETTER SOFT SIGN
    case 0x044d: r=0x042d; break; // CYRILLIC CAPITAL LETTER E
    case 0x044e: r=0x042e; break; // CYRILLIC CAPITAL LETTER YU
    case 0x044f: r=0x042f; break; // CYRILLIC CAPITAL LETTER YA
    case 0x0461: r=0x0460; break; // CYRILLIC CAPITAL LETTER OMEGA
    case 0x0463: r=0x0462; break; // CYRILLIC CAPITAL LETTER YAT
    case 0x0465: r=0x0464; break; // CYRILLIC CAPITAL LETTER IOTIFIED E
    case 0x0467: r=0x0466; break; // CYRILLIC CAPITAL LETTER LITTLE YUS
    case 0x0469: r=0x0468; break; // CYRILLIC CAPITAL LETTER IOTIFIED LITTLE YUS
    case 0x046b: r=0x046a; break; // CYRILLIC CAPITAL LETTER BIG YUS
    case 0x046d: r=0x046c; break; // CYRILLIC CAPITAL LETTER IOTIFIED BIG YUS
    case 0x046f: r=0x046e; break; // CYRILLIC CAPITAL LETTER KSI
    case 0x0471: r=0x0470; break; // CYRILLIC CAPITAL LETTER PSI
    case 0x0473: r=0x0472; break; // CYRILLIC CAPITAL LETTER FITA
    case 0x0475: r=0x0474; break; // CYRILLIC CAPITAL LETTER IZHITSA
    case 0x0477: r=0x0476; break; // CYRILLIC CAPITAL LETTER IZHITSA WITH DOUBLE GRAVE ACCENT
    case 0x0479: r=0x0478; break; // CYRILLIC CAPITAL LETTER UK
    case 0x047b: r=0x047a; break; // CYRILLIC CAPITAL LETTER ROUND OMEGA
    case 0x047d: r=0x047c; break; // CYRILLIC CAPITAL LETTER OMEGA WITH TITLO
    case 0x047f: r=0x047e; break; // CYRILLIC CAPITAL LETTER OT
    case 0x0481: r=0x0480; break; // CYRILLIC CAPITAL LETTER KOPPA
    case 0x048b: r=0x048a; break; // CYRILLIC CAPITAL LETTER SHORT I WITH TAIL
    case 0x048d: r=0x048c; break; // CYRILLIC CAPITAL LETTER SEMISOFT SIGN
    case 0x048f: r=0x048e; break; // CYRILLIC CAPITAL LETTER ER WITH TICK
    case 0x0491: r=0x0490; break; // CYRILLIC CAPITAL LETTER GHE WITH UPTURN
    case 0x0493: r=0x0492; break; // CYRILLIC CAPITAL LETTER GHE WITH STROKE
    case 0x0495: r=0x0494; break; // CYRILLIC CAPITAL LETTER GHE WITH MIDDLE HOOK
    case 0x0497: r=0x0496; break; // CYRILLIC CAPITAL LETTER ZHE WITH DESCENDER
    case 0x0499: r=0x0498; break; // CYRILLIC CAPITAL LETTER ZE WITH DESCENDER
    case 0x049b: r=0x049a; break; // CYRILLIC CAPITAL LETTER KA WITH DESCENDER
    case 0x049d: r=0x049c; break; // CYRILLIC CAPITAL LETTER KA WITH VERTICAL STROKE
    case 0x049f: r=0x049e; break; // CYRILLIC CAPITAL LETTER KA WITH STROKE
    case 0x04a1: r=0x04a0; break; // CYRILLIC CAPITAL LETTER BASHKIR KA
    case 0x04a3: r=0x04a2; break; // CYRILLIC CAPITAL LETTER EN WITH DESCENDER
    case 0x04a5: r=0x04a4; break; // CYRILLIC CAPITAL LIGATURE EN GHE
    case 0x04a7: r=0x04a6; break; // CYRILLIC CAPITAL LETTER PE WITH MIDDLE HOOK
    case 0x04a9: r=0x04a8; break; // CYRILLIC CAPITAL LETTER ABKHASIAN HA
    case 0x04ab: r=0x04aa; break; // CYRILLIC CAPITAL LETTER ES WITH DESCENDER
    case 0x04ad: r=0x04ac; break; // CYRILLIC CAPITAL LETTER TE WITH DESCENDER
    case 0x04af: r=0x04ae; break; // CYRILLIC CAPITAL LETTER STRAIGHT U
    case 0x04b1: r=0x04b0; break; // CYRILLIC CAPITAL LETTER STRAIGHT U WITH STROKE
    case 0x04b3: r=0x04b2; break; // CYRILLIC CAPITAL LETTER HA WITH DESCENDER
    case 0x04b5: r=0x04b4; break; // CYRILLIC CAPITAL LIGATURE TE TSE
    case 0x04b7: r=0x04b6; break; // CYRILLIC CAPITAL LETTER CHE WITH DESCENDER
    case 0x04b9: r=0x04b8; break; // CYRILLIC CAPITAL LETTER CHE WITH VERTICAL STROKE
    case 0x04bb: r=0x04ba; break; // CYRILLIC CAPITAL LETTER SHHA
    case 0x04bd: r=0x04bc; break; // CYRILLIC CAPITAL LETTER ABKHASIAN CHE
    case 0x04bf: r=0x04be; break; // CYRILLIC CAPITAL LETTER ABKHASIAN CHE WITH DESCENDER
    case 0x04c2: r=0x04c1; break; // CYRILLIC CAPITAL LETTER ZHE WITH BREVE
    case 0x04c4: r=0x04c3; break; // CYRILLIC CAPITAL LETTER KA WITH HOOK
    case 0x04c6: r=0x04c5; break; // CYRILLIC CAPITAL LETTER EL WITH TAIL
    case 0x04c8: r=0x04c7; break; // CYRILLIC CAPITAL LETTER EN WITH HOOK
    case 0x04ca: r=0x04c9; break; // CYRILLIC CAPITAL LETTER EN WITH TAIL
    case 0x04cc: r=0x04cb; break; // CYRILLIC CAPITAL LETTER KHAKASSIAN CHE
    case 0x04ce: r=0x04cd; break; // CYRILLIC CAPITAL LETTER EM WITH TAIL
    case 0x04d1: r=0x04d0; break; // CYRILLIC CAPITAL LETTER A WITH BREVE
    case 0x04d3: r=0x04d2; break; // CYRILLIC CAPITAL LETTER A WITH DIAERESIS
    case 0x04d5: r=0x04d4; break; // CYRILLIC CAPITAL LIGATURE A IE
    case 0x04d7: r=0x04d6; break; // CYRILLIC CAPITAL LETTER IE WITH BREVE
    case 0x04d9: r=0x04d8; break; // CYRILLIC CAPITAL LETTER SCHWA
    case 0x04db: r=0x04da; break; // CYRILLIC CAPITAL LETTER SCHWA WITH DIAERESIS
    case 0x04dd: r=0x04dc; break; // CYRILLIC CAPITAL LETTER ZHE WITH DIAERESIS
    case 0x04df: r=0x04de; break; // CYRILLIC CAPITAL LETTER ZE WITH DIAERESIS
    case 0x04e1: r=0x04e0; break; // CYRILLIC CAPITAL LETTER ABKHASIAN DZE
    case 0x04e3: r=0x04e2; break; // CYRILLIC CAPITAL LETTER I WITH MACRON
    case 0x04eb: r=0x04ea; break; // CYRILLIC CAPITAL LETTER BARRED O WITH DIAERESIS
    case 0x04ed: r=0x04ec; break; // CYRILLIC CAPITAL LETTER E WITH DIAERESIS
    case 0x04ef: r=0x04ee; break; // CYRILLIC CAPITAL LETTER U WITH MACRON
    case 0x04f1: r=0x04f0; break; // CYRILLIC CAPITAL LETTER U WITH DIAERESIS
    case 0x04f3: r=0x04f2; break; // CYRILLIC CAPITAL LETTER U WITH DOUBLE ACUTE
    case 0x04f5: r=0x04f4; break; // CYRILLIC CAPITAL LETTER CHE WITH DIAERESIS
    case 0x04f7: r=0x04f6; break; // CYRILLIC CAPITAL LETTER GHE WITH DESCENDER
    case 0x04f9: r=0x04f8; break; // CYRILLIC CAPITAL LETTER YERU WITH DIAERESIS
    case 0x0501: r=0x0500; break; // CYRILLIC CAPITAL LETTER KOMI DE
    case 0x0503: r=0x0502; break; // CYRILLIC CAPITAL LETTER KOMI DJE
    case 0x0505: r=0x0504; break; // CYRILLIC CAPITAL LETTER KOMI ZJE
    case 0x0507: r=0x0506; break; // CYRILLIC CAPITAL LETTER KOMI DZJE
    case 0x0509: r=0x0508; break; // CYRILLIC CAPITAL LETTER KOMI LJE
    case 0x050b: r=0x050a; break; // CYRILLIC CAPITAL LETTER KOMI NJE
    case 0x050d: r=0x050c; break; // CYRILLIC CAPITAL LETTER KOMI SJE
    case 0x050f: r=0x050e; break; // CYRILLIC CAPITAL LETTER KOMI TJE
    case 0x0561: r=0x0531; break; // ARMENIAN CAPITAL LETTER AYB
    case 0x0562: r=0x0532; break; // ARMENIAN CAPITAL LETTER BEN
    case 0x0563: r=0x0533; break; // ARMENIAN CAPITAL LETTER GIM
    case 0x0564: r=0x0534; break; // ARMENIAN CAPITAL LETTER DA
    case 0x0565: r=0x0535; break; // ARMENIAN CAPITAL LETTER ECH
    case 0x0566: r=0x0536; break; // ARMENIAN CAPITAL LETTER ZA
    case 0x0567: r=0x0537; break; // ARMENIAN CAPITAL LETTER EH
    case 0x0568: r=0x0538; break; // ARMENIAN CAPITAL LETTER ET
    case 0x0569: r=0x0539; break; // ARMENIAN CAPITAL LETTER TO
    case 0x056a: r=0x053a; break; // ARMENIAN CAPITAL LETTER ZHE
    case 0x056b: r=0x053b; break; // ARMENIAN CAPITAL LETTER INI
    case 0x056c: r=0x053c; break; // ARMENIAN CAPITAL LETTER LIWN
    case 0x056d: r=0x053d; break; // ARMENIAN CAPITAL LETTER XEH
    case 0x056e: r=0x053e; break; // ARMENIAN CAPITAL LETTER CA
    case 0x056f: r=0x053f; break; // ARMENIAN CAPITAL LETTER KEN
    case 0x0570: r=0x0540; break; // ARMENIAN CAPITAL LETTER HO
    case 0x0571: r=0x0541; break; // ARMENIAN CAPITAL LETTER JA
    case 0x0572: r=0x0542; break; // ARMENIAN CAPITAL LETTER GHAD
    case 0x0573: r=0x0543; break; // ARMENIAN CAPITAL LETTER CHEH
    case 0x0574: r=0x0544; break; // ARMENIAN CAPITAL LETTER MEN
    case 0x0575: r=0x0545; break; // ARMENIAN CAPITAL LETTER YI
    case 0x0576: r=0x0546; break; // ARMENIAN CAPITAL LETTER NOW
    case 0x0577: r=0x0547; break; // ARMENIAN CAPITAL LETTER SHA
    case 0x0578: r=0x0548; break; // ARMENIAN CAPITAL LETTER VO
    case 0x0579: r=0x0549; break; // ARMENIAN CAPITAL LETTER CHA
    case 0x057a: r=0x054a; break; // ARMENIAN CAPITAL LETTER PEH
    case 0x057b: r=0x054b; break; // ARMENIAN CAPITAL LETTER JHEH
    case 0x057c: r=0x054c; break; // ARMENIAN CAPITAL LETTER RA
    case 0x057d: r=0x054d; break; // ARMENIAN CAPITAL LETTER SEH
    case 0x057e: r=0x054e; break; // ARMENIAN CAPITAL LETTER VEW
    case 0x057f: r=0x054f; break; // ARMENIAN CAPITAL LETTER TIWN
    case 0x0580: r=0x0550; break; // ARMENIAN CAPITAL LETTER REH
    case 0x0581: r=0x0551; break; // ARMENIAN CAPITAL LETTER CO
    case 0x0582: r=0x0552; break; // ARMENIAN CAPITAL LETTER YIWN
    case 0x0583: r=0x0553; break; // ARMENIAN CAPITAL LETTER PIWR
    case 0x0584: r=0x0554; break; // ARMENIAN CAPITAL LETTER KEH
    case 0x0585: r=0x0555; break; // ARMENIAN CAPITAL LETTER OH
    case 0x0586: r=0x0556; break; // ARMENIAN CAPITAL LETTER FEH
    case 0x2d00: r=0x10a0; break; // GEORGIAN CAPITAL LETTER AN
    case 0x2d01: r=0x10a1; break; // GEORGIAN CAPITAL LETTER BAN
    case 0x2d02: r=0x10a2; break; // GEORGIAN CAPITAL LETTER GAN
    case 0x2d03: r=0x10a3; break; // GEORGIAN CAPITAL LETTER DON
    case 0x2d04: r=0x10a4; break; // GEORGIAN CAPITAL LETTER EN
    case 0x2d05: r=0x10a5; break; // GEORGIAN CAPITAL LETTER VIN
    case 0x2d06: r=0x10a6; break; // GEORGIAN CAPITAL LETTER ZEN
    case 0x2d07: r=0x10a7; break; // GEORGIAN CAPITAL LETTER TAN
    case 0x2d08: r=0x10a8; break; // GEORGIAN CAPITAL LETTER IN
    case 0x2d09: r=0x10a9; break; // GEORGIAN CAPITAL LETTER KAN
    case 0x2d0a: r=0x10aa; break; // GEORGIAN CAPITAL LETTER LAS
    case 0x2d0b: r=0x10ab; break; // GEORGIAN CAPITAL LETTER MAN
    case 0x2d0c: r=0x10ac; break; // GEORGIAN CAPITAL LETTER NAR
    case 0x2d0d: r=0x10ad; break; // GEORGIAN CAPITAL LETTER ON
    case 0x2d0e: r=0x10ae; break; // GEORGIAN CAPITAL LETTER PAR
    case 0x2d0f: r=0x10af; break; // GEORGIAN CAPITAL LETTER ZHAR
    case 0x2d10: r=0x10b0; break; // GEORGIAN CAPITAL LETTER RAE
    case 0x2d11: r=0x10b1; break; // GEORGIAN CAPITAL LETTER SAN
    case 0x2d12: r=0x10b2; break; // GEORGIAN CAPITAL LETTER TAR
    case 0x2d13: r=0x10b3; break; // GEORGIAN CAPITAL LETTER UN
    case 0x2d14: r=0x10b4; break; // GEORGIAN CAPITAL LETTER PHAR
    case 0x2d15: r=0x10b5; break; // GEORGIAN CAPITAL LETTER KHAR
    case 0x2d16: r=0x10b6; break; // GEORGIAN CAPITAL LETTER GHAN
    case 0x2d17: r=0x10b7; break; // GEORGIAN CAPITAL LETTER QAR
    case 0x2d18: r=0x10b8; break; // GEORGIAN CAPITAL LETTER SHIN
    case 0x2d19: r=0x10b9; break; // GEORGIAN CAPITAL LETTER CHIN
    case 0x2d1a: r=0x10ba; break; // GEORGIAN CAPITAL LETTER CAN
    case 0x2d1b: r=0x10bb; break; // GEORGIAN CAPITAL LETTER JIL
    case 0x2d1c: r=0x10bc; break; // GEORGIAN CAPITAL LETTER CIL
    case 0x2d1d: r=0x10bd; break; // GEORGIAN CAPITAL LETTER CHAR
    case 0x2d1e: r=0x10be; break; // GEORGIAN CAPITAL LETTER XAN
    case 0x2d1f: r=0x10bf; break; // GEORGIAN CAPITAL LETTER JHAN
    case 0x2d20: r=0x10c0; break; // GEORGIAN CAPITAL LETTER HAE
    case 0x2d21: r=0x10c1; break; // GEORGIAN CAPITAL LETTER HE
    case 0x2d22: r=0x10c2; break; // GEORGIAN CAPITAL LETTER HIE
    case 0x2d23: r=0x10c3; break; // GEORGIAN CAPITAL LETTER WE
    case 0x2d24: r=0x10c4; break; // GEORGIAN CAPITAL LETTER HAR
    case 0x2d25: r=0x10c5; break; // GEORGIAN CAPITAL LETTER HOE
    case 0x1e01: r=0x1e00; break; // LATIN CAPITAL LETTER A WITH RING BELOW
    case 0x1e03: r=0x1e02; break; // LATIN CAPITAL LETTER B WITH DOT ABOVE
    case 0x1e0b: r=0x1e0a; break; // LATIN CAPITAL LETTER D WITH DOT ABOVE
    case 0x1e0d: r=0x1e0c; break; // LATIN CAPITAL LETTER D WITH DOT BELOW
    case 0x1e0f: r=0x1e0e; break; // LATIN CAPITAL LETTER D WITH LINE BELOW
    case 0x1e1b: r=0x1e1a; break; // LATIN CAPITAL LETTER E WITH TILDE BELOW
    case 0x1e1d: r=0x1e1c; break; // LATIN CAPITAL LETTER E WITH CEDILLA AND BREVE
    case 0x1e1f: r=0x1e1e; break; // LATIN CAPITAL LETTER F WITH DOT ABOVE
    case 0x1e2b: r=0x1e2a; break; // LATIN CAPITAL LETTER H WITH BREVE BELOW
    case 0x1e2d: r=0x1e2c; break; // LATIN CAPITAL LETTER I WITH TILDE BELOW
    case 0x1e2f: r=0x1e2e; break; // LATIN CAPITAL LETTER I WITH DIAERESIS AND ACUTE
    case 0x1e3b: r=0x1e3a; break; // LATIN CAPITAL LETTER L WITH LINE BELOW
    case 0x1e3d: r=0x1e3c; break; // LATIN CAPITAL LETTER L WITH CIRCUMFLEX BELOW
    case 0x1e3f: r=0x1e3e; break; // LATIN CAPITAL LETTER M WITH ACUTE
    case 0x1e4b: r=0x1e4a; break; // LATIN CAPITAL LETTER N WITH CIRCUMFLEX BELOW
    case 0x1e4d: r=0x1e4c; break; // LATIN CAPITAL LETTER O WITH TILDE AND ACUTE
    case 0x1e4f: r=0x1e4e; break; // LATIN CAPITAL LETTER O WITH TILDE AND DIAERESIS
    case 0x1ea1: r=0x1ea0; break; // LATIN CAPITAL LETTER A WITH DOT BELOW
    case 0x1ea3: r=0x1ea2; break; // LATIN CAPITAL LETTER A WITH HOOK ABOVE
    case 0x1ea5: r=0x1ea4; break; // LATIN CAPITAL LETTER A WITH CIRCUMFLEX AND ACUTE
    case 0x1ea7: r=0x1ea6; break; // LATIN CAPITAL LETTER A WITH CIRCUMFLEX AND GRAVE
    case 0x1ea9: r=0x1ea8; break; // LATIN CAPITAL LETTER A WITH CIRCUMFLEX AND HOOK ABOVE
    case 0x1eab: r=0x1eaa; break; // LATIN CAPITAL LETTER A WITH CIRCUMFLEX AND TILDE
    case 0x1ead: r=0x1eac; break; // LATIN CAPITAL LETTER A WITH CIRCUMFLEX AND DOT BELOW
    case 0x1eaf: r=0x1eae; break; // LATIN CAPITAL LETTER A WITH BREVE AND ACUTE
    case 0x1eb1: r=0x1eb0; break; // LATIN CAPITAL LETTER A WITH BREVE AND GRAVE
    case 0x1eb3: r=0x1eb2; break; // LATIN CAPITAL LETTER A WITH BREVE AND HOOK ABOVE
    case 0x1eb5: r=0x1eb4; break; // LATIN CAPITAL LETTER A WITH BREVE AND TILDE
    case 0x1eb7: r=0x1eb6; break; // LATIN CAPITAL LETTER A WITH BREVE AND DOT BELOW
    case 0x1eb9: r=0x1eb8; break; // LATIN CAPITAL LETTER E WITH DOT BELOW
    case 0x1ebb: r=0x1eba; break; // LATIN CAPITAL LETTER E WITH HOOK ABOVE
    case 0x1ebd: r=0x1ebc; break; // LATIN CAPITAL LETTER E WITH TILDE
    case 0x1ebf: r=0x1ebe; break; // LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND ACUTE
    case 0x1ec1: r=0x1ec0; break; // LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND GRAVE
    case 0x1ec3: r=0x1ec2; break; // LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND HOOK ABOVE
    case 0x1ec5: r=0x1ec4; break; // LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND TILDE
    case 0x1ec7: r=0x1ec6; break; // LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND DOT BELOW
    case 0x1ec9: r=0x1ec8; break; // LATIN CAPITAL LETTER I WITH HOOK ABOVE
    case 0x1ecb: r=0x1eca; break; // LATIN CAPITAL LETTER I WITH DOT BELOW
    case 0x1ecd: r=0x1ecc; break; // LATIN CAPITAL LETTER O WITH DOT BELOW
    case 0x1ecf: r=0x1ece; break; // LATIN CAPITAL LETTER O WITH HOOK ABOVE
    case 0x1ed1: r=0x1ed0; break; // LATIN CAPITAL LETTER O WITH CIRCUMFLEX AND ACUTE
    case 0x1ed3: r=0x1ed2; break; // LATIN CAPITAL LETTER O WITH CIRCUMFLEX AND GRAVE
    case 0x1ed5: r=0x1ed4; break; // LATIN CAPITAL LETTER O WITH CIRCUMFLEX AND HOOK ABOVE
    case 0x1ed7: r=0x1ed6; break; // LATIN CAPITAL LETTER O WITH CIRCUMFLEX AND TILDE
    case 0x1ed9: r=0x1ed8; break; // LATIN CAPITAL LETTER O WITH CIRCUMFLEX AND DOT BELOW
    case 0x1edb: r=0x1eda; break; // LATIN CAPITAL LETTER O WITH HORN AND ACUTE
    case 0x1edd: r=0x1edc; break; // LATIN CAPITAL LETTER O WITH HORN AND GRAVE
    case 0x1edf: r=0x1ede; break; // LATIN CAPITAL LETTER O WITH HORN AND HOOK ABOVE
    case 0x1ee1: r=0x1ee0; break; // LATIN CAPITAL LETTER O WITH HORN AND TILDE
    case 0x1ee3: r=0x1ee2; break; // LATIN CAPITAL LETTER O WITH HORN AND DOT BELOW
    case 0x1ee5: r=0x1ee4; break; // LATIN CAPITAL LETTER U WITH DOT BELOW
    case 0x1ee7: r=0x1ee6; break; // LATIN CAPITAL LETTER U WITH HOOK ABOVE
    case 0x1ee9: r=0x1ee8; break; // LATIN CAPITAL LETTER U WITH HORN AND ACUTE
    case 0x1eeb: r=0x1eea; break; // LATIN CAPITAL LETTER U WITH HORN AND GRAVE
    case 0x1eed: r=0x1eec; break; // LATIN CAPITAL LETTER U WITH HORN AND HOOK ABOVE
    case 0x1eef: r=0x1eee; break; // LATIN CAPITAL LETTER U WITH HORN AND TILDE
    case 0x1ef1: r=0x1ef0; break; // LATIN CAPITAL LETTER U WITH HORN AND DOT BELOW
    case 0x1ef3: r=0x1ef2; break; // LATIN CAPITAL LETTER Y WITH GRAVE
    case 0x1ef5: r=0x1ef4; break; // LATIN CAPITAL LETTER Y WITH DOT BELOW
    case 0x1ef7: r=0x1ef6; break; // LATIN CAPITAL LETTER Y WITH HOOK ABOVE
    case 0x1ef9: r=0x1ef8; break; // LATIN CAPITAL LETTER Y WITH TILDE
    case 0x1f00: r=0x1f08; break; // GREEK CAPITAL LETTER ALPHA WITH PSILI
    case 0x1f01: r=0x1f09; break; // GREEK CAPITAL LETTER ALPHA WITH DASIA
    case 0x1f02: r=0x1f0a; break; // GREEK CAPITAL LETTER ALPHA WITH PSILI AND VARIA
    case 0x1f03: r=0x1f0b; break; // GREEK CAPITAL LETTER ALPHA WITH DASIA AND VARIA
    case 0x1f04: r=0x1f0c; break; // GREEK CAPITAL LETTER ALPHA WITH PSILI AND OXIA
    case 0x1f05: r=0x1f0d; break; // GREEK CAPITAL LETTER ALPHA WITH DASIA AND OXIA
    case 0x1f06: r=0x1f0e; break; // GREEK CAPITAL LETTER ALPHA WITH PSILI AND PERISPOMENI
    case 0x1f07: r=0x1f0f; break; // GREEK CAPITAL LETTER ALPHA WITH DASIA AND PERISPOMENI
    case 0x1f10: r=0x1f18; break; // GREEK CAPITAL LETTER EPSILON WITH PSILI
    case 0x1f11: r=0x1f19; break; // GREEK CAPITAL LETTER EPSILON WITH DASIA
    case 0x1f12: r=0x1f1a; break; // GREEK CAPITAL LETTER EPSILON WITH PSILI AND VARIA
    case 0x1f13: r=0x1f1b; break; // GREEK CAPITAL LETTER EPSILON WITH DASIA AND VARIA
    case 0x1f14: r=0x1f1c; break; // GREEK CAPITAL LETTER EPSILON WITH PSILI AND OXIA
    case 0x1f15: r=0x1f1d; break; // GREEK CAPITAL LETTER EPSILON WITH DASIA AND OXIA
    case 0x1f20: r=0x1f28; break; // GREEK CAPITAL LETTER ETA WITH PSILI
    case 0x1f21: r=0x1f29; break; // GREEK CAPITAL LETTER ETA WITH DASIA
    case 0x1f22: r=0x1f2a; break; // GREEK CAPITAL LETTER ETA WITH PSILI AND VARIA
    case 0x1f23: r=0x1f2b; break; // GREEK CAPITAL LETTER ETA WITH DASIA AND VARIA
    case 0x1f24: r=0x1f2c; break; // GREEK CAPITAL LETTER ETA WITH PSILI AND OXIA
    case 0x1f25: r=0x1f2d; break; // GREEK CAPITAL LETTER ETA WITH DASIA AND OXIA
    case 0x1f26: r=0x1f2e; break; // GREEK CAPITAL LETTER ETA WITH PSILI AND PERISPOMENI
    case 0x1f27: r=0x1f2f; break; // GREEK CAPITAL LETTER ETA WITH DASIA AND PERISPOMENI
    case 0x1f30: r=0x1f38; break; // GREEK CAPITAL LETTER IOTA WITH PSILI
    case 0x1f31: r=0x1f39; break; // GREEK CAPITAL LETTER IOTA WITH DASIA
    case 0x1f32: r=0x1f3a; break; // GREEK CAPITAL LETTER IOTA WITH PSILI AND VARIA
    case 0x1f33: r=0x1f3b; break; // GREEK CAPITAL LETTER IOTA WITH DASIA AND VARIA
    case 0x1f34: r=0x1f3c; break; // GREEK CAPITAL LETTER IOTA WITH PSILI AND OXIA
    case 0x1f35: r=0x1f3d; break; // GREEK CAPITAL LETTER IOTA WITH DASIA AND OXIA
    case 0x1f36: r=0x1f3e; break; // GREEK CAPITAL LETTER IOTA WITH PSILI AND PERISPOMENI
    case 0x1f37: r=0x1f3f; break; // GREEK CAPITAL LETTER IOTA WITH DASIA AND PERISPOMENI
    case 0x1f40: r=0x1f48; break; // GREEK CAPITAL LETTER OMICRON WITH PSILI
    case 0x1f41: r=0x1f49; break; // GREEK CAPITAL LETTER OMICRON WITH DASIA
    case 0x1f42: r=0x1f4a; break; // GREEK CAPITAL LETTER OMICRON WITH PSILI AND VARIA
    case 0x1f43: r=0x1f4b; break; // GREEK CAPITAL LETTER OMICRON WITH DASIA AND VARIA
    case 0x1f44: r=0x1f4c; break; // GREEK CAPITAL LETTER OMICRON WITH PSILI AND OXIA
    case 0x1f45: r=0x1f4d; break; // GREEK CAPITAL LETTER OMICRON WITH DASIA AND OXIA
    case 0x1f51: r=0x1f59; break; // GREEK CAPITAL LETTER UPSILON WITH DASIA
    case 0x1f53: r=0x1f5b; break; // GREEK CAPITAL LETTER UPSILON WITH DASIA AND VARIA
    case 0x1f55: r=0x1f5d; break; // GREEK CAPITAL LETTER UPSILON WITH DASIA AND OXIA
    case 0x1f57: r=0x1f5f; break; // GREEK CAPITAL LETTER UPSILON WITH DASIA AND PERISPOMENI
    case 0x1f60: r=0x1f68; break; // GREEK CAPITAL LETTER OMEGA WITH PSILI
    case 0x1f61: r=0x1f69; break; // GREEK CAPITAL LETTER OMEGA WITH DASIA
    case 0x1f62: r=0x1f6a; break; // GREEK CAPITAL LETTER OMEGA WITH PSILI AND VARIA
    case 0x1f63: r=0x1f6b; break; // GREEK CAPITAL LETTER OMEGA WITH DASIA AND VARIA
    case 0x1f64: r=0x1f6c; break; // GREEK CAPITAL LETTER OMEGA WITH PSILI AND OXIA
    case 0x1f65: r=0x1f6d; break; // GREEK CAPITAL LETTER OMEGA WITH DASIA AND OXIA
    case 0x1f66: r=0x1f6e; break; // GREEK CAPITAL LETTER OMEGA WITH PSILI AND PERISPOMENI
    case 0x1f67: r=0x1f6f; break; // GREEK CAPITAL LETTER OMEGA WITH DASIA AND PERISPOMENI
    case 0x1f80: r=0x1f88; break; // GREEK CAPITAL LETTER ALPHA WITH PSILI AND PROSGEGRAMMENI
    case 0x1f81: r=0x1f89; break; // GREEK CAPITAL LETTER ALPHA WITH DASIA AND PROSGEGRAMMENI
    case 0x1f82: r=0x1f8a; break; // GREEK CAPITAL LETTER ALPHA WITH PSILI AND VARIA AND PROSGEGRAMMENI
    case 0x1f83: r=0x1f8b; break; // GREEK CAPITAL LETTER ALPHA WITH DASIA AND VARIA AND PROSGEGRAMMENI
    case 0x1f84: r=0x1f8c; break; // GREEK CAPITAL LETTER ALPHA WITH PSILI AND OXIA AND PROSGEGRAMMENI
    case 0x1f85: r=0x1f8d; break; // GREEK CAPITAL LETTER ALPHA WITH DASIA AND OXIA AND PROSGEGRAMMENI
    case 0x1f86: r=0x1f8e; break; // GREEK CAPITAL LETTER ALPHA WITH PSILI AND PERISPOMENI AND PROSGEGRAMMENI
    case 0x1f87: r=0x1f8f; break; // GREEK CAPITAL LETTER ALPHA WITH DASIA AND PERISPOMENI AND PROSGEGRAMMENI
    case 0x1f90: r=0x1f98; break; // GREEK CAPITAL LETTER ETA WITH PSILI AND PROSGEGRAMMENI
    case 0x1f91: r=0x1f99; break; // GREEK CAPITAL LETTER ETA WITH DASIA AND PROSGEGRAMMENI
    case 0x1f92: r=0x1f9a; break; // GREEK CAPITAL LETTER ETA WITH PSILI AND VARIA AND PROSGEGRAMMENI
    case 0x1f93: r=0x1f9b; break; // GREEK CAPITAL LETTER ETA WITH DASIA AND VARIA AND PROSGEGRAMMENI
    case 0x1f94: r=0x1f9c; break; // GREEK CAPITAL LETTER ETA WITH PSILI AND OXIA AND PROSGEGRAMMENI
    case 0x1f95: r=0x1f9d; break; // GREEK CAPITAL LETTER ETA WITH DASIA AND OXIA AND PROSGEGRAMMENI
    case 0x1f96: r=0x1f9e; break; // GREEK CAPITAL LETTER ETA WITH PSILI AND PERISPOMENI AND PROSGEGRAMMENI
    case 0x1f97: r=0x1f9f; break; // GREEK CAPITAL LETTER ETA WITH DASIA AND PERISPOMENI AND PROSGEGRAMMENI
    case 0x1fa0: r=0x1fa8; break; // GREEK CAPITAL LETTER OMEGA WITH PSILI AND PROSGEGRAMMENI
    case 0x1fa1: r=0x1fa9; break; // GREEK CAPITAL LETTER OMEGA WITH DASIA AND PROSGEGRAMMENI
    case 0x1fa2: r=0x1faa; break; // GREEK CAPITAL LETTER OMEGA WITH PSILI AND VARIA AND PROSGEGRAMMENI
    case 0x1fa3: r=0x1fab; break; // GREEK CAPITAL LETTER OMEGA WITH DASIA AND VARIA AND PROSGEGRAMMENI
    case 0x1fa4: r=0x1fac; break; // GREEK CAPITAL LETTER OMEGA WITH PSILI AND OXIA AND PROSGEGRAMMENI
    case 0x1fa5: r=0x1fad; break; // GREEK CAPITAL LETTER OMEGA WITH DASIA AND OXIA AND PROSGEGRAMMENI
    case 0x1fa6: r=0x1fae; break; // GREEK CAPITAL LETTER OMEGA WITH PSILI AND PERISPOMENI AND PROSGEGRAMMENI
    case 0x1fa7: r=0x1faf; break; // GREEK CAPITAL LETTER OMEGA WITH DASIA AND PERISPOMENI AND PROSGEGRAMMENI
    case 0x1fb0: r=0x1fb8; break; // GREEK CAPITAL LETTER ALPHA WITH VRACHY
    case 0x1fb1: r=0x1fb9; break; // GREEK CAPITAL LETTER ALPHA WITH MACRON
    case 0x1f70: r=0x1fba; break; // GREEK CAPITAL LETTER ALPHA WITH VARIA
    case 0x1f71: r=0x1fbb; break; // GREEK CAPITAL LETTER ALPHA WITH OXIA
    case 0x1fb3: r=0x1fbc; break; // GREEK CAPITAL LETTER ALPHA WITH PROSGEGRAMMENI
    case 0x1f72: r=0x1fc8; break; // GREEK CAPITAL LETTER EPSILON WITH VARIA
    case 0x1f73: r=0x1fc9; break; // GREEK CAPITAL LETTER EPSILON WITH OXIA
    case 0x1f74: r=0x1fca; break; // GREEK CAPITAL LETTER ETA WITH VARIA
    case 0x1f75: r=0x1fcb; break; // GREEK CAPITAL LETTER ETA WITH OXIA
    case 0x1fc3: r=0x1fcc; break; // GREEK CAPITAL LETTER ETA WITH PROSGEGRAMMENI
    case 0x1fd0: r=0x1fd8; break; // GREEK CAPITAL LETTER IOTA WITH VRACHY
    case 0x1fd1: r=0x1fd9; break; // GREEK CAPITAL LETTER IOTA WITH MACRON
    case 0x1f76: r=0x1fda; break; // GREEK CAPITAL LETTER IOTA WITH VARIA
    case 0x1f77: r=0x1fdb; break; // GREEK CAPITAL LETTER IOTA WITH OXIA
    case 0x1fe0: r=0x1fe8; break; // GREEK CAPITAL LETTER UPSILON WITH VRACHY
    case 0x1fe1: r=0x1fe9; break; // GREEK CAPITAL LETTER UPSILON WITH MACRON
    case 0x1f7a: r=0x1fea; break; // GREEK CAPITAL LETTER UPSILON WITH VARIA
    case 0x1f7b: r=0x1feb; break; // GREEK CAPITAL LETTER UPSILON WITH OXIA
    case 0x1fe5: r=0x1fec; break; // GREEK CAPITAL LETTER RHO WITH DASIA
    case 0x1f78: r=0x1ff8; break; // GREEK CAPITAL LETTER OMICRON WITH VARIA
    case 0x1f79: r=0x1ff9; break; // GREEK CAPITAL LETTER OMICRON WITH OXIA
    case 0x1f7c: r=0x1ffa; break; // GREEK CAPITAL LETTER OMEGA WITH VARIA
    case 0x1f7d: r=0x1ffb; break; // GREEK CAPITAL LETTER OMEGA WITH OXIA
    case 0x1ff3: r=0x1ffc; break; // GREEK CAPITAL LETTER OMEGA WITH PROSGEGRAMMENI
    case 0x2170: r=0x2160; break; // ROMAN NUMERAL ONE
    case 0x2171: r=0x2161; break; // ROMAN NUMERAL TWO
    case 0x2172: r=0x2162; break; // ROMAN NUMERAL THREE
    case 0x2173: r=0x2163; break; // ROMAN NUMERAL FOUR
    case 0x2174: r=0x2164; break; // ROMAN NUMERAL FIVE
    case 0x2175: r=0x2165; break; // ROMAN NUMERAL SIX
    case 0x2176: r=0x2166; break; // ROMAN NUMERAL SEVEN
    case 0x2177: r=0x2167; break; // ROMAN NUMERAL EIGHT
    case 0x2178: r=0x2168; break; // ROMAN NUMERAL NINE
    case 0x2179: r=0x2169; break; // ROMAN NUMERAL TEN
    case 0x217a: r=0x216a; break; // ROMAN NUMERAL ELEVEN
    case 0x217b: r=0x216b; break; // ROMAN NUMERAL TWELVE
    case 0x217c: r=0x216c; break; // ROMAN NUMERAL FIFTY
    case 0x217d: r=0x216d; break; // ROMAN NUMERAL ONE HUNDRED
    case 0x217e: r=0x216e; break; // ROMAN NUMERAL FIVE HUNDRED
    case 0x217f: r=0x216f; break; // ROMAN NUMERAL ONE THOUSAND
    case 0x24d0: r=0x24b6; break; // CIRCLED LATIN CAPITAL LETTER A
    case 0x24d1: r=0x24b7; break; // CIRCLED LATIN CAPITAL LETTER B
    case 0x24d2: r=0x24b8; break; // CIRCLED LATIN CAPITAL LETTER C
    case 0x24d3: r=0x24b9; break; // CIRCLED LATIN CAPITAL LETTER D
    case 0x24d4: r=0x24ba; break; // CIRCLED LATIN CAPITAL LETTER E
    case 0x24d5: r=0x24bb; break; // CIRCLED LATIN CAPITAL LETTER F
    case 0x24d6: r=0x24bc; break; // CIRCLED LATIN CAPITAL LETTER G
    case 0x24d7: r=0x24bd; break; // CIRCLED LATIN CAPITAL LETTER H
    case 0x24d8: r=0x24be; break; // CIRCLED LATIN CAPITAL LETTER I
    case 0x24d9: r=0x24bf; break; // CIRCLED LATIN CAPITAL LETTER J
    case 0x24da: r=0x24c0; break; // CIRCLED LATIN CAPITAL LETTER K
    case 0x24db: r=0x24c1; break; // CIRCLED LATIN CAPITAL LETTER L
    case 0x24dc: r=0x24c2; break; // CIRCLED LATIN CAPITAL LETTER M
    case 0x24dd: r=0x24c3; break; // CIRCLED LATIN CAPITAL LETTER N
    case 0x24de: r=0x24c4; break; // CIRCLED LATIN CAPITAL LETTER O
    case 0x24df: r=0x24c5; break; // CIRCLED LATIN CAPITAL LETTER P
    case 0x24e0: r=0x24c6; break; // CIRCLED LATIN CAPITAL LETTER Q
    case 0x24e1: r=0x24c7; break; // CIRCLED LATIN CAPITAL LETTER R
    case 0x24e2: r=0x24c8; break; // CIRCLED LATIN CAPITAL LETTER S
    case 0x24e3: r=0x24c9; break; // CIRCLED LATIN CAPITAL LETTER T
    case 0x2c30: r=0x2c00; break; // GLAGOLITIC CAPITAL LETTER AZU
    case 0x2c31: r=0x2c01; break; // GLAGOLITIC CAPITAL LETTER BUKY
    case 0x2c32: r=0x2c02; break; // GLAGOLITIC CAPITAL LETTER VEDE
    case 0x2c33: r=0x2c03; break; // GLAGOLITIC CAPITAL LETTER GLAGOLI
    case 0x2c34: r=0x2c04; break; // GLAGOLITIC CAPITAL LETTER DOBRO
    case 0x2c35: r=0x2c05; break; // GLAGOLITIC CAPITAL LETTER YESTU
    case 0x2c36: r=0x2c06; break; // GLAGOLITIC CAPITAL LETTER ZHIVETE
    case 0x2c37: r=0x2c07; break; // GLAGOLITIC CAPITAL LETTER DZELO
    case 0x2c38: r=0x2c08; break; // GLAGOLITIC CAPITAL LETTER ZEMLJA
    case 0x2c39: r=0x2c09; break; // GLAGOLITIC CAPITAL LETTER IZHE
    case 0x2c3a: r=0x2c0a; break; // GLAGOLITIC CAPITAL LETTER INITIAL IZHE
    case 0x2c3b: r=0x2c0b; break; // GLAGOLITIC CAPITAL LETTER I
    case 0x2c3c: r=0x2c0c; break; // GLAGOLITIC CAPITAL LETTER DJERVI
    case 0x2c3d: r=0x2c0d; break; // GLAGOLITIC CAPITAL LETTER KAKO
    case 0x2c3e: r=0x2c0e; break; // GLAGOLITIC CAPITAL LETTER LJUDIJE
    case 0x2c3f: r=0x2c0f; break; // GLAGOLITIC CAPITAL LETTER MYSLITE
    case 0x2c40: r=0x2c10; break; // GLAGOLITIC CAPITAL LETTER NASHI
    case 0x2c41: r=0x2c11; break; // GLAGOLITIC CAPITAL LETTER ONU
    case 0x2c42: r=0x2c12; break; // GLAGOLITIC CAPITAL LETTER POKOJI
    case 0x2c43: r=0x2c13; break; // GLAGOLITIC CAPITAL LETTER RITSI
    case 0x2c44: r=0x2c14; break; // GLAGOLITIC CAPITAL LETTER SLOVO
    case 0x2c45: r=0x2c15; break; // GLAGOLITIC CAPITAL LETTER TVRIDO
    case 0x2c46: r=0x2c16; break; // GLAGOLITIC CAPITAL LETTER UKU
    case 0x2c47: r=0x2c17; break; // GLAGOLITIC CAPITAL LETTER FRITU
    case 0x2c48: r=0x2c18; break; // GLAGOLITIC CAPITAL LETTER HERU
    case 0x2c49: r=0x2c19; break; // GLAGOLITIC CAPITAL LETTER OTU
    case 0x2c4a: r=0x2c1a; break; // GLAGOLITIC CAPITAL LETTER PE
    case 0x2c4b: r=0x2c1b; break; // GLAGOLITIC CAPITAL LETTER SHTA
    case 0x2c4c: r=0x2c1c; break; // GLAGOLITIC CAPITAL LETTER TSI
    case 0x2c4d: r=0x2c1d; break; // GLAGOLITIC CAPITAL LETTER CHRIVI
    case 0x2c4e: r=0x2c1e; break; // GLAGOLITIC CAPITAL LETTER SHA
    case 0x2c4f: r=0x2c1f; break; // GLAGOLITIC CAPITAL LETTER YERU
    case 0x2c50: r=0x2c20; break; // GLAGOLITIC CAPITAL LETTER YERI
    case 0x2c51: r=0x2c21; break; // GLAGOLITIC CAPITAL LETTER YATI
    case 0x2c52: r=0x2c22; break; // GLAGOLITIC CAPITAL LETTER SPIDERY HA
    case 0x2c53: r=0x2c23; break; // GLAGOLITIC CAPITAL LETTER YU
    case 0x2c54: r=0x2c24; break; // GLAGOLITIC CAPITAL LETTER SMALL YUS
    case 0x2c55: r=0x2c25; break; // GLAGOLITIC CAPITAL LETTER SMALL YUS WITH TAIL
    case 0x2c56: r=0x2c26; break; // GLAGOLITIC CAPITAL LETTER YO
    case 0x2c57: r=0x2c27; break; // GLAGOLITIC CAPITAL LETTER IOTATED SMALL YUS
    case 0x2c58: r=0x2c28; break; // GLAGOLITIC CAPITAL LETTER BIG YUS
    case 0x2c59: r=0x2c29; break; // GLAGOLITIC CAPITAL LETTER IOTATED BIG YUS
    case 0x2c5a: r=0x2c2a; break; // GLAGOLITIC CAPITAL LETTER FITA
    case 0x2c5b: r=0x2c2b; break; // GLAGOLITIC CAPITAL LETTER IZHITSA
    case 0x2c5c: r=0x2c2c; break; // GLAGOLITIC CAPITAL LETTER SHTAPIC
    case 0x2c5d: r=0x2c2d; break; // GLAGOLITIC CAPITAL LETTER TROKUTASTI A
    case 0x2c5e: r=0x2c2e; break; // GLAGOLITIC CAPITAL LETTER LATINATE MYSLITE
    case 0x2c81: r=0x2c80; break; // COPTIC CAPITAL LETTER ALFA
    case 0x2c83: r=0x2c82; break; // COPTIC CAPITAL LETTER VIDA
    case 0x2c85: r=0x2c84; break; // COPTIC CAPITAL LETTER GAMMA
    case 0x2c87: r=0x2c86; break; // COPTIC CAPITAL LETTER DALDA
    case 0x2c89: r=0x2c88; break; // COPTIC CAPITAL LETTER EIE
    case 0x2c8b: r=0x2c8a; break; // COPTIC CAPITAL LETTER SOU
    case 0x2c8d: r=0x2c8c; break; // COPTIC CAPITAL LETTER ZATA
    case 0x2c8f: r=0x2c8e; break; // COPTIC CAPITAL LETTER HATE
    case 0x2c91: r=0x2c90; break; // COPTIC CAPITAL LETTER THETHE
    case 0x2c93: r=0x2c92; break; // COPTIC CAPITAL LETTER IAUDA
    case 0x2c95: r=0x2c94; break; // COPTIC CAPITAL LETTER KAPA
    case 0x2c97: r=0x2c96; break; // COPTIC CAPITAL LETTER LAULA
    case 0x2c99: r=0x2c98; break; // COPTIC CAPITAL LETTER MI
    case 0x2c9b: r=0x2c9a; break; // COPTIC CAPITAL LETTER NI
    case 0x2c9d: r=0x2c9c; break; // COPTIC CAPITAL LETTER KSI
    case 0x2c9f: r=0x2c9e; break; // COPTIC CAPITAL LETTER O
    case 0x2ca1: r=0x2ca0; break; // COPTIC CAPITAL LETTER PI
    case 0x2ca3: r=0x2ca2; break; // COPTIC CAPITAL LETTER RO
    case 0x2ca5: r=0x2ca4; break; // COPTIC CAPITAL LETTER SIMA
    case 0x2ca7: r=0x2ca6; break; // COPTIC CAPITAL LETTER TAU
    case 0x2ca9: r=0x2ca8; break; // COPTIC CAPITAL LETTER UA
    case 0x2cab: r=0x2caa; break; // COPTIC CAPITAL LETTER FI
    case 0x2cad: r=0x2cac; break; // COPTIC CAPITAL LETTER KHI
    case 0x2caf: r=0x2cae; break; // COPTIC CAPITAL LETTER PSI
    case 0x2cb1: r=0x2cb0; break; // COPTIC CAPITAL LETTER OOU
    case 0x2cb3: r=0x2cb2; break; // COPTIC CAPITAL LETTER DIALECT-P ALEF
    case 0x2cb5: r=0x2cb4; break; // COPTIC CAPITAL LETTER OLD COPTIC AIN
    case 0x2cb7: r=0x2cb6; break; // COPTIC CAPITAL LETTER CRYPTOGRAMMIC EIE
    case 0x2cb9: r=0x2cb8; break; // COPTIC CAPITAL LETTER DIALECT-P KAPA
    case 0x2cbb: r=0x2cba; break; // COPTIC CAPITAL LETTER DIALECT-P NI
    case 0x2cbd: r=0x2cbc; break; // COPTIC CAPITAL LETTER CRYPTOGRAMMIC NI
    case 0x2cbf: r=0x2cbe; break; // COPTIC CAPITAL LETTER OLD COPTIC OOU
    case 0x2cc1: r=0x2cc0; break; // COPTIC CAPITAL LETTER SAMPI
    case 0x2cc3: r=0x2cc2; break; // COPTIC CAPITAL LETTER CROSSED SHEI
    case 0x2cc5: r=0x2cc4; break; // COPTIC CAPITAL LETTER OLD COPTIC SHEI
    case 0x2cc7: r=0x2cc6; break; // COPTIC CAPITAL LETTER OLD COPTIC ESH
    case 0x2cc9: r=0x2cc8; break; // COPTIC CAPITAL LETTER AKHMIMIC KHEI
    case 0x2ccb: r=0x2cca; break; // COPTIC CAPITAL LETTER DIALECT-P HORI
    case 0x2ccd: r=0x2ccc; break; // COPTIC CAPITAL LETTER OLD COPTIC HORI
    case 0x2ccf: r=0x2cce; break; // COPTIC CAPITAL LETTER OLD COPTIC HA
    case 0x2cd1: r=0x2cd0; break; // COPTIC CAPITAL LETTER L-SHAPED HA
    case 0x2cd3: r=0x2cd2; break; // COPTIC CAPITAL LETTER OLD COPTIC HEI
    case 0x2cd5: r=0x2cd4; break; // COPTIC CAPITAL LETTER OLD COPTIC HAT
    case 0x2cd7: r=0x2cd6; break; // COPTIC CAPITAL LETTER OLD COPTIC GANGIA
    case 0x2cd9: r=0x2cd8; break; // COPTIC CAPITAL LETTER OLD COPTIC DJA
    case 0x2cdb: r=0x2cda; break; // COPTIC CAPITAL LETTER OLD COPTIC SHIMA
    case 0x2cdd: r=0x2cdc; break; // COPTIC CAPITAL LETTER OLD NUBIAN SHIMA
    case 0x2cdf: r=0x2cde; break; // COPTIC CAPITAL LETTER OLD NUBIAN NGI
    case 0x2ce1: r=0x2ce0; break; // COPTIC CAPITAL LETTER OLD NUBIAN NYI
    case 0x2ce3: r=0x2ce2; break; // COPTIC CAPITAL LETTER OLD NUBIAN WAU
    case 0xff41: r=0xff21; break; // FULLWIDTH LATIN CAPITAL LETTER A
    case 0xff42: r=0xff22; break; // FULLWIDTH LATIN CAPITAL LETTER B
    case 0xff43: r=0xff23; break; // FULLWIDTH LATIN CAPITAL LETTER C
    case 0xff44: r=0xff24; break; // FULLWIDTH LATIN CAPITAL LETTER D
    case 0xff45: r=0xff25; break; // FULLWIDTH LATIN CAPITAL LETTER E
    case 0xff46: r=0xff26; break; // FULLWIDTH LATIN CAPITAL LETTER F
    case 0xff47: r=0xff27; break; // FULLWIDTH LATIN CAPITAL LETTER G
    case 0xff48: r=0xff28; break; // FULLWIDTH LATIN CAPITAL LETTER H
    case 0xff49: r=0xff29; break; // FULLWIDTH LATIN CAPITAL LETTER I
    case 0xff4a: r=0xff2a; break; // FULLWIDTH LATIN CAPITAL LETTER J
    case 0xff4b: r=0xff2b; break; // FULLWIDTH LATIN CAPITAL LETTER K
    case 0xff4c: r=0xff2c; break; // FULLWIDTH LATIN CAPITAL LETTER L
    case 0xff4d: r=0xff2d; break; // FULLWIDTH LATIN CAPITAL LETTER M
    case 0xff4e: r=0xff2e; break; // FULLWIDTH LATIN CAPITAL LETTER N
    case 0xff4f: r=0xff2f; break; // FULLWIDTH LATIN CAPITAL LETTER O
    case 0xff50: r=0xff30; break; // FULLWIDTH LATIN CAPITAL LETTER P
    case 0xff51: r=0xff31; break; // FULLWIDTH LATIN CAPITAL LETTER Q
    case 0xff52: r=0xff32; break; // FULLWIDTH LATIN CAPITAL LETTER R
    case 0xff53: r=0xff33; break; // FULLWIDTH LATIN CAPITAL LETTER S
    case 0xff54: r=0xff34; break; // FULLWIDTH LATIN CAPITAL LETTER T
    case 0xff55: r=0xff35; break; // FULLWIDTH LATIN CAPITAL LETTER U
    case 0xff56: r=0xff36; break; // FULLWIDTH LATIN CAPITAL LETTER V
    case 0xff57: r=0xff37; break; // FULLWIDTH LATIN CAPITAL LETTER W
    case 0xff58: r=0xff38; break; // FULLWIDTH LATIN CAPITAL LETTER X
    case 0xff59: r=0xff39; break; // FULLWIDTH LATIN CAPITAL LETTER Y
    case 0xff5a: r=0xff3a; break; // FULLWIDTH LATIN CAPITAL LETTER Z
      /* end of autogenerated code */
    default: r=c; break;
  }
  return r;
}


unichar u_tolower (unichar c) {
  unichar r;
  switch (c) {
      /* begin of autogenerated code */
    case 0x0041: r=0x0061; break; // LATIN CAPITAL LETTER A
    case 0x0042: r=0x0062; break; // LATIN CAPITAL LETTER B
    case 0x0043: r=0x0063; break; // LATIN CAPITAL LETTER C
    case 0x0044: r=0x0064; break; // LATIN CAPITAL LETTER D
    case 0x0045: r=0x0065; break; // LATIN CAPITAL LETTER E
    case 0x0046: r=0x0066; break; // LATIN CAPITAL LETTER F
    case 0x0047: r=0x0067; break; // LATIN CAPITAL LETTER G
    case 0x0048: r=0x0068; break; // LATIN CAPITAL LETTER H
    case 0x0049: r=0x0069; break; // LATIN CAPITAL LETTER I
    case 0x004a: r=0x006a; break; // LATIN CAPITAL LETTER J
    case 0x004b: r=0x006b; break; // LATIN CAPITAL LETTER K
    case 0x004c: r=0x006c; break; // LATIN CAPITAL LETTER L
    case 0x004d: r=0x006d; break; // LATIN CAPITAL LETTER M
    case 0x004e: r=0x006e; break; // LATIN CAPITAL LETTER N
    case 0x004f: r=0x006f; break; // LATIN CAPITAL LETTER O
    case 0x0050: r=0x0070; break; // LATIN CAPITAL LETTER P
    case 0x0051: r=0x0071; break; // LATIN CAPITAL LETTER Q
    case 0x0052: r=0x0072; break; // LATIN CAPITAL LETTER R
    case 0x0053: r=0x0073; break; // LATIN CAPITAL LETTER S
    case 0x0054: r=0x0074; break; // LATIN CAPITAL LETTER T
    case 0x0055: r=0x0075; break; // LATIN CAPITAL LETTER U
    case 0x0056: r=0x0076; break; // LATIN CAPITAL LETTER V
    case 0x0057: r=0x0077; break; // LATIN CAPITAL LETTER W
    case 0x0058: r=0x0078; break; // LATIN CAPITAL LETTER X
    case 0x0059: r=0x0079; break; // LATIN CAPITAL LETTER Y
    case 0x005a: r=0x007a; break; // LATIN CAPITAL LETTER Z
    case 0x00b5: r=0x03bc; break; // MICRO SIGN
    case 0x00c0: r=0x00e0; break; // LATIN CAPITAL LETTER A WITH GRAVE
    case 0x00c1: r=0x00e1; break; // LATIN CAPITAL LETTER A WITH ACUTE
    case 0x00c2: r=0x00e2; break; // LATIN CAPITAL LETTER A WITH CIRCUMFLEX
    case 0x00c3: r=0x00e3; break; // LATIN CAPITAL LETTER A WITH TILDE
    case 0x00c4: r=0x00e4; break; // LATIN CAPITAL LETTER A WITH DIAERESIS
    case 0x00c5: r=0x00e5; break; // LATIN CAPITAL LETTER A WITH RING ABOVE
    case 0x00c6: r=0x00e6; break; // LATIN CAPITAL LETTER AE
    case 0x00c7: r=0x00e7; break; // LATIN CAPITAL LETTER C WITH CEDILLA
    case 0x00c8: r=0x00e8; break; // LATIN CAPITAL LETTER E WITH GRAVE
    case 0x00c9: r=0x00e9; break; // LATIN CAPITAL LETTER E WITH ACUTE
    case 0x00ca: r=0x00ea; break; // LATIN CAPITAL LETTER E WITH CIRCUMFLEX
    case 0x00cb: r=0x00eb; break; // LATIN CAPITAL LETTER E WITH DIAERESIS
    case 0x00cc: r=0x00ec; break; // LATIN CAPITAL LETTER I WITH GRAVE
    case 0x00cd: r=0x00ed; break; // LATIN CAPITAL LETTER I WITH ACUTE
    case 0x00ce: r=0x00ee; break; // LATIN CAPITAL LETTER I WITH CIRCUMFLEX
    case 0x00cf: r=0x00ef; break; // LATIN CAPITAL LETTER I WITH DIAERESIS
    case 0x00d0: r=0x00f0; break; // LATIN CAPITAL LETTER ETH
    case 0x00d1: r=0x00f1; break; // LATIN CAPITAL LETTER N WITH TILDE
    case 0x00d2: r=0x00f2; break; // LATIN CAPITAL LETTER O WITH GRAVE
    case 0x00d3: r=0x00f3; break; // LATIN CAPITAL LETTER O WITH ACUTE
    case 0x00d4: r=0x00f4; break; // LATIN CAPITAL LETTER O WITH CIRCUMFLEX
    case 0x00d5: r=0x00f5; break; // LATIN CAPITAL LETTER O WITH TILDE
    case 0x00d6: r=0x00f6; break; // LATIN CAPITAL LETTER O WITH DIAERESIS
    case 0x00d8: r=0x00f8; break; // LATIN CAPITAL LETTER O WITH STROKE
    case 0x00d9: r=0x00f9; break; // LATIN CAPITAL LETTER U WITH GRAVE
    case 0x00da: r=0x00fa; break; // LATIN CAPITAL LETTER U WITH ACUTE
    case 0x00db: r=0x00fb; break; // LATIN CAPITAL LETTER U WITH CIRCUMFLEX
    case 0x00dc: r=0x00fc; break; // LATIN CAPITAL LETTER U WITH DIAERESIS
    case 0x00dd: r=0x00fd; break; // LATIN CAPITAL LETTER Y WITH ACUTE
    case 0x00de: r=0x00fe; break; // LATIN CAPITAL LETTER THORN
    case 0x0100: r=0x0101; break; // LATIN CAPITAL LETTER A WITH MACRON
    case 0x0102: r=0x0103; break; // LATIN CAPITAL LETTER A WITH BREVE
    case 0x0104: r=0x0105; break; // LATIN CAPITAL LETTER A WITH OGONEK
    case 0x0106: r=0x0107; break; // LATIN CAPITAL LETTER C WITH ACUTE
    case 0x0108: r=0x0109; break; // LATIN CAPITAL LETTER C WITH CIRCUMFLEX
    case 0x010a: r=0x010b; break; // LATIN CAPITAL LETTER C WITH DOT ABOVE
    case 0x010c: r=0x010d; break; // LATIN CAPITAL LETTER C WITH CARON
    case 0x010e: r=0x010f; break; // LATIN CAPITAL LETTER D WITH CARON
    case 0x0110: r=0x0111; break; // LATIN CAPITAL LETTER D WITH STROKE
    case 0x0112: r=0x0113; break; // LATIN CAPITAL LETTER E WITH MACRON
    case 0x0114: r=0x0115; break; // LATIN CAPITAL LETTER E WITH BREVE
    case 0x0116: r=0x0117; break; // LATIN CAPITAL LETTER E WITH DOT ABOVE
    case 0x0118: r=0x0119; break; // LATIN CAPITAL LETTER E WITH OGONEK
    case 0x011a: r=0x011b; break; // LATIN CAPITAL LETTER E WITH CARON
    case 0x011c: r=0x011d; break; // LATIN CAPITAL LETTER G WITH CIRCUMFLEX
    case 0x011e: r=0x011f; break; // LATIN CAPITAL LETTER G WITH BREVE
    case 0x0120: r=0x0121; break; // LATIN CAPITAL LETTER G WITH DOT ABOVE
    case 0x0122: r=0x0123; break; // LATIN CAPITAL LETTER G WITH CEDILLA
    case 0x0124: r=0x0125; break; // LATIN CAPITAL LETTER H WITH CIRCUMFLEX
    case 0x0126: r=0x0127; break; // LATIN CAPITAL LETTER H WITH STROKE
    case 0x0128: r=0x0129; break; // LATIN CAPITAL LETTER I WITH TILDE
    case 0x012a: r=0x012b; break; // LATIN CAPITAL LETTER I WITH MACRON
    case 0x012c: r=0x012d; break; // LATIN CAPITAL LETTER I WITH BREVE
    case 0x012e: r=0x012f; break; // LATIN CAPITAL LETTER I WITH OGONEK
    case 0x0132: r=0x0133; break; // LATIN CAPITAL LIGATURE IJ
    case 0x0134: r=0x0135; break; // LATIN CAPITAL LETTER J WITH CIRCUMFLEX
    case 0x0136: r=0x0137; break; // LATIN CAPITAL LETTER K WITH CEDILLA
    case 0x0139: r=0x013a; break; // LATIN CAPITAL LETTER L WITH ACUTE
    case 0x013b: r=0x013c; break; // LATIN CAPITAL LETTER L WITH CEDILLA
    case 0x013d: r=0x013e; break; // LATIN CAPITAL LETTER L WITH CARON
    case 0x013f: r=0x0140; break; // LATIN CAPITAL LETTER L WITH MIDDLE DOT
    case 0x0141: r=0x0142; break; // LATIN CAPITAL LETTER L WITH STROKE
    case 0x0143: r=0x0144; break; // LATIN CAPITAL LETTER N WITH ACUTE
    case 0x0145: r=0x0146; break; // LATIN CAPITAL LETTER N WITH CEDILLA
    case 0x0147: r=0x0148; break; // LATIN CAPITAL LETTER N WITH CARON
    case 0x014a: r=0x014b; break; // LATIN CAPITAL LETTER ENG
    case 0x014c: r=0x014d; break; // LATIN CAPITAL LETTER O WITH MACRON
    case 0x014e: r=0x014f; break; // LATIN CAPITAL LETTER O WITH BREVE
    case 0x0150: r=0x0151; break; // LATIN CAPITAL LETTER O WITH DOUBLE ACUTE
    case 0x0152: r=0x0153; break; // LATIN CAPITAL LIGATURE OE
    case 0x0154: r=0x0155; break; // LATIN CAPITAL LETTER R WITH ACUTE
    case 0x0156: r=0x0157; break; // LATIN CAPITAL LETTER R WITH CEDILLA
    case 0x0158: r=0x0159; break; // LATIN CAPITAL LETTER R WITH CARON
    case 0x015a: r=0x015b; break; // LATIN CAPITAL LETTER S WITH ACUTE
    case 0x015c: r=0x015d; break; // LATIN CAPITAL LETTER S WITH CIRCUMFLEX
    case 0x015e: r=0x015f; break; // LATIN CAPITAL LETTER S WITH CEDILLA
    case 0x0160: r=0x0161; break; // LATIN CAPITAL LETTER S WITH CARON
    case 0x0162: r=0x0163; break; // LATIN CAPITAL LETTER T WITH CEDILLA
    case 0x0164: r=0x0165; break; // LATIN CAPITAL LETTER T WITH CARON
    case 0x0166: r=0x0167; break; // LATIN CAPITAL LETTER T WITH STROKE
    case 0x0168: r=0x0169; break; // LATIN CAPITAL LETTER U WITH TILDE
    case 0x016a: r=0x016b; break; // LATIN CAPITAL LETTER U WITH MACRON
    case 0x016c: r=0x016d; break; // LATIN CAPITAL LETTER U WITH BREVE
    case 0x016e: r=0x016f; break; // LATIN CAPITAL LETTER U WITH RING ABOVE
    case 0x0170: r=0x0171; break; // LATIN CAPITAL LETTER U WITH DOUBLE ACUTE
    case 0x0172: r=0x0173; break; // LATIN CAPITAL LETTER U WITH OGONEK
    case 0x0174: r=0x0175; break; // LATIN CAPITAL LETTER W WITH CIRCUMFLEX
    case 0x0176: r=0x0177; break; // LATIN CAPITAL LETTER Y WITH CIRCUMFLEX
    case 0x0178: r=0x00ff; break; // LATIN CAPITAL LETTER Y WITH DIAERESIS
    case 0x0179: r=0x017a; break; // LATIN CAPITAL LETTER Z WITH ACUTE
    case 0x017b: r=0x017c; break; // LATIN CAPITAL LETTER Z WITH DOT ABOVE
    case 0x017d: r=0x017e; break; // LATIN CAPITAL LETTER Z WITH CARON
    case 0x017f: r=0x0073; break; // LATIN SMALL LETTER LONG S
    case 0x0181: r=0x0253; break; // LATIN CAPITAL LETTER B WITH HOOK
    case 0x0182: r=0x0183; break; // LATIN CAPITAL LETTER B WITH TOPBAR
    case 0x0184: r=0x0185; break; // LATIN CAPITAL LETTER TONE SIX
    case 0x0186: r=0x0254; break; // LATIN CAPITAL LETTER OPEN O
    case 0x0187: r=0x0188; break; // LATIN CAPITAL LETTER C WITH HOOK
    case 0x0189: r=0x0256; break; // LATIN CAPITAL LETTER AFRICAN D
    case 0x018a: r=0x0257; break; // LATIN CAPITAL LETTER D WITH HOOK
    case 0x018b: r=0x018c; break; // LATIN CAPITAL LETTER D WITH TOPBAR
    case 0x018e: r=0x01dd; break; // LATIN CAPITAL LETTER REVERSED E
    case 0x018f: r=0x0259; break; // LATIN CAPITAL LETTER SCHWA
    case 0x0190: r=0x025b; break; // LATIN CAPITAL LETTER OPEN E
    case 0x0191: r=0x0192; break; // LATIN CAPITAL LETTER F WITH HOOK
    case 0x0193: r=0x0260; break; // LATIN CAPITAL LETTER G WITH HOOK
    case 0x0194: r=0x0263; break; // LATIN CAPITAL LETTER GAMMA
    case 0x0196: r=0x0269; break; // LATIN CAPITAL LETTER IOTA
    case 0x0197: r=0x0268; break; // LATIN CAPITAL LETTER I WITH STROKE
    case 0x0198: r=0x0199; break; // LATIN CAPITAL LETTER K WITH HOOK
    case 0x019c: r=0x026f; break; // LATIN CAPITAL LETTER TURNED M
    case 0x019d: r=0x0272; break; // LATIN CAPITAL LETTER N WITH LEFT HOOK
    case 0x019f: r=0x0275; break; // LATIN CAPITAL LETTER O WITH MIDDLE TILDE
    case 0x01a0: r=0x01a1; break; // LATIN CAPITAL LETTER O WITH HORN
    case 0x01a2: r=0x01a3; break; // LATIN CAPITAL LETTER OI
    case 0x01a4: r=0x01a5; break; // LATIN CAPITAL LETTER P WITH HOOK
    case 0x01a6: r=0x0280; break; // LATIN LETTER YR
    case 0x01a7: r=0x01a8; break; // LATIN CAPITAL LETTER TONE TWO
    case 0x01a9: r=0x0283; break; // LATIN CAPITAL LETTER ESH
    case 0x01ac: r=0x01ad; break; // LATIN CAPITAL LETTER T WITH HOOK
    case 0x01ae: r=0x0288; break; // LATIN CAPITAL LETTER T WITH RETROFLEX HOOK
    case 0x01af: r=0x01b0; break; // LATIN CAPITAL LETTER U WITH HORN
    case 0x01b1: r=0x028a; break; // LATIN CAPITAL LETTER UPSILON
    case 0x01b2: r=0x028b; break; // LATIN CAPITAL LETTER V WITH HOOK
    case 0x01b3: r=0x01b4; break; // LATIN CAPITAL LETTER Y WITH HOOK
    case 0x01b5: r=0x01b6; break; // LATIN CAPITAL LETTER Z WITH STROKE
    case 0x01b7: r=0x0292; break; // LATIN CAPITAL LETTER EZH
    case 0x01b8: r=0x01b9; break; // LATIN CAPITAL LETTER EZH REVERSED
    case 0x01bc: r=0x01bd; break; // LATIN CAPITAL LETTER TONE FIVE
    case 0x01c4: r=0x01c6; break; // LATIN CAPITAL LETTER DZ WITH CARON
    case 0x01c5: r=0x01c6; break; // LATIN CAPITAL LETTER D WITH SMALL LETTER Z WITH CARON
    case 0x01c7: r=0x01c9; break; // LATIN CAPITAL LETTER LJ
    case 0x01c8: r=0x01c9; break; // LATIN CAPITAL LETTER L WITH SMALL LETTER J
    case 0x01ca: r=0x01cc; break; // LATIN CAPITAL LETTER NJ
    case 0x01cb: r=0x01cc; break; // LATIN CAPITAL LETTER N WITH SMALL LETTER J
    case 0x01cd: r=0x01ce; break; // LATIN CAPITAL LETTER A WITH CARON
    case 0x01cf: r=0x01d0; break; // LATIN CAPITAL LETTER I WITH CARON
    case 0x01d1: r=0x01d2; break; // LATIN CAPITAL LETTER O WITH CARON
    case 0x01d3: r=0x01d4; break; // LATIN CAPITAL LETTER U WITH CARON
    case 0x01d5: r=0x01d6; break; // LATIN CAPITAL LETTER U WITH DIAERESIS AND MACRON
    case 0x01d7: r=0x01d8; break; // LATIN CAPITAL LETTER U WITH DIAERESIS AND ACUTE
    case 0x01d9: r=0x01da; break; // LATIN CAPITAL LETTER U WITH DIAERESIS AND CARON
    case 0x01db: r=0x01dc; break; // LATIN CAPITAL LETTER U WITH DIAERESIS AND GRAVE
    case 0x01de: r=0x01df; break; // LATIN CAPITAL LETTER A WITH DIAERESIS AND MACRON
    case 0x01e0: r=0x01e1; break; // LATIN CAPITAL LETTER A WITH DOT ABOVE AND MACRON
    case 0x01e2: r=0x01e3; break; // LATIN CAPITAL LETTER AE WITH MACRON
    case 0x01e4: r=0x01e5; break; // LATIN CAPITAL LETTER G WITH STROKE
    case 0x01e6: r=0x01e7; break; // LATIN CAPITAL LETTER G WITH CARON
    case 0x01e8: r=0x01e9; break; // LATIN CAPITAL LETTER K WITH CARON
    case 0x01ea: r=0x01eb; break; // LATIN CAPITAL LETTER O WITH OGONEK
    case 0x01ec: r=0x01ed; break; // LATIN CAPITAL LETTER O WITH OGONEK AND MACRON
    case 0x01ee: r=0x01ef; break; // LATIN CAPITAL LETTER EZH WITH CARON
    case 0x01f1: r=0x01f3; break; // LATIN CAPITAL LETTER DZ
    case 0x01f2: r=0x01f3; break; // LATIN CAPITAL LETTER D WITH SMALL LETTER Z
    case 0x01f4: r=0x01f5; break; // LATIN CAPITAL LETTER G WITH ACUTE
    case 0x01f6: r=0x0195; break; // LATIN CAPITAL LETTER HWAIR
    case 0x01f7: r=0x01bf; break; // LATIN CAPITAL LETTER WYNN
    case 0x01f8: r=0x01f9; break; // LATIN CAPITAL LETTER N WITH GRAVE
    case 0x01fa: r=0x01fb; break; // LATIN CAPITAL LETTER A WITH RING ABOVE AND ACUTE
    case 0x01fc: r=0x01fd; break; // LATIN CAPITAL LETTER AE WITH ACUTE
    case 0x01fe: r=0x01ff; break; // LATIN CAPITAL LETTER O WITH STROKE AND ACUTE
    case 0x0200: r=0x0201; break; // LATIN CAPITAL LETTER A WITH DOUBLE GRAVE
    case 0x0202: r=0x0203; break; // LATIN CAPITAL LETTER A WITH INVERTED BREVE
    case 0x0204: r=0x0205; break; // LATIN CAPITAL LETTER E WITH DOUBLE GRAVE
    case 0x0206: r=0x0207; break; // LATIN CAPITAL LETTER E WITH INVERTED BREVE
    case 0x0208: r=0x0209; break; // LATIN CAPITAL LETTER I WITH DOUBLE GRAVE
    case 0x020a: r=0x020b; break; // LATIN CAPITAL LETTER I WITH INVERTED BREVE
    case 0x020c: r=0x020d; break; // LATIN CAPITAL LETTER O WITH DOUBLE GRAVE
    case 0x020e: r=0x020f; break; // LATIN CAPITAL LETTER O WITH INVERTED BREVE
    case 0x0210: r=0x0211; break; // LATIN CAPITAL LETTER R WITH DOUBLE GRAVE
    case 0x0212: r=0x0213; break; // LATIN CAPITAL LETTER R WITH INVERTED BREVE
    case 0x0214: r=0x0215; break; // LATIN CAPITAL LETTER U WITH DOUBLE GRAVE
    case 0x0216: r=0x0217; break; // LATIN CAPITAL LETTER U WITH INVERTED BREVE
    case 0x0218: r=0x0219; break; // LATIN CAPITAL LETTER S WITH COMMA BELOW
    case 0x021a: r=0x021b; break; // LATIN CAPITAL LETTER T WITH COMMA BELOW
    case 0x021c: r=0x021d; break; // LATIN CAPITAL LETTER YOGH
    case 0x021e: r=0x021f; break; // LATIN CAPITAL LETTER H WITH CARON
    case 0x0220: r=0x019e; break; // LATIN CAPITAL LETTER N WITH LONG RIGHT LEG
    case 0x0222: r=0x0223; break; // LATIN CAPITAL LETTER OU
    case 0x0224: r=0x0225; break; // LATIN CAPITAL LETTER Z WITH HOOK
    case 0x0226: r=0x0227; break; // LATIN CAPITAL LETTER A WITH DOT ABOVE
    case 0x0228: r=0x0229; break; // LATIN CAPITAL LETTER E WITH CEDILLA
    case 0x022a: r=0x022b; break; // LATIN CAPITAL LETTER O WITH DIAERESIS AND MACRON
    case 0x022c: r=0x022d; break; // LATIN CAPITAL LETTER O WITH TILDE AND MACRON
    case 0x022e: r=0x022f; break; // LATIN CAPITAL LETTER O WITH DOT ABOVE
    case 0x0230: r=0x0231; break; // LATIN CAPITAL LETTER O WITH DOT ABOVE AND MACRON
    case 0x0232: r=0x0233; break; // LATIN CAPITAL LETTER Y WITH MACRON
    case 0x023b: r=0x023c; break; // LATIN CAPITAL LETTER C WITH STROKE
    case 0x023d: r=0x019a; break; // LATIN CAPITAL LETTER L WITH BAR
    case 0x0241: r=0x0294; break; // LATIN CAPITAL LETTER GLOTTAL STOP
    case 0x0345: r=0x03b9; break; // COMBINING GREEK YPOGEGRAMMENI
    case 0x0386: r=0x03ac; break; // GREEK CAPITAL LETTER ALPHA WITH TONOS
    case 0x0388: r=0x03ad; break; // GREEK CAPITAL LETTER EPSILON WITH TONOS
    case 0x0389: r=0x03ae; break; // GREEK CAPITAL LETTER ETA WITH TONOS
    case 0x038a: r=0x03af; break; // GREEK CAPITAL LETTER IOTA WITH TONOS
    case 0x038c: r=0x03cc; break; // GREEK CAPITAL LETTER OMICRON WITH TONOS
    case 0x038e: r=0x03cd; break; // GREEK CAPITAL LETTER UPSILON WITH TONOS
    case 0x038f: r=0x03ce; break; // GREEK CAPITAL LETTER OMEGA WITH TONOS
    case 0x0391: r=0x03b1; break; // GREEK CAPITAL LETTER ALPHA
    case 0x0392: r=0x03b2; break; // GREEK CAPITAL LETTER BETA
    case 0x0393: r=0x03b3; break; // GREEK CAPITAL LETTER GAMMA
    case 0x0394: r=0x03b4; break; // GREEK CAPITAL LETTER DELTA
    case 0x0395: r=0x03b5; break; // GREEK CAPITAL LETTER EPSILON
    case 0x0396: r=0x03b6; break; // GREEK CAPITAL LETTER ZETA
    case 0x0397: r=0x03b7; break; // GREEK CAPITAL LETTER ETA
    case 0x0398: r=0x03b8; break; // GREEK CAPITAL LETTER THETA
    case 0x0399: r=0x03b9; break; // GREEK CAPITAL LETTER IOTA
    case 0x039a: r=0x03ba; break; // GREEK CAPITAL LETTER KAPPA
    case 0x039b: r=0x03bb; break; // GREEK CAPITAL LETTER LAMDA
    case 0x039c: r=0x03bc; break; // GREEK CAPITAL LETTER MU
    case 0x039d: r=0x03bd; break; // GREEK CAPITAL LETTER NU
    case 0x039e: r=0x03be; break; // GREEK CAPITAL LETTER XI
    case 0x039f: r=0x03bf; break; // GREEK CAPITAL LETTER OMICRON
    case 0x03a0: r=0x03c0; break; // GREEK CAPITAL LETTER PI
    case 0x03a1: r=0x03c1; break; // GREEK CAPITAL LETTER RHO
    case 0x03a3: r=0x03c3; break; // GREEK CAPITAL LETTER SIGMA
    case 0x03a4: r=0x03c4; break; // GREEK CAPITAL LETTER TAU
    case 0x03a5: r=0x03c5; break; // GREEK CAPITAL LETTER UPSILON
    case 0x03a6: r=0x03c6; break; // GREEK CAPITAL LETTER PHI
    case 0x03a7: r=0x03c7; break; // GREEK CAPITAL LETTER CHI
    case 0x03a8: r=0x03c8; break; // GREEK CAPITAL LETTER PSI
    case 0x03a9: r=0x03c9; break; // GREEK CAPITAL LETTER OMEGA
    case 0x03aa: r=0x03ca; break; // GREEK CAPITAL LETTER IOTA WITH DIALYTIKA
    case 0x03ab: r=0x03cb; break; // GREEK CAPITAL LETTER UPSILON WITH DIALYTIKA
    case 0x03c2: r=0x03c3; break; // GREEK SMALL LETTER FINAL SIGMA
    case 0x03d0: r=0x03b2; break; // GREEK BETA SYMBOL
    case 0x03d1: r=0x03b8; break; // GREEK THETA SYMBOL
    case 0x03d5: r=0x03c6; break; // GREEK PHI SYMBOL
    case 0x03d6: r=0x03c0; break; // GREEK PI SYMBOL
    case 0x03d8: r=0x03d9; break; // GREEK LETTER ARCHAIC KOPPA
    case 0x03da: r=0x03db; break; // GREEK LETTER STIGMA
    case 0x03dc: r=0x03dd; break; // GREEK LETTER DIGAMMA
    case 0x03de: r=0x03df; break; // GREEK LETTER KOPPA
    case 0x03e0: r=0x03e1; break; // GREEK LETTER SAMPI
    case 0x03e2: r=0x03e3; break; // COPTIC CAPITAL LETTER SHEI
    case 0x03e4: r=0x03e5; break; // COPTIC CAPITAL LETTER FEI
    case 0x03e6: r=0x03e7; break; // COPTIC CAPITAL LETTER KHEI
    case 0x03e8: r=0x03e9; break; // COPTIC CAPITAL LETTER HORI
    case 0x03ea: r=0x03eb; break; // COPTIC CAPITAL LETTER GANGIA
    case 0x03ec: r=0x03ed; break; // COPTIC CAPITAL LETTER SHIMA
    case 0x03ee: r=0x03ef; break; // COPTIC CAPITAL LETTER DEI
    case 0x03f0: r=0x03ba; break; // GREEK KAPPA SYMBOL
    case 0x03f1: r=0x03c1; break; // GREEK RHO SYMBOL
    case 0x03f4: r=0x03b8; break; // GREEK CAPITAL THETA SYMBOL
    case 0x03f5: r=0x03b5; break; // GREEK LUNATE EPSILON SYMBOL
    case 0x03f7: r=0x03f8; break; // GREEK CAPITAL LETTER SHO
    case 0x03f9: r=0x03f2; break; // GREEK CAPITAL LUNATE SIGMA SYMBOL
    case 0x03fa: r=0x03fb; break; // GREEK CAPITAL LETTER SAN
    case 0x0400: r=0x0450; break; // CYRILLIC CAPITAL LETTER IE WITH GRAVE
    case 0x0401: r=0x0451; break; // CYRILLIC CAPITAL LETTER IO
    case 0x0402: r=0x0452; break; // CYRILLIC CAPITAL LETTER DJE
    case 0x0403: r=0x0453; break; // CYRILLIC CAPITAL LETTER GJE
    case 0x0404: r=0x0454; break; // CYRILLIC CAPITAL LETTER UKRAINIAN IE
    case 0x0405: r=0x0455; break; // CYRILLIC CAPITAL LETTER DZE
    case 0x0406: r=0x0456; break; // CYRILLIC CAPITAL LETTER BYELORUSSIAN-UKRAINIAN I
    case 0x0407: r=0x0457; break; // CYRILLIC CAPITAL LETTER YI
    case 0x0408: r=0x0458; break; // CYRILLIC CAPITAL LETTER JE
    case 0x0409: r=0x0459; break; // CYRILLIC CAPITAL LETTER LJE
    case 0x040a: r=0x045a; break; // CYRILLIC CAPITAL LETTER NJE
    case 0x040b: r=0x045b; break; // CYRILLIC CAPITAL LETTER TSHE
    case 0x040c: r=0x045c; break; // CYRILLIC CAPITAL LETTER KJE
    case 0x040d: r=0x045d; break; // CYRILLIC CAPITAL LETTER I WITH GRAVE
    case 0x040e: r=0x045e; break; // CYRILLIC CAPITAL LETTER SHORT U
    case 0x040f: r=0x045f; break; // CYRILLIC CAPITAL LETTER DZHE
    case 0x0410: r=0x0430; break; // CYRILLIC CAPITAL LETTER A
    case 0x0411: r=0x0431; break; // CYRILLIC CAPITAL LETTER BE
    case 0x0412: r=0x0432; break; // CYRILLIC CAPITAL LETTER VE
    case 0x0413: r=0x0433; break; // CYRILLIC CAPITAL LETTER GHE
    case 0x0414: r=0x0434; break; // CYRILLIC CAPITAL LETTER DE
    case 0x0415: r=0x0435; break; // CYRILLIC CAPITAL LETTER IE
    case 0x0416: r=0x0436; break; // CYRILLIC CAPITAL LETTER ZHE
    case 0x0417: r=0x0437; break; // CYRILLIC CAPITAL LETTER ZE
    case 0x0418: r=0x0438; break; // CYRILLIC CAPITAL LETTER I
    case 0x0419: r=0x0439; break; // CYRILLIC CAPITAL LETTER SHORT I
    case 0x041a: r=0x043a; break; // CYRILLIC CAPITAL LETTER KA
    case 0x041b: r=0x043b; break; // CYRILLIC CAPITAL LETTER EL
    case 0x041c: r=0x043c; break; // CYRILLIC CAPITAL LETTER EM
    case 0x041d: r=0x043d; break; // CYRILLIC CAPITAL LETTER EN
    case 0x041e: r=0x043e; break; // CYRILLIC CAPITAL LETTER O
    case 0x041f: r=0x043f; break; // CYRILLIC CAPITAL LETTER PE
    case 0x0420: r=0x0440; break; // CYRILLIC CAPITAL LETTER ER
    case 0x0421: r=0x0441; break; // CYRILLIC CAPITAL LETTER ES
    case 0x0422: r=0x0442; break; // CYRILLIC CAPITAL LETTER TE
    case 0x0423: r=0x0443; break; // CYRILLIC CAPITAL LETTER U
    case 0x0424: r=0x0444; break; // CYRILLIC CAPITAL LETTER EF
    case 0x0425: r=0x0445; break; // CYRILLIC CAPITAL LETTER HA
    case 0x0426: r=0x0446; break; // CYRILLIC CAPITAL LETTER TSE
    case 0x0427: r=0x0447; break; // CYRILLIC CAPITAL LETTER CHE
    case 0x0428: r=0x0448; break; // CYRILLIC CAPITAL LETTER SHA
    case 0x0429: r=0x0449; break; // CYRILLIC CAPITAL LETTER SHCHA
    case 0x042a: r=0x044a; break; // CYRILLIC CAPITAL LETTER HARD SIGN
    case 0x042b: r=0x044b; break; // CYRILLIC CAPITAL LETTER YERU
    case 0x042c: r=0x044c; break; // CYRILLIC CAPITAL LETTER SOFT SIGN
    case 0x042d: r=0x044d; break; // CYRILLIC CAPITAL LETTER E
    case 0x042e: r=0x044e; break; // CYRILLIC CAPITAL LETTER YU
    case 0x042f: r=0x044f; break; // CYRILLIC CAPITAL LETTER YA
    case 0x0460: r=0x0461; break; // CYRILLIC CAPITAL LETTER OMEGA
    case 0x0462: r=0x0463; break; // CYRILLIC CAPITAL LETTER YAT
    case 0x0464: r=0x0465; break; // CYRILLIC CAPITAL LETTER IOTIFIED E
    case 0x0466: r=0x0467; break; // CYRILLIC CAPITAL LETTER LITTLE YUS
    case 0x0468: r=0x0469; break; // CYRILLIC CAPITAL LETTER IOTIFIED LITTLE YUS
    case 0x046a: r=0x046b; break; // CYRILLIC CAPITAL LETTER BIG YUS
    case 0x046c: r=0x046d; break; // CYRILLIC CAPITAL LETTER IOTIFIED BIG YUS
    case 0x046e: r=0x046f; break; // CYRILLIC CAPITAL LETTER KSI
    case 0x0470: r=0x0471; break; // CYRILLIC CAPITAL LETTER PSI
    case 0x0472: r=0x0473; break; // CYRILLIC CAPITAL LETTER FITA
    case 0x0474: r=0x0475; break; // CYRILLIC CAPITAL LETTER IZHITSA
    case 0x0476: r=0x0477; break; // CYRILLIC CAPITAL LETTER IZHITSA WITH DOUBLE GRAVE ACCENT
    case 0x0478: r=0x0479; break; // CYRILLIC CAPITAL LETTER UK
    case 0x047a: r=0x047b; break; // CYRILLIC CAPITAL LETTER ROUND OMEGA
    case 0x047c: r=0x047d; break; // CYRILLIC CAPITAL LETTER OMEGA WITH TITLO
    case 0x047e: r=0x047f; break; // CYRILLIC CAPITAL LETTER OT
    case 0x0480: r=0x0481; break; // CYRILLIC CAPITAL LETTER KOPPA
    case 0x048a: r=0x048b; break; // CYRILLIC CAPITAL LETTER SHORT I WITH TAIL
    case 0x048c: r=0x048d; break; // CYRILLIC CAPITAL LETTER SEMISOFT SIGN
    case 0x048e: r=0x048f; break; // CYRILLIC CAPITAL LETTER ER WITH TICK
    case 0x0490: r=0x0491; break; // CYRILLIC CAPITAL LETTER GHE WITH UPTURN
    case 0x0492: r=0x0493; break; // CYRILLIC CAPITAL LETTER GHE WITH STROKE
    case 0x0494: r=0x0495; break; // CYRILLIC CAPITAL LETTER GHE WITH MIDDLE HOOK
    case 0x0496: r=0x0497; break; // CYRILLIC CAPITAL LETTER ZHE WITH DESCENDER
    case 0x0498: r=0x0499; break; // CYRILLIC CAPITAL LETTER ZE WITH DESCENDER
    case 0x049a: r=0x049b; break; // CYRILLIC CAPITAL LETTER KA WITH DESCENDER
    case 0x049c: r=0x049d; break; // CYRILLIC CAPITAL LETTER KA WITH VERTICAL STROKE
    case 0x049e: r=0x049f; break; // CYRILLIC CAPITAL LETTER KA WITH STROKE
    case 0x04a0: r=0x04a1; break; // CYRILLIC CAPITAL LETTER BASHKIR KA
    case 0x04a2: r=0x04a3; break; // CYRILLIC CAPITAL LETTER EN WITH DESCENDER
    case 0x04a4: r=0x04a5; break; // CYRILLIC CAPITAL LIGATURE EN GHE
    case 0x04a6: r=0x04a7; break; // CYRILLIC CAPITAL LETTER PE WITH MIDDLE HOOK
    case 0x04a8: r=0x04a9; break; // CYRILLIC CAPITAL LETTER ABKHASIAN HA
    case 0x04aa: r=0x04ab; break; // CYRILLIC CAPITAL LETTER ES WITH DESCENDER
    case 0x04ac: r=0x04ad; break; // CYRILLIC CAPITAL LETTER TE WITH DESCENDER
    case 0x04ae: r=0x04af; break; // CYRILLIC CAPITAL LETTER STRAIGHT U
    case 0x04b0: r=0x04b1; break; // CYRILLIC CAPITAL LETTER STRAIGHT U WITH STROKE
    case 0x04b2: r=0x04b3; break; // CYRILLIC CAPITAL LETTER HA WITH DESCENDER
    case 0x04b4: r=0x04b5; break; // CYRILLIC CAPITAL LIGATURE TE TSE
    case 0x04b6: r=0x04b7; break; // CYRILLIC CAPITAL LETTER CHE WITH DESCENDER
    case 0x04b8: r=0x04b9; break; // CYRILLIC CAPITAL LETTER CHE WITH VERTICAL STROKE
    case 0x04ba: r=0x04bb; break; // CYRILLIC CAPITAL LETTER SHHA
    case 0x04bc: r=0x04bd; break; // CYRILLIC CAPITAL LETTER ABKHASIAN CHE
    case 0x04be: r=0x04bf; break; // CYRILLIC CAPITAL LETTER ABKHASIAN CHE WITH DESCENDER
    case 0x04c1: r=0x04c2; break; // CYRILLIC CAPITAL LETTER ZHE WITH BREVE
    case 0x04c3: r=0x04c4; break; // CYRILLIC CAPITAL LETTER KA WITH HOOK
    case 0x04c5: r=0x04c6; break; // CYRILLIC CAPITAL LETTER EL WITH TAIL
    case 0x04c7: r=0x04c8; break; // CYRILLIC CAPITAL LETTER EN WITH HOOK
    case 0x04c9: r=0x04ca; break; // CYRILLIC CAPITAL LETTER EN WITH TAIL
    case 0x04cb: r=0x04cc; break; // CYRILLIC CAPITAL LETTER KHAKASSIAN CHE
    case 0x04cd: r=0x04ce; break; // CYRILLIC CAPITAL LETTER EM WITH TAIL
    case 0x04d0: r=0x04d1; break; // CYRILLIC CAPITAL LETTER A WITH BREVE
    case 0x04d2: r=0x04d3; break; // CYRILLIC CAPITAL LETTER A WITH DIAERESIS
    case 0x04d4: r=0x04d5; break; // CYRILLIC CAPITAL LIGATURE A IE
    case 0x04d6: r=0x04d7; break; // CYRILLIC CAPITAL LETTER IE WITH BREVE
    case 0x04d8: r=0x04d9; break; // CYRILLIC CAPITAL LETTER SCHWA
    case 0x04da: r=0x04db; break; // CYRILLIC CAPITAL LETTER SCHWA WITH DIAERESIS
    case 0x04dc: r=0x04dd; break; // CYRILLIC CAPITAL LETTER ZHE WITH DIAERESIS
    case 0x04de: r=0x04df; break; // CYRILLIC CAPITAL LETTER ZE WITH DIAERESIS
    case 0x04e0: r=0x04e1; break; // CYRILLIC CAPITAL LETTER ABKHASIAN DZE
    case 0x04e2: r=0x04e3; break; // CYRILLIC CAPITAL LETTER I WITH MACRON
    case 0x04e4: r=0x04e5; break; // CYRILLIC CAPITAL LETTER I WITH DIAERESIS
    case 0x04e6: r=0x04e7; break; // CYRILLIC CAPITAL LETTER O WITH DIAERESIS
    case 0x04e8: r=0x04e9; break; // CYRILLIC CAPITAL LETTER BARRED O
    case 0x04ea: r=0x04eb; break; // CYRILLIC CAPITAL LETTER BARRED O WITH DIAERESIS
    case 0x04ec: r=0x04ed; break; // CYRILLIC CAPITAL LETTER E WITH DIAERESIS
    case 0x04ee: r=0x04ef; break; // CYRILLIC CAPITAL LETTER U WITH MACRON
    case 0x04f0: r=0x04f1; break; // CYRILLIC CAPITAL LETTER U WITH DIAERESIS
    case 0x04f2: r=0x04f3; break; // CYRILLIC CAPITAL LETTER U WITH DOUBLE ACUTE
    case 0x04f4: r=0x04f5; break; // CYRILLIC CAPITAL LETTER CHE WITH DIAERESIS
    case 0x04f6: r=0x04f7; break; // CYRILLIC CAPITAL LETTER GHE WITH DESCENDER
    case 0x04f8: r=0x04f9; break; // CYRILLIC CAPITAL LETTER YERU WITH DIAERESIS
    case 0x0500: r=0x0501; break; // CYRILLIC CAPITAL LETTER KOMI DE
    case 0x0502: r=0x0503; break; // CYRILLIC CAPITAL LETTER KOMI DJE
    case 0x0504: r=0x0505; break; // CYRILLIC CAPITAL LETTER KOMI ZJE
    case 0x0506: r=0x0507; break; // CYRILLIC CAPITAL LETTER KOMI DZJE
    case 0x0508: r=0x0509; break; // CYRILLIC CAPITAL LETTER KOMI LJE
    case 0x050a: r=0x050b; break; // CYRILLIC CAPITAL LETTER KOMI NJE
    case 0x050c: r=0x050d; break; // CYRILLIC CAPITAL LETTER KOMI SJE
    case 0x050e: r=0x050f; break; // CYRILLIC CAPITAL LETTER KOMI TJE
    case 0x0531: r=0x0561; break; // ARMENIAN CAPITAL LETTER AYB
    case 0x0532: r=0x0562; break; // ARMENIAN CAPITAL LETTER BEN
    case 0x0533: r=0x0563; break; // ARMENIAN CAPITAL LETTER GIM
    case 0x0534: r=0x0564; break; // ARMENIAN CAPITAL LETTER DA
    case 0x0535: r=0x0565; break; // ARMENIAN CAPITAL LETTER ECH
    case 0x0536: r=0x0566; break; // ARMENIAN CAPITAL LETTER ZA
    case 0x0537: r=0x0567; break; // ARMENIAN CAPITAL LETTER EH
    case 0x0538: r=0x0568; break; // ARMENIAN CAPITAL LETTER ET
    case 0x0539: r=0x0569; break; // ARMENIAN CAPITAL LETTER TO
    case 0x053a: r=0x056a; break; // ARMENIAN CAPITAL LETTER ZHE
    case 0x053b: r=0x056b; break; // ARMENIAN CAPITAL LETTER INI
    case 0x053c: r=0x056c; break; // ARMENIAN CAPITAL LETTER LIWN
    case 0x053d: r=0x056d; break; // ARMENIAN CAPITAL LETTER XEH
    case 0x053e: r=0x056e; break; // ARMENIAN CAPITAL LETTER CA
    case 0x053f: r=0x056f; break; // ARMENIAN CAPITAL LETTER KEN
    case 0x0540: r=0x0570; break; // ARMENIAN CAPITAL LETTER HO
    case 0x0541: r=0x0571; break; // ARMENIAN CAPITAL LETTER JA
    case 0x0542: r=0x0572; break; // ARMENIAN CAPITAL LETTER GHAD
    case 0x0543: r=0x0573; break; // ARMENIAN CAPITAL LETTER CHEH
    case 0x0544: r=0x0574; break; // ARMENIAN CAPITAL LETTER MEN
    case 0x0545: r=0x0575; break; // ARMENIAN CAPITAL LETTER YI
    case 0x0546: r=0x0576; break; // ARMENIAN CAPITAL LETTER NOW
    case 0x0547: r=0x0577; break; // ARMENIAN CAPITAL LETTER SHA
    case 0x0548: r=0x0578; break; // ARMENIAN CAPITAL LETTER VO
    case 0x0549: r=0x0579; break; // ARMENIAN CAPITAL LETTER CHA
    case 0x054a: r=0x057a; break; // ARMENIAN CAPITAL LETTER PEH
    case 0x054b: r=0x057b; break; // ARMENIAN CAPITAL LETTER JHEH
    case 0x054c: r=0x057c; break; // ARMENIAN CAPITAL LETTER RA
    case 0x054d: r=0x057d; break; // ARMENIAN CAPITAL LETTER SEH
    case 0x054e: r=0x057e; break; // ARMENIAN CAPITAL LETTER VEW
    case 0x054f: r=0x057f; break; // ARMENIAN CAPITAL LETTER TIWN
    case 0x0550: r=0x0580; break; // ARMENIAN CAPITAL LETTER REH
    case 0x0551: r=0x0581; break; // ARMENIAN CAPITAL LETTER CO
    case 0x0552: r=0x0582; break; // ARMENIAN CAPITAL LETTER YIWN
    case 0x0553: r=0x0583; break; // ARMENIAN CAPITAL LETTER PIWR
    case 0x0554: r=0x0584; break; // ARMENIAN CAPITAL LETTER KEH
    case 0x0555: r=0x0585; break; // ARMENIAN CAPITAL LETTER OH
    case 0x0556: r=0x0586; break; // ARMENIAN CAPITAL LETTER FEH
    case 0x10a0: r=0x2d00; break; // GEORGIAN CAPITAL LETTER AN
    case 0x10a1: r=0x2d01; break; // GEORGIAN CAPITAL LETTER BAN
    case 0x10a2: r=0x2d02; break; // GEORGIAN CAPITAL LETTER GAN
    case 0x10a3: r=0x2d03; break; // GEORGIAN CAPITAL LETTER DON
    case 0x10a4: r=0x2d04; break; // GEORGIAN CAPITAL LETTER EN
    case 0x10a5: r=0x2d05; break; // GEORGIAN CAPITAL LETTER VIN
    case 0x10a6: r=0x2d06; break; // GEORGIAN CAPITAL LETTER ZEN
    case 0x10a7: r=0x2d07; break; // GEORGIAN CAPITAL LETTER TAN
    case 0x10a8: r=0x2d08; break; // GEORGIAN CAPITAL LETTER IN
    case 0x10a9: r=0x2d09; break; // GEORGIAN CAPITAL LETTER KAN
    case 0x10aa: r=0x2d0a; break; // GEORGIAN CAPITAL LETTER LAS
    case 0x10ab: r=0x2d0b; break; // GEORGIAN CAPITAL LETTER MAN
    case 0x10ac: r=0x2d0c; break; // GEORGIAN CAPITAL LETTER NAR
    case 0x10ad: r=0x2d0d; break; // GEORGIAN CAPITAL LETTER ON
    case 0x10ae: r=0x2d0e; break; // GEORGIAN CAPITAL LETTER PAR
    case 0x10af: r=0x2d0f; break; // GEORGIAN CAPITAL LETTER ZHAR
    case 0x10b0: r=0x2d10; break; // GEORGIAN CAPITAL LETTER RAE
    case 0x10b1: r=0x2d11; break; // GEORGIAN CAPITAL LETTER SAN
    case 0x10b2: r=0x2d12; break; // GEORGIAN CAPITAL LETTER TAR
    case 0x10b3: r=0x2d13; break; // GEORGIAN CAPITAL LETTER UN
    case 0x10b4: r=0x2d14; break; // GEORGIAN CAPITAL LETTER PHAR
    case 0x10b5: r=0x2d15; break; // GEORGIAN CAPITAL LETTER KHAR
    case 0x10b6: r=0x2d16; break; // GEORGIAN CAPITAL LETTER GHAN
    case 0x10b7: r=0x2d17; break; // GEORGIAN CAPITAL LETTER QAR
    case 0x10b8: r=0x2d18; break; // GEORGIAN CAPITAL LETTER SHIN
    case 0x10b9: r=0x2d19; break; // GEORGIAN CAPITAL LETTER CHIN
    case 0x10ba: r=0x2d1a; break; // GEORGIAN CAPITAL LETTER CAN
    case 0x10bb: r=0x2d1b; break; // GEORGIAN CAPITAL LETTER JIL
    case 0x10bc: r=0x2d1c; break; // GEORGIAN CAPITAL LETTER CIL
    case 0x10bd: r=0x2d1d; break; // GEORGIAN CAPITAL LETTER CHAR
    case 0x10be: r=0x2d1e; break; // GEORGIAN CAPITAL LETTER XAN
    case 0x10bf: r=0x2d1f; break; // GEORGIAN CAPITAL LETTER JHAN
    case 0x10c0: r=0x2d20; break; // GEORGIAN CAPITAL LETTER HAE
    case 0x10c1: r=0x2d21; break; // GEORGIAN CAPITAL LETTER HE
    case 0x10c2: r=0x2d22; break; // GEORGIAN CAPITAL LETTER HIE
    case 0x10c3: r=0x2d23; break; // GEORGIAN CAPITAL LETTER WE
    case 0x10c4: r=0x2d24; break; // GEORGIAN CAPITAL LETTER HAR
    case 0x10c5: r=0x2d25; break; // GEORGIAN CAPITAL LETTER HOE
    case 0x1e00: r=0x1e01; break; // LATIN CAPITAL LETTER A WITH RING BELOW
    case 0x1e02: r=0x1e03; break; // LATIN CAPITAL LETTER B WITH DOT ABOVE
    case 0x1e04: r=0x1e05; break; // LATIN CAPITAL LETTER B WITH DOT BELOW
    case 0x1e06: r=0x1e07; break; // LATIN CAPITAL LETTER B WITH LINE BELOW
    case 0x1e08: r=0x1e09; break; // LATIN CAPITAL LETTER C WITH CEDILLA AND ACUTE
    case 0x1e0a: r=0x1e0b; break; // LATIN CAPITAL LETTER D WITH DOT ABOVE
    case 0x1e0c: r=0x1e0d; break; // LATIN CAPITAL LETTER D WITH DOT BELOW
    case 0x1e0e: r=0x1e0f; break; // LATIN CAPITAL LETTER D WITH LINE BELOW
    case 0x1e10: r=0x1e11; break; // LATIN CAPITAL LETTER D WITH CEDILLA
    case 0x1e12: r=0x1e13; break; // LATIN CAPITAL LETTER D WITH CIRCUMFLEX BELOW
    case 0x1e14: r=0x1e15; break; // LATIN CAPITAL LETTER E WITH MACRON AND GRAVE
    case 0x1e16: r=0x1e17; break; // LATIN CAPITAL LETTER E WITH MACRON AND ACUTE
    case 0x1e18: r=0x1e19; break; // LATIN CAPITAL LETTER E WITH CIRCUMFLEX BELOW
    case 0x1e1a: r=0x1e1b; break; // LATIN CAPITAL LETTER E WITH TILDE BELOW
    case 0x1e1c: r=0x1e1d; break; // LATIN CAPITAL LETTER E WITH CEDILLA AND BREVE
    case 0x1e1e: r=0x1e1f; break; // LATIN CAPITAL LETTER F WITH DOT ABOVE
    case 0x1e20: r=0x1e21; break; // LATIN CAPITAL LETTER G WITH MACRON
    case 0x1e22: r=0x1e23; break; // LATIN CAPITAL LETTER H WITH DOT ABOVE
    case 0x1e24: r=0x1e25; break; // LATIN CAPITAL LETTER H WITH DOT BELOW
    case 0x1e26: r=0x1e27; break; // LATIN CAPITAL LETTER H WITH DIAERESIS
    case 0x1e28: r=0x1e29; break; // LATIN CAPITAL LETTER H WITH CEDILLA
    case 0x1e2a: r=0x1e2b; break; // LATIN CAPITAL LETTER H WITH BREVE BELOW
    case 0x1e2c: r=0x1e2d; break; // LATIN CAPITAL LETTER I WITH TILDE BELOW
    case 0x1e2e: r=0x1e2f; break; // LATIN CAPITAL LETTER I WITH DIAERESIS AND ACUTE
    case 0x1e30: r=0x1e31; break; // LATIN CAPITAL LETTER K WITH ACUTE
    case 0x1e32: r=0x1e33; break; // LATIN CAPITAL LETTER K WITH DOT BELOW
    case 0x1e34: r=0x1e35; break; // LATIN CAPITAL LETTER K WITH LINE BELOW
    case 0x1e36: r=0x1e37; break; // LATIN CAPITAL LETTER L WITH DOT BELOW
    case 0x1e38: r=0x1e39; break; // LATIN CAPITAL LETTER L WITH DOT BELOW AND MACRON
    case 0x1e3a: r=0x1e3b; break; // LATIN CAPITAL LETTER L WITH LINE BELOW
    case 0x1e3c: r=0x1e3d; break; // LATIN CAPITAL LETTER L WITH CIRCUMFLEX BELOW
    case 0x1e3e: r=0x1e3f; break; // LATIN CAPITAL LETTER M WITH ACUTE
    case 0x1e40: r=0x1e41; break; // LATIN CAPITAL LETTER M WITH DOT ABOVE
    case 0x1e42: r=0x1e43; break; // LATIN CAPITAL LETTER M WITH DOT BELOW
    case 0x1e44: r=0x1e45; break; // LATIN CAPITAL LETTER N WITH DOT ABOVE
    case 0x1e46: r=0x1e47; break; // LATIN CAPITAL LETTER N WITH DOT BELOW
    case 0x1e48: r=0x1e49; break; // LATIN CAPITAL LETTER N WITH LINE BELOW
    case 0x1e4a: r=0x1e4b; break; // LATIN CAPITAL LETTER N WITH CIRCUMFLEX BELOW
    case 0x1e4c: r=0x1e4d; break; // LATIN CAPITAL LETTER O WITH TILDE AND ACUTE
    case 0x1e4e: r=0x1e4f; break; // LATIN CAPITAL LETTER O WITH TILDE AND DIAERESIS
    case 0x1e50: r=0x1e51; break; // LATIN CAPITAL LETTER O WITH MACRON AND GRAVE
    case 0x1e52: r=0x1e53; break; // LATIN CAPITAL LETTER O WITH MACRON AND ACUTE
    case 0x1e54: r=0x1e55; break; // LATIN CAPITAL LETTER P WITH ACUTE
    case 0x1e56: r=0x1e57; break; // LATIN CAPITAL LETTER P WITH DOT ABOVE
    case 0x1e58: r=0x1e59; break; // LATIN CAPITAL LETTER R WITH DOT ABOVE
    case 0x1e5a: r=0x1e5b; break; // LATIN CAPITAL LETTER R WITH DOT BELOW
    case 0x1e5c: r=0x1e5d; break; // LATIN CAPITAL LETTER R WITH DOT BELOW AND MACRON
    case 0x1e5e: r=0x1e5f; break; // LATIN CAPITAL LETTER R WITH LINE BELOW
    case 0x1e60: r=0x1e61; break; // LATIN CAPITAL LETTER S WITH DOT ABOVE
    case 0x1e62: r=0x1e63; break; // LATIN CAPITAL LETTER S WITH DOT BELOW
    case 0x1e64: r=0x1e65; break; // LATIN CAPITAL LETTER S WITH ACUTE AND DOT ABOVE
    case 0x1e66: r=0x1e67; break; // LATIN CAPITAL LETTER S WITH CARON AND DOT ABOVE
    case 0x1e68: r=0x1e69; break; // LATIN CAPITAL LETTER S WITH DOT BELOW AND DOT ABOVE
    case 0x1e6a: r=0x1e6b; break; // LATIN CAPITAL LETTER T WITH DOT ABOVE
    case 0x1e6c: r=0x1e6d; break; // LATIN CAPITAL LETTER T WITH DOT BELOW
    case 0x1e6e: r=0x1e6f; break; // LATIN CAPITAL LETTER T WITH LINE BELOW
    case 0x1e70: r=0x1e71; break; // LATIN CAPITAL LETTER T WITH CIRCUMFLEX BELOW
    case 0x1e72: r=0x1e73; break; // LATIN CAPITAL LETTER U WITH DIAERESIS BELOW
    case 0x1e74: r=0x1e75; break; // LATIN CAPITAL LETTER U WITH TILDE BELOW
    case 0x1e76: r=0x1e77; break; // LATIN CAPITAL LETTER U WITH CIRCUMFLEX BELOW
    case 0x1e78: r=0x1e79; break; // LATIN CAPITAL LETTER U WITH TILDE AND ACUTE
    case 0x1e7a: r=0x1e7b; break; // LATIN CAPITAL LETTER U WITH MACRON AND DIAERESIS
    case 0x1e7c: r=0x1e7d; break; // LATIN CAPITAL LETTER V WITH TILDE
    case 0x1e7e: r=0x1e7f; break; // LATIN CAPITAL LETTER V WITH DOT BELOW
    case 0x1e80: r=0x1e81; break; // LATIN CAPITAL LETTER W WITH GRAVE
    case 0x1e82: r=0x1e83; break; // LATIN CAPITAL LETTER W WITH ACUTE
    case 0x1e84: r=0x1e85; break; // LATIN CAPITAL LETTER W WITH DIAERESIS
    case 0x1e86: r=0x1e87; break; // LATIN CAPITAL LETTER W WITH DOT ABOVE
    case 0x1e88: r=0x1e89; break; // LATIN CAPITAL LETTER W WITH DOT BELOW
    case 0x1e8a: r=0x1e8b; break; // LATIN CAPITAL LETTER X WITH DOT ABOVE
    case 0x1e8c: r=0x1e8d; break; // LATIN CAPITAL LETTER X WITH DIAERESIS
    case 0x1e8e: r=0x1e8f; break; // LATIN CAPITAL LETTER Y WITH DOT ABOVE
    case 0x1e90: r=0x1e91; break; // LATIN CAPITAL LETTER Z WITH CIRCUMFLEX
    case 0x1e92: r=0x1e93; break; // LATIN CAPITAL LETTER Z WITH DOT BELOW
    case 0x1e94: r=0x1e95; break; // LATIN CAPITAL LETTER Z WITH LINE BELOW
    case 0x1e9b: r=0x1e61; break; // LATIN SMALL LETTER LONG S WITH DOT ABOVE
    case 0x1ea0: r=0x1ea1; break; // LATIN CAPITAL LETTER A WITH DOT BELOW
    case 0x1ea2: r=0x1ea3; break; // LATIN CAPITAL LETTER A WITH HOOK ABOVE
    case 0x1ea4: r=0x1ea5; break; // LATIN CAPITAL LETTER A WITH CIRCUMFLEX AND ACUTE
    case 0x1ea6: r=0x1ea7; break; // LATIN CAPITAL LETTER A WITH CIRCUMFLEX AND GRAVE
    case 0x1ea8: r=0x1ea9; break; // LATIN CAPITAL LETTER A WITH CIRCUMFLEX AND HOOK ABOVE
    case 0x1eaa: r=0x1eab; break; // LATIN CAPITAL LETTER A WITH CIRCUMFLEX AND TILDE
    case 0x1eac: r=0x1ead; break; // LATIN CAPITAL LETTER A WITH CIRCUMFLEX AND DOT BELOW
    case 0x1eae: r=0x1eaf; break; // LATIN CAPITAL LETTER A WITH BREVE AND ACUTE
    case 0x1eb0: r=0x1eb1; break; // LATIN CAPITAL LETTER A WITH BREVE AND GRAVE
    case 0x1eb2: r=0x1eb3; break; // LATIN CAPITAL LETTER A WITH BREVE AND HOOK ABOVE
    case 0x1eb4: r=0x1eb5; break; // LATIN CAPITAL LETTER A WITH BREVE AND TILDE
    case 0x1eb6: r=0x1eb7; break; // LATIN CAPITAL LETTER A WITH BREVE AND DOT BELOW
    case 0x1eb8: r=0x1eb9; break; // LATIN CAPITAL LETTER E WITH DOT BELOW
    case 0x1eba: r=0x1ebb; break; // LATIN CAPITAL LETTER E WITH HOOK ABOVE
    case 0x1ebc: r=0x1ebd; break; // LATIN CAPITAL LETTER E WITH TILDE
    case 0x1ebe: r=0x1ebf; break; // LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND ACUTE
    case 0x1ec0: r=0x1ec1; break; // LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND GRAVE
    case 0x1ec2: r=0x1ec3; break; // LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND HOOK ABOVE
    case 0x1ec4: r=0x1ec5; break; // LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND TILDE
    case 0x1ec6: r=0x1ec7; break; // LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND DOT BELOW
    case 0x1ec8: r=0x1ec9; break; // LATIN CAPITAL LETTER I WITH HOOK ABOVE
    case 0x1eca: r=0x1ecb; break; // LATIN CAPITAL LETTER I WITH DOT BELOW
    case 0x1ecc: r=0x1ecd; break; // LATIN CAPITAL LETTER O WITH DOT BELOW
    case 0x1ece: r=0x1ecf; break; // LATIN CAPITAL LETTER O WITH HOOK ABOVE
    case 0x1ed0: r=0x1ed1; break; // LATIN CAPITAL LETTER O WITH CIRCUMFLEX AND ACUTE
    case 0x1ed2: r=0x1ed3; break; // LATIN CAPITAL LETTER O WITH CIRCUMFLEX AND GRAVE
    case 0x1ed4: r=0x1ed5; break; // LATIN CAPITAL LETTER O WITH CIRCUMFLEX AND HOOK ABOVE
    case 0x1ed6: r=0x1ed7; break; // LATIN CAPITAL LETTER O WITH CIRCUMFLEX AND TILDE
    case 0x1ed8: r=0x1ed9; break; // LATIN CAPITAL LETTER O WITH CIRCUMFLEX AND DOT BELOW
    case 0x1eda: r=0x1edb; break; // LATIN CAPITAL LETTER O WITH HORN AND ACUTE
    case 0x1edc: r=0x1edd; break; // LATIN CAPITAL LETTER O WITH HORN AND GRAVE
    case 0x1ede: r=0x1edf; break; // LATIN CAPITAL LETTER O WITH HORN AND HOOK ABOVE
    case 0x1ee0: r=0x1ee1; break; // LATIN CAPITAL LETTER O WITH HORN AND TILDE
    case 0x1ee2: r=0x1ee3; break; // LATIN CAPITAL LETTER O WITH HORN AND DOT BELOW
    case 0x1ee4: r=0x1ee5; break; // LATIN CAPITAL LETTER U WITH DOT BELOW
    case 0x1ee6: r=0x1ee7; break; // LATIN CAPITAL LETTER U WITH HOOK ABOVE
    case 0x1ee8: r=0x1ee9; break; // LATIN CAPITAL LETTER U WITH HORN AND ACUTE
    case 0x1eea: r=0x1eeb; break; // LATIN CAPITAL LETTER U WITH HORN AND GRAVE
    case 0x1eec: r=0x1eed; break; // LATIN CAPITAL LETTER U WITH HORN AND HOOK ABOVE
    case 0x1eee: r=0x1eef; break; // LATIN CAPITAL LETTER U WITH HORN AND TILDE
    case 0x1ef0: r=0x1ef1; break; // LATIN CAPITAL LETTER U WITH HORN AND DOT BELOW
    case 0x1ef2: r=0x1ef3; break; // LATIN CAPITAL LETTER Y WITH GRAVE
    case 0x1ef4: r=0x1ef5; break; // LATIN CAPITAL LETTER Y WITH DOT BELOW
    case 0x1ef6: r=0x1ef7; break; // LATIN CAPITAL LETTER Y WITH HOOK ABOVE
    case 0x1ef8: r=0x1ef9; break; // LATIN CAPITAL LETTER Y WITH TILDE
    case 0x1f08: r=0x1f00; break; // GREEK CAPITAL LETTER ALPHA WITH PSILI
    case 0x1f09: r=0x1f01; break; // GREEK CAPITAL LETTER ALPHA WITH DASIA
    case 0x1f0a: r=0x1f02; break; // GREEK CAPITAL LETTER ALPHA WITH PSILI AND VARIA
    case 0x1f0b: r=0x1f03; break; // GREEK CAPITAL LETTER ALPHA WITH DASIA AND VARIA
    case 0x1f0c: r=0x1f04; break; // GREEK CAPITAL LETTER ALPHA WITH PSILI AND OXIA
    case 0x1f0d: r=0x1f05; break; // GREEK CAPITAL LETTER ALPHA WITH DASIA AND OXIA
    case 0x1f0e: r=0x1f06; break; // GREEK CAPITAL LETTER ALPHA WITH PSILI AND PERISPOMENI
    case 0x1f0f: r=0x1f07; break; // GREEK CAPITAL LETTER ALPHA WITH DASIA AND PERISPOMENI
    case 0x1f18: r=0x1f10; break; // GREEK CAPITAL LETTER EPSILON WITH PSILI
    case 0x1f19: r=0x1f11; break; // GREEK CAPITAL LETTER EPSILON WITH DASIA
    case 0x1f1a: r=0x1f12; break; // GREEK CAPITAL LETTER EPSILON WITH PSILI AND VARIA
    case 0x1f1b: r=0x1f13; break; // GREEK CAPITAL LETTER EPSILON WITH DASIA AND VARIA
    case 0x1f1c: r=0x1f14; break; // GREEK CAPITAL LETTER EPSILON WITH PSILI AND OXIA
    case 0x1f1d: r=0x1f15; break; // GREEK CAPITAL LETTER EPSILON WITH DASIA AND OXIA
    case 0x1f28: r=0x1f20; break; // GREEK CAPITAL LETTER ETA WITH PSILI
    case 0x1f29: r=0x1f21; break; // GREEK CAPITAL LETTER ETA WITH DASIA
    case 0x1f2a: r=0x1f22; break; // GREEK CAPITAL LETTER ETA WITH PSILI AND VARIA
    case 0x1f2b: r=0x1f23; break; // GREEK CAPITAL LETTER ETA WITH DASIA AND VARIA
    case 0x1f2c: r=0x1f24; break; // GREEK CAPITAL LETTER ETA WITH PSILI AND OXIA
    case 0x1f2d: r=0x1f25; break; // GREEK CAPITAL LETTER ETA WITH DASIA AND OXIA
    case 0x1f2e: r=0x1f26; break; // GREEK CAPITAL LETTER ETA WITH PSILI AND PERISPOMENI
    case 0x1f2f: r=0x1f27; break; // GREEK CAPITAL LETTER ETA WITH DASIA AND PERISPOMENI
    case 0x1f38: r=0x1f30; break; // GREEK CAPITAL LETTER IOTA WITH PSILI
    case 0x1f39: r=0x1f31; break; // GREEK CAPITAL LETTER IOTA WITH DASIA
    case 0x1f3a: r=0x1f32; break; // GREEK CAPITAL LETTER IOTA WITH PSILI AND VARIA
    case 0x1f3b: r=0x1f33; break; // GREEK CAPITAL LETTER IOTA WITH DASIA AND VARIA
    case 0x1f3c: r=0x1f34; break; // GREEK CAPITAL LETTER IOTA WITH PSILI AND OXIA
    case 0x1f3d: r=0x1f35; break; // GREEK CAPITAL LETTER IOTA WITH DASIA AND OXIA
    case 0x1f3e: r=0x1f36; break; // GREEK CAPITAL LETTER IOTA WITH PSILI AND PERISPOMENI
    case 0x1f3f: r=0x1f37; break; // GREEK CAPITAL LETTER IOTA WITH DASIA AND PERISPOMENI
    case 0x1f48: r=0x1f40; break; // GREEK CAPITAL LETTER OMICRON WITH PSILI
    case 0x1f49: r=0x1f41; break; // GREEK CAPITAL LETTER OMICRON WITH DASIA
    case 0x1f4a: r=0x1f42; break; // GREEK CAPITAL LETTER OMICRON WITH PSILI AND VARIA
    case 0x1f4b: r=0x1f43; break; // GREEK CAPITAL LETTER OMICRON WITH DASIA AND VARIA
    case 0x1f4c: r=0x1f44; break; // GREEK CAPITAL LETTER OMICRON WITH PSILI AND OXIA
    case 0x1f4d: r=0x1f45; break; // GREEK CAPITAL LETTER OMICRON WITH DASIA AND OXIA
    case 0x1f59: r=0x1f51; break; // GREEK CAPITAL LETTER UPSILON WITH DASIA
    case 0x1f5b: r=0x1f53; break; // GREEK CAPITAL LETTER UPSILON WITH DASIA AND VARIA
    case 0x1f5d: r=0x1f55; break; // GREEK CAPITAL LETTER UPSILON WITH DASIA AND OXIA
    case 0x1f5f: r=0x1f57; break; // GREEK CAPITAL LETTER UPSILON WITH DASIA AND PERISPOMENI
    case 0x1f68: r=0x1f60; break; // GREEK CAPITAL LETTER OMEGA WITH PSILI
    case 0x1f69: r=0x1f61; break; // GREEK CAPITAL LETTER OMEGA WITH DASIA
    case 0x1f6a: r=0x1f62; break; // GREEK CAPITAL LETTER OMEGA WITH PSILI AND VARIA
    case 0x1f6b: r=0x1f63; break; // GREEK CAPITAL LETTER OMEGA WITH DASIA AND VARIA
    case 0x1f6c: r=0x1f64; break; // GREEK CAPITAL LETTER OMEGA WITH PSILI AND OXIA
    case 0x1f6d: r=0x1f65; break; // GREEK CAPITAL LETTER OMEGA WITH DASIA AND OXIA
    case 0x1f6e: r=0x1f66; break; // GREEK CAPITAL LETTER OMEGA WITH PSILI AND PERISPOMENI
    case 0x1f6f: r=0x1f67; break; // GREEK CAPITAL LETTER OMEGA WITH DASIA AND PERISPOMENI
    case 0x1f88: r=0x1f80; break; // GREEK CAPITAL LETTER ALPHA WITH PSILI AND PROSGEGRAMMENI
    case 0x1f89: r=0x1f81; break; // GREEK CAPITAL LETTER ALPHA WITH DASIA AND PROSGEGRAMMENI
    case 0x1f8a: r=0x1f82; break; // GREEK CAPITAL LETTER ALPHA WITH PSILI AND VARIA AND PROSGEGRAMMENI
    case 0x1f8b: r=0x1f83; break; // GREEK CAPITAL LETTER ALPHA WITH DASIA AND VARIA AND PROSGEGRAMMENI
    case 0x1f8c: r=0x1f84; break; // GREEK CAPITAL LETTER ALPHA WITH PSILI AND OXIA AND PROSGEGRAMMENI
    case 0x1f8d: r=0x1f85; break; // GREEK CAPITAL LETTER ALPHA WITH DASIA AND OXIA AND PROSGEGRAMMENI
    case 0x1f8e: r=0x1f86; break; // GREEK CAPITAL LETTER ALPHA WITH PSILI AND PERISPOMENI AND PROSGEGRAMMENI
    case 0x1f8f: r=0x1f87; break; // GREEK CAPITAL LETTER ALPHA WITH DASIA AND PERISPOMENI AND PROSGEGRAMMENI
    case 0x1f98: r=0x1f90; break; // GREEK CAPITAL LETTER ETA WITH PSILI AND PROSGEGRAMMENI
    case 0x1f99: r=0x1f91; break; // GREEK CAPITAL LETTER ETA WITH DASIA AND PROSGEGRAMMENI
    case 0x1f9a: r=0x1f92; break; // GREEK CAPITAL LETTER ETA WITH PSILI AND VARIA AND PROSGEGRAMMENI
    case 0x1f9b: r=0x1f93; break; // GREEK CAPITAL LETTER ETA WITH DASIA AND VARIA AND PROSGEGRAMMENI
    case 0x1f9c: r=0x1f94; break; // GREEK CAPITAL LETTER ETA WITH PSILI AND OXIA AND PROSGEGRAMMENI
    case 0x1f9d: r=0x1f95; break; // GREEK CAPITAL LETTER ETA WITH DASIA AND OXIA AND PROSGEGRAMMENI
    case 0x1f9e: r=0x1f96; break; // GREEK CAPITAL LETTER ETA WITH PSILI AND PERISPOMENI AND PROSGEGRAMMENI
    case 0x1f9f: r=0x1f97; break; // GREEK CAPITAL LETTER ETA WITH DASIA AND PERISPOMENI AND PROSGEGRAMMENI
    case 0x1fa8: r=0x1fa0; break; // GREEK CAPITAL LETTER OMEGA WITH PSILI AND PROSGEGRAMMENI
    case 0x1fa9: r=0x1fa1; break; // GREEK CAPITAL LETTER OMEGA WITH DASIA AND PROSGEGRAMMENI
    case 0x1faa: r=0x1fa2; break; // GREEK CAPITAL LETTER OMEGA WITH PSILI AND VARIA AND PROSGEGRAMMENI
    case 0x1fab: r=0x1fa3; break; // GREEK CAPITAL LETTER OMEGA WITH DASIA AND VARIA AND PROSGEGRAMMENI
    case 0x1fac: r=0x1fa4; break; // GREEK CAPITAL LETTER OMEGA WITH PSILI AND OXIA AND PROSGEGRAMMENI
    case 0x1fad: r=0x1fa5; break; // GREEK CAPITAL LETTER OMEGA WITH DASIA AND OXIA AND PROSGEGRAMMENI
    case 0x1fae: r=0x1fa6; break; // GREEK CAPITAL LETTER OMEGA WITH PSILI AND PERISPOMENI AND PROSGEGRAMMENI
    case 0x1faf: r=0x1fa7; break; // GREEK CAPITAL LETTER OMEGA WITH DASIA AND PERISPOMENI AND PROSGEGRAMMENI
    case 0x1fb8: r=0x1fb0; break; // GREEK CAPITAL LETTER ALPHA WITH VRACHY
    case 0x1fb9: r=0x1fb1; break; // GREEK CAPITAL LETTER ALPHA WITH MACRON
    case 0x1fba: r=0x1f70; break; // GREEK CAPITAL LETTER ALPHA WITH VARIA
    case 0x1fbb: r=0x1f71; break; // GREEK CAPITAL LETTER ALPHA WITH OXIA
    case 0x1fbc: r=0x1fb3; break; // GREEK CAPITAL LETTER ALPHA WITH PROSGEGRAMMENI
    case 0x1fbe: r=0x03b9; break; // GREEK PROSGEGRAMMENI
    case 0x1fc8: r=0x1f72; break; // GREEK CAPITAL LETTER EPSILON WITH VARIA
    case 0x1fc9: r=0x1f73; break; // GREEK CAPITAL LETTER EPSILON WITH OXIA
    case 0x1fca: r=0x1f74; break; // GREEK CAPITAL LETTER ETA WITH VARIA
    case 0x1fcb: r=0x1f75; break; // GREEK CAPITAL LETTER ETA WITH OXIA
    case 0x1fcc: r=0x1fc3; break; // GREEK CAPITAL LETTER ETA WITH PROSGEGRAMMENI
    case 0x1fd8: r=0x1fd0; break; // GREEK CAPITAL LETTER IOTA WITH VRACHY
    case 0x1fd9: r=0x1fd1; break; // GREEK CAPITAL LETTER IOTA WITH MACRON
    case 0x1fda: r=0x1f76; break; // GREEK CAPITAL LETTER IOTA WITH VARIA
    case 0x1fdb: r=0x1f77; break; // GREEK CAPITAL LETTER IOTA WITH OXIA
    case 0x1fe8: r=0x1fe0; break; // GREEK CAPITAL LETTER UPSILON WITH VRACHY
    case 0x1fe9: r=0x1fe1; break; // GREEK CAPITAL LETTER UPSILON WITH MACRON
    case 0x1fea: r=0x1f7a; break; // GREEK CAPITAL LETTER UPSILON WITH VARIA
    case 0x1feb: r=0x1f7b; break; // GREEK CAPITAL LETTER UPSILON WITH OXIA
    case 0x1fec: r=0x1fe5; break; // GREEK CAPITAL LETTER RHO WITH DASIA
    case 0x1ff8: r=0x1f78; break; // GREEK CAPITAL LETTER OMICRON WITH VARIA
    case 0x1ff9: r=0x1f79; break; // GREEK CAPITAL LETTER OMICRON WITH OXIA
    case 0x1ffa: r=0x1f7c; break; // GREEK CAPITAL LETTER OMEGA WITH VARIA
    case 0x1ffb: r=0x1f7d; break; // GREEK CAPITAL LETTER OMEGA WITH OXIA
    case 0x1ffc: r=0x1ff3; break; // GREEK CAPITAL LETTER OMEGA WITH PROSGEGRAMMENI
    case 0x2126: r=0x03c9; break; // OHM SIGN
    case 0x212a: r=0x006b; break; // KELVIN SIGN
    case 0x212b: r=0x00e5; break; // ANGSTROM SIGN
    case 0x2160: r=0x2170; break; // ROMAN NUMERAL ONE
    case 0x2161: r=0x2171; break; // ROMAN NUMERAL TWO
    case 0x2162: r=0x2172; break; // ROMAN NUMERAL THREE
    case 0x2163: r=0x2173; break; // ROMAN NUMERAL FOUR
    case 0x2164: r=0x2174; break; // ROMAN NUMERAL FIVE
    case 0x2165: r=0x2175; break; // ROMAN NUMERAL SIX
    case 0x2166: r=0x2176; break; // ROMAN NUMERAL SEVEN
    case 0x2167: r=0x2177; break; // ROMAN NUMERAL EIGHT
    case 0x2168: r=0x2178; break; // ROMAN NUMERAL NINE
    case 0x2169: r=0x2179; break; // ROMAN NUMERAL TEN
    case 0x216a: r=0x217a; break; // ROMAN NUMERAL ELEVEN
    case 0x216b: r=0x217b; break; // ROMAN NUMERAL TWELVE
    case 0x216c: r=0x217c; break; // ROMAN NUMERAL FIFTY
    case 0x216d: r=0x217d; break; // ROMAN NUMERAL ONE HUNDRED
    case 0x216e: r=0x217e; break; // ROMAN NUMERAL FIVE HUNDRED
    case 0x216f: r=0x217f; break; // ROMAN NUMERAL ONE THOUSAND
    case 0x24b6: r=0x24d0; break; // CIRCLED LATIN CAPITAL LETTER A
    case 0x24b7: r=0x24d1; break; // CIRCLED LATIN CAPITAL LETTER B
    case 0x24b8: r=0x24d2; break; // CIRCLED LATIN CAPITAL LETTER C
    case 0x24b9: r=0x24d3; break; // CIRCLED LATIN CAPITAL LETTER D
    case 0x24ba: r=0x24d4; break; // CIRCLED LATIN CAPITAL LETTER E
    case 0x24bb: r=0x24d5; break; // CIRCLED LATIN CAPITAL LETTER F
    case 0x24bc: r=0x24d6; break; // CIRCLED LATIN CAPITAL LETTER G
    case 0x24bd: r=0x24d7; break; // CIRCLED LATIN CAPITAL LETTER H
    case 0x24be: r=0x24d8; break; // CIRCLED LATIN CAPITAL LETTER I
    case 0x24bf: r=0x24d9; break; // CIRCLED LATIN CAPITAL LETTER J
    case 0x24c0: r=0x24da; break; // CIRCLED LATIN CAPITAL LETTER K
    case 0x24c1: r=0x24db; break; // CIRCLED LATIN CAPITAL LETTER L
    case 0x24c2: r=0x24dc; break; // CIRCLED LATIN CAPITAL LETTER M
    case 0x24c3: r=0x24dd; break; // CIRCLED LATIN CAPITAL LETTER N
    case 0x24c4: r=0x24de; break; // CIRCLED LATIN CAPITAL LETTER O
    case 0x24c5: r=0x24df; break; // CIRCLED LATIN CAPITAL LETTER P
    case 0x24c6: r=0x24e0; break; // CIRCLED LATIN CAPITAL LETTER Q
    case 0x24c7: r=0x24e1; break; // CIRCLED LATIN CAPITAL LETTER R
    case 0x24c8: r=0x24e2; break; // CIRCLED LATIN CAPITAL LETTER S
    case 0x24c9: r=0x24e3; break; // CIRCLED LATIN CAPITAL LETTER T
    case 0x24ca: r=0x24e4; break; // CIRCLED LATIN CAPITAL LETTER U
    case 0x24cb: r=0x24e5; break; // CIRCLED LATIN CAPITAL LETTER V
    case 0x24cc: r=0x24e6; break; // CIRCLED LATIN CAPITAL LETTER W
    case 0x24cd: r=0x24e7; break; // CIRCLED LATIN CAPITAL LETTER X
    case 0x24ce: r=0x24e8; break; // CIRCLED LATIN CAPITAL LETTER Y
    case 0x24cf: r=0x24e9; break; // CIRCLED LATIN CAPITAL LETTER Z
    case 0x2c00: r=0x2c30; break; // GLAGOLITIC CAPITAL LETTER AZU
    case 0x2c01: r=0x2c31; break; // GLAGOLITIC CAPITAL LETTER BUKY
    case 0x2c02: r=0x2c32; break; // GLAGOLITIC CAPITAL LETTER VEDE
    case 0x2c03: r=0x2c33; break; // GLAGOLITIC CAPITAL LETTER GLAGOLI
    case 0x2c04: r=0x2c34; break; // GLAGOLITIC CAPITAL LETTER DOBRO
    case 0x2c05: r=0x2c35; break; // GLAGOLITIC CAPITAL LETTER YESTU
    case 0x2c06: r=0x2c36; break; // GLAGOLITIC CAPITAL LETTER ZHIVETE
    case 0x2c07: r=0x2c37; break; // GLAGOLITIC CAPITAL LETTER DZELO
    case 0x2c08: r=0x2c38; break; // GLAGOLITIC CAPITAL LETTER ZEMLJA
    case 0x2c09: r=0x2c39; break; // GLAGOLITIC CAPITAL LETTER IZHE
    case 0x2c0a: r=0x2c3a; break; // GLAGOLITIC CAPITAL LETTER INITIAL IZHE
    case 0x2c0b: r=0x2c3b; break; // GLAGOLITIC CAPITAL LETTER I
    case 0x2c0c: r=0x2c3c; break; // GLAGOLITIC CAPITAL LETTER DJERVI
    case 0x2c0d: r=0x2c3d; break; // GLAGOLITIC CAPITAL LETTER KAKO
    case 0x2c0e: r=0x2c3e; break; // GLAGOLITIC CAPITAL LETTER LJUDIJE
    case 0x2c0f: r=0x2c3f; break; // GLAGOLITIC CAPITAL LETTER MYSLITE
    case 0x2c10: r=0x2c40; break; // GLAGOLITIC CAPITAL LETTER NASHI
    case 0x2c11: r=0x2c41; break; // GLAGOLITIC CAPITAL LETTER ONU
    case 0x2c12: r=0x2c42; break; // GLAGOLITIC CAPITAL LETTER POKOJI
    case 0x2c13: r=0x2c43; break; // GLAGOLITIC CAPITAL LETTER RITSI
    case 0x2c14: r=0x2c44; break; // GLAGOLITIC CAPITAL LETTER SLOVO
    case 0x2c15: r=0x2c45; break; // GLAGOLITIC CAPITAL LETTER TVRIDO
    case 0x2c16: r=0x2c46; break; // GLAGOLITIC CAPITAL LETTER UKU
    case 0x2c17: r=0x2c47; break; // GLAGOLITIC CAPITAL LETTER FRITU
    case 0x2c18: r=0x2c48; break; // GLAGOLITIC CAPITAL LETTER HERU
    case 0x2c19: r=0x2c49; break; // GLAGOLITIC CAPITAL LETTER OTU
    case 0x2c1a: r=0x2c4a; break; // GLAGOLITIC CAPITAL LETTER PE
    case 0x2c1b: r=0x2c4b; break; // GLAGOLITIC CAPITAL LETTER SHTA
    case 0x2c1c: r=0x2c4c; break; // GLAGOLITIC CAPITAL LETTER TSI
    case 0x2c1d: r=0x2c4d; break; // GLAGOLITIC CAPITAL LETTER CHRIVI
    case 0x2c1e: r=0x2c4e; break; // GLAGOLITIC CAPITAL LETTER SHA
    case 0x2c1f: r=0x2c4f; break; // GLAGOLITIC CAPITAL LETTER YERU
    case 0x2c20: r=0x2c50; break; // GLAGOLITIC CAPITAL LETTER YERI
    case 0x2c21: r=0x2c51; break; // GLAGOLITIC CAPITAL LETTER YATI
    case 0x2c22: r=0x2c52; break; // GLAGOLITIC CAPITAL LETTER SPIDERY HA
    case 0x2c23: r=0x2c53; break; // GLAGOLITIC CAPITAL LETTER YU
    case 0x2c24: r=0x2c54; break; // GLAGOLITIC CAPITAL LETTER SMALL YUS
    case 0x2c25: r=0x2c55; break; // GLAGOLITIC CAPITAL LETTER SMALL YUS WITH TAIL
    case 0x2c26: r=0x2c56; break; // GLAGOLITIC CAPITAL LETTER YO
    case 0x2c27: r=0x2c57; break; // GLAGOLITIC CAPITAL LETTER IOTATED SMALL YUS
    case 0x2c28: r=0x2c58; break; // GLAGOLITIC CAPITAL LETTER BIG YUS
    case 0x2c29: r=0x2c59; break; // GLAGOLITIC CAPITAL LETTER IOTATED BIG YUS
    case 0x2c2a: r=0x2c5a; break; // GLAGOLITIC CAPITAL LETTER FITA
    case 0x2c2b: r=0x2c5b; break; // GLAGOLITIC CAPITAL LETTER IZHITSA
    case 0x2c2c: r=0x2c5c; break; // GLAGOLITIC CAPITAL LETTER SHTAPIC
    case 0x2c2d: r=0x2c5d; break; // GLAGOLITIC CAPITAL LETTER TROKUTASTI A
    case 0x2c2e: r=0x2c5e; break; // GLAGOLITIC CAPITAL LETTER LATINATE MYSLITE
    case 0x2c80: r=0x2c81; break; // COPTIC CAPITAL LETTER ALFA
    case 0x2c82: r=0x2c83; break; // COPTIC CAPITAL LETTER VIDA
    case 0x2c84: r=0x2c85; break; // COPTIC CAPITAL LETTER GAMMA
    case 0x2c86: r=0x2c87; break; // COPTIC CAPITAL LETTER DALDA
    case 0x2c88: r=0x2c89; break; // COPTIC CAPITAL LETTER EIE
    case 0x2c8a: r=0x2c8b; break; // COPTIC CAPITAL LETTER SOU
    case 0x2c8c: r=0x2c8d; break; // COPTIC CAPITAL LETTER ZATA
    case 0x2c8e: r=0x2c8f; break; // COPTIC CAPITAL LETTER HATE
    case 0x2c90: r=0x2c91; break; // COPTIC CAPITAL LETTER THETHE
    case 0x2c92: r=0x2c93; break; // COPTIC CAPITAL LETTER IAUDA
    case 0x2c94: r=0x2c95; break; // COPTIC CAPITAL LETTER KAPA
    case 0x2c96: r=0x2c97; break; // COPTIC CAPITAL LETTER LAULA
    case 0x2c98: r=0x2c99; break; // COPTIC CAPITAL LETTER MI
    case 0x2c9a: r=0x2c9b; break; // COPTIC CAPITAL LETTER NI
    case 0x2c9c: r=0x2c9d; break; // COPTIC CAPITAL LETTER KSI
    case 0x2c9e: r=0x2c9f; break; // COPTIC CAPITAL LETTER O
    case 0x2ca0: r=0x2ca1; break; // COPTIC CAPITAL LETTER PI
    case 0x2ca2: r=0x2ca3; break; // COPTIC CAPITAL LETTER RO
    case 0x2ca4: r=0x2ca5; break; // COPTIC CAPITAL LETTER SIMA
    case 0x2ca6: r=0x2ca7; break; // COPTIC CAPITAL LETTER TAU
    case 0x2ca8: r=0x2ca9; break; // COPTIC CAPITAL LETTER UA
    case 0x2caa: r=0x2cab; break; // COPTIC CAPITAL LETTER FI
    case 0x2cac: r=0x2cad; break; // COPTIC CAPITAL LETTER KHI
    case 0x2cae: r=0x2caf; break; // COPTIC CAPITAL LETTER PSI
    case 0x2cb0: r=0x2cb1; break; // COPTIC CAPITAL LETTER OOU
    case 0x2cb2: r=0x2cb3; break; // COPTIC CAPITAL LETTER DIALECT-P ALEF
    case 0x2cb4: r=0x2cb5; break; // COPTIC CAPITAL LETTER OLD COPTIC AIN
    case 0x2cb6: r=0x2cb7; break; // COPTIC CAPITAL LETTER CRYPTOGRAMMIC EIE
    case 0x2cb8: r=0x2cb9; break; // COPTIC CAPITAL LETTER DIALECT-P KAPA
    case 0x2cba: r=0x2cbb; break; // COPTIC CAPITAL LETTER DIALECT-P NI
    case 0x2cbc: r=0x2cbd; break; // COPTIC CAPITAL LETTER CRYPTOGRAMMIC NI
    case 0x2cbe: r=0x2cbf; break; // COPTIC CAPITAL LETTER OLD COPTIC OOU
    case 0x2cc0: r=0x2cc1; break; // COPTIC CAPITAL LETTER SAMPI
    case 0x2cc2: r=0x2cc3; break; // COPTIC CAPITAL LETTER CROSSED SHEI
    case 0x2cc4: r=0x2cc5; break; // COPTIC CAPITAL LETTER OLD COPTIC SHEI
    case 0x2cc6: r=0x2cc7; break; // COPTIC CAPITAL LETTER OLD COPTIC ESH
    case 0x2cc8: r=0x2cc9; break; // COPTIC CAPITAL LETTER AKHMIMIC KHEI
    case 0x2cca: r=0x2ccb; break; // COPTIC CAPITAL LETTER DIALECT-P HORI
    case 0x2ccc: r=0x2ccd; break; // COPTIC CAPITAL LETTER OLD COPTIC HORI
    case 0x2cce: r=0x2ccf; break; // COPTIC CAPITAL LETTER OLD COPTIC HA
    case 0x2cd0: r=0x2cd1; break; // COPTIC CAPITAL LETTER L-SHAPED HA
    case 0x2cd2: r=0x2cd3; break; // COPTIC CAPITAL LETTER OLD COPTIC HEI
    case 0x2cd4: r=0x2cd5; break; // COPTIC CAPITAL LETTER OLD COPTIC HAT
    case 0x2cd6: r=0x2cd7; break; // COPTIC CAPITAL LETTER OLD COPTIC GANGIA
    case 0x2cd8: r=0x2cd9; break; // COPTIC CAPITAL LETTER OLD COPTIC DJA
    case 0x2cda: r=0x2cdb; break; // COPTIC CAPITAL LETTER OLD COPTIC SHIMA
    case 0x2cdc: r=0x2cdd; break; // COPTIC CAPITAL LETTER OLD NUBIAN SHIMA
    case 0x2cde: r=0x2cdf; break; // COPTIC CAPITAL LETTER OLD NUBIAN NGI
    case 0x2ce0: r=0x2ce1; break; // COPTIC CAPITAL LETTER OLD NUBIAN NYI
    case 0x2ce2: r=0x2ce3; break; // COPTIC CAPITAL LETTER OLD NUBIAN WAU
      /* end of autogenerated code */
    default: r=c; break;
  }
  return r;
}

/* end of Sebastian Nagel */


