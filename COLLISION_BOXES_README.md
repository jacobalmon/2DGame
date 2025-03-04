# Collision and Hit Box Improvements

This document explains the improvements made to the collision and hit box system for character animations in the game.

## Overview

We've enhanced the collision detection system by implementing separate collision and hit boxes for each character. This provides several benefits:

1. **More Accurate Collisions**: Collision boxes are now smaller than the visual character, matching their actual physical presence.
2. **Dynamic Hit Boxes**: Hit boxes change based on the character's current animation and state.
3. **Visual Debugging**: A debug mode allows you to see the collision and hit boxes in real-time.
4. **Improved Combat**: Combat now uses hit boxes for attack detection, making combat feel more precise.

## Implementation Details

### Box Types

Each character now has three types of boxes:

1. **Character Rectangle (`rect`)**: The main rectangle that defines the character's position and size for rendering.
2. **Collision Box (`collisionBox`)**: A smaller rectangle used for movement and physics collisions.
3. **Hit Box (`hitBox`)**: A dynamic rectangle that changes based on the character's attack state.

### Dynamic Hit Boxes

Hit boxes are dynamically updated based on the character's current state:

#### Samurai
- **Attack**: The hit box extends in front of the character in the direction they're facing.
- **Parry**: The hit box covers the character's body to detect incoming attacks.
- **Other States**: No hit box is active.

#### Goblin
- **Club Attack**: The hit box extends in front of the goblin.
- **Stomp Attack**: The hit box extends downward.
- **AOE Attack**: The hit box is larger and surrounds the goblin.
- **Other States**: No hit box is active.

### Debug Visualization

Press F1 during gameplay to toggle debug mode, which shows:
- **Green Box**: Collision box for movement/physics
- **Red Box**: Hit box for attacks
- **Blue Box**: Character rectangle for positioning

## How to Use

1. **Movement Collisions**: Use `getCollisionBox()` instead of `getRect()` for movement-related collision detection.
2. **Attack Collisions**: Use `getHitBox()` for attack-related collision detection.
3. **Debug Mode**: Toggle with F1 key or use `setDebugBoxes(true/false)` in code.

## Code Example

```cpp
// Handle movement collisions using collision boxes
if(checkCharacterCollision(samurai.getCollisionBox(), goblin.getCollisionBox())) {
    // Basic collision resolution
    samurai.setPosition((Vector2){prevSamuraiRect.x, prevSamuraiRect.y});
    goblin.setPosition((Vector2){prevGoblinRect.x, prevGoblinRect.y});
}

// Handle attack collisions using hit boxes
if (samurai.getState() == ATTACK_SAMURAI && 
    !goblin.getIsInvulnerable() && 
    samurai.getHitBox().width > 0 && 
    checkCharacterCollision(samurai.getHitBox(), goblin.getCollisionBox())) {
    
    goblin.takeDamage(samurai.getAttackDamage());
    goblin.setVelocity((Vector2){(float)samurai.getDirection() * 200.0f, -100.0f});
}
```

## Benefits

1. **More Precise Combat**: Attacks only hit when the weapon actually connects with the opponent.
2. **Better Movement**: Characters don't collide when they're just visually overlapping but not physically touching.
3. **Improved Parrying**: The parry system now uses hit boxes to detect when an attack is successfully blocked.
4. **Easier Debugging**: Visual feedback makes it easier to adjust and fine-tune the collision system.

## Future Improvements

1. **Animation-Specific Hit Boxes**: Define hit boxes for each frame of animation for even more precision.
2. **Hitbox Editor**: Create a tool to visually edit hit boxes for each animation frame.
3. **Collision Layers**: Implement a layer system to control which objects can collide with each other.
4. **Pixel-Perfect Collision**: For very precise collision detection, implement pixel-perfect collision using the alpha channel of the sprites. 