package io.dallen;

public interface IPBlacklist {
    void addIP(String ip);
    boolean blocked(String ip);
    int size();
}
