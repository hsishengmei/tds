#include "TX.h"

void Transaction::TxBegin() {
    readVersion = gvc.read();
    readSet.clear();
    writeSet.clear();
    lockedNodes.clear();
}

void Transaction::TxCommit(TxSortedList& txsl) {
    writeVersion = gvc.addAndFetch();
    validateReadSet(txsl);
    update(txsl);
    releaseLockedNodes(txsl);
}

void Transaction::TxCommit(TxQueue& q) {
    update(q);
    if (push_lock_q) q.head.unlock();
    if (pop_lock_q) q.tail.unlock();
}

void Transaction::TxAbort(TxSortedList& txsl) {
    releaseLockedNodes(txsl);
    readSet.clear();
    writeSet.clear();
    lockedNodes.clear();
    throw TxAbortException();
}
void Transaction::TxAbort(TxQueue& q) {
    rollbackQ(q);
    if (push_lock_q) q.head.unlock();
    if (pop_lock_q) q.tail.unlock();
    throw TxAbortException();
}

void Transaction::acquireWriteSetLock(TxSortedList& txsl) {
    for (auto& op : writeSet) {
        if (op.op == INSERT) {
            if (!op.first->try_lock()) TxAbort(txsl);
            lockedNodes.push_back(op.first);
            if (!op.second->try_lock()) TxAbort(txsl);
            lockedNodes.push_back(op.second);
        }
        else { // REMOVE
            if (!op.n->try_lock()) TxAbort(txsl);
            lockedNodes.push_back(op.n);
            if (!op.first->try_lock()) TxAbort(txsl);
            lockedNodes.push_back(op.first);
            if (!op.second->try_lock()) TxAbort(txsl);
            lockedNodes.push_back(op.second);
        }
    }
}

void Transaction::validateReadSet(TxSortedList& txsl) {
    for (auto& n : readSet) {
        if (n->version > readVersion) TxAbort(txsl);
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
void Transaction::releaseLockedNodes(TxSortedList& txsl) {
    for (auto& n : lockedNodes) {
        n->unlock();
    }
}

void Transaction::update(TxQueue& q) {
    for (auto& i : qPushes) {
        q._push(i);
    }
}

void Transaction::rollbackQ(TxQueue& q) {
    for (auto& i : qPops) {
        q._unpop(i);
    }
}

TxSortedList::TxSortedList() : size(0) {
    head.next = &tail;
    tail.prev = &head;
}

void TxSortedList::insert(int v, Transaction& tx) {
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
    return _find_equal(v, tx);
}

Node* TxSortedList::_find_less_or_equal(int v, Transaction& tx) {
    return _find_less_or_equal_starting_at(v, &head, tx);
}

Node* TxSortedList::_find_less_or_equal_starting_at(int v, Node* cur, Transaction& tx) {
    if (!cur) cur = &head;
    
    // if (!cur->try_lock()) tx.TxAbort(*this);
    // tx.lockedNodes.push_back(cur);
    if (cur->version > tx.readVersion) tx.TxAbort(*this);
    
    while (cur->next != &tail) {
        // if (!cur->next->try_lock()) tx.TxAbort(*this);
        // tx.lockedNodes.push_back(cur->next);
        if (cur->next->version > tx.readVersion) tx.TxAbort(*this);
        if (cur->next->val > v) break; 
        // cur->unlock();
        cur = cur->next;
    }
    // cur->unlock();
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
    // if (!cur->try_lock()) tx.TxAbort(*this);
    // tx.lockedNodes.push_back(cur);
    if (cur->version > tx.readVersion) tx.TxAbort(*this);
    
    while (cur->next != &tail) {
        // if (!cur->next->try_lock()) tx.TxAbort(*this);
        tx.lockedNodes.push_back(cur->next);
        if (cur->next->version > tx.readVersion) tx.TxAbort(*this);
        if (cur->next->val > v) break; 
        // cur->unlock();
        cur = cur->next;
    }
    if (cur->val != v) {
        // cur->unlock();
        return nullptr;
    }
    // cur->unlock();
    tx.readSet.push_back(cur);
    return cur;
}



TxQueue::TxQueue() {
    head.next = &tail;
    tail.prev = &head;
    size = 0;
}

void TxQueue::push(int i, Transaction& tx) {
    if (!tx.push_lock_q) {
        if (!head.try_lock()) tx.TxAbort(*this);
        tx.push_lock_q = true;
    }
    tx.qPushes.push_back(i);
}
void TxQueue::_push(int i) {
    Node* n = new Node;
    Node* next = head.next;
    n->val = i;
    n->prev = &head;
    n->next = next;
    head.next = n;
    next->prev = n;
    ++size;
}
void TxQueue::pop(Transaction& tx) {
    if (!tx.pop_lock_q) {
        if (!tail.try_lock()) tx.TxAbort(*this);
        tx.pop_lock_q = true;
    }
    tx.qPops.push_back(_front());
    _pop();
}
void TxQueue::_pop() {
    Node* prev = tail.prev->prev;
    delete tail.prev;
    tail.prev = prev;
    prev->next = &tail;
    --size;
}
void TxQueue::_unpop(Node* n) {
    Node* prev = tail.prev;
    n->prev = prev;
    n->next = &tail;
    tail.prev = n;
    prev->next = n;
    ++size;
}
Node* TxQueue::front(Transaction& tx) {
    if (!tx.pop_lock_q) {
        if (!tail.try_lock()) tx.TxAbort(*this);
        tx.pop_lock_q = true;
    }
    return _front();
}
Node* TxQueue::_front() {
    if (size == 0) return nullptr;
    return head.next;
}