 /*
  * Unitex
  *
  * Copyright (C) 2001-2006 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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
//
//	get the facon of the data handling : little
//
#ifndef PRO_TYPE_H
#define PRO_TYPE_H
#define PRO_TYPE_BIG    	0xfeff
#define PRO_TYPE_LITTLE	    0xfffe

typedef int (*func_SF)(unichar arg0,FILE *f);
typedef int (*func_LF)(unsigned int arg0,FILE *f);
typedef unichar (*Sfunc_F)(FILE *f);
typedef unsigned int (*Lfunc_F)(FILE *f);
typedef unichar (*Sfunc_B)(unsigned char *arg2);
typedef unsigned int (*Lfunc_B)(unsigned char *arg2);
extern unsigned int  LEintRead4(unsigned char *a);
extern 	unsigned int LEintRead3(unsigned char *a);
extern	unichar      LEintRead2(unsigned char *a);
extern 	int          LEoutbytes2(unichar v,FILE *f);
extern 	int          LEoutbytes3(unsigned int v,FILE *f);
extern 	int          LEoutbytes4(unsigned int v,FILE *f);
extern	unichar      LEinbytes2(FILE *f);
extern	unsigned int LEinbytes3(FILE *f);
extern	unsigned int LEinbytes4(FILE *f);
extern	unsigned int GEintRead4(unsigned char *a);
extern	unsigned int GEintRead3(unsigned char *a);
extern	unichar      GEintRead2(unsigned char *a);
extern	int          GEoutbytes2(unichar v,FILE *f);
extern	int          GEoutbytes3(unsigned int v,FILE *f);
extern	int          GEoutbytes4(unsigned int v,FILE *f);
extern	unichar      GEinbytes2(FILE *f);
extern	unsigned int GEinbytes3(FILE *f);
extern	unsigned int GEinbytes4(FILE *f);

class pro_associ {
	
public:
	unichar uniMark;
//	unichar proType;		// 1 : big endian
						// 0 : little endian
	unichar swapUniMark;
	func_SF	outbytes2;
	func_LF	outbytes3,outbytes4;
	Sfunc_F inbytes2;
	Lfunc_F inbytes3,inbytes4;
	Sfunc_B	intRead2;
	Lfunc_B	intRead3,intRead4;

	pro_associ(){
		uniMark = 0xfeff; // 0xfeff
                swapUniMark= 0xfffe;
//		unsigned char *p = (unsigned char *)&proType;
		
//		proType = PRO_TYPE_BIG;// unicode mark , big endian;
//		if((*p == 0xff) && (*(p+1)== 0xfe )){		// little endian
//			proType = PRO_TYPE_LITTLE;
			outbytes4 =LEoutbytes4;
			outbytes3=LEoutbytes3;
			outbytes2=LEoutbytes2;
			inbytes4=LEinbytes4;
			inbytes3=LEinbytes3;
			inbytes2=LEinbytes2;
			intRead4=LEintRead4;
			intRead3=LEintRead3;
			intRead2=LEintRead2;
#ifdef DDDDDDDDDD
		} else {
		    
			outbytes4 =GEoutbytes4;
			outbytes3=GEoutbytes3;
			outbytes2=GEoutbytes2;
			inbytes4=GEinbytes4;
			inbytes3=GEinbytes3;
			inbytes2=GEinbytes2;
			intRead4=GEintRead4;
			intRead3=GEintRead3;
			intRead2=GEintRead2;
		}
#endif // DDDDDDDDD
//		swapUniMark = swapShort(proType);
	};
	~pro_associ(){};
	unichar swapShort(unichar a)
	{
		return((unichar)(
			((a & 0xff00) >> 8) | ((a & 0xff) << 8))
			);
	}
	unsigned int swapInt(unsigned int a)
	{
		return( (unsigned int)(
			((a & 0xff000000) >> 24)|
			((a & 0x00ff0000) >> 8) |
			((a & 0x0000ff00) << 8) |
			((a & 0x000000ff) << 24)
		));
	}	
};
extern class pro_associ proType;
#define outbytes4	proType.outbytes4
#define outbytes3	proType.outbytes3
#define outbytes2	proType.outbytes2
#define inbytes4	proType.inbytes4
#define inbytes3	proType.inbytes3
#define inbytes2    proType.inbytes2
#define intRead4	proType.intRead4
#define intRead3	proType.intRead3
#define intRead2	proType.intRead2
#endif	// PRO_TYPE_H

