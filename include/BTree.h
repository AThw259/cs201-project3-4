// BTree.h
// A generic in-memory B-Tree (CLRS style, minimum degree t) that maps unique
// keys to values. Project 3 only requires insert and search; we also provide
// an in-order range search (used for name lookups), a level-order walk (used
// by the GUI's tree view) and an invariant checker (used by the tests).
//
// Every node except the root holds between t-1 and 2t-1 keys; an internal
// node with k keys has exactly k+1 children; all leaves sit at the same depth.
#pragma once

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <memory>
#include <queue>
#include <string>
#include <utility>
#include <vector>

template <typename Key, typename Value, int T = 3>
class BTree {
    static_assert(T >= 2, "B-Tree minimum degree t must be at least 2");

public:
    static constexpr int kMinDegree = T;
    static constexpr int kMaxKeys = 2 * T - 1;

    // One key/value pair stored inside a node.
    struct Entry {
        Key key;
        Value value;
    };

    BTree() : root_(new Node()) {}
    BTree(const BTree&) = delete;
    BTree& operator=(const BTree&) = delete;
    BTree(BTree&&) = default;
    BTree& operator=(BTree&&) = default;

    // Inserts key -> value. If the key already exists its value is replaced
    // and false is returned; otherwise the key is added and true is returned.
    bool insert(const Key& key, const Value& value) {
        if (Value* existing = search(key)) {
            *existing = value;
            return false;
        }
        // A full root is split first, which is the only way the tree grows taller.
        if (root_->full()) {
            std::unique_ptr<Node> newRoot(new Node());
            newRoot->leaf = false;
            newRoot->children.push_back(std::move(root_));
            root_ = std::move(newRoot);
            splitChild(*root_, 0);
        }
        insertNonFull(*root_, Entry{key, value});
        ++size_;
        return true;
    }

    // Returns a pointer to the value stored under key, or nullptr if absent.
    // If nodesVisited is given it receives the number of nodes examined,
    // which is at most height() and is shown in the GUI.
    Value* search(const Key& key, int* nodesVisited = nullptr) {
        return const_cast<Value*>(static_cast<const BTree*>(this)->search(key, nodesVisited));
    }

    const Value* search(const Key& key, int* nodesVisited = nullptr) const {
        int visited = 0;
        const Node* x = root_.get();
        while (x != nullptr) {
            ++visited;
            std::size_t i = lowerIndex(*x, key);
            if (i < x->entries.size() && !(key < x->entries[i].key)) {
                if (nodesVisited) *nodesVisited = visited;
                return &x->entries[i].value;
            }
            x = x->leaf ? nullptr : x->children[i].get();
        }
        if (nodesVisited) *nodesVisited = visited;
        return nullptr;
    }

    bool contains(const Key& key) const { return search(key) != nullptr; }

    // Calls fn(key, value) in sorted order for every key with lo <= key <= hi.
    // Subtrees that cannot contain keys in the range are skipped.
    template <typename Fn>
    void rangeSearch(const Key& lo, const Key& hi, Fn&& fn) const {
        rangeSearch(root_.get(), lo, hi, fn);
    }

    // Calls fn(key, value) for every entry in sorted order.
    template <typename Fn>
    void forEach(Fn&& fn) const {
        inOrder(root_.get(), fn);
    }

    // Breadth-first walk: calls fn(depth, entriesOfNode) for every node,
    // level by level from the root (depth 0) down to the leaves.
    template <typename Fn>
    void forEachNodeByLevel(Fn&& fn) const {
        std::queue<std::pair<const Node*, int>> pending;
        pending.push({root_.get(), 0});
        while (!pending.empty()) {
            const Node* x = pending.front().first;
            int depth = pending.front().second;
            pending.pop();
            fn(depth, x->entries);
            for (const auto& child : x->children) pending.push({child.get(), depth + 1});
        }
    }

    std::size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }

    // Number of levels (an empty tree is a single empty leaf: height 1).
    int height() const {
        int h = 1;
        for (const Node* x = root_.get(); !x->leaf; x = x->children.front().get()) ++h;
        return h;
    }

    std::size_t nodeCount() const { return countNodes(root_.get()); }

    // Verifies every B-Tree property. Returns false and fills *why on failure.
    bool validate(std::string* why = nullptr) const {
        int leafDepth = -1;
        std::size_t counted = 0;
        std::string reason;
        bool ok = validateNode(root_.get(), nullptr, nullptr, 0, true, leafDepth, counted, reason);
        if (ok && counted != size_) {
            ok = false;
            reason = "stored key count does not match size()";
        }
        if (!ok && why) *why = reason;
        return ok;
    }

private:
    struct Node {
        bool leaf = true;
        std::vector<Entry> entries;                    // sorted, at most 2t-1
        std::vector<std::unique_ptr<Node>> children;   // entries.size()+1 if internal

        Node() { entries.reserve(kMaxKeys); }
        bool full() const { return static_cast<int>(entries.size()) == kMaxKeys; }
    };

    std::unique_ptr<Node> root_;
    std::size_t size_ = 0;

    // Index of the first entry whose key is >= key (binary search in a node).
    static std::size_t lowerIndex(const Node& x, const Key& key) {
        auto it = std::lower_bound(x.entries.begin(), x.entries.end(), key,
                                   [](const Entry& e, const Key& k) { return e.key < k; });
        return static_cast<std::size_t>(it - x.entries.begin());
    }

    // Splits the full child x.children[i] around its median key. The median
    // moves up into x; the upper t-1 keys move into a new right sibling.
    static void splitChild(Node& x, std::size_t i) {
        Node& y = *x.children[i];
        std::unique_ptr<Node> z(new Node());
        z->leaf = y.leaf;

        z->entries.assign(std::make_move_iterator(y.entries.begin() + T),
                          std::make_move_iterator(y.entries.end()));
        Entry median = std::move(y.entries[T - 1]);
        y.entries.erase(y.entries.begin() + (T - 1), y.entries.end());

        if (!y.leaf) {
            z->children.assign(std::make_move_iterator(y.children.begin() + T),
                               std::make_move_iterator(y.children.end()));
            y.children.erase(y.children.begin() + T, y.children.end());
        }

        x.children.insert(x.children.begin() + static_cast<std::ptrdiff_t>(i) + 1, std::move(z));
        x.entries.insert(x.entries.begin() + static_cast<std::ptrdiff_t>(i), std::move(median));
    }

    // Walks down from a node that is known not to be full, splitting any full
    // child before stepping into it, so the leaf always has room for the key.
    static void insertNonFull(Node& start, Entry entry) {
        Node* x = &start;
        while (!x->leaf) {
            std::size_t i = lowerIndex(*x, entry.key);
            if (x->children[i]->full()) {
                splitChild(*x, i);
                if (x->entries[i].key < entry.key) ++i;
            }
            x = x->children[i].get();
        }
        std::size_t i = lowerIndex(*x, entry.key);
        x->entries.insert(x->entries.begin() + static_cast<std::ptrdiff_t>(i), std::move(entry));
    }

    template <typename Fn>
    static void rangeSearch(const Node* x, const Key& lo, const Key& hi, Fn& fn) {
        // Child i holds keys between entries[i-1] and entries[i], so we can
        // start at the first key >= lo and stop after the first key > hi.
        std::size_t i = lowerIndex(*x, lo);
        for (; i <= x->entries.size(); ++i) {
            if (!x->leaf) rangeSearch(x->children[i].get(), lo, hi, fn);
            if (i == x->entries.size() || hi < x->entries[i].key) break;
            fn(x->entries[i].key, x->entries[i].value);
        }
    }

    template <typename Fn>
    static void inOrder(const Node* x, Fn& fn) {
        for (std::size_t i = 0; i < x->entries.size(); ++i) {
            if (!x->leaf) inOrder(x->children[i].get(), fn);
            fn(x->entries[i].key, x->entries[i].value);
        }
        if (!x->leaf) inOrder(x->children.back().get(), fn);
    }

    static std::size_t countNodes(const Node* x) {
        std::size_t n = 1;
        for (const auto& c : x->children) n += countNodes(c.get());
        return n;
    }

    static bool validateNode(const Node* x, const Key* lo, const Key* hi, int depth, bool isRoot,
                             int& leafDepth, std::size_t& counted, std::string& why) {
        const int n = static_cast<int>(x->entries.size());
        if (n > kMaxKeys) { why = "node has more than 2t-1 keys"; return false; }
        if (!isRoot && n < T - 1) { why = "non-root node has fewer than t-1 keys"; return false; }
        for (int i = 0; i < n; ++i) {
            const Key& k = x->entries[i].key;
            if (i > 0 && !(x->entries[i - 1].key < k)) { why = "keys in a node are not strictly increasing"; return false; }
            if (lo && !(*lo < k)) { why = "key is not greater than its parent's separator"; return false; }
            if (hi && !(k < *hi)) { why = "key is not less than its parent's separator"; return false; }
        }
        counted += static_cast<std::size_t>(n);
        if (x->leaf) {
            if (!x->children.empty()) { why = "leaf has children"; return false; }
            if (leafDepth == -1) leafDepth = depth;
            if (depth != leafDepth) { why = "leaves are at different depths"; return false; }
            return true;
        }
        if (static_cast<int>(x->children.size()) != n + 1) { why = "internal node needs keys+1 children"; return false; }
        for (int i = 0; i <= n; ++i) {
            const Key* childLo = (i == 0) ? lo : &x->entries[i - 1].key;
            const Key* childHi = (i == n) ? hi : &x->entries[i].key;
            if (!validateNode(x->children[i].get(), childLo, childHi, depth + 1, false, leafDepth, counted, why))
                return false;
        }
        return true;
    }
};
