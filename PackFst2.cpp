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

/*
 * File created and contributed by Gilles Vollant (Ergonotics SAS)
 * as part of an UNITEX optimization and reliability effort
 *
 * additional information: http://www.smartversion.com/unitex-contribution/
 * contact : info@winimage.com
 *
 */

#include "PackFst2.h"
#include "AbstractAllocatorPlugCallback.h" 
#include "Persistence.h"
#include "UnusedParameter.h"


#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {


int load_persistent_fst2(const char* name) {
  VersatileEncodingConfig vec = VEC_DEFAULT;

  Fst2* f;
  f = read_pack_fst2_from_file(name, NULL);
  if (f == NULL) {
    f = load_fst2(&vec, name, 1, NULL);
  }
  if (f == NULL)
    return 0;
  set_persistent_structure(name, f);
  return 1;
}


void free_persistent_fst2(const char* name) {
  Fst2* f = (Fst2*)get_persistent_structure(name);
  set_persistent_structure(name, NULL);
  free_Fst2(f);
}

/**********************************************************************************/



/*******************************************************************************/
/*******************************************************************************/

#define GET_NEW_ITEM_ALLOCATED_FROM_OLD_NUMBER(x) \
  ((((x) + ((x) / 2)) > ((x) + 0x100)) ? ((x) + ((x) / 2)) : ((x) + 0x100))

/*******************************************************************************/

class pack_store_int {
 public:
  pack_store_int();
  ~pack_store_int();


  bool AddIntValue(int v);
  size_t flush_value_array(void* dst, size_t buf_size);

  int get_nb_total_value() {
    return nb_item_used;
  };

 private:
  int* item_array;
  int nb_item_used;
  int nb_item_allocated;
};


pack_store_int::pack_store_int()
    : item_array(NULL), nb_item_used(0), nb_item_allocated(0) {
}

pack_store_int::~pack_store_int() {
  if (item_array != NULL)
    free(item_array);
}

bool pack_store_int::AddIntValue(int v) {
  if (nb_item_allocated <= nb_item_used) {
    int* new_item_array;
    int new_nb_item_allocated =
        GET_NEW_ITEM_ALLOCATED_FROM_OLD_NUMBER(nb_item_used);
    if (item_array == NULL)
      new_item_array = (int*)malloc(sizeof(int) * new_nb_item_allocated);
    else
      new_item_array =
          (int*)realloc(item_array, sizeof(int) * new_nb_item_allocated);
    if (new_item_array == NULL)
      return false;
    item_array        = new_item_array;
    nb_item_allocated = new_nb_item_allocated;
  }
  *(item_array + nb_item_used) = v;
  nb_item_used++;
  return true;
}


// calculate the number of bytes needed to store an integer
static int get_nb_bytes_int_after(int num) {
  int abs_num = (num >= 0) ? num : (-num);
  if ((abs_num >= 0) && (abs_num <= 0x0f))
    return 0;
  abs_num /= 0x10;
  int nb_bytes = 0;
  while (abs_num > 0) {
    nb_bytes++;
    abs_num = abs_num / 0x100;
  }
  return nb_bytes;
}

static size_t flush_int(int num, void* dst, size_t size_buf) {
  int nb_byte_after_num = get_nb_bytes_int_after(num);
  size_t size_ret       = (size_t)nb_byte_after_num + 1;
  if ((dst != NULL) && (size_buf >= size_ret)) {
    unsigned char* dst_write = (unsigned char*)dst;
    int absolute_num;
    if (num < 0) {
      absolute_num = -num;
    } else {
      absolute_num = num;
    }
    int work_num         = absolute_num;
    unsigned char cFirst = 0;
    if (num < 0) {
      cFirst |= 8;
    }

    int nb_byte_after = get_nb_bytes_int_after(work_num);
    cFirst |= (unsigned char)nb_byte_after;

    cFirst |= (((unsigned char)(work_num & 0xf)) * 0x10);
    work_num /= 0x10;

    *dst_write = cFirst;
    dst_write++;

    while (nb_byte_after > 0) {
      *dst_write = (unsigned char)(work_num & 0xff);
      dst_write++;
      work_num = work_num / 0x100;
      nb_byte_after--;
    }
  }
  return size_ret;
}

static bool unpack_int(const void* src, size_t buf_len, int* p_dest_int,
                       size_t* p_size_used) {
  if ((src == NULL) || (buf_len == 0))
    return false;
  const unsigned char* buf = (const unsigned char*)src;
  unsigned char cFirst     = *buf;
  buf++;
  size_t size_after_first = (((size_t)cFirst) & 0x7);
  size_t size_this        = size_after_first + 1;
  if (buf_len < size_this)
    return false;
  int ret     = (cFirst & 0xf0) / 0x10;
  int iFactor = 0x10;
  while (size_after_first > 0) {
    int iThis = (int)(*buf);
    buf++;
    size_after_first--;
    ret = ret | (iThis * iFactor);
    iFactor *= 0x100;
  }
  if ((cFirst & 8) != 0)
    ret = -ret;
  if (p_dest_int != NULL)
    *p_dest_int = ret;
  if (p_size_used != NULL)
    *p_size_used = size_this;
  return true;
}

static void flush_uint_32(unsigned int num, void* dst) {
  unsigned char* dst_write = (unsigned char*)dst;
  for (int i = 0; i < 4; i++) {
    *(dst_write++) = (unsigned char)(num & 0xff);
    num            = num / 0x100;
  }
}

static unsigned int unpack_uint_32(const void* src) {
  const unsigned char* src_read = (const unsigned char*)src;
  unsigned int uiFactor         = 1;
  unsigned int ret              = 0;
  for (int i = 0; i < 4; i++) {
    ret |= (*(src_read++)) * uiFactor;
    uiFactor = uiFactor * 0x100;
  }
  return ret;
}

size_t pack_store_int::flush_value_array(void* dst, size_t buf_size) {
  int i;
  size_t size_first = flush_int(nb_item_used, NULL, 0);
  size_t size_total = size_first;
  for (i = 0; i < nb_item_used; i++)
    size_total += flush_int(*(item_array + i), NULL, 0);

  if ((dst != NULL) && (buf_size >= size_total)) {
    unsigned char* dst_write = (unsigned char*)dst;
    flush_int(nb_item_used, dst_write, buf_size);
    buf_size -= size_first;
    dst_write += size_first;

    for (i = 0; i < nb_item_used; i++) {
      size_t size_this = flush_int(*(item_array + i), dst_write, buf_size);
      buf_size -= size_this;
      dst_write += size_this;
    }
  }
  return size_total;
}

/*******************************************************************************/
/*******************************************************************************/

class pack_store_string {
 public:
  pack_store_string();
  ~pack_store_string();

  bool AddStringValue(const unichar* str);
  size_t flush_value_array(void* dst, size_t buf_size);

  int get_nb_total_value() {
    return nb_item_used;
  };

 private:
  unichar** item_array;
  int nb_item_used;
  int nb_item_allocated;
  int nb_total_unichar;
};



pack_store_string::pack_store_string()
    : item_array(NULL),
      nb_item_used(0),
      nb_item_allocated(0),
      nb_total_unichar(0) {
}

pack_store_string::~pack_store_string() {
  if (item_array != NULL) {
    for (int i = 0; i < nb_item_used; i++)
      if ((*(item_array + i)) != NULL)
        free(*(item_array + i));
    free(item_array);
  }
}

bool pack_store_string::AddStringValue(const unichar* str) {
  /*
	const unichar empty_string [] = { 0 };
	if (str == NULL)
		str = empty_string ;
		*/
  if (nb_item_allocated <= nb_item_used) {
    unichar** new_item_array;
    int new_nb_item_allocated =
        GET_NEW_ITEM_ALLOCATED_FROM_OLD_NUMBER(nb_item_used);
    if (item_array == NULL)
      new_item_array =
          (unichar**)malloc(sizeof(unichar*) * new_nb_item_allocated);
    else
      new_item_array = (unichar**)realloc(
          item_array, sizeof(unichar*) * new_nb_item_allocated);
    if (new_item_array == NULL)
      return false;
    item_array        = new_item_array;
    nb_item_allocated = new_nb_item_allocated;
  }
  *(item_array + nb_item_used) = NULL;
  if (str != NULL) {
    *(item_array + nb_item_used) = u_strdup(str);
    nb_total_unichar += u_strlen(str);
  }

  nb_item_used++;
  return true;
}

static size_t flush_ustring(const unichar* str, void* dst, size_t size_buf) {
  unichar c;
  unsigned char* dst_write = (unsigned char*)dst;
  size_t size_buf_walk     = size_buf;
  size_t size_needed       = 0;
  //const unichar empty_string [] = { 0 };
  if (str == NULL) {
    //str = empty_string ;
    if ((dst != NULL) && (size_buf_walk >= 1))
      *((unsigned char*)dst) = 0xff;
    return 1;
  }

  do {
    c = *(str++);

    if (c <= 0x7F) {
      if ((dst != NULL) && (size_buf_walk >= 1)) {
        *(dst_write++) = (unsigned char)c;
        size_buf_walk--;
      }
      size_needed += 1;
    } else if (c <= 0x7FF) {
      if ((dst != NULL) && (size_buf_walk >= 2)) {
        *(dst_write++) = (unsigned char)(0xC0 | (c >> 6));
        *(dst_write++) = (unsigned char)(0x80 | (c & 0x3F));
        size_buf_walk -= 2;
      }
      size_needed += 2;
    } else {
      if ((dst != NULL) && (size_buf_walk >= 3)) {
        *(dst_write++) = (unsigned char)(0xE0 | (c >> 12));
        *(dst_write++) = (unsigned char)(0x80 | ((c >> 6) & 0x3F));
        *(dst_write++) = (unsigned char)(0x80 | (c & 0x3F));
        size_buf_walk -= 3;
      }
      size_needed += 3;
    }

  } while (c != 0);
  return size_needed;
}


size_t pack_store_string::flush_value_array(void* dst, size_t buf_size) {
  size_t size_needed = 4 * 2;  // for nb_item and nb_total_unichar
  unsigned char* dst_write;
  size_t buf_size_content;
  if (buf_size < size_needed) {
    buf_size_content = 0;
    dst_write        = NULL;
  } else {
    buf_size_content = buf_size - (4 * 2);
    dst_write        = (unsigned char*)dst;
    flush_uint_32(nb_item_used, dst_write);
    dst_write += 4;
    flush_uint_32(nb_total_unichar, dst_write);
    dst_write += 4;
  }

  for (int i = 0; i < nb_item_used; i++) {
    size_t size_this_string =
        flush_ustring(*(item_array + i), dst_write, buf_size_content);
    if (dst_write != NULL)
      dst_write += size_this_string;
    if (buf_size_content > size_this_string)
      buf_size_content -= size_this_string;
    else
      buf_size_content = 0;
    size_needed += size_this_string;
  }
  return size_needed;
}

/*********************************************************/


typedef struct {
  pack_store_int* p_packInt;
  pack_store_string* p_packStr;
} Context_PackStore;

static bool flush_list_int(const struct list_int* list,
                          pack_store_int* p_packInt) {
  if (list == NULL) {
    return p_packInt->AddIntValue(0);
  } else {
    int length_list = length(list);
    bool fRet       = p_packInt->AddIntValue(length_list);

    const struct list_int* browse_list = list;
    while (fRet && (browse_list != NULL)) {
      fRet        = p_packInt->AddIntValue(browse_list->n);
      browse_list = browse_list->next;
    }
    return fRet;
  }
}

static bool flush_list_ustring(const struct list_ustring* list,
                       Context_PackStore* p_packStore) {
  if (list == NULL) {
    return p_packStore->p_packInt->AddIntValue(0);
  } else {
    int length_list = length(list);
    bool fRet       = p_packStore->p_packInt->AddIntValue(length_list);

    const struct list_ustring* browse_list = list;
    while (fRet && (browse_list != NULL)) {
      const unichar* str           = browse_list->string;
      const unichar empty_string[] = {0};
      if (str == NULL)
        str = empty_string;
      fRet        = p_packStore->p_packStr->AddStringValue(str);
      browse_list = browse_list->next;
    }
    return fRet;
  }
}

static bool flush_pattern(const struct pattern* pattern,
                  Context_PackStore* p_packStore) {
  bool fRet = true;
  if (fRet)
    fRet = p_packStore->p_packInt->AddIntValue((int)(pattern->type));
  if (fRet)
    fRet = p_packStore->p_packStr->AddStringValue(pattern->inflected);
  if (fRet)
    fRet = p_packStore->p_packStr->AddStringValue(pattern->lemma);
  if (fRet)
    fRet = flush_list_ustring(pattern->grammatical_codes, p_packStore);
  if (fRet)
    fRet = flush_list_ustring(pattern->inflectional_codes, p_packStore);
  if (fRet)
    fRet = flush_list_ustring(pattern->forbidden_codes, p_packStore);
  return fRet;
}


static bool flush_transition(const Transition* t, Context_PackStore* p_packStore) {
  bool fRet = true;
  if (fRet)
    fRet = p_packStore->p_packInt->AddIntValue(t->tag_number);
  if (fRet)
    fRet = p_packStore->p_packInt->AddIntValue(t->state_number);
  return fRet;
}

static int length_transition_list(const Transition* list) {
  int n = 0;
  while (list != NULL) {
    list = list->next;
    n++;
  }
  return n;
}

static bool flush_transition_list(const Transition* list,
                         Context_PackStore* p_packStore) {
  if (list == NULL) {
    return p_packStore->p_packInt->AddIntValue(0);
  } else {
    int length_list = length_transition_list(list);
    bool fRet       = p_packStore->p_packInt->AddIntValue(length_list);

    const Transition* browse_list = list;
    while (fRet && (browse_list != NULL)) {
      fRet        = flush_transition(browse_list, p_packStore);
      browse_list = browse_list->next;
    }
    return fRet;
  }
}

static bool flush_Fst2State(Fst2State Fst2StateSrc, Context_PackStore* p_packStore) {
  bool fRet = true;
  if (Fst2StateSrc == NULL) {
    fRet = p_packStore->p_packInt->AddIntValue(0);
    if (fRet)
      fRet = flush_transition_list(NULL, p_packStore);
  } else {
    fRet = p_packStore->p_packInt->AddIntValue(
        (int)(((unsigned int)Fst2StateSrc->control)));
    if (fRet)
      fRet = flush_transition_list(Fst2StateSrc->transitions, p_packStore);
  }
  return fRet;
}



#define FST2TAG_OUTPUT_MEMBER_PRESENT_MASK (0x00001)
#define FST2TAG_TYPE_MEMBER_PRESENT_MASK (0x00004)
#define FST2TAG_VARIABLE_MEMBER_PRESENT_MASK (0x0008)
#define FST2TAG_CONTROL_MEMBER_PRESENT_MASK (0x000002)

#define FST2TAG_MORPHOLOGICAL_FILTER_MEMBER_PRESENT_MASK (0x00010)
#define FST2TAG_META_MEMBER_PRESENT_MASK (0x000800)
#define FST2TAG_PATTERN_MEMBER_PRESENT_MASK (0x00020)
#define FST2TAG_PATTERN_NUMBER_MEMBER_PRESENT_MASK (0x00040)
#define FST2TAG_MATCHING_TOKENS_MEMBER_PRESENT_MASK (0x00100)
#define FST2TAG_COMPOUND_PATTERN_MEMBER_PRESENT_MASK (0x00200)
#define FST2TAG_FILTER_NUMBER_MEMBER_PRESENT_MASK (0x00400)



static bool flush_Fst2Tag(Fst2Tag Fst2TagSrc, Context_PackStore* p_packStore) {
  bool fRet = true;

  if (Fst2TagSrc == NULL) {
    if (fRet)
      fRet = p_packStore->p_packInt->AddIntValue(0);
    if (fRet)
      fRet = p_packStore->p_packStr->AddStringValue(NULL);
  } else {
    int mask_used_member = 0;


    if (Fst2TagSrc->type != UNDEFINED_TAG)
      mask_used_member |= FST2TAG_TYPE_MEMBER_PRESENT_MASK;

    if (Fst2TagSrc->control != 0)
      mask_used_member |= FST2TAG_CONTROL_MEMBER_PRESENT_MASK;

    if (Fst2TagSrc->morphological_filter != NULL)
      mask_used_member |= FST2TAG_MORPHOLOGICAL_FILTER_MEMBER_PRESENT_MASK;

    if (Fst2TagSrc->filter_number != -1)
      mask_used_member |= FST2TAG_FILTER_NUMBER_MEMBER_PRESENT_MASK;

    if (Fst2TagSrc->output != NULL)
      mask_used_member |= FST2TAG_OUTPUT_MEMBER_PRESENT_MASK;

    if (Fst2TagSrc->meta != ((enum meta_symbol) - 1))
      mask_used_member |= FST2TAG_META_MEMBER_PRESENT_MASK;

    if (Fst2TagSrc->pattern != NULL)
      mask_used_member |= FST2TAG_PATTERN_MEMBER_PRESENT_MASK;

    if (Fst2TagSrc->pattern_number != -1)
      mask_used_member |= FST2TAG_PATTERN_NUMBER_MEMBER_PRESENT_MASK;

    if (Fst2TagSrc->variable != NULL)
      mask_used_member |= FST2TAG_VARIABLE_MEMBER_PRESENT_MASK;

    if (Fst2TagSrc->matching_tokens != NULL)
      mask_used_member |= FST2TAG_MATCHING_TOKENS_MEMBER_PRESENT_MASK;

    if (Fst2TagSrc->compound_pattern != -1)
      mask_used_member |= FST2TAG_COMPOUND_PATTERN_MEMBER_PRESENT_MASK;

    if (fRet)
      fRet = p_packStore->p_packInt->AddIntValue(mask_used_member);

    if ((fRet) && ((mask_used_member & FST2TAG_TYPE_MEMBER_PRESENT_MASK) != 0))
      fRet =
          p_packStore->p_packInt->AddIntValue((int)(((int)Fst2TagSrc->type)));

    if ((fRet) &&
        ((mask_used_member & FST2TAG_CONTROL_MEMBER_PRESENT_MASK) != 0))
      fRet = p_packStore->p_packInt->AddIntValue(
          (int)(((unsigned int)Fst2TagSrc->control)));

    if (fRet)
      fRet = p_packStore->p_packStr->AddStringValue(Fst2TagSrc->input);

    if ((fRet) && ((mask_used_member &
                    FST2TAG_MORPHOLOGICAL_FILTER_MEMBER_PRESENT_MASK) != 0))
      fRet = p_packStore->p_packStr->AddStringValue(
          Fst2TagSrc->morphological_filter);

    if ((fRet) &&
        ((mask_used_member & FST2TAG_OUTPUT_MEMBER_PRESENT_MASK) != 0))
      fRet = p_packStore->p_packStr->AddStringValue(Fst2TagSrc->output);

    if ((fRet) &&
        ((mask_used_member & FST2TAG_VARIABLE_MEMBER_PRESENT_MASK) != 0))
      fRet = p_packStore->p_packStr->AddStringValue(Fst2TagSrc->variable);

    if ((fRet) &&
        ((mask_used_member & FST2TAG_MATCHING_TOKENS_MEMBER_PRESENT_MASK) != 0))
      fRet = flush_list_int(Fst2TagSrc->matching_tokens, p_packStore->p_packInt);

    if ((fRet) &&
        ((mask_used_member & FST2TAG_FILTER_NUMBER_MEMBER_PRESENT_MASK) != 0))
      fRet = p_packStore->p_packInt->AddIntValue(Fst2TagSrc->filter_number);

    if ((fRet) && ((mask_used_member & FST2TAG_META_MEMBER_PRESENT_MASK) != 0))
      fRet =
          p_packStore->p_packInt->AddIntValue((int)(((int)Fst2TagSrc->meta)));

    if ((fRet) &&
        ((mask_used_member & FST2TAG_PATTERN_MEMBER_PRESENT_MASK) != 0))
      fRet = flush_pattern(Fst2TagSrc->pattern, p_packStore);

    if ((fRet) &&
        ((mask_used_member & FST2TAG_PATTERN_NUMBER_MEMBER_PRESENT_MASK) != 0))
      fRet = p_packStore->p_packInt->AddIntValue(Fst2TagSrc->pattern_number);

    if ((fRet) && ((mask_used_member &
                    FST2TAG_COMPOUND_PATTERN_MEMBER_PRESENT_MASK) != 0))
      fRet = p_packStore->p_packInt->AddIntValue(Fst2TagSrc->compound_pattern);
  }
  return fRet;
}

static bool flush_Fst2(Fst2* fst2, Context_PackStore* p_packStore) {
  if (fst2 == NULL)
    return false;
  bool fRet = true;
  int i;
  int debugValue = 0;

#ifdef FST2_STRUCTURE_HAS_DEBUG_MEMBER
  debugValue = fst2->debug;
#endif

  if (fRet)
    fRet = p_packStore->p_packInt->AddIntValue(fst2->number_of_graphs);
  if (fRet)
    fRet = p_packStore->p_packInt->AddIntValue(fst2->number_of_states);
  if (fRet)
    fRet = p_packStore->p_packInt->AddIntValue(fst2->number_of_tags);

  for (i = 0; i < fst2->number_of_states; i++) {
    if (fRet)
      fRet = flush_Fst2State(fst2->states[i], p_packStore);
  }

  for (i = 0; i < fst2->number_of_tags; i++) {
    if (fRet)
      fRet = flush_Fst2Tag(fst2->tags[i], p_packStore);
  }

  for (i = 0; i < fst2->number_of_graphs + 1; i++) {
    if (fRet)
      fRet = p_packStore->p_packInt->AddIntValue(fst2->initial_states[i]);
  }

  for (i = 0; i < fst2->number_of_graphs + 1; i++) {
    if (fRet)
      fRet = p_packStore->p_packInt->AddIntValue(
          fst2->number_of_states_per_graphs[i]);
  }

  for (i = 1;                       /* start at 1 because at pos 0
                                                there is no graph */
       i <= fst2->number_of_graphs; /* consequently the last pos is
                                                number_of_graphs+1 */
       i++) {
    const unichar* cur_graph_names = NULL;
    if (fst2->graph_names != NULL)
      cur_graph_names = fst2->graph_names[i];
    fRet = p_packStore->p_packStr->AddStringValue(cur_graph_names);
  }

  if (fRet)
    fRet = flush_list_ustring(fst2->input_variables, p_packStore);

  if (fRet)
    fRet = flush_list_ustring(fst2->output_variables, p_packStore);


  if (fRet && (debugValue != 0)) {
    fRet = p_packStore->p_packInt->AddIntValue(
        1);  // push version of supplemental footer
    fRet = fRet && p_packStore->p_packInt->AddIntValue(
                       1);  // push size in number of int of supplemental footer
    fRet =
        fRet &&
        p_packStore->p_packInt->AddIntValue(
            debugValue);  // push size in number of int of supplemental footer
  }
  return fRet;
}

/*********************************************************/


class read_stored_int_pack {
 public:
  read_stored_int_pack();
  ~read_stored_int_pack();

  bool init_from_buf(const void* buf, size_t buf_size);
  inline bool get_next_int(int* pv) {
    if (cur_item < nb_item_used) {
      *pv = *(item_array + cur_item);
      cur_item++;
      return true;
    } else {
      return false;
    }
  }

 private:
  int* item_array;
  int nb_item_used;

  int cur_item;
};

read_stored_int_pack::read_stored_int_pack()
    : item_array(NULL), nb_item_used(0), cur_item(0) {
}


read_stored_int_pack::~read_stored_int_pack() {
  if (item_array != NULL)
    free(item_array);
}

bool read_stored_int_pack::init_from_buf(const void* buf, size_t buf_size) {
  if (item_array != NULL)
    free(item_array);
  item_array                    = NULL;
  nb_item_used                  = 0;
  cur_item                      = 0;
  const unsigned char* walk_buf = (const unsigned char*)buf;
  size_t size_buf_first         = 0;
  int nb_item_in_buf            = 0;
  if (!unpack_int(walk_buf, buf_size, &nb_item_in_buf, &size_buf_first))
    return false;
  walk_buf += size_buf_first;

  item_array = (int*)malloc((nb_item_in_buf + 1) * sizeof(int));
  if (item_array == NULL)
    return false;

  for (int i = 0; i < nb_item_in_buf; i++) {
    int this_value   = 0;
    size_t size_this = 0;
    if (!unpack_int(walk_buf, buf_size, &this_value, &size_this))
      return false;
    buf_size -= size_this;
    walk_buf += size_this;
    *(item_array + i) = this_value;
  }
  nb_item_used = nb_item_in_buf;
  return true;
}



class read_stored_string_pack {
 public:
  read_stored_string_pack();
  ~read_stored_string_pack();

  bool init_from_buf(const void* buf, size_t buf_size);

  inline bool get_next_string(const unichar** p_str) {
    if (cur_item < nb_item_used) {
      *p_str = *(item_array + (cur_item++));
      return true;
    } else {
      *p_str = NULL;
      return false;
    }
  };

 private:
  unichar** item_array;
  int nb_item_used;
  int nb_item_allocated;

  int cur_item;
};


read_stored_string_pack::read_stored_string_pack()
    : item_array(NULL), nb_item_used(0), nb_item_allocated(0), cur_item(0) {
}


read_stored_string_pack::~read_stored_string_pack() {
  if (item_array != NULL)
    free(item_array);
}



#define GetUtf8Size(ch)  \
        (((((unsigned char)(ch)) & ((unsigned char)0x80))==((unsigned char)0x00)) ? 1 : \
        (((((unsigned char)(ch)) & ((unsigned char)0xe0))==((unsigned char)0xc0)) ? 2 : \
        (((((unsigned char)(ch)) & ((unsigned char)0xf0))==((unsigned char)0xe0)) ? 3 : \
        (((((unsigned char)(ch)) & ((unsigned char)0xf8))==((unsigned char)0xf0)) ? 4 : \
        (((((unsigned char)(ch)) & ((unsigned char)0xfc))==((unsigned char)0xf8)) ? 5 : \
        (((((unsigned char)(ch)) & ((unsigned char)0xfe))==((unsigned char)0xfc)) ? 6 : 001))))))


#define GetUtf8Mask(ch)  \
        (((((unsigned char)(ch)) & ((unsigned char)0x80))==((unsigned char)0x00)) ? ((unsigned char)0x7f) : \
        (((((unsigned char)(ch)) & ((unsigned char)0xe0))==((unsigned char)0xc0)) ? ((unsigned char)0x1f) : \
        (((((unsigned char)(ch)) & ((unsigned char)0xf0))==((unsigned char)0xe0)) ? ((unsigned char)0x0f) : \
        (((((unsigned char)(ch)) & ((unsigned char)0xf8))==((unsigned char)0xf0)) ? ((unsigned char)0x07) : \
        (((((unsigned char)(ch)) & ((unsigned char)0xfc))==((unsigned char)0xf8)) ? ((unsigned char)0x03) : \
        (((((unsigned char)(ch)) & ((unsigned char)0xfe))==((unsigned char)0xfc)) ? ((unsigned char)0x01) : 0))))))


static size_t unpack_string(unichar* write_content_walk_buf,
                            size_t nb_unichar_alloc_walk,
                            size_t* p_size_this_string_written,
                            const unsigned char* src_walk, size_t buf_size) {
  size_t size_this_string_written = 0;
  size_t nb_pack_read             = 0;
  for (;;) {
    if ((src_walk == NULL) || (buf_size == 0))
      return 0;
    unsigned char ch = *(src_walk++);
    buf_size--;
    nb_pack_read++;

    unichar c;

    if ((ch & 0x80) == 0) {
      c = ch;
    } else {
      c          = ch & GetUtf8Mask(ch);
      int nbbyte = GetUtf8Size(ch);
      if (((int)buf_size) + 1 < nbbyte)
        return 0;

      for (;;) {
        nbbyte--;
        if (nbbyte == 0)
          break;

        c = (c << 6) | ((*(src_walk++)) & 0x3F);
        buf_size--;
        nb_pack_read++;
      }
    }

    if ((write_content_walk_buf != NULL) &&
        (size_this_string_written < nb_unichar_alloc_walk))
      *(write_content_walk_buf + size_this_string_written) = c;
    size_this_string_written++;

    if (c == 0) {
      if (p_size_this_string_written != NULL)
        *p_size_this_string_written = size_this_string_written;
      return nb_pack_read;
    }
  }
}


bool read_stored_string_pack::init_from_buf(const void* buf, size_t buf_size) {
  if ((buf == NULL) || (buf_size < (4 * 2)))
    return false;

  const unsigned char* src_walk = (const unsigned char*)buf;

  unsigned int nb_item_buf = unpack_uint_32(src_walk);
  src_walk += 4;
  buf_size -= 4;

  unsigned int nb_total_unichar_buf = unpack_uint_32(src_walk);
  src_walk += 4;
  buf_size -= 4;

  size_t size_array_ptr         = (sizeof(unichar*) * (nb_item_buf + 1));
  unsigned int nb_unichar_alloc = nb_total_unichar_buf + nb_item_buf + 1;
  size_t size_content           = (sizeof(unichar) * (nb_unichar_alloc));


  void* alloc_buf = malloc(size_array_ptr + size_content);
  if (alloc_buf == NULL)
    return false;

  unichar** item_array_read          = (unichar**)alloc_buf;
  unsigned int nb_unichar_alloc_walk = nb_unichar_alloc;

  unichar* write_content_walk_buf =
      (unichar*)(((char*)alloc_buf) + size_array_ptr);

  for (unsigned int i = 0; i < nb_item_buf; i++) {
    size_t size_this_string_written = 0;
    *(item_array_read + i)          = write_content_walk_buf;
    size_t size_pack_this_string    = 0;
    if (buf_size > 0)
      if ((*src_walk) == 0xff) {
        size_pack_this_string  = 1;  // the NULL situation
        *(item_array_read + i) = NULL;
      }
    if (size_pack_this_string == 0)
      size_pack_this_string =
          unpack_string(write_content_walk_buf, nb_unichar_alloc_walk,
                        &size_this_string_written, src_walk, buf_size);
    if ((size_pack_this_string == 0) || (size_pack_this_string > buf_size)) {
      free(alloc_buf);
      return false;
    }
    src_walk += size_pack_this_string;
    buf_size -= size_pack_this_string;
    nb_unichar_alloc_walk -= (unsigned int)size_this_string_written;
    write_content_walk_buf += size_this_string_written;
  }

  item_array = item_array_read;

  nb_item_allocated = nb_item_used = (int)nb_item_buf;
  item_array                       = item_array_read;

  cur_item = 0;

  return true;
}


typedef struct {
  read_stored_int_pack* p_packrInt;
  read_stored_string_pack* p_packrStr;
} Context_PackRead;

static bool read_list_int(struct list_int** resL, Context_PackRead* p_PackRead,
                          Abstract_allocator prv_alloc) {
  int nb_item_list               = 0;
  struct list_int* retList       = NULL;
  struct list_int** next_filling = &retList;

  bool fRes = p_PackRead->p_packrInt->get_next_int(&nb_item_list);

  if (fRes) {
    while ((fRes) && (nb_item_list > 0)) {
      int value = 0;
      fRes      = p_PackRead->p_packrInt->get_next_int(&value);
      if (!fRes)
        break;
      *next_filling = new_list_int(value, NULL, prv_alloc);
      next_filling  = &((*next_filling)->next);
      nb_item_list--;
    }
  }
  if (fRes)
    *resL = retList;
  else
    free_list_int(retList, prv_alloc);
  return fRes;
}

static bool read_list_ustring(struct list_ustring** resL,
                              Context_PackRead* p_PackRead,
                              Abstract_allocator prv_alloc) {
  int nb_item_list                   = 0;
  struct list_ustring* retList       = NULL;
  struct list_ustring** next_filling = &retList;

  bool fRes = p_PackRead->p_packrInt->get_next_int(&nb_item_list);

  if (fRes) {
    while ((nb_item_list > 0)) {
      const unichar* str;
      fRes = p_PackRead->p_packrStr->get_next_string(&str);
      if (!fRes) {
        break;
      }
      *next_filling = new_list_ustring(str, NULL, prv_alloc);
      next_filling  = &((*next_filling)->next);
      nb_item_list--;
    }
  }
  if (fRes)
    *resL = retList;
  else
    free_list_ustring(retList, prv_alloc);
  return fRes;
}

static inline bool read_and_fill_string(unichar** dest,
                                        Context_PackRead* p_PackRead,
                                        Abstract_allocator prv_alloc) {
  const unichar* read_str = NULL;
  bool fRet               = p_PackRead->p_packrStr->get_next_string(&read_str);
  if (fRet) {
    *dest = u_strdup(read_str, prv_alloc);
  }
  return (fRet);
}

static bool read_pattern(struct pattern** p_pattern,
                        Context_PackRead* p_PackRead,
                        Abstract_allocator prv_alloc) {
  struct pattern* ret_pattern;
  ret_pattern = (struct pattern*)malloc_cb(sizeof(struct pattern), prv_alloc);
  if (ret_pattern == NULL) {
    fatal_error("Not enough memory in ReadPattern\n");
  }
  int pattern_type_int = 0;
  bool fRes            = true;
  if (fRes)
    fRes = p_PackRead->p_packrInt->get_next_int(&pattern_type_int);
  ret_pattern->type = (pattern_type)pattern_type_int;

  if (fRes)
    fRes = read_and_fill_string(&ret_pattern->inflected, p_PackRead, prv_alloc);
  else
    ret_pattern->inflected = NULL;

  if (fRes)
    fRes = read_and_fill_string(&ret_pattern->lemma, p_PackRead, prv_alloc);
  else
    ret_pattern->lemma = NULL;

  if (fRes)
    fRes = read_list_ustring(&ret_pattern->grammatical_codes, p_PackRead,
                            prv_alloc);
  else
    ret_pattern->grammatical_codes = NULL;

  if (fRes)
    fRes = read_list_ustring(&ret_pattern->inflectional_codes, p_PackRead,
                            prv_alloc);
  else
    ret_pattern->inflectional_codes = NULL;

  if (fRes)
    fRes =
        read_list_ustring(&ret_pattern->forbidden_codes, p_PackRead, prv_alloc);
  else
    ret_pattern->forbidden_codes = NULL;

  if (fRes)
    *p_pattern = ret_pattern;
  else
    free_pattern(ret_pattern, prv_alloc);
  return fRes;
}


static bool read_transition(Transition** p_transition,
                            Context_PackRead* p_PackRead,
                            Abstract_allocator prv_alloc) {
  Transition* ret_transition = NULL;

  bool fRes      = true;
  int tag_number = 0;
  if (fRes)
    fRes = p_PackRead->p_packrInt->get_next_int(&tag_number);

  int state_number = 0;
  if (fRes)
    fRes = p_PackRead->p_packrInt->get_next_int(&state_number);
  if (!fRes)
    return false;
  ret_transition = (Transition*)malloc_cb(sizeof(Transition), prv_alloc);
  if (ret_transition == NULL) {
    fatal_error("Not enough memory in ReadTransition\n");
  }
  ret_transition->state_number = state_number;
  ret_transition->tag_number   = tag_number;
  ret_transition->next         = NULL;

  *p_transition = ret_transition;
  return fRes;
}


static bool read_list_Transition(Transition** p_transition_list,
                                 Context_PackRead* p_PackRead,
                                 Abstract_allocator prv_alloc) {
  int nb_item_list          = 0;
  Transition* retList       = NULL;
  Transition** next_filling = &retList;

  bool fRes = p_PackRead->p_packrInt->get_next_int(&nb_item_list);

  if (fRes) {
    while ((nb_item_list > 0)) {
      fRes = read_transition(next_filling, p_PackRead, prv_alloc);
      if (!fRes)
        break;
      next_filling = &((*next_filling)->next);
      nb_item_list--;
    }
  }
  if (fRes)
    *p_transition_list = retList;
  else
    free_Transition_list(retList, prv_alloc);
  return fRes;
}

static bool read_Fst2State(Fst2State* p_Fst2State, Context_PackRead* p_PackRead,
                           Abstract_allocator prv_alloc) {
  Fst2State Fst2dest = (fst2State*)malloc_cb(sizeof(fst2State), prv_alloc);
  if (Fst2dest == NULL) {
    fatal_error("Not enough memory in new_Fst2State_clone\n");
  }
  int read_control  = 0;
  bool fRes         = p_PackRead->p_packrInt->get_next_int(&read_control);
  Fst2dest->control = (unsigned char)((unsigned int)(read_control));
  if (fRes)
    fRes = read_list_Transition(&Fst2dest->transitions, p_PackRead, prv_alloc);

  if (fRes)
    *p_Fst2State = Fst2dest;
  else {
    free_Transition_list(Fst2dest->transitions, prv_alloc);
    free_cb(Fst2dest, prv_alloc);
  }
  return fRes;
}

static bool read_Fst2Tag(Fst2Tag* p_Fst2Tag, Context_PackRead* p_PackRead,
                 Abstract_allocator prv_alloc) {
  Fst2Tag Fst2dest = (fst2Tag*)malloc_cb(sizeof(fst2Tag), prv_alloc);
  if (Fst2dest == NULL) {
    fatal_error("Not enough memory in new_Fst2Tag_clone\n");
  }

  bool fRes = true;


  int mask_used_member = 0;
  if (fRes)
    fRes = p_PackRead->p_packrInt->get_next_int(&mask_used_member);


  Fst2dest->type = UNDEFINED_TAG;
  if ((mask_used_member & FST2TAG_TYPE_MEMBER_PRESENT_MASK) != 0) {
    int read_type  = 0;
    fRes           = p_PackRead->p_packrInt->get_next_int(&read_type);
    Fst2dest->type = (tag_type)(read_type);
  }

  int read_control = 0;
  if ((fRes) && ((mask_used_member & FST2TAG_CONTROL_MEMBER_PRESENT_MASK) != 0))
    fRes = p_PackRead->p_packrInt->get_next_int(&read_control);
  Fst2dest->control = (unsigned char)((unsigned int)(read_control));

  if (fRes)
    fRes = read_and_fill_string(&Fst2dest->input, p_PackRead, prv_alloc);
  else
    Fst2dest->input = NULL;

  if ((fRes) && ((mask_used_member &
                  FST2TAG_MORPHOLOGICAL_FILTER_MEMBER_PRESENT_MASK) != 0))
    fRes = read_and_fill_string(&Fst2dest->morphological_filter, p_PackRead,
                             prv_alloc);
  else
    Fst2dest->morphological_filter = NULL;

  if ((fRes) && ((mask_used_member & FST2TAG_OUTPUT_MEMBER_PRESENT_MASK) != 0))
    fRes = read_and_fill_string(&Fst2dest->output, p_PackRead, prv_alloc);
  else
    Fst2dest->output = NULL;

  if ((fRes) &&
      ((mask_used_member & FST2TAG_VARIABLE_MEMBER_PRESENT_MASK) != 0))
    fRes = read_and_fill_string(&Fst2dest->variable, p_PackRead, prv_alloc);
  else
    Fst2dest->variable = NULL;

  if ((fRes) &&
      ((mask_used_member & FST2TAG_MATCHING_TOKENS_MEMBER_PRESENT_MASK) != 0))
    fRes = read_list_int(&Fst2dest->matching_tokens, p_PackRead, prv_alloc);
  else
    Fst2dest->matching_tokens = NULL;

  if ((fRes) &&
      ((mask_used_member & FST2TAG_FILTER_NUMBER_MEMBER_PRESENT_MASK) != 0)) {
    int read_filter_number = 0;
    if (fRes)
      fRes = p_PackRead->p_packrInt->get_next_int(&read_filter_number);
    Fst2dest->filter_number = read_filter_number;
  } else
    Fst2dest->filter_number = -1;

  if ((fRes) && ((mask_used_member & FST2TAG_META_MEMBER_PRESENT_MASK) != 0)) {
    int read_meta = 0;
    if (fRes)
      fRes = p_PackRead->p_packrInt->get_next_int(&read_meta);
    Fst2dest->meta = (enum meta_symbol)((int)(read_meta));
  } else
    Fst2dest->meta = (enum meta_symbol)((int)(-1));

  Fst2dest->pattern = NULL;
  if ((fRes) && ((mask_used_member & FST2TAG_PATTERN_MEMBER_PRESENT_MASK) != 0))
    fRes = read_pattern(&Fst2dest->pattern, p_PackRead, prv_alloc);

  if ((fRes) &&
      ((mask_used_member & FST2TAG_PATTERN_NUMBER_MEMBER_PRESENT_MASK) != 0)) {
    int read_pattern_number = 0;
    if (fRes)
      fRes = p_PackRead->p_packrInt->get_next_int(&read_pattern_number);
    Fst2dest->pattern_number = read_pattern_number;
  } else
    Fst2dest->pattern_number = -1;

  if ((fRes) && ((mask_used_member &
                  FST2TAG_COMPOUND_PATTERN_MEMBER_PRESENT_MASK) != 0)) {
    int read_compound_pattern = 0;
    if (fRes)
      fRes = p_PackRead->p_packrInt->get_next_int(&read_compound_pattern);
    Fst2dest->compound_pattern = read_compound_pattern;
  } else
    Fst2dest->compound_pattern = -1;


  if (fRes)
    *p_Fst2Tag = Fst2dest;
  else {
    free_cb(Fst2dest->input, prv_alloc);
    free_cb(Fst2dest->morphological_filter, prv_alloc);
    free_cb(Fst2dest->output, prv_alloc);
    free_cb(Fst2dest->variable, prv_alloc);
    free_list_int(Fst2dest->matching_tokens, prv_alloc);
    free_pattern(Fst2dest->pattern, prv_alloc);
    free_cb(Fst2dest, prv_alloc);
  }
  return fRes;
}


Fst2* read_fst2_from_pack(Context_PackRead* p_PackRead,
                          Abstract_allocator prv_alloc) {
  int debugValue = 0;
  Fst2* fst2ret;
  fst2ret = (Fst2*)malloc_cb(sizeof(Fst2), prv_alloc);
  if (fst2ret == NULL) {
    fatal_error("Not enough memory in read_fst2_from_pack\n");
    return NULL;
  }


  memset(fst2ret, 0, sizeof(Fst2));
  int i;
  bool fRes = true;


  fst2ret->states = NULL;
  fst2ret->tags   = NULL;

  int read_number_of_graphs = 0;
  if (fRes)
    fRes = p_PackRead->p_packrInt->get_next_int(&read_number_of_graphs);
  fst2ret->number_of_graphs = read_number_of_graphs;

  int read_number_of_states = 0;
  if (fRes)
    fRes = p_PackRead->p_packrInt->get_next_int(&read_number_of_states);
  fst2ret->number_of_states = read_number_of_states;

  int read_number_of_tags = 0;
  if (fRes)
    fRes = p_PackRead->p_packrInt->get_next_int(&read_number_of_tags);
  fst2ret->number_of_tags = read_number_of_tags;
  

  fst2ret->initial_states              = NULL;
  fst2ret->graph_names                 = NULL;
  fst2ret->number_of_states_per_graphs = NULL;
  fst2ret->input_variables             = NULL;
  fst2ret->output_variables            = NULL;

  fst2ret->states = (Fst2State*)malloc_cb(
      sizeof(Fst2State) * fst2ret->number_of_states, prv_alloc);
  for (i = 0; i < fst2ret->number_of_states; i++) {
    if (fRes)
      fRes = read_Fst2State(&fst2ret->states[i], p_PackRead, prv_alloc);
    else
      fst2ret->states[i] = NULL;
  }

  fst2ret->tags =
      (Fst2Tag*)malloc_cb(sizeof(Fst2Tag) * fst2ret->number_of_tags, prv_alloc);

  for (i = 0; i < fst2ret->number_of_tags; i++) {
    if (fRes)
      fRes = read_Fst2Tag(&fst2ret->tags[i], p_PackRead, prv_alloc);
    else
      fst2ret->tags[i] = NULL;
  }


  fst2ret->initial_states =
      (int*)malloc_cb((fst2ret->number_of_graphs + 1) * sizeof(int), prv_alloc);
  if (fst2ret->initial_states == NULL) {
    fatal_error("Not enough memory in load_fst2\n");
  }

  for (i = 0; i < fst2ret->number_of_graphs + 1; i++) {
    int cur_init_state_read = 0;
    if (fRes)
      fRes = p_PackRead->p_packrInt->get_next_int(&cur_init_state_read);
    fst2ret->initial_states[i] = cur_init_state_read;
  }


  fst2ret->number_of_states_per_graphs =
      (int*)malloc_cb((fst2ret->number_of_graphs + 1) * sizeof(int), prv_alloc);
  if (fst2ret->number_of_states_per_graphs == NULL) {
    fatal_error("Not enough memory in load_fst2\n");
  }

  for (i = 0; i < fst2ret->number_of_graphs + 1; i++) {
    int cur_init_states_per_graphs_read = 0;
    if (fRes)
      fRes = p_PackRead->p_packrInt->get_next_int(
          &cur_init_states_per_graphs_read);
    fst2ret->number_of_states_per_graphs[i] = cur_init_states_per_graphs_read;
  }

  {
    fst2ret->graph_names = (unichar**)malloc_cb(
        sizeof(unichar*) * (fst2ret->number_of_graphs + 1), prv_alloc);
    if (fst2ret->graph_names == NULL) {
      fatal_error("Not enough memory in load_fst2\n");
    }

    if (fst2ret->graph_names != NULL) {
      fst2ret->graph_names[0] = NULL;
      for (i = 1;                          /* start at 1 because at pos 0
                                                    there is no graph */
           i <= fst2ret->number_of_graphs; /* consequently the last pos is
                                                    number_of_graphs+1 */
           i++) {
        fst2ret->graph_names[i] = NULL;
        if (fRes)
          fRes = read_and_fill_string(&fst2ret->graph_names[i], p_PackRead,
                                   prv_alloc);
      }
    }
  }

  if (fRes)
    fRes = read_list_ustring(&fst2ret->input_variables, p_PackRead, prv_alloc);
  if (fRes)
    fRes = read_list_ustring(&fst2ret->output_variables, p_PackRead, prv_alloc);
  

  int suppFootVersion = 0;
  if (p_PackRead->p_packrInt->get_next_int(&suppFootVersion)) {
    if (suppFootVersion >= 1) {
      int sizeInIntSuppFoot = 0;
      if (p_PackRead->p_packrInt->get_next_int(&sizeInIntSuppFoot))
        if (sizeInIntSuppFoot >= 1) {
          p_PackRead->p_packrInt->get_next_int(&debugValue);
        }
    }
  }
#ifdef FST2_STRUCTURE_HAS_DEBUG_MEMBER
  fst2ret->debug = debugValue;
#endif

  if (!fRes) {
    free_Fst2(fst2ret, prv_alloc);
    fst2ret = NULL;
  }
  return fst2ret;
}

bool write_pack_fst2(Fst2* fst2load, const char* fst2_pack_name,
                       bool fVerbose) {
  pack_store_string packStr;
  pack_store_int packInt;
  Context_PackStore cps;
  cps.p_packInt = &packInt;
  cps.p_packStr = &packStr;

  bool fRet = flush_Fst2(fst2load, &cps);

  if (!fRet)
    return false;
  size_t size_needed_int = packInt.flush_value_array(NULL, 0);
  if (fVerbose)
    u_printf("\nsize needed for int item : %u for %u total item\n",
             (unsigned int)size_needed_int, packInt.get_nb_total_value());
  void* buf_int = malloc(size_needed_int);
  packInt.flush_value_array(buf_int, size_needed_int);

  size_t size_needed_str = packStr.flush_value_array(NULL, 0);
  if (fVerbose)
    u_printf("size needed for str item : %u for %u total item\n\n",
             (unsigned int)size_needed_str, packStr.get_nb_total_value());
  void* buf_str = malloc(size_needed_str);
  packStr.flush_value_array(buf_str, size_needed_str);

  unsigned char packFst2Head[32];
  memset(packFst2Head, 0, sizeof(packFst2Head));
  packFst2Head[0] = 'F';
  packFst2Head[1] = 'S';
  packFst2Head[2] = 'T';
  packFst2Head[3] = '2';
  packFst2Head[4] = 'p';
  packFst2Head[5] = 'k';
  packFst2Head[6] = 0;
  packFst2Head[7] = 0x1;
  flush_uint_32((unsigned int)size_needed_int, &packFst2Head[8]);
  flush_uint_32((unsigned int)size_needed_str, &packFst2Head[12]);


  ABSTRACTFILE* f = NULL;
  if (fst2_pack_name != NULL) {
    f = af_fopen(fst2_pack_name, "wb");
    if (f == NULL)
      fRet = false;
  }
  if (f != NULL) {
    {
      if (((long)af_fwrite(packFst2Head, 1, sizeof(packFst2Head), f)) !=
          (long)(sizeof(packFst2Head)))
        fRet = false;
      if (((long)af_fwrite(buf_int, 1, size_needed_int, f)) !=
          (long)(size_needed_int))
        fRet = false;
      if (((long)af_fwrite(buf_str, 1, size_needed_str, f)) !=
          (long)(size_needed_str))
        fRet = false;
      af_fclose(f);
    }
  }
  free(buf_int);
  free(buf_str);
  return fRet;
}

bool convert_fst2_to_fst2_pack_file(const char* fst2_name, const char* fst2_pack_name,
                    bool fVerbose) {
#ifdef VersatileEncodingConfigDefined
  const VersatileEncodingConfig vecDefault = {
      DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT, DEFAULT_ENCODING_OUTPUT,
      DEFAULT_BOM_OUTPUT};
#endif

  Fst2* fst2load;
  struct FST2_free_info fst2load_free;
  fst2load = load_abstract_fst2(
#ifdef VersatileEncodingConfigDefined
      &vecDefault,
#endif
      fst2_name, 1, &fst2load_free);
  if (fst2load == NULL)
    return false;

  bool fRet = true;

  if (fVerbose)
    u_printf("\n\nload %s and save %s\n", fst2_name, fst2_pack_name);
  write_pack_fst2(fst2load, fst2_pack_name, fVerbose);

  free_abstract_Fst2(fst2load, &fst2load_free);
  return fRet;
}


Fst2* read_pack_fst2_from_memory(const void* buf, size_t size_buf,
                             Abstract_allocator prv_alloc) {
  unsigned char packFst2Head[32];
  if (size_buf < 32)
    return NULL;
  memcpy(packFst2Head, buf, 32);
  unsigned int size_int = 0;
  unsigned int size_str = 0;

  if ((packFst2Head[0] == 'F') && (packFst2Head[1] == 'S') &&
      (packFst2Head[2] == 'T') && (packFst2Head[3] == '2') &&
      (packFst2Head[4] == 'p') && (packFst2Head[5] == 'k') &&
      (packFst2Head[6] == 0)) {
    size_int = unpack_uint_32(&packFst2Head[8]);
    size_str = unpack_uint_32(&packFst2Head[12]);
  }
  if ((size_int == 0) || ((32 + size_int + size_str > size_buf)))
    return NULL;

  read_stored_int_pack rsip;
  read_stored_string_pack rssp;
  bool f_init1 = rsip.init_from_buf(((const char*)buf) + 32, size_int);
  bool f_init2 =
      rssp.init_from_buf(((const char*)buf) + 32 + size_int, size_str);
  if ((!f_init1) || (!f_init2))
    return NULL;

  Context_PackRead PackRead;
  PackRead.p_packrInt = &rsip;
  PackRead.p_packrStr = &rssp;
  Fst2* ret           = read_fst2_from_pack(&PackRead, prv_alloc);

  return ret;
}

Fst2* read_pack_fst2_from_file(const char* filename, Abstract_allocator prv_alloc) {
  ABSTRACTMAPFILE* amf = af_open_mapfile(filename, MAPFILE_OPTION_READ, 0);
  if (amf == NULL) {
    return NULL;
  }
  const void* buf = af_get_mapfile_pointer(amf);
  size_t size_buf = af_get_mapfile_size(amf);
  Fst2* ret       = read_pack_fst2_from_memory(buf, size_buf, prv_alloc);
  af_release_mapfile_pointer(amf, buf);
  af_close_mapfile(amf);
  return ret;
}

void free_pack_fst2(Fst2* fst2, Abstract_allocator prv_alloc)
{
  DISCARD_UNUSED_PARAMETER(prv_alloc);

  free_Fst2(fst2,NULL);
}


#ifdef _DEBUG

bool do_test_pack_int_array(const int* iArray, int nb_item) {
  pack_store_int packInt;
  int nb;
  for (nb = 0; nb < nb_item; nb++)
    packInt.AddIntValue(*(iArray + nb));

  size_t size_needed = packInt.flush_value_array(NULL, 0);
  u_printf("\nsize needed for %u int item : %u\n", (unsigned int)nb_item,
           (unsigned int)size_needed);
  void* buf = malloc(size_needed);
  packInt.flush_value_array(buf, size_needed);
  u_printf("pack done\n");
  read_stored_int_pack rsip;

  bool f_init = rsip.init_from_buf(buf, size_needed);
  if (f_init)
    u_printf("init from buf OK\n");
  else
    u_printf("init from buf ERROR !!!!!\n");
  bool fOk = f_init;

  for (nb = 0; nb < nb_item; nb++) {
    int iValueWaited = (*(iArray + nb));
    int iValueGet    = 0;
    bool fget        = rsip.get_next_int(&iValueGet);
    if ((!fget) || (iValueWaited != iValueGet)) {
      u_printf("error on get int\n");
      fOk = false;
    }
  }

  if (fOk)
    u_printf("do_test_pack_int_array OK\n");
  else
    u_printf("do_test_pack_int_array ERROR !!!!!\n");
  return fOk;
}


bool do_test_pack_string_array(const unichar** ustrustr, int nb_item) {
  pack_store_string packStr;
  int nb;
  for (nb = 0; nb < nb_item; nb++)
    packStr.AddStringValue(*(ustrustr + nb));

  size_t size_needed = packStr.flush_value_array(NULL, 0);
  u_printf("\nsize needed for %u str item : %u\n", (unsigned int)nb_item,
           (unsigned int)size_needed);
  void* buf = malloc(size_needed);
  packStr.flush_value_array(buf, size_needed);
  u_printf("pack done\n");
  read_stored_string_pack rssp;

  bool f_init = rssp.init_from_buf(buf, size_needed);
  if (f_init)
    u_printf("init from buf OK\n");
  else
    u_printf("init from buf ERROR !!!!!\n");
  bool fOk = f_init;

  for (nb = 0; nb < nb_item; nb++) {
    const unichar* strValueWaited = (*(ustrustr + nb));
    const unichar* strget         = NULL;
    bool fget                     = rssp.get_next_string(&strget);

    if (fget) {
      if ((strValueWaited == NULL) && (strget != NULL))
        fget = u_strlen(strget) == 0;
      else
        fget = (u_strcmp(strValueWaited, strget) == 0);
    }
    if ((!fget)) {
      u_printf("error on get str\n");
      fOk = false;
    }
  }

  if (fOk)
    u_printf("do_test_pack_string_array OK\n");
  else
    u_printf("do_test_pack_string_array ERROR !!!!!\n");
  return fOk;
}


int do_test_pack_fst2(int argc, char* const argv[]) {
#ifdef VersatileEncodingConfigDefined
  const VersatileEncodingConfig vecDefault = {
      DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT, DEFAULT_ENCODING_OUTPUT,
      DEFAULT_BOM_OUTPUT};
#endif

  const int TestArray4[] = {0, 12, -13, -15};
  //const int TestArray4[]= {  1,2,3,4 } ;
  const int TestArray8[]  = {1, 4010, 3, -6533, -17, -3, 1026, 388};
  const int TestArray8a[] = {1, 25, 34, 12, -17, -3, 17, 388};
  const int TestArray8b[] = {1,        25785,  4434, 4567899,
                             -4657817, -45658, 17,   384788};
  do_test_pack_int_array(TestArray4, 4);
  do_test_pack_int_array(TestArray8, 3);
  do_test_pack_int_array(TestArray8, 8);
  do_test_pack_int_array(TestArray8a, 8);
  do_test_pack_int_array(TestArray8b, 8);

  const unichar str1[]  = {'s', 't', 'r', '1', 0};
  const unichar str2[]  = {'s', 't', 'r', 'i', 'n', 'g', '2', 0};
  const unichar str3[]  = {0};
  const unichar str4a[] = {'s',  't',   'r',    'i', 'n', 'g',
                           0xe0, 0x166, 0x20ac, '3', 0};
  const unichar str4b[] = {'s', 't', 'r', 'i', 'n', 'g', 'b', 'o', 'f', '3', 0};
  const unichar* strstrA[] = {str1, str2, str3, str4a};
  const unichar* strstrB[] = {str1, str2, str3, str4b};
  const unichar* strstrC[] = {str1, str2, str3, NULL, NULL, str3, str4b};
  /*
	const unichar strstr[][]= {
		
		{ 's','t','r','i','n','g','2',0},
	};*/
  u_printf("\n\n");
  u_printf("unic '%S'\n", *(strstrA + 0));
  u_printf("unic '%S'\n", *(strstrA + 1));
  u_printf("unic '%S'\n", *(strstrA + 2));
  u_printf("unic '%S'\n", *(strstrA + 3));

  do_test_pack_string_array(strstrA, 4);
  do_test_pack_string_array(strstrB, 4);
  do_test_pack_string_array(strstrC, 7);

  if (argc >= 3) {
    const char* fst2_name      = argv[1];
    const char* fst2_pack_name = argv[2];
    Fst2* classic_fst2         = load_fst2(
#ifdef VersatileEncodingConfigDefined
        &vecDefault,
#endif
        fst2_name, 1);
    convert_fst2_to_fst2_pack_file(fst2_name, fst2_pack_name, true);

    Fst2* fst2ret = read_pack_fst2_from_file(fst2_pack_name, NULL);
    if (argc >= 4) {
      write_pack_fst2(classic_fst2, argv[3], true);
      write_pack_fst2(fst2ret, argv[3], true);
    }

    free_Fst2(fst2ret);
    free_Fst2(classic_fst2);
  }
  return 1;
}
#endif

}
