#include <ncurses.h>
#include <vector>
#include <cstdlib>
#include <unistd.h>
#include <string>
#include <tuple>
#include <memory>
#include <time.h>
#include <unordered_map>
#include <cmath>
#include <ctime>
#include <algorithm>
#include <fstream>  // For file input and output
#include <sstream>  // For string stream processing
#include <iostream> // For basic input and output (if needed for debugging)

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

class Snake
{
public:
    int startX, startY;
    std::string direction;
    std::vector<std::unique_ptr<std::tuple<int, int>>> body;
    bool isAlive;
    bool isCpu;

    Snake(int x, int y, std::string dir, bool cpuControl = false)
        : startX(x), startY(y), direction(dir), isAlive(true), isCpu(cpuControl)
    {
        body.push_back(std::make_unique<std::tuple<int, int>>(startX, startY));
    }

    virtual void changeDirection(int ch, std::vector<Snake *> snakes, Food food, int width, int height) = 0;

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

class HumanSnake : public Snake
{
public:
    int keyUp, keyDown, keyLeft, keyRight;

    HumanSnake(int x, int y, std::string dir, int upKey, int downKey, int leftKey, int rightKey)
        : Snake(x, y, dir), keyUp(upKey), keyDown(downKey), keyLeft(leftKey), keyRight(rightKey) {}

    void changeDirection(int ch, std::vector<Snake *> snakes, Food food, int width, int height) override
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
};

class RandomSnake : public Snake
{
public:
    RandomSnake(int x, int y, std::string dir)
        : Snake(x, y, dir, true) {}

    void changeDirection(int ch, std::vector<Snake *> snakes, Food food, int width, int height) override
    {
        int randomDirection = rand() % 4;
        std::string newDirection = direction;

        switch (randomDirection)
        {
        case 0:
            newDirection = "up";
            break;
        case 1:
            newDirection = "down";
            break;
        case 2:
            newDirection = "left";
            break;
        case 3:
            newDirection = "right";
            break;
        }

        if ((direction == "up" && newDirection != "down") ||
            (direction == "down" && newDirection != "up") ||
            (direction == "left" && newDirection != "right") ||
            (direction == "right" && newDirection != "left"))
        {
            direction = newDirection;
        }
    }
};

class SmartSnake : public Snake
{
public:
    SmartSnake(int x, int y, std::string dir)
        : Snake(x, y, dir, true) {}

    void changeDirection(int ch, std::vector<Snake *> snakes, Food food, int width, int height) override
    {
        int headX = std::get<0>(*body[0]);
        int headY = std::get<1>(*body[0]);
        int foodX = food.x;
        int foodY = food.y;

        std::string newDirection = direction;

        // Compute the optimal direction to approach the food
        if (headX < foodX && direction != "left")
        {
            newDirection = "right";
        }
        else if (headX > foodX && direction != "right")
        {
            newDirection = "left";
        }
        else if (headY < foodY && direction != "up")
        {
            newDirection = "down";
        }
        else if (headY > foodY && direction != "down")
        {
            newDirection = "up";
        }

        // Ensure the snake does not reverse direction
        if ((direction == "up" && newDirection != "down") ||
            (direction == "down" && newDirection != "up") ||
            (direction == "left" && newDirection != "right") ||
            (direction == "right" && newDirection != "left"))
        {
            direction = newDirection;
        }
    }
};



class QLearningSnake : public Snake
{
public:
    std::unordered_map<std::string, std::vector<double>> QTable; // Q-value table
    double alpha = 0.3;                                          // Learning rate. the higher the faster overwrite old knowledge
    double gamma = 0.3;                                          // Discount factor. the higher the more future orientation
    double epsilon = 0.4;                                        // Exploration rate: the higher the more exploration
    std::vector<std::string> actions = {"up", "down", "left", "right"};

    QLearningSnake(int x, int y, std::string dir)
        : Snake(x, y, dir)
    {
        srand(time(0));
    }

    void changeDirection(int ch, std::vector<Snake *> snakes, Food food, int width, int height) override
    {
        std::string state = getStateRepresentation(snakes, food, width, height);
        std::string action = chooseAction(state);

        if (action == "up" && direction != "down")
        {
            direction = "up";
        }
        else if (action == "down" && direction != "up")
        {
            direction = "down";
        }
        else if (action == "left" && direction != "right")
        {
            direction = "left";
        }
        else if (action == "right" && direction != "left")
        {
            direction = "right";
        }

        // Perform the action, receive the reward, and update Q-values
        int reward = performActionAndGetReward(snakes, food, width, height);
        std::string nextState = getStateRepresentation(snakes, food, width, height);
        updateQValue(state, action, reward, nextState);
    }

    // Method to save QTable to a custom text file
    void saveQTableToFile(const std::string &filename)
    {
        std::ofstream file(filename);
        if (!file.is_open())
            return;

        for (const auto &entry : QTable)
        {
            file << entry.first << " "; // State as string
            for (double qValue : entry.second)
            {
                file << qValue << " "; // Q-values
            }
            file << "\n"; // New line for each state
        }
        file.close();
    }

    // Method to load QTable from a custom text file
    void loadQTableFromFile(const std::string &filename)
    {
        std::ifstream file(filename);
        if (!file.is_open())
            return;

        std::string line;
        while (std::getline(file, line))
        {
            std::istringstream iss(line);
            std::string state;
            iss >> state; // Read the state

            std::vector<double> qValues;
            double qValue;
            while (iss >> qValue)
            {
                qValues.push_back(qValue); // Read Q-values
            }
            QTable[state] = qValues; // Store in QTable
        }
        file.close();
    }

private:
    std::string getStateRepresentation(std::vector<Snake *> snakes, Food food, int width, int height)
    {
        // Example state: relative position of the food
        int headX = std::get<0>(*body[0]);
        int headY = std::get<1>(*body[0]);
        int foodDX = food.x - headX;
        int foodDY = food.y - headY;
        return std::to_string(foodDX) + "," + std::to_string(foodDY);
    }

    std::string chooseAction(const std::string &state)
    {
        if ((double)rand() / RAND_MAX < epsilon)
        {
            // Exploration: Choose a random action
            return actions[rand() % actions.size()];
        }
        // Exploitation: Choose the action with the highest Q-value
        if (QTable.find(state) == QTable.end())
        {
            QTable[state] = std::vector<double>(4, 0.0); // Initialize Q-values
        }
        int bestActionIndex = std::max_element(QTable[state].begin(), QTable[state].end()) - QTable[state].begin();
        return actions[bestActionIndex];
    }

    int performActionAndGetReward(std::vector<Snake *> snakes, Food food, int width, int height)
    {
        // Example reward: +10 for eating food, -10 for dying, 0 otherwise
        if (eatFood(food.x, food.y))
        {
            grow();
            return 10;
        }
        if (checkWallCollision(width, height) || checkSelfCollision())
        {
            isAlive = false;
            return -10;
        }
        return 0;
    }

    void updateQValue(const std::string &state, const std::string &action, int reward, const std::string &nextState)
    {
        if (QTable.find(state) == QTable.end())
        {
            QTable[state] = std::vector<double>(4, 0.0); // Initialize Q-values
        }
        if (QTable.find(nextState) == QTable.end())
        {
            QTable[nextState] = std::vector<double>(4, 0.0); // Initialize Q-values
        }
        int actionIndex = std::distance(actions.begin(), std::find(actions.begin(), actions.end(), action));
        double maxNextQValue = *std::max_element(QTable[nextState].begin(), QTable[nextState].end());
        QTable[state][actionIndex] += alpha * (reward + gamma * maxNextQValue - QTable[state][actionIndex]);
    }
};

void checkSnakeCollisions(std::vector<Snake *> &snakes)
{
    for (size_t i = 0; i < snakes.size(); ++i)
    {
        if (!snakes[i]->isAlive)
            continue; // Skip dead snakes

        int headX = std::get<0>(*snakes[i]->body[0]);
        int headY = std::get<1>(*snakes[i]->body[0]);

        for (size_t j = 0; j < snakes.size(); ++j)
        {
            if (!snakes[j]->isAlive)
                continue; // Skip collisions with dead snakes

            // Skip self-collision check within the same snake
            if (i == j)
                continue;

            for (const auto &part : snakes[j]->body)
            {
                int partX = std::get<0>(*part);
                int partY = std::get<1>(*part);

                if (headX == partX && headY == partY)
                {
                    snakes[i]->isAlive = false;
                    break;
                }
            }
        }
    }
}




void showScoreboard(const std::vector<Snake *> &snakes)
{
    clear();
    mvprintw(0, 0, "Scoreboard:");
    for (size_t i = 0; i < snakes.size(); ++i)
    {
        mvprintw(i + 1, 0, "Player %zu: %d", i, snakes[i]->getLength());
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
    std::vector<Snake *> snakes;
    HumanSnake snake_one(5, 10, "right", 'w', 's', 'a', 'd');
    QLearningSnake snake_two(35, 10, "left");
    snake_two.loadQTableFromFile("qtable.txt");
    snakes.push_back(&snake_one);
    snakes.push_back(&snake_two);

    while (true)
    {
        renderGameWindow(height, width);
        int ch = getch();
        bool anySnakeAlive = false;
        for (auto *snake : snakes)
        {
            snake->changeDirection(ch, snakes, food, width, height);
            snake->move();
            snake->renderSnake();
            if (snake->isAlive)
            {
                anySnakeAlive = true;
                if (snake->checkWallCollision(width, height) || snake->checkSelfCollision())
                {
                    snake->isAlive = false;
                }
                if (snake->eatFood(food.x, food.y))
                {
                    snake->grow();
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
    snake_two.saveQTableToFile("qtable.txt");
    endwin();
    return 0;
}