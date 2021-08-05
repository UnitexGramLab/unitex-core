/*
 * Unitex
 *
 * Copyright (C) 2001-2021 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
/**
##header
 */
/* ************************************************************************** */
// Header for this file
#include "base/unicode/normalization.h"
/* ************************************************************************** */
namespace unitex {
/* ************************************************************************** */
const {n:NormInfoT} {k:NormInfo}[] = {
##normInfos
};

/**
 * All Unicode pages
 * Points to {n:InfoIndex}
 */
const {v:pagesType} {k:NormPageIndex}[] = {
##normPageIndex
};

/**
 * Indexes of {k:NormInfo}
 * {n:InfoIndex}[page index][index in page]
 */
const {v:infoType} {k:NormInfoIndex}[][256] = {
##normInfoIndex
};

/**
 * Decomposition mappings constituted by 4 or more characters
 * ([length][charcode]{length})*
 */
const {v:charName} {k:SpecialMappings}[] = {
##specialMappings
};
/* ************************************************************************** */
}  // namespace unitex
