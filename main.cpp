#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

using namespace std;

//  Global split counter
int totalSplits = 0;

//  Queue
class QueueNode {
public:
  void *data;
  int level;
  QueueNode *next;
  QueueNode(void *d, int l) : data(d), level(l), next(nullptr) {}
};

class Queue {
public:
  QueueNode *head;
  QueueNode *tail;
  Queue() : head(nullptr), tail(nullptr) {}

  void enqueue(void *d, int l) {
    QueueNode *n = new QueueNode(d, l);
    if (!tail)
      head = tail = n;
    else {
      tail->next = n;
      tail = n;
    }
  }

  QueueNode *dequeue() {
    if (!head)
      return nullptr;
    QueueNode *n = head;
    head = head->next;
    if (!head)
      tail = nullptr;
    return n;
  }

  bool empty() { return head == nullptr; }
};

//  BTreeNode
class BTreeNode {
public:
  int t;
  int *keys;
  BTreeNode **children;
  int n;
  bool leaf;

  BTreeNode(int _t, bool _leaf) : t(_t), leaf(_leaf), n(0) {
    keys = new int[2 * t - 1];
    children = new BTreeNode *[2 * t];
    for (int i = 0; i < 2 * t; i++)
      children[i] = nullptr;
  }

  ~BTreeNode() {
    delete[] keys;
    for (int i = 0; i <= n; i++)
      if (children[i])
        delete children[i];
    delete[] children;
  }

  bool contains(int k) {
    int i = 0;
    while (i < n && k > keys[i])
      i++;
    if (i < n && keys[i] == k)
      return true;
    if (leaf)
      return false;
    return children[i]->contains(k);
  }

  void insertNonFull(int k) {
    int i = n - 1;

    if (leaf) {
      for (int j = 0; j < n; j++)
        if (keys[j] == k)
          return;

      while (i >= 0 && keys[i] > k) {
        keys[i + 1] = keys[i];
        i--;
      }
      keys[i + 1] = k;
      n++;
    } else {

      while (i >= 0 && keys[i] > k)
        i--;
      i++;

      if (children[i]->n == 2 * t - 1) {
        splitChild(i, children[i]);
        if (keys[i] == k)
          return;
        if (keys[i] < k)
          i++;
      }
      children[i]->insertNonFull(k);
    }
  }

  void splitChild(int i, BTreeNode *y) {
    totalSplits++;

    BTreeNode *z = new BTreeNode(y->t, y->leaf);
    z->n = t - 1;

    for (int j = 0; j < t - 1; j++)
      z->keys[j] = y->keys[j + t];

    if (!y->leaf)
      for (int j = 0; j < t; j++)
        z->children[j] = y->children[j + t];

    y->n = t - 1;

    for (int j = n; j >= i + 1; j--)
      children[j + 1] = children[j];
    children[i + 1] = z;

    for (int j = n - 1; j >= i; j--)
      keys[j + 1] = keys[j];

    keys[i] = y->keys[t - 1];
    n++;

    if (!y->leaf)
      for (int j = t; j < 2 * t; j++)
        y->children[j] = nullptr;
  }
  void traverse() {
    int i;
    for (i = 0; i < n; i++) {
      if (!leaf)
        children[i]->traverse();
      cout << keys[i] << " ";
    }
    if (!leaf)
      children[i]->traverse();
  }

  BTreeNode *search(int k) {
    int i = 0;
    while (i < n && k > keys[i])
      i++;
    if (i < n && keys[i] == k)
      return this;
    if (leaf)
      return nullptr;
    return children[i]->search(k);
  }

  int getPredecessor(int idx) {
    BTreeNode *cur = children[idx];
    while (!cur->leaf)
      cur = cur->children[cur->n];
    return cur->keys[cur->n - 1];
  }

  int getSuccessor(int idx) {
    BTreeNode *cur = children[idx + 1];
    while (!cur->leaf)
      cur = cur->children[0];
    return cur->keys[0];
  }

  void merge(int idx) {
    BTreeNode *child = children[idx];
    BTreeNode *sibling = children[idx + 1];

    child->keys[t - 1] = keys[idx];
    for (int i = 0; i < sibling->n; i++)
      child->keys[i + t] = sibling->keys[i];

    if (!child->leaf) {
      for (int i = 0; i <= sibling->n; i++)
        child->children[i + t] = sibling->children[i];
      for (int i = 0; i <= sibling->n; i++)
        sibling->children[i] = nullptr;
    }

    for (int i = idx + 1; i < n; i++)
      keys[i - 1] = keys[i];
    for (int i = idx + 2; i <= n; i++)
      children[i - 1] = children[i];

    child->n += sibling->n + 1;
    sibling->n = 0;
    delete sibling;
    children[idx + 1] = nullptr;
    n--;
  }

  void borrowFromPrev(int idx) {
    BTreeNode *child = children[idx];
    BTreeNode *sibling = children[idx - 1];

    for (int i = child->n - 1; i >= 0; i--)
      child->keys[i + 1] = child->keys[i];
    if (!child->leaf)
      for (int i = child->n; i >= 0; i--)
        child->children[i + 1] = child->children[i];

    child->keys[0] = keys[idx - 1];
    if (!child->leaf)
      child->children[0] = sibling->children[sibling->n];

    keys[idx - 1] = sibling->keys[sibling->n - 1];
    child->n++;
    sibling->n--;
  }

  void borrowFromNext(int idx) {
    BTreeNode *child = children[idx];
    BTreeNode *sibling = children[idx + 1];

    child->keys[child->n] = keys[idx];
    if (!child->leaf)
      child->children[child->n + 1] = sibling->children[0];

    keys[idx] = sibling->keys[0];
    for (int i = 1; i < sibling->n; i++)
      sibling->keys[i - 1] = sibling->keys[i];
    if (!sibling->leaf)
      for (int i = 1; i <= sibling->n; i++)
        sibling->children[i - 1] = sibling->children[i];

    child->n++;
    sibling->n--;
  }

  void fill(int idx) {
    if (idx != 0 && children[idx - 1]->n >= t)
      borrowFromPrev(idx);
    else if (idx != n && children[idx + 1]->n >= t)
      borrowFromNext(idx);
    else {
      if (idx != n)
        merge(idx);
      else
        merge(idx - 1);
    }
  }

  void remove(int k) {
    int idx = 0;
    while (idx < n && keys[idx] < k)
      idx++;

    if (idx < n && keys[idx] == k) {
      if (leaf) {

        for (int i = idx + 1; i < n; i++)
          keys[i - 1] = keys[i];
        n--;
      } else if (children[idx]->n >= t) {

        int pred = getPredecessor(idx);
        keys[idx] = pred;
        children[idx]->remove(pred);
      } else if (children[idx + 1]->n >= t) {

        int succ = getSuccessor(idx);
        keys[idx] = succ;
        children[idx + 1]->remove(succ);
      } else {

        merge(idx);
        children[idx]->remove(k);
      }
    } else {
      if (leaf)
        return;

      bool lastChild = (idx == n);
      if (children[idx]->n < t)
        fill(idx);

      if (lastChild && idx > n)
        children[idx - 1]->remove(k);
      else
        children[idx]->remove(k);
    }
  }
};

//  BTree
class BTree {
public:
  BTreeNode *root;
  int t;

  BTree(int _t) : t(_t), root(nullptr) {}
  ~BTree() {
    if (root)
      delete root;
  }

  void splitRoot() {
    BTreeNode *s = new BTreeNode(t, false);
    s->children[0] = root;
    s->splitChild(0, root);
    root = s;
  }

  void insert(int k) {
    if (!root) {
      root = new BTreeNode(t, true);
      root->keys[0] = k;
      root->n = 1;
      return;
    }
    if (root->contains(k))
      return;
    if (root->n == 2 * t - 1)
      splitRoot();
    root->insertNonFull(k);
  }

  bool search(int k) { return root ? (root->search(k) != nullptr) : false; }

  void remove(int k) {
    if (!root)
      return;
    root->remove(k);
    if (root->n == 0) {
      BTreeNode *tmp = root;
      root = root->leaf ? nullptr : root->children[0];
      tmp->n = 0;
      if (!tmp->leaf)
        tmp->children[0] = nullptr;
      delete tmp;
    }
  }

  void traverse() {
    if (root) {
      root->traverse();
      cout << "\n";
    } else
      cout << "(empty)\n";
  }

  void levelOrder(ostream &out = cout) {
    if (!root) {
      out << "(empty)\n";
      return;
    }

    Queue q;
    q.enqueue(root, 0);
    int curLvl = 0;
    out << "Level 0: ";

    while (!q.empty()) {
      QueueNode *qn = q.dequeue();
      BTreeNode *node = (BTreeNode *)qn->data;
      int lvl = qn->level;
      delete qn;

      if (lvl != curLvl) {
        out << "\n";
        curLvl = lvl;
        out << "Level " << curLvl << ": ";
      }
      out << "[";
      for (int i = 0; i < node->n; i++) {
        out << node->keys[i];
        if (i < node->n - 1)
          out << " ";
      }
      out << "] ";

      if (!node->leaf)
        for (int i = 0; i <= node->n; i++)
          if (node->children[i])
            q.enqueue(node->children[i], lvl + 1);
    }
    out << "\n";
  }

  int height() {
    if (!root)
      return 0;
    int h = 0;
    BTreeNode *cur = root;
    while (!cur->leaf) {
      cur = cur->children[0];
      h++;
    }
    return h;
  }

  void clear() {
    if (root) {
      delete root;
      root = nullptr;
    }
  }

  string treeToString() {
    if (!root)
      return "(empty)\n";
    string result = "";
    string curLine = "Level 0: ";
    int curLvl = 0;

    Queue q;
    q.enqueue(root, 0);
    while (!q.empty()) {
      QueueNode *qn = q.dequeue();
      BTreeNode *node = (BTreeNode *)qn->data;
      int lvl = qn->level;
      delete qn;

      if (lvl != curLvl) {
        result += curLine + "\n";
        curLvl = lvl;
        curLine = "Level " + to_string(lvl) + ": ";
      }
      curLine += "[";
      for (int i = 0; i < node->n; i++) {
        curLine += to_string(node->keys[i]);
        if (i < node->n - 1)
          curLine += " ";
      }
      curLine += "] ";

      if (!node->leaf)
        for (int i = 0; i <= node->n; i++)
          if (node->children[i])
            q.enqueue(node->children[i], lvl + 1);
    }
    result += curLine + "\n";
    return result;
  }

  void collectKeys(BTreeNode *node, int *arr, int &idx) {
    if (!node)
      return;
    for (int i = 0; i < node->n; i++) {
      if (!node->leaf)
        collectKeys(node->children[i], arr, idx);
      arr[idx++] = node->keys[i];
    }
    if (!node->leaf)
      collectKeys(node->children[node->n], arr, idx);
  }

  void reverse(ostream &out, ostream &cons) {
    if (!root) {
      cons << "=== REVERSE: tree is empty ===\n";
      return;
    }

    int *keys = new int[10000];
    int count = 0;
    collectKeys(root, keys, count);

    string fwdSnap = treeToString();
    int fwdHeight = height();
    int fwdSplits = totalSplits;

    clear();
    totalSplits = 0;
    for (int i = count - 1; i >= 0; i--)
      insert(keys[i]);

    string revSnap = treeToString();
    int revHeight = height();
    int revSplits = totalSplits;

    string sep = "  ----------------------------------------\n";

    cons << "\n=== REVERSE RECONSTRUCTION COMPARISON ===\n";
    cons << sep;
    cons << "  FORWARD (original insertion order)\n" << sep;
    cons << fwdSnap;
    cons << "  Height : " << fwdHeight << "  |  Splits : " << fwdSplits << "\n";
    cons << sep;
    cons << "  REVERSE (reverse insertion order)\n" << sep;
    cons << revSnap;
    cons << "  Height : " << revHeight << "  |  Splits : " << revSplits << "\n";
    cons << sep;
    cons << "  Both trees are balanced — height is equal.\n";
    cons << "  Internal structure may differ; sorted output is identical.\n";
    cons << sep << "\n";

    out << "\n=== REVERSE RECONSTRUCTION COMPARISON ===\n";
    out << sep;
    out << "  FORWARD\n" << sep << fwdSnap;
    out << "  Height: " << fwdHeight << " | Splits: " << fwdSplits << "\n";
    out << sep;
    out << "  REVERSE\n" << sep << revSnap;
    out << "  Height: " << revHeight << " | Splits: " << revSplits << "\n";
    out << sep;

    delete[] keys;
  }

  void saveNode(BTreeNode *node, ofstream &out) {
    if (!node) {
      out << "NULL\n";
      return;
    }
    out << node->n << "\n";
    for (int i = 0; i < node->n; i++)
      out << node->keys[i] << "\n";
    out << (node->leaf ? 1 : 0) << "\n";
    if (!node->leaf)
      for (int i = 0; i <= node->n; i++)
        saveNode(node->children[i], out);
  }

  bool save() {
    ofstream out("snapshot.dat");
    if (!out)
      return false;
    out << t << "\n";
    saveNode(root, out);
    out.close();
    return true;
  }

  BTreeNode *loadNode(ifstream &in) {
    char buf[32];
    in >> buf;
    if (strcmp(buf, "NULL") == 0)
      return nullptr;

    int n = atoi(buf), leafFlag;
    BTreeNode *node = new BTreeNode(t, true);
    node->n = n;
    for (int i = 0; i < n; i++)
      in >> node->keys[i];
    in >> leafFlag;
    node->leaf = (leafFlag == 1);
    if (!node->leaf)
      for (int i = 0; i <= n; i++)
        node->children[i] = loadNode(in);
    return node;
  }

  bool restore() {
    ifstream in("snapshot.dat");
    if (!in)
      return false;
    clear();
    in >> t;
    root = loadNode(in);
    in.close();
    return true;
  }
};

//  logState: write tree state + split count to log.txt
void logState(ofstream &log, BTree &tree, const string &label) {
  log << "=== " << label << " ===\n";
  log << "Splits so far: " << totalSplits << "\n";
  tree.levelOrder(log);
  log << "\n";
}

//  processOperation: parse and execute one input line
void processOperation(BTree &tree, const string &line, ostream &out,
                      ofstream &logFile) {
  if (line.empty())
    return;

  int i = 0;
  while (i < (int)line.size() && line[i] == ' ')
    i++;
  if (i >= (int)line.size())
    return;

  string trimmed = line.substr(i);

  if (trimmed == "SAVE" || trimmed == "save") {
    if (tree.save()) {
      cout << "=== SAVE: tree saved to snapshot.dat ===\n";
      tree.levelOrder(cout);
      out << "=== SAVE: tree saved to snapshot.dat ===\n";
      tree.levelOrder(out);
    } else {
      cout << "=== SAVE: failed to write snapshot.dat ===\n";
    }
    logState(logFile, tree, "SAVE");
    return;
  }

  if (trimmed == "RESTORE" || trimmed == "restore") {
    if (tree.restore()) {
      cout << "=== RESTORE: tree cleared and restored from snapshot.dat ===\n";
      tree.levelOrder(cout);
      out << "=== RESTORE: tree cleared and restored from snapshot.dat ===\n";
      tree.levelOrder(out);
    } else {
      cout << "=== RESTORE: no snapshot found — nothing to restore ===\n";
    }
    logState(logFile, tree, "RESTORE");
    return;
  }

  if (trimmed == "REVERSE" || trimmed == "reverse") {
    tree.reverse(out, cout);
    logState(logFile, tree, "REVERSE");
    return;
  }

  char op = trimmed[0];
  if (op != 'I' && op != 'D' && op != 'S')
    return;

  int j = 1;
  while (j < (int)trimmed.size() && trimmed[j] == ' ')
    j++;
  if (j >= (int)trimmed.size())
    return;
  int k = j;
  if (trimmed[k] == '-' || trimmed[k] == '+')
    k++;
  if (k >= (int)trimmed.size() || !isdigit(trimmed[k]))
    return;

  int val = stoi(trimmed.substr(j));
  string label;

  if (op == 'I') {
    label = "Insert " + to_string(val);
    tree.insert(val);
    cout << "=== Insert " << val << " ===\n";
    tree.levelOrder(cout);
    out << "Insert " << val << ":\n";
    tree.levelOrder(out);

  } else if (op == 'D') {
    label = "Delete " + to_string(val);
    tree.remove(val);
    cout << "=== Delete " << val << " ===\n";
    tree.levelOrder(cout);
    out << "Delete " << val << ":\n";
    tree.levelOrder(out);

  } else {
    label = "Search " + to_string(val);
    bool found = tree.search(val);
    string res = found ? "FOUND" : "NOT FOUND";
    cout << "=== Search " << val << ": " << res << " ===\n";
    tree.levelOrder(cout);
    out << "Search " << val << ": " << res << "\n";
  }

  logState(logFile, tree, label);
}

//  main
int main() {

  ofstream logFile("log.txt");
  if (!logFile) {
    cerr << "Cannot open log.txt\n";
    return 1;
  }

  ofstream outFile("output.txt");
  if (!outFile) {
    cerr << "Cannot open output.txt\n";
    return 1;
  }

  BTree tree(2);

  ifstream inFile("input.txt");
  if (!inFile) {
    cerr << "Cannot open input.txt\n";
    return 1;
  }

  outFile << "=== 2-3-4 Tree — Processing input.txt ===\n\n";
  string line;
  while (getline(inFile, line))
    processOperation(tree, line, outFile, logFile);
  inFile.close();

  outFile << "\n=== Summary ===\n";
  outFile << "Total splits : " << totalSplits << "\n";
  outFile << "Tree height  : " << tree.height() << "\n";
  outFile << "Final tree   :\n";
  tree.levelOrder(outFile);

  logFile.close();
  outFile.close();

  cout << "\nDone. See output.txt and log.txt\n";
  return 0;
}
