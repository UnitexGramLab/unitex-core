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

#include <assert.h>
#include <ctype.h>

#include "utils.h"
#include "symbol.h"
#include "symbol_op.h"

#define MAXBUF 1024


unichar PUNC_TAB[] = {
  '"', '\'',
  '+', '-', '*', '\\', '=',
  '.', ',', ':', ';', '!', '?',
  '(', ')', '[', ']', '<', '>', '{', '}',
  '%', '#', '@', '/', '$', '&',
  '|', '_',
//  '«', '»',
    0
};


/**
 * Allocates, initializes and returns a new symbol_t corresponding to the
 * given type.
 */
symbol_t* new_symbol(char type) {
symbol_t* symbol=(symbol_t*)malloc(sizeof(symbol_t));
if (symbol==NULL) {
   fatal_error("Not enough memory in new_symbol\n");
}
symbol->type=type;
symbol->negative=false;
symbol->form=0;
symbol->canonic=0;
symbol->POS=NULL;
symbol->feature=NULL;
symbol->nb_features=0;
symbol->next=NULL;
return symbol;
}

/**
 * Allocates, initializes and returns a new symbol_t corresponding to the
 * given POS description. The difference with 'new_symbol' is that this one
 * does not call 'symbol_type', in order to avoid infinite loops.
 */
symbol_t* new_symbol_POS_no_type(POS_t* POS) {
symbol_t* symbol=new_symbol(INC);
symbol->POS=POS;
if (POS->CATs->size!=0) {
   symbol->feature=(char*)malloc(POS->CATs->size*sizeof(char));
   if (symbol->feature==NULL) {
      fatal_error("Not enough memory in new_symbol_no_type\n");
   }
   symbol->nb_features=POS->CATs->size;
} else {
   symbol->feature=NULL;
   symbol->nb_features=0;
}
/* We say that the features have not been initialized yet */
for (int i=0;i< POS->CATs->size;i++) {
   symbol->feature[i]=UNSPECIFIED;
}
symbol->next=NULL;
return symbol;
}


/**
 * Allocates, initializes and returns a new symbol_t corresponding to the
 * given POS description.
 */
symbol_t* new_symbol_POS(POS_t* POS) {
symbol_t* s=new_symbol_POS_no_type(POS);
symbol_type(s);
return s;
}


/**
 * Allocates, initializes and returns a new symbol_t corresponding to the
 * <PNC> tag in ELAG grammars.
 */
symbol_t* new_symbol_PUNC(language_t* language,int canonic) {
POS_t* POS=language_get_POS(language,PUNC_STR);
symbol_t* symbol=new_symbol_POS(POS);
symbol->type=ATOM; 
symbol->negative=false;
symbol->form=0;
symbol->canonic=canonic;
return symbol;
}


/**
 * Allocates, initializes and returns a new symbol_t corresponding to the
 * <PNC> tag in ELAG grammars.
 */
symbol_t* new_symbol_PUNC(int punc) {
return new_symbol_PUNC(LANGUAGE,punc);
}


/**
 * Allocates, initializes and returns a new symbol_t corresponding to the
 * <NB> tag in ELAG grammars.
 */
symbol_t* new_symbol_CHFA(language_t* language,int canonic) {
POS_t* POS=language_get_POS(language,CHFA_STR);
symbol_t* symbol=new_symbol_POS(POS);
symbol->type=ATOM;
symbol->negative=false;
symbol->form=0;
symbol->canonic=canonic;
return symbol;
}


/**
 * Allocates, initializes and returns a new symbol_t corresponding to the
 * <?> tag in ELAG grammars.
 */
symbol_t* new_symbol_UNKNOWN(language_t* language,int form) {
POS_t* POS=language_get_POS(language,UNKNOWN_STR);
symbol_t* symbol=new_symbol_POS(POS);
symbol->type=ATOM;
symbol->negative=false;
symbol->form=0;
symbol->canonic=form;
return symbol;
}


/**
 * Reinitializes the fields of the given symbol_t.
 */
void empty_symbol(symbol_t* symbol) {
if (symbol->feature!=NULL) free(symbol->feature);
if (symbol->negative) free(symbol->negs);
symbol->type=-1;
symbol->negative=false;
symbol->form=0;
symbol->canonic=0;
symbol->POS=NULL;
symbol->feature=NULL;
symbol->nb_features=0;
symbol->next=NULL;
}


/**
 * Copies the symbol 'src' into 'dest'. Note that the content of 'dest' will be
 * overwritten, so that you should call 'empty_symbol' on it before.
 * 
 * WARNING: the POS field of 'src' is just copied and NOT duplicated, so that if
 *          'src->POS' is freed, 'dest->POS' will be undefined.
 */
void copy_symbol(symbol_t* dest,symbol_t* src) {
int i;
dest->type=src->type;
if ((dest->negative=src->negative)==true) {
   dest->nbnegs=src->nbnegs;
   dest->negs=(int*)malloc(dest->nbnegs*sizeof(int));
   if (dest->negs==NULL) {
      fatal_error("Not enough memory in copy_symbol\n");
   }
   for (i=0;i<dest->nbnegs;i++) {
      dest->negs[i]=src->negs[i];
   }
} else {
   dest->form=src->form;
   dest->canonic=src->canonic;
}
dest->POS=src->POS;
dest->feature=(char*)malloc(src->nb_features*sizeof(char));
if (dest->feature==NULL) {
   fatal_error("Not enough memory in copy_symbol\n");
}
for (i=0;i<src->nb_features;i++) {
   dest->feature[i]=src->feature[i];
}
dest->nb_features=src->nb_features;
dest->next=src->next;
}


/**
 * Allocates, initializes and returns a duplicate of the given symbol.
 * Note that it is not the same than allocating a new symbol and using
 * 'copy_symbol', because here, we really duplicate all the fields, even 'POS'.
 */
symbol_t* dup_symbol(const symbol_t* symbol) {
if (symbol==SYMBOL_DEF) {
   fatal_error("Internal error: invalid function call 'dup_symbol(SYMBOL_DEF)'\n");
}
if (symbol==NULL) return NULL;
symbol_t* res=NULL;
int i;
switch (symbol->type) {
   case LEXIC:
      res=new_symbol(LEXIC);
      break;
   case EPSILON:
      res=new_symbol(EPSILON);
      break;
   case ATOM:
   case INC_CAN: 
   case CODE:
   case INC:
   case INC_NEG:
   case CODE_NEG:
      res=new_symbol_POS(symbol->POS);
      res->type=symbol->type;
      if (!symbol->negative) {
         res->form=symbol->form;
         res->canonic=symbol->canonic;
      } else {
         res->negative=true;
         res->nbnegs=symbol->nbnegs;
         res->negs=(int*)malloc(res->nbnegs*sizeof(int));
         if (res->negs==NULL) {
            fatal_error("Not enough memory in dup_symbol\n");
         }
         for (i=0;i<res->nbnegs;i++) {
            res->negs[i]=symbol->negs[i];
         }
      }
      /* The 'feature' array has been initialized in 'new_symbol_POS' */
      for (i=0;i<symbol->nb_features;i++) {
         res->feature[i]=symbol->feature[i];
      }
      break;
   case EXCLAM:
      res=new_symbol(EXCLAM);
      break;
   case EQUAL:
      res=new_symbol(EQUAL);
      break;
   default: fatal_error("dup_symbol: unknown symbol type: '%c'\n",symbol->type);
}
return res;
}


/**
 * Returns a duplicate of the given symbol list. Note that
 * the new list is the mirror of the original one.
 */
symbol_t* dup_symbols(const symbol_t* symbols) {
symbol_t* res=NULL;
while (symbols!=NULL) {
   symbol_t* next=res;
   res=dup_symbol(symbols);
   res->next=next;
   symbols=symbols->next;
}
return res;
}


/**
 * Frees the memory associated to the given symbol, except
 * its 'POS' field, because it is supposed to be freed when
 * the field 'string_hash_ptr* POSs' of language_t is freed.
 */
void free_symbol(symbol_t* symbol) {
if (symbol==NULL || symbol==SYMBOL_DEF) return;
if (symbol->negative) free(symbol->negs);
if (symbol->feature!=NULL) free(symbol->feature);
free(symbol);
}


/**
 * Frees the memory associated to the given symbol list, except
 * their 'POS' fields (see 'free_symbol').
 */
void free_symbols(symbol_t* symbols) {
while (symbols!=NULL && symbols!=SYMBOL_DEF) {
   symbol_t* next=symbols->next;
   free_symbol(symbols);
   symbols=next;
}
}


/**
 * Appends the symbol 'b' at the end of the list 'a'. If 'end' is not NULL,
 * then the function copies in it the address of the last element of the new
 * list.
 */
void concat_symbols(symbol_t* a,symbol_t* b,symbol_t** end) {
if (a==NULL) {
   fatal_error("NULL 'a' in concat_symbols\n");
}
while (a->next!=NULL) {
   a=a->next;
}
a->next=b;
if (end) {
   while (a->next!=NULL) {
      a=a->next;
   }
   (*end)=a;
}
}




/* return how many codes in POS match with s
 * for each features whose value are the same in each matching code
 * set it in pcode (if non null)
 */


static int symbol_match_codes(symbol_t * s, symbol_t * pcode = NULL) {

  int count = 0;


  for (symbol_t * code = s->POS->codes; code; code = code->next) {

    bool ok = true;

    for (int i = 0; ok && i < s->POS->nb_discr; i++) {

      switch (code->feature[i]) {

      case UNSPECIFIED:
	fatal_error("match_codes: in POS '%S': code with UNSPEC discr code\n", s->POS->name);

      default:
	if ((s->feature[i] != UNSPECIFIED) && (s->feature[i] != code->feature[i])) { ok = false; }
	break;
      }
    }

    if (ok) {

      if (pcode) {

	if (count == 0) { // copy first matching code in pcode

	  for (int i = 0; i < s->POS->nb_discr; i++) { pcode->feature[i] = code->feature[i]; }

	} else { // set to UNSPEC features which divergent

	  for (int i = 0; i < s->POS->nb_discr; i++) {
	    if (pcode->feature[i] != code->feature[i]) { pcode->feature[i] = UNSPECIFIED; }
	  }
	}
      }

      /*
      debug("match_codes:\n");
      symbol_dump(s); fprintf(stderr, " matches with "); symbol_dump(code); fprintf(stderr, "\n");
      */
      count++;
    }
  }

  return count;
}


/*
 * set the symbol type:
 * * if canonic starts with '!' it's NEGATIF (should probably split NEG into NEG_INC and NEG_CODE)
 * * if its traits matches with only one full label, then its a CODE or ATOM (depending if it contains a canonic form)
 *   and we lock all discriminant traits which are set to UNSPECIFIED.
 * * if it doesn't match any then it is an invalid symbol
 * * if all discr traits are fixed then it is also a code (or atom)
 * * else it is an INCOMPLET code.
 */


int symbol_type(symbol_t * symb) {

  symb->type = INC;

  if (symb->POS->codes) {

    symbol_t * mcode = new_symbol_POS_no_type(symb->POS);

    int count = symbol_match_codes(symb, mcode);

    if (count == 0) { // symbol doesn't match any POS code -> invalid
      symb->type = -1;
      free_symbol(mcode);
      return symb->type;
    }

    if (count == 1) { // it's a code we set discr values to matching code's one

      symb->type = CODE;
      for (int i = 0; i < symb->POS->nb_discr; i++) { symb->feature[i] = mcode->feature[i]; }

    } else { // we try to lock some traits wich are same value on all matching codes

      symb->type = INC;
      for (int i = 0; i < symb->POS->nb_discr; i++) {
	if (symb->feature[i] == UNSPECIFIED) { symb->feature[i] = mcode->feature[i]; }
      }      
    }

    free_symbol(mcode);
  }


  if (symb->type == INC) { // if all discr traits are fixed, it's a code

    int i;
    for (i = 0; i < symb->POS->nb_discr; i++) { if (symb->feature[i] == UNSPECIFIED) { break; } }

    if (i == symb->POS->nb_discr) { symb->type = CODE; }
  }


  /* look for canonical */

  if (symb->type == INC) {

    if (symb->negative) {
      symb->type = INC_NEG;
    } else if (symb->canonic) {
      symb->type = INC_CAN;
    }

    return symb->type;
  }

  /* type == code */

  if (symb->negative) {
    symb->type = CODE_NEG;
  } else if (symb->canonic) {
    symb->type = ATOM;
  }

  return symb->type;
}



void symbol_dump_all(const symbol_t * symb, FILE * f) {

  static const unichar locked[] = { 'l', 'o', 'c', 'k', 'e', 'd', 0 };

  int i;
  language_t * lang = symb->POS ? symb->POS->language : LANGUAGE;

  u_fprintf(f, "<%c:", symb->type);

  if (symb->negative) {
    for (i = 0; i < symb->nbnegs; i++) {
      u_fprintf(f, "!%S", language_get_form(lang, symb->negs[i]));
    }
  } else {
    u_fprintf(f, "%S,%S", language_get_form(lang, symb->form), language_get_form(lang, symb->canonic));
  }


  if (symb->POS) {

    u_fprintf(f, ".%S", symb->POS->name);

    for (i = symb->POS->nb_inflect; i < symb->POS->CATs->size; i++) {
      CAT_t * CAT = POS_get_CAT(symb->POS,i);
      u_fprintf(f, "+%S=%S", CAT->name, (symb->feature[i] < 0) ? locked : (unichar *) CAT->values->value[symb->feature[i]]);
    }

    u_fprintf(f, ":");

    for (i = 0; i < symb->POS->nb_inflect; i++) {
      CAT_t * CAT = POS_get_CAT(symb->POS,i);
      u_fprintf(f, "+%S=%S", CAT->name, (symb->feature[i] < 0) ? locked : (unichar *) CAT->values->value[symb->feature[i]]);
    }

  } else {
    u_fprintf(f, ".*");
  }

  u_fprintf(f, ">");
}



void symbol_to_text_label(const symbol_t * s, Ustring * ustr) {

  if (s == NULL) { fatal_error("symbol 2 text label: symb is null\n"); }

  if (s == SYMBOL_DEF) { fatal_error("symb2txt: symb is <def>\n"); }

  if (s->type != ATOM) {
    error("symbol2txt: symbol '"); symbol_dump(s); fatal_error("' is'nt an atom.\n");
  }

  language_t * lang = s->POS->language;
  if ((u_strcmp(s->POS->name, UNKNOWN_STR) == 0) || (u_strcmp(s->POS->name, PUNC_STR) == 0) || (u_strcmp(s->POS->name, CHFA_STR) == 0)) {
    u_strcpy(ustr, language_get_form(lang, s->canonic));
    return;
  }
  u_sprintf(ustr, "{%S,%S.%S", language_get_form(lang, s->form),language_get_form(lang, s->canonic),s->POS->name);
  int i;
  CAT_t * CAT;

  for (i = s->POS->nb_inflect; i < s->nb_features; i++) {
    if (s->feature[i] > 0) {
      CAT = POS_get_CAT(s->POS, i);
      u_strcatf(ustr, "+%S", CAT_get_valname(CAT, s->feature[i]));
    }
  }


  bool colons = false;

  for (i = 0; i < s->POS->nb_inflect; i++) {

    CAT = POS_get_CAT(s->POS, i);

    if (s->feature[i] > 0) {
      if (colons == false) { u_strcat(ustr, ":"); colons = true; }
      u_strcat(ustr, CAT_get_valname(CAT, s->feature[i]));
    }
  }

  u_strcat(ustr, "}");
}


void symbol_to_implosed_text_label(const symbol_t * s, Ustring * ustr) {

  if (s == NULL) { fatal_error("symbol to text label: symb is null\n"); }

  if (s == SYMBOL_DEF) { fatal_error("symb2txt: symb is <def>\n"); }

  if (s->type != ATOM) {
    error("symbol2txt: symbol '"); symbol_dump(s); fatal_error("' is'nt an atom.\n");
  }

  language_t * lang = s->POS->language;

  if ((u_strcmp(s->POS->name, UNKNOWN_STR) == 0) || (u_strcmp(s->POS->name, PUNC_STR) == 0) || (u_strcmp(s->POS->name, CHFA_STR) == 0)) {
    u_strcpy(ustr, language_get_form(lang, s->canonic));
    return;
  }

  u_sprintf(ustr, "{%S,%S.%S", language_get_form(lang, s->form), language_get_form(lang, s->canonic), s->POS->name);

  int i;
  CAT_t * CAT;

  for (i = s->POS->nb_inflect; i < s->nb_features; i++) {
    if (s->feature[i] > 0) {
      CAT = POS_get_CAT(s->POS, i);
      u_strcatf(ustr, "+%S", CAT_get_valname(CAT, s->feature[i]));
    }
  }


  for (; s; s = s->next) {

    bool colons = false;

    for (i = 0; i < s->POS->nb_inflect; i++) {

      if (s->feature[i] > 0) {

	CAT = POS_get_CAT(s->POS, i);

	if (colons == false) { u_strcat(ustr, ":"); colons = true; }
	u_strcat(ustr, CAT_get_valname(CAT, s->feature[i]));
      }
    }
  }

  u_strcat(ustr, "}");
}



void symbol_to_locate_label(const symbol_t * s, Ustring * ustr) {

  if (s == NULL) { fatal_error("symb2locate label: symb is null\n"); }
  if (s == SYMBOL_DEF) { error("symb2locate: symb is <def>\n"); u_strcpy(ustr, "<def>"); return; }

  switch (s->type) {

  case LEXIC:
    u_strcpy(ustr, "<.>");
    return;

  case EPSILON:
    u_strcpy(ustr, "<E>");
    return;

  case EXCLAM:
    error("'<!>' in concordance fst2 ???\n");
    u_strcpy(ustr, "!");
    return;

  case EQUAL:
    error("'<=>' in concordance fst2 ???\n");
    u_strcpy(ustr, "=");
    return;
  }

  language_t * lang = s->POS->language;

  if ((u_strcmp(s->POS->name, UNKNOWN_STR) == 0)) { // unknown word
    u_strcpy(ustr, "<MOT>");
    return;
  } else if ((u_strcmp(s->POS->name, PUNC_STR) == 0) || (u_strcmp(s->POS->name, CHFA_STR) == 0)) {
    if (s->canonic) {
      //      debug("CHFA(%S) : canonic=%S\n", s->POS->name, s->canonic);
      u_strcpy(ustr, language_get_form(lang, s->canonic));
      return;
    }
  }


  if (! s->negative && s->canonic) {
    u_sprintf(ustr, "<%S.%S", language_get_form(lang, s->canonic), s->POS->name);
  } else {
    u_sprintf(ustr, "<%S", s->POS->name);
  }

  int i;
  CAT_t * CAT;

  for (i = s->POS->nb_inflect; i < s->nb_features; i++) {

    if (s->feature[i] > 0) {
      CAT = POS_get_CAT(s->POS, i);
      u_strcatf(ustr, "+%S", CAT_get_valname(CAT, s->feature[i]));
    }
  }


  bool colons = false;

  for (i = 0; i < s->POS->nb_inflect; i++) {

    CAT = POS_get_CAT(s->POS, i);

    if (s->feature[i] > 0) {

      if (colons == false) { u_strcat(ustr, ":"); colons = true; }
      u_strcat(ustr, CAT_get_valname(CAT, s->feature[i]));

    }
  }

  u_strcat(ustr, ">");
}



void symbol_to_grammar_label(const symbol_t * s, Ustring * ustr) {

  //  debug("symbtogrammlabel\n"); symbol_dump(s); endl();
  
  if (s == NULL) { fatal_error("symb2grammar label: symb is null\n"); }
  if (s == SYMBOL_DEF) { u_strcpy(ustr, "<def>"); return; }

  switch (s->type) {
  case LEXIC:
    u_strcpy(ustr, "<.>");
    return;
  case EPSILON:
    u_strcpy(ustr, "<E>");
    return;
  case EXCLAM:
    u_strcpy(ustr, "<!>");
    return;
  case EQUAL:
    u_strcpy(ustr, "<=>");
    return;
  }


  language_t * lang = s->POS->language;

  if (s->negative) {

    u_strcpy(ustr, "<");
    for (int i = 0; i < s->nbnegs; i++) {
      u_strcatf(ustr, "!%S", language_get_form(lang, s->negs[i]));
    }
    u_strcatf(ustr, ".%S", s->POS->name);

  } else if (s->canonic) {

    u_sprintf(ustr, "<%S.%S", language_get_form(lang, s->canonic), s->POS->name);

  } else {    

    u_sprintf(ustr, "<%S", s->POS->name);
  }

  int i;
  CAT_t * CAT;

  for (i = s->POS->nb_inflect; i < s->nb_features; i++) {

    if (s->feature[i] > 0) {

      CAT = POS_get_CAT(s->POS, i);
      u_strcatf(ustr, "+%S", CAT_get_valname(CAT, s->feature[i]));

    } else if (s->feature[i] == LOCKED) {
      /* Pour specifier que l'attribut est bloque on l'ecrit !attrname 
       * ex : !discr
       */
      CAT = POS_get_CAT(s->POS, i);
      u_strcatf(ustr, "!%S", CAT->name);
    }
  }


  bool colons = false;

  for (i = 0; i < s->POS->nb_inflect; i++) {

    CAT = POS_get_CAT(s->POS, i);

    if (s->feature[i] > 0) {

      if (colons == false) { u_strcat(ustr, ":"); colons = true; }
      u_strcat(ustr, CAT_get_valname(CAT, s->feature[i]));

    } else if (s->feature[i] == LOCKED) {

      /* pour specifier que le code de flexion est bloquer on l'ecrit @valeur
       * ex: @m signifie la meme chose que @f puisque m et f sont dans la meme categorie genres
       */

      if (colons == false) { u_strcat(ustr, ":"); colons = true; }
      u_strcatf(ustr, "@%S", CAT_get_valname(CAT, 1));
    }
  }

  u_strcat(ustr, ">");
}



void symbol_to_str(const symbol_t * s, Ustring * ustr) {

  if (s == NULL) { u_strcpy(ustr, "nil"); return; }
  if (s == SYMBOL_DEF) { u_strcpy(ustr, "<def>"); return; }

  switch (s->type) {
  case LEXIC:
    u_strcpy(ustr, "<.>");
    return;
  case EPSILON:
    u_strcpy(ustr, "<E>");
    return;
  case EXCLAM:
    u_strcpy(ustr, "<!>");
    return;
  case EQUAL:
    u_strcpy(ustr, "<=>");
    return;
  }

  language_t * lang = s->POS->language;

  u_sprintf(ustr, "<%c:", s->type);

  if (s->negative) {

    for (int i = 0; i < s->nbnegs; i++) {
      u_strcatf(ustr, "!%S(%d)", language_get_form(lang, s->negs[i]), s->negs[i]);
      //ustring_concatf(ustr, "!%S", language_get_form(lang, s->negs[i]));
    }

  } else {

    if (s->form)    { u_strcatf(ustr, "%S,", language_get_form(lang, s->form)); }
    if (s->canonic) { u_strcatf(ustr, "%S", language_get_form(lang, s->canonic)); }
  }

  u_strcatf(ustr, ".%S", s->POS->name);

  int i;
  CAT_t * CAT;

  for (i = s->POS->nb_inflect; i < s->nb_features; i++) {
    if (s->feature[i] > 0) {
      CAT = POS_get_CAT(s->POS, i);
      u_strcatf(ustr, "+%S", CAT_get_valname(CAT, s->feature[i]));
    } else if (s->feature[i] == LOCKED) {
      CAT = POS_get_CAT(s->POS, i);
      u_strcatf(ustr, "!%S", CAT->name);
    }
  }

  u_strcat(ustr, ":");

  for (i = 0; i < s->POS->nb_inflect; i++) {

    CAT = POS_get_CAT(s->POS, i);

    if (s->feature[i] > 0) {
      u_strcat(ustr, CAT_get_valname(CAT, s->feature[i]));
    } else if (s->feature[i] == LOCKED) {
      u_strcatf(ustr, "!{%S}", CAT->name);
    }
  }  

  u_strcat(ustr, ">");
}


void symbol_dump(const symbol_t * s, FILE * f) {
  Ustring * ustr = new_Ustring();
  symbol_to_str(s, ustr);
  u_fprintf(f, "%S", ustr->str);
  free_Ustring(ustr);
}


void symbols_dump(const symbol_t * s, FILE * f) {

  u_fprintf(f, "(");
  while (s) { symbol_dump(s, f); if ((s = ((s == SYMBOL_DEF) ? NULL : s->next))) { u_fprintf(f, ", "); } }
  u_fprintf(f, ")");
}

/* extract different label parts
 * at least pos should be non null
 */


// form,can.POS [ +traits:flex ]
// can.POS [ +traits:flex ]
// POS [ +traits:flex ]
// POS


static inline void split_dic_label(unichar * label, unichar ** form, unichar ** canonic, unichar ** pos,
				   unichar ** traits, unichar ** flex) {

  unichar * p = label;
  unichar * q;

  if ((q = u_strchr(p, ',')) != NULL) {
    *q = 0;
    *form = p;
    p = q + 1;

  } else { *form = NULL; }


  if ((q = u_strchr(p, '.')) != NULL) {
    *q = 0;
    *canonic = p;
    p = q + 1;

  } else { *canonic = NULL; }


  *pos = p;

  if ((q = u_strchr(p, '+')) != NULL) {

    *q = 0;
    *traits = q + 1;
    p = q + 1;

  } else { *traits = NULL; }


  if ((q = u_strchr(p, ':')) != NULL) {

    *q = 0;
    *flex = q + 1;

  } else { *flex = NULL; }

}




inline int check_dic_entry(const unichar * label) {

  /* {__,__.__} */

  if (*label != '{') { error("'%S': malformed DELA entry ('{' is missing)\n", label); return -1; }

  const unichar * p = label + 1;

  while (*p !=  ',') { // look for ','
    if (*p == 0) { error("'%S': malformed DELA entry (',' is missing).\n", label); return -1; }
    p++;
  }

  while (*p != '.') {
    if (*p == 0) { error("'%S': malformed DELA entry ('.' is missing).\n", label); return -1; }
    p++;
  }

  while (*p != '}') {
    if (*p == 0) { error("'%S': malformed DELA entry: '%S' ('}' is missing).\n", label); return -1; }
    p++;
  }

  p++;
  if (*p != 0) { error("'%S': malformed label: label should end with '}'.\n", label); return -1; }

  return 0;
}





/* text label can be either a full DELA entry, either a punc symbol, either an unknow word
 */

static inline int check_text_label(unichar * label) {


  if ((label == NULL)) { error("check label: no label\n"); return -1; }

  if (*label == 0) { error("error: check label: label is empty\n"); return -1; }

  if (*label == '{' && *(label + 1)) { return check_dic_entry(label); }


  /* no spaces */

  // DON'T check for spaces ! (cause errors in GREEK)
  //for (unichar * p = label; *p; p++) { if (isspace(*p)) { error("malformed label: '%S'.\n", label); return -1; } }

  return 0;
}




#warning replace parsing functions with StringParsing functions
unichar * u_strtok_char(unichar * str, char * delim) {

  static unichar * next = NULL;
  unichar * p;

  if (str == NULL) { str = next; }

  if (str == NULL) { return NULL; }

  while (u_strchr(delim,*str)) { str++; }  // skip all delims at the begining of str

  if (*str == 0) { next = NULL; return NULL; }

  /* we have a token (begin at str) */

  p = str;

  while (*p) {
    if (u_strchr(delim,*p)) {
      *p = 0;
      p++;
      next = (*p == 0) ? NULL : p;
      return str;
    }
    p++;
  } 

  next = NULL;
  return str;
}



/* load a label of the form {form,canonic.POS[+traits]*[:flex]*}
 * buf is a writeable copy of label
 */
symbol_t * load_dic_entry(language_t * lang, const unichar * label, unichar * buf, bool warnmissing) {

  buf[u_strlen(buf) - 1] = 0; // chomp trailing '}'

  unichar * form, * canonic, * pos, * traits, * flexs;

  split_dic_label(buf + 1, & form, & canonic, & pos, & traits, & flexs);

  POS_t * POS = language_get_POS(lang, pos);

  if (POS == NULL) {
    error("'%S': unknow POS '%S'\n", label, pos);
    return NULL;
  }

  symbol_t * symb = new_symbol_POS(POS);
  symbol_t * model;

  symb->type = ATOM;

  symb->form    = language_add_form(lang, form);
  symb->canonic = *canonic ? language_add_form(lang, canonic) : symb->form;


  /* additionnal traits ... */

  // we lock all featuress which are not explicitly set

  for (int i = 0; i < POS->CATs->size; i++) { symb->feature[i] = LOCKED; }


  unichar * p = u_strtok_char(traits, "+");

  while (p) {

    feature_info_t * info = POS_get_trait_infos(POS, p);

    if (info) {

      symb->feature[info->CATid] = info->val;

    } else if (warnmissing) {

      if (get_value_index(p,lang->unknown_codes,DONT_INSERT) == -1) {
	      error("in symbol '%S': unknow value '%S', will not be taken into account\n", label, p);
	      get_value_index(p,lang->unknown_codes,INSERT_IF_NEEDED,NULL);
      }
    }

    p = u_strtok_char(NULL, "+");
  }


  /* flexional codes */

  p = u_strtok_char(flexs, ":");

  if (p == NULL) { // no flexionnal code

    if (POS->codes && (symbol_match_codes(symb) == 0)) {
      error("'%S': doesn't match with POS '%S' definition\n", label, POS->name);
      goto err_symb;
    }

    return symb;
  }


  model = symb;
  symb = NULL;

  while (p) {

    symbol_t * nouvo = dup_symbol(model);

    for (; *p; p++) {

      feature_info_t * infos = POS_get_flex_infos(POS, *p);

      if (infos == NULL) { error("'%S': unknow flexionnal code '%C'\n", label, *p); goto err_model; }

      nouvo->feature[infos->CATid] = infos->val;
    }


    if (POS->codes && (symbol_match_codes(nouvo) == 0)) {
      error("'%S': doesn't match with POS '%S' definition\n", label, POS->name);
      goto err_model;
    }

    nouvo->next = symb;
    symb = nouvo;

    p = u_strtok_char(NULL, ":");
  }

  free_symbol(model);
  
  return symb;

err_model:
  free_symbol(model);

err_symb:
  free_symbols(symb);

  return NULL;
}



symbol_t * load_text_symbol(language_t * lang, unichar * label) {

  unichar buf[u_strlen(label) + 1];
  u_strcpy(buf, label);

  if (check_text_label(buf) == -1) {
    error("bad format in text symbol: '%S'\n", label);
    return NULL;
  }

  if (u_strcmp(label, "<E>") == 0) {
    return new_symbol(EPSILON);
  }

  if (u_strcmp(label, "<def>") == 0) { fatal_error("<def> trans in text automaton!\n"); }

  if (*buf == '{' && buf[1]) {   /*  dictionnary entry ( {__,__.__} ) */
    return load_dic_entry(lang, label, buf);
  }


  /* mot inconnu dans un texte ou ponctuation */

  int idx = language_add_form(lang, buf);

  if (u_strchr(PUNC_TAB, *buf)) {          /* ponctuation */

    if (buf[1] && buf[0] != '\\') { fatal_error("bad text symbol '%S' (ponctuation too long)\n", label); }

    return new_symbol_PUNC(lang, idx);

  } else if (u_is_digit(*buf)) {     /* chiffre arabe */

    for (unichar * p = buf; *p; p ++) { if (! u_is_digit(*p)) { fatal_error("bad symbol : '%S' (mixed nums and chars)\n", label); } }

    return new_symbol_CHFA(lang, idx);
  }

  /* unknow word  */

  return new_symbol_UNKNOWN(lang, idx);
}


/* LEXIC_minus_POS: in this file because we need it in load_gramm_symbol fonction
 */

symbol_t * LEXIC_minus_POS(POS_t * POS) {

  symbol_t res;
  res.next = NULL;
  symbol_t * end = & res;

  for (int i = 0; i < LANGUAGE->POSs->size; i++) {

    POS_t * POS2 = (POS_t *) LANGUAGE->POSs->value[i];

    if (POS2 == POS) { continue; }

    concat_symbols(end, new_symbol_POS(POS2), & end);
  }

  return res.next;
}




static void fill_neglist(language_t * lang, symbol_t * s, unichar * canonic) {

  unichar * p;
  int nbnegs;
  for (p = canonic, nbnegs = 0; *p; p++) { if (*p == '!') { nbnegs++; } }


  s->negs = (int *) xmalloc(nbnegs * sizeof(int));
  s->nbnegs = 0;

  p = u_strtok_char(canonic, "!");

  while (p) {

    assert(s->nbnegs < nbnegs);

    int idx = language_add_form(lang, p);
    
    /* bubble sort */

    int pos = s->nbnegs; /* pos for (bubble) position (not part of speech here) */
    s->negs[pos] = idx;

    while (pos && (s->negs[pos - 1] > s->negs[pos])) {
      int tmp = s->negs[pos - 1];
      s->negs[pos - 1] = s->negs[pos];
      s->negs[pos] = tmp;
      pos--;
    }

    if (pos && (s->negs[pos - 1] == s->negs[pos])) { // bubble expulsion (doublon in neg list)

      error("bubble expulsion (%S)\n", p);

      while (pos < s->nbnegs) {
	int tmp = s->negs[pos + 1];
	s->negs[pos + 1] = s->negs[pos];
	s->negs[pos] = tmp;
	pos++;
      }
    } else { s->nbnegs++; }

    p = u_strtok_char(NULL, "!");
  }

  if (s->nbnegs != nbnegs) { error("in fill_neglist: different nbnegs?\n"); }
}


/* load a symbol which has the generic form '<canonic.POS+trait:flex>'
 * called from load_grammar_symbol only
 */

static symbol_t * load_gram_symbol(language_t * lang, const unichar * label, unichar * buf) {

  /* etiquette incomplete */

  int len = u_strlen(buf);
  if (buf[len - 1] != '>') { fatal_error("bad grammar symbol: '%S'\n", label); }

  buf[len - 1] = 0; // chomp trailing '>'

  unichar * p;
  unichar * canonic, * pos;

  if ((p = u_strchr(buf, '.')) != NULL) { //form canonic or negative form is present
    canonic = buf + 1;
    *p = 0;
    pos = p + 1;

  } else { canonic = NULL; pos = buf + 1; }


  if (*pos == '!') { // negation d'un code grammatical (ex: <!PNC>)

    POS_t * POS = language_get_POS(lang, pos + 1);

    if (POS == NULL) { fatal_error("in symbol '%S': unknow part of speech '%S'\n", label, pos + 1); }

    return LEXIC_minus_POS(POS);
  }

  
  unichar next;

  if ((p = u_strpbrk(pos, "+!:")) != NULL) { // additionnals features or flexional codes
    next = *p;
    *p   = 0;
  } else { next = 0; } // POS only


  POS_t * POS = language_get_POS(lang, pos);

  if (POS == NULL) { fatal_error("in symbol '%S': unknow part of speech '%S'\n", label, pos); }


  symbol_t * symb = new_symbol_POS(POS);

  if (canonic && *canonic) {

    if (*canonic != '!') {

      symb->canonic = language_add_form(lang, canonic); 

    } else { // negative symbol

      symb->negative = true;
      fill_neglist(lang, symb, canonic);
    }
  }


  /* additionnal traits ... */

  unichar type = next;
  unichar * attr = p + 1;

  while ((type == '+') || (type == '!')) {

    if ((p = u_strpbrk(attr, "+!:")) != NULL) { // c'est pas termine
      next = *p;
      *p   = 0;
    } else { next = 0; }


    if (type == '+') { // attribute is set

      feature_info_t * info = POS_get_trait_infos(POS, attr);

      if (info) {

	symb->feature[info->CATid] = info->val;

      } else {

	if (get_value_index(attr,lang->unknown_codes,DONT_INSERT) == -1) {
	  error("in symbol '%S': unknow attribute '%S', will not be taken into account\n", label, attr);
	  get_value_index(attr,lang->unknown_codes,INSERT_IF_NEEDED,NULL);
	}
      }

    } else { // feature is locked (type == '!')

      int idx = POS_get_CATid(POS, attr);
      if (idx == -1) { fatal_error("in symbol '%S': unknow feature '%S'\n", label, attr); }
      if (idx < POS->nb_inflect) { fatal_error("in symbol '%S': '%S' is a flexionnal feature!\n", label, attr); }
      if (symb->feature[idx] != UNSPECIFIED) { fatal_error("in symbol '%S': '%S' cannot be locked and set\n", label, attr); }
      symb->feature[idx] = LOCKED;
    }

    attr = p + 1;
    type = next;
  }


  /* flexional codes */

  if (type == 0) { // no flexionnal code
    if (symbol_type(symb) == -1) { fatal_error("'%S' is not a valid\n", label); }
    return symb;
  }


  assert(type == ':');
  assert(p);

  symbol_t * model = symb;
  symb = NULL;

  while (p) { // still flex code sequence

    // debug("attr=%S\n", attr);

    if ((p = u_strchr(attr, ':')) != NULL) { // c'est pas termine
      *p = 0;
    }

    symbol_t * nouvo = dup_symbol(model);

    for (; *attr; attr++) {

      feature_info_t * infos;

      if (*attr == '@') { // flexionnal feature is locked
	attr++;
	if ((infos = POS_get_flex_infos(POS, *attr)) == NULL) {
	  fatal_error("in symbol '%S': unknow flex code '%C'\n", label, *attr);
	}
	nouvo->feature[infos->CATid] = LOCKED;
	
      } else { // flexionnal code is set

	if ((infos = POS_get_flex_infos(POS, *attr)) == NULL) {
	  fatal_error("in symbol '%S': unknow flexionnal code '%C'\n", label, *attr);
	}

	nouvo->feature[infos->CATid] = infos->val;
      }
    }

    if (symbol_type(nouvo) == -1) { fatal_error("'%S' is not a valid symbol\n", label); }

    nouvo->next = symb;
    symb = nouvo;

    attr = p + 1;
  }

  free_symbol(model);

  return symb;
}


/* load a symbol from an Elag grammar label
 */

symbol_t * load_grammar_symbol(language_t * lang, unichar * label) {

  unichar buf[u_strlen(label) + 1];
  u_strcpy(buf, label);

  if (*buf == '{' && buf[1]) {   /*  dictionnary entry ( {__,__.__} ) */

    if (u_strcmp(buf, "{S}") == 0) { return new_symbol_PUNC(lang, language_add_form(lang, buf)); } // limite de phrase

    error("'%S': DELAS entry in Elag grammar????\n", label);

    if (check_dic_entry(buf) == -1) { fatal_error("bad grammar label '%S'\n", label); }

    return load_dic_entry(lang, label, buf);
  }


  /* mot inconnu dans un texte ou ponctuation */

  if (*buf == '<' && *(buf + 1)) { // etiquette

    /* EPSILON */

    if (u_strcmp(buf, "<E>") == 0) { return new_symbol(EPSILON); }

    /* UNIVERSEL */

    if (u_strcmp(buf, "<.>") == 0) { return new_symbol(LEXIC); }


    /* special def label */

    if (u_strcmp(buf, "<def>") == 0) { return SYMBOL_DEF; }


    /* special EXCLAM symbol */

    if (u_strcmp(buf, "<!>") == 0) { return new_symbol(EXCLAM); }


    /* special EQUAL symbol */

    if (u_strcmp(buf, "<=>") == 0) { return new_symbol(EQUAL); }


    /* etiquette incomplete */

    return load_gram_symbol(lang, label, buf);
  }



  /* special EXCLAM symbol */

  if (*buf == '!' && buf[1] == 0) { return new_symbol(EXCLAM); }


  /* special EQUAL symbol */

  if (*buf == '=' && buf[1] == 0) { return new_symbol(EQUAL); }



  /* ponctuation */

  int idx = language_add_form(lang, buf);

  if (u_strchr(PUNC_TAB, *buf)) {

    if (*buf == '\\' && (! buf[1] || buf[2])) { fatal_error("bad PUNC symbol '%S'\n", label); }
    if (buf[1] && buf[0] != '\\') { fatal_error("bad symbol '%S' (PONC too long)\n", label); }

    return new_symbol_PUNC(lang, idx);
  }


  /* chiffre arabe */

  if (u_is_digit(*buf)) {

    for (unichar * p = buf; *p; p ++) {
      if (! u_is_digit(*p)) { fatal_error("bad symbol : '%S' (mixed nums and chars)\n", label); }
    }

    return new_symbol_CHFA(lang, idx);
  }


  /* unknow word  */

  error("label '%S': unknow word in grammar???\n", label);

  return new_symbol_UNKNOWN(lang, idx);
}



#if 0
static bool symbol_equals_traits(symbol_t * a, symbol_t * b) {

  if (b->nb_features < a->nb_features) {
    symbol_t * tmp = a;
    a = b;
    b = tmp;
  }

  int i;
  for (i = 0; i < a->nb_features; i++) {
    if (a->values[i] != b->values[i]) { return false; }
  }

  for (; i < b->nb_features; i++) {
    if (b->values[i] != UNSPECIFIED) { return false; }
  }
  return true;
}


bool symbol_equals(symbol_t * a, symbol_t * b) {

  if (a == b) { return true; }

  if (a == NULL || b == NULL || a == SYMBOL_DEF || b == SYMBOL_DEF) { return false; }

  if ((a->type != b->type) || (a->POS != b->POS)) { return false; }

  if (a->canonic) {

    if ((! b->canonic) || u_strcmp(a->canonic, b->canonic)) { return false; }

  } else if (b->canonic) { return false; }

  return symbol_equals_traits(a, b);
}
#endif
