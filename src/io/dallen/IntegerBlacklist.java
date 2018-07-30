package io.dallen;

import java.util.HashSet;
import java.util.Set;

public class IntegerBlacklist implements IPBlacklist {

    private final Set<Integer> blacklist;

    public IntegerBlacklist(int defSize) {
        blacklist = new HashSet<Integer>(defSize);
    }

    @Override
    public void addIP(String ip) {
        blacklist.add(pack(ip));
    }

    @Override
    public boolean blocked(String ip) {
        return blacklist.contains(pack(ip));
    }

    @Override
    public int size() {
        return blacklist.size();
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
