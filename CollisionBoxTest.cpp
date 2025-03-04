#include "raylib.h"
#include "GameConstants.h"

// Game states
enum GameState {
    TITLE,
    PLAYING,
    GAME_OVER
};

// Character states
enum CharacterState {
    IDLE,
    WALKING,
    ATTACKING,
    PARRYING,
    HURT
};

// Character direction
enum CharacterDirection {
    LEFT = -1,
    RIGHT = 1
};

// Character class with collision and hit boxes
class Character {
private:
    Rectangle rect;            // Main character rectangle
    Rectangle collisionBox;    // Collision box for movement
    Rectangle hitBox;          // Hit box for attacks
    Vector2 position;          // Position
    Vector2 velocity;          // Velocity
    Color color;               // Character color
    CharacterState state;      // Current state
    CharacterDirection direction; // Current direction
    bool showDebugBoxes;       // Debug visualization
    
public:
    Character(Vector2 pos, Color col) : 
        position(pos),
        velocity({ 0, 0 }),
        color(col),
        state(IDLE),
        direction(RIGHT),
        showDebugBoxes(true) {
        
        // Initialize rectangles
        rect = { position.x, position.y, 64, 64 };
        collisionBox = { position.x + 16, position.y + 16, 32, 48 };
        hitBox = { 0, 0, 0, 0 }; // No hit box by default
    }
    
    // Update character state
    void update(float deltaTime) {
        // Update position based on velocity
        position.x += velocity.x * deltaTime;
        position.y += velocity.y * deltaTime;
        
        // Update rectangles
        rect.x = position.x;
        rect.y = position.y;
        
        // Update collision box
        collisionBox.x = position.x + 16;
        collisionBox.y = position.y + 16;
        
        // Update hit box based on state
        updateHitBox();
    }
    
    // Update hit box based on current state
    void updateHitBox() {
        if (state == ATTACKING) {
            // Extend hit box in the direction the character is facing
            if (direction == RIGHT) {
                hitBox = { position.x + 40, position.y + 10, 48, 40 };
            } else {
                hitBox = { position.x - 24, position.y + 10, 48, 40 };
            }
        } else if (state == PARRYING) {
            // Parry box covers the character's body
            hitBox = { position.x + 10, position.y, 44, 64 };
        } else {
            // No hit box for other states
            hitBox = { 0, 0, 0, 0 };
        }
    }
    
    // Draw the character
    void draw() {
        // Draw character
        DrawRectangle(rect.x, rect.y, rect.width, rect.height, color);
        
        // Draw direction indicator
        if (direction == RIGHT) {
            DrawTriangle(
                (Vector2){ rect.x + rect.width, rect.y + rect.height / 2 },
                (Vector2){ rect.x + rect.width - 10, rect.y + rect.height / 2 - 10 },
                (Vector2){ rect.x + rect.width - 10, rect.y + rect.height / 2 + 10 },
                BLACK
            );
        } else {
            DrawTriangle(
                (Vector2){ rect.x, rect.y + rect.height / 2 },
                (Vector2){ rect.x + 10, rect.y + rect.height / 2 - 10 },
                (Vector2){ rect.x + 10, rect.y + rect.height / 2 + 10 },
                BLACK
            );
        }
        
        // Draw state indicator
        const char* stateText = "";
        switch (state) {
            case IDLE: stateText = "IDLE"; break;
            case WALKING: stateText = "WALKING"; break;
            case ATTACKING: stateText = "ATTACKING"; break;
            case PARRYING: stateText = "PARRYING"; break;
            case HURT: stateText = "HURT"; break;
        }
        DrawText(stateText, rect.x, rect.y - 20, 16, BLACK);
        
        // Draw debug boxes if enabled
        if (showDebugBoxes) {
            // Draw collision box (green)
            DrawRectangleLinesEx(collisionBox, 1.0f, GREEN);
            
            // Draw hit box (red) if active
            if (hitBox.width > 0) {
                DrawRectangleLinesEx(hitBox, 1.0f, RED);
            }
            
            // Draw character rect (blue)
            DrawRectangleLinesEx(rect, 1.0f, BLUE);
        }
    }
    
    // Set character state
    void setState(CharacterState newState) {
        state = newState;
        updateHitBox();
    }
    
    // Set character direction
    void setDirection(CharacterDirection newDirection) {
        direction = newDirection;
        updateHitBox();
    }
    
    // Set character velocity
    void setVelocity(Vector2 newVelocity) {
        velocity = newVelocity;
    }
    
    // Toggle debug boxes
    void toggleDebugBoxes() {
        showDebugBoxes = !showDebugBoxes;
    }
    
    // Get collision box
    Rectangle getCollisionBox() const {
        return collisionBox;
    }
    
    // Get hit box
    Rectangle getHitBox() const {
        return hitBox;
    }
    
    // Get character state
    CharacterState getState() const {
        return state;
    }
    
    // Check collision with another character
    bool checkCollision(const Character& other) const {
        return CheckCollisionRecs(collisionBox, other.collisionBox);
    }
    
    // Check hit collision with another character
    bool checkHitCollision(const Character& other) const {
        return hitBox.width > 0 && CheckCollisionRecs(hitBox, other.collisionBox);
    }
};

int main() {
    // Initialize window
    InitWindow(800, 600, "Collision Box Test");
    
    // Create characters
    Character player((Vector2){ 200, 300 }, BLUE);
    Character enemy((Vector2){ 500, 300 }, RED);
    
    // Game state
    GameState gameState = PLAYING;
    bool collisionDetected = false;
    bool hitDetected = false;
    
    SetTargetFPS(60);
    
    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        
        // Toggle debug boxes with F1
        if (IsKeyPressed(KEY_F1)) {
            player.toggleDebugBoxes();
            enemy.toggleDebugBoxes();
        }
        
        // Update player based on input
        Vector2 playerVelocity = { 0, 0 };
        
        // Movement
        if (IsKeyDown(KEY_A)) {
            playerVelocity.x = -200;
            player.setDirection(LEFT);
            player.setState(WALKING);
        } else if (IsKeyDown(KEY_D)) {
            playerVelocity.x = 200;
            player.setDirection(RIGHT);
            player.setState(WALKING);
        } else if (player.getState() == WALKING) {
            player.setState(IDLE);
        }
        
        // Attack
        if (IsKeyPressed(KEY_SPACE)) {
            player.setState(ATTACKING);
        } else if (player.getState() == ATTACKING && !IsKeyDown(KEY_SPACE)) {
            player.setState(IDLE);
        }
        
        // Parry
        if (IsKeyPressed(KEY_E)) {
            player.setState(PARRYING);
        } else if (player.getState() == PARRYING && !IsKeyDown(KEY_E)) {
            player.setState(IDLE);
        }
        
        player.setVelocity(playerVelocity);
        
        // Update characters
        player.update(deltaTime);
        enemy.update(deltaTime);
        
        // Check collisions
        collisionDetected = player.checkCollision(enemy);
        hitDetected = player.checkHitCollision(enemy);
        
        // Drawing
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        // Draw floor
        DrawRectangle(0, 400, 800, 200, DARKGRAY);
        
        // Draw characters
        player.draw();
        enemy.draw();
        
        // Draw collision status
        if (collisionDetected) {
            DrawText("COLLISION DETECTED!", 300, 50, 20, RED);
        }
        
        if (hitDetected) {
            DrawText("HIT DETECTED!", 320, 80, 20, RED);
        }
        
        // Draw controls
        DrawText("Controls: A/D - Move, SPACE - Attack, E - Parry, F1 - Toggle Debug Boxes", 10, 10, 20, DARKGRAY);
        
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
} 