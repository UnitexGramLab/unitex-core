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
 * Cassys_io.h
 *
 *  Created on: 8 oct. 2012
 *      Author: David Nott, Nathalie Friburger (nathalie.friburger@univ-tours.fr)
 */

#ifndef CASSYS_IO_H_
#define CASSYS_IO_H_



#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

#define CASSYS_DIRECTORY_EXTENSION "_csc"


/**
 * \brief Creates the directory \path
 *
 * @param path the directory to be created
 *
 * @todo Windows version
 */
int make_directory(const char *path);



/**
 * \brief Creates the cassys working directory and copies the target \b text and the associated \b snt directory
 * into the working directory with number label
 *
 * The working directory is the canonical name of the the target \b text file with suffix \b _csc. A numbered suffix
 * is also added to the copied \b text and to the associated \snt directory.
 *
 * If the \b text is \b foo.snt and the associated \b snt directory is \b foo_snt/, the function will create the
 * following files and directories :
 * - directory \b foo_csc/
 * - file \b foo_csc/foo_0.snt (copy of \b foo.snt)
 * - directory \b foo_csc/foo_0.snt/ (copy of \b foo_snt/ ant its content)
 *
 * @param[in] text the target text file
 */
int initialize_working_directory(const char *text,int must_create_directory);




/**
 * \brief
 *
 * \param[in] text
 * \param[in] next_transducer_label
 */
char* create_labeled_files_and_directory(
		const char *text,
		int previous_transducer_label,
		int next_transducer_label,
		int previous_iteration,
		int next_iteration,
		int must_create_directory,
		int must_copy_file) ;


int copy_directory_snt_item(const char*dest_snt_dir,const char*src_snd_dir,const char*filename,int mandatory);

/**
 * \brief Copies the content of a snt directory \b src in the directory \b dest
 *
 * @param src the source directory
 * @param dest the destination directory
 *
 */
int copy_directory_snt_content(const char *dest, const char *src);


/**
 * copy the content of the file with stream f and return this content.
 */
unichar* read_file(U_FILE *f);
void protect_text(const char *fileName, const VersatileEncodingConfig* vec);
}

#endif /* CASSYS_IO_H_ */
