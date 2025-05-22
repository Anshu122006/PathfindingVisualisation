#pragma once

#include <algorithm>
#include <atomic>
#include <cmath>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <stack>
#include <thread>
#include <vector>

using namespace std;

struct Node {
  double g, h, f;
  int x, y;
  bool isBlocked, isChecked, isMarked, inPath, isStart, isEnd;
  Node *parent;

  Node(int xval, int yval);
  double calculateh(int ex, int ey);
};

struct Compare {
  bool operator()(Node *a, Node *b);
};

class LogicThread {
private:
  condition_variable taskCV;
  mutex taskMutex;
  atomic<bool> stop, taskReady;
  function<void()> task;
  thread worker;

  void run();

public:
  LogicThread();
  ~LogicThread();
  void schedule(std::function<void()> newTask);
  void stopThread();
};

// class pqueue {
//   std::vector<Node *> q;

// public:
//   void push(Node *n);
//   Node *top();
//   void pop();
//   bool empty();
// };

bool isValid(int x, int y, std::vector<std::vector<Node *>> grid);
void tracePath(Node *end);
void bfs(std::pair<int, int> s, std::pair<int, int> e,
         std::vector<std::vector<Node *>> grid);
void dijkstar(std::pair<int, int> s, std::pair<int, int> e,
              std::vector<std::vector<Node *>> grid);
void astar(std::pair<int, int> s, std::pair<int, int> e,
           std::vector<std::vector<Node *>> grid);
void backgroundTask();
