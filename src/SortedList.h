#pragma once

#include <iostream>

template<class Node, class Value>
class SortedList {
public:
    SortedList() : size(0) {
        head = new Node;
        tail = new Node;
        head->next = tail;
        tail->prev = head;
    }
    ~SortedList() {
        delete head;
        delete tail;
    }
    void insert(Value v) {
        Node* cur = _find_less_or_equal(v);
        Node* newNode = new Node;
        newNode->val = v;
        cur->next->prev = newNode;
        newNode->next = cur->next;
        newNode->prev = cur;
        cur->next = newNode;
        ++size;
    }
    void remove(Value v) {
        Node* cur = _find_equal(v);
        if (cur) {
            cur->prev->next = cur->next;
            cur->next->prev = cur->prev;
            delete cur;
            --size;
        }
    }
    Node* find(Value v) {
        return _find_equal(v);
    }
    
    Node* _find_less_or_equal(Value v) {
        Node* cur = head;
        
        while (cur->next != tail) {
            if (cur->next->val > v) break; 
            cur = cur->next;
        }
        return cur;
    }
    Node* _find_equal(Value v) {
        Node* cur = head;
        
        while (cur->next != tail) {
            if (cur->next->val > v) break; 
            cur = cur->next;
        }
        if (cur->val != v) {
            return nullptr;
        }
        return cur;
    }
    Node* head;
    Node* tail;
    int size;
};