/*
 * Unitex
 *
 * Copyright (C) 2001-2020 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
 * File created and contributed by Gilles Vollant (Ergonotics SAS and Amabis SAS) 
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
 *    "%JAVA_HOME%\\bin\\javac.exe" UnitexJniPkgDemo.java
 *    "%JAVA_HOME%\\bin\\java.exe" -classpath .; UnitexPkgJniDemo %1 %2 %3 %4
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
 *  then run the script/batch, with testHour.lingpkg in current page
 *
 */

public class UnitexJniPkgDemo {

      private static final String pathSeparator = UnitexJni.isUnderWindows() ? "\\" : "/";

      public static final String getVirtualFilePfx()
      {
        if (UnitexJni.unitexAbstractPathExists("*"))
            return "*";

        if (UnitexJni.unitexAbstractPathExists("$:"))
            return "$:";

        return null;
      }

	  
    public static void main(String [] args) {
		
		String pkgdir="testHour.lingpkg";
		
        if (args.length>=1)
            pkgdir=args[0];
		String workdir=UnitexJni.combineUnitexFileComponent(getVirtualFilePfx(),"work");
		String resdir=UnitexJni.combineUnitexFileComponent(getVirtualFilePfx(),"res");
		String intxt=UnitexJni.combineUnitexFileComponent(getVirtualFilePfx(),"in.txt");
		String outtxt=UnitexJni.combineUnitexFileComponent(getVirtualFilePfx(),"out.txt");
		String script=UnitexJni.combineUnitexFileComponent(resdir,"script","standard.uniscript");
		
        System.out.println("os ms-windows:"+UnitexJni.isUnderWindows()+" : "+System.getProperty("os.name")+ " "+java.io.File.separator);
        
        System.out.println("");
		
		UnitexJni.execUnitexTool("UnitexTool SelectOutput -o off"); 
 
		String cmdInstall="UnitexTool InstallLingResourcePackage -p " + pkgdir + " -x " + resdir;
		UnitexJni.execUnitexTool(cmdInstall);
					  
 				  
        UnitexJni.writeUnitexFile(intxt,"it is 11:04am now and soon 4:17pm");

		UnitexJni.execUnitexTool("UnitexTool RunScript -v -a INPUT_FILE_1=" + intxt + " -a CORPUS_WORK_DIR="+workdir+
		       " -a PACKAGE_DIR=" + resdir + " -a OUTPUT_FILE_1="+outtxt + " "+script);
		 
					  
		UnitexJni.execUnitexTool("UnitexTool InstallLingResourcePackage -p " + pkgdir +
		              " -x " + resdir +" --uninstall");
					  
		
        String rescontent  = UnitexJni.getUnitexFileString(outtxt);

		
        System.out.println("");
        System.out.println("result:");
        System.out.println(rescontent);
		
}

}
