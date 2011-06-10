import java.io.File;
import java.util.ArrayList;
import java.util.List;
import java.io.*;
import java.net.*;


import fr.univ_mlv.unitex.UnitexJNIExperiment;


public class UnitexJNIExperimentDemo {






private static final String sepPathWin = "\\";
private static final String sepPathUnixMac = "/";

      public static boolean isMSWindows(){
            String os = System.getProperty("os.name").toLowerCase();
          return (os.indexOf( "win" ) >= 0); 
      }

private static final String sepPath = isMSWindows() ? sepPathWin: sepPathUnixMac;

private static String[] cmdArray={"UnitexTool","Normalize","-k","UTF8","-q","UTF8","unnormalized.txt"};

    public static void main(String [] args) {
   
 
 


		System.loadLibrary("UnitexJNIExperiment");
	
fr.univ_mlv.unitex.UnitexJNIExperiment uje = new fr.univ_mlv.unitex.UnitexJNIExperiment();



System.out.println("Hello Normalize");



 
int retcpr=uje.PerformUnitexToolByStringArray(cmdArray);

System.out.println("res Normalize:"+retcpr);
}
}
