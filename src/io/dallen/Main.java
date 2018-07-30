package io.dallen;

import java.io.*;
import java.util.*;

public class Main {

    private static final int TEST_SIZE =  8 * 1000 * 1000;

    private static final String ipfile = "sampleips.txt";

    private static final Runtime runtime = Runtime.getRuntime();

    public static void main(String[] args) throws Exception {
//        createIPFile(ipfile, TEST_SIZE * 2);
//        System.out.println("Built ip file, " + TEST_SIZE * 2 + " unique IPs created");

//        StringBlacklist stringBlacklist = new StringBlacklist(TEST_SIZE);
//        runSpeedTest("Simple String Blacklist", stringBlacklist, ipfile);
//
//        IntegerBlacklist integerBlacklist = new IntegerBlacklist(TEST_SIZE);
//        runSpeedTest("Simple Integer Blacklist", integerBlacklist, ipfile);
//
//        IntegerTreeBlacklist integerTreeBlacklist = new IntegerTreeBlacklist();
//        runSpeedTest("Integer Tree Blacklist", integerTreeBlacklist, ipfile);

//        IntegerLinearTreeBlacklist integerLinearTreeBlacklist = new IntegerLinearTreeBlacklist();
//        runSpeedTest("Integer Linear Tree Blacklist", integerLinearTreeBlacklist, ipfile);

        PrimitiveIntegerHashBlacklist primitiveIntegeHashBlacklist = new PrimitiveIntegerHashBlacklist(TEST_SIZE);
        runSpeedTest("Primitive int HashSet Blacklist", primitiveIntegeHashBlacklist, ipfile);
//
        new Scanner(System.in).nextLine();
//
//        System.out.println("Integer Linear Tree BlackList size: " + String.valueOf(integerLinearTreeBlacklist.size()));
//        System.out.println("Integer Tree BlackList size: " + String.valueOf(integerTreeBlacklist.size()));
//        System.out.println("Integer BlackList size: " + String.valueOf(integerBlacklist.size()));
//        System.out.println("String BlackList size: " + String.valueOf(stringBlacklist.size()));
    }

    static void createIPFile(String name, int number) {
        LinkedList<String> ips = generateRandomIpv4s(number);
        try(BufferedWriter writer = new BufferedWriter(new FileWriter(name))){
            for(String ip : ips) {
                writer.write(ip + "\n");

            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    static void runSpeedTest(String name, IPBlacklist list, String ips) throws FileNotFoundException {

        Scanner ipScanner = new Scanner(new File(ips));

        long beforeTime = System.nanoTime();

        for (int i = 0; i < TEST_SIZE; i++) {
            double done = (double) i / (double) TEST_SIZE * 100d;
            if(done % 10 == 0) {
                System.out.print(done + "%...");
            }
            list.addIP(ipScanner.nextLine());
        }
        System.out.println();

        double insertBreak = System.nanoTime();

        ipScanner =  new Scanner(new File(ips));

        for (int i = 0; i < TEST_SIZE; i++) {
            double done = (double) i / (double) (TEST_SIZE * 2) * 100d;
            if(done % 10 == 0) {
                System.out.print(done + "%...");
            }
            if(!list.blocked(ipScanner.nextLine())) {
                System.err.println("IP on list not blocked");
            }
        }

        for (int i = TEST_SIZE; i < TEST_SIZE * 2; i++) {
            double done = (double) i / (double) (TEST_SIZE * 2) * 100d;
            if(done % 10 == 0) {
                System.out.print(done + "%...");
            }
            if(list.blocked(ipScanner.nextLine())) {
                System.err.println("IP not on list not blocked");
            }
        }
        System.out.println();

        double testBreak = System.nanoTime();

//        ipScanner =  new Scanner(new File(ips));
//
//        while (ipScanner.hasNext()) {
//            list.removeIP(ipScanner.nextLine());
//        }
//
//        double removeBreak = System.nanoTime();

        System.out.println();
        System.out.println(name + ":");
        System.out.println("\tInsert Time: " + Double.toString((insertBreak - beforeTime)/(1000.0 * 1000.0)) + " ms");
        System.out.println("\tTest Time: " + Double.toString((testBreak - insertBreak)/(1000.0 * 1000.0)) + " ms");
//        System.out.println("\tRemove Time: " + Double.toString((removeBreak - testBreak)/(1000.0 * 1000.0)) + " ms");
        System.out.println();
    }

    static LinkedList<String> generateRandomIpv4s(int number) {
        Set<String> ips = new HashSet<>();
        for(int i = 0; i < number; i++) {
            String ip;
            do {
                ip = generateRandomIpv4();
            } while (ips.contains(ip));
            ips.add(ip);
        }
        return new LinkedList<String>(ips);
    }

    static String generateRandomIpv4() {
        Random ipGenerator = new Random();

        return Integer.toString(ipGenerator.nextInt(254) + 1) + "." +
                Integer.toString(ipGenerator.nextInt(254) + 1) + "." +
                Integer.toString(ipGenerator.nextInt(254) + 1) + "." +
                Integer.toString(ipGenerator.nextInt(254) + 1);
    }
}
