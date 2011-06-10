#!/bin/bash

rm unnormalized.snt
javac fr/univ_mlv/unitex/UnitexJNIExperiment.java
javac UnitexJNIExperimentDemo.java
javah fr.univ_mlv.unitex.UnitexJNIExperiment


java -Djava.library.path=. UnitexJNIExperimentDemo

ls -l unnormalized.*
