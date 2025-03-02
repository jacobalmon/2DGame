# Aseprite Character Animation Integration Guide

This guide explains how to use Aseprite animations with the character classes in the game.

## Overview

The game now supports loading and displaying character animations from Aseprite files. This provides several benefits:

- Better animation control with frame-by-frame animation
- Support for animation tags and directions
- Easier to add new animations and characters
- Consistent animation system across all characters

## Quick Start

1. Export your Aseprite animations using the provided scripts:
   ```
   ./export_samurai.sh
   ./export_goblin.sh
   ```

2. Compile and run the character test program:
   ```
   make aseprite_character_test
   ./aseprite_character_test
   ```

## Character Classes

Two new character classes have been created that use the Aseprite animation system:

1. `SamuraiAseprite` - A player-controlled character with animations for idle, walk, run, jump, attack, parry, hurt, and dead states.
2. `GoblinAseprite` - An enemy character with animations for idle, walk, attack (3 types), hurt, and dead states.

## How to Use

### Exporting Animations

The export scripts (`export_samurai.sh` and `export_goblin.sh`) will:

1. Extract frames from the Aseprite files
2. Combine them into a single sprite sheet
3. Generate a JSON file with animation data
4. Save the output to the `assets/[Character]/exported/` directory

### Loading Animations in Code

To load animations for a character:

```cpp
SamuraiAseprite samurai(Vector2{300, 400});
samurai.loadAnimations("assets/Samurai/exported/samurai_sheet.png", 
                       "assets/Samurai/exported/samurai_sheet.json");
```

### Updating and Drawing

In your game loop:

```cpp
// Update character state
samurai.update(GetFrameTime());

// Apply physics
samurai.applyVelocity(GetFrameTime());

// Draw the character
samurai.draw();
```

### Animation Tags

The following animation tags are supported:

#### Samurai
- `idle` - Standing idle
- `walk` - Walking
- `run` - Running
- `jump` - Jumping
- `attack` - Attacking
- `parry` - Blocking/parrying
- `hurt` - Taking damage
- `dead` - Death animation

#### Goblin
- `idle` - Standing idle
- `walk` - Walking
- `attack_club` - Club attack
- `attack_stomp` - Stomp attack
- `attack_aoe` - Area of effect attack
- `hurt` - Taking damage
- `dead` - Death animation

## Adding New Characters

To add a new character with Aseprite animations:

1. Create a new class based on the `SamuraiAseprite` or `GoblinAseprite` template
2. Create an export script for the character's animations
3. Update the character class to use the appropriate animation tags

## Troubleshooting

- If animations don't load, check that the export scripts have been run and the PNG/JSON files exist
- Make sure the animation tags in your code match the ones in the JSON file
- If animations look wrong, check that the sprite sheet is correctly formatted

## Dependencies

- ImageMagick (for the export scripts)
- jq (for JSON processing in the export scripts)
- Aseprite command-line tool (for exporting from Aseprite files)

## Further Improvements

- Add support for multiple animation directions
- Implement animation blending for smoother transitions
- Add support for animation events (e.g., for attack hit frames)
- Create a more generic character base class 