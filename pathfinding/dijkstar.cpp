#include <iostream>
#include <queue>
#include <stack>
using namespace std;

struct Node {
  int g = __INT_MAX__;
  Node *parent = nullptr;

  Node(int val) { g = val; };
};

class PQueue {
  queue<Node *> q;

public:
  void add(Node *n) {
    stack<Node *> temp;
    Node *qn = q.front();

    while (!q.empty()) {
      if ((qn->g) >= (n->g)) {
        q.push(n);
      } else {
        q.pop();
        temp.push(qn);
      }
    }

    while (!temp.empty()) {
      Node *sn = temp.top();
      q.push(sn);
    }
  }

  void showQueue() {
    while (!q.empty()) {
      int val = (q.front())->g;
      cout << val << endl;
    }
  }
};

int main() {
  Node *q1 = new Node(3);
  Node *q2 = new Node(9);
  Node *q3 = new Node(8);
  Node *q4 = new Node(2);
  Node *q5 = new Node(34);
  Node *q6 = new Node(6);

  PQueue pqueue;
  pqueue.add(q1);
  pqueue.add(q2);
  pqueue.add(q3);
  pqueue.add(q4);
  pqueue.add(q5);
  pqueue.add(q6);

  pqueue.showQueue();

  return 0;
}
