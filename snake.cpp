#include <ncurses.h>
#include <vector>
#include <cstdlib>
#include <unistd.h>
#include <string>
#include <tuple>
#include <memory>
#include <time.h>

struct Point
{
    int x, y;
    Point(int col, int row) : x(col), y(row) {}
};

void initGame()
{
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    refresh();
}

void renderGameWindow(int height, int width)
{
    clear();
    for (int i = 0; i < width; i++)
    {
        mvprintw(0, i, "#");
        mvprintw(height - 1, i, "#");
    }
    for (int i = 0; i < height; i++)
    {
        mvprintw(i, 0, "#");
        mvprintw(i, width - 1, "#");
    }
    refresh();
}

class Snake
{
public:
    int startX, startY;
    std::string direction;
    std::vector<std::unique_ptr<std::tuple<int, int>>> body;
    int keyUp, keyDown, keyLeft, keyRight;
    bool isAlive;

    Snake(int x, int y, std::string dir, int upKey, int downKey, int leftKey, int rightKey)
        : startX(x), startY(y), direction(dir), keyUp(upKey), keyDown(downKey), keyLeft(leftKey), keyRight(rightKey), isAlive(true)
    {
        body.push_back(std::make_unique<std::tuple<int, int>>(startX, startY));
    }

    void renderSnake()
    {
        if (isAlive)
        {
            int headX = std::get<0>(*body[0]);
            int headY = std::get<1>(*body[0]);
            mvprintw(headY, headX, "O");
            for (size_t i = 1; i < body.size(); ++i)
            {
                int x = std::get<0>(*body[i]);
                int y = std::get<1>(*body[i]);
                mvprintw(y, x, "o");
            }
        }
        refresh();
    }

    void changeDirection(int ch)
    {
        std::string newDirection = direction;
        if (ch == keyUp)
            newDirection = "up";
        else if (ch == keyDown)
            newDirection = "down";
        else if (ch == keyLeft)
            newDirection = "left";
        else if (ch == keyRight)
            newDirection = "right";
        if ((direction == "up" && newDirection != "down") ||
            (direction == "down" && newDirection != "up") ||
            (direction == "left" && newDirection != "right") ||
            (direction == "right" && newDirection != "left"))
        {
            direction = newDirection;
        }
    }

    void move()
    {
        if (!isAlive)
            return;
        int headX = std::get<0>(*body[0]);
        int headY = std::get<1>(*body[0]);
        if (direction == "up")
            headY -= 1;
        else if (direction == "down")
            headY += 1;
        else if (direction == "left")
            headX -= 1;
        else if (direction == "right")
            headX += 1;
        for (int i = body.size() - 1; i > 0; --i)
        {
            *body[i] = *body[i - 1];
        }
        *body[0] = std::make_tuple(headX, headY);
    }

    bool checkWallCollision(int width, int height)
    {
        int headX = std::get<0>(*body[0]);
        int headY = std::get<1>(*body[0]);
        if (headX <= 0 || headX >= width - 1 || headY <= 0 || headY >= height - 1)
        {
            isAlive = false;
            return true;
        }
        return false;
    }

    bool checkSelfCollision()
    {
        int headX = std::get<0>(*body[0]);
        int headY = std::get<1>(*body[0]);
        for (size_t i = 1; i < body.size(); ++i)
        {
            if (std::get<0>(*body[i]) == headX && std::get<1>(*body[i]) == headY)
            {
                isAlive = false;
                return true;
            }
        }
        return false;
    }

    bool eatFood(int foodX, int foodY)
    {
        int headX = std::get<0>(*body[0]);
        int headY = std::get<1>(*body[0]);
        return headX == foodX && headY == foodY;
    }

    void grow()
    {
        body.push_back(std::make_unique<std::tuple<int, int>>(*body.back()));
    }

    int getLength() const
    {
        return body.size();
    }
};

struct Food
{
    int x, y;
    Food(int maxX, int maxY) { spawn(maxX, maxY); }
    void spawn(int maxX, int maxY)
    {
        x = rand() % (maxX - 2) + 1;
        y = rand() % (maxY - 2) + 1;
    }
    void render() const
    {
        mvprintw(y, x, "F");
        refresh();
    }
};

void checkSnakeCollisions(std::vector<Snake> &snakes)
{
    std::vector<bool> collisionFlags(snakes.size(), false);
    for (size_t i = 0; i < snakes.size(); ++i)
    {
        if (!snakes[i].isAlive)
            continue;
        int headX = std::get<0>(*snakes[i].body[0]);
        int headY = std::get<1>(*snakes[i].body[0]);
        for (size_t j = 0; j < snakes.size(); ++j)
        {
            if (i == j)
                continue;
            for (const auto &part : snakes[j].body)
            {
                int partX = std::get<0>(*part);
                int partY = std::get<1>(*part);
                if (headX == partX && headY == partY)
                {
                    collisionFlags[i] = true;
                    break;
                }
            }
        }
    }
    for (size_t i = 0; i < snakes.size(); ++i)
    {
        if (collisionFlags[i])
            snakes[i].isAlive = false;
    }
}

void showScoreboard(const std::vector<Snake> &snakes)
{
    clear();
    mvprintw(0, 0, "Scoreboard:");
    for (size_t i = 0; i < snakes.size(); ++i)
    {
        mvprintw(i + 1, 0, "Player %zu: %d", i, snakes[i].getLength());
    }
    refresh();
    usleep(1500000);
}

int main()
{
    srand(time(0));
    int height = 20;
    int width = 40;
    initGame();
    Food food(width, height);
    std::vector<Snake> snakes;
    Snake snake_one(5, 10, "right", 'w', 's', 'a', 'd');
    snakes.push_back(std::move(snake_one));
    Snake snake_two(35, 10, "left", KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT);
    snakes.push_back(std::move(snake_two));
    while (true)
    {
        renderGameWindow(height, width);
        int ch = getch();
        bool anySnakeAlive = false;
        for (auto &snake : snakes)
        {
            snake.changeDirection(ch);
            snake.move();
            snake.renderSnake();
            if (snake.isAlive)
            {
                anySnakeAlive = true;
                if (snake.checkWallCollision(width, height) || snake.checkSelfCollision())
                {
                    snake.isAlive = false;
                }
                if (snake.eatFood(food.x, food.y))
                {
                    snake.grow();
                    food.spawn(width, height);
                }
            }
        }
        checkSnakeCollisions(snakes);
        if (!anySnakeAlive)
            break;
        food.render();
        usleep(100000);
    }
    showScoreboard(snakes);
    endwin();
    return 0;
}
