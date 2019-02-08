/*
 * Unitex
 *
 * Copyright (C) 2001-2019 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "Symbol.h"
#include "Symbol_op.h"
#include "StringParsing.h"
#include "List_int.h"
#include "DELA.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/* see http://en.wikipedia.org/wiki/Variable_Length_Array . MSVC did not support it
   see http://msdn.microsoft.com/en-us/library/zb1574zs(VS.80).aspx */
#if defined(_MSC_VER) && (!(defined(NO_C99_VARIABLE_LENGTH_ARRAY)))
#define NO_C99_VARIABLE_LENGTH_ARRAY 1
#endif

#define MAXBUF 1024


const unichar PUNC_TAB[] = {
  '"', '\'',
  '+', '-', '*', '\\', '=',
  '.', ',', ':', ';', '!', '?',
  '(', ')', '[', ']', '<', '>', '{', '}',
  '%', '#', '@', '/', '$', '&',
  '|', '_',
  0xAB,0xBB,
    0
};


/**
 * Allocates, initializes and returns a new symbol_t corresponding to the
 * given type.
 */
symbol_t* new_symbol(SymbolType type,int tag_number) {
symbol_t* symbol=(symbol_t*)malloc(sizeof(symbol_t));
if (symbol==NULL) {
   fatal_alloc_error("new_symbol");
}
symbol->type=type;
symbol->negative=false;
symbol->form=0;
symbol->lemma=0;
symbol->POS=NULL;
symbol->feature=NULL;
symbol->nb_features=0;
symbol->tfsttag_index=tag_number;
symbol->next=NULL;
return symbol;
}


/**
 * Allocates, initializes and returns a new symbol_t corresponding to the
 * given POS description. The difference with 'new_symbol' is that this one
 * does not call 'symbol_type', in order to avoid infinite loops.
 */
symbol_t* new_symbol_POS_no_type(POS_t* POS,int tag_number) {
symbol_t* symbol=new_symbol(S_INC,tag_number);
symbol->POS=POS;
if (POS->CATs->size!=0) {
   symbol->feature=(char*)malloc(POS->CATs->size*sizeof(char));
   if (symbol->feature==NULL) {
      fatal_alloc_error("new_symbol_POS_no_type");
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
symbol_t* new_symbol_POS(POS_t* POS,int tag_number) {
symbol_t* s=new_symbol_POS_no_type(POS,tag_number);
type_symbol(s);
return s;
}


/**
 * Allocates, initializes and returns a new symbol_t corresponding to the
 * <PNC> tag in ELAG grammars.
 */
symbol_t* new_symbol_PUNC(language_t* language,int canonic,int tag_number) {
POS_t* POS=language_get_POS(language,PUNC_STR);
symbol_t* symbol=new_symbol_POS(POS,tag_number);
symbol->type=S_ATOM;
symbol->negative=false;
symbol->form=0;
symbol->lemma=canonic;
return symbol;
}


/**
 * Allocates, initializes and returns a new symbol_t corresponding to the
 * <NB> tag in ELAG grammars.
 */
symbol_t* new_symbol_CHFA(language_t* language,int canonic,int tag_number) {
POS_t* POS=language_get_POS(language,CHFA_STR);
symbol_t* symbol=new_symbol_POS(POS,tag_number);
symbol->type=S_ATOM;
symbol->negative=false;
symbol->form=0;
symbol->lemma=canonic;
return symbol;
}


/**
 * Allocates, initializes and returns a new symbol_t corresponding to the
 * <?> tag in ELAG grammars.
 */
symbol_t* new_symbol_UNKNOWN(language_t* language,int form,int tag_number) {
POS_t* POS=language_get_POS(language,UNKNOWN_STR);
symbol_t* symbol=new_symbol_POS(POS,tag_number);
symbol->type=S_ATOM;
symbol->negative=false;
symbol->form=0;
symbol->lemma=form;
return symbol;
}


/**
 * Reinitializes the fields of the given symbol_t.
 */
void empty_symbol(symbol_t* symbol) {
if (symbol->feature!=NULL) free(symbol->feature);
if (symbol->negative) free(symbol->negs);
symbol->type=S_UNTYPED;
symbol->negative=false;
symbol->form=0;
symbol->lemma=0;
symbol->POS=NULL;
symbol->feature=NULL;
symbol->nb_features=0;
symbol->tfsttag_index=-1;
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
      fatal_alloc_error("copy_symbol");
   }
   for (i=0;i<dest->nbnegs;i++) {
      dest->negs[i]=src->negs[i];
   }
} else {
   dest->form=src->form;
   dest->lemma=src->lemma;
}
dest->POS=src->POS;
dest->feature=(char*)malloc(src->nb_features*sizeof(char));
if (dest->feature==NULL) {
   fatal_alloc_error("copy_symbol");
}
for (i=0;i<src->nb_features;i++) {
   dest->feature[i]=src->feature[i];
}
dest->nb_features=src->nb_features;
dest->tfsttag_index=src->tfsttag_index;
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
   case S_LEXIC:
      res=new_symbol(S_LEXIC,symbol->tfsttag_index);
      break;
   case S_EPSILON:
      res=new_symbol(S_EPSILON,symbol->tfsttag_index);
      break;
   case S_ATOM:
   case S_INC_CAN:
   case S_CODE:
   case S_INC:
   case S_INC_NEG:
   case S_CODE_NEG:
      res=new_symbol_POS(symbol->POS,symbol->tfsttag_index);
      res->type=symbol->type;
      if (!symbol->negative) {
         res->form=symbol->form;
         res->lemma=symbol->lemma;
      } else {
         res->negative=true;
         res->nbnegs=symbol->nbnegs;
         res->negs=(int*)malloc(res->nbnegs*sizeof(int));
         if (res->negs==NULL) {
            fatal_alloc_error("dup_symbol");
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
   case S_EXCLAM:
      res=new_symbol(S_EXCLAM,symbol->tfsttag_index);
      break;
   case S_EQUAL:
      res=new_symbol(S_EQUAL,symbol->tfsttag_index);
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
 * Returns an identical duplicate of the given symbol list, in same order
 */
symbol_t* dup_symbols_identical(const symbol_t* symbols) {
    symbol_t* ret = NULL;
    symbol_t* prev = NULL;
    while (symbols!=NULL) {
        symbol_t* new_item = dup_symbol(symbols);
        if (ret == NULL)
        {
            ret = new_item;
        }
        else
        {
            prev->next = new_item;
        }
        prev=new_item;
        symbols=symbols->next;
    }
    return ret;
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


/**
 * Returns how many codes in POS match with 's'. For instance, if we have the French
 * tag "<V:s>", it can match all the following codes:
 *
 * C <pers> s
 * F <pers> s
 * I <pers> s
 * J <pers> s
 * P <pers> s
 * S <pers> s
 * T <pers> s
 * X 1 s   # euss� duss� puiss� fuss�
 * Y 2 s
 * K <gender> s
 *
 * If 'matching_code' is not NULL, we copy the first matching code in it. If
 * there are several matching codes, we set the common values and we set to
 * UNSPECIFIED the divergent values. In our example, we would have:
 *
 * tense=UNSPECIFIED pers=UNSPECIFIED gender=UNSPECIFIED number='s'
 */
int symbol_match_codes(symbol_t* s,symbol_t* matching_code) {
int count=0;
/* We look at every complete code for s's POS */
for (symbol_t* code=s->POS->codes;code!=NULL;code=code->next) {
   bool ok=true;
   for (int i=0;ok && i<s->POS->nb_discr;i++) {
      switch (code->feature[i]) {
         case UNSPECIFIED: {
            /* This should never happen */
            fatal_error("symbol_match_codes: in POS '%S': code with UNSPECIFIED discriminative feature\n",s->POS->name);
         }
         default:
            if (s->feature[i]!=UNSPECIFIED && s->feature[i]!=code->feature[i]) {
               /* If a feature is set with different values in the current code
                * and in s, then s cannot match this code */
               ok=false;
            }
         break;
      }
   }
   if (ok) {
      /* If s can match the current code */
      if (matching_code!=NULL) {
         if (count==0) {
            /* We copy the first matching code's features in 'matching_code' */
            for (int i=0;i<s->POS->nb_discr;i++) {
               matching_code->feature[i]=code->feature[i];
            }
         } else {
            /* If there are more than one matching code, we set to
             * UNSPECiFIED the features that are divergent */
            for (int i=0;i<s->POS->nb_discr;i++) {
               if (matching_code->feature[i]!=code->feature[i]) {
                  matching_code->feature[i]=UNSPECIFIED;
               }
            }
         }
      }
      count++;
   }
}
return count;
}


/**
 * This function takes a symbol and compares it to the valid symbols that are
 * possible for the symbol's POS. For instance, if an adjective must be defined
 * with a number and a gender, then the symbol corresponding to "<A:m>" will
 * be typed as an incomplete one. The function returns -1 if the symbol is
 * not a correct one; otherwise, the type of the symbol is returned (see
 * the enum SymbolType).
 *
 * If the lemma form was a negative list, then the symbol type will be
 * INC_NEG or CODE_NEG.
 * If the feature values match with only one full code, then its a CODE or ATOM
 * (depending if it contains a lemma).
 * If it doesn't match any code then it is an invalid symbol.
 * If all discriminative features are fixed then it is also a CODE (or ATOM).
 * Otherwise, it is an incomplete code (INC).
 */
int type_symbol(symbol_t* symbol) {
/* By default, a symbol is incomplete */
symbol->type=S_INC;
if (symbol->POS->codes!=NULL) {
   /* If there are feature combinations to look at */
   symbol_t* matching_code=new_symbol_POS_no_type(symbol->POS,-1);
   int count=symbol_match_codes(symbol,matching_code);
   if (count==0) {
      /* If the symbol doesn't match any POS code, then it is an invalid one */
      symbol->type=S_UNTYPED;
      free_symbol(matching_code);
      return symbol->type;
   }
   if (count==1) {
      /* If the symbol corresponds to one code, we set its discriminative
       * features as the matching code's one, so that the symbol is no more
       * an INC but a CODE */
      symbol->type=S_CODE;
      for (int i=0;i<symbol->POS->nb_discr;i++) {
         symbol->feature[i]=matching_code->feature[i];
      }
   } else {
      /* If the symbol matches more than one code, we try to lock some features
       * that have same values on all matching codes. Moreover, the symbol is still
       * an incomplete one. */
      for (int i=0;i<symbol->POS->nb_discr;i++) {
         if (symbol->feature[i]==UNSPECIFIED) {
            symbol->feature[i]=matching_code->feature[i];
         }
      }
   }
   free_symbol(matching_code);
}
if (symbol->type==S_INC) {
   /* If the symbol is not yet marked as a CODE, we check if some discriminative
    * features remain UNSPECIFIED, because if (count>1) above, we may have set
    * some feature values. */
   int i;
   for (i=0;i<symbol->POS->nb_discr;i++) {
      if (symbol->feature[i]==UNSPECIFIED) {
         break;
      }
   }
   if (i==symbol->POS->nb_discr) {
      /* If we find that all dicriminative features have been set, then
       * we promote the symbol to CODE */
      symbol->type=S_CODE;
   }
}
/* Now we look for a lemma, firstly in the case of an INC symbol... */
if (symbol->type==S_INC) {
   if (symbol->negative) {
      symbol->type=S_INC_NEG;
   } else if (symbol->lemma) {
      symbol->type=S_INC_CAN;
   }
   return symbol->type;
}
/* ...and then for a CODE symbol */
if (symbol->negative) {
   symbol->type=S_CODE_NEG;
} else if (symbol->lemma) {
   symbol->type=S_ATOM;
}
return symbol->type;
}



void symbol_dump_all(language_t* language,const symbol_t * symb) {

  static const unichar locked[] = { 'l', 'o', 'c', 'k', 'e', 'd', 0 };

  int i;

  while (symb!=NULL) {

  language_t * lang = symb->POS ? symb->POS->language : language;

  u_printf("<%c:", symb->type);

  if (symb->negative) {
    for (i = 0; i < symb->nbnegs; i++) {
      u_printf("!%S", language_get_form(lang, symb->negs[i]));
    }
  } else {
    u_printf("%S,%S", language_get_form(lang, symb->form), language_get_form(lang, symb->lemma));
  }


  if (symb->POS) {
    u_printf(".%S", symb->POS->name);

    for (i = symb->POS->nb_inflect; i < symb->POS->CATs->size; i++) {
      CAT_t * CAT = POS_get_CAT(symb->POS,i);
      u_printf("+%S=%S", CAT->name, (symb->feature[i]<0) ? locked : (unichar *) CAT->values->value[(int)symb->feature[i]]);
    }

    u_printf(":");

    for (i = 0; i < symb->POS->nb_inflect; i++) {
      CAT_t * CAT = POS_get_CAT(symb->POS,i);
      u_printf("+%S=%S", CAT->name, (symb->feature[i] < 0) ? locked : (unichar *) CAT->values->value[(int)symb->feature[i]]);
    }

  } else {
    u_printf(".*");
  }

  u_printf("> ");

  symb=symb->next;
  }

}


/**
 * This function converts a the given symbol into a text .fst2 tag.
 */
void symbol_to_text_label(const symbol_t* s,Ustring* ustr) {
if (s==NULL) {
   fatal_error("NULL error in symbol_to_text_label\n");
}
if (s==SYMBOL_DEF) {
   fatal_error("Unexpected <def> symbol in symbol_to_text_label\n");
}
if (s->type!=S_ATOM) {
   error("symbol_to_text_label: symbol '");
   symbol_dump(s);
   fatal_error("' is'nt an atom.\n");
}
language_t* language=s->POS->language;
if (!u_strcmp(s->POS->name,UNKNOWN_STR) || !u_strcmp(s->POS->name,PUNC_STR)
   || !u_strcmp(s->POS->name,CHFA_STR)) {
   /* If the symbol is an unknown word, a digit sequence or a punctuation mark */
   u_strcpy(ustr,language_get_form(language,s->lemma));
   return;
}
u_sprintf(ustr,"{%S,%S.%S",language_get_form(language,s->form),language_get_form(language,s->lemma),s->POS->name);
int i;
/* We concatenate the semantic features like "+hum+z1" */
CAT_t* CAT;
for (i=s->POS->nb_inflect;i<s->nb_features;i++) {
   if (s->feature[i]>0) {
      CAT=POS_get_CAT(s->POS,i);
      u_strcatf(ustr,"+%S",CAT_get_valname(CAT,s->feature[i]));
   }
}
/* Then we deal with the inflectional features */
bool colons=false;
for (i=0;i<s->POS->nb_inflect;i++) {
   CAT=POS_get_CAT(s->POS,i);
   if (s->feature[i]>0) {
      if (colons==false) {
         u_strcat(ustr,":");
         colons=true;
      }
      u_strcat(ustr,CAT_get_valname(CAT,s->feature[i]));
   }
}
u_strcat(ustr,"}");
}


/**
 * This function converts a the given symbol into a text tag.
 * If the symbol is a list of several symbols corresponding to several
 * inflectional codes, the resulting entry will aggregate them like:
 *
 * {fait,faire.V:Kms:P3s}
 *
 * NOTE: if this function is called on a symbol that is not a dictionary
 *       entry, it will return NULL.
 */
struct dela_entry* symbol_to_dela_entry(const symbol_t* s) {
if (s==NULL) {
   fatal_error("NULL error in symbol_to_dela_entry\n");
}
if (s==SYMBOL_DEF) {
   fatal_error("Unexpected <def> symbol in symbol_to_dela_entry\n");
}
if (s->type!=S_ATOM) {
   error("symbol_to_dela_entry: symbol '");
   symbol_dump(s);
   fatal_error("' is'nt an atom.\n");
}
language_t* language=s->POS->language;
if (!u_strcmp(s->POS->name,UNKNOWN_STR) || !u_strcmp(s->POS->name,PUNC_STR)
   || !u_strcmp(s->POS->name,CHFA_STR)) {
   /* If the symbol is an unknown word, a digit sequence or a punctuation mark */
   return NULL;
}
struct dela_entry* e=new_dela_entry(language_get_form(language,s->form),language_get_form(language,s->lemma),s->POS->name);
int i;
/* We concatenate the semantic features like "+hum+z1" */
CAT_t* CAT;
for (i=s->POS->nb_inflect;i<s->nb_features;i++) {
   if (s->feature[i]>0) {
      CAT=POS_get_CAT(s->POS,i);
      e->semantic_codes[(e->n_semantic_codes)++]=u_strdup(CAT_get_valname(CAT,s->feature[i]));
   }
}
/* Then we deal with the inflectional features */
unichar foo[128];
foo[0]='\0';
for (i=0;i<s->POS->nb_inflect;i++) {
   CAT=POS_get_CAT(s->POS,i);
   if (s->feature[i]>0) {
      u_strcat(foo,CAT_get_valname(CAT,s->feature[i]));
   }
}
if (foo[0]!='\0') {
   e->inflectional_codes[(e->n_inflectional_codes)++]=u_strdup(foo);
}
while (s->next!=NULL) {
   s=s->next;
   foo[0]='\0';
   for (i=0;i<s->POS->nb_inflect;i++) {
      CAT=POS_get_CAT(s->POS,i);
      if (s->feature[i]>0) {
         u_strcat(foo,CAT_get_valname(CAT,s->feature[i]));
      }
   }
   e->inflectional_codes[(e->n_inflectional_codes)++]=u_strdup(foo);
}
return e;
}



/**
 * This function converts a the given symbol into a .fst2 grammar tag one.
 */
void symbol_to_locate_label(const symbol_t* s,Ustring * ustr) {
if (s==NULL) {
   fatal_error("NULL error in symbol_to_locate_label\n");
}
if (s==SYMBOL_DEF) {
   fatal_error("Unexpected default transition in symbol_to_locate_label\n");
}
/* First, we deal with special tags */
switch (s->type) {
   case S_LEXIC:
      /* We copy a special tag that will be rewritten later */
      u_strcpy(ustr,"<.>");
      return;

   case S_EPSILON:
      u_strcpy(ustr,"<E>");
      return;

   case S_EXCLAM:
      fatal_error("Unexpected <!> tag in symbol_to_locate_label\n");

   case S_EQUAL:
      fatal_error("Unexpected <=> tag in symbol_to_locate_label\n");

   default: ; /* nothing to do: just want to avoid a warning */
}
language_t* lang=s->POS->language;
if (!u_strcmp(s->POS->name,UNKNOWN_STR)) {
   /* If we have an unknown POS */
   u_strcpy(ustr, "<!DIC>");
   return;
}
if (!u_strcmp(s->POS->name,PUNC_STR) || !u_strcmp(s->POS->name,CHFA_STR)) {
   /* If we have a punctuation or a digit sequence */
   if (s->lemma!=0) {
      u_strcpy(ustr,language_get_form(lang,s->lemma));
      return;
   }
   /* We can get here if we have <PNC>, but we don't have to worry since the special case
    * of <PNC> is dealt with in fst_file_write */
}
if (!s->negative && s->lemma!=0) {
   u_sprintf(ustr,"<%S.%S",language_get_form(lang,s->lemma),s->POS->name);
} else {
   u_sprintf(ustr,"<%S",s->POS->name);
}
int i;
CAT_t* CAT;
for (i=s->POS->nb_inflect;i<s->nb_features;i++) {
   if (s->feature[i]>0) {
      CAT=POS_get_CAT(s->POS,i);
      u_strcatf(ustr,"+%S",CAT_get_valname(CAT,s->feature[i]));
   }
}
bool colons=false;
for (i=0;i<s->POS->nb_inflect;i++) {
   CAT=POS_get_CAT(s->POS,i);
   if (s->feature[i]>0) {
      if (colons==false) {
         u_strcat(ustr,":");
         colons=true;
      }
      u_strcat(ustr,CAT_get_valname(CAT,s->feature[i]));
   }
}
u_strcat(ustr,">");
}



void symbol_to_grammar_label(const symbol_t * s, Ustring * ustr) {

  //  debug("symbtogrammlabel\n"); symbol_dump(s); endl();

  if (s == NULL) { fatal_error("symb2grammar label: symb is null\n"); }
  if (s == SYMBOL_DEF) { u_strcpy(ustr, "<def>"); return; }

  switch (s->type) {
  case S_LEXIC:
    u_strcpy(ustr, "<.>");
    return;
  case S_EPSILON:
    u_strcpy(ustr, "<E>");
    return;
  case S_EXCLAM:
    u_strcpy(ustr, "<!>");
    return;
  case S_EQUAL:
    u_strcpy(ustr, "<=>");
    return;
  default: ; /* nothing to do: just want to avoid a warning */
  }

  unichar tmp[1024];

  language_t * lang = s->POS->language;
  if (s->negative) {

    u_strcpy(ustr, "<");
    for (int i = 0; i < s->nbnegs; i++) {
       escape(language_get_form(lang, s->negs[i]),tmp,P_ELAG_TAG);
      u_strcatf(ustr, "!%S",tmp);
    }
    u_strcatf(ustr, ".%S",s->POS->name);

  } else if (s->lemma) {

     escape(language_get_form(lang, s->lemma),tmp,P_ELAG_TAG);
    u_sprintf(ustr, "<%S.%S",tmp,s->POS->name);

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


/**
 * Builds a representation of the given symbol.
 */
void symbol_to_str(const symbol_t* s,Ustring* ustr) {
/* First, we deal with special symbols */
if (s==NULL) {
   u_strcpy(ustr,"nil");
   return;
}
if (s==SYMBOL_DEF) {
   u_strcpy(ustr,"<def>");
   return;
}
switch (s->type) {
   case S_LEXIC: u_strcpy(ustr,"<.>"); return;
   case S_EPSILON: u_strcpy(ustr,"<E>"); return;
   case S_EXCLAM: u_strcpy(ustr,"<!>"); return;
   case S_EQUAL: u_strcpy(ustr,"<=>"); return;
   default: ; /* nothing to do: just want to avoid a warning */
}
/* Then, we process other symbols */
language_t* lang=s->POS->language;
u_sprintf(ustr,"<%c:",s->type);
if (s->negative) {
   /* If the symbol is a negative one, we print the
    * list of forbidden lemmas */
   for (int i=0;i<s->nbnegs;i++) {
      u_strcatf(ustr,"!%S(%d)",language_get_form(lang,s->negs[i]),s->negs[i]);
   }
} else {
   /* If the symbol is a positive one, we may have to print
    * an inflected form and/or a lemma */
   if (s->form) {
      u_strcatf(ustr,"%S,",language_get_form(lang,s->form));
   }
   if (s->lemma) {
      u_strcatf(ustr,"%S",language_get_form(lang,s->lemma));
   }
}
/* Anyway we print the POS code */
u_strcatf(ustr,".%S",s->POS->name);
int i;
CAT_t* CAT;
/* Then we deal with the semantic codes like z1 */
for (i=s->POS->nb_inflect;i<s->nb_features;i++) {
   if (s->feature[i]>0) {
      CAT=POS_get_CAT(s->POS,i);
      u_strcatf(ustr,"+%S",CAT_get_valname(CAT,s->feature[i]));
   } else if (s->feature[i]==LOCKED) {
      CAT=POS_get_CAT(s->POS,i);
      u_strcatf(ustr,"!%S",CAT->name);
   }
}
/* Then we deal with the inflectional codes */
u_strcat(ustr,":");
for (i=0;i<s->POS->nb_inflect;i++) {
   CAT=POS_get_CAT(s->POS,i);
   if (s->feature[i]>0) {
      u_strcat(ustr,CAT_get_valname(CAT,s->feature[i]));
   } else if (s->feature[i]==LOCKED) {
      u_strcatf(ustr,"!{%S}",CAT->name);
   }
}
u_strcat(ustr,">");
}


void symbol_dump(const symbol_t * s) {
  Ustring * ustr = new_Ustring();
  symbol_to_str(s, ustr);
  u_printf("%S", ustr->str);
  free_Ustring(ustr);
}


void symbols_dump(const symbol_t * s) {

  u_printf("(");
  while (s) { symbol_dump(s); s = ((s == SYMBOL_DEF) ? NULL : s->next); if (s) { u_printf(", "); } }
  u_printf(")");
}




/**
 * This function checks whether the given string is a valid DELAF tag token or not.
 */
// this function is commented because it is unused
/*
static inline int check_text_label(unichar* label) {
if (label==NULL) {
   error("NULL error in check_text_label\n");
   return -1;
}
if (label[0]=='\0') {
   error("check_text_label called with an empty label\n");
   return -1;
}
if (label[0]=='{' && label[1]!='\0') {
   // If we have something that looks like a tag token of the form {__,__.__}
   return (check_tag_token(label,1)==1)?0:-1;
}
return 0;
}
*/

/**
 * Loads a tag that is DELAF entry of the form {form,canonic.POS[+traits]*[:flex]*}.
 * 'tag' is the string representing the original tag and 'entry' is a structure
 * representing the entry after tokenization. The function returns a list of symbols
 * for all inflectional codes. For instance, if we have the tag "{rouge,.A:ms:fs}",
 * we will load it as two symbols corresponding to "{rouge,.A:ms}" and "{rouge,.A:fs}".
 */
symbol_t* load_dic_entry(language_t* language,unichar* tag,struct dela_entry* entry,int tag_number) {
int i;
POS_t* POS=language_get_POS(language,entry->semantic_codes[0]);
if (POS==NULL) {
   if (get_value_index(entry->semantic_codes[0],language->unknown_codes,DONT_INSERT)==-1) {
      unichar tmp[4096];
      if (tag==NULL) {
         build_tag(entry,NULL,tmp);
         tag=tmp;
      }
      error("'%S': unknown POS '%S'\n",tag,entry->semantic_codes[0]);
      get_value_index(entry->semantic_codes[0],language->unknown_codes,INSERT_IF_NEEDED,NULL);
   }
   return NULL;
}
symbol_t* symbol=new_symbol_POS(POS,tag_number);
symbol_t* model;
symbol->type=S_ATOM;
symbol->form=language_add_form(language,entry->inflected);
symbol->lemma=language_add_form(language,entry->lemma);
/* We lock all the features that are not explicitly set in the tag */
for (i=0;i<POS->CATs->size;i++) {
   symbol->feature[i]=LOCKED;
}
/* We consider the semantic feature like +hum+z1 */
for (i=1;i<entry->n_semantic_codes;i++) {
   feature_info_t* info=POS_get_semantic_feature_infos(POS,entry->semantic_codes[i]);
   if (info!=NULL) {
      symbol->feature[info->CATid]=info->val;
   } else {
      if (get_value_index(entry->semantic_codes[i],language->unknown_codes,DONT_INSERT)==-1) {
         error("Unknown semantic value '%S', will not be taken into account\n",entry->semantic_codes[i]);
          get_value_index(entry->semantic_codes[i],language->unknown_codes,INSERT_IF_NEEDED,NULL);
      }
   }
}
/* Then we look at the inflectional codes */
if (entry->n_inflectional_codes==0) {
   /* If there is no inflectional code */
   if (POS->codes!=0 && !symbol_match_codes(symbol,NULL)) {
      unichar tmp[4096];
      if (tag==NULL) {
         build_tag(entry,NULL,tmp);
         tag=tmp;
      }
      if (get_value_index(tag,language->unknown_codes,DONT_INSERT)==-1) {
         /* We don't want to print several times the same error message */
         error("'%S': doesn't match with POS '%S' definition\n",tag,POS->name);
         get_value_index(tag,language->unknown_codes,INSERT_IF_NEEDED,NULL);
      }
      free_symbols(symbol);
      return NULL;
   }
   return symbol;
}
model=symbol;
if (model->next!=NULL) {
    fatal_error("load_dic_entry: symbol list should not happen\n");
}
symbol=NULL;
for (i=0;i<entry->n_inflectional_codes;i++) {
   symbol_t* tmp=dup_symbol(model);
   unichar* p=entry->inflectional_codes[i];
   for (;*p!='\0';p++) {
      feature_info_t* infos=POS_get_inflect_feature_infos(POS,*p);
      if (infos==NULL) {
         unichar tmp_tag[4096];
         if (tag==NULL) {
            build_tag(entry,NULL,tmp_tag);
            tag=tmp_tag;
         }
         error("'%S': unknown inflectional code '%C'\n",tag,*p);
         free_symbol(tmp);
         free_symbol(model);
         free_symbols(symbol);
         return NULL;
      }
      tmp->feature[infos->CATid]=infos->val;
   }
   if (POS->codes!=0 && !symbol_match_codes(tmp,NULL)) {
      unichar tmp_tag[4096];
      if (tag==NULL) {
         build_tag(entry,NULL,tmp_tag);
         tag=tmp_tag;
      }
      if (get_value_index(tag,language->unknown_codes,DONT_INSERT)==-1) {
         /* We don't want to print several times the same error message */
         error("'%S': doesn't match with POS '%S' definition\n",tag,POS->name);
         get_value_index(tag,language->unknown_codes,INSERT_IF_NEEDED,NULL);
      }
      free_symbol(tmp);
      free_symbol(model);
      free_symbols(symbol);
      return NULL;
   }
   tmp->next=symbol;
   symbol=tmp;
}
free_symbol(model);
return symbol;
}


/**
 * Loads a tag from a text .tfst and returns the associated symbol or NULL
 * if the tag is not valid. The given tag number is used to remember the original
 * TfstTag that corresponds to the symbol_t*, because we will need this information
 * when saving the output .tfst.
 */
symbol_t* load_text_symbol(language_t* language,unichar* tag,int tag_number) {
if (tag==NULL) {
   error("NULL error in load_text_symbol\n");
   return NULL;
}
if (tag[0]=='\0') {
   error("load_text_symbol called with an empty tag\n");
   return NULL;
}
if (!u_strcmp(tag,"<E>")) {
   return new_symbol(S_EPSILON,tag_number);
}
if (!u_strcmp(tag,"<def>")) {
   fatal_error("Unexpected '<def>' tag in text automaton\n");
}
if (tag[0]=='{' && tag[1]!='\0') {
   /*  If we have something that looks like a dictionary entry of the form {__,__.__} */
   struct dela_entry* entry=tokenize_tag_token(tag,1);
   if (entry==NULL) {
      fatal_error("Cannot load invalid tag '%S'\n",tag);
   }
   symbol_t* result=load_dic_entry(language,tag,entry,tag_number);
   if (result==NULL) {
      /* Nothing to do: an error message has already been printed */
   } else {
      result->tfsttag_index=tag_number;
   }
   free_dela_entry(entry);
   return result;
}
/* If the tag is not a DELAF entry, then it may be an unknown word or
 * a punctuation mark */
int index=language_add_form(language,tag);
if (u_strchr(PUNC_TAB,tag[0])) {
   /* If the tag looks like a punctuation mark */
   if (tag[1]!='\0' && tag[0]!='\\') {
      fatal_error("Bad text symbol '%S' (ponctuation too long)\n",tag);
   }
   return new_symbol_PUNC(language,index,tag_number);
}
/* If we have a digit sequence */
if (u_is_digit(tag[0])) {
   for (int i=0;tag[i]!='\0';i++) {
      if (!u_is_digit(tag[i])) {
         fatal_error("Bad symbol : '%S' (mixed digits and characters)\n",tag);
      }
   }
   return new_symbol_CHFA(language, index,tag_number);
}
/* If we have an unknown word  */
return new_symbol_UNKNOWN(language,index,tag_number);
}


/**
 * This function returns a list that contains symbols for all POS except
 * the given one. It is used when there is a negative tag like <!A>.
 */
symbol_t* LEXIC_minus_POS(language_t* language,POS_t* POS) {
symbol_t res;
res.next=NULL;
symbol_t* end=&res;
for (int i=0;i<language->POSs->size;i++) {
   POS_t* POS2=(POS_t*)language->POSs->value[i];
   if (POS2==POS) {
      continue;
   }
   concat_symbols(end,new_symbol_POS(POS2,-1),&end);
}
return res.next;
}


/**
 * This function takes a string that represent a negative lemma list like
 * "!man!woman". It adds into the given symbol their indices in the language's
 * forms. Note that the array will be sorted.
 */
void fill_negative_lemma_list(language_t* language,symbol_t* s,unichar* lemma) {
struct list_int* list=NULL;
/* We take a buffer large enough */
#ifdef NO_C99_VARIABLE_LENGTH_ARRAY
unichar *tmp = (unichar*)malloc(sizeof(unichar)*(u_strlen(lemma)+1));
#else
unichar tmp[u_strlen(lemma)+1];
#endif
int position=0;
while (lemma[position]!='\0') {
   if (lemma[position]!='!') {
      fatal_error("Malformed negative lemma list: '%S'\n",lemma);
   }
   /* We skip the ! */
   position++;
   if (P_OK!=parse_string(lemma,&position,tmp,P_EXCLAMATION)) {
      fatal_error("Malformed negative lemma list: '%S'\n",lemma);
   }
   int index=language_add_form(language,tmp);
   list=sorted_insert(index,list);
}
s->negs=dump(list,&(s->nbnegs));
free_list_int(list);
#ifdef NO_C99_VARIABLE_LENGTH_ARRAY
free(tmp);
#endif
}


/**
 * Loads a grammar symbol which has the generic form '<canonic.POS+trait:flex>'.
 * The function returns a list of symbols for all inflectional codes. For
 * instance, if we have the tag "<A:@m@s:fs>", we will load it as two
 * symbols corresponding to "<A:@m@s>" and "<A:fs>".
 */
symbol_t* load_gram_symbol(language_t* language,const unichar* tag) {
int length=u_strlen(tag);
if (tag[0]!='<' || tag[length-1]!='>') {
   fatal_error("Bad grammar symbol: '%S'\n",tag);
}
/* We copy the content of the label without the angles */
unichar* buffer=u_strdup(&(tag[1]),length-2);
#ifdef NO_C99_VARIABLE_LENGTH_ARRAY
unichar *lemma = (unichar *)malloc(sizeof(unichar)*(length-1));
unichar *tmp = (unichar *)malloc(sizeof(unichar)*(length-1));
#else
unichar lemma[length-1];
unichar tmp[length-1];
#endif
int position=0;
/* We look for a dot in the tag */
if (P_OK!=parse_string(buffer,&position,lemma,P_DOT)) {
   fatal_error("Bad grammar symbol: '%S'\n",tag);
}
if (buffer[position]!='.') {
   /* If we haven't found one, it means that the tag has no lemma*/
   lemma[0]='\0';
   position=0;
} else {
   /* We skip the dot */
   position++;
}
if (buffer[position]=='!') {
   /* If we have the negation of a grammatical code like <!PNC> */
   if (position!=0) {
      error("Unexpected lemma in negative grammar tag '%S': lemma will be ignored\n",tag,lemma);
   }
   /* We assume that the whole code after the ! is a grammatical code. If not (i.e. <!N+z1>),
    * it will wrongly be taken as one code and it won't be found in the valid POS */
   POS_t* POS=language_get_POS(language,&(buffer[position+1]));
   if (POS==NULL) {
      fatal_error("In symbol '%S': unknown part of speech '%S'\n",tag,buffer[position+1]);
   }
   free(buffer);
#ifdef NO_C99_VARIABLE_LENGTH_ARRAY
   free(lemma);
   free(tmp);
#endif
   return LEXIC_minus_POS(language,POS);
}
/* We look for the end of the POS in the tag */
if (P_OK!=parse_string(buffer,&position,tmp,"+!:")) {
   fatal_error("Bad grammar symbol: '%S'\n",tag);
}
POS_t* POS=language_get_POS(language,tmp);
if (POS==NULL) {
   fatal_error("In symbol '%S': unknown part of speech '%S'\n",tag,tmp);
}
symbol_t* symbol=new_symbol_POS(POS,-1);
if (lemma[0]!='\0') {
   /* If the lemma is not empty */
   if (lemma[0]!='!') {
      /* And if it's not a negative lemma */
      symbol->lemma=language_add_form(language,lemma);
   } else {
      /* If we have a negative lemma list like in <!man!woman.N>*/
      symbol->negative=true;
      fill_negative_lemma_list(language,symbol,lemma);
   }
}
/* Now, we deal with the additional semantic features, if any */
while (buffer[position]=='+' || buffer[position]=='!') {
   /* We skip the delimiter, but we keep it */
   unichar type=buffer[position];
   position++;
   /* We look for the end of the feature in the tag */
   if (P_OK!=parse_string(buffer,&position,tmp,"+!:")) {
      fatal_error("Bad grammar symbol: '%S'\n",tag);
   }
   if (type=='+') {
      /* If we have to set a semantic feature */
      feature_info_t* info=POS_get_semantic_feature_infos(POS,tmp);
      if (info!=NULL) {
          symbol->feature[info->CATid]=info->val;
      } else {
         if (get_value_index(tmp,language->unknown_codes,DONT_INSERT)==-1) {
            /* If we haven't already encountered this unknown code, we can print
             * an error message */
            error("in symbol '%S': unknow attribute '%S', will not be taken into account\n",tag,tmp);
            get_value_index(tmp,language->unknown_codes,INSERT_IF_NEEDED,NULL);
         }
      }
   } else {
      /* If we have to lock a semantic feature */
      int index=POS_get_CATid(POS,tmp);
      if (index==-1) {
         fatal_error("In symbol '%S': unknown feature '%S'\n",tag,tmp);
      }
      if (index<POS->nb_inflect) {
         fatal_error("In symbol '%S': '%S' is an inflectional feature!\n",tag,tmp);
      }
      if (symbol->feature[index]!=UNSPECIFIED) {
         fatal_error("In symbol '%S': '%S' cannot be both locked and set\n",tag,tmp);
      }
      symbol->feature[index]=LOCKED;
   }
}
/* Now we look at the inflectional codes, if any */
if (buffer[position]=='\0') {
   /* If there are no inflectional codes */
   if (type_symbol(symbol)==-1) {
      fatal_error("'%S' is not a valid tag (1)\n",tag);
   }
   free(buffer);
#ifdef NO_C99_VARIABLE_LENGTH_ARRAY
   free(lemma);
   free(tmp);
#endif
   return symbol;
}
symbol_t* model=symbol;
symbol=NULL;
while (buffer[position]!='\0') {
   /* If there is an inflectional code to read, we look for its end. To
    * do that, we must skip the : */
   position++;
   if (P_OK!=parse_string(buffer,&position,tmp,P_COLON)) {
      fatal_error("Bad grammar symbol: '%S'\n",tag);
   }
   symbol_t* tmp_symbol=dup_symbol(model);
   unichar* attr=tmp;
   for (;*attr!='\0';attr++) {
      feature_info_t* infos;
      if (*attr=='@') {
         /* If the inflectional feature is locked */
         attr++;
         if ((infos=POS_get_inflect_feature_infos(POS,*attr))==NULL) {
            fatal_error("In symbol '%S': unknown inflectional code '%C'\n",tag,*attr);
         }
         tmp_symbol->feature[infos->CATid]=LOCKED;
      } else {
         /* If we have to set the inflectional feature */
         if ((infos=POS_get_inflect_feature_infos(POS,*attr))==NULL) {
            fatal_error("In symbol '%S': unknown inflectional code '%C'\n",tag,*attr);
         }
         tmp_symbol->feature[infos->CATid]=infos->val;
      }
   }
   if (type_symbol(tmp_symbol)==-1) {
      fatal_error("'%S' is not a valid tag (2)\n",tag);
   }
   tmp_symbol->next=symbol;
   symbol=tmp_symbol;
}
free_symbol(model);
free(buffer);
#ifdef NO_C99_VARIABLE_LENGTH_ARRAY
free(lemma);
free(tmp);
#endif
return symbol;
}


/**
 * Loads a symbol from an Elag grammar tag.
 */
symbol_t* load_grammar_symbol(language_t* language,unichar* tag) {
#ifdef NO_C99_VARIABLE_LENGTH_ARRAY
unichar *buf = (unichar *)malloc(sizeof(unichar)*(u_strlen(tag)+1));
#else
unichar buf[u_strlen(tag)+1];
#endif
u_strcpy(buf,tag);
if (tag[0]=='{' && tag[1]!='\0') {
   /* If we have something like a dictionary entry of the form {__,__.__} */
   if (!u_strcmp(tag,"{S}")) {
      /* First we check if it is not a sentence delimiter */
#ifdef NO_C99_VARIABLE_LENGTH_ARRAY
      free(buf);
#endif
      return new_symbol_PUNC(language,language_add_form(language,tag),-1);
   }
   /* If it is really a dictionary entry, it shouldn't be there */
   error("'%S': DELAF entry should not appear in Elag grammar\n",tag);
   struct dela_entry* entry=tokenize_tag_token(tag,1);
   /*if (check_dic_entry(buf)==-1) {
      fatal_error("bad grammar label '%S'\n",tag);
   }*/
   if (entry==NULL) {
      fatal_error("Cannot use invalid tag '%S' in an ELAG grammar\n",tag);
   }
   symbol_t* result=load_dic_entry(language,tag,entry,-1);
   free_dela_entry(entry);
#ifdef NO_C99_VARIABLE_LENGTH_ARRAY
      free(buf);
#endif
   return result;
}
if (tag[0]=='<' && tag[1]!='\0') {
#ifdef NO_C99_VARIABLE_LENGTH_ARRAY
      free(buf);
#endif
   /* If we have a tag like <xxxx>, we test if it's a special symbol */
   if (!u_strcmp(tag,"<E>")) return new_symbol(S_EPSILON,-1);
   if (!u_strcmp(tag,"<.>")) {
       fatal_error("Invalid ELAG tag <.>\n");
   }
   if (!u_strcmp(tag,"<def>")) return SYMBOL_DEF;
   if (!u_strcmp(tag,"<!>")) return new_symbol(S_EXCLAM,-1);
   if (!u_strcmp(tag,"<=>")) return new_symbol(S_EQUAL,-1);
   /* Otherwise, we try to read a tag like <PRO:1s> */
   return load_gram_symbol(language,tag);
}



  /* special EXCLAM symbol */

  if (*buf == '!' && buf[1] == 0) {
#ifdef NO_C99_VARIABLE_LENGTH_ARRAY
      free(buf);
#endif
      return new_symbol(S_EXCLAM,-1);
  }


  /* special EQUAL symbol */

  if (*buf == '=' && buf[1] == 0) {
#ifdef NO_C99_VARIABLE_LENGTH_ARRAY
      free(buf);
#endif
      return new_symbol(S_EQUAL,-1);
  }



  /* ponctuation */

  int idx = language_add_form(language, buf);

  if (u_strchr(PUNC_TAB, *buf)) {

    if (*buf == '\\' && (! buf[1] || buf[2])) {
#ifdef NO_C99_VARIABLE_LENGTH_ARRAY
        free(buf);
#endif
        fatal_error("bad PUNC symbol '%S'\n", tag);
    }
    if (buf[1] && buf[0] != '\\') {
#ifdef NO_C99_VARIABLE_LENGTH_ARRAY
        free(buf);
#endif
        fatal_error("bad symbol '%S' (PONC too long)\n", tag);
    }
#ifdef NO_C99_VARIABLE_LENGTH_ARRAY
      free(buf);
#endif
    return new_symbol_PUNC(language, idx,-1);
  }


  /* chiffre arabe */

  if (u_is_digit(*buf)) {

    for (unichar * p = buf; *p; p ++) {
      if (! u_is_digit(*p)) { fatal_error("bad symbol : '%S' (mixed nums and chars)\n", tag); }
    }

#ifdef NO_C99_VARIABLE_LENGTH_ARRAY
      free(buf);
#endif
    return new_symbol_CHFA(language, idx,-1);
  }


  /* unknow word  */

  error("Label '%S': unknown word in grammar???\n", tag);
#ifdef NO_C99_VARIABLE_LENGTH_ARRAY
      free(buf);
#endif
  return new_symbol_UNKNOWN(language, idx,-1);
}

} // namespace unitex

