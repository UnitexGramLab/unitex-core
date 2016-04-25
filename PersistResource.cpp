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

/*
 * File created and contributed by Gilles Vollant (Ergonotics SAS)
 * as part of an UNITEX optimization and reliability effort
 *
 * additional information: http://www.ergonotics.com/unitex-contribution/
 * contact : unitex-contribution@ergonotics.com
 *
 */

#include "Copyright.h"
#include "Error.h"
#include "File.h"
#include "PersistResource.h"
#include "UnitexGetOpt.h"
#include "PersistenceInterface.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif


namespace unitex {

const char* usage_PersistResource =
         "Usage: PersistResource [OPTIONS] <resfile>\n"
         "\n"
         "  <outfile>: the resource file\n"
         "\n"
         "OPTIONS:\n"
         "  -g/--graph: to persist a compiled graph resource\n"
         "  -d/--dictionary: to persist a compiled dictionary resource\n"
         "  -a/--alphabet: to persist an alphabet resource\n"
         "  -u/--unpersist: to unpersist the resource\n"
         "  -o OUTFILE/--output=OUTFILE: path to output file to create\n"
         "                 with persisted filename. Incompatible with -u/--unpersist\n"
         "  -v/--verbose: emit message when running\n"
         "  -V/--only-verify-arguments: only verify arguments syntax and exit\n"
         "  -h/--help: this help\n"
         "\n";


static void usage() {
  display_copyright_notice();
  u_printf(usage_PersistResource);
}

const char* optstring_PersistResource=":Vhvgdauo:k:q:";
const struct option_TS lopts_PersistResource[]= {
  {"graph", no_argument_TS,NULL,'g'},
  {"dictionary", no_argument_TS,NULL,'d'},
  {"alphabet", no_argument_TS,NULL,'a'},
  {"unpersist", no_argument_TS,NULL,'u' },
  {"output",required_argument_TS,NULL,'o' },
  {"input_encoding",required_argument_TS,NULL,'k'},
  {"output_encoding",required_argument_TS,NULL,'q'},
  {"only_verify_arguments",no_argument_TS,NULL,'V'},
  {"verbose",no_argument_TS, NULL,'v' },
  {"help", no_argument_TS, NULL, 'h'},
  {NULL,no_argument_TS,NULL,0}
};




int main_PersistResource(int argc,char* const argv[]) {
if (argc==1) {
   usage();
   return SUCCESS_RETURN_CODE;
}

const char *resource_file = NULL;
int res_graph=0;
int res_alphabet=0;
int res_dico=0;
int unpersist=0;
int verbose=0;
VersatileEncodingConfig vec = VEC_DEFAULT;
int val,index=-1;
bool only_verify_arguments = false;
const char*output_file = NULL;
const char*resource_type = NULL;
UnitexGetOpt options;

while (EOF!=(val=options.parse_long(argc,argv,optstring_PersistResource,lopts_PersistResource,&index))) {
   switch(val) {
   case 'a': res_alphabet = 1; resource_type = "alphabet"; break;
   case 'g': res_graph = 1; resource_type = "graph"; break;
   case 'd': res_dico = 1; resource_type = "dictionary"; break;
   case 'u': unpersist = 1; break;
   case 'v': verbose = 1; break;
   case 'o': if (options.vars()->optarg[0]=='\0') {
                error("Empty output argument\n");
                return USAGE_ERROR_CODE;
             }
             output_file = options.vars()->optarg; // FIXME(gvollant)
             break;
   case 'V': only_verify_arguments = true;
             break;
   case 'h': usage();
             return SUCCESS_RETURN_CODE;
   case ':': index==-1 ? error("Missing argument for option -%c\n",options.vars()->optopt):
                         error("Missing argument for option --%s\n",lopts_PersistResource[index].name);
             return USAGE_ERROR_CODE;
   case '?': index==-1 ? error("Invalid option -%c\n",options.vars()->optopt) :
                         error("Invalid option --%s\n",options.vars()->optarg);
             return USAGE_ERROR_CODE;
   case 'k':
   case 'q': /* ignore -k and -q parameter instead to raise an error */
             break;
   }
   index=-1;
}

if ((res_graph+res_alphabet+res_dico) != 1) {
    error("Invalid arguments: rerun with --help\n");
    return USAGE_ERROR_CODE;
}

if ((output_file!=NULL) && (unpersist!=0)) {
    error("Invalid arguments: rerun with --help\n");
    return USAGE_ERROR_CODE;
}

if (options.vars()->optind!=argc-1) {
    error("Invalid arguments: rerun with --help\n");
    return USAGE_ERROR_CODE;
}

if (only_verify_arguments) {
    // freeing all allocated memory
    return SUCCESS_RETURN_CODE;
}

resource_file = argv[options.vars()->optind];
size_t size_buf_persisted_filename = strlen(resource_file) + 0x200;
char* buf_persisted_filename = (char*)malloc(size_buf_persisted_filename +1);
if (buf_persisted_filename == NULL) {
    alloc_error("PersistResource's main");
    return ALLOC_ERROR_CODE;
}
*buf_persisted_filename='\0';
if (unpersist == 0) {
    int result = 0;
    if (res_alphabet)
        result = standard_load_persistence_alphabet(resource_file, buf_persisted_filename, size_buf_persisted_filename);
    if (res_graph)
        result = standard_load_persistence_fst2(resource_file, buf_persisted_filename, size_buf_persisted_filename);
    if (res_dico)
        result = standard_load_persistence_dictionary(resource_file, buf_persisted_filename, size_buf_persisted_filename);
    if (result && verbose)
        u_printf("Success on persist %s resource %s to persisted name %s\n", resource_type, resource_file, buf_persisted_filename);
    if (!result)
        error("The %s resource %s cannnot be persisted\n", resource_type, resource_file);


    if (result && (output_file != NULL)) {
        U_FILE* text = u_fopen(&vec, output_file, U_WRITE);
        if (text == NULL) {
            error("Cannot create text file %s\n", output_file);
            free(buf_persisted_filename);
            return DEFAULT_ERROR_CODE;
        }
        u_fprintf(text, "%s", buf_persisted_filename);
        u_fclose(text);
    }

    free(buf_persisted_filename);
    return result ? SUCCESS_RETURN_CODE : DEFAULT_ERROR_CODE;
}
else
{
    if (res_alphabet)
        standard_unload_persistence_alphabet(resource_file);
    if (res_graph)
        standard_unload_persistence_fst2(resource_file);
    if (res_dico)
        standard_unload_persistence_dictionary(resource_file);
    u_printf("The %s resource %s unpersisted\n", resource_type, resource_file);
    return SUCCESS_RETURN_CODE;
}

}

} // namespace unitex
