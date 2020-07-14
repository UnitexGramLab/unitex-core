/*
 * Unitex
 *
 * Copyright (C) 2001-2020 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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


#include "Xml.h"
#include "HTMLCharacters.h"
#include "Alphabet.h"
#include "Ustring.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {



struct selpath_info
{
    const unichar* tagname;
    const unichar* tagkey;
    const unichar* tagvalue;
    bool entered_conform_name;
    bool entered_conform_key_value;
};

class XmlSelect
{
   public:
       XmlSelect() :
         path_initialized(false), nbDepthSelPath(0), depthCurPath(-1), buf(NULL), paths(NULL), path_conform(true)
         {};
       ~XmlSelect() {
         clear();
       }
       bool initSelPath(const char* selPath);
       void enterNewTag(const unichar* tag, bool& in_selection);
       void addKeyValueOnTag(const unichar* key, const unichar* value, bool& in_selection);
       void exitTag(bool& in_selection);
   private:
       void clear() {
           path_initialized=false;
           nbDepthSelPath=0;
           depthCurPath=-1;
           free(paths);
           paths=NULL;
           free(buf);
           buf=NULL;
           path_conform=true;
       }
       bool path_initialized;
       int nbDepthSelPath;
       int depthCurPath;
       unichar* buf;
       struct selpath_info* paths;
       bool path_conform;
};

// exemple: /TEI/teiHeader//sourceDesc//analytic/idno[@type='DOI']
bool XmlSelect::initSelPath(const char* selPath)
{
    int maxDepthSelPath = 0;
    size_t pos;
    clear();

    if ((selPath == NULL) || (selPath[0] == '\0'))
      return true;

    size_t l_path=strlen(selPath);
    if (selPath[0]!='/') return false;
    for (pos=0;pos<l_path;pos++) {
      if (selPath[pos]=='/') maxDepthSelPath++;
    }

    buf=(unichar*)malloc(sizeof(unichar*)*(size_t)((maxDepthSelPath*4)+(l_path*2)));
    if (buf == NULL) {
        alloc_error("initSelPath");
        clear();
        return false;
    }
    unichar* browse_buf = buf;


    paths = (struct selpath_info*)malloc(sizeof(struct selpath_info)*(maxDepthSelPath + 1));

    if (paths == NULL) {
        alloc_error("initSelPath");
        clear();
        return false;
    }

    size_t i=0;
    size_t next_start = 1;
    for (pos=1;pos<=l_path;pos++) {
      const bool is_end = ((selPath[pos] == '\0') || (selPath[pos] == '/'));
      if (is_end) {
        size_t end_tag_name=pos;

        paths[i].tagkey = NULL;
        paths[i].tagvalue = NULL;
        paths[i].entered_conform_name = paths[i].entered_conform_key_value = false;

        for (size_t j = next_start; j < pos; j++) {
          if (selPath[j] == '[') {
              end_tag_name = j;
              if (selPath[j+1] != '@') {
                  clear();
                  return false;
              }

              size_t pos_egal;
              for (pos_egal = j + 1; pos_egal < pos; pos_egal++)
                  if (selPath[pos_egal] == '=') break;
              if (pos_egal==pos) {
                  clear();
                  return false;
              }

              u_strcpy_sized(browse_buf, pos_egal - (j+1), selPath + j + 2);
              paths[i].tagkey = browse_buf;
              browse_buf += u_strlen(browse_buf) + 1;

              size_t pos_end_tag_val;
              for (pos_end_tag_val = pos_egal + 1; pos_end_tag_val<l_path;pos_end_tag_val++)
                if (selPath[pos_end_tag_val] == ']')
                  break;

              if (pos_end_tag_val == pos) {
                  clear();
                  return false;
              }

              const char* begin_val = selPath + pos_egal + 1;
              size_t size_val       = pos_end_tag_val - (pos_egal + 1);

              if ((size_val >= 2) && (begin_val[0] == begin_val[size_val - 1]) && (begin_val[0] == '\'')) {
                begin_val++;
                size_val -= 2;
              }

              if ((size_val >= 2) && (begin_val[0] == begin_val[size_val - 1]) && (begin_val[0] == '"')) {
                begin_val++;
                size_val -= 2;
              }

              u_strcpy_sized(browse_buf, size_val+1, begin_val);
              paths[i].tagvalue = browse_buf;
              browse_buf += u_strlen(browse_buf) + 1;

              break;
          }
        }

        u_strcpy_sized(browse_buf,1+end_tag_name-next_start,selPath+next_start);
        paths[i].tagname = browse_buf;
        browse_buf += u_strlen(browse_buf) + 1;
        i++;
        next_start = pos + 1;
      }
    }

    nbDepthSelPath = (int)i;

    return path_initialized = true;
}


void XmlSelect::enterNewTag(const unichar* tag, bool& in_selection)
{
    if (!path_initialized) return;
    if (u_strcmp(tag, "?xml") == 0)
        return;
    if (tag[0] == '/') {
        exitTag(in_selection);
        return;
    }

    bool previousConform = true;
    if ((depthCurPath>0) && (depthCurPath<nbDepthSelPath))
        previousConform = (paths[depthCurPath].entered_conform_name) && (paths[depthCurPath].entered_conform_key_value);
    depthCurPath++;


#ifdef XMLSELECT_VERBOSE
    for (int i = 0; i < depthCurPath;i++) u_printf(" ");
    u_printf("-> enter tag '%S' (depth %i)\n",tag,depthCurPath);
#endif

    if ((depthCurPath >= nbDepthSelPath) || (depthCurPath < 0))
        return;
    struct selpath_info& curPath = paths[depthCurPath];
    if ((curPath.tagname == NULL) || (curPath.tagname[0] == '\0'))
        curPath.entered_conform_name = previousConform;
    else
        curPath.entered_conform_name = previousConform && ((u_strcmp(curPath.tagname,tag) == 0));

    if (!curPath.entered_conform_name)
      curPath.entered_conform_name = curPath.entered_conform_name;

    if (curPath.tagkey == NULL)
        curPath.entered_conform_key_value = curPath.entered_conform_name;
    else
        curPath.entered_conform_key_value = false;

    if ((curPath.entered_conform_name && curPath.entered_conform_key_value) && (depthCurPath+1==nbDepthSelPath))
        in_selection=true;
}

void XmlSelect::addKeyValueOnTag(const unichar* key, const unichar* value, bool& in_selection)
{
    if (!path_initialized) return;
#ifdef XMLSELECT_VERBOSE
    for (int i = 0; i < depthCurPath;i++) u_printf(" ");
    u_printf("-> add key,value '%S'='%S' (depth %i)\n",key,value,depthCurPath);
#endif

    if ((depthCurPath>=nbDepthSelPath) || (depthCurPath<0))
        return;
    struct selpath_info& curPath = paths[depthCurPath];
    if ((curPath.entered_conform_name) && (curPath.tagkey != NULL)) {
        if ((u_strcmp(key,curPath.tagkey)==0) && (u_strcmp(value,curPath.tagvalue)==0)) {
            curPath.entered_conform_key_value=true;
            if (depthCurPath+1==nbDepthSelPath)
                in_selection=true;
        }
    }
}

void XmlSelect::exitTag(bool& in_selection)
{
    if (!path_initialized) return;

#ifdef XMLSELECT_VERBOSE
    for (int i = 0; i < depthCurPath;i++) u_printf(" ");
    u_printf("Exit tag (depth %i)\n",depthCurPath);
#endif

    if (depthCurPath+1 == nbDepthSelPath)
        in_selection = false;

    depthCurPath--;
}

static int skip_tag(U_FILE* f,U_FILE* f_out,int *pos,int *new_pos,vector_offset* offsets,
        UnxmlizeOpts* options,unichar* bastien[],U_FILE* f_bastien,
        Ustring* ustr1, Ustring* ustr2, XmlSelect& xmlSelect, bool& write_enabled);
static int decode_html_char(U_FILE* f,U_FILE* f_out,bool write_enabled,
        int *pos,int *new_pos,vector_offset* offsets,void* html_ctx);


/**
 * Produces a version of the input file removing all xml tags.
 * Returns 1 in case of success; 0 otherwise.
 *
 * If 'html' is non null, special HTML filtering is applied
 * (i.e. skipping script code, replacing any tag by a space).
 */
int unxmlize(U_FILE* input,U_FILE* output,vector_offset* offsets,UnxmlizeOpts* options,
        unichar* bastien[],U_FILE* f_bastien, int tolerate_markup_malformation, const char* selPath) {
int c;
int pos=0,new_pos=0;
void* html_ctx=init_HTML_character_context();
XmlSelect xmlSelect;
// ustr1 and ustr2 are recycling Ustring buffer, to avoid a lot of malloc/free
Ustring* ustr1 = new_Ustring();
Ustring* ustr2 = new_Ustring();
if (selPath == NULL)
  selPath = "";
if (!xmlSelect.initSelPath(selPath)) {
    fatal_error("Invalid xml selection path: '%s'\n", selPath);
    return 0;
}
bool write_enabled = (selPath[0] == '\0');
while ((c=u_fgetc_raw(input))!=EOF) {
    int markup_malformation=0;
    pos++;
    if (c=='<') {
        if (!skip_tag(input,output,&pos,&new_pos,offsets,options,bastien,f_bastien,ustr1,ustr2,xmlSelect,write_enabled)) {
            //free_HTML_character_context(html_ctx);
            markup_malformation=1;
        }
    }
    else if (c=='&') {
        if (!decode_html_char(input,output,write_enabled,&pos,&new_pos,offsets,html_ctx)) {
            //free_HTML_character_context(html_ctx);
            markup_malformation=1;
        }
    } else {
        if (write_enabled) {
            u_fputc_raw((unichar)c,output);
            new_pos++;
        }
    }

    if (markup_malformation!=0) {
        if (tolerate_markup_malformation==0) {
            free_HTML_character_context(html_ctx);
            return 0;
        } else {
          if (write_enabled) {
              u_fputc_raw((unichar)c,output);
              new_pos++;
          }
        }
    }
}

free_HTML_character_context(html_ctx);
free_Ustring(ustr1);
free_Ustring(ustr2);
return 1;
}


void write_offsets(vector_offset* offsets,int a,int b,int c,int d) {
if (offsets!=NULL) {
    vector_offset_add(offsets,a,b,c,d);
}
}


/**
 * This function is called when <!-- was read and it skips everything
 * until --> has been read.
 */
int skip_comment(U_FILE* f,int *pos) {
int c,state=0;
while (state!=3) {
    c=u_fgetc_raw(f);
    if (c==EOF) return 0;
    (*pos)++;
    switch(state) {
    case 0: {
        if (c=='-') state=1;
        break;
    }
    case 1: {
        if (c=='-') state=2;
        else state=0;
        break;
    }
    case 2: {
        if (c=='-') state=2;
        else if (c=='>') state=3;
        else state=0;
        break;
    }
    }
}
return 1;
}


/**
 * Returns 1 if the string seq can be read from the given file;
 * 0 otherwise.
 */
int read(U_FILE* f,const char* seq) {
while (*seq) {
    if (u_fgetc_raw(f)!=*seq) return 0;
    seq++;
}
return 1;
}


/**
 * Same as 'read', but ignoring case.
 */
int read2(U_FILE* f,const char* seq) {
int c;
while (*seq) {
    c=u_fgetc_raw(f);
    if (u_toupper((unichar)c)!=u_toupper(*seq)) return 0;
    seq++;
}
return 1;
}


/**
 * This function assumes that <![CDATA[ has been read. It will consider
 * all text until ]]> is read. This text will be taken as is, except for
 * &gt; that will be turned into a >.
 */
static int skip_cdata(U_FILE* f,U_FILE* f_out,bool write_enabled,int *pos,int *new_pos,vector_offset* offsets) {
int c,state=0;
while (state!=3) {
    c=u_fgetc_raw(f);
    if (c==EOF) return 0;
    (*pos)++;
    switch (state) {
    case 0: {
        if (c==']') state=1;
        else if (c=='&') state=4;
        else {
            if (write_enabled) {
                u_fputc_raw((unichar)c,f_out);
                (*new_pos)++;
            }
            state=0;
        }
        break;
    }
    case 1: {
        if (c==']') state=2;
        else if (c=='&') {
            /* To come here, we had read a ] that will not be used as a
             * part of ]]> so we have to dump this char in the output file */
            state=4;
            if (write_enabled) {
                u_fputc_raw(']',f_out);
                (*new_pos)++;
            }
        }
        else {
            /* We have to save ]c */
            if (write_enabled) {
                u_fputc_raw(']',f_out);
                u_fputc_raw((unichar)c,f_out);
                (*new_pos)+=2;
            }
            state=0;
        }
        break;
    }
    case 2: {
        if (c==']') {
            /* This is the third ], we have to save one */
            if (write_enabled) {
                u_fputc_raw(']',f_out);
                (*new_pos)++;
            }
            state=2;
        }
        else if (c=='&') {
            /* We have ]] to save */
            if (write_enabled) {
                u_fputc_raw(']',f_out);
                u_fputc_raw(']',f_out);
                (*new_pos)+=2;
            }
            state=4;
        }
        else if (c=='>') state=3;
        else {
            /* We have ]]c to save */
            if (write_enabled) {
                u_fputc_raw(']',f_out);
                u_fputc_raw(']',f_out);
                u_fputc_raw((unichar)c,f_out);
                (*new_pos)+=3;
            }
            state=0;
        }
        break;
    }
    case 4: {
        if (c==']') {
            /* We have & to save */
            if (write_enabled) {
                u_fputc_raw('&',f_out);
                (*new_pos)++;
            }
            state=1;
        }
        else if (c=='&') {
            /* We have & to save */
            if (write_enabled) {
                u_fputc_raw('&',f_out);
                (*new_pos)++;
            }
            state=4;
        }
        else if (c=='g') state=5;
        else {
            /* We have &c to save */
            if (write_enabled) {
                u_fputc_raw('&',f_out);
                u_fputc_raw((unichar)c,f_out);
                (*new_pos)+=2;
            }
            state=0;
        }
        break;
    }
    case 5: {
        if (c==']') {
            /* We have &g to save */
            if (write_enabled) {
                u_fputc_raw('&',f_out);
                u_fputc_raw('g',f_out);
                (*new_pos)+=2;
            }
            state=1;
        } else if (c=='&') {
            /* We have &g to save */
            if (write_enabled) {
                u_fputc_raw('&',f_out);
                u_fputc_raw('g',f_out);
                (*new_pos)+=2;
            }
            state=4;
        } else if (c=='t') state=6;
        else {
            /* We have &g to save */
            if (write_enabled) {
                u_fputc_raw('&',f_out);
                u_fputc_raw('g',f_out);
                (*new_pos)+=2;
            }
            state=0;
        }
        break;
    }
    case 6: {
        if (c==']') {
            /* We have &gt to save */
            if (write_enabled) {
                u_fputc_raw('&',f_out);
                u_fputc_raw('g',f_out);
                u_fputc_raw('t',f_out);
                (*new_pos)+=3;
            }
            state=1;
        }
        else if (c=='&') {
            /* We have &gt to save */
            if (write_enabled) {
                u_fputc_raw('&',f_out);
                u_fputc_raw('g',f_out);
                u_fputc_raw('t',f_out);
                (*new_pos)+=3;
            }
            state=4;
        } else if (c==';') {
            /* We have to replace &gt; by > */
            if (write_enabled) {
                u_fputc_raw('>',f_out);
                write_offsets(offsets,(*pos)-4,*pos,*new_pos,(*new_pos)+1);
                (*new_pos)++;
            }
            state=0;
        }
        else {
            /* We have &gt to save */
            if (write_enabled) {
                u_fputc_raw('&',f_out);
                u_fputc_raw('g',f_out);
                u_fputc_raw('t',f_out);
                (*new_pos)+=3;
            }
            state=0;
        }
        break;
    }
    }
}
/* We have to write ]]> => nothing in the offsets */
write_offsets(offsets,(*pos)-3,*pos,*new_pos,(*new_pos));
return 1;
}


/**
 * This function is called when < was read and it skips everything
 * until > has been read.
 */
static int skip_normal_tag(U_FILE* f,int *pos,unichar* bastien[],U_FILE* f_bastien,
                           Ustring* ustr1, Ustring* ustr2, XmlSelect& xmlSelect, bool& write_enabled) {
int c;
Ustring* ustr=ustr1;
Ustring* ustr_key=ustr2;
int tag_name_found=(bastien!=NULL)?0:-1;
int tag_index=-1;
int old_pos=*pos;
bool read_tag_name=true;
int prev_c=0;
empty(ustr);
empty(ustr_key);
while ((c = u_fgetc_raw(f)) != '>') {
    if (c==EOF) goto err;
    (*pos)++;
    if (c=='"') {
        /* If we have to skip an attribute between double quotes */
        empty(ustr);
        while ((c=u_fgetc_raw(f))!='"') {
            if (c==EOF) goto err;
            u_strcat(ustr,(unichar)c);
            (*pos)++;
        }
        if (tag_name_found==2) {
            tag_name_found=3;
            for (int i=tag_index;i<10;i++) {
                free(bastien[i]);
                bastien[i]=NULL;
            }
            bastien[tag_index]=u_strdup(ustr->str);
            empty(ustr);
            for (int i=0;i<10;i++) {
                if (bastien[i]!=NULL) {
                    if (ustr->len!=0) {
                        u_strcat(ustr,'-');
                    }
                    u_strcat(ustr,bastien[i]);
                }
            }
            u_fprintf(f_bastien,"%d %S\n",old_pos,ustr->str);
        }
        (*pos)++;
        continue;
    }
    if (c=='\'') {
        /* If we have to skip an attribute between single quotes */
        while ((c=u_fgetc_raw(f))!='\'') {
            if (c==EOF) goto err;
            (*pos)++;
        }
        (*pos)++;
        continue;
    }
    if (c!=' ' && c!='=') {
        u_strcat(ustr,(unichar)c);
    }
    if (c==' ') {
      if (read_tag_name) {
          xmlSelect.enterNewTag(ustr->str, write_enabled);
      } else {
          xmlSelect.addKeyValueOnTag(ustr_key->str, ustr->str, write_enabled);
      }
      empty(ustr_key);
      read_tag_name = false;
        if (tag_name_found==0) {
            tag_name_found=1;
            unichar z,foo;
            if (1==u_sscanf(ustr->str,"R%C%C",&z,&foo) && z>='0' && z<='9') {
                tag_index=z-'0';
            }
        }
        empty(ustr);
    }
    if (c=='=') {
        if (tag_name_found==1) {
            if (!u_strcmp(ustr->str,"utxShort")) {
                tag_name_found=2;
            }
        }
        u_switch(ustr_key, ustr);
        empty(ustr);
    }
    prev_c=c;
}

if (read_tag_name) {
    xmlSelect.enterNewTag(ustr->str,write_enabled);
}

if (!read_tag_name)
{
    xmlSelect.addKeyValueOnTag(ustr_key->str, ustr->str, write_enabled);
}

if (prev_c =='/') {
  xmlSelect.exitTag(write_enabled);
}
(*pos)++;
return 1;
err:
return 0;
}


/**
 * This function is called when '<script ' or '<script>' was read and it skips everything
 * until </script> has been read.
 *
 * NOTE: I (S.P.) know that this may bug if a script contains the pattern
 * </script> in a comment or a string, but I don't want to waste time
 * taking into account such a silly situation with a full and painful
 * analysis of script codes.
 */
int skip_script(U_FILE* f,int *pos) {
int c,state=0;
while (state!=9) {
    c=u_fgetc_raw(f);
    if (c==EOF) return 0;
    (*pos)++;
    switch(state) {
        case 0: {
        if (c=='<') state=1;
        break;
    }
    case 1: {
        if (c=='/') state=2;
        else state=0;
        break;
    }
    case 2: {
        if (c=='s' || c=='S') state=3;
        else state=0;
        break;
    }
    case 3: {
        if (c=='c' || c=='C') state=4;
        else state=0;
        break;
    }
    case 4: {
        if (c=='r' || c=='R') state=5;
        else state=0;
        break;
    }
    case 5: {
        if (c=='i' || c=='I') state=6;
        else state=0;
        break;
    }
    case 6: {
        if (c=='p' || c=='P') state=7;
        else state=0;
        break;
    }
    case 7: {
        if (c=='t' || c=='T') state=8;
        else state=0;
        break;
    }
    case 8: {
        if (c=='>') state=9;
        else state=0;
        break;
    }
}
}
return 1;
}


/**
 * This function is called when a '<' has just been read. It is
 * then supposed to skip the tag. If offsets is not NULL, we
 * save the offsets shifts.
 * Returns 1 in case of success; 0 if the tag is malformed.
 */
static int skip_tag(U_FILE* f,U_FILE* f_out,int *pos,int *new_pos,vector_offset* offsets,
                    UnxmlizeOpts* options,unichar* bastien[],U_FILE* f_bastien,
                    Ustring* ustr1, Ustring* ustr2, XmlSelect& xmlSelect, bool& write_enabled) {
int old_pos=(*pos)-1;
long current=ftell(f);
/* We may read a comment */
if (read(f,"!--")) {
    (*pos)+=3;
    if (!skip_comment(f,pos)) {
        error("Invalid comment\n");
        fseek(f,current,SEEK_SET);
        return 0;
    }
    if (options->comments==UNXMLIZE_IGNORE) {
        write_offsets(offsets,old_pos,*pos,*new_pos,*new_pos);
    } else {
        /* We may have to replace comments by a space */
        if (write_enabled) {
            write_offsets(offsets,old_pos,*pos,*new_pos,(*new_pos)+1);
            (*new_pos)++;
            u_fputc_raw(' ',f_out);
        }
    }
    return 1;
}
fseek(f,current,SEEK_SET);
/* Or a CDATA */
if (read(f,"![CDATA[")) {
    (*pos)+=8;
    write_offsets(offsets,old_pos,*pos,*new_pos,*new_pos);
    if (!skip_cdata(f,f_out,write_enabled,pos,new_pos,offsets)) {
        error("Invalid CDATA\n");
        fseek(f,current,SEEK_SET);
        return 0;
    }
    return 1;
}
fseek(f,current,SEEK_SET);
/* Or a html script code */
if (options->scripts!=UNXMLIZE_DO_NOTHING) {
    int ok=read2(f,"script ");
    if (!ok) {
        fseek(f,current,SEEK_SET);
        ok=read2(f,"script>");
    }
    if (ok) {
        (*pos)+=7;
        if (!skip_script(f,pos)) {
            error("Invalid script code\n");
            fseek(f,current,SEEK_SET);
            return 0;
        }
        if (options->scripts==UNXMLIZE_IGNORE) {
            write_offsets(offsets,old_pos,*pos,*new_pos,*new_pos);
        } else {
            /* We replace script sections by a space */
            if (write_enabled) {
                write_offsets(offsets,old_pos,*pos,*new_pos,(*new_pos)+1);
                (*new_pos)++;
                u_fputc_raw(' ',f_out);
            }
        }
        return 1;
    }
}
fseek(f,current,SEEK_SET);
/* Or a normal tag */
if (!skip_normal_tag(f,pos,bastien,f_bastien,ustr1,ustr2,xmlSelect,write_enabled)) {
    error("Invalid xml tag\n");
    fseek(f,current,SEEK_SET);
    return 0;
}
if (options->normal_tags==UNXMLIZE_IGNORE) {
    write_offsets(offsets,old_pos,*pos,*new_pos,*new_pos);
} else {
    /* We replace tags by a space */
    if (write_enabled) {
        write_offsets(offsets,old_pos,*pos,*new_pos,(*new_pos)+1);
        (*new_pos)++;
        u_fputc_raw(' ',f_out);
    }
}
return 1;
}


/**
 * This function is called when & has been read. It reads an html char
 * like &gt; or &#206;
 */
static int decode_html_char(U_FILE* f,U_FILE* f_out,bool write_enabled,
                            int *pos,int *new_pos,vector_offset* offsets,void* html_ctx) {
char tmp[32];
int c,i=0;
long current=ftell(f);
while (i<32 && (c=u_fgetc_raw(f))!=';') {
    if (c>255) {
        /* Should not happen with valid html chars */
        tmp[i]='\0';
        error("Invalid html char: &%s%C;\n",tmp,c);
        fseek(f,current,SEEK_SET);
        return 0;
    }
    tmp[i++]=(char)c;
    (*pos)++;
}
if (i==32) {
    /* Should not happen with valid html chars */
    tmp[31]='\0';
    error("Too long HTML character: %s\n",tmp);
    error("This may come from an invalid & found in text instead of &amp;\n");
    fseek(f,current,SEEK_SET);
    return 0;
}
(*pos)++;
tmp[i]='\0';
c=get_HTML_character(html_ctx,tmp,1);
if (c<0) {
    error("Invalid html character: &%s;\n",tmp);
    fseek(f,current,SEEK_SET);
    return 0;
}
if (write_enabled) {
    u_fputc_raw((unichar)c,f_out);
    write_offsets(offsets,(*pos)-(2+i),*pos,*new_pos,(*new_pos)+1);
    (*new_pos)++;
}
return 1;
}

} // namespace unitex
