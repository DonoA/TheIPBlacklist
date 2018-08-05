package io.dallen;

import java.io.*;
import java.util.*;
import java.util.stream.Collectors;
import java.util.stream.IntStream;
import java.util.stream.Stream;

public class Main {

    private static Random ipGenerator = new Random();

    private static Iterator<String> uniqueIps =
            Stream.generate(() ->
                Stream.generate(() -> Integer.toString(ipGenerator.nextInt(254) + 1)).limit(4)
                    .collect(Collectors.joining("."))
            ).distinct().iterator();

    private static final String ipfile = "sampleips.txt";

    private static int testSize;

    public static void main(String[] args) throws Exception {
        testSize = Integer.parseInt(System.getenv("TEST_COUNT"));

        if(System.getenv("BUILD_TEST") != null){
            createIPFile(ipfile, testSize * 2);
            System.out.println("Built ip file, " + testSize * 2 + " unique IPs created");
        }

        StringBlacklist stringBlacklist;
        IntegerBlacklist integerBlacklist;
        IntegerTreeBlacklist integerTreeBlacklist;
        IntegerLinearTreeBlacklist integerLinearTreeBlacklist;
        PrimitiveIntegerHashBlacklist primitiveIntegerHashBlacklist;
        Integer2DHashSetBlacklist integer2DHashSetBlacklist;

        if(System.getenv("STRING_BLACKLIST") != null){
            stringBlacklist = new StringBlacklist(testSize);
            runSpeedTest("Simple String Blacklist", stringBlacklist, ipfile);
        }

        if(System.getenv("INTEGER_BLACKLIST") != null){
            integerBlacklist = new IntegerBlacklist(testSize);
            runSpeedTest("Simple Integer Blacklist", integerBlacklist, ipfile);
        }

        if(System.getenv("INTEGER_TREE_BLACKLIST") != null){
            integerTreeBlacklist = new IntegerTreeBlacklist();
            runSpeedTest("Integer Tree Blacklist", integerTreeBlacklist, ipfile);
        }

        if(System.getenv("INTEGER_TREE_BLACKLIST") != null){
            integerLinearTreeBlacklist = new IntegerLinearTreeBlacklist();
            runSpeedTest("Integer Linear Tree Blacklist", integerLinearTreeBlacklist, ipfile);
        }

        if(System.getenv("PRIMITVE_INT_HASHSET") != null){
            primitiveIntegerHashBlacklist = new PrimitiveIntegerHashBlacklist(testSize);
            runSpeedTest("Primitive int HashSet Blacklist", primitiveIntegerHashBlacklist, ipfile);
        }

        if(System.getenv("INTEGER_TREE_BLACKLIST") != null){
            integer2DHashSetBlacklist = new Integer2DHashSetBlacklist(testSize,
                    i -> i * 37, i -> i * 31);
            runSpeedTest("2D primitive int HashSet Blacklist", integer2DHashSetBlacklist, ipfile);
        }

        // Pause so profiler can run
        System.out.println("Press enter to complete");
        new Scanner(System.in).nextLine();
    }

    static void createIPFile(String name, int number) {
        try (BufferedWriter writer = new BufferedWriter(new FileWriter(name))) {
            IntStream.range(0,number).forEach( i -> safeWrite(writer, uniqueIps.next() + "\n"));
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    static void runSpeedTest(String name, IPBlacklist list, String ips) throws FileNotFoundException {

        Scanner ipScanner = new Scanner(new File(ips));

        long beforeTime = System.nanoTime();

        for (int i = 0; i < testSize; i++) {
            double done = (double) i / (double) testSize * 100d;
            if (done % 10 == 0) {
                System.out.print(done + "%...");
            }
            list.add(ipScanner.nextLine());
        }
        System.out.println();

        double insertBreak = System.nanoTime();

        ipScanner = new Scanner(new File(ips));

        for (int i = 0; i < testSize; i++) {
            double done = (double) i / (double) (testSize * 2) * 100d;
            if (done % 10 == 0) {
                System.out.print(done + "%...");
            }
            if (!list.contains(ipScanner.nextLine())) {
                System.err.println("IP on list not blocked");
            }
        }

        double blockedBreak = System.nanoTime();


        for (int i = testSize; i < testSize * 2; i++) {
            double done = (double) i / (double) (testSize * 2) * 100d;
            if (done % 10 == 0) {
                System.out.print(done + "%...");
            }
            if (list.contains(ipScanner.nextLine())) {
                System.err.println("IP not on list not blocked");
            }
        }
        System.out.println();

        double notBlockedBreak = System.nanoTime();

        System.out.println();
        System.out.println(name + ":");
        System.out.println("\tInsert Time:\t\t\t\t" + Double.toString((insertBreak - beforeTime) / (1000.0 * 1000.0)) + " ms");
        System.out.println("\tTest Containing Time:\t\t" + Double.toString((blockedBreak - insertBreak) / (1000.0 * 1000.0)) + " ms");
        System.out.println("\tTest Not Containing Time:\t" + Double.toString((notBlockedBreak - blockedBreak) / (1000.0 * 1000.0)) + " ms");
        System.out.println();
    }

    private static void safeWrite(BufferedWriter writer, String str) {
        try {
            writer.write(str);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
