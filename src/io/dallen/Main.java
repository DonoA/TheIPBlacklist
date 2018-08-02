package io.dallen;

import java.io.*;
import java.util.*;
import java.util.stream.Collectors;
import java.util.stream.Stream;

public class Main {

    private static final int TEST_SIZE = 20 * 1000; // 8 * 1000 * 1000;

    private static final String ipfile = "sampleips.txt";

    public static void main(String[] args) throws Exception {
//        createIPFile(ipfile, TEST_SIZE * 2);
//        System.out.println("Built ip file, " + TEST_SIZE * 2 + " unique IPs created");

//        StringBlacklist stringBlacklist = new StringBlacklist(TEST_SIZE);
//        runSpeedTest("Simple String Blacklist", stringBlacklist, ipfile);

//        IntegerBlacklist integerBlacklist = new IntegerBlacklist(TEST_SIZE);
//        runSpeedTest("Simple Integer Blacklist", integerBlacklist, ipfile);

//        IntegerTreeBlacklist integerTreeBlacklist = new IntegerTreeBlacklist();
//        runSpeedTest("Integer Tree Blacklist", integerTreeBlacklist, ipfile);

//        IntegerLinearTreeBlacklist integerLinearTreeBlacklist = new IntegerLinearTreeBlacklist();
//        runSpeedTest("Integer Linear Tree Blacklist", integerLinearTreeBlacklist, ipfile);

        PrimitiveIntegerHashBlacklist primitiveIntegerHashBlacklist = new PrimitiveIntegerHashBlacklist(TEST_SIZE);
        runSpeedTest("Primitive int HashSet Blacklist", primitiveIntegerHashBlacklist, ipfile);

        Integer2DHashSetBlacklist integer2DHashSetBlacklist = new Integer2DHashSetBlacklist(TEST_SIZE,
                i -> i * 37, i -> i * 31);
        runSpeedTest("2D primitive int HashSet Blacklist", integer2DHashSetBlacklist, ipfile);

        // Pause so profiler can run
        new Scanner(System.in).nextLine();
    }

    static void createIPFile(String name, int number) {
        LinkedList<String> ips = generateRandomIpv4s(number);
        try (BufferedWriter writer = new BufferedWriter(new FileWriter(name))) {
            for (String ip : ips) {
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
            if (done % 10 == 0) {
                System.out.print(done + "%...");
            }
            list.add(ipScanner.nextLine());
        }
        System.out.println();

        double insertBreak = System.nanoTime();

        ipScanner = new Scanner(new File(ips));

        for (int i = 0; i < TEST_SIZE; i++) {
            double done = (double) i / (double) (TEST_SIZE * 2) * 100d;
            if (done % 10 == 0) {
                System.out.print(done + "%...");
            }
            if (!list.contains(ipScanner.nextLine())) {
                System.err.println("IP on list not blocked");
            }
        }

        double blockedBreak = System.nanoTime();


        for (int i = TEST_SIZE; i < TEST_SIZE * 2; i++) {
            double done = (double) i / (double) (TEST_SIZE * 2) * 100d;
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

    static LinkedList<String> generateRandomIpv4s(int number) {
        Random ipGenerator = new Random();

        return Stream.generate(() ->
                Stream.generate(() -> Integer.toString(ipGenerator.nextInt(254) + 1)).limit(4)
                    .collect(Collectors.joining("."))
        ).distinct().limit(number).collect(Collectors.toCollection(LinkedList::new));
    }
}
