# 2-3-4 Tree (B-Tree of Order 4)

## Project Description

A complete implementation of a 2-3-4 Tree in C++, which is a B-Tree with minimum degree `t = 2`. Every internal node holds between 1 and 3 keys and has between 2 and 4 children. The tree is always perfectly balanced — all leaves sit at the same depth regardless of insertion order.

## How to Build and Run

```bash
g++ src/main.cpp -o btree
./btree
```

The program reads `input.txt` and produces:

| File | Contents |
|---|---|
| `output.txt` | Full operation trace with level-order views |
| `log.txt` | Tree state and split count after every operation |
| `snapshot.dat` | Serialized tree created by the SAVE operation |

## Input File Format

```
I 10      → Insert 10
D 10      → Delete 10
S 20      → Search 20
SAVE      → Serialize current tree to snapshot.dat
RESTORE   → Clear tree and rebuild from snapshot.dat, print result
REVERSE   → Rebuild tree in reverse key order, print comparison
```

Any line that does not match these formats is silently skipped.

---

## Implementation — Part by Part

### Part 2: Core Functions

**`insertNonFull(int k)`**
Inserts key `k` into a node. If the node is a leaf, it shifts keys right and places `k` in sorted position. If it is internal, it finds the correct child to descend into, splitting that child if it is full before recursing. 

**`splitChild(int i, BTreeNode* y)`**
Splits a full child `y` at child index `i`. A new node `z` is created and receives the right half of `y`'s keys and child pointers. The median key at position `t-1` is promoted up into the calling node. The global `totalSplits` counter is incremented on every call.

**`traverse()`**
Performs an in-order walk of the tree, recursively visiting the left subtree before each key and the rightmost subtree after the last key. Always produces keys in sorted ascending order.

**`levelOrder(ostream& out)`**
traversal using a queue. Nodes at the same depth are printed on the same line. Output format:
```
Level 0: [20]
Level 1: [10] [30 40]
```

### Part 3: Input Parsing

`processOperation()` reads each line as a `string`, trims leading spaces, and checks for the keyword operations (`SAVE`, `RESTORE`, `REVERSE`) before falling through to single-character operations (`I`, `D`, `S`). The integer value is validated character by character — any line without a valid op or a valid integer is silently skipped with no output.

### Part 4: Console Output

After every operation the console prints the operation name and the current tree in level-order. Example:
```
=== Insert 20 ===
Level 0: [10 20]
```

### Part 5: File Logging

`logState()` is called after every operation and writes to `log.txt`:
```
=== Insert 20 ===
Splits so far: 0
Level 0: [10 20]
```

### Part 6: Visualization

The same `levelOrder()` function is used for console, `output.txt`, and `log.txt`. 

### Part 7: Edge Cases

**Duplicate keys** — `insert()` calls `root->contains(k)` before any insertion logic. If the key already exists it is silently rejected. 

**Full root** — when the root holds `2t-1 = 3` keys, `splitRoot()` is called before descending. It creates a new empty internal root, makes the old root its left child, then calls `splitChild()` to promote the median. 

### Part 8: Reverse Reconstruction (REVERSE keyword)

`reverse()` does the following:
1. Collects all keys in sorted order via in-order traversal into a raw array.
2. Snapshots the current level-order structure as a string using `treeToString()`.
3. Clears the tree and resets `totalSplits`.
4. Rebuilds by inserting keys from last to first (reverse sorted order).
5. Prints a side-by-side comparison of forward vs. reverse tree to console and `output.txt`.

### Part 9: Split Analysis

`totalSplits` is a global integer incremented inside `splitChild()` every time a node is split. 

### Part 11: SAVE / RESTORE

**SAVE** — serializes the tree to `snapshot.dat` using preorder traversal. Each node record contains: key count, all keys, a leaf flag (0 or 1), then recursively all children. 

**RESTORE** — clears the current tree and rebuilds it by reading `snapshot.dat` in the same preorder format. Prints the restored tree to console, `output.txt`, and `log.txt`.

**Why saving only keys is not enough** — keys alone carry no information about which keys belong to the same node, how nodes are grouped into levels, or which nodes are leaves vs. internal nodes. 

**How node types are distinguished** — each node record includes an explicit leaf flag written as `1` (leaf) or `0` (internal).

---

## Concept Questions (Part 10)

### 1. Why is a 2-3-4 Tree a special case of a B-Tree?

A B-Tree is parameterized by its minimum degree `t`. Each node holds between `t-1` and `2t-1` keys and has between `t` and `2t` children. A 2-3-4 Tree sets `t = 2`, so each node holds 1 to 3 keys and has 2 to 4 children — which is exactly where the name "2-3-4" comes from. 

### 2. Maximum number of children?

With `t = 2` a node holds at most `2t - 1 = 3` keys, giving at most `2t = 4` children.The maximum is **4 children** per node.

### 3. Explain the split operation

A split is triggered when a node is full (holds `2t - 1 = 3` keys). The procedure:

1. A new sibling node `z` is created.
2. The right `t - 1` keys and their child pointers are moved from the full node into `z`.
3. The median key at index `t - 1` is promoted up into the parent node.
4. The parent gains one new key and one new child pointer to `z`.

In this implementation splits are performed **proactively top-down**: before descending into any child, we check whether it is full and split it immediately. This guarantees the parent always has room to absorb the promoted median, so the tree stays balanced with a single downward pass. When the root itself is full, `splitRoot()` is called — a new empty root is created, the old root becomes its left child, and the split promotes the median into the new root. This is the only operation that increases tree height.

### 4. Complexity of insertion?

 **O(log n)**.

---

## Edge Case Analysis (Part 7)

### Duplicate keys

Duplicates are silently rejected. 


### Full root split

When the root holds 3 keys and a 4th insertion arrives, `splitRoot()` fires before descending. The median is promoted to a new root and height increases by 1:

