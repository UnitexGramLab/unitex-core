/*
 * Unitex
 *
 * Copyright (C) 2001-2016 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "Offsets.h"
#include "Unicode.h"
#include "Error.h"
#include "Overlap.h"
#include "File.h"
#include "Ustring.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * Loads the given offset file. Returns NULL in case of error.
 */
vector_offset* load_offsets(const VersatileEncodingConfig* vec,const char* name) {
U_FILE* f=u_fopen(vec,name,U_READ);
if (f==NULL) return NULL;
int a,b,c,d,n;
vector_offset* res=new_vector_offset();
while ((n=u_fscanf(f,"%d%d%d%d",&a,&b,&c,&d))!=EOF) {
    if (n!=4) {
        fatal_error("Corrupted offset file %s\n",name);
    }
    vector_offset_add(res,a,b,c,d);
}
u_fclose(f);
return res;
}


void save_offsets(U_FILE* f, int a, int b, int c, int d) {
u_fprintf(f, "%d %d %d %d\n", a, b, c, d);
//error("save: %d %d %d %d\n", a, b, c, d);
}


void save_offsets(U_FILE* f, const vector_offset* offsets) {
if (offsets == NULL) {
    return;
}
int nb_offsets_items = offsets->nbelems;

for (int i = 0; i < nb_offsets_items; i++) {
    Offsets x = offsets->tab[i];
    save_offsets(f, x.old_start, x.old_end, x.new_start, x.new_end);
}
}



/*static void save_offsets(U_FILE* f,vector_offset* v) {
if (v==NULL) return;
for (int i=0;i<v->nbelems;i++) {
    Offsets o=v->tab[i];
    save_offsets(f,o.old_start,o.old_end,o.new_start,o.new_end);
}
}*/


/*static const char* N(Overlap o) {
switch (o) {
case A_BEFORE_B: return "A_BEFORE_B";
case A_BEFORE_B_OVERLAP: return "A_BEFORE_B_OVERLAP";
case A_INCLUDES_B: return "A_INCLUDES_B";
case A_EQUALS_B: return "A_EQUALS_B";
case B_INCLUDES_A: return "B_INCLUDES_A";
case A_AFTER_B_OVERLAP: return "A_AFTER_B_OVERLAP";
case A_AFTER_B: return "A_AFTER_B";
case -1: return "[-1]";
default: return "OOOOOOOOPS";
}
}*/


int save_offsets(const VersatileEncodingConfig* vec, const char* filename, const vector_offset* offsets) {
    U_FILE* f_output_offsets = u_fopen(vec, filename, U_WRITE);
    if (f_output_offsets == NULL) {
        error("Cannot create offset file %s\n", filename);
        return 1;
    }
    save_offsets(f_output_offsets, offsets);
    u_fclose(f_output_offsets);
    return 0;
}


static inline int vector_offset_add(vector_offset* vec, Offsets o) {
    while (vec->nbelems >= vec->size) {
        vector_offset_resize(vec, vec->size * 2);
    }
    vec->tab[vec->nbelems] = o;
    vec->nbelems++;
    return vec->nbelems - 1;
}


/**
 * add offset, merge with previous if possible
 */
static inline int vector_offset_add_with_merging(vector_offset* vec, Offsets o) {
    if (vec->nbelems > 0) {
        Offsets* latest = &(vec->tab[vec->nbelems - 1]);
        if ((latest->old_end == o.old_start) && (latest->new_end == o.new_start)) {
            latest->old_end = o.old_end;
            latest->new_end = o.new_end;
            return vec->nbelems - 1;
        }
    }

    return vector_offset_add(vec, o);
}


/**
 * Check if a Common offset item is valid
 */
static int DetectCommonIncoherency(
    int old_size, int old_pos, int old_limit,
    int new_size, int new_pos, int new_limit)
{
    if ((old_limit < old_pos) || (new_limit < new_pos)) {
        return 1;
    }
    else if ((old_limit > old_size) || (new_limit > new_size)) {
        return 1;
    }
    else if ((old_limit - old_pos) != (new_limit - new_pos)) {
        return 1;
    }
    return 0;
}



static int global_shift_from_modified_offsets(const vector_offset* offsets) {
    int nb_offsets_items = offsets->nbelems;
    int global_shift = 0;
    for (int i = 0; i < nb_offsets_items; i++) {
        Offsets x = offsets->tab[i];
        int size_old_sequence = x.old_end - x.old_start;
        int size_new_sequence = x.new_end - x.new_start;
        global_shift += (size_new_sequence - size_old_sequence);
    }
    //u_printf("SHIFT %d\n\n", global_shift);
    return global_shift;
}


/**
 * Convert offset data with modified zone to list of common offsets
 *
 */
vector_offset* modified_offsets_to_common(const vector_offset* offsets, int old_size, int new_size) {
    if ((old_size == -1) && (new_size != -1)) {
        old_size = new_size - global_shift_from_modified_offsets(offsets);
    } else if ((old_size != -1) && (new_size == -1)) {
        new_size = old_size + global_shift_from_modified_offsets(offsets);
    }

    int nb_offsets_items = offsets->nbelems;
    vector_offset* inverted_vector_offset = new_vector_offset(nb_offsets_items + 1);
    for (int i = 0; i < nb_offsets_items; i++) {
        Offsets curOffset = offsets->tab[i];
        Offsets prevOffset;
        if (i > 0) {
            prevOffset = offsets->tab[i - 1];
        }
        else {
            prevOffset.old_end = prevOffset.new_end = 0;
        }

        Offsets CommonOffset;
        CommonOffset.old_start = prevOffset.old_end;
        CommonOffset.old_end = curOffset.old_start;
        CommonOffset.new_start = prevOffset.new_end;
        CommonOffset.new_end = curOffset.new_start;
        if (DetectCommonIncoherency(old_size, CommonOffset.old_start, CommonOffset.old_end,
            new_size, CommonOffset.new_start, CommonOffset.new_end)) {
            free_vector_offset(inverted_vector_offset);
            error("coherency problem on offset file");
            return NULL;
        }

        if (CommonOffset.new_start != CommonOffset.new_end) {
            vector_offset_add(inverted_vector_offset, CommonOffset);
        }
    }

    Offsets LastCommonOffset;
    if (nb_offsets_items > 0) {
        LastCommonOffset.old_start = offsets->tab[nb_offsets_items - 1].old_end;
        LastCommonOffset.new_start = offsets->tab[nb_offsets_items - 1].new_end;
    } else {
        LastCommonOffset.old_start = 0;
        LastCommonOffset.new_start = 0;
    }
    LastCommonOffset.old_end = old_size;
    LastCommonOffset.new_end = new_size;


    if (DetectCommonIncoherency(old_size, LastCommonOffset.old_start, LastCommonOffset.old_end,
        new_size, LastCommonOffset.new_start, LastCommonOffset.new_end)) {
        free_vector_offset(inverted_vector_offset);
        error("coherency problem on offset file");
        return NULL;
    }

    if (LastCommonOffset.new_start != LastCommonOffset.new_end) {
        vector_offset_add(inverted_vector_offset, LastCommonOffset);
    }

    return inverted_vector_offset;
}


vector_offset* common_offsets_to_modified(const vector_offset* common_offsets, int old_size, int new_size) {
    if (common_offsets == NULL) {
        return NULL;
    }
    int nb_common_offsets_items = common_offsets->nbelems;
    vector_offset* modifed_vector_offset = new_vector_offset(nb_common_offsets_items + 2);

    Offsets latest_common;
    latest_common.old_start = latest_common.old_end = latest_common.new_start = latest_common.new_end = 0;
    for (int i = 0; i < nb_common_offsets_items; i++) {
        Offsets current_common = common_offsets->tab[i];

        if ((current_common.old_end - current_common.old_start) !=
            (current_common.new_end - current_common.new_start)) {
            error("Mismatch in length in common offset");
            free_vector_offset(modifed_vector_offset);
            return NULL;
        }

        Offsets modified_offset;
        modified_offset.old_start = latest_common.old_end;
        modified_offset.new_start = latest_common.new_end;

        modified_offset.old_end = current_common.old_start;
        modified_offset.new_end = current_common.new_start;

        if ((modified_offset.old_end > modified_offset.old_start) ||
            (modified_offset.new_end > modified_offset.new_start)) {
            vector_offset_add(modifed_vector_offset, modified_offset);
        }
        latest_common = current_common;
    }

    Offsets latest_modified_offset;
    latest_modified_offset.old_start = latest_common.old_end;
    latest_modified_offset.new_start = latest_common.new_end;

    latest_modified_offset.old_end = old_size;
    latest_modified_offset.new_end = new_size;

    if ((latest_modified_offset.old_end > latest_modified_offset.old_start) ||
        (latest_modified_offset.new_end > latest_modified_offset.new_start)) {
        vector_offset_add(modifed_vector_offset, latest_modified_offset);
    }

    return modifed_vector_offset;
}


vector_offset* common_offsets_to_modifed(const vector_offset* offsets, int old_size, int new_size) {

    int nb_offsets_items = offsets->nbelems;
    vector_offset* inverted_vector_offset = new_vector_offset(nb_offsets_items + 2);
    for (int i = 0; i < nb_offsets_items; i++) {
        Offsets curOffset = offsets->tab[i];
        Offsets prevOffset;
        if (i > 0) {
            prevOffset = offsets->tab[i - 1];
        }
        else {
            prevOffset.old_end = prevOffset.new_end = 0;
        }

        Offsets DifferentOffset;
        DifferentOffset.old_start = prevOffset.old_end;
        DifferentOffset.old_end = curOffset.old_start;
        DifferentOffset.new_start = prevOffset.new_end;
        DifferentOffset.new_end = curOffset.new_start;

        vector_offset_add(inverted_vector_offset, DifferentOffset);
    }

    Offsets LastDifferentOffset;
    if (nb_offsets_items > 0) {
        LastDifferentOffset.old_start = offsets->tab[nb_offsets_items - 1].old_end;
        LastDifferentOffset.new_start = offsets->tab[nb_offsets_items - 1].new_end;
    }
    else {
        LastDifferentOffset.old_start = 0;
        LastDifferentOffset.new_start = 0;
    }
    LastDifferentOffset.old_end = old_size;
    LastDifferentOffset.new_end = new_size;

    if ((LastDifferentOffset.old_start != LastDifferentOffset.old_end) ||
        (LastDifferentOffset.old_start != LastDifferentOffset.old_end)) {

        vector_offset_add(inverted_vector_offset, LastDifferentOffset);
    }

    return inverted_vector_offset;
}


// redefine very simple min and max
#define offset_min(a,b) (((a) < (b)) ? (a) : (b))
#define offset_max(a,b) (((a) > (b)) ? (a) : (b))

vector_offset* process_common_offsets(const vector_offset* first_offsets, const vector_offset* second_offsets)
{
    if ((first_offsets == NULL) || (second_offsets == NULL)) {
        return NULL;
    }
    int first_nb_offsets_items = first_offsets->nbelems;
    int second_nb_offsets_items = second_offsets->nbelems;
    vector_offset* merged_vector_offset = new_vector_offset(first_nb_offsets_items + second_nb_offsets_items + 1);

    int pos_in_first_offsets = 0;
    for (int i = 0; i < second_nb_offsets_items; i++)
    {
        Offsets common_offset_in_second = second_offsets->tab[i];
        if ((common_offset_in_second.old_end - common_offset_in_second.old_start) !=
            (common_offset_in_second.new_end - common_offset_in_second.new_start))
        {
            free_vector_offset(merged_vector_offset);
            error("Invalid common offset file");
            return NULL;
        }

        while ((common_offset_in_second.old_end - common_offset_in_second.old_start) != 0) {
            for (;;)
            {
                if (pos_in_first_offsets == first_nb_offsets_items) {
                    // we have no common part in first file to process
                    return merged_vector_offset;
                }

                if (first_offsets->tab[pos_in_first_offsets].new_end > common_offset_in_second.old_start) {
                    break;
                }
                pos_in_first_offsets++;
            }

            int nb_common = 0;
            Offsets current_common_in_first = first_offsets->tab[pos_in_first_offsets];
            if (current_common_in_first.new_start > common_offset_in_second.old_start) {
                int skip_second = current_common_in_first.new_start - common_offset_in_second.old_start;
                common_offset_in_second.old_start += skip_second;
                common_offset_in_second.new_start += skip_second;
            }
            if (current_common_in_first.new_start < common_offset_in_second.old_start) {
                int skip_first = common_offset_in_second.old_start - current_common_in_first.new_start;
                current_common_in_first.old_start += skip_first;
                current_common_in_first.new_start += skip_first;
            }

            int len_common = offset_min(current_common_in_first.new_end - current_common_in_first.new_start,
                common_offset_in_second.old_end - common_offset_in_second.old_start);
            if (len_common > 0) {
                Offsets CommonOffsetToWrite;
                nb_common = len_common;
                int shift_in_first = 0;//common_offset_in_second.old_start >= current_common_in_first.new_start;
                CommonOffsetToWrite.old_start = current_common_in_first.old_start + shift_in_first;
                CommonOffsetToWrite.old_end = CommonOffsetToWrite.old_start + nb_common;
                CommonOffsetToWrite.new_start = common_offset_in_second.new_start;
                CommonOffsetToWrite.new_end = CommonOffsetToWrite.new_start + nb_common;
                vector_offset_add_with_merging(merged_vector_offset, CommonOffsetToWrite);
                common_offset_in_second.old_start += nb_common;
                common_offset_in_second.new_start += nb_common;
            }
            else {
                break;
            }
        }
    }

    return merged_vector_offset;
}


// if you want uses old process_offsets, before the version introduced in
// united revision 3832 May 3, 2015

#if (!defined(IGNORE_BUGFIX)) && (!defined(OLD_PROCESS_OFFSET))
vector_offset* process_offsets(const vector_offset* first_offsets, const vector_offset* second_offsets) {


    // We don't known the size of the two file.
    // We calculate possible value
    int global_shift_first = global_shift_from_modified_offsets(first_offsets);
    int global_shift_second = global_shift_from_modified_offsets(second_offsets);

    int last_pos_mentionned_file0 = (first_offsets->nbelems > 0) ? first_offsets->tab[first_offsets->nbelems-1].old_end : 0;
    int last_pos_mentionned_file1a = (first_offsets->nbelems > 0) ? first_offsets->tab[first_offsets->nbelems-1].new_end : 0;
    int last_pos_mentionned_file1b = (second_offsets->nbelems > 0) ? second_offsets->tab[second_offsets->nbelems-1].old_end : 0;
    int last_pos_mentionned_file2 = (second_offsets->nbelems > 0) ? second_offsets->tab[second_offsets->nbelems-1].new_end : 0;


    int size_possible_file_1a = offset_max(last_pos_mentionned_file0 + global_shift_first, last_pos_mentionned_file1a);
    int size_possible_file_1b = offset_max(last_pos_mentionned_file2 - global_shift_second, last_pos_mentionned_file1b);
    int size_possible_file_1 = offset_max(size_possible_file_1a, size_possible_file_1b);

    int size_possible_file_0 = size_possible_file_1 - global_shift_first;
    int size_possible_file_2 = size_possible_file_1 + global_shift_second;

    // if we add the same positive integer to size_possible_file_0, size_possible_file_1, size_possible_file_2
    // the merged_modified result will be the same
    vector_offset* first_offset_common = modified_offsets_to_common(first_offsets, size_possible_file_0, size_possible_file_1);
    vector_offset* second_offset_common = modified_offsets_to_common(second_offsets, size_possible_file_1, size_possible_file_2);
    vector_offset* merged_common = process_common_offsets(first_offset_common, second_offset_common);
    vector_offset* merged_modified = common_offsets_to_modified(merged_common, size_possible_file_0, size_possible_file_2);

    free_vector_offset(first_offset_common);
    free_vector_offset(second_offset_common);
    free_vector_offset(merged_common);

    return merged_modified;
}


/**
* This function takes two offset arrays:
*
* old_offsets=original text => input text
* new_offsets=input text => output text
*
* and it computes and prints in the given file the new offset file:
*
* original text => output text
*
* If old_offsets is NULL, new offsets are saved in the output file
* without any modification.
*/
void process_offsets(const vector_offset* first_offsets, const vector_offset* second_offsets, U_FILE* f) {
if (f == NULL || second_offsets == NULL) return;
if (first_offsets == NULL) {
    save_offsets(f, second_offsets);
}
else {
    vector_offset* processed_offset = process_offsets(first_offsets, second_offsets);
    save_offsets(f, processed_offset);

    free_vector_offset(processed_offset);
}
}


#else


/**
* This function takes two offset arrays:
*
* old_offsets=original text => input text
* new_offsets=input text => output text
*
* and it computes and prints in the given file the new offset file:
*
* original text => output text
*
* If old_offsets is NULL, new offsets are saved in the output file
* without any modification.
*/
void process_offsets(const vector_offset* old_offsets, const vector_offset* new_offsets, U_FILE* f) {
    if (f == NULL || new_offsets == NULL) return;
    if (old_offsets == NULL) {
        /* If there are no old offsets, we just have to save the new ones */
        Offsets x;
        for (int i = 0;i<new_offsets->nbelems;i++) {
            x = new_offsets->tab[i];
            save_offsets(f, x.old_start, x.old_end, x.new_start, x.new_end);
        }
        return;
    }
    int i = 0, j = 0;
    /* shift_A is the current shift between original text and input text. It is
    * to be used when B is before A, in order to adjust B's old coordinates */
    int shift_A = 0;
    /* shift_B is the current shift between input text and output text. It is
    * to be used when A is before B, in order to adjust A's new coordinates */
    int shift_B = 0;
    int A_includes_B_shift = 0;
    int B_includes_A_shift = 0;
    Offsets x, y;
    Overlap o;
    while (j < new_offsets->nbelems) {
        y = x = new_offsets->tab[j++];
        o = (Overlap)-1;
        for (; i < old_offsets->nbelems; i++) {
            x = old_offsets->tab[i];
            /* Note: below, A stands for x's new interval and B stands for y's old interval */
            /*error("\n\nTEST OVERLAP entre %d;%d->%d;%d et %d;%d->%d;%d\n",
            x.old_start,x.old_end,x.new_start,x.new_end,
            y.old_start,y.old_end,y.new_start,y.new_end);*/
            o = overlap(x.new_start, x.new_end, y.old_start, y.old_end);
            //error("o==%s\n",N(o));
            if (o == A_BEFORE_B) {
                save_offsets(f,
                    x.old_start,
                    x.old_end,
                    x.new_start + shift_B,
                    x.new_end + shift_B + A_includes_B_shift);
                A_includes_B_shift = 0;
                shift_A = x.old_end - x.new_end;
            }
            else {
                break;
            }
        }
        if (i < old_offsets->nbelems) {
            /* There may be an overlap */
            if (o == A_INCLUDES_B) {
                /* If A includes B, then we just have to stay on A, note
                * the shift induced by B and then ignore B */
                //error("A includes B entre %d;%d et %d;%d\n",x.new_start,x.new_end,y.old_start,y.old_end);
                int b_old_length = y.old_end - y.old_start;
                int b_new_length = y.new_end - y.new_start;
                A_includes_B_shift += b_new_length - b_old_length;
                continue;
            }
            if (o == B_INCLUDES_A) {
                /* If B includes A, then we just have to stay on B, note
                * the shift induced by A and then ignore A */
                //error("B includes A entre %d;%d et %d;%d\n",x.new_start,x.new_end,y.old_start,y.old_end);
                int a_old_length = x.old_end - x.old_start;
                int a_new_length = x.new_end - x.new_start;
                /* Note: it is normal this shift is not computed in the same way than A_includes_B_shift */
                B_includes_A_shift += a_old_length - a_new_length;
                /* j--: we want to stay on B
                * i++: we want to skip A */
                j--;
                i++;
                continue;
            }
            if (o == A_BEFORE_B_OVERLAP) {
                /* If A overlaps B starting before B, then we have to produce an output */
                //error("A before B overlap entre %d;%d et %d;%d\n",x.new_start,x.new_end,y.old_start,y.old_end);
                int new_shift_A = x.old_end - x.new_end;
                save_offsets(f,
                    x.old_start + shift_A,
                    y.old_end + shift_A + B_includes_A_shift + new_shift_A,
                    x.new_start + shift_B,
                    y.new_end + shift_B + A_includes_B_shift);
                A_includes_B_shift = 0;
                B_includes_A_shift = 0;
                shift_A += new_shift_A;
                shift_B = y.new_end - y.old_end;
                /* We skip A */
                i++;
                continue;
            }
            if (o == A_AFTER_B_OVERLAP) {
                /* If A overlaps B starting after B, then we have to produce an output */
                //error("A after B overlap entre %d;%d et %d;%d\n",x.new_start,x.new_end,y.old_start,y.old_end);
                int new_shift_B = y.new_end - y.old_end;
                //error("new shift B = %d\n",new_shift_B);

                // Bugfix 2015/05/01, svn revision 3828
                // remove the + shift_B on the two last parameters of save_offsets
                // do a #define IGNORE_BUGFIX to revert and compare if needed
#ifdef IGNORE_BUGFIX
                save_offsets(f,
                    y.old_start + shift_A,
                    x.old_end + shift_A + B_includes_A_shift,
                    y.new_start + shift_B,
                    x.new_end + shift_B + A_includes_B_shift + new_shift_B);
#else
                save_offsets(f,
                    y.old_start + shift_A,
                    x.old_end + shift_A + B_includes_A_shift,
                    y.new_start,
                    x.new_end + A_includes_B_shift + new_shift_B);
#endif

                A_includes_B_shift = 0;
                B_includes_A_shift = 0;
                shift_B += new_shift_B;
                shift_A = x.old_end - x.new_end;
                /* We skip A */
                i++;
                continue;
            }
            if (o == A_EQUALS_B) {
                /* If A equals B starting after B, then we have to produce an output */
                //error("A equals B entre %d;%d et %d;%d\n",x.new_start,x.new_end,y.old_start,y.old_end);
                save_offsets(f,
                    x.old_start + shift_A,
                    x.old_end + shift_A + B_includes_A_shift,
                    y.new_start + shift_B,
                    y.new_end + shift_B + A_includes_B_shift);
                A_includes_B_shift = 0;
                B_includes_A_shift = 0;
                shift_A = x.old_end - x.new_end;
                shift_B = y.new_end - y.old_end;
                /* We skip A */
                i++;
                continue;
            }
        }
        if (o != (Overlap)-1 && o != A_AFTER_B && i != old_offsets->nbelems)
            fatal_error("o==%d\n", o);
        /* Default case: A_AFTER_B */
        save_offsets(f,
            y.old_start + shift_A,
            y.old_end + shift_A + B_includes_A_shift,
            y.new_start,
            y.new_end);
        /* The following line was introduced as a bug fix in r2771, but
        * it caused several problems. Since commenting it out solve
        * all those issues, let's do. If someone steps again on the bug
        * that this line was fixing, we will check this out again.
        */
        //shift_A+=B_includes_A_shift;
        B_includes_A_shift = 0;
        shift_B = y.new_end - y.old_end;
    }
    while (i < old_offsets->nbelems) {
        x = old_offsets->tab[i++];
        save_offsets(f,
            x.old_start,
            x.old_end,
            x.new_start + shift_B,
            x.new_end + shift_B + A_includes_B_shift);
        A_includes_B_shift = 0;
    }
}
#endif


/**
 * Saves snt offsets to the given file, as a binary file containing integers.
 * Returns 1 in case of success; 0 otherwise.
 */
int save_snt_offsets(vector_int* snt_offsets,const char* name) {
if (snt_offsets==NULL) {
    fatal_error("Unexpected NULL offsets in save_snt_offsets\n");
}
if (snt_offsets->nbelems%3 != 0) {
    fatal_error("Invalid offsets in save_snt_offsets\n");
}
U_FILE* f=u_fopen(BINARY,name,U_WRITE);
if (f==NULL) return 0;
int ret=(int)(fwrite(snt_offsets->tab,sizeof(int),snt_offsets->nbelems,f));
u_fclose(f);
return (ret==snt_offsets->nbelems);
}


/**
 * Loads snt offsets from the given binary file.
 */
vector_int* load_snt_offsets(const char* name) {
U_FILE* f=u_fopen(BINARY,name,U_READ);
if (f==NULL) return NULL;
long size=get_file_size(f);
if (size%(3*sizeof(int))!=0) {
    u_fclose(f);
    return NULL;
}
vector_int* v=new_vector_int((int)(size/sizeof(int)));
if (size!=0) {
    int n=(int)fread(v->tab,sizeof(int),size/sizeof(int),f);
    u_fclose(f);
    if (n!=(int)(size/sizeof(int))) {
        free_vector_int(v);
        return NULL;
    }
    v->nbelems=v->size;
}
return v;
}


/**
 * This function adds a new token shift to the given snt offsets.
 */
void add_snt_offsets(vector_int* snt_offsets,int token_pos,int shift_before,int shift_after) {
vector_int_add(snt_offsets,token_pos);
vector_int_add(snt_offsets,shift_before);
vector_int_add(snt_offsets,shift_after);
}


/**
 * Reads the start and end positions of each token stored in the file
 * produced by Tokenize's --output_offsets option.
 */
vector_uima_offset* load_uima_offsets(const VersatileEncodingConfig* vec,const char* name) {
U_FILE* f;
f=u_fopen(vec,name,U_READ);
if (f==NULL) {
   return NULL;
}
vector_int* v=new_vector_int();
Ustring* line=new_Ustring();
int a,b,c;
while (EOF!=readline(line,f)) {
    u_sscanf(line->str,"%d%d%d",&a,&b,&c);
    vector_int_add(v,b);
    vector_int_add(v,c);
}
free_Ustring(line);
u_fclose(f);
return (vector_uima_offset*)v;
}




static int compare_offset_translation_by_position(const void* a, const void* b) {
    const offset_translation* translation_a = (const offset_translation*)a;
    const offset_translation* translation_b = (const offset_translation*)b;
    if (translation_a->position_to_translate < translation_b->position_to_translate) return -1;
    if (translation_a->position_to_translate > translation_b->position_to_translate) return 1;
    if (translation_a->sort_order < translation_b->sort_order) return -1;
    if (translation_a->sort_order > translation_b->sort_order) return 1;
    return 0;
}


static int compare_offset_translation_by_sort_order(const void* a, const void* b) {
    const offset_translation* translation_a = (const offset_translation*)a;
    const offset_translation* translation_b = (const offset_translation*)b;

    if (translation_a->sort_order < translation_b->sort_order) return -1;
    if (translation_a->sort_order > translation_b->sort_order) return 1;
    if (translation_a->position_to_translate < translation_b->position_to_translate) return -1;
    if (translation_a->position_to_translate > translation_b->position_to_translate) return 1;
    return 0;
}
/**
 * translate a set of offset
 * ofs is an array of nb_translations items of type offset_translation
 * Before calling the function, you must fill:
 *   (ofs + #)->position_to_translate with a position (offset) to translate
 *   (ofs + #)->sort_order=# (because array will be sorted on position_to_translate, then back to sort_order
 */
void translate_offset(offset_translation* ofs, int nb_translations, const vector_offset* offsets, int revert) {
if (offsets == NULL) {
    return;
}
if ((nb_translations == 0) || (ofs == NULL)) {
    return;
}
int i;
int sorted_by_position = 1;
for (i = 1;i < nb_translations;i++)
    if (((ofs + i)->position_to_translate) < ((ofs + i - 1)->position_to_translate)) {
        sorted_by_position = 0;
        break;
    }
if (!sorted_by_position) {
    qsort(ofs,nb_translations,sizeof(offset_translation), compare_offset_translation_by_position);
}

int last_position_in_offsets = revert ?
    (offsets->tab[offsets->nbelems - 1].new_start + offsets->tab[offsets->nbelems-1].new_end) :
    (offsets->tab[offsets->nbelems - 1].old_start + offsets->tab[offsets->nbelems-1].old_end);
int last_position_to_translate = (ofs + nb_translations - 1)->position_to_translate;
int minimal_filesize = offset_max(last_position_in_offsets, last_position_to_translate);


vector_offset* common_offsets = modified_offsets_to_common(offsets, -1, minimal_filesize);

int pos_common_offsets = 0;
for (i = 0; i < nb_translations; i++) {
    int pos_to_translate = (ofs + i)->position_to_translate;
    for (;;) {
        if (pos_common_offsets == common_offsets->nbelems)
            break;
        int end_current_common = revert ?
            (common_offsets->tab[pos_common_offsets].new_end) :
            (common_offsets->tab[pos_common_offsets].old_end);
        if (end_current_common > pos_to_translate)
            break;
        pos_common_offsets++;
    }

    int current_common_start = -1;
    int current_common_end = -1;

    if (pos_common_offsets < common_offsets->nbelems) {
        current_common_start = revert ?
            (common_offsets->tab[pos_common_offsets].new_start) :
            (common_offsets->tab[pos_common_offsets].old_start);
        current_common_end = revert ?
            (common_offsets->tab[pos_common_offsets].new_end) :
            (common_offsets->tab[pos_common_offsets].old_end);
        int translate_common_start = revert ?
            (common_offsets->tab[pos_common_offsets].old_start) :
            (common_offsets->tab[pos_common_offsets].new_start);

        if ((pos_to_translate >= current_common_start) && (pos_to_translate < current_common_end)) {
            (ofs + i)->translated_position = translate_common_start + (pos_to_translate - current_common_start);
            (ofs + i)->translation_pos_in_common = 1;
        }
        else {
            (ofs + i)->translated_position = translate_common_start + (current_common_end - current_common_start);
            (ofs + i)->translation_pos_in_common = -1;
        }


    } else {
        int last_pos_translated = 0;
        if (common_offsets->nbelems > 0)
            last_pos_translated = revert ?
                ((common_offsets->tab[common_offsets->nbelems-1].old_start) -1) :
                ((common_offsets->tab[common_offsets->nbelems-1].new_start) -1);
        if (last_pos_translated == -1)
            last_pos_translated = 0;
        (ofs + i)->translated_position = last_pos_translated;
        (ofs + i)->translation_pos_in_common = -1;
    }

}

free_vector_offset(common_offsets);

int sorted_by_sort_order = 1;
for (i = 1;i < nb_translations;i++)
    if (((ofs + i)->sort_order) < ((ofs + i - 1)->sort_order)) {
        sorted_by_sort_order = 0;
        break;
    }
if (!sorted_by_sort_order) {
    qsort(ofs, nb_translations, sizeof(offset_translation), compare_offset_translation_by_sort_order);
}
}

} // namespace unitex
