
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include "Node.h"
#include "GVC.h"

enum OP {
    INSERT,
    REMOVE
};

struct nodeOp {
    Node* first;
    Node* second;
    Node* n;
    int v;
    OP op;
};

class TxAbortException : public std::exception {};