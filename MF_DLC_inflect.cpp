/*
  * Unitex 
  *
  * Copyright (C) 2001-2003 Universit<E9> de Marne-la-Vall<E9>e <unitex@univ-mlv.fr>
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
  * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
  *
  */

/* Created by Agata Savary (agata.savary@univ-tours.fr)
 * Last modification on Sept 08 2005
 */
//---------------------------------------------------------------------------

/********************************************************************************/
/* INFLECTION OF A DELAC FILE INTO A DELACF                                     */
/********************************************************************************/


#include "unicode.h"
#include "MF_DLC_inflect.h"
#include "MF_SU_morpho.h"
#include "MF_LangMorpho.h"
#include "MF_Util.h"
#include "MF_DicoMorpho.h"

//Alphabet of the current language
extern Alphabet* alph;

int DLC_line2entry(unichar* line, DLC_entry_T* entry);
int DLC_scan_unit(SU_id_T* u,unichar* line);
int DLC_scan_codes(unichar* codes[MAX_CODES],unichar* line);
int DLC_scan_comment(unichar** comment,unichar* line);
l_class_T* DLC_class_para(unichar* para);
int DLC_print_entry(DLC_entry_T* entry);
int DLC_print_unit(SU_id_T* unit);
int DLC_format_form(unichar* entry, int max, MU_f_T f, DLC_entry_T* dlc_entry);
void DLC_delete_entry(DLC_entry_T* entry);


/////////////////////////////////////////////////////////////////////////////////
// Inflect a DELAC into a DELACF.
// On error returns 1, 0 otherwise.
int DLC_inflect(char* DLC, char* DLCF) {
  FILE *dlc, *dlcf;  //DELAC and DELACF files
  unichar dlc_line[MAX_DLC_LINE];  //current DELAC line 
  unichar dlcf_line[MAX_DLCF_LINE];  //current DELAC line 
  int l;  //length of the line scanned
  DLC_entry_T* dlc_entry;
  MU_forms_T* MU_forms;  //inflected forms of the MWU
  int err;
  unichar nl[3];  //a newline
  u_strcpy_char(nl,"\n");
  
  //Open DELAC
  dlc = u_fopen(DLC, U_READ);
  if (!dlc) {
    fprintf(stderr,"Unable to open DELAC file: '%s' !\n", DLC);
    return 1;
  }

  //Open DELACF
  dlcf = u_fopen(DLCF, U_WRITE);
  if (!dlc) {
    fprintf(stderr,"Unable to open DELACF file: '%s' !\n", DLCF);
    return 1;
  }

  //Inflect one entry at a time
  l = u_fgets(dlc_line,MAX_DLC_LINE-1,dlc);
  //Omit the final newline
  if (u_strlen(dlc_line)>0 && dlc_line[u_strlen(dlc_line)-1]==(unichar)'\n')
    dlc_line[u_strlen(dlc_line)-1] = (unichar)'\0';
  //If a line is empty the file is not necessarily finished. 
  //If the last entry has no newline, we should noty skip this entry
  while (l || !feof(dlc)) {  
    dlc_entry = (DLC_entry_T*) malloc(sizeof(DLC_entry_T));
    if (!dlc_entry) {
      fprintf(stderr,"Memory allocation problem in function 'DLC_inflect'!\n");
      return 1;
    }
    //Convert a DELAC entry into the internal multi-word format
    err = DLC_line2entry(dlc_line,dlc_entry);
    if (err == -1) //Not enough memory
      return 1;
    if (!err) {
      MU_forms = (MU_forms_T*) malloc(sizeof(MU_forms_T));
      if (!MU_forms) {
	fprintf(stderr,"Memory allocation problem in function 'DLC_inflect'!\n");
	return 1;
      }

      //Inflect the entry
      err = MU_inflect(dlc_entry->lemma, MU_forms);

      if (!err) {
	int f;  //index of the current inflected form
	//Inform the user if no form generated 
	if (MU_forms->no_forms == 0) {
	  fprintf(stderr,"No inflected form could be generated for ");
	  DLC_print_entry(dlc_entry);
	}
	//Print inflected forms
	for (f=0; f<MU_forms->no_forms; f++) {
	  //Format the inflected form to the DELACF format
	  err = DLC_format_form(dlcf_line,MAX_DLCF_LINE-1,MU_forms->forms[f],dlc_entry);
	  if (!err) {
	    //Print one inflected form at a time to the DELACF file
	    u_fprints(dlcf_line,dlcf);
	    u_fprints(nl,dlcf);
	  }
	}
      }
      MU_delete_inflection(MU_forms);      
    }
    DLC_delete_entry(dlc_entry);
    //Get next entry
    l = u_fgets(dlc_line,MAX_DLC_LINE-1,dlc);
    //Omit the final newline
    if (u_strlen(dlc_line)>0 && dlc_line[u_strlen(dlc_line)-1]==(unichar)'\n')
      dlc_line[u_strlen(dlc_line)-1] = (unichar)'\0';
  }
  u_fclose(dlc);
  u_fclose(dlcf);
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////
// Converts a DELAC line ('line') into a structured DELAC entry ('entry').
// 'line' is non terminated by a newline.
// Initially, entry has its space allocated but is empty.
// Returns 1 if 'line' is empty, 2 if its format is incorrect, -1 if memory allocation problems, 0 otherwise. 
int DLC_line2entry(unichar* line, DLC_entry_T* entry) {
  int l;  //length of the scanned sequence
  int pos;  //index of the next character to be read
  SU_id_T* unit;

  pos = 0;
  if (!line[pos])   //Empty line
    return 1;

  //Initalize the lemma
  entry->lemma = (MU_lemma_T*) malloc(sizeof(MU_lemma_T));
  if (!entry->lemma) { 
    fprintf(stderr,"Memory allocation problem in function 'DLC_line2entry'!\n");
    return -1;
  }
  entry->lemma->no_units = 0;

  //Scan the single units
  while (line[pos] && line[pos] != (unichar) ',') {  //Each DELAC line must contain a comma
    unit = (SU_id_T*) malloc(sizeof(SU_id_T));
    if (!unit) { 
      fprintf(stderr,"Memory allocation problem in function 'DLC_line2entry'!\n");
      MU_delete_lemma(entry->lemma);
      return -1;
    }
    l = DLC_scan_unit(unit,&(line[pos]));
    if (l <= 0) {
      free(unit);
      MU_delete_lemma(entry->lemma);
      return 2;
    }
    entry->lemma->units[entry->lemma->no_units] = unit;
    entry->lemma->no_units++;
    pos += l;
  }
  
  if (line[pos] != (unichar) ',') {
    fprintf(stderr,"Comma missing in DELAC line: ");
    u_fprints(line,stderr);
    fprintf(stderr," !\n");
    MU_delete_lemma(entry->lemma);
    return 2;
  }

  //Scan the inflection paradigm
  unichar tmp[MAX_DLC_LINE];
  pos++;  //Omit the comma
  l = u_scan_until_char(tmp,&(line[pos]),MAX_DLC_LINE-1,"+:)\\",1);
  pos += l;
  if (!l) {
    fprintf(stderr,"Inflection paradigm inexistent in line: ");
    u_fprints(line,stderr);
    fprintf(stderr," !\n");
    MU_delete_lemma(entry->lemma);
    return 2;
  }
  entry->lemma->paradigm = (char*) malloc((u_strlen(tmp)+1) * sizeof(char));
  if (!entry->lemma->paradigm)  { 
    fprintf(stderr,"Memory allocation problem in function 'DLC_line2entry'!\n");
    MU_delete_lemma(entry->lemma);
    return -1;
  }
  for (int c=0; c<=u_strlen(tmp); c++) //Convert to char and copy
    entry->lemma->paradigm[c] = (char) tmp[c];

  //Determine the class (e.g. noun)
  l_class_T* cl;
  cl = DLC_class_para(tmp);
  if (!cl) {    
    fprintf(stderr,"Impossible to deduce the compound's inflection class (noun, adj, etc.): ");
    u_fprints(line,stderr);
    fprintf(stderr," !\n");
    MU_delete_lemma(entry->lemma);
    return 2;
  }
  entry->lemma->cl = cl;

  //Scan the semantic codes
  l = DLC_scan_codes(entry->codes,&(line[pos]));
  pos += l;

  //Scan the comment
  l = DLC_scan_comment(&(entry->comment),&(line[pos]));
  pos += l;
  
  if (line[pos]) {
    fprintf(stderr,"Bad format in DELAC line: ");
    u_fprints(line,stderr);
    fprintf(stderr," !\n");
    MU_delete_lemma(entry->lemma);  //delete lemma
    for (int c=0; entry->codes[c]; c++)  //delete codes
      free(entry->codes[c]);  
    free(entry->comment);  //delete comment
    return 2;
  }
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////
// Scans a single unit from a DELAC entry. 'line' is non terminated by a newline.
// Initially, 'u' has its space allocated but is empty.
// Returns the length of the scanned sequence, -1 if a format error occured, -2 if a memory allocation problem occured. 
int DLC_scan_unit(SU_id_T* u,unichar* line) {
  int l;  //length of the scanned sequence
  int pos;  //index of the next caracter to be scanned
  unichar tmp[MAX_DLC_LINE];
  
  /*
  printf("\nIn 'DLC_scan_unit': line=");
  u_prints(line);
  printf("\n");
  */

  pos = 0;
  //Scan a unit
  l = SU_get_unit(tmp,line,MAX_DLC_LINE-1,alph,0);  //The single word module determines what is a word and what is a separator, etc.
  if (l<=0) {
    return -1;
  }
  u->form = (unichar*) malloc((u_strlen(tmp)+1) * sizeof(unichar));
  if (!u->form) { 
    fprintf(stderr,"Memory allocation problem in function 'DLC_scan_unit'!\n");
    return -2;
  }
  u_strcpy(u->form,tmp);
  pos += l;

  //If no lemma indication
  if (line[pos] != (unichar) '(') {
    u->lemma = NULL;
    u->feat = NULL;
  }

  //Scan the unit's description contained between '(' and ')'
  else {
    pos++;  //Omit the '(' 
    //Scan the lemma if any
    u->lemma = (SU_lemma_T*) malloc(sizeof(SU_lemma_T));
    if (!u->lemma) { 
      fprintf(stderr,"Memory allocation problem in function 'DLC_scan_unit'!\n");
      free(u->form);
      return -2;
    }
    l = SU_get_unit(tmp,&(line[pos]),MAX_DLC_LINE-1,alph,0);  //The single word module determines what is a word and what is a separator, etc.
    if (l < 0) {
      free(u->form);
      SU_delete_lemma(u->lemma);
      return l;
    }
    u->lemma->unit = (unichar*) malloc((u_strlen(tmp)+1) * sizeof(unichar));
    if (!u->lemma->unit) { 
      fprintf(stderr,"Memory allocation problem in function 'DLC_scan_unit'!\n");
      return -2;
    }
    u_strcpy(u->lemma->unit,tmp);
    pos += l;
    
    //Scan the lemma's inflection paradigm
    if (line[pos] != (unichar) '.') {
      fprintf(stderr,"Dot missing after a unit's lemma: ");
      u_fprints(line,stderr);
      fprintf(stderr," !\n");
      free(u->form);
      SU_delete_lemma(u->lemma);
      return -1;
    }
    pos ++;  //Omit the dot
    unichar u_para[MAX_DLC_LINE];
    l = u_scan_until_char(u_para,&(line[pos]),MAX_DLC_LINE-1,"+:\\",1);
    if (!l) {    
      fprintf(stderr,"Unit's inflection paradigm non existent in DELAC line: ");
      u_fprints(line,stderr);
      fprintf(stderr," !\n");
      free(u->form);
      SU_delete_lemma(u->lemma);
      return -1;     
    }
    u->lemma->paradigm = (char*) malloc((u_strlen(u_para)+1) * sizeof(char));
    if (! u->lemma->paradigm) {
      fprintf(stderr,"Memory allocation problem in function 'DLC_scan_unit'!\n");
      free(u->form);
      SU_delete_lemma(u->lemma);
      return -2;
    }
    for (int c=0; c<=u_strlen(u_para); c++)
      u->lemma->paradigm[c] = (char)u_para[c];

    //Determine the lemma's inflection class (noun, adj, etc.)
    l_class_T* cl;
    cl = DLC_class_para(u_para);
    if (!cl) {    
      fprintf(stderr,"Impossible to deduce the unit's inflection class (noun, adj, etc.): ");
      u_fprints(line,stderr);
      fprintf(stderr," !\n");
      free(u->form);
      SU_delete_lemma(u->lemma);
      return -1;
    }
    u->lemma->cl = cl;
    pos += l;

    //Scan the unit's inflection features
    unichar tmp[MAX_DLC_LINE];
    if (line[pos] != (unichar) ':') {
      fprintf(stderr,"Colon missing after a unit's lemma: ");
      u_fprints(line,stderr);
      fprintf(stderr," !\n");
      free(u->form);
      SU_delete_lemma(u->lemma);
      return -1;
    }
    pos ++;  //Omit the colon
    l = u_scan_until_char(tmp,&(line[pos]),MAX_DLC_LINE-1,")",1);
    if (l<=0) {
      fprintf(stderr,"Inflection features missing after ':' for a unit: ");
      u_fprints(line,stderr);
      fprintf(stderr," !\n");
      free(u->form);
      SU_delete_lemma(u->lemma);
      return -1;
    }
    pos += l;
    if (line[pos] != (unichar) ')') {
      fprintf(stderr,"')' missing after a unit's inflection features: ");
      u_fprints(line,stderr);
      fprintf(stderr," !\n");
      free(u->form);
      SU_delete_lemma(u->lemma);
      return -1;
    }
    pos++; //Omit the ')'
    u->feat = d_get_feat_str(tmp);
    if (! u->feat) {
      fprintf(stderr,"Incorrect inflection features in a unit: ");
      u_fprints(line,stderr);
      fprintf(stderr," !\n");
      free(u->form);
      SU_delete_lemma(u->lemma);
      return -1;
    }
  }
  return pos;
}

/////////////////////////////////////////////////////////////////////////////////
// Scans semantic codes (e.g. "+Hum+z1") from a DELAC entry. 'line' is non terminated by a newline.
// The function allocates space for codes scanned. It must be liberated by the calling function.
// Returns the length of the scanned sequence, -1 if a format error occured, -2 if a memory allocation problem occured. 
int DLC_scan_codes(unichar* codes[MAX_CODES],unichar* line) {
  int l; //length of the scanned sequence
  int pos; //position of the current character in line
  int c;  //number of codes
  unichar tmp[MAX_DLC_LINE];

  pos = 0;
  c = 0;
  while (line[pos] == (unichar) '+') {
    pos ++;  //Omit the '+'
    l = u_scan_until_char(tmp,&(line[pos]),MAX_DLC_LINE-1,":/\\+",1);
    if (l) {
      codes[c] = (unichar*) malloc((u_strlen(tmp)+1) * sizeof(unichar));
      if (!codes[c]) {
	fprintf(stderr,"Memory allocation problem in function 'DLC_scan_codes'!\n");
	return -2;
      }
      u_strcpy(codes[c],tmp);
      pos += l;
      c++;
    }
  }
  codes[c] = NULL; 
  return pos;
}

/////////////////////////////////////////////////////////////////////////////////
// Scans comment (e.g. "/electricity") from a DELAC entry. 'line' is non terminated by a newline.
// The function allocates space for comment scanned. It must be liberated by the calling function.
// Returns the length of the scanned sequence, -1 if a format error occured, -2 if a memory allocation problem occured. 
int DLC_scan_comment(unichar** comment,unichar* line) {
  int l;  //length of the scanned sequence
  unichar tmp[MAX_DLC_LINE];

  l = 0;
  if (line[0] == '/') {
    line++;  //Omit the '/'
    l = u_scan_until_char(tmp,line,MAX_DLC_LINE-1,"",1);
    if (l) {
      *comment =(unichar*) malloc((u_strlen(tmp)+1) * sizeof(unichar));
      if (!(*comment)) {
	fprintf(stderr,"Memory allocation problem in function 'DLC_scan_comment'!\n");
	return -2;
      }
      u_strcpy(*comment,tmp);
    }
  }
  else
    *comment = NULL;
  return l+1;  //Length od the comment plus '/'
}
/**************************************************************************************/
/* Deduces the morphological class (e.g. noun) from the inflection paradigm (e.g. "N41").*/
/* If the string beginning the inflection paradigm is not in the list of class        */
/* equivalences, returns NULL.                                                        */
l_class_T* DLC_class_para(unichar* para) {
  unichar cl[MAX_CLASS_NAME];  //buffer for the class string
  int l;   //length of a scanned sequence
  l = u_scan_until_char(cl,para,MAX_CLASS_NAME-1,"0123456789 _;-:/\\+",1);
  if (l) 
    return d_get_class_str(cl);
  else 
    return NULL;
}

/////////////////////////////////////////////////////////////////////////////////
// Prints a DELAC entry into a DELAC file..
// If entry void or entry's lemma void returns 1, 0 otherwise.
int DLC_print_entry(DLC_entry_T* entry) {
  unichar line[MAX_DLC_LINE];
  u_strcpy_char(line,"");
  
  if (!entry || !entry->lemma) 
    return 1;  

  //Print  units
  for (int u=0; u<entry->lemma->no_units; u++) 
    DLC_print_unit(entry->lemma->units[u]);

  //Print paradigm
  u_strcat_char(line,",");
  u_strcat_char(line,entry->lemma->paradigm);
  
  //Concat codes
  for (int c=0; entry->codes[c]; c++) {
    u_strcat_char(line,"+");
    u_strcat(line,entry->codes[c]);
  }

  //Concat comment
  if (entry->comment) {
    u_strcat_char(line,"/");
    u_strcat(line,entry->comment);
  }

  u_strcat_char(line,"\n");

  //Print line
  u_prints(line);
  
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////
// Prints single unit into a DELAC file.
// If 'unit' void returns 1, if memory allocation problem returns -1, 0 otherwise.
int DLC_print_unit(SU_id_T* unit) {
  unichar unit_str[MAX_DLC_LINE];
  u_strcpy_char(unit_str,"");
  if (!unit)
    return 1;
  
  u_strcat(unit_str,unit->form);
  if (unit->lemma) {
    u_strcat_char(unit_str,"(");
    u_strcat(unit_str,unit->lemma->unit);
    u_strcat_char(unit_str,".");
    u_strcat_char(unit_str,unit->lemma->paradigm);
    if (unit->feat) {
      u_strcat_char(unit_str,":");
      unichar* tmp;
      tmp = d_get_str_feat(unit->feat);
      if (!tmp)
	return -1;
      u_strcat(unit_str,tmp);
      free(tmp);
    }
    u_strcat_char(unit_str,")");      
  }
  u_prints(unit_str);
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////
// Puts an inflected multi-word form 'f' corresponding to the DELAC entry 'dlc_entry' into the DELACF format ('entry').
// The resulting enntry may takes up to 'max' characters.
// 'entry' almready has its space allocated.
// Returns 1 on error, 0 otherwise.
int DLC_format_form(unichar* entry, int max, MU_f_T f, DLC_entry_T* dlc_entry) {
  int l;  //length of the entry

  //Inflected form
  l = u_strlen(f.form);
  if (l >= max)
    return 1;
  u_strcpy(entry, f.form);

  //Comma
  l++; 
  if (l >= max)
    return 1;
  u_strcat_char(entry,",");

  //Lemma
  int u;  //index of the current unit in the lemma of the MW form
  for(u=0; u<dlc_entry->lemma->no_units; u++)
    l = l + u_strlen(dlc_entry->lemma->units[u]->form);
  if (l >= max)
    return 1;
  for(u=0; u<dlc_entry->lemma->no_units; u++)
    u_strcat(entry,dlc_entry->lemma->units[u]->form);

  //Full stop
  l++; 
  if (l >= max)
    return 1;
  u_strcat_char(entry,".");

  //Inflection paradigm
  l = l + strlen(dlc_entry->lemma->paradigm);
  if (l >= max)
    return 1;
  u_strcat_char(entry,dlc_entry->lemma->paradigm);

  //Semantic codes
  int c;  //index of the current semantic code
  for (c=0; dlc_entry->codes[c]; c++)
    l = l + u_strlen(dlc_entry->codes[c]) + 1;
  if (l >= max)
    return 1;
  for (c=0; dlc_entry->codes[c]; c++) {
    u_strcat_char(entry,"+");
    u_strcat(entry,dlc_entry->codes[c]);
  }

  //Inflection features
  unichar* feat;  //sequence of single-letter inflection features, e.g. 'sIf'
  if (f.features && f.features->no_cats > 0) {
    feat = d_get_str_feat(f.features);
    l = l + u_strlen(feat) + 1;  //Place for a ':' and all features
    if (l >= max)
      return 1;
    u_strcat_char(entry,":");
    u_strcat(entry,feat);
    free(feat);
  }

  //Comment
  if (dlc_entry->comment && u_strlen(dlc_entry->comment)) {
    l = l + u_strlen(dlc_entry->comment);//Place for a '/' and the comment
    if (l >= max)
      return 1;
    u_strcat_char(entry,"/");
    u_strcat(entry,dlc_entry->comment);    
  }
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////
// Liberates the memory allocated for a DELAC entry.
void DLC_delete_entry(DLC_entry_T* entry) {
  if (!entry)
    return;
  MU_delete_lemma(entry->lemma);
  for (int c=0; entry->codes[c]; c++)  //delete codes
    free(entry->codes[c]);  
  free(entry->comment);  //delete comment
  free(entry);
}

