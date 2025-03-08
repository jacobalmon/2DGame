#include <raylib.h>
#include <vector>

// Directions for Goblin movement (LEFT_GOBLIN or RIGHT_GOBLIN)
enum DirectionGoblin {
    LEFT_GOBLIN = -1,
    RIGHT_GOBLIN = 1
};

// Possible states for the Goblin (Idle, Walking, Attacking, Dead)
enum CurrentStateGoblin {
    DEAD_GOBLIN = 0,
    ATTACK_CLUB = 1,
    ATTACK_STOMP = 2,
    ATTACK_AOE = 3,
    IDLE_GOBLIN = 4,
    WALK_GOBLIN = 5
};

// Animation types for Goblin (Repeating or One-shot)
enum AnimationTypeGoblin {
    REPEATING_GOBLIN,
    ONESHOT_GOBLIN
};

// Struct that holds animation data for each state
struct AnimationGoblin {
    int firstFrame;      // First frame of the animation
    int lastFrame;       // Last frame of the animation
    int currentFrame;    // Current frame being displayed
    int offset;          // Offset for animation positioning
    float speed;         // Speed of animation (time per frame)
    float timeLeft;      // Remaining time until frame change
    AnimationTypeGoblin type;  // Type of animation (repeating or one-shot)
};

class Goblin {
public:
    Rectangle rect;          // Goblin's position and size
    Vector2 velocity;        // Goblin's movement velocity
    Texture2D spriteSheet;   // Goblin's sprite sheet
    DirectionGoblin direction; // Goblin's facing direction
    CurrentStateGoblin state; // Current state of the Goblin (idle, walking, attacking)
    std::vector<AnimationGoblin> animations; // List of animations
    std::vector<Texture2D> sprites; // List of sprite sheets for each state

    bool isAttacking = false;    // Flag to check if Goblin is attacking
    bool hasFinishedAttack = true;  // Flag to check if the attack animation is finished

    // Constructor initializing the Goblin's properties and animations
    Goblin(Vector2 position) {
        rect = (Rectangle) {position.x, position.y, 64.0f, 64.0f};  // Initial position and size
        velocity = (Vector2) {0.0f, 0.0f}; // No initial velocity
        direction = RIGHT_GOBLIN; // Initially facing right
        state = IDLE_GOBLIN; // Starting state is idle

        // Initializing the animation states for the Goblin
        animations = {
            {0, 8, 0, 0, 0.1f, 0.1f, ONESHOT_GOBLIN},  // Dead Goblin
            {0, 9, 0, 5, 0.1f, 0.1f, ONESHOT_GOBLIN},  // Attack Club
            {5, 9, 0, 0, 0.1f, 0.1f, ONESHOT_GOBLIN},  // Attack Stomp
            {0, 23, 0, 0, 0.1f, 0.1f, ONESHOT_GOBLIN}, // Attack AoE
            {0, 6, 0, 0, 0.1f, 0.1f, REPEATING_GOBLIN}, // Idle Goblin
            {0, 7, 0, 0, 0.1f, 0.1f, REPEATING_GOBLIN}  // Walking Goblin
        };
    }

    // Destructor to unload textures
    ~Goblin() {
        for (auto& sprite : sprites) {
            UnloadTexture(sprite); // Unload all textures
        }
    }

    // Load textures for the Goblin's states (walking, idle, etc.)
    void loadTextures() {
        sprites.resize(6);

        // Load sprite sheets for each state
        sprites[DEAD_GOBLIN] = LoadTexture("assets/Goblin/Hobgoblin Die/Hobgoblin Beheaded.png");
        sprites[ATTACK_CLUB] = LoadTexture("assets/Goblin/Hobgoblin Attack 1 & 2/Hobgoblin Attack 1 and 2.png");
        sprites[ATTACK_STOMP] = LoadTexture("assets/Goblin/Hobgoblin Attack 1 & 2/Hobgoblin Attack 1 and 2.png");
        sprites[ATTACK_AOE] = LoadTexture("assets/Goblin/Hobgoblin Attack 3/Hobgoblin Attack 3.png");
        sprites[IDLE_GOBLIN] = LoadTexture("assets/Goblin/Hobgoblin Idle/GoblinK Idle.png");
        sprites[WALK_GOBLIN] = LoadTexture("assets/Goblin/Hobgoblin Walk/Hobgoblin Walk.png");
    }

    // Update the Goblin's animation based on the state and frame timing
    void updateAnimation() {
        AnimationGoblin& anim = animations[state];
        float deltaTime = GetFrameTime();  // Time elapsed per frame
    
        anim.timeLeft -= deltaTime;
        if (anim.timeLeft <= 0) {
            anim.timeLeft = anim.speed;
            anim.currentFrame++;  // Move to the next frame
    
            // For ATTACK_CLUB, stop at frame 4
            if (state == ATTACK_CLUB && anim.currentFrame > 4) {
                anim.currentFrame = 4;  // Stop at frame 4
                hasFinishedAttack = true;  // Mark the attack as finished
                state = IDLE_GOBLIN;  // Transition to idle after attack finishes
            }
    
            // Handle animation completion and reset logic
            if (anim.currentFrame > anim.lastFrame) {
                if (anim.type == REPEATING_GOBLIN) {
                    anim.currentFrame = anim.firstFrame;  // Loop animation
                } else if (anim.type == ONESHOT_GOBLIN) {
                    if (state != ATTACK_CLUB) {
                        anim.currentFrame = anim.lastFrame;  // Stay on the last frame for one-shot animations
                        hasFinishedAttack = true;  // Mark the attack as finished
                        state = IDLE_GOBLIN;  // Transition to idle
                    }
                }
            }
        }
    }

    // Get the current frame of the animation for rendering
    Rectangle getAnimationFrame() const {
        const AnimationGoblin& anim = animations[state];
        int frameWidth = sprites[state].width / (anim.lastFrame + 1);  // Calculate width of each frame
        int frameHeight = sprites[state].height;  // Height of the frame

        return (Rectangle) {
            (float)frameWidth * anim.currentFrame, 0, (float)frameWidth, (float)frameHeight
        };
    }

    // Draw the Goblin with the current animation frame
    void draw() const {
        Rectangle source = getAnimationFrame();  // Get the current frame for the animation
        float scale = 2.0f;  // Scale the size of the sprite

        Rectangle dest = { 
            rect.x, rect.y, 
            rect.width * scale,   // Scale the width
            rect.height * scale   // Scale the height
        };

        source.width *= direction;  // Flip the sprite depending on direction

        DrawTexturePro(sprites[state], source, dest, {0, 0}, 0.0f, WHITE);  // Draw the sprite
    }

    // Handle Goblin's movement based on key inputs
    void move() {
        if (!hasFinishedAttack) return;  
    
        float moveSpeed = 300.0f;
        velocity.x = 0.0f;  
    
        if (IsKeyDown(KEY_X)) {
            velocity.x = -moveSpeed;
            direction = LEFT_GOBLIN;
            state = WALK_GOBLIN;
        } 
        else if (IsKeyDown(KEY_C)) {
            velocity.x = moveSpeed;
            direction = RIGHT_GOBLIN;
            state = WALK_GOBLIN;
        } 
        else {
            state = IDLE_GOBLIN;
        }
    
        // When attacking, reset velocity to stop movement
        if (IsKeyPressed(KEY_KP_1) && hasFinishedAttack) {
            state = ATTACK_CLUB;
            hasFinishedAttack = false;
            velocity.x = 0; // Prevent movement during attack
            animations[ATTACK_CLUB].currentFrame = animations[ATTACK_CLUB].firstFrame;
        }
        if (IsKeyPressed(KEY_KP_2) && hasFinishedAttack) {
            state = ATTACK_STOMP;
            hasFinishedAttack = false;
            velocity.x = 0;
            animations[ATTACK_STOMP].currentFrame = animations[ATTACK_STOMP].firstFrame;
        }
        if (IsKeyPressed(KEY_KP_3) && hasFinishedAttack) {
            state = ATTACK_AOE;
            hasFinishedAttack = false;
            velocity.x = 0;
            animations[ATTACK_AOE].currentFrame = animations[ATTACK_AOE].firstFrame;
        }
    }

    // Apply velocity to the Goblin's position based on time
    void applyVelocity() {
        // Apply velocity only when not attacking
        if (hasFinishedAttack) {
            rect.x += velocity.x * GetFrameTime();
        }
    }    
};