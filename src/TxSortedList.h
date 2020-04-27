#pragma once

#include "Utils.h"

extern GVC gvc;
extern bool _debug;

class TxSortedList;

class Transaction {
public:
    void TxBegin();

    void TxCommit(TxSortedList& txsl);

    void TxAbort();

    void acquireWriteSetLock(TxSortedList& txsl);
    void validateReadSet();

    void update(TxSortedList& txsl);

    void releaseLockedNodes();

    int readVersion;
    int writeVersion;
    std::vector<Node*> readSet;
    std::vector<nodeOp> writeSet;
    std::vector<Node*> lockedNodes;

    std::vector<Node*> deleteNodes;
    std::vector<Node*> writeNodes;
};

// template<class Node, class int>
class TxSortedList {
public:
    TxSortedList();
    // ~TxSortedList() {
    //     delete head;
    //     delete tail;
    // }
    void insert(int v, Transaction& tx);

    void _insert(int v, Node* prev, Node* next);

    void remove(int v, Transaction& tx);
    void _remove(Node* cur);

    Node* find(int v, Transaction& tx);

    Node* _find_less_or_equal(int v, Transaction& tx);
    Node* _find_less_or_equal_starting_at(int v, Node* cur, Transaction& tx);

    Node* _find_less_or_equal_starting_at_wo_lock(int v, Node* cur);

    Node* _find_equal(int v, Transaction& tx);

    Node* _find_equal_starting_at(int v, Node* cur, Transaction& tx);

    Node head;
    Node tail;
    int size;
};