/*	$OpenBSD: getopt_long.c,v 1.16 2004/02/04 18:17:25 millert Exp $	*/
/*	$NetBSD: getopt_long.c,v 1.15 2002/01/31 22:43:40 tv Exp $	*/

/*
 * Copyright (c) 2002 Todd C. Miller <Todd.Miller@courtesan.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Sponsored in part by the Defense Advanced Research Projects
 * Agency (DARPA) and Air Force Research Laboratory, Air Force
 * Materiel Command, USAF, under agreement number F39502-99-1-0512.
 */
/*-
 * Copyright (c) 2000 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Dieter Baron and Thomas Klausner.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *        This product includes software developed by the NetBSD
 *        Foundation, Inc. and its contributors.
 * 4. Neither the name of The NetBSD Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char *rcsid = "$OpenBSD: getopt_long.c,v 1.16 2004/02/04 18:17:25 millert Exp $";
#endif /* LIBC_SCCS and not lint */

#include <errno.h>
#include "getopt.h"
#include <stdlib.h>
#include <string.h>
#include "Unicode.h"
#include "Error.h"


#define PRINT_ERROR	((vars->opterr) && (*options != ':'))

#define FLAG_PERMUTE	0x01	/* permute non-options to the end of argv */
#define FLAG_ALLARGS	0x02	/* treat non-options as args to option "-1" */
#define FLAG_LONGONLY	0x04	/* operate as getopt_long_only */

/* return values */
#define	BADCH		(int)'?'
#define	BADARG		((*options == ':') ? (int)':' : (int)'?')
#define	INORDER 	(int)1

static char EMSG[] = "";

static int getopt_internal_TS(int, char * const *, const char *,
			   const struct option_TS *, int *, int,struct OptVars*);
static int parse_long_options_TS(char * const *, const char *,
			      const struct option_TS *, int *, int,struct OptVars*);
static int gcd(int, int);
static void permute_args(int, int, int, char * const *);

static char* place = EMSG; /* option letter processing */

/* XXX: set optreset to 1 rather than these two */

/* Error messages */
static const char recargchar[] = "option requires an argument -- %c";
static const char recargstring[] = "option requires an argument -- %s";
static const char ambig[] = "ambiguous option -- %.*s";
static const char noarg[] = "option doesn't take an argument -- %.*s";
static const char illoptchar[] = "unknown option -- %c";
static const char illoptstring[] = "unknown option -- %s";

/*
 * Compute the greatest common divisor of a and b.
 */
static int
gcd(int a, int b)
{
	int c;

	c = a % b;
	while (c != 0) {
		a = b;
		b = c;
		c = a % b;
	}

	return (b);
}

/*
 * Exchange the block from nonopt_start to nonopt_end with the block
 * from nonopt_end to opt_end (keeping the same order of arguments
 * in each block).
 */
static void
permute_args(int panonopt_start, int panonopt_end, int opt_end,
	char * const *nargv)
{
	int cstart, cyclelen, i, j, ncycle, nnonopts, nopts, pos;
	char *swap;

	/*
	 * compute lengths of blocks and number and size of cycles
	 */
	nnonopts = panonopt_end - panonopt_start;
	nopts = opt_end - panonopt_end;
	ncycle = gcd(nnonopts, nopts);
	cyclelen = (opt_end - panonopt_start) / ncycle;

	for (i = 0; i < ncycle; i++) {
		cstart = panonopt_end+i;
		pos = cstart;
		for (j = 0; j < cyclelen; j++) {
			if (pos >= panonopt_end)
				pos -= nnonopts;
			else
				pos += nopts;
			swap = nargv[pos];
			/* LINTED const cast */
			((char **) nargv)[pos] = nargv[cstart];
			/* LINTED const cast */
			((char **)nargv)[cstart] = swap;
		}
	}
}

/*
 * parse_long_options --
 *	Parse long options in argc/argv argument vector.
 * Returns -1 if short_too is set and the option does not match long_options.
 */
static int
parse_long_options_TS(char * const *nargv, const char *options,
	const struct option_TS *long_options, int *idx, int short_too,struct OptVars* vars)
{
	char *current_argv, *has_equal;
	size_t current_argv_len;
	int i, match;

	current_argv = place;
	match = -1;

	(vars->optind)++;

	if ((has_equal = strchr(current_argv, '=')) != NULL) {
		/* argument found (--option=arg) */
		current_argv_len = has_equal - current_argv;
		has_equal++;
	} else
		current_argv_len = strlen(current_argv);

	for (i = 0; long_options[i].name; i++) {
		/* find matching long option */
		if (strncmp(current_argv, long_options[i].name,
		    current_argv_len))
			continue;

		if (strlen(long_options[i].name) == current_argv_len) {
			/* exact match */
			match = i;
			break;
		}
		/*
		 * If this is a known short option, don't allow
		 * a partial match of a single character.
		 */
		if (short_too && current_argv_len == 1)
			continue;

		if (match == -1)	/* partial match */
			match = i;
		else {
			/* ambiguous abbreviation */
			if (PRINT_ERROR) {
			//	warnx(ambig, (int)current_argv_len, current_argv);
         }
			vars->optopt = 0;
			return (BADCH);
		}
	}
	if (match != -1) {		/* option found */
	   *idx = match;
		if (long_options[match].has_arg == no_argument_TS
		    && has_equal) {
			if (PRINT_ERROR) {
				//warnx(noarg, (int)current_argv_len, current_argv);
                 }			/*
			 * XXX: GNU sets optopt to val regardless of flag
			 */
			if (long_options[match].flag == NULL)
				vars->optopt = long_options[match].val;
			else
				vars->optopt = 0;
			return (BADARG);
		}
		if (long_options[match].has_arg == required_argument_TS ||
		    long_options[match].has_arg == optional_argument_TS) {
			if (has_equal)
				vars->optarg = has_equal;
			else if (long_options[match].has_arg ==
			    required_argument_TS) {
				/*
				 * optional argument doesn't use next nargv
				 */
			   vars->optarg = nargv[(vars->optind)++];
			}
		}
		if ((long_options[match].has_arg == required_argument_TS)
		    && (vars->optarg == NULL)) {
			/*
			 * Missing argument; leading ':' indicates no error
			 * should be generated.
			 */
			if (PRINT_ERROR) {
			//	warnx(recargstring,  current_argv);
         }
			/*
			 * XXX: GNU sets optopt to val regardless of flag
			 */
			if (long_options[match].flag == NULL)
			   vars->optopt = long_options[match].val;
			else
			   vars->optopt = 0;
			--(vars->optind);
			return (BADARG);
		}
	} else {			/* unknown option */
		if (short_too) {
			--(vars->optind);
			return (-1);
		}
		if (PRINT_ERROR) {
		//	warnx(illoptstring, current_argv);
      }
		vars->optopt = 0;
		/* S.P. */
		vars->optarg = current_argv;
		*idx=0;
		/* S.P. end */
		return (BADCH);
	}
	if (long_options[match].flag) {
		*long_options[match].flag = long_options[match].val;
		return (0);
	} else
		return (long_options[match].val);
}

/*
 * getopt_internal --
 *	Parse argc/argv argument vector.  Called by user level routines.
 */
static int
getopt_internal_TS(int nargc, char * const *nargv, const char *options,
	const struct option_TS *long_options, int *idx, int flags,struct OptVars* vars)
{
	char *oli;				/* option letter list index */
	int optchar, short_too;
	static int posixly_correct = -1;

	if (options == NULL)
		return (-1);

	/*
	 * Disable GNU extensions if POSIXLY_CORRECT is set or options
	 * string begins with a '+'.
	 */
	if (posixly_correct == -1)
		posixly_correct = (getenv("POSIXLY_CORRECT") != NULL);
	if (posixly_correct || *options == '+')
		flags &= ~FLAG_PERMUTE;
	else if (*options == '-')
		flags |= FLAG_ALLARGS;
	if (*options == '+' || *options == '-')
		options++;

	/*
	 * XXX Some GNU programs (like cvs) set optind to 0 instead of
	 * XXX using optreset.  Work around this braindamage.
	 */
	if (vars->optind == 0)
	   vars->optind = vars->optreset = 1;

	vars->optarg = NULL;
	if (vars->optreset)
	   vars->nonopt_start = vars->nonopt_end = -1;
start:
	if (vars->optreset || !*place) {		/* update scanning pointer */
	   vars->optreset = 0;
		if (vars->optind >= nargc) {          /* end of argument vector */
			place = EMSG;
			if (vars->nonopt_end != -1) {
				/* do permutation, if we have to */
				permute_args(vars->nonopt_start, vars->nonopt_end,
				      vars->optind, nargv);
				vars->optind -= vars->nonopt_end - vars->nonopt_start;
			}
			else if (vars->nonopt_start != -1) {
				/*
				 * If we skipped non-options, set optind
				 * to the first of them.
				 */
			   vars->optind = vars->nonopt_start;
			}
			vars->nonopt_start = vars->nonopt_end = -1;
			return (-1);
		}
		if (*(place = nargv[vars->optind]) != '-' ||
		    (place[1] == '\0' && strchr(options, '-') == NULL)) {
			place = EMSG;		/* found non-option */
			if (flags & FLAG_ALLARGS) {
				/*
				 * GNU extension:
				 * return non-option as argument to option 1
				 */
			   vars->optarg = nargv[(vars->optind)++];
				return (INORDER);
			}
			if (!(flags & FLAG_PERMUTE)) {
				/*
				 * If no permutation wanted, stop parsing
				 * at first non-option.
				 */
				return (-1);
			}
			/* do permutation */
			if (vars->nonopt_start == -1)
			   vars->nonopt_start = vars->optind;
			else if (vars->nonopt_end != -1) {
				permute_args(vars->nonopt_start, vars->nonopt_end,
				      vars->optind, nargv);
				vars->nonopt_start = vars->optind -
				    (vars->nonopt_end - vars->nonopt_start);
				vars->nonopt_end = -1;
			}
			(vars->optind)++;
			/* process next argument */
			goto start;
		}
		if (vars->nonopt_start != -1 && vars->nonopt_end == -1)
		   vars->nonopt_end = vars->optind;

		/*
		 * If we have "-" do nothing, if "--" we are done.
		 */
		if (place[1] != '\0' && *++place == '-' && place[1] == '\0') {
			(vars->optind)++;
			place = EMSG;
			/*
			 * We found an option (--), so if we skipped
			 * non-options, we have to permute.
			 */
			if (vars->nonopt_end != -1) {
				permute_args(vars->nonopt_start, vars->nonopt_end,
				      vars->optind, nargv);
				vars->optind -= vars->nonopt_end - vars->nonopt_start;
			}
			vars->nonopt_start = vars->nonopt_end = -1;
			return (-1);
		}
	}

	/*
	 * Check long options if:
	 *  1) we were passed some
	 *  2) the arg is not just "-"
	 *  3) either the arg starts with -- we are getopt_long_only()
	 */
	if (long_options != NULL && place != nargv[vars->optind] &&
	    (*place == '-' || (flags & FLAG_LONGONLY))) {
		short_too = 0;
		if (*place == '-')
			place++;		/* --foo long option */
		else if (*place != ':' && strchr(options, *place) != NULL)
			short_too = 1;		/* could be short option too */

		optchar = parse_long_options_TS(nargv, options, long_options,
		    idx, short_too,vars);
		if (optchar != -1) {
			place = EMSG;
			return (optchar);
		}
	}

	if ((optchar = (int)*place++) == (int)':' ||
	    (optchar == (int)'-' && *place != '\0') ||
	    (oli = strchr(options, optchar)) == NULL) {
		/*
		 * If the user specified "-" and  '-' isn't listed in
		 * options, return -1 (non-option) as per POSIX.
		 * Otherwise, it is an unknown option character (or ':').
		 */
		if (optchar == (int)'-' && *place == '\0')
			return (-1);
		if (!*place)
			++(vars->optind);
		if (PRINT_ERROR) {
		//	warnx(illoptchar, optchar);
      }
		vars->optopt = optchar;
		return (BADCH);
	}
	if (long_options != NULL && optchar == 'W' && oli[1] == ';') {
		/* -W long-option */
		if (*place)			/* no space */
			/* NOTHING */;
		else if (++(vars->optind) >= nargc) {	/* no arg */
			place = EMSG;
			if (PRINT_ERROR) {
			//	warnx(recargchar, optchar);
         }
			vars->optopt = optchar;
			return (BADARG);
		} else				/* white space */
			place = nargv[vars->optind];
		optchar = parse_long_options_TS(nargv, options, long_options, idx, 0,vars);
		place = EMSG;
		return (optchar);
	}
	if (*++oli != ':') {			/* doesn't take argument */
		if (!*place)
			++(vars->optind);
	} else {				/* takes (optional) argument */
	   vars->optarg = NULL;
		if (*place)			/* no white space */
		   vars->optarg = place;
		/* XXX: disable test for :: if PC? (GNU doesn't) */
		else if (oli[1] != ':') {	/* arg not optional */
			if (++(vars->optind) >= nargc) {	/* no arg */
				place = EMSG;
				if (PRINT_ERROR) {
				//	warnx(recargcha, optchar);
            }
				vars->optopt = optchar;
				return (BADARG);
			} else
			   vars->optarg = nargv[vars->optind];
		} else if (!(flags & FLAG_PERMUTE)) {
			/*
			 * If permutation is disabled, we can accept an
			 * optional arg separated by whitespace.
			 */
			if (vars->optind + 1 < nargc)
			   vars->optarg = nargv[++(vars->optind)];
		}
		place = EMSG;
		++(vars->optind);
	}
	/* dump back option letter */
	return (optchar);
}

/*
 * getopt in thread safe version
 *	Parse argc/argv argument vector.
 */
int
getopt_TS(int nargc, char * const *nargv, const char *options,struct OptVars* vars)
{

	/*
	 * We dont' pass FLAG_PERMUTE to getopt_internal() since
	 * the BSD getopt(3) (unlike GNU) has never done this.
	 *
	 * Furthermore, since many privileged programs call getopt()
	 * before dropping privileges it makes sense to keep things
	 * as simple (and bug-free) as possible.
	 */
	return (getopt_internal_TS(nargc, nargv, options, NULL, NULL, /*0*/FLAG_PERMUTE,vars));
}

/*
 * getopt_long in thread safe version
 *	Parse argc/argv argument vector.
 */
int
getopt_long_TS(int nargc, char * const *nargv, const char *options,
            const struct option_TS *long_options,int* idx,struct OptVars* vars)
{

	return getopt_internal_TS(nargc, nargv, options, long_options, idx,FLAG_PERMUTE,vars);
}

/*
 * getopt_long_only in thread safe version
 *	Parse argc/argv argument vector.
 */
int
getopt_long_only_TS(int nargc, char * const *nargv, const char *options,
                 const struct option_TS * long_options,int* idx,struct OptVars* vars)
{

	return getopt_internal_TS(nargc, nargv, options, long_options, idx,
	                          FLAG_PERMUTE|FLAG_LONGONLY,vars);
}


/**
 * Allocates, initializes and returns a new struct OptVars*
 */
struct OptVars* new_OptVars() {
struct OptVars* o=(struct OptVars*)malloc(sizeof(struct OptVars));
if (o==NULL) {
   fatal_alloc_error("new_OptVars");
}
o->opterr=1;
o->optind=1;
o->optopt='?';
o->nonopt_start=-1;
o->nonopt_end=-1;
o->optreset=0;
return o;
}


/**
 * Frees all the memory associated to the given struct OptVars*
 */
void free_OptVars(struct OptVars* o) {
if (o==NULL) return;
free(o);
}

