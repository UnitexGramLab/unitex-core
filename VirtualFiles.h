/*
 * Unitex
 *
 * Copyright (C) 2001-2014 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef VirtualFiles_H
#define VirtualFiles_H

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {
namespace virtualfile {

/**
 * The goal of this library is to offer a virtual file system.
 * To make a file virtual, you just have to prefix its name with $:
 * Then, if the file is not already present in memory, on the first
 * open in read mode, its content will be loaded. Then, the disk
 * will not be touched again, until VFS_dump is called to save the file
 * or VFS_reload is called to load again the file from the disk.
 */

#define VIRTUAL_FILE_PFX "$:"

/* Starts the virtual file system */
void init_virtual_files();

/* Returns a list of current virtual file names as
 * a NULL terminated char* array */
char** VFS_ls();

/* Returns the length in bytes of the given virtual file,
 * or -1 if not found */
long VFS_size(const char* name);

/* Returns a pointer on the virtual file content
 * THIS MUST NOT BE FREED! */
void* VFS_content(const char* name);

/* Reloads a file from the disk */
int VFS_reload(const char* name);

/* Dumps a virtual file to the disk */
int VFS_dump(const char* name);

/* Deletes a virtual file */
void VFS_remove(const char* name);

/* Deletes all virtual files */
void VFS_reset();

} // namespace virtualfile
} // namespace unitex

#endif
