# Snake AI Learning Experiment

This project focuses on teaching a learning AI to play Snake and experimenting with different AI strategies for a two-player Snake game. The game also supports a user-controlled player for real-time testing and gameplay.

## Features

- Train an AI to learn and improve at Snake.
- Experiment with multiple predefined AI strategies.
- Two-player mode: User vs. AI or AI vs. AI.
- Terminal-based gameplay.

## Quickstart

```bash
# clone the repo
sudo apt-get install g++ libncurses5-dev libncursesw5-dev
g++ snake.cpp -o snake -lncurses && ./snake
```

## Gameplay

- Player 1 (user) uses the WASD keys for movement.
- Player 2 can be controlled by:
  - An AI: Automatically moves based on the selected strategy or learned behavior.
  - A second user: Uses the arrow keys for movement.
