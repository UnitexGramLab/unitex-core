#/*
# * Unitex
# *
# * Copyright (C) 2001-2014 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
# *
# * This library is free software; you can redistribute it and/or
# * modify it under the terms of the GNU Lesser General Public
# * License as published by the Free Software Foundation; either
# * version 2.1 of the License, or (at your option) any later version.
# *
# * This library is distributed in the hope that it will be useful,
# * but WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# * Lesser General Public License for more details.
# *
# * You should have received a copy of the GNU Lesser General Public
# * License along with this library; if not, write to the Free Software
# * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
# *
# */

#/*
# * File created and contributed by Gilles Vollant (Ergonotics SAS) 
# * as part of an UNITEX optimization and reliability effort
# *
# * additional information: http://www.smartversion.com/unitex-contribution/
# * https://github.com/ergonotics/JNI-for-Unitex-2.1
# * contact : info@winimage.com
# *
# */

def virtualFilePrefix
    
    if unitexAbstractPathExists('*')
        return '*'
    end
    
    if unitexAbstractPathExists('$:')
        return '$:'
    end
    
  return ''
end

def virtualResourcePrefix
    return virtualFilePrefix + 'resources'
end

def load_dictionary(res_dir, dic_name)
    copyUnitexFile(res_dir + '/' + dic_name + '.bin', virtualResourcePrefix + '/' + dic_name + '.bin')
    copyUnitexFile(res_dir + '/' +dic_name + '.inf', virtualResourcePrefix + '/' + dic_name + '.inf')
    return loadPersistentDictionary(virtualResourcePrefix + '/' + dic_name + '.bin')
end


def load_graph(res_dir, graph_name)
    copyUnitexFile(res_dir + '/' + graph_name + '.fst2', virtualResourcePrefix + '/' + graph_name + '.fst2')
    return loadPersistentFst2(virtualResourcePrefix + '/' + graph_name + '.fst2')
end


def load_alphabet(res_dir, alphabet_name)
    copyUnitexFile(res_dir + '/' + alphabet_name + '.txt', virtualResourcePrefix + '/' + alphabet_name + '.txt')
    return loadPersistentAlphabet(virtualResourcePrefix + '/' + alphabet_name + '.txt')
end

def load_norm(res_dir, norm_name)
    virtualNorm = virtualResourcePrefix + '/' + norm_name
    copyUnitexFile(res_dir + '/' + norm_name, virtualNorm)
    return virtualNorm
end



def process_sentence(sentence, dic_array, graph_array, alphabet, norm)


  corpusPrefix = virtualFilePrefix + 'work_' + getUniqueString + '/'
  # alternative
  # corpusPrefix = virtualFilePrefix + 'work_' + "#{Process.pid}_#{Thread.current.object_id}" + '/'
  puts(corpusPrefix+ ' ----')
  createUnitexFolder(corpusPrefix + 'corpus_snt');
  
  writeUnitexFileString(corpusPrefix + 'corpus.txt', sentence)
  UnitexToolArray(['UnitexTool','Normalize',corpusPrefix + 'corpus.txt','-r',norm])
  
  UnitexToolArray(['UnitexTool','Tokenize',corpusPrefix + 'corpus.snt','-a',alphabet])
  
  UnitexToolArray(['UnitexTool','Dico', '-t', corpusPrefix + 'corpus.snt','-a',alphabet] + dic_array)
  
  
  graph_array.each  {  |graph_item|
      UnitexToolArray(['UnitexTool','Locate','-t',corpusPrefix + 'corpus.snt', '-a', alphabet,
                                                    '-L','-R','--all','-b','-Y', graph_item])
  }
  
  locate_result = getUnitexFileStringEncoded(corpusPrefix + 'corpus_snt/concord.ind')

  removeUnitexFolder(corpusPrefix)

  # removeUnitexFolder is sometime no recursive
  todelete = getFileList(corpusPrefix)
  if todelete.count > 0
    todelete.each { |file| removeUnitexFile(file) }
    removeUnitexFolder(corpusPrefix)
  end
  
  return locate_result
end


def init_resources_and_work(res_dir)
    dic1 = load_dictionary(res_dir, 'dictionnary/dela-en-public')
    dic2 = load_dictionary(res_dir, 'dictionnary/UNITEX-convert-en')
    dic_array = [dic1, dic2]
    graph = load_graph(res_dir, 'graph/AAA-hoursgilles')
    graph_array = [ graph ]
    alphabet = load_alphabet(res_dir, 'others/Alphabet')
    norm = load_norm(res_dir, 'norm/Norm.txt')
    
    
    
    setStdOutTrashMode(true)
    setStdErrTrashMode(true)

	# init is done, now do this 
    sentence = 'I want watch at 4:00 am see at 6:00 pm before leave at 15.47'

    result_process = process_sentence(sentence, dic_array, graph_array, alphabet, norm)
    
    puts('result is:')
    puts(result_process)

end


#
# now somes tests


$LOAD_PATH << '/Users/GillesVollant/mytest2'


require 'unitexruby'
include UnitexRuby

puts getUnitexInfoString


puts numberAbstractFileSpaceInstalled
puts virtualFilePrefix


puts getUniqueString
puts getUniqueString


# replace with location of resources from https://github.com/ergonotics/JNI-for-Unitex-2.1 (in samples/demojnires)

init_resources_and_work('/Users/GillesVollant/resources_uni')
