#ifndef ASEPRITE_EXAMPLE_H
#define ASEPRITE_EXAMPLE_H

#include "AsepriteLoader.h"
#include "raylib.h"
#include <string>
#include <iostream>

// A simple character class that uses Aseprite animations
class AsepriteCharacter {
private:
    AsepriteAnimation animation;
    Vector2 position;
    float direction;  // 1 for right, -1 for left
    std::string currentState;
    bool loaded;

public:
    AsepriteCharacter(Vector2 startPos) : 
        position(startPos), 
        direction(1.0f),
        currentState("idle"),
        loaded(false) {
    }

    ~AsepriteCharacter() {
        animation.Unload();
    }

    // Load character animations from PNG and JSON files
    bool LoadAnimations(const std::string& pngPath, const std::string& jsonPath) {
        if (!animation.LoadFromFiles(pngPath.c_str(), jsonPath.c_str())) {
            std::cerr << "Failed to load character animations from: " << pngPath << std::endl;
            return false;
        }
        
        // Try to play the idle animation by default
        std::vector<std::string> tags = animation.GetAllTags();
        if (!tags.empty()) {
            // Look for an idle animation first
            bool foundIdle = false;
            for (const auto& tag : tags) {
                if (tag == "idle" || tag == "IDLE") {
                    animation.Play(tag);
                    currentState = tag;
                    foundIdle = true;
                    break;
                }
            }
            
            // If no idle animation, just play the first one
            if (!foundIdle) {
                animation.Play(tags[0]);
                currentState = tags[0];
            }
        }
        
        loaded = true;
        return true;
    }

    // Update character state based on input
    void Update(float deltaTime) {
        if (!loaded) return;
        
        // Handle input for character movement and actions
        bool isMoving = false;
        
        // Move left
        if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
            position.x -= 200.0f * deltaTime;
            direction = -1.0f;
            isMoving = true;
            
            // Try to play walk animation if available
            if (currentState != "walk" && currentState != "WALK") {
                if (animation.Play("walk") || animation.Play("WALK")) {
                    currentState = "walk";
                }
            }
        }
        // Move right
        else if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
            position.x += 200.0f * deltaTime;
            direction = 1.0f;
            isMoving = true;
            
            // Try to play walk animation if available
            if (currentState != "walk" && currentState != "WALK") {
                if (animation.Play("walk") || animation.Play("WALK")) {
                    currentState = "walk";
                }
            }
        }
        
        // Attack
        if (IsKeyPressed(KEY_SPACE)) {
            // Try to play attack animation if available
            if (animation.Play("attack") || animation.Play("ATTACK")) {
                currentState = "attack";
            }
        }
        
        // If not moving or attacking, return to idle
        if (!isMoving && (currentState == "walk" || currentState == "WALK" || !animation.IsPlaying())) {
            if (animation.Play("idle") || animation.Play("IDLE")) {
                currentState = "idle";
            }
        }
        
        // Update animation
        animation.Update(deltaTime);
    }

    // Draw the character
    void Draw() {
        if (!loaded) return;
        
        // Draw the character with appropriate flipping based on direction
        animation.DrawEx(position.x, position.y, direction < 0, 2.0f);
        
        // Draw debug info
        DrawText(currentState.c_str(), position.x - 20, position.y - 40, 20, RED);
    }

    // Check if animations are loaded
    bool IsLoaded() const {
        return loaded;
    }

    // Get character position
    Vector2 GetPosition() const {
        return position;
    }

    // Set character position
    void SetPosition(Vector2 newPosition) {
        position = newPosition;
    }
};

/*
// Example usage in main game file:

AsepriteCharacter* character = nullptr;

void InitGame() {
    character = new AsepriteCharacter(Vector2{400, 300});
    character->LoadAnimations("assets/character_sheet.png", "assets/character_sheet.json");
}

void UpdateGame() {
    character->Update(GetFrameTime());
}

void DrawGame() {
    character->Draw();
}

void UnloadGame() {
    delete character;
}
*/

#endif // ASEPRITE_EXAMPLE_H 