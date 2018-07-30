package io.dallen;

public class PrimitiveIntegerHashBlacklist implements IPBlacklist {

    private static final byte EMPTY = 0;
    private static final byte FILLED = 1;
    private static final byte DELETED = 2;

    private int[] table;

    private byte[] flags;

    private int load;

    public PrimitiveIntegerHashBlacklist(int size) {
        table = new int[(int) (size * 1.5)];
        flags = new byte[(int) (size * 1.5)];
    }

    @Override
    public void addIP(String ip) {
        int intIp = pack(ip);
        if(this.load + 1 >= table.length) {
            System.err.println("Out of space");
            return;
        }
        int insertPos = Math.abs(intIp * 37) % flags.length;
        for(;flags[insertPos] == FILLED;insertPos = (insertPos + 1) % flags.length);
        flags[insertPos] = FILLED;
        table[insertPos] = intIp;
        this.load++;
    }

    @Override
    public boolean blocked(String ip) {
        int intIp = pack(ip);
        for(int dataPos = Math.abs(intIp * 37) % flags.length; flags[dataPos] != EMPTY; dataPos = (dataPos + 1) % flags.length) {
            if(table[dataPos] == intIp) {
                return true;
            }
        }
        return false;
    }

    @Override
    public int size() {
        return this.load;
    }

    private int pack(String ipAddress) {

        int result = 0;

        String[] ipAddressInArray = ipAddress.split("\\.");

        for (int i = 3; i >= 0; i--) {
            int ip = Integer.parseInt(ipAddressInArray[3 - i]);
            result |= ip << (i * 8);
        }

        return result;
    }
}
