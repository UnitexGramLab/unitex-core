package fr.univ_mlv.unitex;

import java.io.File;
import java.util.ArrayList;
import java.util.List;



public class UnitexJNIExperiment {

        // Load the dll that exports functions callable from java
    static {
        System.loadLibrary("UnitexJNIExperiment");
		System.out.println("loading jni UnitexJNIExperiment");
        }

    // Imported function declarations
        public native static int PerformUnitexToolByStringArray(String [] cmdarray);

        public static boolean isWindows(){
                String os = System.getProperty("os.name").toLowerCase();
            return (os.indexOf( "win" ) >= 0);
        }




private static final String sepPathWin = "\\";
private static final String sepPathUnixMac = "/";

private static final String sepPath = isWindows() ? sepPathWin: sepPathUnixMac;
}
