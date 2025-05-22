#include <algorithm>
#include <atomic>
#include <cmath>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <pathfinding.h>
#include <queue>
#include <stack>
#include <thread>
#include <vector>

using namespace std;

// node struct
Node::Node(int xval, int yval)
    : g(__DBL_MAX__), h(__DBL_MAX__), f(__DBL_MAX__), x(xval), y(yval),
      isBlocked(false), isChecked(false), isMarked(false), inPath(false),
      isStart(false), isEnd(false), parent(nullptr) {}

double Node::calculateh(int ex, int ey) {
  h = std::sqrt(std::pow(ex - x, 2) + std::pow(ey - y, 2));
  return h;
}

// priority queue
bool Compare::operator()(Node *a, Node *b) { return a->f > b->f; }

using pqueue = priority_queue<Node *, vector<Node *>, Compare>;

// class to manage the pathfing on a separate thread
LogicThread::LogicThread() : stop(false), taskReady(false), task(NULL) {
  worker = thread(&LogicThread::run, this);
}

LogicThread::~LogicThread() { stopThread(); }

void LogicThread::run() {
  while (true) {
    unique_lock<mutex> lock(taskMutex);
    taskCV.wait(lock, [&] { return taskReady || stop; });

    if (stop)
      break;

    if (taskReady) {
      lock.unlock();
      task();
      lock.lock();
      task = NULL;
      taskReady = false;
    }
  }
}

void LogicThread::schedule(std::function<void()> newTask) {
  {
    lock_guard<std::mutex> lock(taskMutex);
    task = move(newTask);
    taskReady = true;
  }
  taskCV.notify_one();
}

void LogicThread::stopThread() {
  {
    std::lock_guard<std::mutex> lock(taskMutex);
    stop = true;
  }
  taskCV.notify_one();
  if (worker.joinable())
    worker.join();
}

bool isValid(int x, int y, vector<vector<Node *>> grid) {
  int row = grid.size();
  int column = grid[0].size();

  if (x < 0 || x >= column)
    return false;
  if (y < 0 || y >= row)
    return false;

  return true;
}

void tracePath(Node *end) {
  Node *n = end;

  while (n->parent != nullptr) {
    n->inPath = true;
    n = n->parent;
  }

  n->inPath = true;
  n = NULL;
}

void bfs(pair<int, int> s, pair<int, int> e, vector<vector<Node *>> grid) {
  Node *start = grid[s.second][s.first];
  Node *end = grid[e.second][e.first];

  if (!isValid(start->x, start->y, grid) || !isValid(end->x, end->y, grid)) {
    cout << "invalid nodes" << endl;
    return;
  }

  start->isChecked = true;

  queue<Node *> neighbours;
  neighbours.push(start);
  while (!neighbours.empty()) {
    Node *currNode = neighbours.front();
    neighbours.pop();

    int x = currNode->x, y = currNode->y;
    if (x == end->x && y == end->y) {
      tracePath(end);
      return;
    }
    vector<pair<int, int>> directions = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

    for (auto [dx, dy] : directions) {
      if (!isValid(x + dx, y + dy, grid))
        continue;

      Node *n = grid[y + dy][x + dx];
      if (n->isBlocked || n->isChecked)
        continue;

      // lock_guard<mutex> lock(taskMutex);
      n->parent = currNode;
      n->isChecked = true;
      neighbours.push(n);
      this_thread::sleep_for(chrono::milliseconds(10));
    }
  }

  // return false;
}

void dijkstar(pair<int, int> s, pair<int, int> e, vector<vector<Node *>> grid) {
  Node *start = grid[s.second][s.first];
  Node *end = grid[e.second][e.first];

  if (!isValid(start->x, start->y, grid) || !isValid(end->x, end->y, grid)) {
    cout << "invalid nodes" << endl;
    return;
  }

  pqueue neighbours;

  start->g = 0;
  start->h = 0;
  start->f = 0;
  neighbours.push(start);

  while (!neighbours.empty()) {
    // lock_guard<mutex> lock(taskMutex);
    Node *currNode = neighbours.top();
    neighbours.pop();
    currNode->isChecked = true;

    int x = currNode->x, y = currNode->y;
    if (x == end->x && y == end->y) {
      tracePath(end);
      return;
    }
    vector<pair<int, int>> directions = {{-1, 0},  {1, 0},  {0, -1}, {0, 1},
                                         {-1, -1}, {-1, 1}, {1, -1}, {1, 1}};

    for (auto [dx, dy] : directions) {
      if (!isValid(x + dx, y + dy, grid))
        continue;

      Node *n = grid[y + dy][x + dx];
      if (n->isBlocked || n->isChecked)
        continue;

      double ng = currNode->g + sqrt(dx * dx + dy * dy);

      if (ng < n->g) {
        n->parent = currNode;
        n->g = ng;
        n->h = 0;
        n->f = ng;
        neighbours.push(n);
      }
      this_thread::sleep_for(chrono::milliseconds(10));
    }
  }

  // return false;
}

void astar(pair<int, int> s, pair<int, int> e, vector<vector<Node *>> grid) {
  Node *start = grid[s.second][s.first];
  Node *end = grid[e.second][e.first];

  if (!isValid(start->x, start->y, grid) || !isValid(end->x, end->y, grid)) {
    cout << "invalid nodes" << endl;
    return;
  }

  start->g = 0;
  start->calculateh(end->x, end->y);
  start->f = start->g + start->h;

  pqueue neighbours;
  neighbours.push(start);
  while (!neighbours.empty()) {
    // lock_guard<mutex> lock(taskMutex);
    Node *currNode = neighbours.top();
    neighbours.pop();
    currNode->isChecked = true;

    int x = currNode->x, y = currNode->y;
    if (x == end->x && y == end->y) {
      tracePath(end);
      return;
    }
    vector<pair<int, int>> directions = {{-1, 0},  {1, 0},  {0, -1}, {0, 1},
                                         {-1, -1}, {-1, 1}, {1, -1}, {1, 1}};

    for (auto [dx, dy] : directions) {
      if (!isValid(x + dx, y + dy, grid))
        continue;

      Node *n = grid[y + dy][x + dx];
      if (n->isBlocked || n->isChecked)
        continue;

      double ng = currNode->g + sqrt(dx * dx + dy * dy);
      double nh = n->calculateh(end->x, end->y);
      double nf = ng + nh;

      if (nf < n->f) {
        n->parent = currNode;
        n->g = ng;
        n->f = nf;
        neighbours.push(n);
      }
      this_thread::sleep_for(chrono::milliseconds(10));
    }
  }

  // return false;
}
