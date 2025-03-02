# Aseprite Animation Integration Guide

This guide provides step-by-step instructions on how to use Aseprite animations in your game.

## Prerequisites

- [Aseprite](https://www.aseprite.org/) (commercial or compiled from source)
- Raylib game framework
- C++17 compatible compiler

## Quick Start

1. Create your animations in Aseprite
2. Export them using our script: `./export_aseprite.sh path/to/your/file.aseprite`
3. Place the exported PNG and JSON files in your game's assets directory
4. Use the `AsepriteAnimation` class to load and display your animations

## Detailed Instructions

### 1. Creating Animations in Aseprite

1. Create a new sprite in Aseprite
2. Create separate frames for your animation
3. Use the Timeline panel to organize your frames into "Tags" (these will be your animation states)
4. Common tags to create:
   - `idle`: Character standing still
   - `walk`: Walking animation
   - `attack`: Attack animation
   - `jump`: Jumping animation
   - `death`: Death animation

### 2. Exporting Animations

#### Option 1: Using our script (recommended)

```bash
./export_aseprite.sh path/to/your/file.aseprite
```

This will create two files:
- `filename_sheet.png`: The sprite sheet image
- `filename_sheet.json`: The animation data in JSON format

#### Option 2: Manual export from Aseprite

1. Open your animation in Aseprite
2. Go to `File > Export Sprite Sheet`
3. In the export dialog:
   - Set Layout to "Horizontal Strip"
   - Check "JSON Data" and set Format to "Array"
   - Set the output paths for both the PNG and JSON files
   - Click "Export"

### 3. Integrating in Your Game

#### Basic Usage

```cpp
#include "AsepriteLoader.h"

// Create an animation object
AsepriteAnimation animation;

// Load the animation from exported files
if (animation.LoadFromFiles("character_sheet.png", "character_sheet.json")) {
    // Play a specific animation tag (defined in Aseprite)
    animation.Play("idle");
}

// In your game loop:
void Update() {
    // Update animation with delta time
    animation.Update(GetFrameTime());
}

void Draw() {
    // Draw the current frame
    animation.Draw(400, 300, 2.0f);  // x, y, scale
    
    // Or with flipping
    animation.DrawEx(400, 300, true, 2.0f);  // x, y, flipX, scale
}
```

#### Using the AsepriteCharacter Class

We've included an example character class that shows how to integrate Aseprite animations:

```cpp
#include "AsepriteExample.h"

// Create a character
AsepriteCharacter character(Vector2{400, 300});

// Load animations
character.LoadAnimations("character_sheet.png", "character_sheet.json");

// In your game loop:
void Update() {
    character.Update(GetFrameTime());
}

void Draw() {
    character.Draw();
}
```

### 4. Testing Your Animations

Run the test program to see if your animations work correctly:

```bash
make aseprite_test && ./aseprite_test
```

This will look for animation files in:
- `assets/character_sheet.png` and `assets/character_sheet.json`
- `assets/Goblin/goblin_sheet.png` and `assets/Goblin/goblin_sheet.json`

## Advanced Usage

### Animation Tags

In Aseprite, tags define different animations. When exporting, these tags are preserved in the JSON file and can be accessed by name:

```cpp
// Play different animations
animation.Play("idle");
animation.Play("walk");
animation.Play("attack");
```

### Animation Direction

Aseprite supports different animation directions:
- `forward`: Plays frames in order
- `reverse`: Plays frames in reverse order
- `pingpong`: Plays frames forward, then backward

These are automatically handled by the `AsepriteAnimation` class.

### Looping

You can control whether animations loop:

```cpp
// Loop animation (default)
animation.Play("walk", true);

// Play once and stop
animation.Play("attack", false);
```

### Checking Animation State

```cpp
// Check if animation is still playing
if (!animation.IsPlaying()) {
    // Animation has finished
}

// Get current animation tag
std::string currentTag = animation.GetCurrentTag();
```

## Troubleshooting

- If animations aren't showing, check that both the PNG and JSON files exist and are in the correct format.
- Make sure your Aseprite tags are correctly defined.
- Check the console for any error messages from the loader.
- If you're seeing errors about missing files, verify the file paths.

## Additional Resources

- [Aseprite Documentation](https://www.aseprite.org/docs/)
- [Raylib Documentation](https://www.raylib.com/cheatsheet/cheatsheet.html)
- [JSON Format Reference](https://github.com/aseprite/aseprite/blob/master/docs/ase-file-specs.md#json-data) 