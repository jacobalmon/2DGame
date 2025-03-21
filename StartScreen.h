#ifndef START_SCREEN_H
#define START_SCREEN_H

#include "raylib.h"

class StartScreen {
public:
    StartScreen() {
        playButton = { 320, 250, 160, 50 };
        exitButton = { 320, 320, 160, 50 };
        startGame = false;
        exitGame = false;
    }

    void Update() {
        Vector2 mouse = GetMousePosition();

        // Detect button hover and click
        if (CheckCollisionPointRec(mouse, playButton)) {
            playButtonColor = LIGHTGRAY;
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                startGame = true; // Start the game
            }
        } else {
            playButtonColor = GRAY;
        }

        if (CheckCollisionPointRec(mouse, exitButton)) {
            exitButtonColor = LIGHTGRAY;
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                exitGame = true; // Exit the game
            }
        } else {
            exitButtonColor = GRAY;
        }
    }

    void Draw() {
        ClearBackground(BLUE);

        // Draw title
        DrawText("PLATFORMER GAME", 250, 100, 30, DARKGRAY);

        // Draw play and exit buttons
        DrawRectangleRec(playButton, playButtonColor);
        DrawRectangleRec(exitButton, exitButtonColor);

        // Draw button text
        DrawText("PLAY", playButton.x + 50, playButton.y + 15, 20, BLACK);
        DrawText("EXIT", exitButton.x + 50, exitButton.y + 15, 20, BLACK);
    }

    bool ShouldStartGame() { return startGame; }
    bool ShouldExitGame() { return exitGame; }

private:
    Rectangle playButton;
    Rectangle exitButton;
    Color playButtonColor = GRAY;
    Color exitButtonColor = GRAY;
    bool startGame;
    bool exitGame;
};

#endif // START_SCREEN_H