![EKFB7789](https://github.com/user-attachments/assets/8086c965-b3ab-4619-91e8-f7d19dc50e9e)

**YouTube:** https://studio.youtube.com/video/bC17AgROvXw/edit

**Instructable:** https://www.instructables.com/Conways-Game-of-Life-XL-Screen-Raspberry-Pi-Projec/

### ▶ [Try the Web Simulator](https://htmlpreview.github.io/?https://github.com/lonesoulsurfer/Conways_Game_of_Life__Large_Screen/blob/main/Game_of_Life_Simulation.html) — no hardware required, runs in your browser

---

## Contents

- [Hardware & Setup](#hardware--setup)
- [Web Simulator](#web-simulator)
- [Main Menu](#main-menu)
- [Standard Controls](#standard-controls-conway--alt-games--rule-explorer)
- [Conway's Life Modes](#conways-life-modes)
- [Alternative Cellular Automata (Alt Games)](#alternative-cellular-automata-alt-games)
- [Lenia Sandbox](#lenia-sandbox)
- [Tools Menu](#tools-menu)
- [Arcade Games](#arcade-games)

---

## Hardware & Setup

The device runs on a Raspberry Pi Pico / RP2040 connected to a 3.5" ST7796S/ST7796U TFT display (480×320 pixels) via SPI. Six buttons handle all navigation and gameplay: Up, Down, Left, Right, SET (A), and B. A buzzer on GPIO 26 provides sound effects and music. Display brightness is PWM-controlled, and all settings (sound, volume, brightness) are saved to EEPROM so they persist between power cycles.

## Web Simulator

Don't have the hardware? [**Game_of_Life_Simulation.html**](Game_of_Life_Simulation.html) is a single-file, browser-based recreation of the firmware that runs on any modern desktop browser — [click here to launch it directly](https://htmlpreview.github.io/?https://github.com/lonesoulsurfer/Conways_Game_of_Life__Large_Screen/blob/main/Game_of_Life_Simulation.html), or download the file and open it locally.

It's a faithful port of the firmware's menu logic and simulation rules, built directly from this repo's source — not a from-scratch reimplementation. It covers:

- The full Conway's Life menu shell: Presets, Random, Symmetric, Custom, and Rule Explorer (including the Custom Rule bit editor)
- All seven Alt Games automata: Brian's Brain, Day & Night, Seeds, Cyclic CA, Wireworld, Langton's Ant, and Wolfram 1D
- The Lenia Sandbox, with its own live parameter panel and control scheme
- The Tools menu (Grid Lines, Population Counter, Trail Mode, Colour Mode, Toroidal World)

On-screen buttons mirror the physical Up/Down/Left/Right/A/B layout, and the same controls also work from a keyboard (arrow keys, A/Enter/Space, B) — including two-button combos like Up+Down for colour mode.

**Not included:** Arcade Games (Star Wars, Breakout Beyond, Gyruss) and hardware-only features like sound and screen brightness, which don't have a meaningful browser equivalent.

## Main Menu

On startup the device boots directly into the main scrolling menu with eight sections:

| # | Section | Description |
|---|---|---|
| 1 | Presets | Hand-crafted Conway's Life patterns |
| 2 | Random | Randomised Conway's Life |
| 3 | Symmetric | Procedurally generated symmetric patterns |
| 4 | Custom | Draw your own pattern |
| 5 | Rule Explorer | Define custom birth/survival rules |
| 6 | Alt Games | Alternative cellular automata (including Lenia Sandbox) |
| 7 | Tools | Display and simulation settings |
| 8 | Arcade | Three built-in arcade games |

Navigate with Up/Down, select with SET, go back with B.

## Standard Controls (Conway / Alt Games / Rule Explorer)

These controls apply to Conway's Life modes, Rule Explorer, and every Alt Games automaton except Lenia Sandbox, which has its own control scheme. See the [Lenia Sandbox](#lenia-sandbox) section.

| Button | Action |
|---|---|
| Up | Increase simulation speed |
| Down | Decrease simulation speed |
| Left | Cycle cell size (Small 3px, Normal 4px, Large 8px) |
| Right | Reset / regenerate the current pattern |
| Up + Down | Toggle colour mode |
| SET (hold) | Start simulation from edit mode |
| SET (hold, running) | Return to edit mode (Custom / Seeds / Brian's Brain only) |
| B (short press) | Return to menu |
| B (hold ~1 second) | Show/hide rules and information panel |

## Conway's Life Modes

### Presets (13 patterns)

Press SET once to open a preset, then A to start the simulation. While paused you can press Left to change cell size (the pattern rescales and redraws) or Right to reset it back to its starting state.

| # | Pattern | Notes |
|---|---|---|
| 1 | Coe Ship | Puffer-type spaceship that leaves debris trails |
| 2 | Gosper Gun | The first glider gun ever discovered, period 30 |
| 3 | Diamond | Symmetric expanding diamond pattern |
| 4 | Achim p144 | Rare period-144 oscillator |
| 5 | 56P6H1V0 | High-period spaceship |
| 6 | LWSS Convoy | Three Lightweight Spaceships in formation |
| 7 | MWSS | Middleweight Spaceship, 8 cells, speed c/2 |
| 8 | Pulsar | Period-3 oscillator with 4-fold symmetry, 48 cells |
| 9 | Pentadecathlon | 15-cell oscillator made from a modified 10-cell row |
| 10 | R-Pentomino | 5-cell seed that runs for 1,103 generations |
| 11 | Acorn | 7-cell methuselah running 5,206 generations |
| 12 | Simkin Gun | Smallest known glider gun, 36 cells, period 120 |
| 13 | Queen Bee | Period-30 oscillator, the first of its kind ever found |

### Random

Fills the board with random live cells at approximately 30% density. Immediately starts running. Press Right to re-randomise.

### Symmetric

Generates a random pattern with one of three symmetry types (vertical, horizontal, or rotational) in small, medium, or large sizes. Good for producing structured chaos.

### Custom

Puts the board into edit mode with a blinking red crosshair cursor. Move the cursor with Up/Down/Left/Right. Press SET to toggle a cell on or off. When happy with your pattern, hold SET for about a second to start the simulation. Hold SET again to return to edit mode. Press B to go back to the menu.

### Rule Explorer

Define your own cellular automaton by setting custom Birth (B) and Survival (S) rules, the same notation used by Life (B3/S23). Choose from 12 built-in rule presets or enter your own bit-by-bit. The simulation runs with your rules applied to a random starting board.

**Included preset rules:** High Life, 34 Life, Diamoeba, Replicator, Long Life, Maze, Coral, 2x2, Dry Life, Amoeba, Coagulate, Gnarl

## Alternative Cellular Automata (Alt Games)

**Brian's Brain.** A three-state automaton (On, Dying, Off) producing a constantly moving stream of light-speed "signals." Choose from small, medium, large, or random seedings, or draw a custom starting pattern.

**Day & Night** (B3678/S34678). A symmetric ruleset where dead and live regions are interchangeable. Dense regions survive and grow in a way that mirrors the original, creating organic, blob-like patterns.

**Seeds** (B2/S). Every live cell dies every generation but any dead cell with exactly two live neighbours is born. Creates explosive, constantly moving patterns that never stabilise.

**Cyclic CA.** A multi-state automaton where cells cycle through states 0→1→2→…→N→0. Cells advance only when they have a neighbour in the next state. Produces rotating spiral waves. Choose small, medium, or large seedings.

**Wireworld.** A four-state automaton (Empty, Wire, Electron Head, Electron Tail) that models digital logic circuits. Electrons travel along wires, interact at junctions, and can form AND gates and clocks. The built-in demo includes a working circuit layout.

**Langton's Ant.** A single ant on a grid follows two rules: turn right on a white cell and flip it black, turn left on a black cell and flip it white. After roughly 10,000 chaotic steps it spontaneously builds a repeating diagonal "highway." Toggle age-colour mode with Up+Down to see a heat map of how often each cell has been visited.

**Wolfram 1D Automata.** One-dimensional elementary cellular automata: a single row of cells evolves downward according to one of 255 possible rules. Each rule produces a unique pattern, from pure chaos (Rule 30) to the Sierpinski triangle (Rule 90) to a Turing-complete system (Rule 110). Seven named presets are provided, or you can enter any rule number directly.

**Lenia Sandbox.** A continuous cellular automaton with its own control scheme, described below.

## Lenia Sandbox

Unlike every automaton above, Lenia isn't discrete. Cells hold a value from 0-255 instead of a simple on/off state, and each step blends that value with a weighted neighbourhood (the kernel) instead of applying binary birth/survival rules. The result is smooth, organic life forms that grow and drift.

The play area occupies the right two-thirds of the screen, with a live parameter panel on the left third.

**Parameters** (adjust with Up/Down to select, Left/Right to change value):

| Parameter | Meaning |
|---|---|
| Mu | Growth centre: the neighbourhood value that produces the strongest growth |
| Sigma | Growth width: how forgiving that target value is |
| Radius | How far each cell looks at its neighbours |
| Speed | How much each step nudges the field (time step / dt) |

**Controls:**

| Button | Action |
|---|---|
| Up / Down | Select a parameter row |
| Left / Right | Adjust the selected parameter |
| SET (A) | Reseed the board with fresh random blobs |
| Up + Down | Toggle colour scheme (blue/cyan/white or fire) |
| B (short tap) | Return to the Alt Games menu |
| B (hold ~1 second) | Show/hide the rules and parameter info panel |
| SET + B (hold ~1 second) | Full reset: parameters back to defaults, fresh seed |

Honours the shared Toroidal World setting from the Tools menu (wrap edges on/off) and the Auto-Cycle screensaver tool (periodically reseeds on its own, same as the other automata).

## Tools Menu

**Grid Lines.** Overlays a faint grid matching the current cell size.

**Population Counter.** Shows live cell count and generation number during simulation.

**Trail Mode.** Dying cells leave a soft 20-step fade trail rather than disappearing instantly.

**Colour Mode.** Age-based colouring; young cells are one colour, older cells shift through the spectrum.

**Toroidal World.** When on, edges wrap around so the board is a torus. When off, edges are hard boundaries.

**Brightness.** Four levels: 25%, 50%, 75%, 100%.

**Auto-Cycle / Screensaver.** Automatically switches to a random game mode or Wolfram rule every 300 generations. Useful as a display piece.

**Sound settings.** Enabled/disabled, three volume levels. Also accessible here and saved to EEPROM.

## Arcade Games

Accessed from main menu item 8. Three games are available. In all three, press A + B together at any time to exit directly back to the main menu.

### Star Wars

A first-person Star Wars game inspired by the 80's arcade game. Progress through multiple stages: Womp Rat Training, Space Battle, Death Star Approach, Surface Run, and the Exhaust Port shot. Features full Star Wars music (Main Theme, Binary Sunset, Imperial March, Victory Fanfare), sound effects, TIE fighters, laser cannons, proton torpedoes, and a dramatic finale sequence. Press B on the title screen to exit.

| Button | Action |
|---|---|
| Up/Down/Left/Right | Move crosshair / pilot ship |
| SET | Fire |
| A + B | Exit to main menu |
| Up + Down (title screen) | Enter games menu to play any of the games or watch the cinematic shorts |

### Breakout Beyond

A feature-rich Breakout variant with a neon aesthetic. The paddle sits at the bottom, bricks at the top. Features include combo multipliers, spin physics, multiball, shield power-ups, bomb bricks, and hard bricks across 20 levels.

| Button | Action |
|---|---|
| Left/Right | Move paddle |
| B | Speed boost |
| SET | Launch ball / pause |
| A + B (hold) | Exit to main menu |

### Gyruss

A circular shoot-em-up inspired by the 1983 Konami arcade classic. Your ship orbits around the edge of the screen shooting inward at waves of enemies that fly in formation patterns.

| Button | Action |
|---|---|
| Left/Right | Rotate ship around the ring |
| SET or B | Fire |
| Up (hold) | Smart bomb |
| A + B (hold) | Exit to main menu |
