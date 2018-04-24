/**
 * @author joewellHe
 * @descripition compute pearson score between result and golden standard score
 * @date 2018-1-10
 */

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.StringTokenizer;

public class Pearson {

    public static void main(String[] args) throws IOException {

        if(args.length==0){
            System.out.println("please run Pearson program by follow form:");
            System.out.println("java Pearson [resultFile] [goldenFile]");
            //jiawei 2018-1-10
            System.exit(0);
        }
        for (int i = 0; i < args.length; i++) {
            System.out.println(args[i]);
        }

        String resultFile = args[0];
        String goldenFile = args[1];

        ArrayList<Double> res = new ArrayList<Double>();
        ArrayList<Double> gold = new ArrayList<Double>();

        // read test result file
        readFile(resultFile, res);
        // read golden score file
        readFile(goldenFile, gold);

        double p = 0;
        double sumX = 0, sumY = 0, sumXsquare = 0, sumYsquare = 0, sumXYProduct = 0;
        if (res.size() != gold.size()) {
            System.out.println("data format error!");
            System.exit(1);
        } else {
            int n = res.size();
            System.out.println("=======the data size is: "+n);
            for (int m = 0; m < res.size(); m++) {
                double x = res.get(m);
                double y = gold.get(m);

                sumX += x;
                sumY += y;
                sumXsquare += x * x;
                sumYsquare += y * y;
                sumXYProduct += x * y;

            }
            double pearson = (n * sumXYProduct - sumX * sumY)
                    / (Math.sqrt(n * sumXsquare - sumX * sumX) * Math.sqrt(n * sumYsquare - sumY * sumY));

            System.out.println("Pearson score: " + String.format("%.2f", pearson));
        }
    }

    public static void readFile(String filepath, ArrayList<Double> data)throws IOException{
        File readFile2 = new File(filepath);
        if (readFile2.length() > 0) {
            BufferedReader reader = new BufferedReader(new FileReader(readFile2));
            try {
                String line = null;
                while (((line = reader.readLine()) != null)) {

                    StringTokenizer t = new StringTokenizer(line, "\n");
                    data.add(Double.parseDouble(t.nextToken()));
                }
                reader.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        } else {
            System.out.println("Read File failed , Exit!");
            System.exit(1);
        }
    }
}
