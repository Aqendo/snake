#include <signal.h>
#include <sys/ioctl.h>
#include <termios.h>

#include <algorithm>
#include <chrono>
#include <deque>
#include <iostream>
#include <random>
#include <thread>
using namespace std;

// height of the area
const int SC_HEIGHT = 10;
// width of the area
const int SC_WIDTH = 15;
// amount of attempts to spawn an apple
// until program crashes with winning message
const int MAX_SPAWN_COUNT = 10;
const int SLEEP_TIMEOUT_MS = 150;
const char* clearscreen = "\033[2J\033[1;1H";

mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());

enum class Direction { up, right, down, left };
Direction direction = Direction::right;

// initial fruit coords
int fruitI = rng() % SC_HEIGHT;
int fruitJ = rng() % SC_WIDTH;

// IO functions to catch key press without enter
void enable_raw_mode() {
  termios term;
  tcgetattr(0, &term);
  term.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(0, TCSANOW, &term);
}

void disable_raw_mode() {
  termios term;
  tcgetattr(0, &term);
  term.c_lflag |= ICANON | ECHO;
  tcsetattr(0, TCSANOW, &term);
}

bool kbhit() {
  int byteswaiting;
  ioctl(0, FIONREAD, &byteswaiting);
  return byteswaiting > 0;
}

// initial snake
deque<pair<int, int>> snake = {{2, 2}};

bool isOverlaping(int i, int j) {
  for (pair<int, int>& p : snake) {
    if (p.first == i && p.second == j) {
      return true;
    }
  }
  return false;
}

void gameOver(string reason, bool clearScreen) {
  disable_raw_mode();
  tcflush(0, TCIFLUSH);
  cout << '\n';
  if (clearScreen) {
    cout << clearscreen;
  }
  cout << reason << endl;
  exit(0);
}

void logic() {
  int next_i = 0;
  int next_j = 0;
  switch (direction) {
    case Direction::up:
      next_i = snake.back().first - 1;
      next_j = snake.back().second;
      break;
    case Direction::left:
      next_i = snake.back().first;
      next_j = snake.back().second - 1;
      break;
    case Direction::down:
      next_i = snake.back().first + 1;
      next_j = snake.back().second;
      break;
    case Direction::right:
      next_i = snake.back().first;
      next_j = snake.back().second + 1;

      break;
  }
  if (next_i != fruitI || next_j != fruitJ) {
    snake.pop_front();
  } else {
    int count_spawn = 0;
    do {
      count_spawn++;
      fruitI = rng() % SC_HEIGHT;
      fruitJ = rng() % SC_WIDTH;
    } while (isOverlaping(fruitI, fruitJ) && count_spawn < MAX_SPAWN_COUNT);
    if (count_spawn == MAX_SPAWN_COUNT) {
      gameOver("Nowhere to spawn fruits! You win, I guess..", false);
    }
  }
  if (next_i < 0) {
    next_i = SC_HEIGHT - 1;
  } else if (next_i >= SC_HEIGHT) {
    next_i = 0;
  }
  if (next_j < 0) {
    next_j = SC_WIDTH - 1;
  }
  if (next_j >= SC_WIDTH) {
    next_j = 0;
  }

  if (isOverlaping(next_i, next_j)) {
    gameOver("Game over! You crashed into yourself!", false);
  }
  snake.push_back({next_i, next_j});
}

void draw() {
  for (int i = 0; i < SC_WIDTH + 2; ++i) {
    cout << '#';
  }
  cout << '\n';
  bool headPrinted = false;
  for (int i = 0; i < SC_HEIGHT; ++i) {
    cout << '#';
    for (int j = 0; j < SC_WIDTH; ++j) {
      // Encountered an apple
      if (i == fruitI && j == fruitJ) {
        cout << 'A';
        continue;
      }
      bool foundSnakesPart = false;
      for (pair<int, int>& p : snake) {
        if (p.first == i && p.second == j) {
          foundSnakesPart = true;
          if (p == snake.back()) {
            // snake's head
            cout << 'O';
            headPrinted = true;
            continue;
          }
          // snake's tail
          cout << '*';
        }
      }
      if (!foundSnakesPart) cout << ' ';
    }
    cout << '#';
    cout << '\n';
  }
  for (int i = 0; i < SC_WIDTH + 2; ++i) {
    cout << '#';
  }
  cout.flush();
  if (kbhit()) {
    // change direction
    char c;
    cin >> c;
    if (c == 27) {
      // arrow keys
      char _;
      cin >> _ >> c;
    }
    switch (c) {
      case 65:
      case 'w':
        if (direction == Direction::down) break;
        direction = Direction::up;
        break;
      case 68:
      case 'a':
        if (direction == Direction::right) break;
        direction = Direction::left;
        break;
      case 66:
      case 's':
        if (direction == Direction::up) break;
        direction = Direction::down;
        break;
      case 67:
      case 'd':
        if (direction == Direction::left) break;
        direction = Direction::right;
        break;
    }
  }
  logic();
}

void loop() {
  cout << clearscreen;
  draw();
  std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMEOUT_MS));
}

void handle_exit(sig_atomic_t s) {
  gameOver("CTRL-C was detected. Shutting down!", true);
}

int main() {
  signal(SIGINT, handle_exit);
  enable_raw_mode();
  while (true) {
    loop();
  }
  return 0;
}
