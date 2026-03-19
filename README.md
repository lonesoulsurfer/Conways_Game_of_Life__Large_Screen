In-Game Controls (All Simulation Modes)

    Up - Increase simulation speed

    Down - Decrease simulation speed

    Left - Cycle cell size (Small 3px → Normal 4px → Large 8px)

    Right - Reset / regenerate the current pattern

    Up + Down - colour mode

    SET (hold) - Start simulation from edit mode

    SET (hold, running) - Return to edit mode (Custom/Seeds/Brian's Brain only)

    B (short press) - Return to menu

    B (hold ~1 second) - Show/hide rules and information panel



Tools Menu

        Grid Lines — overlays a faint grid matching the current cell size
        
        Population Counter — shows live cell count and generation number during simulation

        Trail Mode — dying cells leave a soft 20-step fade trail rather than disappearing instantly

        Colour Mode — age-based colouring (young cells are one colour, older cells shift through the spectrum)

        Toroidal World — when on, edges wrap around so the board is a torus; when off, edges are hard boundaries

        Brightness — four levels (25%, 50%, 75%, 100%)

        Auto-Cycle / Screensaver — automatically switches to a random game mode or Wolfram rule every 300 generations; useful as a display piece

        Sound settings (enabled/disabled, three volume levels) are also accessible here and saved to EEPROM.


Hardware & Setup

        The device runs on a Raspberry Pi Pico / RP2040 connected to a 3.5" ST7796S/ST7796U TFT display (480×320 pixels) via SPI. Six buttons handle all navigation and gameplay — Up, Down, Left, Right, SET (A), and B. A           
        buzzer on GPIO 26 provides sound effects and music. Display brightness is PWM-controlled, and all settings (sound, volume, brightness) are saved to EEPROM so they persist between power cycles.


Main Menu

        On startup the device boots directly into the main scrolling menu with eight sections:
        
        Presets — hand-crafted Conway's Life patterns
        
        Random — randomised Conway's Life
        
        Symmetric — procedurally generated symmetric patterns
        
        Custom — draw your own pattern

        Rule Explorer — define custom birth/survival rules
        
        Alt Games — alternative cellular automata
        
        Tools — display and simulation settings
        
        Arcade — three built-in arcade games
        
        Navigate with Up/Down, select with SET, go back with B.


Conway's Life Modes

        Presets (13 patterns)
        
        Each preset loads a famous Life pattern centred on screen. Press SET once to open it, then A to start the simulation. While paused you can press Left to change cell size (the pattern rescales and redraws) or Right         to reset it back to its starting state.

        1 - Coe Ship. Puffer-type spaceship that leaves debris trails

        2 - Gosper Gun. The first glider gun ever discovered — period 30

        3 - Diamond. Symmetric expanding diamond pattern
        
        4 - Achim p144. Rare period-144 oscillator
        
        5 - 56P6H1V0. High-period spaceship
        
        6 - LWSS Convoy. Three Lightweight Spaceships in formation
        
        7 - MWSS. Middleweight Spaceship — 8 cells, speed c/2
        
        8 - Pulsar. Period-3 oscillator with 4-fold symmetry, 48 cells

        9 - Pentadecathlon. 15 oscillator made from a modified 10-cell row

        10 - R-Pentomino. 5-cell seed that runs for 1,103 generations

        11 - Acorn. 7-cell methuselah running 5,206 generations

        12 - Simkin Gun. Smallest known glider gun — 36 cells, period 120

        13 - Queen Bee. Period-30 oscillator, the first of its kind ever found



Random - Fills the board with random live cells at approximately 30% density. Immediately starts running. Press Right to re-randomise.


        Symmetric - Generates a random pattern with one of three symmetry types — vertical, horizontal, or rotational — in small, medium, or large sizes. Good for producing interesting structured chaos.
        
        Custom - Puts the board into edit mode with a blinking red crosshair cursor. Move the cursor with Up/Down/Left/Right. Press SET to toggle a cell on or off. When happy with your pattern, hold SET for about a second         to start the simulation. Hold SET again to return to edit mode. Press B to go back to the menu.

        Rule Explorer - Lets you define your own cellular automaton by setting custom Birth (B) and Survival (S) rules — the same notation used by Life (B3/S23). Choose from 12 built-in rule presets or enter your own bit-        by-bit. The simulation runs with your rules applied to a random starting board.
        
Included preset rules:

        High Life, 34 Life, Diamoeba, Replicator, Long Life, Maze, Coral, 2x2, Dry Life, Amoeba, Coagulate, Gnarl

Alternative Cellular Automata (Alt Games)

        Brian's Brain - A three-state automaton (On, Dying, Off) producing a constantly moving stream of light-speed "signals." Choose from small, medium, large, or random seedings, or draw a custom starting pattern.

        Day & Night (B3678/S34678) - A symmetric ruleset where dead and live regions are interchangeable. Dense regions survive and grow in a way that mirrors the original — creating organic, blob-like patterns.

        Seeds (B2/S) - Every live cell dies every generation but any dead cell with exactly two live neighbours is born. Creates explosive, constantly moving patterns that never stabilise.

        Cyclic CA - A multi-state automaton where cells cycle through states 0→1→2→…→N→0. Cells advance only when they have a neighbour in the next state. Produces stunning rotating spiral waves. Choose small, medium, or 
        large seedings.

        Wireworld - A four-state automaton (Empty, Wire, Electron Head, Electron Tail) that models digital logic circuits. Electrons travel along wires, interact at junctions, and can form AND gates and clocks. The built-        in demo includes a working circuit layout.

        Langton's Ant - A single ant on a grid follows two rules: turn right on a white cell (flip it black), turn left on a black cell (flip it white). After ~10,000 chaotic steps it spontaneously builds a repeating             diagonal "highway." Toggle age-colour mode with Up+Down to see a heat map of how often each cell has been visited.

        Wolfram 1D Automata - One-dimensional elementary cellular automata — a single row of cells evolves downward according to one of 255 possible rules. Each rule produces a unique pattern, from pure chaos (Rule 30) to         the Sierpinski triangle (Rule 90) to a Turing-complete system (Rule 110). Seven named presets are provided or you can enter any rule number direct

Arcade Games

        Accessed from main menu item 8. Three games are available. In all three, press A + B together at any time to exit directly back to the main menu.

        Star Wars - A first-person Star Wars game inspired by the 80's arcade game. Progress through multiple stages: Womp Rat Training, Space Battle, Death Star Approach, Surface Run, and the Exhaust Port shot. Features         full Star Wars music (Main Theme, Binary Sunset, Imperial March, Victory Fanfare), sound effects, TIE fighters, laser cannons, proton torpedoes, and a dramatic finale sequence. Press B on the title screen to exit.

        Controls: Up/Down/Left/Right — move crosshair / pilot ship

        SET — fire

        A + B — exit to main menu

        Up + Down -  in start up screen will take you into a games menu where you can play any of the games or watch the cinematic shorts


Breakout Beyond

        A feature-rich Breakout variant with a neon aesthetic. The paddle sits at the bottom, bricks at the top. Features include combo multipliers, spin physics, multiball, shield power-ups, bomb bricks, and hard bricks         across 20 levels.

Controls:
        Left/Right — move paddle
        
        B — speed boost
        
        SET — launch ball / pause
        
        A + B (hold) — exit to main menu

    
Gyruss

        A circular shoot-em-up inspired by the 1983 Konami arcade classic. Your ship orbits around the edge of the screen shooting inward at waves of enemies that fly in formation patterns.

Controls:
        Left/Right — rotate ship around the ring
        
        SET or B — fire
        
        Up (hold) — smart bomb
        
        A + B (hold) — exit to main menu
