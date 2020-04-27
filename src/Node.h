#pragma once
#include <mutex>

class Node {
public:
    Node() : val(0), version(0), prev(nullptr), next(nullptr), 
            deleted(false), locked(false) {}
    // static bool compNode(const Node& n1, const Node& n2) {
    //     return n1.val < n2.val;
    // }
    bool try_lock() {
        // if (_debug) printf("try_lock %d\n", val);
        // if (locked) return false;
        // locked = true;
        return mutex.try_lock();
    }
    // bool locked() {
    //     // if (_debug) printf("%d locked?\n", val);
    //     if (try_lock()) {
    //         unlock();
    //         // if (_debug) printf("false\n");
    //         return false;
    //     }
    //     // if (_debug) printf("true\n");
    //     return true;
    // }
    void unlock() {
        // if (_debug) printf("unlock %d\n", val);
        mutex.unlock();
        // locked = false;
    }
    int val;
    int version;
    Node* prev;
    Node* next;
    std::recursive_mutex mutex;
    bool deleted;
    bool locked;
};