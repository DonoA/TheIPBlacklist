package io.dallen;

public class PrimitiveIntegerHashBlacklist implements IPBlacklist {

    private static final double INIT_SIZE_MOD = 1.3;

    // The value 0 denotes a space is empty, the value -1 denotes a space is deleted
    private final int EMPTY = 0;
    private final int DELETED = -1;

    private int maxDisplacement = 0;

    private int maxSearched = 0;

    private int[] table;

    private int load;

    public PrimitiveIntegerHashBlacklist(int size) {
        table = new int[(int) (size * INIT_SIZE_MOD)];
    }

    @Override
    public void addIP(String ip) {
        int intIp = pack(ip);
        if(this.load + 1 >= table.length) {
            System.err.println("Out of space");
            return;
        }
        int defPos;
        int insertPos = defPos = hash(intIp) % table.length;
        for(;table[insertPos] != EMPTY;insertPos = (insertPos + 1) % table.length) {
            // if the displacement of the inserting element is further than the displacement of the current element
            // swap the two and continue
            if(displacementOf(table[insertPos], insertPos) < insertPos - defPos ) {
                // record the displacement
                if(insertPos - defPos > maxDisplacement) {
                    maxDisplacement = insertPos - defPos;
                }
                // swap the two
                int x = table[insertPos];
                table[insertPos] = intIp;
                intIp = x;
                // reset the counting and such
                insertPos = defPos = hash(intIp) % table.length;
            }
        }
        if(insertPos - defPos > maxDisplacement) {
            maxDisplacement = insertPos - defPos;
        }
        table[insertPos] = intIp;
        this.load++;
    }

    @Override
    public boolean blocked(String ip) {
        int intIp = pack(ip);
        int defPos, dataPos;
        for(dataPos = defPos = hash(intIp) % table.length;
            table[dataPos] != EMPTY && table[dataPos] != DELETED && (dataPos-defPos) <= maxDisplacement;
            dataPos = (dataPos + 1) % table.length) {
            if(table[dataPos] == intIp) {
                return true;
            }
        }
        if(dataPos - defPos > maxSearched) {
            maxSearched = dataPos - defPos;
        }
        return false;
    }

    private int displacementOf(int ip, int location) {
        return location - (hash(ip) % table.length);
    }

    private int hash(int ip) {
        return Math.abs(ip * 37);
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
