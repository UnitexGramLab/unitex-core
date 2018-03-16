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

import fr.umlv.unitex.jni.UnitexJni;

public class TestUnitexJni {

	public static void main(String[] args) {
	try {

		boolean tryNormalize = true;

		if (args.length>0) {
			if (args[0].equals("UnitexTool")) {
				tryNormalize = false;
				UnitexJni.execUnitexTool(args);
			}
		}

		if (args.length>0) {
			if (args[0].equals("RunLog")) {
				tryNormalize = false;
				UnitexJni.execRunLog(args);
			}
		}

		if (tryNormalize) {
			UnitexJni.execUnitexTool(new String[] {"UnitexToolLogger","Normalize","--help"});
		}
	} catch (UnsatisfiedLinkError e) {
		System.err.println("Cannot load the UnitexJni library. You may have to set the java.library.path");
		System.err.println("property as follows:\n");
		System.err.println("java -Djava.library.path=<path to [lib]UnitexJni.so/dll> ...\n");
	}
	}

}

