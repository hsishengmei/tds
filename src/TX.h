#pragma once

#include "Utils.h"

extern GVC gvc;

class TxSortedList;
class TxQueue;

class Transaction {
public:
    void TxBegin();

    void TxCommit(TxSortedList&);
    void TxCommit(TxQueue&);

    void TxAbort(TxSortedList&);
    void TxAbort(TxQueue&);

    void rollbackQ(TxQueue&);

    void acquireWriteSetLock(TxSortedList&);
    void validateReadSet(TxSortedList&);

    void update(TxSortedList&);
    void update(TxQueue&);

    void releaseLockedNodes(TxSortedList&);

    int readVersion;
    int writeVersion;
    
    // support for sorted lists
    std::vector<Node*> readSet;
    std::vector<nodeOp> writeSet;
    std::vector<Node*> lockedNodes;
    std::vector<Node*> deleteNodes;
    std::vector<Node*> writeNodes;

    // support for queues
    std::vector<Node*> qPops;
    std::vector<int> qPushes;
    bool push_lock_q = false;
    bool pop_lock_q = false;
};

class TxSortedList {
public:
    TxSortedList();

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


class TxQueue {
public:
    TxQueue();
    void push(int, Transaction&);
    void _push(int);
    void pop(Transaction&);
    void _pop();
    void _unpop(Node*);
    Node* front(Transaction&);
    Node* _front();

    Node head;
    Node tail;
    int size;
};