#!/bin/sh
# =============================================================================
# Unitex/GramLab Update Copyright Year
# =============================================================================
# Copyright (C) 2016 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
# 
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public
# License along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
#
# cristian.martinez@univ-paris-est.fr (martinec)
# 
# =============================================================================
# This shell script must work in any POSIX-like system, including systems without 
# bash. See this helpful document on writing portable shell scripts:
# @see http://www.gnu.org/s/hello/manual/autoconf/Portable-Shell.html
# =============================================================================
# Script code must be ShellCheck-compliant for information about how to run 
# ShellCheck locally @see http://www.shellcheck.net/about.html
# e.g shellcheck -s sh <script name.sh>
# =============================================================================
# Constants
# =============================================================================
THIS_SCRIPT_NAME=$(basename -- "$0")             # This script name
# =============================================================================
# Script working directory
# =============================================================================
# Working directory snippet from @source http://stackoverflow.com/a/17744637/2042871
THIS_SCRIPT_FILE=$(cd -P -- "$(dirname -- "$0")" && pwd -P) &&\
THIS_SCRIPT_FILE="$THIS_SCRIPT_FILE/$THIS_SCRIPT_NAME"

# Resolve symlinks snippet from @source http://stackoverflow.com/a/697552/2042871
while [ -h "$THIS_SCRIPT_FILE" ]; do
    THIS_SCRIPT_DIR=$(dirname -- "$THIS_SCRIPT_FILE")
    THIS_SCRIPT_SYM=$(readlink   "$THIS_SCRIPT_FILE")
    THIS_SCRIPT_FILE="$(cd "$THIS_SCRIPT_DIR"                   &&\
                        cd "$(dirname   -- "$THIS_SCRIPT_SYM")" &&\
                        pwd)/$(basename -- "$THIS_SCRIPT_SYM")"
done  # [ -h "$THIS_SCRIPT_FILE" ]

# Set-up working directory 
THIS_SCRIPT_BASEDIR="$(dirname  -- "$THIS_SCRIPT_FILE")"

# =============================================================================
# Main
# =============================================================================
CURRENT_YEAR=$(date +'%Y')
COPYRIGHT_SYMBOL="Copyright (C)"
COPYRIGHT_HOLDER="Université Paris-Est Marne-la-Vallée"
# =============================================================================
COPYRIGHT_CURRENT_YEAR_REGEX="$COPYRIGHT_SYMBOL \(.*\)$CURRENT_YEAR"
COPYRIGHT_ANY_YEAR_REGEX="$COPYRIGHT_SYMBOL \(.*\)[0-9]\{4\}"
COPYRIGHT_FIND_REGEX="$COPYRIGHT_ANY_YEAR_REGEX $COPYRIGHT_HOLDER"
COPYRIGHT_REPLACE_REGEX="$COPYRIGHT_SYMBOL \1$CURRENT_YEAR $COPYRIGHT_HOLDER"
# =============================================================================
cd "$THIS_SCRIPT_BASEDIR/../../"
find . -type f -not -path "*/\.*" -exec grep -Iq . {} \; -and -print0 |\
     xargs -0 grep -Zli "$COPYRIGHT_HOLDER"                           |\
     xargs -0 grep -ZLi "$COPYRIGHT_CURRENT_YEAR_REGEX"               |\
     xargs -0 sed -i "s|$COPYRIGHT_FIND_REGEX|$COPYRIGHT_REPLACE_REGEX|gi"
