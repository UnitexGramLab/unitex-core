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

#ifndef KR_ETC_H
#define KR_ETC_H

extern int uniToInt(unichar *);
extern void fopenErrMessage(const char *m);
extern void freadError(const char *m);
extern int utoi(unichar *ws);
extern char *getExtension(char *a);
extern int getStringTableFile(char *,unichar *&,unichar **&);
extern void loadStrTable(unichar *,unichar **&,int &);
extern int getStringTableFileAvecNull(char *,unichar *&,unichar **&);
extern void loadStrTableAvecNull(unichar *,unichar **&,int &);
extern void strFileHeadLine(U_FILE *f,int sz);
extern void fillIntAtArray(int v,unichar *A,int cdepth);
extern 	int find_index_array(int *arr,int sz,int v);
extern unichar *ctl_char_name_string[];
extern unichar *uTokenGet(unichar *line,unichar *seg,int flag);
extern unichar get4HexVal(unichar *l);
extern unichar getValueIdx(unichar *s,int &idx);

extern unichar *uascToNum(unichar *uasc,int *val);
extern int debugPrFlag;

struct changeStrContext {
unichar changeStrTo[16][16];
int changeStrToIdx;
} ;

extern void initChangeStrContext(changeStrContext* ctx);
extern int findChangeStr(changeStrContext* ctx,unichar *v,unichar *des);
extern int changeStrToVal(changeStrContext* ctx,unichar *src);
extern int setStrToVal(changeStrContext* ctx,unichar *str,unichar v);

extern unichar u_null_string[];
extern unichar u_epsilon_string[];
extern unichar u_phraseMark_string[];
extern unichar defaultSpaceStr[];
extern int loadChangeFileToTable(char *f);
extern unichar *getConvTable(unichar v);
extern void converTableInit();
extern char *getUtoChar(unichar *);
extern unichar *assignUstring(unichar *src);
#endif 

