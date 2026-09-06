#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <stdexcept>
#include <cstdlib>
#include <ctime>
#include <sstream>
using namespace std;

// ====== Base Class ======
class GameObject {
public:
    virtual void display() const = 0;
    virtual ~GameObject() {}
};

// ====== Cell ======
enum class CellState { Closed, Opened, Flagged };

class Cell : public GameObject {
public:
    int x, y;
    bool mine;
    int around;
    CellState state;

    Cell(int x1 = 0, int y1 = 0) {
        x = x1; y = y1;
        mine = false;
        around = 0;
        state = CellState::Closed;
    }

    void open() {
        if (state == CellState::Opened)
            throw runtime_error("Kletka uzhe otkryta");
        if (state == CellState::Flagged)
            throw runtime_error("Snachala snimite flazhok");
        state = CellState::Opened;
    }

    void toggleFlag() {
        if (state == CellState::Opened)
            throw runtime_error("Nelzya stavit flag na otkrytuyu kletku");
        state = (state == CellState::Flagged) ? CellState::Closed : CellState::Flagged;
    }

    char getSymbol(bool showMines = false) const {
        if (state == CellState::Flagged) return 'F';
        if (state == CellState::Closed) return '.';
        if (mine) return showMines ? '*' : '*';
        if (around == 0) return ' ';
        return char('0' + around);
    }

    void display() const override {
        cout << getSymbol();
    }
};

// ====== Board ======
class Board : public GameObject {
public:
    int w, h, mines;
    vector<vector<Cell>> grid;
    bool minesPlaced = false;

    Board(int width = 9, int height = 9, int mineCount = 10) {
        w = width; h = height; mines = mineCount;
        grid.assign(h, vector<Cell>(w));
        for (int y = 0; y < h; y++)
            for (int x = 0; x < w; x++)
                grid[y][x] = Cell(x, y);
    }

    bool valid(int x, int y) const {
        return x >= 0 && x < w && y >= 0 && y < h;
    }

    void checkCoord(int x, int y) const {
        if (!valid(x, y)) throw out_of_range("Koordinaty vne polya");
    }

    void placeMines(int sx, int sy) {
        srand(static_cast<unsigned int>(time(nullptr)));
        int count = 0;
        while (count < mines) {
            int x = rand() % w;
            int y = rand() % h;
            if (abs(x - sx) <= 1 && abs(y - sy) <= 1) continue;
            if (!grid[y][x].mine) {
                grid[y][x].mine = true;
                count++;
            }
        }
        for (int y = 0; y < h; y++)
            for (int x = 0; x < w; x++) {
                if (grid[y][x].mine) continue;
                int c = 0;
                for (int dy = -1; dy <= 1; dy++)
                    for (int dx = -1; dx <= 1; dx++) {
                        int nx = x + dx, ny = y + dy;
                        if (valid(nx, ny) && grid[ny][nx].mine) c++;
                    }
                grid[y][x].around = c;
            }
        minesPlaced = true;
    }

    bool openCell(int x, int y, bool first) {
        checkCoord(x, y);
        if (!minesPlaced && first) placeMines(x, y);

        Cell& c = grid[y][x];
        c.open();
        if (c.mine) return false;

        queue<pair<int, int>> q;
        q.push({ x,y });
        while (!q.empty()) {
            auto p = q.front(); q.pop();
            int cx = p.first;
            int cy = p.second;
            if (grid[cy][cx].around == 0) {
                for (int dy = -1; dy <= 1; dy++)
                    for (int dx = -1; dx <= 1; dx++) {
                        int nx = cx + dx, ny = cy + dy;
                        if (valid(nx, ny)) {
                            Cell& n = grid[ny][nx];
                            if (n.state == CellState::Closed && !n.mine) {
                                n.state = CellState::Opened;
                                if (n.around == 0) q.push({ nx,ny });
                            }
                        }
                    }
            }
        }
        return true;
    }

    void show(bool showMines = false) const {
        cout << "   ";
        for (int x = 0; x < w; x++) cout << (x + 1 < 10 ? " " : "") << x + 1 << " ";
        cout << "\n";
        for (int y = 0; y < h; y++) {
            cout << (y + 1 < 10 ? " " : "") << y + 1 << " ";
            for (int x = 0; x < w; x++)
                cout << " " << grid[y][x].getSymbol(showMines) << " ";
            cout << "\n";
        }
    }

    void display() const override {
        show();
    }
};

// ====== Player ======
class Player : public GameObject {
public:
    int flags;
    int maxFlags;

    Player(int m = 10) { flags = 0; maxFlags = m; }

    void flag(Board& b, int x, int y) {
        b.checkCoord(x, y);
        Cell& c = b.grid[y][x];
        if (c.state == CellState::Flagged) {
            c.toggleFlag();
            flags--;
        }
        else {
            if (flags >= maxFlags)
                throw runtime_error("Dostignut limit flazhkow");
            c.toggleFlag();
            flags++;
        }
    }

    void display() const override {
        cout << "Flazhkow: " << flags << "/" << maxFlags << "\n";
    }
};

// ====== Game ======
class Game : public GameObject {
public:
    Board board;
    Player player;
    bool firstMove = true;
    bool win = false, lose = false;

    void start() {
        bool running = true;

        while (running) {
            cout << "\nVyberite uroven: 1 - legkiy, 2 - slozhniy, q - vykhod\n> ";
            string s; getline(cin, s);
            if (s == "1") { board = Board(9, 9, 10); player = Player(10); }
            else if (s == "2") { board = Board(16, 16, 40); player = Player(40); }
            else if (s == "q") return;
            else continue;

            firstMove = true; win = false; lose = false;

            bool gameRunning = true;
            while (gameRunning) {
                system("cls");

                board.show(lose);
                player.display();

                if (lose) {
                    cout << "Vy proigrali!\n";
                    gameRunning = false;
                    continue;
                }
                if (win) {
                    cout << "Vy pobedili!\n";
                    gameRunning = false;
                    continue;
                }

                cout << "\nKomandy: o x y (otkryt), f x y (flag), q (v menyu)\n> ";
                string cmd; getline(cin, cmd);
                try {
                    process(cmd);
                }
                catch (const runtime_error& e) {
                    if (string(e.what()) == "Vykhod v menyu") {
                        gameRunning = false;
                    }
                    else {
                        cout << "Oshibka igry: " << e.what() << "\n";
                        cin.get();
                    }
                }
                catch (const exception& e) {
                    cout << "Oshibka: " << e.what() << "\n";
                    cin.get();
                }
            }
        }
    }

    void process(const string& line) {
        if (line.empty()) throw invalid_argument("Pustaya komanda");
        string c; int x, y;
        stringstream ss(line);
        ss >> c;
        if (c == "q") throw runtime_error("Vykhod v menyu");
        else if (c == "o") {
            if (!(ss >> x >> y)) throw invalid_argument("Nevernye koordinaty");
            x--; y--;
            bool safe = board.openCell(x, y, firstMove);
            firstMove = false;
            if (!safe) { lose = true; return; }
            checkWin();
        }
        else if (c == "f") {
            if (!(ss >> x >> y)) throw invalid_argument("Nevernye koordinaty");
            x--; y--;
            player.flag(board, x, y);
        }
        else throw invalid_argument("Neizvestnaya komanda");
    }

    void checkWin() {
        int opened = 0;
        for (auto& r : board.grid)
            for (auto& c : r)
                if (!c.mine && c.state == CellState::Opened)
                    opened++;
        if (opened == board.w * board.h - board.mines) win = true;
    }

    void display() const override {
        board.display();
        player.display();
        cout << (win ? "Win\n" : lose ? "Lose\n" : "Idet igra...\n");
    }
};

int main() {
    Game game;
    game.start();
    return 0;
}
