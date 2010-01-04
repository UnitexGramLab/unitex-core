 /*
  * Unitex
  *
  * Copyright (C) 2001-2010 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef FORMATION_DIC
#define FORMATION_DIC
#define MAX_NUM_ELEMENT_BY_UNIT	4
#define MAX_NUM_ENTREE_COMP_LINE		5

//#define  EC_flechi				array[0]
//#define  EC_orgin				array[1]
//#define  EC_canonique			array[2]
//#define  EC_code_gramm			array[3]
//#define  EC_code				array[4]	// point of enter the transductor of suffixes
#define  EC_suffixe				EC_code	// point of enter the transductor of suffixes


class dicLines {
public:
	class dicLines *next;
    unichar *EC_flechi;
    unichar *EC_orgin;
    unichar *EC_canonique;
    unichar *EC_code_gramm;
    unichar *EC_code;// point of enter the transductor of suffixes
//#define  EC_suffixe;

	dicLines(){
		next = 0;
		EC_flechi = EC_orgin = EC_canonique = EC_code_gramm = EC_code = u_null_string;
	};
	~dicLines(){
		cleanDatas();
	};
	void set(unichar *f,unichar *c,unichar *o,unichar *cdg,unichar *cd)
	{
	    unichar *s,*wp;
		if(*f){
			EC_flechi =(unichar *)malloc((u_strlen((unichar *)f) +1 )*sizeof(unichar));
			u_strcpy((unichar *)EC_flechi,(unichar *)f);
		}
		if(*c){
			EC_canonique =(unichar *) 
				malloc((u_strlen((unichar *)c) +1 )*sizeof(unichar));
			u_strcpy((unichar *)EC_canonique,(unichar *)c);
		}
		if(*o){
			EC_orgin =(unichar *)
				malloc((u_strlen((unichar *)o) +1 )*sizeof(unichar));
			u_strcpy((unichar *)EC_orgin,(unichar *)o);
		}
		if(*cdg){
			EC_code_gramm =(unichar *) 
				malloc((u_strlen((unichar *)cdg) +1 )*sizeof(unichar));
			u_strcpy((unichar *)EC_code_gramm,(unichar *)cdg);
		}
  		if(*cd){
			EC_code =(unichar *) 
				malloc((u_strlen((unichar *)cd) +1 )*sizeof(unichar));
			s = cd;wp = EC_code;
			while(*s){ if((*s != ' ') & (*s != '\t')) *wp++ = *s; s++;}
			*wp = 0;
		}

	}
	void makeInfField(unichar *f,unichar *o,unichar *c,unichar *cdg,unichar *cd)
	{
		unichar tmp[256];
		unichar *wp = tmp;
		unichar *s;
		if(f && *f && u_strcmp(f,u_epsilon_string) ){
			EC_flechi =new unichar[u_strlen((unichar*)f) +1];
			u_strcpy((unichar *)EC_flechi,(unichar *)f);
		} else {
			EC_flechi = u_null_string;
		}
		
		*wp = 0;
		if(c &&  *c &&  u_strcmp(c,u_epsilon_string)){
			s =c;while(*s) *wp++ = *s++;
		}
		*wp++ = '.';
		if(o && *o && u_strcmp(o,u_epsilon_string) ){
			s = o; while(*s) *wp++ = *s++;			
		}
		*wp++ = '.';
		if(cdg && *cdg && u_strcmp(cdg,u_epsilon_string)){
			s = cdg;while(*s) *wp++ = *s++;			
		}
		*wp = 0;
		EC_code_gramm = new unichar[u_strlen((unichar *)tmp) +1];
		u_strcpy((unichar *)EC_code_gramm,(unichar *)tmp);
  
		if(cd && *cd && u_strcmp(cd,u_epsilon_string)){
			EC_suffixe =new unichar [u_strlen((unichar *)cd) +1 ];
			u_strcpy((unichar *)EC_suffixe,(unichar *)cd);
		} else {
			EC_suffixe =u_null_string; 
		}
		
	}
	
	
	void getll(unichar *out)
	{
		unichar *s,*d;
		d = out;
		s = EC_flechi; while(*s) *d++ = *s++;
		*d++ = ',';	
		s = EC_orgin;while(*s) *d++ = *s++;
		*d++ = ',';
		s = EC_canonique; while(*s) *d++ = *s++;
		*d++ = ',';
		s = EC_code_gramm;while(*s) *d++ = *s++;
		*d++ = ',';
		s = EC_code; while(*s) *d++ = *s++;
		*d++ = '\0';
	};
	void fputs(U_FILE *f)
	{
	    u_fprintf(f,";%S,",EC_flechi);
	    u_fprintf(f,"%S,",EC_orgin);
	    u_fprintf(f,"%S,",EC_canonique);
	    u_fprintf(f,"%S,",EC_code_gramm);
	    u_fprintf(f,"%S",EC_code);
	    
	}
	void
	cleanDatas()
	{
		if(EC_flechi != u_null_string)
        { free(EC_flechi); EC_flechi = u_null_string;}
		if(EC_canonique!= u_null_string)
        { free(EC_canonique); EC_canonique = u_null_string;}
		if(EC_orgin!= u_null_string)
        { free(EC_orgin); EC_orgin = u_null_string;}
		if(EC_code!= u_null_string)// point of enter the transductor of suffixes
        { free(EC_code);EC_code = u_null_string;}
		if(EC_code_gramm!= u_null_string)
        { free(EC_code_gramm); EC_code_gramm = u_null_string;}
	}

};

class dicElements {
public:
	class dicLines *hPtr;
	class dicLines *tPtr;
	class dicElements *next;
	dicElements(){
		hPtr = tPtr = 0;
		next = 0;
	};
	~dicElements(){};

	void prEle(U_FILE *f){
		class dicLines *wPtr = hPtr;
		while(wPtr){
			wPtr->fputs(f);
			if(next) next->prEle(f);
			else u_fprintf(f,"\n");
			wPtr = wPtr->next;
		}
	}
	void put(class dicLines *a)
	{
		if(tPtr) {
			tPtr->next = a;
		} else {
			hPtr = a;
		}
		tPtr = a;
		tPtr->next = 0;
	}
};
#endif	// FORMATION_DIC_LINE_DEF
