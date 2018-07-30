package io.dallen;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.Set;

public class StringBlacklist implements IPBlacklist {

    private final Set<String> blacklist;

    public StringBlacklist(int defSize) {
        blacklist = new HashSet<String>(defSize);
    }

    @Override
    public void addIP(String ip) {
        blacklist.add(ip);
    }

    @Override
    public boolean blocked(String ip) {
        return blacklist.contains(ip);
    }

    @Override
    public int size() {
        return blacklist.size();
    }
}
