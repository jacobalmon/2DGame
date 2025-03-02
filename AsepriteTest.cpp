#include "raylib.h"
#include "AsepriteLoader.h"
#include "AsepriteExample.h"
#include <iostream>
#include <string>

// Test program to demonstrate Aseprite animations
int main() {
    // Initialize window
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "Aseprite Animation Test");
    SetTargetFPS(60);
    
    // Create character
    AsepriteCharacter character(Vector2{400, 500});
    
    // Try to load animations from different potential paths
    bool animationsLoaded = false;
    
    // First try the default path
    if (FileExists("assets/character_sheet.png") && FileExists("assets/character_sheet.json")) {
        animationsLoaded = character.LoadAnimations("assets/character_sheet.png", "assets/character_sheet.json");
    }
    // Then try the Goblin path as fallback
    else if (FileExists("assets/Goblin/goblin_sheet.png") && FileExists("assets/Goblin/goblin_sheet.json")) {
        animationsLoaded = character.LoadAnimations("assets/Goblin/goblin_sheet.png", "assets/Goblin/goblin_sheet.json");
    }
    
    // Main game loop
    while (!WindowShouldClose()) {
        // Update
        character.Update(GetFrameTime());
        
        // Draw
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        // Draw floor line
        DrawLine(0, 550, screenWidth, 550, DARKGRAY);
        
        // Draw character if animations are loaded
        if (animationsLoaded) {
            character.Draw();
            
            // Draw controls
            DrawText("Controls:", 20, 20, 20, BLACK);
            DrawText("- Left/Right or A/D: Move", 20, 50, 20, DARKGRAY);
            DrawText("- Space: Attack", 20, 80, 20, DARKGRAY);
        } else {
            // Show warning if no animations are loaded
            DrawText("No animation files found!", screenWidth/2 - 150, screenHeight/2 - 30, 20, RED);
            DrawText("Please export your Aseprite animations to:", screenWidth/2 - 200, screenHeight/2, 20, DARKGRAY);
            DrawText("assets/character_sheet.png and assets/character_sheet.json", screenWidth/2 - 250, screenHeight/2 + 30, 20, DARKGRAY);
            DrawText("or", screenWidth/2 - 10, screenHeight/2 + 60, 20, DARKGRAY);
            DrawText("assets/Goblin/goblin_sheet.png and assets/Goblin/goblin_sheet.json", screenWidth/2 - 250, screenHeight/2 + 90, 20, DARKGRAY);
            
            DrawText("Use the export_aseprite.sh script to export your animations", screenWidth/2 - 250, screenHeight/2 + 150, 20, DARKGRAY);
        }
        
        EndDrawing();
    }
    
    // Close window and unload resources
    CloseWindow();
    
    return 0;
} 