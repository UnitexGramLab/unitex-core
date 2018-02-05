/*
 * Unitex
 *
 * Copyright (C) 2001-2018 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
 
import java.io.File;
import java.util.ArrayList;
import java.util.List;

import fr.umlv.unitex.jni.UnitexJni;

/*
 * This program is a sample of complete use of Unitex JNI with a Java program
 *  
 * You can read somes documentation at http://unitex-library-fr.readthedocs.org/fr/latest/
 *   and http://unitex-library.readthedocs.org/en/latest/
 *
 * you must create a resource demo directory demojnires with
 *  demojnires/graph/AAA-hours-demo.fst2 from this directory
 *  demojnires/others/Alphabet.txt from unitex/ling/English/Alphabet.txt
 *  demojnires/others/Norm.txt from unitex/ling/English/Norm.txt
 *  demojnires/dictionnary/dela-en-public.bin from unitex/ling/English/Dela/dela-en-public.bin
 *  demojnires/dictionnary/dela-en-public.inf from unitex/ling/English/Dela/dela-en-public.inf
 *
 *  Under MS-Windows, a batch startUnitexJniDemo.bat with these line (without the *)
 *    "%JAVA_HOME%\\bin\\javac.exe" fr\\umlv\\unitex\\jni\\UnitexJni.java
 *    "%JAVA_HOME%\\bin\\javac.exe" UnitexJniDemo.java
 *    "%JAVA_HOME%\\bin\\java.exe" -classpath .; UnitexJniDemo %1 %2 %3 %4
 *
 *  Under Unix (Linux, MacOSX...), a script startUnitexJniDemo.sh with these line (without the *)
 *    #!/bin/bash
 *
 *    javac UnitexJniDemo.java
 *    java -Djava.library.path=. UnitexJniDemo $1 $2 $3 $4 
 *
 *  copy the JNI Library (UnitexJni.dll for MS-Windows, libUnitexJni.so for Linux, libUnitexJni.dylib for MacOSX)
 *    on the directory with UnitexJniDemo.java and the batch/script
 *
 *  then run the script/batch, with parameters [ressource_dir] [base_work_dir] [nb_loop] [param]
 *    where [ressource_dir] : the location of resource folder demojnires (default is "demojnires")
 *          [base_work_dir] : location of working directory (default is ".")
 *          [nb_loop] : number of iteration of running the job
 *          [param] : configuration parameters
 *              param=0 : no vfs (virtual file system) and no persistance of resource (slowest)
 *              param=1 : vfs and no persistance
 *              param=2 : no vfs and persistance
 *              param=3 : vfs and persistance (fastest)");
 *
 */

public class UnitexJniDemo {

      private static final String pathSeparator = UnitexJni.isUnderWindows() ? "\\" : "/";

      public static final String getVirtualFilePfx()
      {
        if (UnitexJni.unitexAbstractPathExists("*"))
            return "*";

        if (UnitexJni.unitexAbstractPathExists("$:"))
            return "$:";

        return null;
      }

      private static String processUnitexWork(String othersResDir,String workingDicoFileName, String workingGraphFileName,String corpusPath,String corpusText)
      {
          String pSep = pathSeparator;
          UnitexJni.writeUnitexFile(UnitexJni.combineUnitexFileComponent(corpusPath,"corpus.txt"),corpusText);
		  
		  // we create offsets file offset1.txt and offset2.txt to get position against the original corpus in the xml file

          String cmdNorm = "Normalize " + UnitexJni.combineUnitexFileComponentWithQuote(corpusPath,"corpus.txt") + " -r "+UnitexJni.combineUnitexFileComponentWithQuote(othersResDir,"Norm.txt") + " --output_offsets="+ UnitexJni.combineUnitexFileComponentWithQuote(corpusPath,"offset1.txt") ;
          String cmdTok = "Tokenize " + UnitexJni.combineUnitexFileComponentWithQuote(corpusPath,"corpus.txt") + " -a "+ UnitexJni.combineUnitexFileComponentWithQuote(othersResDir,"Alphabet.txt") + " --input_offsets="+ UnitexJni.combineUnitexFileComponentWithQuote(corpusPath,"offset1.txt") + " --output_offsets="+ UnitexJni.combineUnitexFileComponentWithQuote(corpusPath,"offset2.txt") ;
          String cmdDico = "Dico -t "+ UnitexJni.combineUnitexFileComponentWithQuote(corpusPath,"corpus.snt")+ " -a " + UnitexJni.combineUnitexFileComponentWithQuote(othersResDir,"Alphabet.txt")+" "+UnitexJni.combineUnitexFileComponentWithQuote(workingDicoFileName) ;
          String cmdLocate = "Locate -t "+UnitexJni.combineUnitexFileComponentWithQuote(corpusPath,"corpus.snt")+ " " + UnitexJni.combineUnitexFileComponentWithQuote(workingGraphFileName)+ " -a " + UnitexJni.combineUnitexFileComponentWithQuote(othersResDir,"Alphabet.txt")+ " -L -R --all -b -Y";
          String cmdConcord = "Concord "+ UnitexJni.combineUnitexFileComponentWithQuote(corpusPath,"corpus_snt","concord.ind")+ " -m " + UnitexJni.combineUnitexFileComponentWithQuote(corpusPath,"corpus.txt") ;

          String cmdConcordXml = "Concord "+ UnitexJni.combineUnitexFileComponentWithQuote(corpusPath,"corpus_snt","concord.ind")+ 
     		  " --uima="+ UnitexJni.combineUnitexFileComponentWithQuote(corpusPath,"offset2.txt") +" --xml";

      
	  
          UnitexJni.execUnitexTool("UnitexTool " + cmdNorm);
          UnitexJni.execUnitexTool("UnitexTool " + cmdTok);
          UnitexJni.execUnitexTool("UnitexTool " + cmdDico);
          UnitexJni.execUnitexTool("UnitexTool " + cmdLocate);
          UnitexJni.execUnitexTool("UnitexTool " + cmdConcord);
          UnitexJni.execUnitexTool("UnitexTool " + cmdConcordXml);

          // these 6 lines can be replaced by only one execution (with very small speed improvement)
          /*
          UnitexJni.execUnitexTool("UnitexTool { " + cmdNorm + " } { " + cmdTok + " } { " + cmdDico + " } { "  + cmdLocate + " } { " + cmdConcord + " } { " + cmdConcord2+ " }");

          */

          String merged =  UnitexJni.getUnitexFileString(UnitexJni.combineUnitexFileComponent(corpusPath,"corpus.txt"));
          String xml = UnitexJni.getUnitexFileString(UnitexJni.combineUnitexFileComponent(corpusPath,"corpus_snt","concord.xml"));
          return xml;
      }

    public static void main(String [] args) {
        //System.out.println(UnitexJni.combineUnitexFileComponent("A","b","c"));
        //System.out.println(UnitexJni.combineUnitexFileComponentWithQuote("b","c"));
		
        System.out.println("is ms-windows:"+UnitexJni.isUnderWindows()+" : "+System.getProperty("os.name")+ " "+java.io.File.separator);
        System.out.println("Usage : UnitexJniDemo [ressource_dir] [base_work_dir] [nb_loop] [param]");
        System.out.println("  param=0 : no vfs and no persistance");
        System.out.println("  param=1 : vfs and no persistance");
        System.out.println("  param=2 : no vfs and persistance");
        System.out.println("  param=3 : vfs and persistance (fastest)");
        System.out.println("");


        String baseWorkDir = ".";
        String ressourceDir = UnitexJni.isUnderWindows() ? ".\\demojnires": "./demojnires";
        int nbLoop=1;
        int cfgParam = 0;


        if (args.length>=1)
            ressourceDir=args[0];

        if (args.length>=2)
            baseWorkDir=args[1];

        if (args.length>=3)
            nbLoop=Integer.parseInt(args[2]);
        if (nbLoop<1)
            nbLoop=1;

        if (args.length>=4)
            cfgParam=Integer.parseInt(args[3]);

        System.out.println("resource path : '"+ressourceDir+"' and work path is '"+baseWorkDir+"' and "+nbLoop+" executions");

        String graphResDir = UnitexJni.combineUnitexFileComponent(ressourceDir, "graph");
        String dictionnaryResDir = UnitexJni.combineUnitexFileComponent(ressourceDir, "dictionnary");
        String othersResDir = UnitexJni.combineUnitexFileComponent(ressourceDir, "others");


		UnitexJni.setStdOutTrashMode(true);
        //UnitexJni.setStdErrTrashMode(true);

        //////////////

        boolean fusevfs=(cfgParam == 1) || (cfgParam == 3);
        boolean fusepersist=(cfgParam == 2) || (cfgParam == 3);
        String PrefixVFS = "";
        System.out.println("use vfs:"+fusevfs+ (fusevfs ? (" on '"+getVirtualFilePfx()+"'") : "")) ;
        System.out.println("use persist:"+fusepersist);
        if (fusevfs)
        {
            PrefixVFS = getVirtualFilePfx();
            baseWorkDir=PrefixVFS+baseWorkDir;

            UnitexJni.copyUnitexFile(UnitexJni.combineUnitexFileComponent(othersResDir,"Alphabet.txt"),
                    UnitexJni.combineUnitexFileComponent(PrefixVFS+othersResDir,"Alphabet.txt"));
            UnitexJni.copyUnitexFile(UnitexJni.combineUnitexFileComponent(othersResDir,"Norm.txt"),
                    UnitexJni.combineUnitexFileComponent(PrefixVFS+othersResDir,"Norm.txt"));

            othersResDir=PrefixVFS+othersResDir;

            if (!fusepersist)
            {
                UnitexJni.copyUnitexFile(UnitexJni.combineUnitexFileComponent(dictionnaryResDir,"dela-en-public.bin"),
                                                UnitexJni.combineUnitexFileComponent(PrefixVFS+dictionnaryResDir,"dela-en-public.bin"));
                UnitexJni.copyUnitexFile(UnitexJni.combineUnitexFileComponent(dictionnaryResDir,"dela-en-public.inf"),
                        UnitexJni.combineUnitexFileComponent(PrefixVFS+dictionnaryResDir,"dela-en-public.inf"));
                dictionnaryResDir=PrefixVFS+dictionnaryResDir;

                UnitexJni.copyUnitexFile(UnitexJni.combineUnitexFileComponent(graphResDir,"AAA-hours-demo.fst2"),
                                                UnitexJni.combineUnitexFileComponent(PrefixVFS+graphResDir,"AAA-hours-demo.fst2"));
                graphResDir=PrefixVFS+graphResDir;
            }
        }

        String dicoFileName = UnitexJni.combineUnitexFileComponent(dictionnaryResDir,"dela-en-public.bin");
        String graphFileName = UnitexJni.combineUnitexFileComponent(graphResDir,"AAA-hours-demo.fst2");

        String workingGraphFileName;
        String workingDicoFileName;
        if (fusepersist)
        {
            workingDicoFileName = UnitexJni.loadPersistentDictionary(dicoFileName);
            workingGraphFileName = UnitexJni.loadPersistentFst2(graphFileName);
        }
        else
        {
            workingDicoFileName = dicoFileName;
            workingGraphFileName = graphFileName;
        }



        ////////////


        String CorpusWorkPath = UnitexJni.combineUnitexFileComponent(baseWorkDir, "workUnitexThread" + Thread.currentThread().getId());

        System.out.println("will work on "+CorpusWorkPath);
        UnitexJni.createUnitexFolder(CorpusWorkPath);
        UnitexJni.createUnitexFolder(UnitexJni.combineUnitexFileComponent(CorpusWorkPath,"corpus_snt"));
        String res="";

        long startT = System.currentTimeMillis();
        for (int i=0;i<nbLoop;i++)
        {
			// there is two space after watch, to check the offsets
            res=processUnitexWork(othersResDir,workingDicoFileName,workingGraphFileName,CorpusWorkPath,
                       "I want watch  at "+ ((i%10)+1)+":02 am see at 6:00 pm before leave at 15.47");
        }
        long endT = System.currentTimeMillis();

		
		// debug : you can remove this line to inspect files
        //UnitexJni.removeUnitexFolder(CorpusWorkPath);

        if (fusepersist)
        {
            UnitexJni.freePersistentDictionary(workingDicoFileName);
            UnitexJni.freePersistentFst2(workingGraphFileName);
        }

        System.out.println("");
        System.out.println("result:");
        System.out.println(res);
        System.out.println("time : "+(endT-startT)+" ms (average "+ ((endT-startT)/nbLoop)+" ms per iteration)");
}

}
