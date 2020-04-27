#pragma once
#include <mutex>

class Node {
public:
    Node() : val(0), version(0), prev(nullptr), next(nullptr), 
            deleted(false), locked(false) {}
    bool try_lock() {
        return mutex.try_lock();
    }
    void unlock() {
        mutex.unlock();
    }
    int val;
    int version;
    Node* prev;
    Node* next;
    std::recursive_mutex mutex;
    bool deleted;
    bool locked;
};