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
//---------------------------------------------------------------------------
#include "GF_lib.h"
//---------------------------------------------------------------------------
#ifdef TRE_WCHAR
int HASH_FILTERS_DIM = 1024;

void    ExtractInflected( unichar*, wchar_t* );
void    SplitFilter( unichar*, unichar*, char* );
void    w_strcpy( wchar_t*, unichar* );

/*----------------------------------------------------------------------------*/

MasterGF_T* CreateMasterGF( Automate_fst2* fst2 , Alphabet* alph )
{
    struct string_hash* hashFilters = new_string_hash_N(HASH_FILTERS_DIM);
    Etiquette* fst2Labels = fst2 -> etiquette;
    int i, ccode;
    
    for (i = 0; i < fst2 -> nombre_etiquettes; ++i) {
        if (fst2Labels[i] -> contentGF != NULL) {
            fst2Labels[i] -> entryMasterGF = get_hash_number(fst2Labels[i] -> contentGF, hashFilters);
        }
        else
            fst2Labels[i] -> entryMasterGF = -1;
            
        }

/*
printf("\n>>> CreateMasterGF :\n");
printf("fst2->nombre_etiquettes=%d\n", fst2 -> nombre_etiquettes);
for (i = 0; i < fst2 -> nombre_etiquettes; ++i) {
    printf("i=%d", i); 
    printf(" - fst2Labels[i] -> contenu="); u_prints(fst2Labels[i] -> contenu);
    printf(" - fst2Labels[i] -> contentGF="); u_prints(fst2Labels[i] -> contentGF);
    printf(" - fst2Labels[i] -> entryMasterGF=%d", fst2Labels[i] -> entryMasterGF);
    printf("\n");
    }
*/
        
    MasterGF_T* masterGF = (MasterGF_T *) malloc(sizeof(MasterGF_T));
    
    if (hashFilters -> N > 0) {
        
        unichar filterContent[512];
        char    filterOptions[512];
        int     regBasic, /*regICase,*/ cflags;
        wchar_t warray[512];
        
        masterGF -> tabDim = hashFilters -> N;
        masterGF -> tab = (MasterGFTab_T *) malloc(sizeof(MasterGFTab_T) * hashFilters -> N);
        
        for (i = 0; i < hashFilters -> N; ++i) {
        
            masterGF -> tab[i].content = NULL;
            masterGF -> tab[i].options = NULL;
            masterGF -> tab[i].preg = NULL;
        
            SplitFilter(hashFilters -> tab[i], filterContent, filterOptions);
            
            masterGF -> tab[i].options = (char *) malloc(sizeof(char) * (1 + strlen(filterOptions)));
            strcpy(masterGF -> tab[i].options, filterOptions);
            
            regBasic = 0; //regICase = 0;
            int replaceLetters=1;
            for (unsigned int j = 0; j < strlen(filterOptions); ++j) {
                switch (filterOptions[j]) {
                    case 'f':   replaceLetters=0;
                                break;
                    case 'b':   regBasic = 1;
                                break;
                    /*case 'i':   regICase = 1;
                                break;*/
                    default:
                                char errbuf[512];
                                u_to_char(errbuf, filterContent);
                                fprintf(stderr, "Morphological filter '%s' : ", errbuf);
                                fprintf(stderr, "Invalid option(s) : '%s'\n", filterOptions);
                                free_string_hash(hashFilters);
                                FreeMasterGF(masterGF, i);
                                return NULL;
                    }
                }

            if (replaceLetters==1) {
               // replace ".+e" par ".+[eE]"
               unichar temp[1024];
               replace_letter_by_letter_set(alph,temp,filterContent);
               u_strcpy(filterContent,temp);
            }
            masterGF -> tab[i].content = (unichar *) malloc(sizeof(unichar) * (1 + u_strlen(filterContent)));
            u_strcpy(masterGF -> tab[i].content, filterContent);

            cflags = REG_NOSUB;
            if (! regBasic) cflags |= REG_EXTENDED;
            //if (regICase)   cflags |= REG_ICASE;
            
            masterGF -> tab[i].preg = (regex_t *) malloc(sizeof(regex_t));
            
            w_strcpy(warray, masterGF -> tab[i].content);
            ccode = regwcomp(masterGF -> tab[i].preg, warray, cflags);

            if (ccode != 0) {
                char errbuf[512];
                u_to_char(errbuf, masterGF -> tab[i].content);
                fprintf(stderr, "Morphological filter '%s' : ", errbuf);
                regerror(ccode, masterGF -> tab[i].preg, errbuf, 512);
                fprintf(stderr, "Syntax error : %s\n", errbuf);
                free_string_hash(hashFilters);
                FreeMasterGF(masterGF, i);
                return NULL;
                }
            }
            
        }
    else {
        masterGF -> tabDim = 0;
        masterGF -> tab = NULL;
        }
    
    free_string_hash(hashFilters);

/*
printf("masterGF -> tabDim=%d\n", masterGF -> tabDim);
for (i = 0; i < masterGF -> tabDim; ++i) {
    printf("i=%d", i); 
    printf(" - masterGF -> tab[i].content="); u_prints(masterGF -> tab[i].content);
    printf(" - masterGF -> tab[i].options=%s", masterGF -> tab[i].options);
    printf("\n");
    }
*/
    
    return masterGF;

}

/*----------------------------------------------------------------------------*/

void FreeMasterGF( MasterGF_T* masterGF, int dim )
{
    int i, n;
    if (masterGF == NULL) return;
    if (masterGF->tab == NULL) {
       free(masterGF);
       return;
    }
    n = (dim == 0) ? masterGF -> tabDim : dim + 1;
    for (i = 0; i < n; ++i) {
        if (masterGF -> tab[i].options != NULL) free(masterGF -> tab[i].options);
        if (masterGF -> tab[i].content != NULL) free(masterGF -> tab[i].content);
        if (masterGF -> tab[i].preg != NULL) regfree(masterGF -> tab[i].preg);
        }
    free(masterGF -> tab);
    free(masterGF);
}

/*----------------------------------------------------------------------------*/

IndexGF_T* CreateIndexGF( MasterGF_T* masterGF, struct string_hash* tok )
{
    int i, j, k;
    wchar_t inflected[512];
    
    IndexGF_T* indexGF = (IndexGF_T *) malloc(sizeof(IndexGF_T));
    if (indexGF == NULL) return NULL;
    
    if (masterGF -> tabDim > 0) {
        indexGF -> rowDim = tok -> N;
        indexGF -> bitDim = masterGF -> tabDim;
        indexGF -> colDim = (indexGF -> bitDim / 8) + 1;
        indexGF -> tab = (unsigned char * *) malloc(sizeof(unsigned char *) * indexGF -> rowDim);
        if (indexGF -> tab == NULL) {
            free(indexGF);
            return NULL;
            }
        
        for (i = 0; i < indexGF -> rowDim; ++i) {
            
            if (tok -> tab[i][0] == '{')
                ExtractInflected(tok -> tab[i], inflected);
            else
                w_strcpy(inflected, tok -> tab[i]);
            
            indexGF -> tab[i] = (unsigned char *) malloc(sizeof(unsigned char) * indexGF -> colDim);
            if (indexGF -> tab[i] == NULL) {
                FreeIndexGF(indexGF, i - 1);
                return NULL;
                }
            
            for (j = 0; j < indexGF -> colDim; ++j)
                indexGF -> tab[i][j] = 0;
            
            for (k = 0; k < indexGF -> bitDim; ++k)
                if (regwexec(masterGF -> tab[k].preg, inflected, 0, NULL, 0) == 0)
                    indexGF -> tab[i][k/8] = (unsigned char) (indexGF -> tab[i][k/8] | (1 << (k%8)));
            }    
        }
    
    else {
        indexGF -> rowDim = indexGF -> colDim = indexGF -> bitDim = 0;
        indexGF -> tab = NULL;
        }

/*
printf("indexGF -> rowDim=%d\n", indexGF -> rowDim);
printf("indexGF -> colDim=%d\n", indexGF -> colDim);
printf("indexGF -> bitDim=%d\n", indexGF -> bitDim);
char c;
int n;
for (i =0; i < indexGF -> rowDim; ++i) {
    n = 0;
    for (k = 0; k < indexGF -> bitDim; ++k) 
        if (indexGF -> tab[i][k/8] &  (1<<(k%8))) ++n;
    if (n > 0) {
        printf("i=%d", i);
        printf("\t");
        for (k = 0; k < indexGF -> bitDim; ++k) {
            c = indexGF -> tab[i][k/8] &  (1<<(k%8)) ? '1' : '-';
            printf("%c", c);
            }
        printf("\t"); u_prints(tok -> tab[i]);
        printf("\n");
        }
    }
*/
        
    return indexGF;
}

/*----------------------------------------------------------------------------*/

void FreeIndexGF( IndexGF_T* indexGF, int dim )
{
    int i, n;
    n = (dim == 0) ? indexGF -> rowDim : dim + 1;
    for (i = 0; i < n; ++i) free(indexGF -> tab[i]);
    free(indexGF -> tab);
    free(indexGF);
}

/*----------------------------------------------------------------------------*/

int MatchGF( unichar* token, regex_t* pattern )
{
    wchar_t tmp[512];
    
    if (token[0] == '{') 
        ExtractInflected(token, tmp);
    else
        w_strcpy(tmp, token);
        
    return regwexec(pattern, tmp, 0, NULL, 0);
}

/*----------------------------------------------------------------------------*/

int MatchRawGF(MasterGF_T * masterGF, unichar* s, int pattern )
{
    wchar_t tmp[512];

    w_strcpy(tmp, s);
        
    return regwexec(masterGF->tab[pattern].preg, tmp, 0, NULL, 0);
}

/*----------------------------------------------------------------------------*/

int OptMatchGF( IndexGF_T* indexGF, int iToken, int iPattern )
{
    return indexGF -> tab[iToken][iPattern/8] & (1 << (iPattern%8)) ? 0 : 1;
}



/*----------------------------------------------------------------------------*/

void ExtractInflected( unichar token[], wchar_t inflected[] )
{
    int i, j;
    i = 1; j = 0;
    while (token[i] != '\0' && token[i] != ',' && token[i] != '}')
        inflected[j++] = (wchar_t) token[i++];
    inflected[j] = '\0';
    return;
}

/*----------------------------------------------------------------------------*/

void SplitFilter( unichar filterAll[], unichar filterContent[], char filterOptions[] )
{
    int i, j;
    
    i = 2; j = 0;
    while (filterAll[i] != '\0' && (filterAll[i] != '>' || filterAll[i+1] != '>'))
        filterContent[j++] = filterAll[i++];
    filterContent[j] = '\0';
    
    j = 0;
    if (filterAll[i] != '\0' && filterAll[i+2] == '_') {
        i += 3;
        while (filterAll[i] != '\0' && filterAll[i] != '_')
            filterOptions[j++] = (char) filterAll[i++];
        }
    filterOptions[j] = '\0';
    return;
}

/*----------------------------------------------------------------------------*/

void w_strcpy( wchar_t* target, unichar* source )
{
    int i;
    i = 0;
    while ((target[i] = (wchar_t) source[i]) != L'\0') ++i;
}

/*----------------------------------------------------------------------------*/

#endif
