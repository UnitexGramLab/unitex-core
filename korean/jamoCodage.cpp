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


#include <stdlib.h>

using namespace std;
#include "unicode.h"
#include "jamoCodage.h"
#include "etc.h"


char *defaultSylToJamoMap = ""\
"# map of  korean alphabet in the UNICODE\n"\
"#		\n"\
"#		\n"\
"		\n"\
"<SS>	0x318D			# bounce of syllabe\n"\
"Initial_Consonants				\n"\
"0x1100	0x3131	0x1100				#0x3131\n"\
"0x1101 0x3132	0x1100	0x1100 	 	#0x3132\n"\
"0x1102	0x3134	0x1102			 	#0x3134\n"\
"0x1103	0x3137	0x1103				#0x3137\n"\
"0x1104	0x3138	0x1103	0x1103		#0x3138\n"\
"0x1105	0x3139	0x1105				#0x3139\n"\
"0x1106	0x3141	0x1106				#0x3141\n"\
"0x1107	0x3142	0x1107			 	#0x3142\n"\
"0x1108	0x3143	0x1107 0x1107		#0x3143\n"\
"0x1109	0x3145	0x1109			 	#0x3145\n"\
"0x110A	0x3146	0x1109 0x1109		#0x3146\n"\
"0x110B	0x3147						#0x3147\n"\
"0x110C	0x3148	0x110C				#0x3148\n"\
"0x110D	0x3149	0x110C 0x110C		#0x3149\n"\
"0x110E	0x314A	0x110E       		#0x314A\n"\
"0x110F	0x314B	0x110F				#0x314B\n"\
"0x1110	0x314c	0x1110				#0x314C\n"\
"0x1111	0x314D	0x1111				#0x314D\n"\
"0x1112	0x314E	0x1112   			#0x314E\n"\
"Vowels				\n"\
"0x1161	0x314F	0x1161				#0x314F\n"\
"0x1162	0x3150	0x1161 0x1175		#0x3150\n"\
"0x1163	0x3151	0x1175 0x1161		#0x3151\n"\
"0x1164	0x3152	0x1175 0x1161 0x1175#0x3152\n"\
"0x1165	0x3153	0x1165				#0x3153\n"\
"0x1166	0x3154	0x1165 0x1175		#0x3154\n"\
"0x1167	0x3155	0x1175 0x1165		#0x3155\n"\
"0x1168	0x3156	0x1175 0x1165 0x1175#0x3156\n"\
"0x1169	0x3157	0x1169				#0x3157\n"\
"0x116A	0x3158	0x1169 0x1161		#0x3158\n"\
"0x116B	0x3159	0x1169 0x1175 0x1165#0x3159\n"\
"0x116C	0x315A	0x1169 0x1175		#0x315A\n"\
"0x116D	0x315B	0x1175 0x1169		#0x315B\n"\
"0x116E	0x315c	0x116E				#0x315C\n"\
"0x116F	0x315d	0x116E 0x1165		#0x315D\n"\
"0x1170	0x315E	0x116E 0x1165 0x1175#0x315E\n"\
"0x1171	0x315F	0x116E 0x1175		#0x315F\n"\
"0x1172	0x3160	0x1175 0x116E		#0x3160\n"\
"0x1173	0x3161	0x1173				#0x3161\n"\
"0x1174	0x3162	0x1173 0x1175		#0x3162\n"\
"0x1175	0x3163	0x1175				#0x3163\n"\
"Final_Consonants\n"\
"FCNULL	-	-	-	#final consonant is vide\n"\
"0x11A8	0x3131	0x1100				#0x3131\n"\
"0x11A9	0x3132	0x1100 0x1100		#0x3132\n"\
"0x11AA	0x3133	0x1100 0x1109		#0x3133\n"\
"0x11AB	0x3134	0x1102				#0x3134\n"\
"0x11AC	0x3135	0x1102 0x110C		#0x3135\n"\
"0x11AD	0x3136	0x1102 0x1112		#0x3136\n"\
"0x11AE	0x3137	0x1103				#0x3137\n"\
"0x11AF	0x3139	0x1105				#0x3139\n"\
"0x11B0	0x313a	0x1105 0x1100		#0x313A\n"\
"0x11B1	0x313b	0x1105 0x1106		#0x313B\n"\
"0x11B2	0x313c	0x1105 0x1107		#0x313C\n"\
"0x11B3	0x313d	0x1105 0x1109		#0x313D\n"\
"0x11B4	0x313e	0x1105 0x1110		#0x313E\n"\
"0x11B5	0x313f	0x1105 0x1111		#0x313F\n"\
"0x11B6	0x3140	0x1105 0x1112		#0x3140\n"\
"0x11B7	0x3141	0x1106				#0x3141\n"\
"0x11B8	0x3142	0x1107				#0x3142\n"\
"0x11B9	0x3144	0x1107 0x1109		#0x3144\n"\
"0x11BA	0x3145	0x1109				#0x3145\n"\
"0x11BB	0x3146	0x1109 0x1109		#0x3146\n"\
"0x11BC	0x3147	0x110B				#0x3147\n"\
"0x11BD	0x3148	0x110C				#0x3148\n"\
"0x11BE	0x314a	0x110E				#0x314A\n"\
"0x11BF	0x314B	0x110F				#0x314B\n"\
"0x11C0	0x314c	0x1110				#0x314C\n"\
"0x11C1	0x314d	0x1111				#0x314D\n"\
"0x11C2	0x314e	0x1112				#0x314E\n"\
;


int
jamoCodage::loadJamoMap(char *mfName)
{
	FILE *f=0;
	unsigned short *wp; // token buff
	int i;
	int svFlag;
	char line[256];
	unsigned short  buff[256];

	int rsz;
	char *rp;
	unichar roffset;
	
	
	int JamoIndex,JamoCnt;
	unsigned short *segPtr[16];
	int segCnt;
	unsigned short *swp;

	if(!mfName){ rp = defaultSylToJamoMap;roffset = 0;}
	else {
		f = u_fopen(mfName,U_READ);
		if(!f) exitMessage(0);
	}
	jamoSize = 0;
	int jamoOffset = 0;
	do {
		rsz = 0;

		
		if(f){
          if(!u_read_line(f,(unichar *)buff)) break;
		} else{
          if(*rp == 0) break;
          while(*rp != '\n') line[rsz++] = *rp++;
          rp++;
          line[rsz++] = 0; 
          mbcsToUniStr((unsigned char *)line,(unichar *)buff);
        }
         
//printf(":::%s",line); u_fprintf(stderr,"L::%s\n",buff);
        segCnt = 0;
        svFlag = 1;
        swp = buff;
        while(*swp){
             switch(*swp){
             case '#':
                      *swp = 0; break;
             case ' ':
             case '\t':
                      svFlag = 1;*swp++ = 0;break;
             default:
              if(svFlag){
               segPtr[segCnt++] = swp;
               svFlag = 0;
               }
               swp++;
             }
         }
//for(i = 0; i < segCnt;i++) u_fprintf(stderr,"[%d:%S]",i,segPtr[i]);u_fprintf(stderr,"%d\n",segCnt); 
		if(!segCnt) continue;
		JamoCnt = 0;
		wp = segPtr[0];
		if(*wp == '0'){	// hex value
			JamoIndex=uniToInt((unichar *)wp);
			orderTableJamo[jamoOffset] = new unichar[8];
			mapJamoTable[JamoIndex - 0x1100] = orderTableJamo[jamoOffset];
			for(int i = 0; i < 8;i++)orderTableJamo[jamoOffset][i] = 0;
			orderTableJamo[jamoOffset][JamoCnt++] = JamoIndex;
		} else if( *wp == '<'){	// symbol for syllabe mark
			sylMarkStr = new unichar [u_strlen((unsigned short *)wp)+1];
            u_strcpy((unsigned short *)sylMarkStr,(unsigned short *)wp);
			if(segCnt == 2){
			    wp = segPtr[1];
				orderTableJamo[0xff] = new unichar[8];
				for(int i = 0; i < 8;i++)orderTableJamo[0xff][i] = 0;
				if(*wp == '0') 
    orderTableJamo[0xff][0] = uniToInt((unichar *)wp);
				else orderTableJamo[0xff][0] = *wp;
				sylMark = orderTableJamo[0xff][0];
			}
			continue;
		} else if(!u_strcmp_char(wp,"Initial_Consonants")){
			jamoOffset = 0;
			continue;
		} else if(!u_strcmp_char(wp,"Vowels")){
			if( jamoOffset != 19)
                     exitMessage("table error");
			continue;
		} else if(!u_strcmp_char(wp,"Final_Consonants")){
			if( jamoOffset != 40) exitMessage("table error");
			continue;
		} else if(!u_strcmp_char(wp,"FCNULL")){
			orderTableJamo[jamoOffset] = new unichar[8];
				for(int i = 0; i < 8;i++)orderTableJamo[jamoOffset][i] = 0;
			jamoOffset++;
			continue;
		}
		if(segCnt < 2) exitMessage("Line format :error");
		wp = segPtr[1];
		        
		if(wp[1] == '\0') 
			orderTableJamo[jamoOffset][JamoCnt++] = *wp;
		else
			orderTableJamo[jamoOffset][JamoCnt++] = uniToInt((unichar *)wp);
		 
		for(i = 2;i < segCnt;i++)
		{
            wp = segPtr[i];
			if(wp[1] == '\0') 
				orderTableJamo[jamoOffset][JamoCnt++] = *wp;
			else
				orderTableJamo[jamoOffset][JamoCnt++] = uniToInt((unichar *)wp);
		}
		jamoOffset++;
	} while(true);
	jamoSize = jamoOffset;

	for(int i = 0; i< jamoSize;i++)
		if(orderTableJamo[i][1])
			CjamoUjamoTable[orderTableJamo[i][1] - 0x3130] = &orderTableJamo[i][2];

	if(f) fclose(f);
	return(1);
}


//unichar SS;	// syllabe mark
int
jamoCodage::sylToJamo(unichar syl,unichar *obuff,int o_off)
{
	if(orderTableJamo[0xff][0]) obuff[o_off++] = sylMark;
	unichar tmp = syl - 0xac00;
	int fc = tmp % 28; tmp /= 28;
	int vw = tmp % 21;
	int ic = tmp / 21;
	unichar *wp = &orderTableJamo[ic][2];while(*wp) obuff[o_off++] = *wp++;
wp = &orderTableJamo[vw+19][2];	while(*wp) obuff[o_off++] = *wp++;
wp = &orderTableJamo[fc+40][2];	while(*wp) obuff[o_off++] = *wp++;
	return(o_off);
}
int
jamoCodage::jamoToSJamo(unichar jamo,unichar *obuff,int o_off)
{
	unichar *wp = CjamoUjamoTable[jamo - 0x3130];
	if(wp)
		while(*wp){ 
			obuff[o_off++] = *wp++;
		}
	else
		obuff[o_off++] = jamo;
	return(o_off);
}

int
jamoCodage::convertSylToJamo(unichar *ibuff,unichar *obuff,int sz,int limit)
{
	int osz = 0;
//	printf("%x %x\n",def_uchar_hangul_GA,def_uchar_hangul_HIH);
	for(int i = 0; i < sz;i++){
//		if((ibuff[i] >= L'°¡') && (ibuff[i] <= L'ÆR'))
		if((ibuff[i] >= HANGUL_SYL_START) && (ibuff[i] <= HANGUL_SYL_END))
			osz=sylToJamo(ibuff[i],obuff,osz);
		else 
			obuff[osz++] =ibuff[i];
		if(osz >= limit) return(limit-1);
	}
	obuff[osz]=0;
	return(osz);
}
int
jamoCodage::convertSyletCjamoToJamo(unichar *ibuff,unichar *obuff,int sz,int limit)
{
	int osz = 0;
	int i;
//	printf("%x %x\n",def_uchar_hangul_GA,def_uchar_hangul_HIH);
	for(i = 0; i < sz;i++){
		if(ibuff[i] == '\\'){
			i++; continue;
		}
		if((ibuff[i] >= HANGUL_SYL_START) && (ibuff[i] <= HANGUL_SYL_END))
			osz=sylToJamo(ibuff[i],obuff,osz);
		else if((ibuff[i] >= 0x3130 ) && (ibuff[i] <= 0x318F))	// alphabet coree Jamo
			osz = jamoToSJamo(ibuff[i],obuff,osz);
		else
			obuff[osz++] = ibuff[i];
		
		if(osz >= limit){
			osz = limit;
			break;
		}
	}
	obuff[osz] = 0;
//	printf("%s ",getUtoChar(getUibuff));
//	for( i = 0 ; i < sz ;i++) printf("0x%04x:",ibuff[i]);
//	printf("\n");
//	printf("%s ",getUtoChar(obuff));
//	for( i = 0; i < osz;i++) printf("0x%04x:",obuff[i]);
//	printf("\n");
	return(osz);
}
void jamoCodage::jamoMapOut()
{
	unichar markReturn = '\n';
	
	FILE *f;
	int i,j;
	if(!(f=u_fopen("jamoCTable.txt",U_WRITE))) fopenErrMessage("jamoCTable.txt");

	u_fprintf(f,"Unicode Jamos, pair of Hangul Jamo et Hangul Compatiblity Jamo\n");	
	for(i = 0; i < jamoSize;i++){
		if(orderTableJamo[i][0]){	// detect the condition of which the final consonrant is vide
            u_fputc(orderTableJamo[i][0],f);u_fputc(orderTableJamo[i][1],f);//			fwrite(&orderTableJamo[i][0],2,2,f);
        }        
        u_fputc(markReturn,f);//		fwrite(&markReturn,2,1,f);
	}
	u_fprintf(f,"Using Basic Jamo:");
	unichar basicJamo[128];
	int cntBasicJamo = 0;

	for(i = 0; i < jamoSize;i++){
		if(orderTableJamo[i][0]){
			if(orderTableJamo[i][2] && !orderTableJamo[i][3])	// detect a character
			{
				for( j = 0; j < cntBasicJamo;j++){
					if(basicJamo[j] == orderTableJamo[i][2]) break;
				}
				if(j == cntBasicJamo){
					basicJamo[cntBasicJamo++] = orderTableJamo[i][2];
					if(cntBasicJamo == 127) exitMessage("too long Jamos");
				}
			}
		}
	}
	for( i = 0; i < cntBasicJamo;i++){
		for( j = i;j< cntBasicJamo;j++){
			if(basicJamo[i] > basicJamo[j]){
				basicJamo[127] = basicJamo[i];
				basicJamo[i] = basicJamo[j];
				basicJamo[j] = basicJamo[127];
			}
		}
	}
	basicJamo[cntBasicJamo++]=orderTableJamo[0xff][0];
	for( i = 0; i < cntBasicJamo;i++){
		u_fputc(basicJamo[i],f); // fwrite(&basicJamo[i],2,1,f);
		u_fputc(markReturn,f); //fwrite(&markReturn,2,1,f);
	}


	fclose(f);
}

