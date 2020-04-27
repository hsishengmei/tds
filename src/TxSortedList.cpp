#include "TxSortedList.h"

#include <thread>
#include <chrono>

void Transaction::TxBegin() {
    if (_debug) printf("TxBegin\n");
    readVersion = gvc.read();
    readSet.clear();
    writeSet.clear();
    lockedNodes.clear();
}

void Transaction::TxCommit(TxSortedList& txsl) {
    if (_debug) printf("TxCommit\n");
    writeVersion = gvc.addAndFetch();
    validateReadSet();
    update(txsl);
    releaseLockedNodes();
}

void Transaction::TxAbort() {
    if (_debug) printf("TxAbort\n");
    releaseLockedNodes();
    readSet.clear();
    writeSet.clear();
    lockedNodes.clear();
    std::this_thread::sleep_for(std::chrono::microseconds(100));
    throw TxAbortException();
}

void Transaction::acquireWriteSetLock(TxSortedList& txsl) {
    for (auto& op : writeSet) {
        if (op.op == INSERT) {
            if (!op.first->try_lock()) TxAbort();
            lockedNodes.push_back(op.first);
            if (!op.second->try_lock()) TxAbort();
            lockedNodes.push_back(op.second);
        }
        else { // REMOVE
            if (!op.n->try_lock()) TxAbort();
            lockedNodes.push_back(op.n);
            if (!op.first->try_lock()) TxAbort();
            lockedNodes.push_back(op.first);
            if (!op.second->try_lock()) TxAbort();
            lockedNodes.push_back(op.second);
        }
    }
}
void Transaction::validateReadSet() {
    for (auto& n : readSet) {
        if (n->version > readVersion) TxAbort();
    }
}

void Transaction::update(TxSortedList& txsl) {
    for (auto& op : writeSet) {
        if (op.op == INSERT) {
            Node* cur = txsl._find_less_or_equal_starting_at_wo_lock(op.v, op.first);
            txsl._insert(op.v, cur, cur->next);
            cur->next->version = writeVersion;
            writeNodes.push_back(cur);
            writeNodes.push_back(cur->next);
            writeNodes.push_back(cur->next->next);
        }
        else { // REMOVE
            Node* cur = op.n;
            while (cur->val == op.v && cur != &txsl.head) {
                if (!cur->deleted) {
                    cur->deleted = true;
                    deleteNodes.push_back(cur);
                    break;
                }
                cur = cur->prev;
            }
            writeNodes.push_back(cur->prev);
            writeNodes.push_back(cur->next);
            txsl._remove(cur);
        }
    }

    for (auto& n : writeNodes) {
        n->version = writeVersion;
    }

    for (auto& n : deleteNodes) {
        txsl._remove(n);
    }
}
void Transaction::releaseLockedNodes() {
    for (auto& n : lockedNodes) {
        n->unlock();
    }
}


TxSortedList::TxSortedList() : size(0) {
    if (_debug) printf("constructor\n");
    head.next = &tail;
    tail.prev = &head;
}
// ~TxSortedList() {
//     delete head;
//     delete tail;
// }
void TxSortedList::insert(int v, Transaction& tx) {
    if (_debug) printf("txsl insert %d\n", v);
    Node* cur = _find_less_or_equal(v, tx);
    tx.writeSet.push_back(nodeOp{cur,cur->next,nullptr,v,INSERT});
}

void TxSortedList::_insert(int v, Node* prev, Node* next) {
    Node* newNode = new Node;
    newNode->val = v;
    prev->next->prev = newNode;
    newNode->next = prev->next;
    newNode->prev = prev;
    prev->next = newNode;
    ++size;
}

void TxSortedList::remove(int v, Transaction& tx) {
    if (_debug) printf("txsl remove %d\n", v);
    Node* cur = _find_equal(v, tx);
    if (cur) {
        tx.writeSet.push_back(nodeOp{cur->prev,cur->next,cur,0,REMOVE});

    }
}
void TxSortedList::_remove(Node* cur) {
    cur->prev->next = cur->next;
    cur->next->prev = cur->prev;
    delete cur;
    --size;
}

Node* TxSortedList::find(int v, Transaction& tx) {
    if (_debug) printf("txsl find %d\n", v);
    return _find_equal(v, tx);
}

Node* TxSortedList::_find_less_or_equal(int v, Transaction& tx) {
    return _find_less_or_equal_starting_at(v, &head, tx);
}

Node* TxSortedList::_find_less_or_equal_starting_at(int v, Node* cur, Transaction& tx) {
    if (!cur) cur = &head;
    
    if (!cur->try_lock()) tx.TxAbort();
    tx.lockedNodes.push_back(cur);
    if (cur->version > tx.readVersion) tx.TxAbort();
    
    while (cur->next != &tail) {
        if (!cur->next->try_lock()) tx.TxAbort();
        tx.lockedNodes.push_back(cur->next);
        if (cur->next->version > tx.readVersion) tx.TxAbort();
        if (cur->next->val > v) break; 
        cur->unlock();
        cur = cur->next;
    }
    cur->unlock();
    tx.readSet.push_back(cur);
    return cur;
}

Node* TxSortedList::_find_less_or_equal_starting_at_wo_lock(int v, Node* cur) {
    while (cur->next != &tail) {
        if (cur->next->val > v) break; 
        cur = cur->next;
    }
    return cur;
}

Node* TxSortedList::_find_equal(int v, Transaction& tx) {
    return _find_equal_starting_at(v, &head, tx);
}

Node* TxSortedList::_find_equal_starting_at(int v, Node* cur, Transaction& tx) {
    if (!cur->try_lock()) tx.TxAbort();
    tx.lockedNodes.push_back(cur);
    if (cur->version > tx.readVersion) tx.TxAbort();
    
    while (cur->next != &tail) {
        if (!cur->next->try_lock()) tx.TxAbort();
        tx.lockedNodes.push_back(cur->next);
        if (cur->next->version > tx.readVersion) tx.TxAbort();
        if (cur->next->val > v) break; 
        cur->unlock();
        cur = cur->next;
    }
    if (cur->val != v) {
        cur->unlock();
        return nullptr;
    }
    cur->unlock();
    tx.readSet.push_back(cur);
    return cur;
}