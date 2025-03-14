#include <raylib.h>
#include <vector>

const float GRAVITY = 800.0f;
const float JUMP_FORCE = -400.0f;
const float GROUND_LEVEL = 400.0f; // Adjust based on your scene

enum DirectionWolf {
    LEFT_WOLF = -1,
    RIGHT_WOLF = 1
};

enum CurrentStateWolf {
    DEAD_WOLF = 0,
    ATTACK_SWIPE,
    ATTACK_RUN,
    HURT_WOLF,
    IDLE_WOLF,
    JUMP_WOLF,
    RUN_WOLF,
    WALK_WOLF
};

enum AnimationTypeWolf {
    REPEATING_WOLF,
    ONESHOT_WOLF
};

struct AnimationWolf {
    int firstFrame, lastFrame, currentFrame, offset;
    float speed, timeLeft;
    AnimationTypeWolf type;
};

class Werewolf {
    public:
        Rectangle rect;
        Vector2 velocity;
        DirectionWolf direction;
        CurrentStateWolf state;
        bool isOnGround = true;
        int health;  // New health member
    
        std::vector<AnimationWolf> animations;
        std::vector<Texture2D> sprites;
    
        bool isAttacking = false;
        bool hasFinishedAttack = true;
        bool isWalkingSoundPlaying = false;

        Sound attackSound;
        Sound runSound;
        Sound hurtSound;
    
        Werewolf(Vector2 position) {
            rect = { position.x, position.y, 64.0f, 64.0f };  
            velocity = { 0.0f, 0.0f }; 
            direction = RIGHT_WOLF; 
            state = IDLE_WOLF; 
            health = 100;  // Initialize health (100 health points)
    
            animations = {
                {0, 1, 0, 0, 0.1f, 0.1f, ONESHOT_WOLF},  
                {0, 3, 0, 0, 0.1f, 0.1f, ONESHOT_WOLF},  
                {0, 6, 0, 0, 0.1f, 0.1f, ONESHOT_WOLF},  
                {0, 1, 0, 0, 0.1f, 0.1f, ONESHOT_WOLF},  
                {0, 7, 0, 0, 0.1f, 0.1f, REPEATING_WOLF},
                {0, 10, 0, 0, 0.1f, 0.1f, ONESHOT_WOLF}, 
                {0, 8, 0, 0, 0.1f, 0.1f, REPEATING_WOLF},
                {0, 10, 0, 0, 0.1f, 0.1f, REPEATING_WOLF} 
            };
        }
    
        ~Werewolf() {
            for (auto& sprite : sprites) {
                UnloadTexture(sprite);
            }

            UnloadSound(attackSound);
            UnloadSound(runSound);
            UnloadSound(hurtSound);
        }
    
        void loadTextures() {
            sprites.resize(8);
    
            sprites[DEAD_WOLF] = LoadTexture("assets/Werewolf/Dead.png");
            sprites[ATTACK_SWIPE] = LoadTexture("assets/Werewolf/Attack_2.png");
            sprites[ATTACK_RUN] = LoadTexture("assets/Werewolf/Run+Attack.png");
            sprites[HURT_WOLF] = LoadTexture("assets/Werewolf/Hurt.png");
            sprites[IDLE_WOLF] = LoadTexture("assets/Werewolf/Idle.png");
            sprites[JUMP_WOLF] = LoadTexture("assets/Werewolf/Jump.png");
            sprites[RUN_WOLF] = LoadTexture("assets/Werewolf/Run.png");
            sprites[WALK_WOLF] = LoadTexture("assets/Werewolf/Walk.png");
        }

        void loadSounds() {
            attackSound = LoadSound("sounds/werewolf/movement-swipe-whoosh-3-186577.wav");
            runSound = LoadSound("sounds/werewolf/run-112647.wav");
            hurtSound = LoadSound("sounds/werewolf/whimper-104684.wav");
        }
    
        void takeDamage(int damage) {
            velocity.x = 0;
            if (state != DEAD_WOLF && state != HURT_WOLF) {
                health -= damage;
                PlaySound(hurtSound);
                if (health <= 0) {
                    health = 0;
                    state = DEAD_WOLF;  // Transition to dead state
                } else {
                    state = HURT_WOLF;  // Transition to hurt state
                    animations[HURT_WOLF].currentFrame = animations[HURT_WOLF].firstFrame;  // Start hurt animation
                }
            }
        }        
    
        void updateAnimation() {
            AnimationWolf& anim = animations[state];
            float deltaTime = GetFrameTime();
            anim.timeLeft -= deltaTime;
        
            if (anim.timeLeft <= 0) {
                anim.timeLeft = anim.speed;
                anim.currentFrame++;
        
                if (anim.currentFrame > anim.lastFrame) {
                    if (anim.type == REPEATING_WOLF) {
                        anim.currentFrame = anim.firstFrame;
                    } else {
                        anim.currentFrame = anim.lastFrame;
        
                        // If in hurt state, transition to idle after hurt animation completes
                        if (state == HURT_WOLF) {
                            state = IDLE_WOLF;  // Transition to idle after hurt animation finishes
                            return;  // Exit to avoid further state change
                        }
                        
                        hasFinishedAttack = true;  // Allow transitioning back to idle or other state
                    }
                }
            }
        }        
    
        Rectangle getAnimationFrame() const {
            const AnimationWolf& anim = animations[state];
            int frameWidth = sprites[state].width / (anim.lastFrame + 1);
            int frameHeight = sprites[state].height;
    
            return { (float)frameWidth * anim.currentFrame, 0, (float)frameWidth, (float)frameHeight };
        }
    
        void draw() const {
            // Get the current frame from the animation based on the current state
            Rectangle source = getAnimationFrame();
            float scale = 2.0f;
        
            // Set the destination rectangle for drawing the sprite
            Rectangle dest = { rect.x, rect.y, rect.width * scale, rect.height * scale };
        
            // Flip the sprite if moving to the left
            if (direction == LEFT_WOLF) {
                source.width = -source.width;
            }
        
            // Draw the current frame of the animation
            DrawTexturePro(sprites[state], source, dest, { 0, 0 }, 0.0f, WHITE);
        }        
    
        void move() {
            // Don't allow movement if we're in the middle of an attack, hurt, or dead state
            if (!hasFinishedAttack || state == DEAD_WOLF || state == HURT_WOLF) return;  // Prevent movement during hurt animation

            float moveSpeed = 300.0f;
            velocity.x = 0.0f;  // Reset horizontal velocity during attack
            
            // Handle movement only if not attacking or hurt
            if (IsKeyDown(KEY_V)) {
                velocity.x = -moveSpeed;
                direction = LEFT_WOLF;

                if (!isWalkingSoundPlaying) {
                    PlaySound(runSound);
                    isWalkingSoundPlaying = true;
                }

                if (isOnGround) {
                    if (state != ATTACK_SWIPE && state != ATTACK_RUN && state != HURT_WOLF) {
                        state = RUN_WOLF;
                    }
                }
            } 
            else if (IsKeyDown(KEY_B)) {
                velocity.x = moveSpeed;
                direction = RIGHT_WOLF;

                if (!isWalkingSoundPlaying) {
                    PlaySound(runSound);
                    isWalkingSoundPlaying = true;
                }

                if (isOnGround) {
                    if (state != ATTACK_SWIPE && state != ATTACK_RUN && state != HURT_WOLF) {
                        state = RUN_WOLF;
                    }
                }
            } 
            else if (isOnGround && state != ATTACK_SWIPE && state != ATTACK_RUN && state != HURT_WOLF) {
                state = IDLE_WOLF;
                if (isWalkingSoundPlaying){
                    StopSound(runSound);
                    isWalkingSoundPlaying = false;
                }
            }
                
            // Jump logic
            if (IsKeyPressed(KEY_G) && isOnGround) {
                velocity.y = JUMP_FORCE;
                state = JUMP_WOLF;
                isOnGround = false;
                animations[JUMP_WOLF].currentFrame = animations[JUMP_WOLF].firstFrame;
            }
        
            // Swipe attack logic
            if (IsKeyPressed(KEY_KP_4) && hasFinishedAttack) {
                state = ATTACK_SWIPE;
                hasFinishedAttack = false;
                velocity.x = 0;
                PlaySound(attackSound);
                animations[ATTACK_SWIPE].currentFrame = animations[ATTACK_SWIPE].firstFrame;
            }
        
            // Run attack logic
            if (IsKeyPressed(KEY_KP_5) && hasFinishedAttack) {
                state = ATTACK_RUN;
                hasFinishedAttack = false;
                velocity.x = 0;
                PlaySound(attackSound);
                animations[ATTACK_RUN].currentFrame = animations[ATTACK_RUN].firstFrame;
            }
        
            // Reset to idle after the attack finishes
            if (hasFinishedAttack && (state == ATTACK_SWIPE || state == ATTACK_RUN)) {
                state = IDLE_WOLF;
            }
        
            // Hurt logic (trigger damage when "H" is pressed)
            if (IsKeyPressed(KEY_H)) {
                takeDamage(10);  // For testing, dealing 10 damage
            }
        }              
    
        void applyVelocity() {
            float deltaTime = GetFrameTime();
    
            velocity.y += GRAVITY * deltaTime;
    
            rect.x += velocity.x * deltaTime;
            rect.y += velocity.y * deltaTime;
    
            if (rect.y >= GROUND_LEVEL) {
                rect.y = GROUND_LEVEL;
                velocity.y = 0;
                isOnGround = true;
                if (state == JUMP_WOLF) {
                    state = IDLE_WOLF;
                }
            }
        }
};    