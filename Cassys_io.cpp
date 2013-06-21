/*
 * Unitex
 *
 * Copyright (C) 2001-2013 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
 * Cassys_io.cpp
 *
 *  Created on: 8 oct. 2012
 *      Author: David Nott, Nathalie Friburger (nathalie.friburger@univ-tours.fr)
 */

#include "Cassys_io.h"
#include "Cassys_lexical_tags.h"

#include "File.h"
#include "Snt.h"
#include "Unicode.h"

#include "DirHelper.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {



int make_directory(const char *path){
	return mkDirPortable(path);
}


int copy_directory_snt_item(const char*dest_snt_dir,const char*src_snd_dir,const char*filename,int mandatory)
{
    char fullname_src[1024];
    char fullname_dest[1024];

    sprintf(fullname_src,"%s%s",src_snd_dir,filename);
    sprintf(fullname_dest,"%s%s",dest_snt_dir,filename);

    int ret_copy = af_copy(fullname_src,fullname_dest);
    if (!mandatory)
        return 1;
    return (ret_copy == 0);
}



int copy_directory_snt_content(const char*dest_snt_dir,const char*src_snd_dir)
{
    int result=1;

    result = result && copy_directory_snt_item(dest_snt_dir,src_snd_dir,"concord.ind",0);
    result = result && copy_directory_snt_item(dest_snt_dir,src_snd_dir,"concord.n",0);
    result = result && copy_directory_snt_item(dest_snt_dir,src_snd_dir,"concord.txt",0);
    result = result && copy_directory_snt_item(dest_snt_dir,src_snd_dir,"dlc",0);
    result = result && copy_directory_snt_item(dest_snt_dir,src_snd_dir,"dlf",0);
    result = result && copy_directory_snt_item(dest_snt_dir,src_snd_dir,"enter.pos",1);
    result = result && copy_directory_snt_item(dest_snt_dir,src_snd_dir,"err",0);
    result = result && copy_directory_snt_item(dest_snt_dir,src_snd_dir,"stat_dic.n",0);
    result = result && copy_directory_snt_item(dest_snt_dir,src_snd_dir,"stats.n",0);
    result = result && copy_directory_snt_item(dest_snt_dir,src_snd_dir,"tags.ind",0);
    result = result && copy_directory_snt_item(dest_snt_dir,src_snd_dir,"text.cod",1);
    result = result && copy_directory_snt_item(dest_snt_dir,src_snd_dir,"tok_by_alph.txt",0);
    result = result && copy_directory_snt_item(dest_snt_dir,src_snd_dir,"tok_by_freq.txt",0);
    result = result && copy_directory_snt_item(dest_snt_dir,src_snd_dir,"tokens.txt",1);

    return result;
}



int initialize_working_directory(const char *text,int must_create_directory){
	char path[FILENAME_MAX];
	get_path(text,path);

	char canonical_name[FILENAME_MAX];
	remove_path_and_extension(text, canonical_name);

	char extension[FILENAME_MAX];
	get_extension(text,extension);

	char working_directory[FILENAME_MAX];
	sprintf(working_directory, "%s%s%s%c",path, canonical_name, CASSYS_DIRECTORY_EXTENSION, PATH_SEPARATOR_CHAR);

	if (must_create_directory != 0) {
        make_directory(working_directory);
    }

	char text_in_wd[FILENAME_MAX];
	sprintf(text_in_wd, "%s%s_0_0%s",working_directory,canonical_name,extension );
	copy_file(text_in_wd,text);

	char snt_dir_text_in_wd[FILENAME_MAX];
	get_snt_path(text_in_wd, snt_dir_text_in_wd);
    if (must_create_directory != 0) {
        make_directory(snt_dir_text_in_wd);
    }

	char original_snt_dir[FILENAME_MAX];
	get_snt_path(text,original_snt_dir);
	copy_directory_snt_content(snt_dir_text_in_wd, original_snt_dir);

	return 0;
}


//char* create_labeled_files_and_directory(const char *text, int next_transducer_label,int must_create_directory,int must_copy_file) {
char* create_labeled_files_and_directory(
		const char *text,
		int previous_transducer_label,
		int next_transducer_label,
		int previous_iteration,
		int next_iteration,
		int must_create_directory,
		int must_copy_file) {

	char path[FILENAME_MAX];
	get_path(text, path);

	char canonical_text_name[FILENAME_MAX];
	remove_path_and_extension(text, canonical_text_name);

	char extension[FILENAME_MAX];
	get_extension(text, extension);

	char working_directory[FILENAME_MAX];
	sprintf(working_directory, "%s%s%s%c", path, canonical_text_name,
			CASSYS_DIRECTORY_EXTENSION, PATH_SEPARATOR_CHAR);

	// copy the text label i- to i
	char old_labeled_text_name[FILENAME_MAX];
	sprintf(old_labeled_text_name, "%s%s_%d_%d%s", working_directory,
			canonical_text_name, previous_transducer_label, previous_iteration, extension);

	char new_labeled_text_name[FILENAME_MAX];
	sprintf(new_labeled_text_name, "%s%s_%d_%d%s", working_directory,
			canonical_text_name, next_transducer_label, next_iteration, extension);

	char new_labeled_snt_directory[FILENAME_MAX];
	get_snt_path(new_labeled_text_name, new_labeled_snt_directory);
    if (must_create_directory != 0) {
        make_directory(new_labeled_snt_directory);
    }

    if (must_copy_file != 0)
    {
	    copy_file(new_labeled_text_name, old_labeled_text_name);

	    // create snt directory labeled i
	    char old_labeled_snt_directory[FILENAME_MAX];
	    get_snt_path(old_labeled_text_name, old_labeled_snt_directory);


	    // copy dictionary files in the new snt directory
	    struct snt_files *old_snt_ = new_snt_files(old_labeled_text_name);
	    struct snt_files *new_snt_ = new_snt_files(new_labeled_text_name);

	    if (fexists(old_snt_->dlc)) {
		    copy_file(new_snt_->dlc, old_snt_->dlc);
	    }
	    if (fexists(old_snt_-> dlf)) {
		    copy_file(new_snt_->dlf, old_snt_->dlf);
	    }
	    if (fexists(old_snt_-> err)) {
		    copy_file(new_snt_->err, old_snt_->err);
	    }
	    if (fexists(old_snt_->dlc_n)) {
		    copy_file(new_snt_->dlc_n, old_snt_->dlc_n);
	    }
	    if (fexists(old_snt_->dlf_n)) {
		    copy_file(new_snt_->dlf_n, old_snt_->dlf_n);
	    }
	    if (fexists(old_snt_-> err_n)) {
		    copy_file(new_snt_->err_n, old_snt_->err_n);
	    }
	    if (fexists(old_snt_->stat_dic_n)) {
		    copy_file(new_snt_->stat_dic_n, old_snt_->stat_dic_n);
	    }
	    free_snt_files(old_snt_);
	    free_snt_files(new_snt_);
    }
	char *labeled_text_name;
	labeled_text_name = (char*)malloc(sizeof(char)*(strlen(new_labeled_text_name)+1));
	if(labeled_text_name == NULL){
		fatal_alloc_error("create_labeled_files_and_directory");
		exit(1);
	}
	strcpy(labeled_text_name, new_labeled_text_name);
	return labeled_text_name;
}




void protect_text(const char *fileName, const VersatileEncodingConfig* vec){

	U_FILE *file_reader = u_fopen(vec, fileName, U_READ);
	if(file_reader == NULL){
		fatal_error("u_fopen");
	}

	unichar *text = read_file(file_reader);

	unichar *protected_text = protect_lexical_tag(text, false);

	free(text);
	u_fclose(file_reader);

	U_FILE *file_write = u_fopen(vec, fileName, U_WRITE);
	if(file_write == NULL){
		fatal_error("u_fopen");
	}

	int written = u_fwrite(protected_text, u_strlen(protected_text),file_write);
	if(written != (int)u_strlen(protected_text)){
		fatal_error("u_fwrite");
	}

	u_fclose(file_write);
	free(protected_text);

}



unichar* read_file(U_FILE *f){

	unichar *text = NULL;

	text = (unichar *)malloc(sizeof(unichar));
	if(text==NULL){
		fatal_alloc_error("malloc");
	}
	text[0]='\0';

	int total_read = 0;
	int read;
	do {
		unichar buffer[4096+1];
		memset(buffer,0,sizeof(unichar)*(4096+1));

		int ok=1;

		read = u_fread(buffer,4096,f,&ok);

		total_read += u_strlen(buffer);
		text = (unichar *)realloc(text,sizeof(unichar)*(total_read+1));
		if(text==NULL){
				fatal_alloc_error("realloc");
		}
		u_strcat(text,buffer);

	} while (read == 4096);

	text[total_read]='\0';

	return text;

}




}
