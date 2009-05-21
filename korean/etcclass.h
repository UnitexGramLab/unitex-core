 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#ifndef ETCCLASS_KR
#define ETCCLASS_KR

#include "etc.h"
template <class T>
class link_tables {
public:
	int total;
	int max_alloc_size;	

	struct tables_link {
		T *table;
		struct tables_link *next;
	};
	int curOffset;
	struct tables_link *Lhead,*Ltail;
	T *tableLoaded;		// use at read
	link_tables(){
		total = 0;
		Lhead = 0;
		Ltail = 0;
		curOffset = 0;
		tableLoaded = 0;
	};
	~link_tables()
	{
		while(Lhead){
			Ltail = Lhead;
			Lhead = Lhead->next;
			delete [] Ltail->table;
			delete Ltail;
		}
		if(tableLoaded) delete [] tableLoaded;
	}
	void setMaxSz(int sz){ max_alloc_size = sz;}

	void insertAtTable(T i)
	{
		if(!Lhead){
			Lhead = Ltail = new struct tables_link;
			Ltail->table = new T [max_alloc_size];
			Ltail->next = 0;
		}
		if(curOffset >= max_alloc_size){
			total += max_alloc_size;
			Ltail->next = new struct tables_link;
			Ltail= Ltail->next;
			Ltail->table = new T [max_alloc_size];
			Ltail->next = 0;
			curOffset = 0;
		}
		Ltail->table[curOffset++] = i;
	}

	void
	writeTableAtFile(U_FILE *fout)
	{
		struct tables_link *wp = Lhead;
		total = 0;
		while(wp){
			if(wp == Ltail){
				fwrite(wp->table,curOffset,4,fout);
			} else {
				fwrite(wp->table,max_alloc_size,4,fout);
			}
			wp=wp->next;
		}
	}
	void
	readTablesFromFile(U_FILE *fin)
	{
		tableLoaded = new T[total];
		if(!tableLoaded) fatal_error("mem alloc fail for table\n");
		if(!fread(tableLoaded,sizeof(T)*total,1,fin))
			fatal_error("table read error\n");
	}

};

class nameOfControlChars {
	class arbre_string0 nameOfCtlChars;
public:
    static const char *ctl_Bchar_name_string[0x21];
    unsigned short *ctl_Uchar_name_string[0x21];
	
	nameOfControlChars()
	{
		for(int i = 0; i < 0x21;i++){
		    ctl_Uchar_name_string[i] = (unsigned short *)malloc(
                  (strlen(ctl_Bchar_name_string[i])+1)*2);
		    u_strcpy(ctl_Uchar_name_string[i],ctl_Bchar_name_string[i]);
			nameOfCtlChars.put(ctl_Uchar_name_string[i]);
		}
	};
	~nameOfControlChars(){
	   for(int i = 0; i < 0x21;i++){
     		  free(ctl_Uchar_name_string[i]);
		}
    };
	int
	checkName(unsigned short *a)
	{
		return(nameOfCtlChars.check(a));
	};
};
const char*nameOfControlChars::ctl_Bchar_name_string[0x21]
= {
"<NULL>",	"<STX>",	"<SOT>",	"<ETX>",
"<EOT>",	"<ENQ>",	"<ACK>",	"<BEL>",
"<BACK>",	"<HTAB>",	"<HFEED>",	"<VFEED>",
"<FF>",	"<CR>",	"<S0>",	"<S1>",
"<DLE>",	"<DC1>",	"<DC2>",	"<DC3>",
"<DC4>",	"<NACK>",	"<SYNC>",	"<ETAB>",
"<CAN>",	"<EM>",	"<SUB>",	"<ESC>",
"<FS>",	"<GS>",	"<RS>",	"<US>",
"<SP>"
};
#endif
