# Retro Raycasting Dungeon Crawler
#### Video Demo: <URL HERE>
#### Description:

This project is an old-school dungeon crawler maze escape game featuring hand-drawn pixel art and a turn-based combat system. The player spawns at a designated point on a map, navigates through narrow corridors, encounters enemies, and attempts to find the exit gate. (Though, as the project is still under active development, some mechanics are still being refined.)

Most of the technical and architectural choices were guided simply by how I wanted to explore and learn new concepts. After all, the core goal of CS50 is learning by doing. The engine will continue to improve and will eventually be polished enough for a release on itch.io.

---

### Technical Approach & Solutions
#### DDA Raycasting:

The 3D perspective is essentially an optical illusion created by projecting 2D rays from the player's position and drawing vertical slices on the screen based on each ray's length.

Instead of moving a ray forward pixel by pixel, the engine implements the **Digital Differential Analysis (DDA) algorithm** . DDA advances the ray by grid boundaries (where walls could actually exist), reaching the target in just a few iterations rather than hundreds.

To illustrate why DDA is vastly superior:
Imagine a 10x10 map where each grid cell is 100 pixels wide. In pixel space, the map is 1000x1000 pixels. If an algorithm advances the ray 1 pixel at a time, calculating a ray traveling across the grid could easily require up to 800-900 iterations just to find a single wall hit.

DDA, on the other hand, jumps directly from one grid grid-line to the next. In the worst-case scenario on the same map, it checks only about 9 to 10 grid boundaries, making the computation lightweight and running smoothly in real-time.

For more details on the algorithmic breakdown, see: https://lodev.org/cgtutor/raycasting.html

#### Third-Party Libraries & Dependencies:

* **[Raylib](https://www.raylib.com/):** Used for window creation, user input handling, and 2D drawing primitives.

---

### Limitations & Known Issues

> **Note on Code Comments:** The comments throughout the codebase are currently written in Turkish (my native language). As the engine and systems expanded, keeping personal technical notes in my mother tongue allowed me to understand, debug, and iterate on complex math and rendering logic much faster during active development. These comments will be refactored into English in future iterations as the project matures toward an itch.io release.

While DDA raycasting itself is a well-documented technique, layering an atmospheric dynamic torch lighting effect on top was challenging. Although I experimented with various AI-assisted formulas, I intentionally chose to leave certain implementations in their current state so I could fully understand and own the codebase:

* **Floor & Ceiling Reflections:** The current vertical gradient implementation creates a slight shiny/reflective floor look rather than a flat matte texture.

* **Corner Clipping:** Moving into wall corners at specific angles can occasionally clip into collision boundaries.

* **Victory Screen Rendering:** The victory screen uses the main splash art asset and currently executes its render calls inside the Update lifecycle branch.

* **Combat State Quirks:** When player HP reaches zero, the state currently defaults to the victory screen.

* **Pause Menu Behavior:** Pressing Tab returns the player directly to the main menu screen rather than a separate pause overlay.

Drawing the pixel art by hand was time-consuming and limited how much content could be finalized before the deadline, leaving plenty of room for future polish.

---

### Core Game Loop Architecture

The underlying structure is built around a lightweight finite state machine handled by two core member functions in the Game class:

* **Update():** Checks the active state (STATE_MENU, STATE_EXPLORATION, STATE_COMBAT, STATE_VICTORY) and delegates execution to that state's dedicated update method, handling input, timers, and movement.

* **Draw():** Evaluates the active state and dictates what gets rendered to the screen.

Every gameplay mechanic, transition, and render pass flows directly through this straightforward state-driven pattern.

---
*Note: This documentation was originally drafted in Turkish and translated into English with the assistance of an AI tool.*
