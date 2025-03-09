#include <raylib.h>
#include <vector>

// Direction of the Character.
enum Direction {
    LEFT = -1,
    RIGHT = 1
};

// States for the Character. (Note: not all will be used).
enum CurrentState {
    DEAD = 0,
    ATTACK = 1,
    HURT = 2,
    IDLE = 3,
    JUMP = 4,
    RUN = 5
};

// Determines whether an animation is ran once or repeats.
enum AnimationType {
    REPEATING,
    ONESHOT
};

// Represents an Animation.
struct Animation {
    int firstFrame; // First Frame of Animation.
    int lastFrame; // Last Frame of Animation.
    int currentFrame; // Current Frame Displayed.
    int offset; // Frame Offset (if needed).
    float speed; // Speed of Animation.
    float timeLeft; // Time Left for next frame.
    AnimationType type; // Type of Animation.
};

class Samurai {
private:
    Rectangle rect; // Character's rectangle for position and size.
    Vector2 velocity; // Velocity of the character for movement.
    Texture2D spriteSheet; // The texture sheet used for animations.
    Direction direction; // Current facing direction of the character.
    CurrentState state; // Current state of the character (e.g., idle, attack, etc.).
    std::vector<Animation> animations; // List of animations for different states.
    std::vector<Texture2D> sprites; // List of textures for each state.
    float groundLevel; // The Y-coordinate of the ground level.
    bool isRunning = false; // Flag for checking if the player is running or not.

    // Define Health Variables
    int maxHealth = 100; // Maximum health value.
    int currentHealth = 100; // Current health of the samurai.
    bool isDead = false; // Flag to indicate if the samurai is dead.
    bool wasInAir = false; // Flag to indicate if the character was in the air.

    // Define Sound Varaibles.
    Sound attackSound;
    Sound jumpSound;
    Sound hurtSound;
    Sound runSound;
    Sound deadSound;
    Sound landSound;

    // Helper method to update the animation frame.
    void updateAnimation() {
        Animation& anim = animations[state];
        float deltaTime = GetFrameTime();

        // Prevent animation switch while in the HURT state.
        if (state == HURT) {
            if (anim.currentFrame == anim.lastFrame) {
                if (currentHealth > 0) {
                    state = IDLE;  // Switch to idle once hurt animation finishes.
                }
            }
        }

        // Rest of the animation handling.
        if (anim.currentFrame == anim.firstFrame && anim.timeLeft == anim.speed) {
            anim.timeLeft = anim.speed;
        }

        anim.timeLeft -= deltaTime;
        if (anim.timeLeft <= 0) {
            anim.timeLeft = anim.speed;
            anim.currentFrame++;

            if (anim.currentFrame > anim.lastFrame) {
                if (anim.type == REPEATING) {
                    anim.currentFrame = anim.firstFrame;
                } else if (anim.type == ONESHOT) {
                    anim.currentFrame = anim.lastFrame;
                }
            }
        }
    }

    // Helper method to get the animation frame for drawing.
    Rectangle getAnimationFrame() const {
        const Animation &anim = animations[state];
        // Calculate Frame Dimensions.
        int frameWidth = sprites[state].width / (anim.lastFrame + 1);
        int frameHeight = sprites[state].height;

        return (Rectangle){ // Return Rectangle for the current frame.
            (float)frameWidth * anim.currentFrame, 0, (float)frameWidth, (float)frameHeight
        };
    }

    void move() {
        float moveSpeed = 300.0f; // Set movement speed.
        velocity.x = 0.0f; // Reset horizontal velocity.
    
        // Prevent movement during attack or hurt states.
        if (state == ATTACK || state == HURT || state == DEAD) {
            if (isRunning) { // Stop running sound if attacking or hurt
                StopSound(runSound);
                isRunning = false;
            }
            return;
        }
    
        // Handle Jumping
        if (IsKeyPressed(KEY_W) && rect.y >= groundLevel) { // Jump action.
            state = JUMP;
            animations[JUMP].currentFrame = animations[JUMP].firstFrame; // Reset jump animation.
            velocity.y = -250.0f; // Set vertical velocity for jump.
            velocity.x *= 0.5f; // Reduce horizontal speed during jump.
            PlaySound(jumpSound);
    
            if (isRunning) { // Stop running sound when jumping
                StopSound(runSound);
                isRunning = false;
            }
            wasInAir = false; // Reset falling state on jump start
        }
    
        bool moving = false; // Track if the player is moving
    
        if (IsKeyDown(KEY_A)) { // Move Left.
            velocity.x = -moveSpeed;
            direction = LEFT;
            moving = true;
        } 
        else if (IsKeyDown(KEY_D)) { // Move Right.
            velocity.x = moveSpeed;
            direction = RIGHT;
            moving = true;
        }
    
        // Handle Running Sound while on the ground
        if (moving && rect.y >= groundLevel) {
            if (!isRunning) {
                PlaySound(runSound);
                isRunning = true;
            }
            state = RUN;
        } 
        else { // No movement OR airborne
            if (isRunning) { // Stop running sound when movement stops or when airborne
                StopSound(runSound);
                isRunning = false;
            }
            if (rect.y >= groundLevel && state != JUMP) state = IDLE;
        }
    
        // Check for Landing: Play landing sound only when falling
        if (wasInAir && rect.y >= groundLevel) {
            PlaySound(landSound); // Play landing sound after falling
            wasInAir = false; // Reset the falling state
        }
    
        // Attack action
        if (IsKeyPressed(KEY_SPACE)) { // Attack action.
            state = ATTACK;
            animations[state].currentFrame = animations[state].firstFrame;
            PlaySound(attackSound);
    
            if (isRunning) { // Stop running sound if attacking
                StopSound(runSound);
                isRunning = false;
            }
        }
    
        // Handle Jump and Falling States
        if (rect.y < groundLevel) { // **Check if airborne after jumping or falling**
            state = JUMP; // The character is in the air.
            if (!wasInAir) {
                wasInAir = true; // Character is falling now (after jumping)
            }
    
            if (isRunning) { // Stop running sound if in the air
                StopSound(runSound);
                isRunning = false;
            }
        } 
        else if (state == JUMP && rect.y >= groundLevel) {
            state = IDLE; // Return to idle state after landing
        }
    }

    // Helper method to apply velocity changes to the samurai's position.
    void applyVelocity() {
        rect.x += velocity.x * GetFrameTime(); // Apply horizontal velocity.
        rect.y += velocity.y * GetFrameTime(); // Apply vertical velocity.

        if (rect.y < groundLevel) {
            velocity.y += 500.0f * GetFrameTime(); // Apply gravity if in air.
        } else {
            velocity.y = 0.0f; // Stop vertical movement if on ground.
            rect.y = groundLevel; // Reset position to ground level.
        }

        if (state == ATTACK && animations[state].currentFrame == animations[state].lastFrame) {
            state = IDLE; // Return to idle after attack animation completes.
        }
    }

    // Helper method to handle damage logic.
    void takeDamage(int damage) {
        if (isDead) return; // Prevent damage if already dead.

        currentHealth -= damage; // Apply damage to health.

        if (currentHealth <= 0) {
            currentHealth = 0;
            isDead = true; // Set character to dead.
            state = DEAD; // Set state to dead.
            PlaySound(deadSound);

            if (isRunning) { // Stop running sound when dying
                StopSound(runSound);
                isRunning = false;
            }
        } else {
            state = HURT; // Change state to hurt when damaged.
            animations[HURT].currentFrame = animations[HURT].firstFrame; // Reset hurt animation.
            animations[HURT].timeLeft = animations[HURT].speed; // Reset animation time.
            PlaySound(hurtSound);

            if (isRunning) { // Stop running sound when hurt
                StopSound(runSound);
                isRunning = false;
            }
        }
    }

    // Helper method to heal the samurai.
    void heal(int healingAmount) {
        if (isDead) return; // Prevent healing if dead.

        currentHealth += healingAmount; // Increase health.
        if (currentHealth > maxHealth) {
            currentHealth = maxHealth; // Cap health at max value.
        }
    }

    // Helper method to check for healing input.
    void checkForHealing() {
        if (IsKeyPressed(KEY_H)) { // Press 'H' to heal.
            heal(20); // Heal 20 health points.
        }
    }

    // Helper method to simulate taking damage for testing.
    void checkForDamage() {
        if (IsKeyPressed(KEY_K)) {  // Press 'K' to simulate enemy attack.
            takeDamage(20);          // Apply 20 damage.
        }
    }

public:
    Samurai(Vector2 position) {
        rect = (Rectangle){position.x, position.y, 64.0f, 64.0f}; // Set initial position and size.
        velocity = (Vector2){0.0f, 0.0f}; // Initialize velocity.
        direction = RIGHT; // Default facing direction.
        state = IDLE; // Start in idle state.
        groundLevel = position.y; // Set initial ground level.

        // Initialize animations for different states.
        animations = {
            {0, 2, 0, 0, 0.1f, 0.1f, ONESHOT}, // Dead animation.
            {0, 3, 0, 0, 0.1f, 0.1f, ONESHOT}, // Attack animation.
            {0, 1, 0, 0, 0.1f, 0.1f, ONESHOT}, // Hurt animation.
            {0, 5, 0, 0, 0.1f, 0.1f, REPEATING}, // Idle animation.
            {0, 11, 0, 0, 0.1f, 0.1f, ONESHOT}, // Jump animation.
            {0, 7, 0, 0, 0.1f, 0.1f, REPEATING} // Run animation.
        };
    }

    ~Samurai() {
        for (auto& sprite : sprites) {
            UnloadTexture(sprite); // Unload textures from memory when done.
        }

        // Unload Sounds.
        UnloadSound(deadSound);
        UnloadSound(jumpSound);
        UnloadSound(hurtSound);
        UnloadSound(runSound);
        UnloadSound(attackSound);
        UnloadSound(landSound);
    }

    // Load textures for each state.
    void loadTextures() {
        sprites.resize(6);

        // Load textures for different states (idle, attack, etc.).
        sprites[DEAD] = LoadTexture("assets/Samurai/Dead.png");
        sprites[ATTACK] = LoadTexture("assets/Samurai/Attack_2.png");
        sprites[HURT] = LoadTexture("assets/Samurai/Hurt.png");
        sprites[IDLE] = LoadTexture("assets/Samurai/Idle.png");
        sprites[JUMP] = LoadTexture("assets/Samurai/Jump.png");
        sprites[RUN] = LoadTexture("assets/Samurai/Run.png");
    }

    void loadSounds() {
        deadSound = LoadSound("sounds/samurai/female-death.wav");
        hurtSound = LoadSound("sounds/samurai/female-hurt-2-94301.wav");
        jumpSound = LoadSound("sounds/samurai/female-jump.wav");
        attackSound = LoadSound("sounds/samurai/sword-sound-2-36274.wav");
        runSound = LoadSound("sounds/samurai/running-on-concrete-268478.wav");
        landSound = LoadSound("sounds/samurai/land2-43790.wav");
    }

    // Draw the samurai's current animation frame.
    void draw() const {
        Rectangle source = getAnimationFrame(); // Get the current animation frame.
        float scale = 2.0f; // Scale the character's size.

        Rectangle dest = { // Destination rectangle for drawing.
            rect.x, rect.y, 
            rect.width * scale,   
            rect.height * scale   
        };

        // Flip sprite if facing left.
        source.width *= static_cast<int>(direction);

        // Draw the sprite.
        DrawTexturePro(sprites[state], source, dest, {0, 0}, 0.0f, WHITE);
    }

    // Draw the health bar.
    void drawHealthBar() {
        float healthPercentage = (float)currentHealth / maxHealth;
        DrawRectangle(20, 20, 200, 20, DARKGRAY); // Health bar background.
        DrawRectangle(20, 20, (int)(200 * healthPercentage), 20, RED); // Health bar foreground.
        DrawText(TextFormat("Health: %d/%d", currentHealth, maxHealth), 20, 45, 20, WHITE); // Display health.
    }

    // Update the samurai's state and perform actions.
    void updateSamurai() {
        if (!isDead) {
            move(); // Allow movement if not dead.
            applyVelocity(); // Apply velocity to update position.
        } else {
            velocity.x = 0; // Stop movement if dead.
            velocity.y = 0;
        }

        updateAnimation(); // Update the animation state.
        checkForHealing(); // Check if the healing key is pressed.
        checkForDamage(); // Check if the damage key is pressed.
    }
    
    // Returns the center position of the samurai
    // Added getter method to retrieve the current position.
    Vector2 getPosition() const 
    {
        return 
        { 
            rect.x + rect.width / 2.0f, rect.y + rect.height / 2.0f 
        };
    }
};