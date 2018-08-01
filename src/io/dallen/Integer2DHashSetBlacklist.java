package io.dallen;

public class Integer2DHashSetBlacklist implements IPBlacklist {

    private static final double INIT_SIZE_MOD = 1.1;

    private static final int EMPTY = 0;

    private final int[][] table;

    private int load = 0;

    private final HashFunction hash1;
    private final HashFunction hash2;

    public Integer2DHashSetBlacklist(int size, HashFunction h1, HashFunction h2) {
        int len = (int) Math.sqrt((double) size * INIT_SIZE_MOD);
        table = new int[len][len];
        hash1 = h1;
        hash2 = h2;
    }

    @Override
    public void add(String ip) {
        if(load > (table.length * table.length)) {
            throw new ArrayIndexOutOfBoundsException("HashSet full");
        }
        int intIp = pack(ip);
        int searchX, x = searchX = Math.abs(hash1.hash(intIp)) % table.length;
        int searchY, y = searchY = Math.abs(hash2.hash(intIp)) % table.length;
        int bellowX = (x != 0) ? x - 1 : table.length - 1;
        int bellowY = (y != 0) ? y - 1 : table.length - 1;
        for(;table[searchX][searchY] != EMPTY && searchX != bellowX; searchX = (searchX + 1) % table.length) {
            for(;table[searchX][searchY] != EMPTY && searchY != bellowY; searchY = (searchY + 1) % table.length);
        }
        table[searchX][searchY] = intIp;
    }

    @Override
    public boolean contains(String ip) {
        int intIp = pack(ip);
        int searchX, x = searchX = Math.abs(hash1.hash(intIp)) % table.length;
        int searchY, y = searchY = Math.abs(hash2.hash(intIp)) % table.length;
        int bellowX = (x != 0) ? x - 1 : table.length - 1;
        int bellowY = (y != 0) ? y - 1 : table.length - 1;
        for(;table[searchX][searchY] != EMPTY && searchX != bellowX; searchX = (searchX + 1) % table.length) {
            for(;table[searchX][searchY] != EMPTY && searchY != bellowY; searchY = (searchY + 1) % table.length) {
                if(table[searchX][searchY] == intIp) {
                    return true;
                }
            }
        }
        return false;
    }

    @Override
    public int size() {
        return load;
    }

    public interface HashFunction {
        int hash(int ip);
    }

    private static int pack(String ipAddress) {

        int result = 0;

        String[] ipAddressInArray = ipAddress.split("\\.");

        for (int i = 3; i >= 0; i--) {
            int ip = Integer.parseInt(ipAddressInArray[3 - i]);
            result |= ip << (i * 8);
        }

        return result;
    }
}
