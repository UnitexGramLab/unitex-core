===============================================================================

                                 UnitexNET

                      .NET WRAPPER FOR UNITEX LIBRARY

===============================================================================

UnitexNET is managed C++/CLI wrapper around Unitex library, to be used in .NET
applications based on .NET 4.0 framework.

Target platforms are:
  - Win32: 32 bits
  - x64: 64 bits ("modern" Windows)

1. BUILDING UnitexNET
=====================

From Visual Studio, build project UnitexNet for both platforms: Win32 and x64.

This will generate the following files:
  - <source dir>\build\x86\UnitexNet\<config>\UnitexNet.dll (32 bits)
  - <source dir>\build\x64\UnitexNet\<config>\UnitexNet64.dll (32 bits)

where
<source dir> is Unitex-C++ source directory,
<config>     is either Debug or Release, depending on the chosen configuration 
             in Visual Studio

2. USING UnitexNET IN A .NET APPLICATION
========================================

Let us consider a C# application "TestUnitexNet".

1) Copy both UnitexNet.dll and Unitex64.dll somewhere into your application 
   source directory, but both in the same directory.
   For instance into TestUnitexNet\Lib.

2) Reference UnitexNET so that you can easily see the library's functions from 
   your C# code.
   
   Right click on your application's project in the Solution Explorer, then 
   Add reference and select either UnitexNet64.dll or UnitexNet.dll, depending
   on your machine's bitness (modern machines are 64 bits).

   This will import classes and function defintions into your project.

3) Copy into your project the file Unitex-C++\UnitexNet\UnitexNetLoader.cs.

4) Code sample

using System;
using System.Diagnostics;
using System.IO;
using UnitexNet;

namespace TestUnitexNet
{
    static class Program
    {
        static void Main(string[] args)
        {
            using (var unitexLoader = new UnitexNetLoader())
            {
                var unitex = new Unitex();

                const string workdir = @"C:\Users\Sylvain\temp";

                unitex.WriteUnitexFile(Path.Combine(workdir,"test_unicode.txt"), "un éssai unicode 16 en €");
                unitex.WriteUnitexFileUtf(Path.Combine(workdir,"test_utf8bom.txt"), "un éssai unicode 16 en €", true);
                unitex.WriteUnitexFileUtf(Path.Combine(workdir,"test_utf8nobom.txt"), "un éssai unicode 16 en €", false);

                var files = new[] { "ee_testunitex_utf16le.txt", "ee_testunitex_utf16be.txt", "ee_testunitex_utf8.txt", "ee_testunitex_utf8nobom.txt" };
                foreach (var file in files)
                {
                    var filename = Path.Combine(workdir, file);
                    Trace.TraceInformation("content of : " + filename);
                    var str = unitex.GetUnitexFileString(filename);
                    Trace.TraceInformation(str);
                    Trace.TraceInformation("---------");
                }
            }
        }
    }
}
    