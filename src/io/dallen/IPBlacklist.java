package io.dallen;

public interface IPBlacklist {
    void add(String ip);
    boolean contains(String ip);
    int size();
}
