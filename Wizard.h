#include <raylib.h>
#include <vector>

const float GRAVITY_WIZARD = 800.0f;
const float JUMP_FORCE_WIZARD = -400.0f;
const float GROUND_LEVEL_WIZARD = 200.0f; 

enum DirectionWizard {
    LEFT_WIZARD = -1,
    RIGHT_WIZARD = 1
};

enum CurrentStateWizard {
    DEAD_WIZARD = 0,
    ATTACK1_WIZARD = 1,
    ATTACK2_WIZARD = 2,
    HURT_WIZARD = 3,
    IDLE_WIZARD = 4,
    JUMP_WIZARD = 5,
    RUN_WIZARD = 6
};

enum AnimationTypeWizard {
    REPEATING_WIZARD,
    ONESHOT_WIZARD
};

struct AnimationWizard {
    int firstFrame, lastFrame, currentFrame, offset;
    float speed, timeLeft;
    AnimationTypeWizard type;
};

class Wizard {
    public:
        Rectangle rect;
        Vector2 velocity;
        DirectionWizard direction;
        CurrentStateWizard state;
        bool isOnGround = true;
        int health = 100; // Wizard's health

        std::vector<AnimationWizard> animations;
        std::vector<Texture2D> sprites;

        bool isAttacking = false;
        bool hasFinishedAttack = true;

        Wizard(Vector2 position) {
            rect = {position.x, position.y, 64.0f, 64.0f};  
            velocity = {0.0f, 0.0f}; 
            direction = RIGHT_WIZARD; 
            state = IDLE_WIZARD; 

            animations = {
                {0, 6, 0, 0, 0.1f, 0.1f, ONESHOT_WIZARD}, // DEAD
                {0, 7, 0, 0, 0.1f, 0.1f, ONESHOT_WIZARD}, // ATTACK1
                {0, 7, 0, 0, 0.1f, 0.1f, ONESHOT_WIZARD}, // ATTACK2
                {0, 2, 0, 0, 0.1f, 0.1f, ONESHOT_WIZARD}, // HURT
                {0, 7, 0, 0, 0.1f, 0.1f, REPEATING_WIZARD}, // IDLE
                {0, 1, 0, 0, 0.1f, 0.1f, ONESHOT_WIZARD}, // JUMP
                {0, 7, 0, 0, 0.1f, 0.1f, REPEATING_WIZARD} // RUN
            }; 
        }

        ~Wizard() {
            for (auto& sprite : sprites) {
                UnloadTexture(sprite);
            }
        }

        void loadTextures() {
            sprites.resize(7);

            sprites[DEAD_WIZARD] = LoadTexture("assets/Wizard/Sprites/Death.png");
            sprites[ATTACK1_WIZARD] = LoadTexture("assets/Wizard/Sprites/Attack1.png");
            sprites[ATTACK2_WIZARD] = LoadTexture("assets/Wizard/Sprites/Attack2.png");
            sprites[HURT_WIZARD] = LoadTexture("assets/Wizard/Sprites/Take hit.png");
            sprites[IDLE_WIZARD] = LoadTexture("assets/Wizard/Sprites/Idle.png");
            sprites[JUMP_WIZARD] = LoadTexture("assets/Wizard/Sprites/Jump.png");
            sprites[RUN_WIZARD] = LoadTexture("assets/Wizard/Sprites/Run.png");
        }

        void takeDamage(int damage) {
            health -= damage;
            if (health <= 0) {
                health = 0;
                state = DEAD_WIZARD;
                hasFinishedAttack = true; // Prevent further actions
            } else {
                state = HURT_WIZARD;
                animations[HURT_WIZARD].currentFrame = animations[HURT_WIZARD].firstFrame;
            }
        }

        void updateAnimation() {
            AnimationWizard& anim = animations[state];
            float deltaTime = GetFrameTime();
            anim.timeLeft -= deltaTime;
        
            if (anim.timeLeft <= 0) {
                anim.timeLeft = anim.speed;
                anim.currentFrame++;
        
                if (anim.currentFrame > anim.lastFrame) {
                    if (anim.type == REPEATING_WIZARD) {
                        anim.currentFrame = anim.firstFrame;
                    } else {
                        anim.currentFrame = anim.lastFrame;
                        hasFinishedAttack = true;
        
                        // Reset state after being hurt
                        if (state == HURT_WIZARD) {
                            state = IDLE_WIZARD;
                        }
                    }
                }
            }
        }        

        Rectangle getAnimationFrame() const {
            const AnimationWizard& anim = animations[state];
            int frameWidth = sprites[state].width / (anim.lastFrame + 1);
            int frameHeight = sprites[state].height;
    
            return { (float)frameWidth * anim.currentFrame, 0, (float)frameWidth, (float)frameHeight };
        }

        void draw() const {
            Rectangle source = getAnimationFrame();
            float scale = 5.0f;
    
            Rectangle dest = { rect.x, rect.y, rect.width * scale, rect.height * scale };
    
            if (direction == LEFT_WIZARD) {
                source.x += source.width;
                source.width = -source.width;
            }
    
            DrawTexturePro(sprites[state], source, dest, { 0, 0 }, 0.0f, WHITE);
        }

        void move() {
            if (!hasFinishedAttack || state == HURT_WIZARD || state == DEAD_WIZARD) return; // Prevent movement if hurt or dead
        
            float moveSpeed = 300.0f;
            velocity.x = 0.0f;
        
            if (IsKeyDown(KEY_N)) {
                velocity.x = -moveSpeed;
                direction = LEFT_WIZARD;
                if (isOnGround) state = RUN_WIZARD;
            } 
            else if (IsKeyDown(KEY_M)) {
                velocity.x = moveSpeed;
                direction = RIGHT_WIZARD;
                if (isOnGround) state = RUN_WIZARD;
            } 
            else if (isOnGround && state != DEAD_WIZARD) { // Prevent switching to idle if dead
                state = IDLE_WIZARD;
            }
        
            if (IsKeyPressed(KEY_J) && isOnGround) {
                velocity.y = JUMP_FORCE_WIZARD;
                state = JUMP_WIZARD;
                isOnGround = false;
                animations[JUMP_WIZARD].currentFrame = animations[JUMP_WIZARD].firstFrame;
            }
        
            if (IsKeyPressed(KEY_KP_7) && hasFinishedAttack) {
                state = ATTACK1_WIZARD;
                hasFinishedAttack = false;
                velocity.x = 0;
                animations[ATTACK1_WIZARD].currentFrame = animations[ATTACK1_WIZARD].firstFrame;
            }
        
            if (IsKeyPressed(KEY_KP_8) && hasFinishedAttack) {
                state = ATTACK2_WIZARD;
                hasFinishedAttack = false;
                velocity.x = 0;
                animations[ATTACK2_WIZARD].currentFrame = animations[ATTACK2_WIZARD].firstFrame;
            }
        
            if (IsKeyPressed(KEY_H)) {
                takeDamage(20);
            }
        }        
        
        void applyVelocity() {
            float deltaTime = GetFrameTime();
    
            velocity.y += GRAVITY_WIZARD * deltaTime;
    
            rect.x += velocity.x * deltaTime;
            rect.y += velocity.y * deltaTime;
    
            if (rect.y >= GROUND_LEVEL_WIZARD) {
                rect.y = GROUND_LEVEL_WIZARD;
                velocity.y = 0;
                isOnGround = true;
                if (state == JUMP_WIZARD) {
                    state = IDLE_WIZARD;
                }
            }
        }
};