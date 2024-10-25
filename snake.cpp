#include <ncurses.h> // ncurses library for handling the terminal screen
#include <vector>    // Standard vector library for storing snake body
#include <cstdlib>   // For random numbers (used later for food spawning)
#include <unistd.h>
#include <string>

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

        for (int i = body.size() - 1; i > 0; --i)
        {
            body[i] = body[i - 1];
        }
        body[0] = std::make_tuple(headX, headY);
    }
};
// void move(int direction);       // Move the snake in the given direction
// void grow();                    // Make the snake longer
// bool checkCollision();          // Check if the snake hits the wall or itself
// std::vector<Point> getBody();   // Return the snake's body (for food collision)

int main()
{
    int height = 20; // Game window height
    int width = 40;  // Game window width
    initGame();      // Initialize ncurses game screen

    std::string initialDirection = "right";           // Create a string for initial direction
    Snake snake_one = Snake(5, 5, &initialDirection); // Pass the address of the string to the constructor

    int timer = 0;

    bool gameOver = false;
    while (!gameOver)
    {
        renderGameWindow(height, width); // Render the game window with borders
        snake_one.renderSnake();
        snake_one.move();
        usleep(100000); // Sleep for 100ms for game speed
        timer = timer + 1;
        initialDirection = "down";
        if (timer > 10)
        {
            gameOver = true;
        }
    }

    getch();  // ncurses: Wait for user input (pauses game)
    endwin(); // ncurses: End ncurses mode and return to normal terminal

    return 0;
}
