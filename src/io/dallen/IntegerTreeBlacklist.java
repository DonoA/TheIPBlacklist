package io.dallen;

import java.util.*;

public class IntegerTreeBlacklist implements IPBlacklist {

    private final TreeNode root = new TreeNode((byte) 0);

    @Override
    public void add(String ip) {
        String[] bits = ip.split("\\.");
        TreeNode current = root;
        for(String s : bits) {
            byte b = (byte) Integer.parseInt(s);
            current = current.findOrCreateChild(b);
        }
    }

    @Override
    public boolean contains(String ip) {
        String[] bits = ip.split("\\.");
        TreeNode current = root;
        for(String s : bits) {
            byte b = (byte) Integer.parseInt(s);
            current = current.find(b);
            if(current == null) {
                return false;
            }
        }
        return true;
    }

    @Override
    public int size() {
        return root.getSize();
    }

    static class TreeNode {
        Map<Byte, TreeNode> children = new HashMap<>();

        final byte val;

        public TreeNode(byte val) {
            this.val = val;
        }

        public TreeNode findOrCreateChild(byte v) {
            TreeNode tr = children.get(v);
            if(tr == null) {
                tr = new TreeNode(v);
                children.put(v, tr);
            }
            return tr;
        }

        public TreeNode find(byte v) {
            return children.get(v);
        }

        public int getSize() {
            if(children.isEmpty()) {
                return 1;
            }
            int size = 0;
            for(TreeNode treeNode : children.values()) {
                size += treeNode.getSize();
            }
            return size;
        }
    }
}
