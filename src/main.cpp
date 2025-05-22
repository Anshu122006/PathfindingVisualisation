#include <SDL.h>
#include <cmath>
#include <iostream>
#include <mutex>
#include <pathfinding.h>
#include <queue>
#include <thread>
#include <vector>

using namespace std;

int WINDOW_HEIGHT, WINDOW_WIDTH;
int cellHeight = 40, cellWidth = 40;
int row, column;
pair<int, int> startId = {0, 0}, endId = {0, 0};

void initGrid();
void drawGrid(SDL_Renderer *renderer);
void drawGridObjects(SDL_Renderer *renderer);
pair<double, double> position(int x, int y);
pair<int, int> index(double px, double py);
void clearPath();

vector<vector<Node *>> grid;

int main(int argc, char *argv[]) {
  row = 20;
  column = 20;
  WINDOW_HEIGHT = cellHeight * row;
  WINDOW_WIDTH = cellWidth * column;

  LogicThread *logicThread = nullptr;

  SDL_Window *window = nullptr;
  // SDL_Surface *windowSurface = nullptr;
  SDL_Renderer *renderer = nullptr;

  //
  // initializing video
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    cerr << "SDL_Init Error: " << SDL_GetError() << endl;
    return 1;
  }

  //
  // creating window
  window =
      SDL_CreateWindow("GRID", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                       WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
  if (!window) {
    cerr << "SDL_CreateWindow Error: " << SDL_GetError() << endl;
    SDL_Quit();
    return 1;
  }
  // windowSurface = SDL_GetWindowSurface(window);

  //
  // creating renderer
  renderer = SDL_CreateRenderer(
      window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

  //
  // main running loop
  bool isRunning = true;
  bool isDragging = false;
  bool placeWall = true;
  bool setStart = false;
  bool setEnd = false;
  bool isPathfinding = false;
  SDL_Event ev;

  initGrid();

  while (isRunning) {
    while (SDL_PollEvent(&ev) != 0) {
      if (ev.type == SDL_QUIT) {
        isRunning = false;
      }

      if (ev.type == SDL_KEYDOWN) {
        switch (ev.key.keysym.sym) {
        case SDLK_z:
          clearPath();
          logicThread->stopThread();
          delete logicThread;
          isPathfinding = false;
          break;
        case SDLK_s:
          setStart = true;
          break;
        case SDLK_e:
          setEnd = true;
          break;
        case SDLK_b:
          logicThread = new LogicThread();
          logicThread->schedule([] { bfs(startId, endId, grid); });
          isPathfinding = true;
          break;
        case SDLK_d:
          logicThread = new LogicThread();
          logicThread->schedule([] { dijkstar(startId, endId, grid); });
          isPathfinding = true;
          break;
        case SDLK_a:
          logicThread = new LogicThread();
          logicThread->schedule([] { astar(startId, endId, grid); });
          isPathfinding = true;
          break;
        }
      }

      if (!isPathfinding) {
        switch (ev.type) {
        case SDL_MOUSEBUTTONDOWN: {
          pair<int, int> nodeIndex = index(ev.button.x, ev.button.y);
          int x = nodeIndex.first, y = nodeIndex.second;

          if (!isValid(x, y, grid))
            break;

          if (setStart) {
            Node *s = grid[startId.second][startId.first];
            s->isStart = false;
            startId = nodeIndex;
            grid[y][x]->isStart = true;
            setStart = false;
            break;
          } else if (setEnd) {
            Node *e = grid[endId.second][endId.first];
            e->isEnd = false;
            endId = nodeIndex;
            grid[y][x]->isEnd = true;
            setEnd = false;
            break;
          }

          if (ev.button.button == SDL_BUTTON_LEFT) {
            isDragging = true;
            placeWall = true;
          } else if (ev.button.button == SDL_BUTTON_RIGHT) {
            isDragging = true;
            placeWall = false;
          }

          Node *n = grid[y][x];
          if (placeWall) {
            if (!n->isBlocked)
              n->isBlocked = true;
          } else {
            if (n->isBlocked)
              n->isBlocked = false;
          }

        } break;

        case SDL_MOUSEMOTION:
          if (isDragging) {
            pair<int, int> nodeIndex = index(ev.motion.x, ev.motion.y);
            int x = nodeIndex.first, y = nodeIndex.second;

            if (!isValid(x, y, grid))
              break;

            Node *n = grid[y][x];
            if (placeWall) {
              if (!n->isBlocked)
                n->isBlocked = true;
            } else {
              if (n->isBlocked)
                n->isBlocked = false;
            }
          }
          break;
        case SDL_MOUSEBUTTONUP:
          isDragging = false;
          break;
        }
      }
    }

    drawGridObjects(renderer);
    drawGrid(renderer);

    SDL_RenderPresent(renderer);
    SDL_UpdateWindowSurface(window);
  }

  logicThread->~LogicThread();

  SDL_DestroyRenderer(renderer);
  renderer = nullptr;

  SDL_DestroyWindow(window);
  window = nullptr;
  // windowSurface = nullptr;

  SDL_Quit();
  return 0;
}

//
// initialize grid with nodes
void initGrid() {
  grid = vector<vector<Node *>>(row, vector<Node *>(column, nullptr));

  for (int y = 0; y < row; y++) {
    for (int x = 0; x < column; x++) {
      Node *n = new Node(x, y);
      grid[y][x] = n;
    }
  }
}

//
// pontion and index conversions
pair<double, double> position(int x, int y) {
  double px = x * cellWidth;
  double py = y * cellHeight;

  return make_pair(px, py);
}

pair<int, int> index(double px, double py) {
  int x = floor(px / cellWidth);
  int y = floor(py / cellHeight);

  return make_pair(x, y);
}

//
// draw grid
void drawGrid(SDL_Renderer *renderer) {
  SDL_SetRenderDrawColor(renderer, 120, 120, 120, 255);

  // hirzontal lines
  for (int x = 0; x < WINDOW_WIDTH; x += cellWidth) {
    SDL_RenderDrawLine(renderer, x, 0, x, WINDOW_HEIGHT);
  }

  // vetrical lines
  for (int y = 0; y < WINDOW_WIDTH; y += cellHeight) {
    SDL_RenderDrawLine(renderer, 0, y, WINDOW_WIDTH, y);
  }
}

//
// draw grid objects
void drawGridObjects(SDL_Renderer *renderer) {
  for (int y = 0; y < row; y++) {
    for (int x = 0; x < column; x++) {
      Node *n = grid[y][x];
      if (n->isBlocked) {
        SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
      } else if (n->isStart) {
        SDL_SetRenderDrawColor(renderer, 200, 200, 0, 255);
      } else if (n->isEnd) {
        SDL_SetRenderDrawColor(renderer, 0, 200, 200, 255);
      } else if (n->inPath) {
        SDL_SetRenderDrawColor(renderer, 0, 200, 0, 255);
      } else if (n->isChecked) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 200, 255);
      } else {
        SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
      }
      SDL_Rect rect = {
          static_cast<int>(cellWidth) * x, static_cast<int>(cellHeight) * y,
          static_cast<int>(cellWidth), static_cast<int>(cellHeight)};
      SDL_RenderFillRect(renderer, &rect);
    }
  }
}

void clearPath() {
  for (int i = 0; i < row; i++) {
    for (int j = 0; j < column; j++) {
      Node *n = grid[i][j];
      n->isChecked = false;
      n->isMarked = false;
      n->inPath = false;
      n->isStart = false;
      n->isEnd = false;
      n->g = __DBL_MAX__;
      n->h = __DBL_MAX__;
      n->f = __DBL_MAX__;
      n->parent = nullptr;
    }
  }
}
