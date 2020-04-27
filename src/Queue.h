#include "Node.h"

class Queue {
public:
    Queue() {
        head.next = &tail;
        tail.prev = &head;
        size = 0;
    }
    void push(int i) {
        Node* n = new Node;
        Node* next = head.next;
        n->val = i;
        n->prev = &head;
        n->next = next;
        head.next = n;
        next->prev = n;
        ++size;
    }
    void pop() {
        Node* prev = tail.prev->prev;
        delete tail.prev;
        tail.prev = prev;
        prev->next = &tail;
        --size;
    }
    Node* front() {
        if (size == 0) return nullptr;
        return head.next;
    }

    Node head;
    Node tail;
    int size;
};