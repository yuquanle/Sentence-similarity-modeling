#!/bin/sh
cd ACVTK
g++ *.cpp -o MAIN
mv MAIN ..
cd ..
cd Evaluate
javac Pearson.java
mv Pearson.class ..
cd ..

./MAIN -i ACVT/2013ACVT/ACVT_STS2013.FNWN.txt -o result_ACVT_STS2013.FNWN -w ACVT/2013PSL/PSL_STS2013.FNWN.txt
./MAIN -i ACVT/2013ACVT/ACVT_STS2013.headlines.txt -o result_ACVT_STS2013.headlines -w ACVT/2013PSL/PSL_STS2013.headlines.txt
./MAIN -i ACVT/2013ACVT/ACVT_STS2013.OnWN.txt -o result_ACVT_STS2013.OnWN -w ACVT/2013PSL/PSL_STS2013.OnWN.txt

java Pearson result_ACVT_STS2013.FNWN ACVT/2013GS/2013STS.gs.FNWN.txt
java Pearson result_ACVT_STS2013.headlines ACVT/2013GS/2013STS.gs.headlines.txt
java Pearson result_ACVT_STS2013.OnWN ACVT/2013GS/2013STS.gs.OnWN.txt
