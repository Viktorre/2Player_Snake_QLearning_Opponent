#include <ncurses.h> // ncurses library for handling the terminal screen
#include <vector>    // Standard vector library for storing snake body
#include <cstdlib>   // For random numbers (used later for food spawning)
#include <unistd.h>
#include <string>
#include <tuple> // Include tuple library
#include <time.h>

// Define a point struct to represent the snake's body parts (coordinates)
struct Point
{
    int x, y;
    Point(int col, int row) : x(col), y(row) {}
};

void initGame()
{
    initscr();             // ncurses: Start ncurses mode
    cbreak();              // ncurses: Disable line buffering
    noecho();              // ncurses: Don't show keypresses on the screen
    curs_set(0);           // ncurses: Hide the cursor
    keypad(stdscr, TRUE);  // ncurses: Enable arrow keys input
    nodelay(stdscr, TRUE); // ncurses: Non-blocking input (game loop can run)
    refresh();             // ncurses: Refresh the screen to apply changes
}

void renderGameWindow(int height, int width)
{
    clear(); // ncurses: Clear the screen for rendering
    // Draw border using ncurses
    for (int i = 0; i < width; i++)
    {
        mvprintw(0, i, "#");          // ncurses: Top border at row 0
        mvprintw(height - 1, i, "#"); // ncurses: Bottom border at row (height-1)
    }
    for (int i = 0; i < height; i++)
    {
        mvprintw(i, 0, "#");         // ncurses: Left border at column 0
        mvprintw(i, width - 1, "#"); // ncurses: Right border at column (width-1)
    }
    refresh(); // ncurses: Refresh to display the new game window
}

class Snake
{
public:
    int startX;                             // Starting x-coordinate of the snake
    int startY;                             // Starting y-coordinate of the snake
    std::string *direction;                 // Pointer to the direction of the snake
    std::vector<std::tuple<int, int>> body; // Vector of tuples to represent the snake's body parts

    Snake(int x, int y, std::string *dir) : startX(x), startY(y), direction(dir) // Take a pointer to a string
    {
        body.push_back(std::make_tuple(startX, startY)); // Initialize the body at the starting position
    }

    void renderSnake()
    {
        for (const auto &part : body)
        {
            int x = std::get<0>(part);
            int y = std::get<1>(part);
            mvprintw(y, x, "o"); // Render each part of the snake at its coordinates
        }
        refresh(); // Refresh the screen to display the snake
    }

    void move()
    {
        // Get current head position
        int headX = std::get<0>(body[0]);
        int headY = std::get<1>(body[0]);

        // Calculate new head position based on direction
        if (*direction == "up")
        {
            headY -= 1;
        }
        else if (*direction == "down")
        {
            headY += 1;
        }
        else if (*direction == "left")
        {
            headX -= 1;
        }
        else if (*direction == "right")
        {
            headX += 1;
        }

        // Move the body
        for (int i = body.size() - 1; i > 0; --i)
        {
            body[i] = body[i - 1];
        }
        body[0] = std::make_tuple(headX, headY);
    }

    bool checkCollision(int height, int width)
    {
        int headX = std::get<0>(body[0]);
        int headY = std::get<1>(body[0]);

        if (headX <= 0 || headX >= width - 1 || headY <= 0 || headY >= height - 1)
        {
            return true; // Collision with wall
        }

        for (size_t i = 1; i < body.size(); ++i)
        {
            if (std::get<0>(body[i]) == headX && std::get<1>(body[i]) == headY)
            {
                return true; // Collision with self
            }
        }
        return false; // No collision
    }

    void changeDirection()
    {
        int ch = getch(); // ncurses: Get key press if available

        std::string newDirection;
        if (ch == 'w' || ch == 'W')
        {
            newDirection = "up";
        }
        else if (ch == 's' || ch == 'S')
        {
            newDirection = "down";
        }
        else if (ch == 'a' || ch == 'A')
        {
            newDirection = "left";
        }
        else if (ch == 'd' || ch == 'D')
        {
            newDirection = "right";
        }
        else
        {
            return; // No valid direction change
        }

        if ((*direction == "up" && newDirection != "down") ||
            (*direction == "down" && newDirection != "up") ||
            (*direction == "left" && newDirection != "right") ||
            (*direction == "right" && newDirection != "left"))
        {
            *direction = newDirection;
        }
    }

    // New method to check if the snake eats the food
    bool eatFood(int foodX, int foodY)
    {
        int headX = std::get<0>(body[0]);
        int headY = std::get<1>(body[0]);
        return headX == foodX && headY == foodY; // Return true if food is eaten
    }

    // New method to grow the snake's body
    void grow()
    {
        body.push_back(body.back()); // Duplicate the last segment
    }
};

struct Food
{
    int x, y;

    Food(int maxX, int maxY)
    {
        spawn(maxX, maxY);
    }

    void spawn(int maxX, int maxY)
    {
        x = rand() % (maxX - 2) + 1; // Avoid walls (1 to maxX - 2)
        y = rand() % (maxY - 2) + 1; // Avoid walls (1 to maxY - 2)
    }

    void render() const
    {
        mvprintw(y, x, "F"); // Use ncurses to print food at position
        refresh();
    }
};

int main()
{
    srand(time(0));  // Seed random number generator
    int height = 20; // Game window height
    int width = 40;  // Game window width
    initGame();      // Initialize ncurses game screen

    Food food(width, height); // Pass width and height to Food constructor

    std::vector<Snake> snakes;                           // Vector to hold multiple Snake objects
    std::string direction_snake_one = "right";           // Create a string for initial direction
    Snake snake_one = Snake(5, 5, &direction_snake_one); // Pass the address of the string to the constructor
    snakes.push_back(snake_one);                         // Add snake_one to the vector

    bool gameOver = false;
    while (!gameOver)
    {
        renderGameWindow(height, width); // Render the game window with borders
        for (auto &snake : snakes)
        {
            snake.changeDirection();
            snake.renderSnake();
            snake.move();
            gameOver = snake.checkCollision(height, width);
            if (snake.eatFood(food.x, food.y))
            {
                snake.grow();              // Call the grow method to increase the snake's length
                food.spawn(width, height); // Respawn the food
            }
        }
        food.render();

        usleep(100000); // Sleep for 100ms for game speed
    }

    getch();  // ncurses: Wait for user input (pauses game)
    endwin(); // ncurses: End ncurses mode and return to normal terminal

    return 0;
}
