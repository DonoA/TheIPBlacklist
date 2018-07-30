package io.dallen;

public class IntegerLinearTreeBlacklist implements IPBlacklist {

    private byte[] tree  = new byte[0];

    private int[] childPos = new int[0];

    @Override
    public void addIP(String ip) {
        String[] bits = ip.split("\\.");
        byte b;
        int endOfChildren = childPos.length > 0 ? childPos[0] : 0;
        int startOfChildren = 0;

        for(int i = 0; i < bits.length; i++) {
            b = (byte) Integer.parseInt(bits[i]);
            int p = scanFor(tree, b, startOfChildren, endOfChildren);
            if(p == -1) {
                // this node does not exist and neither will its children
                int[] newPos = new int[bits.length - i];

                newPos[0] = endOfChildren;

                for(int j = 1; j < newPos.length; j++) {
                    newPos[j] = childPos.length > endOfChildren ? childPos[endOfChildren] + j : tree.length + j;
                    endOfChildren = childPos.length > endOfChildren ? childPos[endOfChildren] : childPos.length;
                }

                boolean lastOct = !(i + 1 < bits.length);
                // Create space for the new node
                extendTree(newPos);
                // set the nodes
                for(int j = 0; j < newPos.length; j++) {
                    tree[newPos[j]] = (byte) Integer.parseInt(bits[i + j]);
                    if(j + 1 < newPos.length) {
                        childPos[newPos[j]] = newPos[j + 1];
                    }
                }
                break;
            } else {
                // lookup where children should be
                if(childPos.length > p) {
                    startOfChildren = childPos[p];
                    if (childPos.length > p + 1) {
                        endOfChildren = childPos[p + 1];
                    } else {
                        endOfChildren = tree.length;
                    }
                }
            }
        }
    }

    // gets the position of a value within the searched bounds of the tree, or -1
    private int scanFor(byte[] list, byte val, int start, int end) {
        for(int i = start; i < end; i++) {
            if(list[i] == val) {
                return i;
            }
        }
        return -1;
    }

    // gets the position of a value within the searched bounds of the tree, or -1
    private int scanFor(int[] list, int val, int start, int end) {
        for(int i = start; i < end; i++) {
            if(list[i] == val) {
                return i;
            }
        }
        return -1;
    }

    // expand the tree, leaving space at what will be pos provided
    private void extendTree(int[] pos) {
        byte[] newTree = new byte[tree.length + pos.length];
        int[] newChildPos = new int[childPos.length + pos.length - 1];
        // not as fast as system copy, should figure out if that can be used!
        int skipped = 0;
        for(int i = 0, j = 0; i < newTree.length && j < tree.length; i++) {
            if(scanFor(pos, i, 0, pos.length) == -1) {
                newTree[i] = tree[j];
                // adjust for change in positions due to shift
                if(i < newChildPos.length) {
                    newChildPos[i] = childPos[j] + countBellow(pos, childPos[j] + skipped);
                }
                j++;
            } else {
                skipped++;
            }
        }
        tree = newTree;
        childPos = newChildPos;
    }

    private int countBellow(int[] list, int val) {
        int count = 0;
        for(int i : list) {
            if(i <= val) {
                count++;
            }
        }
        return count;
    }

    @Override
    public boolean blocked(String ip) {
        String[] bits = ip.split("\\.");
        byte b;
        int endOfChildren = childPos.length > 0 ? childPos[0] : 0;
        int startOfChildren = 0;

        for(int i = 0; i < bits.length; i++) {
            b = (byte) Integer.parseInt(bits[i]);
            int p = scanFor(tree, b, startOfChildren, endOfChildren);
            if(p == -1) {
                return false;
            } else {
                // lookup where children should be
                if(childPos.length > p) {
                    startOfChildren = childPos[p];
                    if (childPos.length > p + 1) {
                        endOfChildren = childPos[p + 1];
                    } else {
                        endOfChildren = tree.length;
                    }
                }
            }
        }
        return true;
    }

    @Override
    public int size() {
        return tree.length - childPos[childPos[childPos[0]] - 1] - 1;
    }
}
