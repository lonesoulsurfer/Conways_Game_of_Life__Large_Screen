#include "sw_types.h"
/*
 * STAR WARS: TRENCH RUN - Complete Arcade Experience
 * ===================================================
 * 
 * Relive the Battle of Yavin! This complete Star Wars game for Arduino recreates
 * the iconic Death Star assault from A New Hope with authentic vector graphics,
 * multiple training sequences, and the legendary trench run finale.
 * 
 * HARDWARE REQUIREMENTS:
 * ----------------------
 * - Adafruit Trinket M0
 * - 128x64 OLED Display (SSD1306, I2C on address 0x3C)
 * - Resistor ladder button pad on analog pin A3
 * 
 * BUTTON CONFIGURATION (Analog Pin A3):
 * -------------------------------------
 * - SET (A):   Fire/Select     (200-300)
 * - UP:        Move Up          (38-40)
 * - DOWN:      Move Down        (44-47)
 * - LEFT:      Move Left        (74-75)
 * - RIGHT:     Move Right       (141-143)
 * - B:         Alternative      (1020-1023)
 * 
 * GAME STAGES:
 * ------------
 * 1.  TATOOINE SUNSET (Cinematic) - Watch twin suns set over Luke's home
 * 2.  WOMP RAT TRAINING - Destroy 200pts of womp rats in Beggar's Canyon
 * 3.  FORCE LESSON (Story) - Obi-Wan teaches about the Force
 * 4.  BLINDFOLD TRAINING - Deflect remote blasts for 200pts (Jedi training)
 * 5.  SPACE BATTLE - Destroy TIEs for 500pts (TIE Fighters/Interceptors/Bombers)
 * 6.  DEATH STAR APPROACH (Cinematic) - Dramatic flyby of the battle station
 * 7.  DEATH STAR SURFACE - Destroy towers for 500pts (with banking mechanics)
 * 8.  TRENCH ENTRY (Cinematic) - Dive into the trench
 * 9.  TRENCH RUN - Navigate and destroy turrets for 500pts
 * 10. USE THE FORCE (Cinematic) - Obi-Wan's voice guides you
 * 11. EXHAUST PORT - Precision targeting (6-second time limit)
 * 12. MISSILE SHAFT (Cinematic) - Torpedoes travel to reactor core
 * 13. DEATH STAR EXPLOSION (Cinematic) - Epic multi-phase destruction
 * 14. VICTORY! - Mission accomplished
 * 
 * SCORING:
 * --------
 * - Womp Rats: 10pts          - TIE Fighters: 10pts
 * - TIE Interceptors: 20pts   - TIE Bombers: 30pts
 * - Trench Turrets: 20pts     - Surface Towers: 20pts
 * - Power-ups: 20pts          - Exhaust Port: 500pts
 * 
 * HEALTH SYSTEM:
 * --------------
 * - Start: 100% shields
 * - Auto-heal: +5% every 200 points (all stages)
 * - Jedi Training Bonus: +20% upon completion
 * - Invincibility: 1 second after taking damage
 * 
 * FEATURES:
 * ---------
 * - Vector-style wireframe graphics (authentic 1983 arcade aesthetic)
 * - 3D perspective rendering with depth calculation
 * - Smart AI with multiple movement patterns
 * - Progressive difficulty scaling
 * - Dynamic particle effects and explosions
 * - Parallax starfield with motion trails
 * - Banking mechanics during surface run
 * - Power-up system (shields, rapid fire)
 * - Cinematic story sequences
 * 
 * CONTROLS:
 * ---------
 * - Move crosshair/ship with directional buttons
 * - Hold SET (A) for continuous fire in combat stages
 * - Precision required for exhaust port targeting
 * - Banking controlled by horizontal movement on surface
 * 
 * TECHNICAL SPECS:
 * ----------------
 * - Performance: 20 FPS stable (50ms frame time)
 * - Max Enemies: 8 simultaneous (optimized for Arduino Uno)
 * - Projectiles: 16 simultaneous
 * - Starfield: 30 parallax stars
 * - Memory: Optimized for 2KB RAM
 * 
 * TIPS:
 * -----
 * - In Womp Rat Training: Rats evade your crosshair - lead your shots!
 * - In Blindfold Training: Wait for remote to flash (warning), then shoot
 * - In Space Battle: Keep moving to avoid enemy fire
 * - In Trench Run: Turrets need 2 hits to destroy
 * - In Exhaust Port: Take your time - you have 6 seconds and multiple shots
 * 
 * May the Force be with you, pilot!
 * 
 * Based on the 1983 Atari Star Wars arcade game
 * Reimagined for Arduino with enhanced features
 * 
 * Version: 1.0
 * Author: Lonesoulsurfer https://github.com/lonesoulsurfer
 * Date: Oct 2025
 */

// ============================================================
// HARDWARE CONFIGURATION  (ST7796S 3.5" TFT + RP2040)
// ============================================================
#include <Arduino_GFX_Library.h>

// Colour constants (RGB565 format)
#define BLACK        0x0000
#define WHITE        0xFFFF
#define RED          0xF800
#define GREEN        0x07E0
#define BLUE         0x001F
#define YELLOW       0xFFE0
#define CYAN         0x07FF
#define MAGENTA      0xF81F
// Extended Star Wars palette
#define ORANGE       0xFD20   // TIE laser / explosions / suns
#define DARK_ORANGE  0xFA00   // Deeper explosion colour
#define LIGHT_BLUE   0x565F   // X-wing engine glow / lightsaber
#define DARK_GREEN   0x03E0   // Trench floor panels
#define GREY         0x8410   // Death Star surface / metal
#define DARK_GREY    0x4208   // Darker Death Star panels
#define SAND         0xF6E4   // Tatooine desert floor
#define DUSK_SKY     0xF9A4   // Tatooine sunset sky (orange-pink)
#define DUSK_LOW     0xFC00   // Horizon glow (deep amber)
#define BROWN        0x8200   // Canyon walls / rocks
#define RED_LASER    0xF800   // Enemy lasers
#define GREEN_LASER  0x07E0   // Player lasers

// ----- Display SPI pins (SPI1 bus) -----
#ifndef TFT_SCK
#define TFT_SCK   14   // GP14
#endif
#ifndef TFT_MOSI
#define TFT_MOSI  15   // GP15
#endif
#ifndef TFT_RST
#define TFT_RST   28   // GP28
#endif
#ifndef TFT_DC
#define TFT_DC    27   // GP27
#endif
#ifndef TFT_CS
#define TFT_CS    29   // GP29
#endif
#define TFT_BL     1   // GP1  (PWM backlight)
#ifndef TFT_BL_BRIGHTNESS
#define TFT_BL_BRIGHTNESS 80  // 0-255
#endif

// ----- Button GPIO pins (active LOW with INPUT_PULLUP) -----
#define BTN_UP     2   // GP2  (BTN_UP_PIN)
#define BTN_DOWN   3   // GP3  (BTN_DOWN_PIN)
#define BTN_LEFT   4   // GP4  (BTN_RIGHT_PIN)
#define BTN_RIGHT  5   // GP5  (BTN_LEFT_PIN)
#define BTN_FIRE   7   // GP7  (BTN_SET_PIN)
#define BTN_B      6   // GP6  (BTN_B_PIN)

// ----- Buzzer (passive piezo on GP8) -----
#ifndef BUZZER_PIN
#define BUZZER_PIN  26 // GP26
#endif

// ── Integration mode ─────────────────────────────────────────────────────────
// Define GOL_INTEGRATION when compiling inside Game_of_Life_Colour_Arcade.
// Comment out to compile as a standalone sketch.
#define GOL_INTEGRATION

// ── Display objects ───────────────────────────────────────────────────────────
// In GOL_INTEGRATION mode GoL already owns 'bus' and 'display'.
// Declaring them extern here avoids duplicate symbols and ensures we use the
// same already-initialised hardware object — no second begin(), no SPI conflict.
#ifdef GOL_INTEGRATION
  extern Arduino_GFX *display;
#else
  Arduino_DataBus *bus    = new Arduino_RPiPicoSPI(TFT_DC, TFT_CS, TFT_SCK, TFT_MOSI, -1, spi1);
  Arduino_GFX    *display = new Arduino_ST7796(bus, TFT_RST, 3 /* landscape */, false /* IPS */);
#endif

// ============================================================
// DISPLAY WRAPPER  —  128×64 canvas → 4× scaled blit to TFT
//
// Identical rendering to the standalone sketch:
//   1. All draw calls go into an offscreen 128×64 canvas (no flicker).
//   2. display() blits the complete frame to the 480×320 TFT at 4× scale
//      with 32 px letterbox top and bottom.
//
// STATIC FRAMEBUFFER — why not malloc():
//   Arduino_Canvas::begin() calls malloc(128×64×2 = 16 KB).
//   After GoL's large static arrays the heap may be fragmented; malloc()
//   silently returns nullptr and nothing is ever drawn (blank screen).
//   Declaring the buffer as a static array guarantees it at link time.
//
// RAM BUDGET:
//   GoL's briansBrain_dying/newDying arrays (34 KB) are now aliased to
//   cyclicStates/cyclicNewStates (never used simultaneously), freeing exactly
//   the headroom needed for SW_STATIC_FB.
// ============================================================

static uint16_t SW_STATIC_FB[128 * 64];   // 16 KB — in data segment, never heap

// SW_Canvas points _framebuffer at the static array instead of calling malloc.
class SW_Canvas : public Arduino_Canvas {
public:
  SW_Canvas() : Arduino_Canvas(128, 64, nullptr, 0, 0) {}
  bool begin(int32_t = GFX_NOT_DEFINED) override {
    _framebuffer = SW_STATIC_FB;
    return true;
  }
};

class DisplayWrapper {
private:
  Arduino_GFX* tft;
  SW_Canvas    canvas;

  static const int VW     = 128;
  static const int VH     = 64;
  static const int SCALE  = 4;
  static const int OFF_Y  = 32;    // (320 - 64*4) / 2
  static const int CROP   = 4;     // virtual px to skip each side so 120 virt = 480 phys
  static const int DRAW_VW = 120;  // virtual columns drawn

public:
  DisplayWrapper() : tft(nullptr) {}

  void begin(Arduino_GFX* sharedTft, Arduino_ST7796* /*unused*/) {
    tft = sharedTft;
    canvas.begin();
    canvas.fillScreen(BLACK);
    tft->fillScreen(BLACK);
  }

  // Blit the offscreen canvas to the TFT at 4× using RLE drawFastHLine.
  // drawFastHLine is public on all Arduino_GFX subclasses — no protected-method issues.
  // RLE is fast because Star Wars frames are mostly black (long identical runs).
  void display() {
    if (!tft) return;
    uint16_t *fb = canvas.getFramebuffer();
    if (!fb) return;

    tft->fillRect(0,   0,   480, OFF_Y,       BLACK);   // top letterbox
    tft->fillRect(0,   288, 480, 32,           BLACK);   // bottom letterbox

    tft->startWrite();
    for (int vy = 0; vy < VH; vy++) {
      uint16_t *src = fb + vy * VW + CROP;
      int vx = 0;
      while (vx < DRAW_VW) {
        uint16_t col = src[vx];
        int run = 1;
        while (vx + run < DRAW_VW && src[vx + run] == col) run++;
        int physX    = vx * SCALE;
        int physW    = run * SCALE;
        int physYbase = OFF_Y + vy * SCALE;
        for (int dy = 0; dy < SCALE; dy++)
          tft->drawFastHLine(physX, physYbase + dy, physW, col);
        vx += run;
      }
    }
    tft->endWrite();
  }

  void resetBorders() {
    if (tft) {
      tft->fillRect(0,   0,   480, OFF_Y, BLACK);
      tft->fillRect(0, 288,   480, 32,    BLACK);
    }
  }

  uint8_t* getBuffer() { return (uint8_t*)canvas.getFramebuffer(); }

  void clearDisplay()                                    { canvas.fillScreen(BLACK); }
  void setTextSize(int s)                                { canvas.setTextSize(s); }
  void setTextColor(uint16_t c)                          { canvas.setTextColor(c); }
  void setCursor(int x, int y)                           { canvas.setCursor(x, y); }
  void print(const char* s)                              { canvas.print(s); }
  void print(int n)                                      { canvas.print(n); }
  void println(const char* s)                            { canvas.println(s); }
  void println(int n)                                    { canvas.println(n); }
  void println()                                         { canvas.println(); }
  void drawPixel(int x, int y, uint16_t c)               { canvas.drawPixel(x, y, c); }
  void drawLine(int x0,int y0,int x1,int y1,uint16_t c)  { canvas.drawLine(x0,y0,x1,y1,c); }
  void drawRect(int x,int y,int w,int h,uint16_t c)      { canvas.drawRect(x,y,w,h,c); }
  void fillRect(int x,int y,int w,int h,uint16_t c)      { canvas.fillRect(x,y,w,h,c); }
  void drawCircle(int x,int y,int r,uint16_t c)          { canvas.drawCircle(x,y,r,c); }
  void fillCircle(int x,int y,int r,uint16_t c)          { canvas.fillCircle(x,y,r,c); }
  void drawRoundRect(int x,int y,int w,int h,int r,uint16_t c)
                                                         { canvas.drawRoundRect(x,y,w,h,r,c); }
  void fillRoundRect(int x,int y,int w,int h,int r,uint16_t c)
                                                         { canvas.fillRoundRect(x,y,w,h,r,c); }
};

DisplayWrapper sw_display;

// ============================================================
// SOUND ENGINE  –  non-blocking tone sequencer
// ============================================================
// All music / SFX is driven by a single updateSound() call
// in loop().  No delay() is used so the game never stalls.
//
// ---- NOTE FREQUENCIES (Hz) ----
#define NOTE_REST 0
#define NOTE_A3  220
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_G5  784
#define NOTE_A5  880
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_D6  1175
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_G6  1568
#define NOTE_A6  1760
#define NOTE_AS4 466
#define NOTE_CS5 554
#define NOTE_FS5 740
#define NOTE_GS5 831
#define NOTE_AS5 932
#define NOTE_CS6 1109
#define NOTE_DS5 622
#define NOTE_DS6 1245
#define NOTE_FS6 1480
#define NOTE_GS6 1661

// ---- TUNE IDs + Note struct ----
// Defined in sw_types.h to avoid Arduino IDE prototype-generation errors

// ---- SEQUENCE TABLES ----
// Each entry: { frequency, duration_ms }

// Main Title opening fanfare (simplified motif)
static const Note MAIN_THEME[] = {
  {NOTE_G4,350},{NOTE_G4,350},{NOTE_G4,350},{NOTE_C5,250},{NOTE_G5,250},
  {NOTE_F5,125},{NOTE_E5,125},{NOTE_D5,125},{NOTE_C6,500},{NOTE_G5,250},
  {NOTE_F5,125},{NOTE_E5,125},{NOTE_D5,125},{NOTE_C6,500},{NOTE_G5,250},
  {NOTE_F5,125},{NOTE_E5,125},{NOTE_F5,125},{NOTE_D5,700},{NOTE_REST,50}
};
static const uint8_t MAIN_THEME_LEN = sizeof(MAIN_THEME)/sizeof(Note);

// Binary Sunset / Luke's theme (slow, wistful)
static const Note BINARY_SUNSET[] = {
  {NOTE_D5,600},{NOTE_A4,400},{NOTE_B4,300},{NOTE_FS5,700},{NOTE_REST,100},
  {NOTE_E5,400},{NOTE_FS5,200},{NOTE_G5,200},{NOTE_FS5,600},{NOTE_REST,100},
  {NOTE_D5,600},{NOTE_A4,400},{NOTE_B4,300},{NOTE_FS5,700},{NOTE_REST,100},
  {NOTE_E5,600},{NOTE_FS5,400},{NOTE_D5,900},{NOTE_REST,200},
  {NOTE_G5,600},{NOTE_FS5,400},{NOTE_E5,300},{NOTE_D5,700},{NOTE_REST,100},
  {NOTE_CS5,400},{NOTE_D5,200},{NOTE_E5,200},{NOTE_D5,1200},{NOTE_REST,300}
};
static const uint8_t BINARY_SUNSET_LEN = sizeof(BINARY_SUNSET)/sizeof(Note);

// Imperial March (first phrase)
static const Note IMPERIAL_MARCH[] = {
  {NOTE_A4,500},{NOTE_A4,500},{NOTE_A4,500},{NOTE_F4,350},{NOTE_C5,150},
  {NOTE_A4,500},{NOTE_F4,350},{NOTE_C5,150},{NOTE_A4,650},{NOTE_REST,150},
  {NOTE_E5,500},{NOTE_E5,500},{NOTE_E5,500},{NOTE_F5,350},{NOTE_C5,150},
  {NOTE_GS5,500},{NOTE_F4,350},{NOTE_C5,150},{NOTE_A4,650},{NOTE_REST,150}
};
static const uint8_t IMPERIAL_MARCH_LEN = sizeof(IMPERIAL_MARCH)/sizeof(Note);

// Victory fanfare (Throne Room theme opening)
static const Note VICTORY_FANFARE[] = {
  {NOTE_G5,200},{NOTE_A5,200},{NOTE_C6,200},{NOTE_A5,200},{NOTE_C6,400},
  {NOTE_G5,200},{NOTE_A5,200},{NOTE_C6,200},{NOTE_A5,200},{NOTE_C6,400},
  {NOTE_G5,200},{NOTE_A5,200},{NOTE_C6,200},{NOTE_E6,400},{NOTE_D6,400},
  {NOTE_C6,400},{NOTE_B5,200},{NOTE_A5,200},{NOTE_G5,800},{NOTE_REST,100}
};
static const uint8_t VICTORY_FANFARE_LEN = sizeof(VICTORY_FANFARE)/sizeof(Note);

// Game Over (short descending motif)
static const Note GAME_OVER_JINGLE[] = {
  {NOTE_A5,200},{NOTE_GS5,200},{NOTE_G5,200},{NOTE_FS5,400},{NOTE_REST,100},
  {NOTE_E5,200},{NOTE_DS5,200},{NOTE_D5,600},{NOTE_REST,200}
};
static const uint8_t GAME_OVER_JINGLE_LEN = sizeof(GAME_OVER_JINGLE)/sizeof(Note);

// SFX: one-shot sequences
static const Note SFX_LASER_SEQ[]         = { {1800,40},{1400,40},{NOTE_REST,20} };
static const Note SFX_EXPLOSION_SEQ[]     = { {200,60},{150,60},{100,80},{80,100},{NOTE_REST,20} };
static const Note SFX_HIT_PLAYER_SEQ[]    = { {500,25},{280,70},{180,80},{120,90},{NOTE_REST,20} };
static const Note SFX_POWER_UP_SEQ[]      = { {600,80},{800,80},{1000,80},{1200,100},{NOTE_REST,20} };
static const Note SFX_FORCE_SEQ[]         = { {NOTE_C5,200},{NOTE_E5,200},{NOTE_G5,200},{NOTE_C6,400},{NOTE_REST,50} };
static const Note SFX_EXHAUST_LOCK_SEQ[]  = { {1200,60},{1400,60},{1600,80},{NOTE_REST,20} };
static const Note SFX_TORPEDO_SEQ[]       = { {900,50},{800,50},{700,50},{600,50},{NOTE_REST,20} };
static const Note SFX_WOMP_RAT_SEQ[]      = { {600,40},{900,40},{600,40},{NOTE_REST,20} };
static const Note SFX_TIE_FLYBY_SEQ[]     = { {1800,50},{1600,50},{1400,50},{1200,50},{1000,50},{NOTE_REST,20} };
// Proton torpedo screaming down the shaft — descending sweep then rising shriek at impact
static const Note SFX_SHAFT_WHOOSH_SEQ[]  = { {1400,60},{1100,60},{900,70},{700,80},{500,90},{350,100},{250,110},{180,130},{NOTE_REST,20} };

// ---- SEQUENCER STATE ----
struct SoundPlayer {
  const Note* seq;          // pointer to current sequence in PROGMEM
  uint8_t     len;          // total notes
  uint8_t     idx;          // current note index
  unsigned long noteEnd;    // millis() when current note finishes
  bool        looping;      // restart sequence when done
  bool        active;
  TuneID      currentTune;
  // SFX priority: music = low, SFX = high
  bool        sfxPlaying;
  // Saved music position during SFX
  const Note* savedSeq;
  uint8_t     savedLen;
  uint8_t     savedIdx;
  bool        savedLooping;
  TuneID      savedTune;
};

SoundPlayer snd;
bool musicAllowed = false; // true only during cinematic states that have background music
extern bool soundEnabled;  // declared in Game_of_Life_Colour_Arcade.ino; shared across all games

// ---- HELPERS ----
// tunePtr implemented as a macro so the Arduino IDE never generates a prototype for it.
// This avoids "Note does not name a type" errors from prototype generation before includes.
#define tunePtr(id, outLen) _tunePtrImpl((int)(id), outLen)
static const void* _tunePtrImpl(int id, uint8_t &outLen) {
  switch(id) {
    case TUNE_MAIN_THEME:     outLen = MAIN_THEME_LEN;      return MAIN_THEME;
    case TUNE_BINARY_SUNSET:  outLen = BINARY_SUNSET_LEN;   return BINARY_SUNSET;
    case TUNE_IMPERIAL_MARCH: outLen = IMPERIAL_MARCH_LEN;  return IMPERIAL_MARCH;
    case TUNE_VICTORY_FANFARE:outLen = VICTORY_FANFARE_LEN; return VICTORY_FANFARE;
    case TUNE_GAME_OVER_JINGLE:outLen= GAME_OVER_JINGLE_LEN;return GAME_OVER_JINGLE;
    case SFX_LASER:           outLen = 3;  return SFX_LASER_SEQ;
    case SFX_EXPLOSION:       outLen = 5;  return SFX_EXPLOSION_SEQ;
    case SFX_HIT_PLAYER:      outLen = 5;  return SFX_HIT_PLAYER_SEQ;
    case SFX_POWER_UP:        outLen = 5;  return SFX_POWER_UP_SEQ;
    case SFX_FORCE:           outLen = 5;  return SFX_FORCE_SEQ;
    case SFX_EXHAUST_LOCK:    outLen = 4;  return SFX_EXHAUST_LOCK_SEQ;
    case SFX_PROTON_TORPEDO:  outLen = 5;  return SFX_TORPEDO_SEQ;
    case SFX_WOMP_RAT:        outLen = 4;  return SFX_WOMP_RAT_SEQ;
    case SFX_TIE_FLYBY:       outLen = 6;  return SFX_TIE_FLYBY_SEQ;
    case SFX_SHAFT_WHOOSH:    outLen = 9;  return SFX_SHAFT_WHOOSH_SEQ;
    default: outLen = 0; return nullptr;
  }
}

// Start background music (loops) — uses int param to avoid TuneID in prototype
void playMusic(int id) {
  if(!soundEnabled) return;
  if(snd.currentTune == id && snd.active && !snd.sfxPlaying) return;
  uint8_t len; const Note* p = (const Note*)tunePtr(id, len);
  if(!p) return;
  snd.seq = p; snd.len = len; snd.idx = 0;
  snd.looping = true; snd.active = true;
  snd.currentTune = (TuneID)id; snd.sfxPlaying = false;
  snd.noteEnd = 0;
}

// Play one-shot SFX — uses int param to avoid TuneID in prototype
void playSFX(int id) {
  if(!soundEnabled) return;
  if(snd.sfxPlaying) {
    if(id == SFX_LASER) return;
  }
  if(musicAllowed) {
    snd.savedSeq     = snd.seq;
    snd.savedLen     = snd.len;
    snd.savedIdx     = snd.idx;
    snd.savedLooping = snd.looping;
    snd.savedTune    = snd.currentTune;
  } else {
    snd.savedSeq     = nullptr;
    snd.savedLen     = 0;
    snd.savedIdx     = 0;
    snd.savedLooping = false;
    snd.savedTune    = TUNE_NONE;
  }
  uint8_t len; const Note* p = (const Note*)tunePtr(id, len);
  if(!p) return;
  snd.seq = p; snd.len = len; snd.idx = 0;
  snd.looping = false; snd.active = true;
  snd.currentTune = (TuneID)id; snd.sfxPlaying = true;
  snd.noteEnd = 0;
}

void stopSound() {
  noTone(BUZZER_PIN);
  snd.active = false;
  snd.sfxPlaying = false;
  snd.currentTune = TUNE_NONE;
  // Clear saved state too – otherwise a queued SFX restore will bring music back
  snd.savedSeq     = nullptr;
  snd.savedLen     = 0;
  snd.savedIdx     = 0;
  snd.savedLooping = false;
  snd.savedTune    = TUNE_NONE;
}

// Call once per loop() – advances sequencer, never blocks
void updateSound() {
  if(!soundEnabled) { noTone(BUZZER_PIN); snd.active = false; return; }
  if(!snd.active) return;
  unsigned long now = millis();
  if(now < snd.noteEnd) return; // still playing current note

  if(snd.idx >= snd.len) {
    // Sequence finished
    noTone(BUZZER_PIN);
    if(snd.looping) {
      snd.idx = 0; // restart
    } else if(snd.sfxPlaying) {
      // Restore music
      snd.sfxPlaying = false;
      snd.seq     = snd.savedSeq;
      snd.len     = snd.savedLen;
      snd.idx     = snd.savedIdx;
      snd.looping = snd.savedLooping;
      snd.currentTune = snd.savedTune;
      if(!snd.seq) { snd.active = false; return; }
      snd.noteEnd = 0;
      return;
    } else {
      snd.active = false;
      return;
    }
  }

  // Read note from PROGMEM
  Note n;
  memcpy(&n, &snd.seq[snd.idx], sizeof(Note));
  snd.idx++;

  if(n.freq == NOTE_REST || n.freq == 0) {
    noTone(BUZZER_PIN);
  } else {
    tone(BUZZER_PIN, n.freq, n.dur);
  }
  snd.noteEnd = now + n.dur;
}
// ============================================================
// END SOUND ENGINE
// ============================================================

// Game states
  enum GameState {
  TITLE_SCREEN,
  WOMP_RAT_TRAINING,        
  TRAINING_COMPLETE, 
  FORCE_LESSON,
  BLINDFOLD_TRAINING,
  TRAINING_COMPLETE_2,
  TATOOINE_SUNSET,    
  SPACE_BATTLE,
  DEATH_STAR_APPROACH,
  DEATH_STAR_SURFACE,
  TRENCH_ENTRY,
  TRENCH_RUN,
  USE_THE_FORCE,
  EXHAUST_PORT,
  DEATH_STAR_EXPLOSION,
  VICTORY,
  GAME_OVER
};

GameState currentState = TITLE_SCREEN;

// ── Dev / stage-select menu ───────────────────────────────────────────────────
// Accessed from the title screen by pressing UP+DOWN together.
bool devMenuActive    = false;
int  devMenuSelection = 0;
int  devMenuPrevSel   = 0;   // tracks previous row for flicker-free update
static const int DEV_MENU_ITEMS = 15;
const char* devMenuLabels[DEV_MENU_ITEMS] = {
  // ── Gameplay stages ──────────────────────────────────────────────────────
  "WOMP RATS",
  "JEDI TRAINING",
  "SPACE BATTLE",
  "DEATH STAR SFC",
  "TRENCH RUN",
  "EXHAUST PORT",
  // ── Separator (non-selectable) ────────────────────────────────────────────
  "-- CINEMATICS --",
  // ── Cinematic scenes (story order) ───────────────────────────────────────
  "TATOOINE SUNSET",
  "FORCE LESSON",
  "DEATH STAR APP.",
  "TRENCH ENTRY",
  "USE THE FORCE",
  "MISSILE SHAFT",
  "DS EXPLOSION",
  "VICTORY"
};

// ── Difficulty / score gates ──────────────────────────────────────────────
// All stage-completion checks read these variables so changing difficulty
// automatically updates every gate AND every indicator bar.
enum Difficulty { DIFF_EASY=0, DIFF_MEDIUM=1, DIFF_HARD=2 };
int  swDifficulty     = DIFF_MEDIUM;   // current setting

// Gate values (pts needed to finish each stage)
// Set by applyDifficulty() — do not edit these directly.
int GATE_WOMP_RAT  = 250;
int GATE_JEDI      = 200;
int GATE_SPACE     = 500;
int GATE_SURFACE   = 1000;
int GATE_TRENCH    = 1000;

void applyDifficulty() {
  // Base values × difficulty multiplier
  // Easy=0.6×  Medium=1.0×  Hard=1.5×
  float mult = (swDifficulty == DIFF_EASY)   ? 0.6f :
               (swDifficulty == DIFF_HARD)   ? 1.5f : 1.0f;
  GATE_WOMP_RAT  = max(50,  (int)(250  * mult));
  GATE_JEDI      = max(50,  (int)(200  * mult));
  GATE_SPACE     = max(100, (int)(500  * mult));
  GATE_SURFACE   = max(200, (int)(1000 * mult));
  GATE_TRENCH    = max(200, (int)(1000 * mult));
}

// Movement pattern types
enum MovementPattern {
  PATTERN_CIRCLE,
  PATTERN_DIVE,
  PATTERN_ZIGZAG,
  PATTERN_RANDOM
};

// Player variables
// Trench z-depth constants — used by update + collision code
static const int TR_NEAR = 8;    // z when segment reaches player
static const int TR_FAR  = 240;  // z when segment spawned

// ── Trench geometry ──────────────────────────────────────────────────────────
//
//  Vanishing point dead centre of screen: (64, 32)
//
//  The trench has:
//    Left wall  — fills x=0..TR_WALL_W at near plane, converges right to VP
//    Right wall — fills x=127..127-TR_WALL_W, converges left to VP
//    Floor      — bottom strip, converges up to VP
//    Open top   — no ceiling, player can fly anywhere top↔bottom
//
//  Wall inner edge at near plane: x = TR_WALL_W (left) and x = 127-TR_WALL_W (right)
//  Floor top edge at near plane:  y = TR_FLOOR_Y
//
#define TR_VP_X      64    // vanishing point X
#define TR_VP_Y      32    // vanishing point Y — dead centre
#define TR_WALL_W    28    // left wall inner edge X at near plane (right = 127-28=99)
#define TR_FLOOR_Y   50    // floor top edge Y at near plane
#define TR_WALL_COL  GREY
#define TR_FLOOR_COL DARK_GREEN

// t=0 → near plane,  t=1 → vanishing point
// Given a near-plane X, project to screen X at depth t
inline int trWallX(int nearX, float t) {
  return (int)(nearX + t * (TR_VP_X - nearX));
}
// Given a near-plane Y, project to screen Y at depth t
inline int trWallY(int nearY, float t) {
  return (int)(nearY + t * (TR_VP_Y - nearY));
}
// z → t
inline float zToT(float z) {
  float t = 1.0f - (z - TR_NEAR) / (float)(TR_FAR - TR_NEAR);
  if(t < 0) t = 0;
  if(t > 1) t = 1;
  return t;
}
// Compatibility shims for hit/fire code that still uses railX/railY
inline int railX(int nearX, float t) { return trWallX(nearX, t); }
inline int railY(int nearY, float t) { return trWallY(nearY, t); }

float playerX = 64.0, playerY = 50.0;
float crosshairX = 64.0, crosshairY = 32.0;
float crosshairVX = 0.0, crosshairVY = 0.0;  // inertial velocity for exhaust port
int score = 0;
int shields = 100;
int stageStartScore = 0;  // Score when current stage started

// Crosshair flash effect
unsigned long flashCrosshair = 0;
bool hitFlash = false;  // true for hit, false for miss
unsigned long damageFlash = 0;  // Screen shake when taking damage

// Game timing
unsigned long gameTimer = 0;
unsigned long lastUpdate = 0;
unsigned long stateTimer = 0;

// Advanced enemy system
struct Enemy {
  float x, y, z;
  float vx, vy, vz;
  float angle;
  int type; // 0=TIE, 1=Interceptor, 2=Bomber
  bool active;
  int health;
  unsigned long lastFire;
  int weaponType; // 0=single, 1=burst, 2=spread
  bool canFire;   // only ~60% of ships fire — rest just manoeuvre
  bool hasFired;  // each ship fires once then never again
};

Enemy enemies[8];
int enemyCount = 0;

// Advanced projectile system
struct Projectile {
  float x, y, z;
  float vx, vy, vz;
  bool active;
  bool isPlayerShot;
  int damage;
  unsigned long birthTime;
};

Projectile projectiles[12];

// Starfield for vector effects
struct Star {
  float x, y, z;
  float speed;
};

Star stars[20];

// Trench run variables
struct TrenchWall {
  float leftX, rightX, y, z;
  bool  hasObstacle;
  float obstacleX;
  int   turretHealth;
  int   turretSide;      // 0=left wall, 1=right wall, 2=floor centre
  bool  turretLocked;    // true = turret is actively tracking & about to fire
  unsigned long lockTime; // when it locked on
};

// Enemy TIEs that fly down the trench toward the player
struct TrenchTIE {
  float  z;        // depth: TR_FAR=far, TR_NEAR=player
  float  x;        // horizontal world position
  float  y;        // vertical fraction: 0=trench top, 1=trench bottom
  float  vx;       // side-to-side weave velocity
  float  vy;       // up-down drift velocity
  int    health;
  bool   active;
  unsigned long lastFire;
  unsigned long nextSpawn;
  int    anger;
};
static const int NUM_TRENCH_TIES = 3;

TrenchWall trenchSegments[6];
TrenchTIE  trenchTIEs[NUM_TRENCH_TIES];
// Keep vaderTIE as alias to first slot so old refs still compile
#define vaderTIE trenchTIEs[0]
float trenchSpeed = 4.5;

// Vector drawing effects
struct VectorLine {
  float x1, y1, x2, y2;
  unsigned long fadeTime;
  bool active;
};

VectorLine vectorTrails[10];

// Explosion system
struct Explosion {
  float x, y;
  int frame;
  bool active;
  bool small;   // true = compact womp-rat kill puff (half size, half duration)
  bool isDebris; // true = building/tower debris style, false = ship fireball
  // Debris particle state (6 chunks + 4 sparks)
  float debrisX[6], debrisY[6];   // chunk positions
  float debrisVX[6], debrisVY[6]; // chunk velocities
  float sparkX[4],  sparkY[4];
  float sparkVX[4], sparkVY[4];
};

Explosion explosions[5];

// Button handling
unsigned long lastButtonPress = 0;
unsigned long lastFirePress = 0;
const int DEBOUNCE_DELAY = 50;  // Reduced general debounce
const int FIRE_DEBOUNCE_DELAY = 200;  // 5 shots/sec — 120ms flash + 80ms gap = clearly visible pulse

// Death Star approach variables
float deathStarDistance = 500.0;
bool inTrenchApproach = false;

// Trench turrets
struct Turret {
  float x, y, z;
  bool active;
  unsigned long lastFire;
  int health;
};

Turret turrets[4];

// Power-ups
struct PowerUp {
  float x, y, z;
  float vx, vy; // Velocity for floating movement
  int type; // 0=shield, 1=rapid fire, 2=extra life
  bool active;
  unsigned long spawnTime;
};

PowerUp powerUps[3];

// Level system
int currentLevel = 1;
int enemiesKilled = 0;
bool rapidFire = false;
unsigned long rapidFireEnd = 0;
unsigned long showHealthBonus = 0;
unsigned long exhaustPortStartTime = 0;
unsigned long lastDamageTime = 0;
const unsigned long INVINCIBILITY_TIME = 1000; // 1 second of invincibility after damage
unsigned long hitConfirmationTime = 0;
int hitConfirmationX = 0, hitConfirmationY = 0;
const unsigned long EXHAUST_PORT_TIMEOUT = 6000; // 5 seconds

// Exhaust port targeting variables
unsigned long exhaustPortTimer = 0;
bool targetingActive = false;
bool shotFired = false;
float targetAccuracy = 0.0; // How close to center when shot fired
unsigned long shotTime = 0;
bool missionSuccess = false;

// Missile shaft animation variables
bool inMissileShaft = false;
float shaftDepth = 0.0;
float missileY = 0.0;
unsigned long shaftStartTime = 0;
bool shaftImpactFlash = false;        // white-flash on core impact
unsigned long shaftImpactTime = 0;    // millis() when flash was triggered
struct ShaftSegment {
  float z;
  float width;
  bool hasObstacle;
  float obstacleX;
};
ShaftSegment shaftSegments[8];

// Surface tower variables
struct SurfaceTower {
  float x, y, z;
  int health;
  bool active;
  int type;
  unsigned long lastHit;
};

// TIE fighters that dive from space during the Death Star surface run
struct SurfaceTIE {
  float x, y;       // canvas position
  float vx, vy;     // velocity (diving down-toward-player)
  bool  active;
  bool  hasFired;
  bool  retreating;  // true after firing — TIE climbs back out of frame
  int   health;
  unsigned long spawnTime;
};
static const int NUM_SURFACE_TIES = 2;
SurfaceTIE surfaceTIEs[NUM_SURFACE_TIES];
unsigned long nextTIESpawn    = 0;   // millis() timestamp for next TIE attack wave
int           tieWavesSpawned = 0;   // total TIE waves launched this stage

float bankingAngle = 0.0; // Current banking angle
float targetBankingAngle = 0.0; // Target banking angle based on input
float bankingSpeed = 0.15; // How fast banking responds

SurfaceTower surfaceTowers[4];
float surfaceSpeed = 2.0;

unsigned long lastRespawn = 0;
int activeTowers = 0;
int lastHealthBonus = 0;  // Tracks score milestones for health restoration

// Stage-progression globals

struct BackgroundShip {
  float x, y, z;
  float vx, vy, vz;
  int type; // 0=X-wing, 1=TIE
  bool active;
  unsigned long lastShot;
};

BackgroundShip backgroundShips[6];

// Womp rat training variables
struct WompRat {
  float x, y;
  float vx, vy;
  bool active;
  unsigned long lastDirection;
};

WompRat wompRats[6];
int wompRatsKilled = 0;
float canyonScroll = 0;
float t16X = 64.0;

struct CanyonRock {
  float x, y;
  int size;
  bool active;
};

CanyonRock canyonRocks[3];

// Blindfold training variables
struct TrainingRemote {
  float x, y;
  float vx, vy;
  bool active;
  unsigned long nextFireTime;
  int predictX, predictY; // Where remote will fire from
  bool showPrediction;
  unsigned long predictionTime;
  MovementPattern currentPattern;
  unsigned long patternStartTime;
  float patternSpeed;
  float centerX, centerY;
  float angle;
  int diveDirection;
  int zigzagDirection;
};

TrainingRemote trainingRemote;
int deflectionsSuccessful = 0;
int deflectionsMissed = 0;
unsigned long lastDeflectionAttempt = 0;
bool blindfoldActive = false;
float dangerZoneRadius = 50.0;
float dangerZoneMinRadius = 20.0;
float dangerZoneShrinkRate = 0.01;

// Death Star explosion variables
unsigned long explosionStartTime = 0;

// Function prototypes
void handleInput();
void sw_updateGame();
void drawGame();
void initializeStars();
void updateStars();
void drawStars();
void spawnEnemy();
void updateEnemies();
void drawEnemies();
void fireProjectile(float x, float y, float vx, float vy, bool isPlayer);
void updateProjectiles();
void drawProjectiles();
void drawVectorEffect(float x1, float y1, float x2, float y2);
void updateVectorTrails();
void drawVectorTrails();
void initializeTrench();
void updateTrench();
void drawTrench();
void drawTrenchSpeedLines();
float trenchTIEProject(const TrenchTIE &tie, int &scx, int &scy);
void drawVaderTIE();
void drawTrenchHUD();
bool checkCollisions();
void drawUI();
void drawTitleScreen();
void drawDevMenu();
void drawTitleScreenDirect();
void drawDevMenuDirect();
void drawGameOver();
void drawVictory();
int readButtonValue();
bool sw_isBtnUp(int val);
bool sw_isBtnDown(int val);
bool sw_isBtnLeft(int val);
bool sw_isBtnRight(int val);
bool sw_isBtnSet(int val);
bool sw_isBtnB(int val);
extern bool arcadeExitToMain;  // set true on A+B to go back to main menu
void drawVectorTIE(float x, float y, float scale, float angle);
void drawVectorInterceptor(float x, float y, float scale, float angle);
void drawVectorBomber(float x, float y, float scale, float angle);
void drawSpaceBattlePlayer();
void drawTrenchPlayer();
void drawExhaustPort();
void drawTargeting();
void createExplosion(float x, float y, bool small = false);
void createDebrisExplosion(float x, float y);
void updateExplosions();
void drawExplosions(); 
void drawCockpitWindow();
void spawnPowerUp(float x, float y, int type);
void updatePowerUps();
void drawPowerUps();
void drawDeathStarApproach();
void drawTrenchEntry();
void checkTurretFiring(int segmentIndex, float turretX, float turretY, float scale);
void handleExhaustPortFiring();
void drawUseTheForce();
void initializeShaft();
void drawMissileShaft();
void drawDeathStarExplosion();
void initializeSurfaceTowers();
void updateSurfaceTowers();
void drawSurfaceTowers();
void initializeSurfaceTIEs();
void updateSurfaceTIEs();
void drawSurfaceTIEs();
void drawDeathStarSurface();
void handleFireButton();
void drawWompRatTraining();
void updateWompRatTraining();
void drawTrainingComplete();
void drawForceLesson();
void drawTrainingComplete2();
void initializeBlindfolding();
void initializeWompRats();
void initializeCanyonRocks();
void drawTatooineSunset();
void initializeBlindfolding();
void drawBlindfolding();
void updateBlindfolding();

// ── Shared initialisation (used by both sw_init and standalone setup) ─────────
static void sw_resetGame() {
  memset(&snd, 0, sizeof(snd));
  initializeStars();
  initializeTrench();
  initializeSurfaceTowers();
  for(int i = 0; i < 8;  i++) enemies[i].active     = false;
  for(int i = 0; i < 12; i++) projectiles[i].active  = false;
  for(int i = 0; i < 10; i++) vectorTrails[i].active = false;
  for(int i = 0; i < NUM_SURFACE_TIES; i++) surfaceTIEs[i].active = false;
  gameTimer       = millis();
  randomSeed(millis());
  applyDifficulty();
  score           = 0;
  shields         = 100;
  stageStartScore = 0;
  stateTimer      = millis();
  lastButtonPress = 0;
  lastFirePress   = 0;
  lastUpdate      = 0;
  musicAllowed    = false;
  currentState    = TITLE_SCREEN;
  devMenuActive   = false;
}

#ifdef GOL_INTEGRATION
// ── GoL entry points ──────────────────────────────────────────────────────────
// GoL has already called display->begin() — do NOT call it again.

void sw_init() {
  sw_display.begin(display, (Arduino_ST7796*)display);
  sw_resetGame();
  playMusic(TUNE_MAIN_THEME);
  drawTitleScreenDirect();  // draw title once — no per-frame redraw needed
}

// Returns true = keep running, false = player exited (B on title screen).
bool sw_runFrame() {
  // A+B held for ~4 frames → exit to main menu at any time
  static int swExitCombo = 0;
  if (digitalRead(BTN_FIRE) == LOW && digitalRead(BTN_B) == LOW) {
    if (++swExitCombo >= 4) {
      swExitCombo = 0;
      arcadeExitToMain = true;
      stopSound();
      delay(200);
      return false;
    }
  } else { swExitCombo = 0; }

  if (currentState == TITLE_SCREEN && digitalRead(BTN_B) == LOW) {
    stopSound();
    delay(200);
    return false;
  }
  updateSound();

  unsigned long now = millis();
  if (now - lastUpdate < 6) return true;
  lastUpdate = now;

  // ── Crosshair movement: fixed step per frame ─────────────────────────────
  {
    int btnValue = readButtonValue();
    bool L = sw_isBtnLeft(btnValue);
    bool R = sw_isBtnRight(btnValue);
    bool U = sw_isBtnUp(btnValue);
    bool D = sw_isBtnDown(btnValue);

    bool isSurface = false;
    float stepLR = isSurface ? 2.0f : 8.0f;
    float stepUD = isSurface ? 1.5f : 5.0f;

    bool moving = (
      currentState == WOMP_RAT_TRAINING || currentState == BLINDFOLD_TRAINING ||
      currentState == SPACE_BATTLE      || currentState == DEATH_STAR_SURFACE  ||
      currentState == DEATH_STAR_APPROACH || currentState == EXHAUST_PORT);

    if(moving) {
      if(L) { crosshairX += stepLR; if(crosshairX > 118) crosshairX = 118; }
      if(R) { crosshairX -= stepLR; if(crosshairX <  10) crosshairX =  10; }
      if(D) { crosshairY += stepUD; if(crosshairY >  54) crosshairY =  54; }
      if(U) { crosshairY -= stepUD; if(crosshairY <  10) crosshairY =  10; }
    }

    if(currentState == TRENCH_RUN) {
      // Left/right reversed to match first-person perspective + faster speed
      if(L) { playerX += 9.0f; if(playerX >119) playerX =119; }
      if(R) { playerX -= 9.0f; if(playerX <  8) playerX =  8; }
      if(U) { playerY -= 7.0f; if(playerY < 24) playerY = 24; }
      if(D) { playerY += 7.0f; if(playerY > 58) playerY = 58; }
    }

    if(currentState == DEATH_STAR_SURFACE) {
      static float lastCX = 64;
      targetBankingAngle = constrain((crosshairX - lastCX) * 5, -25, 25);
      lastCX = crosshairX;
    }

    if(currentState == WOMP_RAT_TRAINING) {
      int gapAtY = 25 + (50-25)*57/64;
      if(crosshairX < 64-gapAtY+8) crosshairX = 64-gapAtY+8;
      if(crosshairX > 64+gapAtY-8) crosshairX = 64+gapAtY-8;
    }
  }

  handleInput();
  sw_updateGame();
  drawGame();
  return true;
}

#else
// ── Standalone entry points ───────────────────────────────────────────────────
void setup() {
  pinMode(BTN_UP,    INPUT_PULLUP);
  pinMode(BTN_DOWN,  INPUT_PULLUP);
  pinMode(BTN_LEFT,  INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_FIRE,  INPUT_PULLUP);
  pinMode(BTN_B,     INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  pinMode(TFT_BL, OUTPUT);
  analogWrite(TFT_BL, TFT_BL_BRIGHTNESS);
  display->begin(80000000);
  sw_display.begin(display, (Arduino_ST7796*)display);
  sw_resetGame();
  playMusic(TUNE_MAIN_THEME);
}

void loop() {
  updateSound();
  unsigned long currentMillis = millis();
  if(currentMillis - lastUpdate < 6) return;
  lastUpdate = currentMillis;
  {
    int btnValue = readButtonValue();
    bool L=sw_isBtnLeft(btnValue),R=sw_isBtnRight(btnValue);
    bool U=sw_isBtnUp(btnValue),  D=sw_isBtnDown(btnValue);
    bool isSurface=false; // all games same speed
    float stepLR=isSurface?2.0f:8.0f, stepUD=isSurface?1.5f:5.0f;
    bool moving=(currentState==WOMP_RAT_TRAINING||currentState==BLINDFOLD_TRAINING||
                 currentState==SPACE_BATTLE||currentState==DEATH_STAR_SURFACE||currentState==DEATH_STAR_APPROACH||currentState==EXHAUST_PORT);
    if(moving){
      if(L){crosshairX+=stepLR;if(crosshairX>118)crosshairX=118;}
      if(R){crosshairX-=stepLR;if(crosshairX<10)crosshairX=10;}
      if(D){crosshairY+=stepUD;if(crosshairY>54)crosshairY=54;}
      if(U){crosshairY-=stepUD;if(crosshairY<10)crosshairY=10;}
    }
    if(currentState==TRENCH_RUN){
      if(L){playerX+=8.0f;if(playerX>113)playerX=113;}
      if(R){playerX-=8.0f;if(playerX<15)playerX=15;}
    }
  }
  handleInput();
  sw_updateGame();
  drawGame();
}
#endif  // GOL_INTEGRATION


void handleInput() {
  int btnValue = readButtonValue();
  
  // Static button vars like V1 — state preserved between frames, edge detection works
  static bool firePressed    = false;
  static bool prevFirePressed= false;
  static bool leftPressed    = false;
  static bool rightPressed   = false;
  static bool upPressed      = false;
  static bool downPressed    = false;

  prevFirePressed = firePressed;
  firePressed  = sw_isBtnSet(btnValue);
  leftPressed  = sw_isBtnLeft(btnValue);
  rightPressed = sw_isBtnRight(btnValue);
  upPressed    = sw_isBtnUp(btnValue);
  downPressed  = sw_isBtnDown(btnValue);

  bool newFirePress = firePressed && !prevFirePressed;

  // V1 fire logic: combat states repeat while held, others fire on press only
  bool inCombat = (currentState == WOMP_RAT_TRAINING || currentState == SPACE_BATTLE ||
                   currentState == BLINDFOLD_TRAINING || currentState == DEATH_STAR_SURFACE ||
                   currentState == TRENCH_RUN || currentState == EXHAUST_PORT ||
                   currentState == DEATH_STAR_APPROACH);
  if(firePressed && millis() - lastFirePress > FIRE_DEBOUNCE_DELAY &&
     (inCombat || !prevFirePressed)) {
    lastFirePress = millis();
    handleFireButton();
  }

  if(currentState == TITLE_SCREEN) {
    // Left/Right cycles difficulty on title screen
    {
      static bool prevL = false, prevR = false;
      bool lEdge = leftPressed  && !prevL;
      bool rEdge = rightPressed && !prevR;
      prevL = leftPressed; prevR = rightPressed;
      if(lEdge || rEdge) {
        swDifficulty = (swDifficulty + (rEdge ? 1 : 2)) % 3;
        applyDifficulty();
      }
    }
    if(upPressed && downPressed && !devMenuActive) {
      devMenuActive = true; devMenuSelection = 0; devMenuPrevSel = 0;
      delay(200); drawDevMenuDirect(); return;
    }
    if(devMenuActive) {
      static bool prevUp = false, prevDown = false;
      bool upEdge   = upPressed   && !prevUp;
      bool downEdge = downPressed && !prevDown;
      prevUp = upPressed; prevDown = downPressed;
      bool selChanged = false;
      if(upEdge)   { devMenuSelection = (devMenuSelection - 1 + DEV_MENU_ITEMS) % DEV_MENU_ITEMS; selChanged = true; }
      if(downEdge) { devMenuSelection = (devMenuSelection + 1) % DEV_MENU_ITEMS;                  selChanged = true; }
      if(selChanged) {
        // Skip over the non-selectable separator row (index 6)
        if(devMenuSelection == 6)
          devMenuSelection = upEdge ? 5 : 7;
        // Repaint only the two affected rows — no fillScreen, no flicker
        devMenuDrawRow(devMenuPrevSel, false);
        devMenuDrawRow(devMenuSelection, true);
        devMenuPrevSel = devMenuSelection;
      }
      if(digitalRead(BTN_B) == LOW) { devMenuActive = false; delay(200); drawTitleScreenDirect(); return; }
      if(newFirePress) {
        devMenuActive = false; stopSound();
        score=0; shields=100; stageStartScore=0; lastHealthBonus=0; musicAllowed=false;
        playerX=64; playerY=50; crosshairX=64; crosshairY=32;
        enemiesKilled=0; inMissileShaft=false; missionSuccess=false;
        shotFired=false; targetingActive=false; deathStarDistance=500.0;
        for(int i=0;i<8; i++) enemies[i].active=false;
        for(int i=0;i<12;i++) projectiles[i].active=false;
        for(int i=0;i<5; i++) explosions[i].active=false;
        lastButtonPress=millis()+500; stateTimer=millis();
        switch(devMenuSelection) {
          case 0: initializeWompRats();initializeCanyonRocks();canyonScroll=0;wompRatsKilled=0;currentState=WOMP_RAT_TRAINING;break;
          case 1: initializeBlindfolding();currentState=BLINDFOLD_TRAINING;break;
          case 2: currentState=SPACE_BATTLE;break;
          case 3: initializeSurfaceTowers();initializeSurfaceTIEs();surfaceSpeed=2.0;crosshairX=64;crosshairY=35;currentState=DEATH_STAR_SURFACE;break;
          case 4: trenchSpeed=4.5;playerY=40;initializeTrench();currentState=TRENCH_RUN;break;
          case 5: targetingActive=false;shotFired=false;inMissileShaft=false;crosshairVX=0;crosshairVY=0;currentState=EXHAUST_PORT;break;
          case 6: devMenuActive=true;drawDevMenuDirect();return;
          // ── Cinematics ────────────────────────────────────────────────────
          case 7:  stateTimer=millis();currentState=TATOOINE_SUNSET;break;
          case 8:  stateTimer=millis();musicAllowed=false;playSFX(SFX_FORCE);currentState=FORCE_LESSON;break;
          case 9:  deathStarDistance=500.0;musicAllowed=true;playMusic(TUNE_IMPERIAL_MARCH);currentState=DEATH_STAR_APPROACH;break;
          case 10: stateTimer=millis();currentState=TRENCH_ENTRY;break;
          case 11: stateTimer=millis();musicAllowed=false;playSFX(SFX_FORCE);currentState=USE_THE_FORCE;break;
          case 12: inMissileShaft=true;shaftDepth=0.0;missileY=0.0;shaftImpactFlash=false;initializeShaft();playSFX(SFX_SHAFT_WHOOSH);currentState=EXHAUST_PORT;break;
          case 13: explosionStartTime=millis();currentState=DEATH_STAR_EXPLOSION;break;
          case 14: musicAllowed=true;playMusic(TUNE_VICTORY_FANFARE);currentState=VICTORY;break;
        }
        return;
      }
      return;
    }
    if(newFirePress) { currentState=TATOOINE_SUNSET; stateTimer=millis(); lastButtonPress=millis(); stopSound(); }
    return;
  }

  if(currentState == FORCE_LESSON) {
    if(newFirePress && millis()-stateTimer>1000) { currentState=BLINDFOLD_TRAINING; stateTimer=millis(); stageStartScore=score; initializeBlindfolding(); lastButtonPress=millis(); }
    return;
  }
  if(currentState == TRAINING_COMPLETE_2) {
    if(newFirePress && millis()-stateTimer>2000) { currentState=SPACE_BATTLE; stateTimer=millis(); stageStartScore=score; lastButtonPress=millis(); }
    return;
  }
  if(currentState == GAME_OVER || currentState == VICTORY) {
    if(newFirePress && millis()-lastButtonPress>500) {
      currentState=TITLE_SCREEN; score=0; stageStartScore=0; lastHealthBonus=0; musicAllowed=false; shields=100;
      playerX=64;playerY=50;crosshairX=64;crosshairY=32; currentLevel=1; enemiesKilled=0;
      inMissileShaft=false; missionSuccess=false; shotFired=false; targetingActive=false;
      deathStarDistance=500.0; lastButtonPress=millis();
      playMusic(TUNE_MAIN_THEME); drawTitleScreenDirect();
    }
    return;
  }
  // Movement is handled in sw_runFrame() with smooth delta-time velocity.
}

void handleFireButton() {
  // Safety: count active projectiles before allowing fire
  int activeProjectiles = 0;
  for(int i = 0; i < 12; i++) {
    if(projectiles[i].active) activeProjectiles++;
  }
  // Only block fire for states that actually SPAWN player projectiles
  bool usesProjectiles = (currentState == TRENCH_RUN ||
                          currentState == DEATH_STAR_APPROACH || currentState == EXHAUST_PORT);
  if(usesProjectiles && activeProjectiles >= 10) return;
  
  if(currentState == WOMP_RAT_TRAINING) {
    flashCrosshair = millis() + 120;
    playSFX(SFX_LASER);  // Laser sound
    
    for(int i = 0; i < 6; i++) {
      if(wompRats[i].active) {
        float dist = sqrt(pow(crosshairX - wompRats[i].x, 2) + 
                         pow(crosshairY - wompRats[i].y, 2));
        if(dist < 8) {
          wompRats[i].active = false;
          score += 10;
          createExplosion(wompRats[i].x, wompRats[i].y, true);
          playSFX(SFX_WOMP_RAT);  // Womp rat hit SFX
          break;
        }
      }
    }
    return;
  }

  if(currentState == BLINDFOLD_TRAINING) {
    lastDeflectionAttempt = millis();
    flashCrosshair = millis() + 120;
    playSFX(SFX_LASER);  // Lightsaber deflect sound
    
    // Only allow shooting when ball is in warning state (flashing)
    if(trainingRemote.showPrediction) {
      // Check if player is shooting at the remote ball
      float dist = sqrt(pow(crosshairX - trainingRemote.x, 2) + 
                       pow(crosshairY - trainingRemote.y, 2));
      
      // Scoring based on accuracy
      if(dist < 2) {
        // Perfect hit - dead center
        deflectionsSuccessful++;
        score += 20;
        createExplosion(trainingRemote.x, trainingRemote.y);
        
        // Reset ball for next attack
        trainingRemote.showPrediction = false;
        trainingRemote.nextFireTime = millis() + random(2500, 4000);
      } else if(dist < 4) {
        // Near middle
        deflectionsSuccessful++;
        score += 10;
        createExplosion(trainingRemote.x, trainingRemote.y);
        
        // Reset ball for next attack
        trainingRemote.showPrediction = false;
        trainingRemote.nextFireTime = millis() + random(2500, 4000);
      } else if(dist < 8) {
        // Just outside ball - edge hit
        deflectionsSuccessful++;
        score += 5;
        createExplosion(trainingRemote.x, trainingRemote.y);
        
        // Reset ball for next attack
        trainingRemote.showPrediction = false;
        trainingRemote.nextFireTime = millis() + random(2500, 4000);
      } else {
        // Complete miss - shot during warning but missed
        deflectionsMissed++;
      }
    }
    // If not in warning state, shooting does nothing (no penalty)
    return;
  }

  if(currentState == SPACE_BATTLE) {
    hitFlash = false;
    playSFX(SFX_LASER);  // X-wing laser
    
    for(int i = 0; i < 8; i++) {
      if(enemies[i].active) {
        float dist = sqrt(pow(crosshairX - enemies[i].x, 2) + 
                         pow(crosshairY - enemies[i].y, 2));
        if(dist < 10) {
          hitFlash = true;
          enemies[i].health -= 1;
          
          if(enemies[i].health <= 0) {
            enemies[i].active = false;
            enemyCount--;
            score += (enemies[i].type + 1) * 10;
            createExplosion(enemies[i].x, enemies[i].y);
            playSFX(SFX_EXPLOSION);  // TIE explosion
            
            if(random(100) < 20) {
              float offsetX = enemies[i].x + random(-15, 15);
              float offsetY = enemies[i].y + random(-10, 10);
              if(offsetX < 15) offsetX = 15;
              if(offsetX > 113) offsetX = 113;
              if(offsetY < 15) offsetY = 15;
              if(offsetY > 45) offsetY = 45;
              spawnPowerUp(offsetX, offsetY, 0);
            }
          }
          break;
        }
      }
    }
    
    flashCrosshair = millis() + 120;
    
  } else if(currentState == DEATH_STAR_SURFACE) {
    flashCrosshair = millis() + 120;
    playSFX(SFX_LASER);
    handleSurfaceTIEHit();  // check if a diving TIE is in crosshairs
    
    for(int i = 0; i < 4; i++) {
        if(surfaceTowers[i].active) {
        float scale = 100.0 / (surfaceTowers[i].z + 20);
        
        if(scale > 0.3) {
          int towerWidth = max(1, (int)(3 * scale));
          int towerHeight = max(2, (int)(8 * scale));
          
          float dist = sqrt(pow(crosshairX - surfaceTowers[i].x, 2) + 
                           pow(crosshairY - (surfaceTowers[i].y - towerHeight/2), 2));
          if(dist < (towerWidth + 8)) {
            surfaceTowers[i].health -= 1;
            surfaceTowers[i].lastHit = millis();
            
            if(surfaceTowers[i].health <= 0) {
              surfaceTowers[i].active = false;
              score += 20;
              createDebrisExplosion(surfaceTowers[i].x, surfaceTowers[i].y - towerHeight/2);
              playSFX(SFX_EXPLOSION);  // Tower destroyed
            }
            break;
          }
        }
      }
    }
  } else if(currentState == TRENCH_RUN) {
    // Fire two converging bolts toward vanishing point
    // Fire two bolts straight forward up the trench — slight inward angle
    // so they converge toward the vanishing point naturally
    // Perspective-correct trench bolts — two wing bolts
    fireTrenchBolt(playerX - 3 - 64, true);
    fireTrenchBolt(playerX + 3 - 64, true);
    playSFX(SFX_LASER);
  } else if(currentState == EXHAUST_PORT) {
    handleExhaustPortFiring();
  }
}

void sw_updateGame() {
  updateStars();
  updateVectorTrails();
  updateExplosions();

  // Universal health restoration every 200 points (all game states)
  int currentHealthLevel = score / 200;
  if(currentHealthLevel > lastHealthBonus) {
  shields += 5;  // Changed from 10 to 5 (10% to 5%)
  if(shields > 100) shields = 100;
  showHealthBonus = millis() + 1000;
  lastHealthBonus = currentHealthLevel;
}

  switch(currentState) {
    case TITLE_SCREEN:
      // Animate title screen
      break;

    case WOMP_RAT_TRAINING:
  updateWompRatTraining();
  
  if(score - stageStartScore >= GATE_WOMP_RAT) {
    currentState = FORCE_LESSON;
    stateTimer = millis();
    stageStartScore = score;
  }
  break; 
      
    case TRAINING_COMPLETE:
      // Wait for player to press button
      break;
      
      case FORCE_LESSON:
  // Wait for player to press button
  break;
  
case BLINDFOLD_TRAINING:
  updateBlindfolding();
  
  // Win condition: 200 points in THIS stage
  if(score - stageStartScore >= GATE_JEDI) {
    // Give 20% health boost for completing Jedi training
    shields += 20;
    if(shields > 100) shields = 100;
    showHealthBonus = millis() + 1000;  // Show the +20 notification
    
    currentState = TRAINING_COMPLETE_2;
    stateTimer = millis();
  }
  break;

case TATOOINE_SUNSET:
      // 8 second cinematic scene, then auto-transition to womp rat training
      // Play Binary Sunset (Luke's theme) for the twin suns moment
      if(snd.currentTune != TUNE_BINARY_SUNSET && !snd.sfxPlaying) {
        musicAllowed = true;
        playMusic(TUNE_BINARY_SUNSET);
      }
      if(millis() - stateTimer > 8000) {
        musicAllowed = false;
        currentState = WOMP_RAT_TRAINING;
        stateTimer = millis();
        stageStartScore = score;
        wompRatsKilled = 0;
        initializeWompRats();
        initializeCanyonRocks();
        stopSound(); // silence when leaving sunset
      }
      break;
      
    case SPACE_BATTLE: {
      updateEnemies();
      updateProjectiles();
      updatePowerUps();
      
      // Spawn enemies — rate increases with score
      int stageScoreSB = score - stageStartScore;
      int spawnChance = 5 + (stageScoreSB / 80);   // 5% → 11% over the stage
      if(spawnChance > 11) spawnChance = 11;
      int actualEnemyCount = 0;
      for(int i = 0; i < 8; i++) if(enemies[i].active) actualEnemyCount++;
      int maxEnemies = 4 + (stageScoreSB / 150);   // 4 → 7 — never exceeds array size 8
      if(maxEnemies > 7) maxEnemies = 7;
      if(random(100) < spawnChance && actualEnemyCount < maxEnemies) {
        spawnEnemy();
        // Occasional double-spawn for intensity at higher scores
        if(stageScoreSB > 150 && random(100) < 35 && actualEnemyCount < maxEnemies - 1)
          spawnEnemy();
      }
      
      // Check for Death Star approach transition
      if(score - stageStartScore >= GATE_SPACE) {
        currentState = DEATH_STAR_APPROACH;
        stateTimer = millis();
        deathStarDistance = 500.0;
      }
      break;
    }

    case DEATH_STAR_APPROACH:
  // No enemies or projectiles during approach - just cinematic flyby
  if(snd.currentTune != TUNE_IMPERIAL_MARCH && !snd.sfxPlaying) {
    musicAllowed = true;
    playMusic(TUNE_IMPERIAL_MARCH);
  }
  // Approach Death Star — fast and dramatic
  deathStarDistance -= 5.0;
  
  // Transition to surface when close enough
  if(deathStarDistance <= 80) {
    musicAllowed = false;
    stageStartScore = score;
    currentState = DEATH_STAR_SURFACE;
    stateTimer = millis();
    surfaceSpeed = 2.0;
    playerY = 50;
    crosshairX = 64;
    crosshairY = 35;
    initializeSurfaceTowers();
    initializeSurfaceTIEs();
    stopSound();
  }
  break;

    case DEATH_STAR_SURFACE: {
      updateSurfaceTowers();
      updateSurfaceTIEs();
      updateProjectiles();
      updateExplosions();
      
      // Player movement constraints for surface phase
      if(playerX < 15) playerX = 15;
      if(playerX > 113) playerX = 113;
      
      // Transition to trench run after destroying enough towers
      int activeTowers = 0;
      for(int i = 0; i < 4; i++) {
        if(surfaceTowers[i].active) activeTowers++;
      }
      if(score - stageStartScore >= GATE_SURFACE) {
        stageStartScore = score;  // Set new baseline for trench run
        currentState = TRENCH_ENTRY;  // New transition state
        stateTimer = millis();
        surfaceSpeed = 4.0;  // Speed up for dramatic effect
      }
      break;
      }
    case TRENCH_ENTRY:
      // 2-second diving transition
      if(millis() - stateTimer > 5000) {
        currentState = TRENCH_RUN;
        stageStartScore = score;   // ← start trench run score gate from here
        trenchSpeed = 4.5;
        playerY = 40;
      }
      break;
  
    case USE_THE_FORCE:
     // 7 second "Use the Force" message
      if(millis() - stateTimer > 7000) {
        currentState = EXHAUST_PORT;
        exhaustPortTimer = millis() + 6000;
        exhaustPortStartTime = millis();
        crosshairX = random(15, 113);
        crosshairY = random(15, 49);
        crosshairVX = 0; crosshairVY = 0;   // no carry-over velocity into targeting
        targetingActive = true;
        shotFired = false;
        missionSuccess = false;
      }
      break;
      
    case TRENCH_RUN:
      updateTrench();
      updateProjectiles();
      
      // Check turret collisions with player (turret passes through near plane)
      for(int seg = 0; seg < 6; seg++) {
        if(!trenchSegments[seg].hasObstacle) continue;
        if(trenchSegments[seg].z > TR_NEAR + 4 || trenchSegments[seg].z < 0) continue;
        // Turret has reached the player — unavoidable hit
        trenchSegments[seg].hasObstacle = false;
        trenchSegments[seg].turretLocked = false;
        if(millis() - lastDamageTime > INVINCIBILITY_TIME) {
          shields -= 10;
          lastDamageTime = millis();
          playSFX(SFX_HIT_PLAYER);
          damageFlash = millis() + 400;
          createDebrisExplosion(playerX, playerY);
          if(shields <= 0) currentState = GAME_OVER;
        }
      }
      
      // Check collisions with walls
      // Temporarily disabled for trench testing
      // if(checkCollisions()) {
      //   shields -= 25;
      //   if(shields <= 0) {
      //       currentState = GAME_OVER;
      //   }
      // }
      
      // Check for exhaust port transition
      // Remove time-based transition - only score-based now
      if(score - stageStartScore >= GATE_TRENCH) { 
        stageStartScore = score;  // Set new baseline for exhaust port
        currentState = USE_THE_FORCE;
        stateTimer = millis();
      }
      break;
      
    case EXHAUST_PORT:
      updateProjectiles();
      
      // Lock-on beep when crosshair is near exhaust port center
      {
        static unsigned long lastLockBeep = 0;
        float lockDist = sqrt(pow(crosshairX - 64, 2) + pow(crosshairY - 40, 2));
        int beepInterval = (lockDist < 5) ? 200 : (lockDist < 12) ? 500 : 9999;
        if(millis() - lastLockBeep > beepInterval) {
          playSFX(SFX_EXHAUST_LOCK);
          lastLockBeep = millis();
        }
      }

       // Check for 5-second timeout — skip entirely if missile is already in the shaft
      if(!inMissileShaft && millis() - exhaustPortStartTime > EXHAUST_PORT_TIMEOUT && !missionSuccess) {
        // Time's up! Go back to trench run
        currentState = TRENCH_RUN;
        stageStartScore = score;
        stateTimer = millis();
        return;
      }
      
      
      // Update missile shaft animation if active
if(inMissileShaft) {
  shaftDepth += 5.0;  // Faster — was 3.0
  missileY = shaftDepth * 0.8;

  // Update shaft segments at matching speed
  for(int i = 0; i < 8; i++) {
    shaftSegments[i].z -= 5.0;
    if(shaftSegments[i].z < -20) {
      shaftSegments[i].z = 140;
      shaftSegments[i].width = 40 - ((int)(shaftDepth/20) * 2);
      if(shaftSegments[i].width < 8) shaftSegments[i].width = 8;
    }
  }

  // Torpedo visually reaches core at depth ~215; trigger impact flash
  if(!shaftImpactFlash && shaftDepth > 215) {
    shaftImpactFlash = true;
    shaftImpactTime  = millis();
    playSFX(SFX_EXPLOSION);   // Reactor core detonation
  }

  // Transition to explosion after flash has played (260 ms)
  if(shaftImpactFlash && millis() - shaftImpactTime > 260) {
    shaftImpactFlash = false;
    inMissileShaft   = false;
    currentState     = DEATH_STAR_EXPLOSION;
    explosionStartTime = millis();
  }
} else {
  // Check for victory/failure after shot animation
  if(shotFired && millis() - shotTime > 1000) { // 1 second animation instead of 3
    if(missionSuccess) {
      // Start missile shaft animation instead of going directly to victory
      inMissileShaft    = true;
      shaftStartTime    = millis();
      shaftDepth        = 0.0;
      missileY          = 0.0;
      shaftImpactFlash  = false;
      initializeShaft();
      playSFX(SFX_SHAFT_WHOOSH);  // Torpedo screaming down the shaft
    } else {
      currentState = GAME_OVER;
    }
  }
  }
  break;

  case DEATH_STAR_EXPLOSION:
      // Run explosion for 4 seconds then go to victory
      if(millis() - explosionStartTime > 6000) {
        currentState = VICTORY;
        playMusic(TUNE_VICTORY_FANFARE);
      }
      break;
    
  case GAME_OVER:
  case VICTORY:
    // Play end-state sounds once
    {
      static GameState lastEndState = TITLE_SCREEN;
      if(currentState != lastEndState) {
        lastEndState = currentState;
        if(currentState == GAME_OVER) playSFX(TUNE_GAME_OVER_JINGLE);
        // Victory fanfare triggered in DEATH_STAR_EXPLOSION case above
      }
    }
    break;
  }
}

void drawGame() {
  if(!sw_display.getBuffer()) return;

  // Dev menu is drawn directly on TFT (like GoL) — skip canvas entirely when active.
  if(devMenuActive) return;

  sw_display.clearDisplay();
  
  switch(currentState) {
    case TITLE_SCREEN:
      drawTitleScreen();
      break;
      case WOMP_RAT_TRAINING:
      drawWompRatTraining();
      drawUI();
      break;
      
    case TRAINING_COMPLETE:
      drawTrainingComplete();
      break;
      
    case FORCE_LESSON:
      drawForceLesson();
      break;
      
    case BLINDFOLD_TRAINING:
      drawBlindfolding();
      drawUI();
      break;
      
    case TRAINING_COMPLETE_2:
      drawTrainingComplete2();
      break;
      
    case TATOOINE_SUNSET:
      drawTatooineSunset();
      break;

    case DEATH_STAR_SURFACE:
      drawCockpitWindow();
      drawDeathStarSurface();
      drawSurfaceTowers();
      drawSurfaceTIEs();
      drawProjectiles();
      drawExplosions();
      // Draw crosshair for targeting
      if(millis() < flashCrosshair) {
        // Flashing crosshair when firing
        sw_display.drawCircle(crosshairX, crosshairY, 8, YELLOW);
        sw_display.drawLine(crosshairX-12, crosshairY, crosshairX+12, crosshairY, YELLOW);
        sw_display.drawLine(crosshairX, crosshairY-12, crosshairX, crosshairY+12, YELLOW);
        // Flashing center circle when firing
        sw_display.fillCircle(crosshairX, crosshairY, 3, WHITE);
      } else {
        // Normal crosshair
        sw_display.drawCircle(crosshairX, crosshairY, 6, GREEN_LASER);
        sw_display.drawLine(crosshairX-8, crosshairY, crosshairX+8, crosshairY, GREEN_LASER);
        sw_display.drawLine(crosshairX, crosshairY-8, crosshairX, crosshairY+8, GREEN_LASER);
        // Normal center dot
        sw_display.drawPixel(crosshairX, crosshairY, WHITE);
      }
      drawUI();
      break;

case SPACE_BATTLE:
  drawCockpitWindow();
  drawStars();
  drawEnemies();
  drawProjectiles();
  drawPowerUps();
  drawExplosions();
  drawSpaceBattlePlayer();
  drawUI();
  break;

case DEATH_STAR_APPROACH:
  drawStars();
  drawDeathStarApproach();  // Death Star drawn first
  drawCockpitWindow();      // Cockpit/console drawn on top, overlaying the Death Star
  drawUI();
  break;

case TRENCH_ENTRY:
  drawTrenchEntry();
  break;

case TRENCH_RUN:
  drawTrench();
  drawTrenchPlayer();
  drawProjectiles();
  drawExplosions();
  drawTrenchHUD();
  drawUI();
  break;

case USE_THE_FORCE:
  drawUseTheForce();
  break;
  
case EXHAUST_PORT:
  if(inMissileShaft) {
    drawMissileShaft();
  } else {
    drawExhaustPort();
    drawTargeting();
    drawProjectiles();
  }
  drawUI();
  break;
  
  case DEATH_STAR_EXPLOSION:
    drawDeathStarExplosion();
    break;

  case VICTORY:
    drawVictory();
    break;

  case GAME_OVER:
    drawGameOver();
    break;
}  // End of switch statement

  // ── Damage flash overlay – drawn LAST so it always appears on top ──
  if(millis() < damageFlash) {
    // Intensity fades over the flash duration: bright at start, gone at end
    unsigned long remaining = damageFlash - millis();
    // Alternate between two colours each frame for a strobe effect
    uint16_t col = ((millis() / 50) % 2 == 0) ? RED : ORANGE;
    // Thick filled edge bars (4px each side) – solid hit vignette
    sw_display.fillRect(0,  0,  128, 4,  col);   // top
    sw_display.fillRect(0,  60, 128, 4,  col);   // bottom
    sw_display.fillRect(0,  0,  4,  64, col);   // left
    sw_display.fillRect(124,0,  4,  64, col);   // right
    // Extra inner outline for depth
    sw_display.drawRect(4, 4, 120, 56, col);
  }

sw_display.display();
}

void handleExhaustPortFiring() {  
  if(currentState == EXHAUST_PORT && targetingActive && !shotFired) {
    targetAccuracy = sqrt(pow(crosshairX - 64, 2) + pow(crosshairY - 40, 2));
    
    if(targetAccuracy <= 3) {
      shotFired = true;
      shotTime = millis();
      missionSuccess = true;
      score += 500;
      fireProjectile(crosshairX, crosshairY, 0, -4, true);
      playSFX(SFX_PROTON_TORPEDO);  // Proton torpedo launched!
    } else {
    }
    // Miss - don't end targeting, let player keep trying until timer expires
      fireProjectile(crosshairX, crosshairY, 0, -4, true);
      playSFX(SFX_LASER);  // Normal laser miss
  }
}
  
void initializeStars() {
  for(int i = 0; i < 20; i++) {
    stars[i].x = random(10, 118);  // Keep away from edges
    stars[i].y = random(10, 54);
    stars[i].z = random(50, 200);
    stars[i].speed = random(1, 4);
  }
}

void updateStars() {
  for(int i = 0; i < 20; i++) {
    stars[i].z -= stars[i].speed;
    
    // Calculate direction from center (64, 32) outward
    float centerX = 64.0;
    float centerY = 32.0;
    float dx = stars[i].x - centerX;
    float dy = stars[i].y - centerY;
    float distance = sqrt(dx*dx + dy*dy);
    
    // Move star outward from center based on speed and proximity
    if(distance > 0) {
      float moveSpeed = stars[i].speed * 0.3;
      stars[i].x += (dx / distance) * moveSpeed;
      stars[i].y += (dy / distance) * moveSpeed;
    }
    
     if(stars[i].z <= 0) {
      stars[i].x = random(54, 74);  // Respawn near center
      stars[i].y = random(22, 42);
       stars[i].z = 200;
      stars[i].speed = random(1, 4);
    }
  }
}

void drawStars() {
  for(int i = 0; i < 20; i++) {
    if(stars[i].z > 0) {
      float size = map(stars[i].z, 200, 1, 1, 4);
      int brightness = map(stars[i].z, 200, 1, 50, 255);
      
      // Draw star with depth effect
      if(size < 1.5) {
        sw_display.drawPixel(stars[i].x, stars[i].y, WHITE);
      } else {
        sw_display.fillCircle(stars[i].x, stars[i].y, size/2, WHITE);
      }
      
      // Draw motion trail outward from center for close stars
      if(stars[i].z < 50) {
        float centerX = 64.0;
        float centerY = 32.0;
        float dx = stars[i].x - centerX;
        float dy = stars[i].y - centerY;
        float distance = sqrt(dx*dx + dy*dy);
        float trailLength = (200 - stars[i].z) / 15;
        
        // Trail points backwards toward center (where star came from)
        float trailX = stars[i].x - (dx / distance) * trailLength;
        float trailY = stars[i].y - (dy / distance) * trailLength;
        sw_display.drawLine(stars[i].x, stars[i].y, trailX, trailY, LIGHT_BLUE);
      }
    }
  }
}

void spawnEnemy() {
  if(enemyCount >= 8) return;

  int actualCount = 0;
  for(int i = 0; i < 8; i++) if(enemies[i].active) actualCount++;
  if(actualCount >= 8) return; // array is indices 0-7, never write past end

  // Difficulty ramp: enemies get faster and more aggressive as score rises
  int stageScore = score - stageStartScore;
  float diff = 1.0f + (stageScore / 500.0f) * 1.2f;  // 1.0 → 2.2 over the stage
  if(diff > 2.2f) diff = 2.2f;

  for(int i = 0; i < 8; i++) {
    if(!enemies[i].active) {
      enemies[i].x = random(20, 108);
      enemies[i].y = random(8, 40);
      enemies[i].z = random(80, 180);
      // Each ship gets a distinct speed personality — some lazy, some fast
      float speedPersonality = 0.6f + random(100) / 100.0f;  // 0.6–1.6×
      enemies[i].vx = (random(2) ? 1 : -1) * (random(10, 25) / 10.0f) * speedPersonality;
      enemies[i].vy = (random(2) ? 1 : -1) * (random(5, 18) / 10.0f) * speedPersonality;
      enemies[i].vz = -(2.5f + random(15, 45) / 10.0f) * diff * speedPersonality;
      enemies[i].angle = random(0, 360);
      enemies[i].type = random(0, 3);
      enemies[i].active = true;
      enemies[i].health = (enemies[i].type == 2) ? 3 : 2;
      enemies[i].lastFire = millis() + random(500, 1500);
      // Only ~60% of ships fire — the rest just manoeuvre, making battles feel real
      enemies[i].canFire  = (random(100) < 60);
      enemies[i].hasFired = false;  // each ship fires its salvo once only

      if(enemies[i].type == 0)      enemies[i].weaponType = 0;
      else if(enemies[i].type == 1) enemies[i].weaponType = random(0, 2);
      else                          enemies[i].weaponType = 2;

      enemyCount++;
      break;
    }
  }
}

void updateEnemies() {
  int stageScore = score - stageStartScore;
  float diff = 1.0f + (stageScore / 500.0f) * 1.2f;
  if(diff > 2.2f) diff = 2.2f;

  for(int i = 0; i < 8; i++) {
    if(enemies[i].active) {
      switch(enemies[i].type) {
        case 0: // TIE Fighter - straight attack run
          enemies[i].x += enemies[i].vx;
          enemies[i].y += enemies[i].vy;
          enemies[i].z += enemies[i].vz;
          break;

        case 1: // TIE Interceptor - aggressive weaving
          enemies[i].x += enemies[i].vx + sin(millis() * 0.007f + i) * 2.5f;
          enemies[i].y += enemies[i].vy + cos(millis() * 0.005f + i) * 1.8f;
          enemies[i].z += enemies[i].vz * 1.3f;
          break;

        case 2: // TIE Bomber - slow but menacing, drifts toward player
          enemies[i].x += enemies[i].vx * 0.6f + (crosshairX - enemies[i].x) * 0.005f;
          enemies[i].y += enemies[i].vy * 0.4f + sin(millis() * 0.003f + i) * 1.2f;
          enemies[i].z += enemies[i].vz * 0.9f;
          if(random(150) < 1) enemies[i].vx = -enemies[i].vx;
          break;
      }

      enemies[i].angle += 6;

      if(enemies[i].z <= 10 || enemies[i].x < 5 || enemies[i].x > 123 ||
         enemies[i].y < 5  || enemies[i].y > 59) {
        enemies[i].active = false;
        enemyCount--;
        continue;
      }

      // Enemy firing — only ships flagged canFire shoot, and only once
      if(enemies[i].canFire && !enemies[i].hasFired && (currentState == SPACE_BATTLE || currentState == DEATH_STAR_APPROACH)) {
        // Fire rates start slow and tighten with difficulty — always dodgeable
        int fireDelay = (int)(2500 / diff);   // ~2500ms at start, ~1100ms at max diff
        if(enemies[i].weaponType == 1) fireDelay = (int)(2000 / diff);
        if(enemies[i].weaponType == 2) fireDelay = (int)(3500 / diff);
        if(fireDelay < 800) fireDelay = 800;  // hard floor so it never gets ridiculous

        if(millis() - enemies[i].lastFire > fireDelay + random(300)) {
          enemies[i].lastFire = millis();

          float dx = crosshairX - enemies[i].x;
          float dy = crosshairY - enemies[i].y;
          float distance = sqrt(dx*dx + dy*dy);

          if(distance > 0.1f && distance < 130) {
            // Bullets noticeably faster than before
            float speed = 3.5f + diff * 0.5f;
            float vx = (dx / distance) * speed;
            float vy = (dy / distance) * speed;

            switch(enemies[i].weaponType) {
              case 0: // Single shot
                fireProjectile(enemies[i].x, enemies[i].y, vx, vy, false);
                enemies[i].hasFired = true;
                break;

              case 1: { // Burst — 3 slightly spread shots
                fireProjectile(enemies[i].x, enemies[i].y, vx, vy, false);
                static unsigned long lastBurst[8] = {0};
                if(millis() - lastBurst[i] > 150) {
                  fireProjectile(enemies[i].x, enemies[i].y, vx * 1.15f, vy * 0.9f, false);
                  fireProjectile(enemies[i].x, enemies[i].y, vx * 0.85f, vy * 1.1f, false);
                  lastBurst[i] = millis();
                }
                enemies[i].hasFired = true;
                break;
              }

              case 2: { // Spread cone
                float ang = atan2(vy, vx);
                float spread = 0.35f;
                fireProjectile(enemies[i].x, enemies[i].y, vx, vy, false);
                fireProjectile(enemies[i].x, enemies[i].y,
                               cos(ang - spread) * speed, sin(ang - spread) * speed, false);
                fireProjectile(enemies[i].x, enemies[i].y,
                               cos(ang + spread) * speed, sin(ang + spread) * speed, false);
                enemies[i].hasFired = true;
                break;
              }
            }
          }
        }
      }
    }
  }
}

void drawEnemies() {
  for(int i = 0; i < 8; i++) {
    if(enemies[i].active) {
      float scale = map(enemies[i].z, 200, 10, 2, 12);
      if(scale < 1)  scale = 1;
      if(scale > 13) scale = 13;
      int drawX = (int)enemies[i].x;
      int drawY = (int)enemies[i].y;

      // Skip drawing if ship is too close to canvas edge — wing extensions at scale 13
      // can reach 30px from centre, writing outside the 128x64 buffer = memory corruption.
      int margin = (int)(scale * 2.5f) + 2;
      if(drawX < margin || drawX > 128 - margin ||
         drawY < margin || drawY > 64  - margin) continue;

      switch(enemies[i].type) {
        case 0: drawVectorTIE(drawX, drawY, scale, enemies[i].angle);         break;
        case 1: drawVectorInterceptor(drawX, drawY, scale, enemies[i].angle); break;
        case 2: drawVectorBomber(drawX, drawY, scale, enemies[i].angle);      break;
      }
    }
  }
}

void drawVectorTIE(float x, float y, float scale, float angle) {
  int cockpitR = max(2, (int)(scale / 2));

  // Cockpit sphere — white outer ring, light inner detail
  sw_display.drawCircle(x, y, cockpitR, WHITE);
  if(scale > 5) sw_display.drawCircle(x, y, cockpitR - 1, GREY);

  // Hexagonal viewport at larger scales
  if(scale > 4) {
    int ws = max(1, (int)(cockpitR * 0.65f));
    for(int s = 0; s < 6; s++) {
      float a1 = (s * PI) / 3.0f;
      float a2 = ((s+1) * PI) / 3.0f;
      sw_display.drawLine(x + cos(a1)*ws, y + sin(a1)*ws,
                          x + cos(a2)*ws, y + sin(a2)*ws, DARK_GREY);
    }
  }

  // Solar panels — white with grey internal lines
  int pw  = max(2, (int)(scale * 0.6f));
  int ph  = max(6, (int)(scale * 1.8f));
  int pOff = cockpitR + pw / 2 + 2;

  int lx = x - pOff;
  sw_display.drawRect(lx - pw/2, y - ph/2, pw, ph, WHITE);
  if(scale > 7) {
    sw_display.drawLine(lx - pw/2, y - ph/6, lx + pw/2, y - ph/6, GREY);
    sw_display.drawLine(lx - pw/2, y + ph/6, lx + pw/2, y + ph/6, GREY);
    sw_display.drawLine(lx, y - ph/2, lx, y + ph/2, GREY);
  }

  int rx = x + pOff;
  sw_display.drawRect(rx - pw/2, y - ph/2, pw, ph, WHITE);
  if(scale > 7) {
    sw_display.drawLine(rx - pw/2, y - ph/6, rx + pw/2, y - ph/6, GREY);
    sw_display.drawLine(rx - pw/2, y + ph/6, rx + pw/2, y + ph/6, GREY);
    sw_display.drawLine(rx, y - ph/2, rx, y + ph/2, GREY);
  }

  // Struts — white connecting cockpit to panels
  sw_display.drawLine(x - cockpitR, y, lx + pw/2, y, WHITE);
  sw_display.drawLine(x + cockpitR, y, rx - pw/2, y, WHITE);

  // Panel corner highlights
  if(scale > 5) {
    sw_display.drawPixel(lx - pw/2, y - ph/2, WHITE);
    sw_display.drawPixel(lx - pw/2, y + ph/2, WHITE);
    sw_display.drawPixel(rx + pw/2, y - ph/2, WHITE);
    sw_display.drawPixel(rx + pw/2, y + ph/2, WHITE);
  }

  // Engine glow — blue/green ion engines behind cockpit (flickers)
  if(scale > 5) {
    bool glowOn = (millis() / 120) % 2 == 0;
    uint16_t glowCol = glowOn ? LIGHT_BLUE : CYAN;
    sw_display.drawPixel(x, y + cockpitR + 1, glowCol);
    if(scale > 7) {
      sw_display.drawPixel(x - 1, y + cockpitR + 2, glowOn ? CYAN : LIGHT_BLUE);
      sw_display.drawPixel(x + 1, y + cockpitR + 2, glowOn ? CYAN : LIGHT_BLUE);
    }
  }
}

void drawVectorInterceptor(float x, float y, float scale, float angle) {
  // TIE Interceptor — sleek angular dagger wings, white with orange engine accents

  int cockpitR = max(2, (int)(scale / 3));
  sw_display.drawCircle(x, y, cockpitR, WHITE);
  if(scale > 4) {
    int ws = cockpitR;
    sw_display.drawRect(x - ws/2, y - ws/2, ws, ws, GREY);
  }

  int wingLen   = max(8, (int)(scale * 2.0f));
  int wingSpread= max(3, (int)(scale * 0.8f));
  int cEdge     = cockpitR + 1;

  // Left dagger wing — white outline
  int lbx = x - cEdge,  ltx = x - cEdge - wingLen;
  sw_display.drawLine(lbx, y - wingSpread/2, ltx + wingSpread/3, y - wingSpread/4, WHITE);
  sw_display.drawLine(ltx + wingSpread/3, y - wingSpread/4, ltx, y, WHITE);
  sw_display.drawLine(ltx, y, ltx + wingSpread/3, y + wingSpread/4, WHITE);
  sw_display.drawLine(ltx + wingSpread/3, y + wingSpread/4, lbx, y + wingSpread/2, WHITE);
  sw_display.drawLine(lbx, y + wingSpread/2, lbx, y - wingSpread/2, WHITE);
  // Internal spine
  if(scale > 6) sw_display.drawLine(lbx, y, ltx + wingSpread/3, y, GREY);

  // Right dagger wing — mirror
  int rbx = x + cEdge,  rtx = x + cEdge + wingLen;
  sw_display.drawLine(rbx, y - wingSpread/2, rtx - wingSpread/3, y - wingSpread/4, WHITE);
  sw_display.drawLine(rtx - wingSpread/3, y - wingSpread/4, rtx, y, WHITE);
  sw_display.drawLine(rtx, y, rtx - wingSpread/3, y + wingSpread/4, WHITE);
  sw_display.drawLine(rtx - wingSpread/3, y + wingSpread/4, rbx, y + wingSpread/2, WHITE);
  sw_display.drawLine(rbx, y + wingSpread/2, rbx, y - wingSpread/2, WHITE);
  if(scale > 6) sw_display.drawLine(rbx, y, rtx - wingSpread/3, y, GREY);

  // Struts
  sw_display.drawLine(x - cockpitR, y, lbx, y, WHITE);
  sw_display.drawLine(x + cockpitR, y, rbx, y, WHITE);

  // Laser cannon tips — bright orange glow (the one colour accent)
  if(scale > 5) {
    bool glowOn = (millis() / 100) % 2 == 0;
    uint16_t tipCol = glowOn ? ORANGE : RED_LASER;
    sw_display.drawPixel(ltx, y, tipCol);
    sw_display.drawPixel(rtx, y, tipCol);
    if(scale > 8) {
      sw_display.drawLine(ltx-1, y, ltx+1, y, tipCol);
      sw_display.drawLine(rtx-1, y, rtx+1, y, tipCol);
    }
  }
}

void drawVectorBomber(float x, float y, float scale, float angle) {
  // TIE Bomber — dual-pod design, white hull, blue ion engine exhaust

  int podR       = max(2, (int)(scale * 0.4f));
  int podSpacing = max(6, (int)(scale * 1.0f));
  int connW      = max(2, (int)(scale * 0.3f));

  // Left pod — white
  sw_display.drawCircle(x - podSpacing/2, y, podR, WHITE);
  if(scale > 4) {
    int ws = (int)(podR * 0.8f);
    sw_display.drawRect(x - podSpacing/2 - ws/2, y - ws/2, ws, ws/2, GREY);
  }

  // Right pod — white
  sw_display.drawCircle(x + podSpacing/2, y, podR, WHITE);
  if(scale > 5) {
    sw_display.drawRect(x + podSpacing/2 - 1, y - 1, 2, 2, GREY);
  }

  // Connecting spine — white
  int cLeft  = x - podSpacing/2 + podR;
  int cRight = x + podSpacing/2 - podR;
  sw_display.drawRect(cLeft, y - connW/2, cRight - cLeft, connW, WHITE);

  // Solar wings — white rectangles at sides
  int wh    = max(6, (int)(scale * 1.5f));
  int ww    = max(2, (int)(scale * 0.5f));
  int wOff  = podSpacing/2 + podR + ww/2 + 1;

  int lwx = x - wOff;
  sw_display.drawRect(lwx - ww/2, y - wh/2, ww, wh, WHITE);
  if(scale > 5) {
    sw_display.drawLine(lwx - ww/2+1, y - wh/3, lwx + ww/2-1, y - wh/4, GREY);
    sw_display.drawLine(lwx - ww/2+1, y + wh/3, lwx + ww/2-1, y + wh/4, GREY);
  }

  int rwx = x + wOff;
  sw_display.drawRect(rwx - ww/2, y - wh/2, ww, wh, WHITE);
  if(scale > 5) {
    sw_display.drawLine(rwx - ww/2+1, y - wh/4, rwx + ww/2-1, y - wh/3, GREY);
    sw_display.drawLine(rwx - ww/2+1, y + wh/4, rwx + ww/2-1, y + wh/3, GREY);
  }

  // Support struts — white
  sw_display.drawLine(x - podSpacing/2 - podR, y, lwx + ww/2, y, WHITE);
  sw_display.drawLine(x + podSpacing/2 + podR, y, rwx - ww/2, y, WHITE);

  // Engine exhaust — blue glow at bottom wing edges (flickers)
  if(scale > 4) {
    bool glowOn = (millis() / 140) % 2 == 0;
    uint16_t exhaustCol = glowOn ? LIGHT_BLUE : CYAN;
    sw_display.drawPixel(lwx - ww/2, y + wh/2, exhaustCol);
    sw_display.drawPixel(rwx + ww/2, y + wh/2, exhaustCol);
    if(scale > 7) {
      sw_display.drawPixel(lwx,       y + wh/2 + 1, glowOn ? CYAN : BLUE);
      sw_display.drawPixel(rwx,       y + wh/2 + 1, glowOn ? CYAN : BLUE);
    }
  }
}

void fireProjectile(float x, float y, float vx, float vy, bool isPlayer) {
  // Check if array is full BEFORE looping
  int activeCount = 0;
  int firstFree = -1;
  for(int i = 0; i < 12; i++) {
    if(projectiles[i].active) {
      activeCount++;
    } else if(firstFree == -1) {
      firstFree = i; // Remember first free slot
    }
  }
  
  // Don't spawn if array is nearly full OR no free slot found
  if(activeCount >= 10 || firstFree == -1) return;
  
  // Use the first free slot we found
  projectiles[firstFree].x = x;
  projectiles[firstFree].y = y;
  projectiles[firstFree].z = 50;
  projectiles[firstFree].vx = vx;
  projectiles[firstFree].vy = vy;
  projectiles[firstFree].vz = 0;
  projectiles[firstFree].active = true;
  projectiles[firstFree].isPlayerShot = isPlayer;
  projectiles[firstFree].damage = isPlayer ? 1 : 5;
  projectiles[firstFree].birthTime = millis();
}

// Fire a perspective-correct trench bolt.
// wx = world X offset from centre (e.g. playerX-64), wy unused (Y is derived from z)
// The bolt stores world-x in x, world-z in z, advances z each frame.
// Project a trench TIE z-depth to screen position
// Returns scale (0=far/invisible, up to ~12=close)
float trenchTIEProject(const TrenchTIE &tie, int &scx, int &scy) {
  if(tie.z <= TR_NEAR || tie.z > TR_FAR) return 0;
  float t = (tie.z - TR_NEAR) / (float)(TR_FAR - TR_NEAR);
  float scale = (1.0f - t) * 12.0f;
  float nearW = 127.0f, farW = 71.0f;
  float depthW = nearW + t * (farW - nearW);
  scx = (int)(64 + (tie.x - 64) * (depthW / nearW));
  // Vertical: tie.y fraction maps near(21..63) to far(22..56)
  float topY = 21.0f + t * (22.0f - 21.0f);
  float botY = 63.0f + t * (56.0f - 63.0f);
  scy = (int)(topY + tie.y * (botY - topY));
  scx = constrain(scx, 4, 123);
  scy = constrain(scy, 21, 62);
  return scale;
}

void fireTrenchBolt(float wx, bool isPlayer) {
  int firstFree = -1;
  int activeCount = 0;
  for(int i = 0; i < 12; i++) {
    if(projectiles[i].active) activeCount++;
    else if(firstFree < 0) firstFree = i;
  }
  if(activeCount >= 10 || firstFree < 0) return;
  Projectile &p = projectiles[firstFree];
  p.vx         = wx;       // world X offset from centre (persists)
  p.x          = 64 + wx;  // initial screen X (near plane)
  p.y          = playerY;  // initial screen Y
  p.z          = 60.0f;    // start just ahead of player
  p.vy         = 0;
  p.vz         = 14.0f;    // advance down the trench per frame
  p.active     = true;
  p.isPlayerShot = isPlayer;
  p.damage     = isPlayer ? 1 : 5;
  p.birthTime  = millis();
}

void updateProjectiles() {
  // First pass: cleanup stuck projectiles
  for(int i = 0; i < 12; i++) {
    if(projectiles[i].active && millis() - projectiles[i].birthTime > 5000) {
      projectiles[i].active = false; // Remove projectiles older than 5 seconds
    }
  }
  
  for(int i = 0; i < 12; i++) {
    if(projectiles[i].active) {
      if(currentState == TRENCH_RUN && projectiles[i].isPlayerShot
         && projectiles[i].vz > 0) {
        // Perspective trench bolt — advance z, reproject screen x,y
        projectiles[i].z += projectiles[i].vz;
        // Despawn when bolt reaches far end
        if(projectiles[i].z > TR_FAR) { projectiles[i].active = false; continue; }
        // Reproject: screen x from world x offset and z
        float t_b = (projectiles[i].z - TR_NEAR) / (float)(TR_FAR - TR_NEAR);
        if(t_b < 0) t_b = 0; if(t_b > 1) t_b = 1;
        float nearW = 127.0f, farW = 71.0f;
        float depthW = nearW + t_b * (farW - nearW);
        projectiles[i].x = 64 + projectiles[i].vx * (depthW / nearW);
        // screen y: interpolate between player Y and far-centre Y as bolt advances
        int nearCY_b = (int)playerY;
        int farCY_b  = (22 + 56) / 2;   // 39
        projectiles[i].y = nearCY_b + t_b * (farCY_b - nearCY_b);
      } else {
        projectiles[i].x += projectiles[i].vx;
        projectiles[i].y += projectiles[i].vy;
      }
      // Prevent coordinate overflow
      if(projectiles[i].x < -100 || projectiles[i].x > 228 ||
         projectiles[i].y < -100 || projectiles[i].y > 164) {
        projectiles[i].active = false;
        continue;
      }
      
      // Remove if off screen or too old
      if(projectiles[i].x < -5 || projectiles[i].x > 133 ||
         projectiles[i].y < 8 || projectiles[i].y > 69 ||
         millis() - projectiles[i].birthTime > 3000) {
        projectiles[i].active = false;
        continue;
      }
      
      // Check collisions
      if(projectiles[i].isPlayerShot) {
        if(currentState == TRENCH_RUN) {
          for(int seg = 0; seg < 6; seg++) {
            if(!trenchSegments[seg].hasObstacle) continue;
            float z = trenchSegments[seg].z;
            if(z <= TR_NEAR || z > TR_FAR) continue;

            float t = zToT(z);
            int lx_t = (int)(0   + t*(28 - 0));
            int rx_t = (int)(127 + t*(99-127));
            int lx_b = (int)(24  + t*(44 - 24));
            int rx_b = (int)(103 + t*(83-103));
            int wty  = (int)(21  + t*(22 - 21));
            int wby  = (int)(63  + t*(56 - 63));
            bool onLeft = (trenchSegments[seg].turretSide % 2 == 0);
            int tx = onLeft ? (lx_t+lx_b)/2 : (rx_t+rx_b)/2;
            int ty = (wty+wby)/2;
            float psc = 1.0f - t;
            ty += max(3,(int)(psc*14)) + max(2,(int)(psc*6))/2;
            int hitRadius = max(6, (int)(psc * 18));

            float dist = sqrt(pow(projectiles[i].x - tx, 2) +
                              pow(projectiles[i].y - ty, 2));
            if(dist < hitRadius) {
              trenchSegments[seg].turretHealth -= 1;
              projectiles[i].active = false;
              if(trenchSegments[seg].turretHealth <= 0) {
                trenchSegments[seg].hasObstacle = false;
                score += 20;
                createDebrisExplosion(tx, ty);
                playSFX(SFX_EXPLOSION);
                if(vaderTIE.active && vaderTIE.anger < 2) vaderTIE.anger++;
              } else {
                score += 5;
                flashCrosshair = millis() + 120;
                hitFlash = true;
              }
              break;
            }
          }
        }
        
        // Check trench TIE hits
        if(currentState == TRENCH_RUN) {
          for(int ti = 0; ti < NUM_TRENCH_TIES; ti++) {
            TrenchTIE &tie = trenchTIEs[ti];
            if(!tie.active) continue;
            int scx, scy;
            float scale = trenchTIEProject(tie, scx, scy);
            if(scale < 0.5f) continue;
            int hitR = max(5, (int)(scale * 1.2f));
            float dist = sqrt(pow(projectiles[i].x - scx, 2) +
                              pow(projectiles[i].y - scy, 2));
            if(dist < hitR) {
              tie.active    = false;
              tie.nextSpawn = millis() + random(4000, 7000);
              projectiles[i].active = false;
              score += 30;
              createExplosion(scx, scy);
              playSFX(SFX_EXPLOSION);
              break;
            }
          }
        }

        // Check enemy hits
        for(int j = 0; j < 8; j++) {
          if(enemies[j].active) {
            float dist = sqrt(pow(projectiles[i].x - enemies[j].x, 2) + 
                            pow(projectiles[i].y - enemies[j].y, 2));
            if(dist < 8) {
              enemies[j].health -= projectiles[i].damage;
              projectiles[i].active = false;
              
              if(enemies[j].health <= 0) {
                enemies[j].active = false;
                enemyCount--;
                score += (enemies[j].type + 1) * 10;
                 
              createExplosion(enemies[j].x, enemies[j].y);
              }
              break;
            }
          }
        }
       } else {
        // Enemy shot - check player hit
        // In DEATH_STAR_SURFACE the player's "location" is the crosshair position
        float playerHitX = (currentState == SPACE_BATTLE || currentState == DEATH_STAR_SURFACE) ? crosshairX : playerX;
        float playerHitY = (currentState == SPACE_BATTLE || currentState == DEATH_STAR_SURFACE) ? crosshairY : playerY;
        
        float dist = sqrt(pow(projectiles[i].x - playerHitX, 2) + 
                        pow(projectiles[i].y - playerHitY, 2));
        if(dist < 6) {
          // Only take damage if not in invincibility window (already have this check below)
          if(millis() - lastDamageTime > INVINCIBILITY_TIME) {
            shields -= projectiles[i].damage;
            damageFlash = millis() + 400;
            lastDamageTime = millis();
            playSFX(SFX_HIT_PLAYER);  // Shield hit sound
            
            if(shields <= 0) {
              currentState = GAME_OVER;
            }
          }
          // Always destroy the projectile even during invincibility
          projectiles[i].active = false;
        }
      } 
      } 
    }
  }

void drawProjectiles() {
  for(int i = 0; i < 12; i++) {
    if(projectiles[i].active) {
      int px = (int)projectiles[i].x;
      int py = (int)projectiles[i].y;

      if(currentState == TRENCH_RUN && projectiles[i].isPlayerShot
         && projectiles[i].vz > 0) {
        // Perspective trench bolt — size shrinks as it recedes
        float t_b = (projectiles[i].z - TR_NEAR) / (float)(TR_FAR - TR_NEAR);
        if(t_b < 0) t_b = 0; if(t_b > 1) t_b = 1;
        int blen = max(1, (int)((1.0f - t_b) * 6));  // long near, short far
        sw_display.drawLine(px, py, px, py + blen, GREEN_LASER);  // trail behind
        sw_display.drawPixel(px, py, YELLOW);  // bright tip

      } else if(projectiles[i].isPlayerShot) {
        // Space battle player shot — bright green laser bolt with yellow core
        float age = (millis() - projectiles[i].birthTime) / 1000.0f;
        // Bolt direction: normalise velocity and draw a short line
        float len = sqrt(projectiles[i].vx*projectiles[i].vx + projectiles[i].vy*projectiles[i].vy);
        if(len > 0.1f) {
          int tx = px - (int)(projectiles[i].vx / len * 5);
          int ty = py - (int)(projectiles[i].vy / len * 5);
          sw_display.drawLine(px, py, tx, ty, GREEN_LASER);
          sw_display.drawPixel(px, py, YELLOW);  // bright tip
        } else {
          sw_display.fillCircle(px, py, 2, GREEN_LASER);
        }

      } else {
        // Enemy shot — small spinning diamond, easy to spot, hard to ignore
        // Derive spin angle from birth time so each bolt has its own spin phase
        float spin = ((millis() - projectiles[i].birthTime) * 0.012f) + i * 0.7f;
        float cs = cos(spin), sn = sin(spin);
        const int R = 2;  // diamond half-size (tiny = 2px)

        // Four diamond points rotated by spin
        int ax = px + (int)(cs * R),  ay = py + (int)(sn * R);
        int bx = px + (int)(-sn * R), by = py + (int)(cs * R);
        int cx2= px + (int)(-cs * R), cy2= py + (int)(-sn * R);
        int dx = px + (int)(sn * R),  dy = py + (int)(-cs * R);

        // Draw diamond outline in red with orange centre dot
        sw_display.drawLine(ax, ay, bx, by, RED_LASER);
        sw_display.drawLine(bx, by, cx2,cy2, RED_LASER);
        sw_display.drawLine(cx2,cy2, dx, dy, RED_LASER);
        sw_display.drawLine(dx, dy, ax, ay, RED_LASER);
        sw_display.drawPixel(px, py, ORANGE);   // bright centre — makes it pop
      }
    }
  }
}

void drawVectorEffect(float x1, float y1, float x2, float y2) {
  for(int i = 0; i < 10; i++) {
    if(!vectorTrails[i].active) {
      vectorTrails[i].x1 = x1;
      vectorTrails[i].y1 = y1;
      vectorTrails[i].x2 = x2;
      vectorTrails[i].y2 = y2;
      vectorTrails[i].fadeTime = millis() + random(200, 800);
      vectorTrails[i].active = true;
      break;
    }
  }
}

void updateVectorTrails() {
  for(int i = 0; i < 10; i++) {
    if(vectorTrails[i].active) {
      if(millis() > vectorTrails[i].fadeTime) {
        vectorTrails[i].active = false;
      }
    }
  }
}

void drawVectorTrails() {
  for(int i = 0; i < 10; i++) {
    if(vectorTrails[i].active) {
      sw_display.drawLine(vectorTrails[i].x1, vectorTrails[i].y1,
                      vectorTrails[i].x2, vectorTrails[i].y2, YELLOW);
    }
  }
}

// ── Ship fireball explosion ───────────────────────────────────────────────────
void createExplosion(float x, float y, bool small) {
  for(int i = 0; i < 5; i++) {
    if(!explosions[i].active) {
      explosions[i].x       = x;
      explosions[i].y       = y;
      explosions[i].frame   = 0;
      explosions[i].active  = true;
      explosions[i].small   = small;
      explosions[i].isDebris = false;
      break;
    }
  }
}

// ── Building / tower debris explosion ────────────────────────────────────────
// 6 angular debris chunks fly outward + 4 sparks arc up, all affected by
// pseudo-gravity so fragments fall back down like a collapsing structure.
void createDebrisExplosion(float x, float y) {
  for(int i = 0; i < 5; i++) {
    if(!explosions[i].active) {
      explosions[i].x        = x;
      explosions[i].y        = y;
      explosions[i].frame    = 0;
      explosions[i].active   = true;
      explosions[i].small    = false;
      explosions[i].isDebris = true;

      // 6 debris chunks spread in a low arc (no upward bias — chunks fall forward)
      // Angles chosen so debris fans left/right and slightly up, not directly up
      // (buildings collapse outward and down, not like a fireball)
      float chunkAngles[6] = { 200, 230, 260, 280, 310, 340 };
      for(int j = 0; j < 6; j++) {
        float spd = 0.6f + random(8) / 10.0f;   // 0.6 – 1.4 px/frame
        float rad = radians(chunkAngles[j] + random(-15, 15));
        explosions[i].debrisX[j]  = x;
        explosions[i].debrisY[j]  = y;
        explosions[i].debrisVX[j] = cos(rad) * spd;
        explosions[i].debrisVY[j] = sin(rad) * spd;
      }

      // 4 sparks fly steeply upward then arc down (like struck embers)
      for(int j = 0; j < 4; j++) {
        float rad = radians(220 + j * 28 + random(-10, 10));
        float spd = 1.0f + random(6) / 10.0f;
        explosions[i].sparkX[j]  = x;
        explosions[i].sparkY[j]  = y;
        explosions[i].sparkVX[j] = cos(rad) * spd;
        explosions[i].sparkVY[j] = sin(rad) * spd * 0.7f;
      }
      break;
    }
  }
}

void updateExplosions() {
  for(int i = 0; i < 5; i++) {
    if(!explosions[i].active) continue;

    explosions[i].frame++;
    int maxFrame = explosions[i].small ? 10 : 22;
    if(explosions[i].frame > maxFrame) {
      explosions[i].active = false;
      continue;
    }

    if(explosions[i].isDebris) {
      // Move debris chunks — light gravity each frame
      for(int j = 0; j < 6; j++) {
        explosions[i].debrisX[j]  += explosions[i].debrisVX[j];
        explosions[i].debrisY[j]  += explosions[i].debrisVY[j];
        explosions[i].debrisVY[j] += 0.08f;   // gravity
        explosions[i].debrisVX[j] *= 0.97f;   // slight air drag
      }
      // Move sparks — stronger gravity, they arc visibly
      for(int j = 0; j < 4; j++) {
        explosions[i].sparkX[j]  += explosions[i].sparkVX[j];
        explosions[i].sparkY[j]  += explosions[i].sparkVY[j];
        explosions[i].sparkVY[j] += 0.14f;
      }
    }
  }
}

void drawExplosions() {
  for(int i = 0; i < 5; i++) {
    if(!explosions[i].active) continue;

    int   frame = explosions[i].frame;
    float x     = explosions[i].x;
    float y     = explosions[i].y;

    if(explosions[i].isDebris) {
      // ── Building debris ──────────────────────────────────────────────────────

      // Frame 0-2: sharp white impact flash at strike point
      if(frame <= 2) {
        int flashR = frame + 1;
        sw_display.fillCircle(x, y, flashR, WHITE);
        sw_display.drawCircle(x, y, flashR + 2, YELLOW);
      }

      // Frame 1+: draw debris chunks as small grey rectangles (1×2 or 2×1)
      if(frame >= 1) {
        for(int j = 0; j < 6; j++) {
          int dx = (int)explosions[i].debrisX[j];
          int dy = (int)explosions[i].debrisY[j];
          // Fade chunks from GREY to DARK_GREY after frame 12
          uint16_t chunkCol = (frame < 12) ? GREY : DARK_GREY;
          // Alternate horizontal / vertical 1×2 slivers for angular look
          if(j % 2 == 0) sw_display.fillRect(dx,   dy,   2, 1, chunkCol);
          else            sw_display.fillRect(dx,   dy,   1, 2, chunkCol);
          // Small shadow pixel one step behind each chunk
          if(frame < 15) sw_display.drawPixel(dx - (int)explosions[i].debrisVX[j],
                                              dy - (int)explosions[i].debrisVY[j],
                                              DARK_GREY);
        }

        // Draw sparks as single orange/yellow pixels that cool to red
        for(int j = 0; j < 4; j++) {
          int sx = (int)explosions[i].sparkX[j];
          int sy = (int)explosions[i].sparkY[j];
          uint16_t sparkCol;
          if(frame < 6)       sparkCol = YELLOW;
          else if(frame < 12) sparkCol = ORANGE;
          else                sparkCol = RED;
          sw_display.drawPixel(sx, sy, sparkCol);
          // Leave a fading trail pixel one frame behind
          if(frame < 14) {
            sw_display.drawPixel(sx - (int)explosions[i].sparkVX[j],
                                 sy - (int)explosions[i].sparkVY[j], DARK_ORANGE);
          }
        }

        // Dust cloud: a couple of dark pixels drifting down near origin (frames 8-20)
        if(frame >= 8 && frame <= 20) {
          int dustSpread = (frame - 8) / 2;
          sw_display.drawPixel(x - dustSpread,     y + dustSpread/2,     DARK_GREY);
          sw_display.drawPixel(x + dustSpread - 1, y + dustSpread/2 + 1, DARK_GREY);
        }
      }

    } else {
      // ── Ship fireball (unchanged) ─────────────────────────────────────────────
      float sz = explosions[i].small ? 0.5f : 1.0f;

      if(frame < 5) {
        int radius = (int)(frame * 2 * sz);
        sw_display.fillCircle(x, y, max(1, radius/2), WHITE);
        sw_display.drawCircle(x, y, max(1, radius), YELLOW);
      } else if(frame < 12) {
        int radius = (int)(frame * 2 * sz);
        sw_display.drawCircle(x, y, max(1, radius), ORANGE);
        sw_display.fillCircle(x, y, max(1, radius/2), DARK_ORANGE);
      } else {
        for(int j = 0; j < 8; j++) {
          float ang = j * 45 + frame * 10;
          int debrisX = x + cos(radians(ang)) * (frame - 5) * sz;
          int debrisY = y + sin(radians(ang)) * (frame - 5) * sz;
          uint16_t col = (j % 2 == 0) ? ORANGE : RED;
          sw_display.fillCircle(debrisX, debrisY, 1, col);
        }
      }
    }
  }
}

void initializeTrench() {
  for(int i = 0; i < 6; i++) {
    trenchSegments[i].z           = 40 + i * 38;
    trenchSegments[i].hasObstacle = (i > 0) && (random(100) < 45);
    trenchSegments[i].obstacleX   = random(30, 98);
    trenchSegments[i].turretHealth= 2;
    trenchSegments[i].turretSide  = random(0, 2);   // 0=left wall 1=right wall
    trenchSegments[i].turretLocked= false;
    trenchSegments[i].lockTime    = 0;
  }
  // Initialise trench TIE slots
  for(int ti = 0; ti < NUM_TRENCH_TIES; ti++) {
    trenchTIEs[ti].active    = false;
    trenchTIEs[ti].z         = TR_FAR;
    trenchTIEs[ti].x         = 64;
    trenchTIEs[ti].y         = 0.5f;
    trenchTIEs[ti].vx        = 0;
    trenchTIEs[ti].vy        = 0;
    trenchTIEs[ti].health    = 1;
    trenchTIEs[ti].lastFire  = 0;
    trenchTIEs[ti].nextSpawn = millis() + 3000 + ti * 2000;
    trenchTIEs[ti].anger     = 0;
  }
}

void updateTrench() {
  // Speed ramps from 3 → 7 over the stage
  trenchSpeed += 0.008f;
  if(trenchSpeed > 10.0f) trenchSpeed = 10.0f;

  // ── Segment scrolling ─────────────────────────────────────────────────────
  for(int i = 0; i < 6; i++) {
    trenchSegments[i].z -= trenchSpeed;

    // Turret lock-on: starts 1.5 s warning before it fires
    if(trenchSegments[i].hasObstacle && !trenchSegments[i].turretLocked
       && trenchSegments[i].z < 90 && trenchSegments[i].z > 20) {
      trenchSegments[i].turretLocked = true;
      trenchSegments[i].lockTime     = millis();
    }

    // Turret fires 1.5 s after lock (aimed at player X)
    if(trenchSegments[i].hasObstacle && trenchSegments[i].turretLocked
       && trenchSegments[i].z > TR_NEAR) {
      unsigned long sincelock = millis() - trenchSegments[i].lockTime;
      if(sincelock > 1500 && sincelock < 1600) {
        float z = trenchSegments[i].z;
        float t_fire = zToT(z);
        int lx_tf = (int)(0   + t_fire*(28 - 0));
        int rx_tf = (int)(127 + t_fire*(99-127));
        int lx_bf = (int)(24  + t_fire*(44 - 24));
        int rx_bf = (int)(103 + t_fire*(83-103));
        int wty_f = (int)(21  + t_fire*(22 - 21));
        int wby_f = (int)(63  + t_fire*(56 - 63));
        bool onLeft_f = (trenchSegments[i].turretSide % 2 == 0);
        float psc_f = 1.0f - t_fire;
        int tx = onLeft_f ? (lx_tf+lx_bf)/2 : (rx_tf+rx_bf)/2;
        int ty = (wty_f+wby_f)/2 + max(3,(int)(psc_f*14)) + max(2,(int)(psc_f*6))/2;
        float dx = playerX - tx;
        float dy = playerY - ty;
        float dist = sqrt(dx*dx + dy*dy);
        if(dist > 0.5f) {
          float spd = 2.8f;
          fireProjectile(tx, ty, (dx/dist)*spd, (dy/dist)*spd, false);
          playSFX(SFX_TIE_FLYBY);
        }
        trenchSegments[i].turretLocked = false;
        trenchSegments[i].lockTime     = millis();
      }
    }

    // Segment passed player — respawn at back
    if(trenchSegments[i].z < TR_NEAR) {
      trenchSegments[i].z           = TR_FAR;
      trenchSegments[i].hasObstacle = (random(100) < 50);
      trenchSegments[i].turretSide  = random(0, 2);
      trenchSegments[i].turretHealth= 2;
      trenchSegments[i].turretLocked= false;
      trenchSegments[i].lockTime    = 0;
    }
  }

  // ── Trench TIEs — fly down the corridor, weave side-to-side, fire ─────────
  float tieSpeed = trenchSpeed * 0.55f;
  for(int ti = 0; ti < NUM_TRENCH_TIES; ti++) {
    TrenchTIE &tie = trenchTIEs[ti];

    // Spawn when timer fires and slot is free
    if(!tie.active && millis() >= tie.nextSpawn) {
      tie.z        = (float)TR_FAR;
      tie.x        = 64 + random(-25, 26);
      tie.y        = 0.15f + random(70) / 100.0f;
      tie.vx       = (random(2) ? 1.0f : -1.0f);
      tie.vy       = (random(2) ? 0.003f : -0.003f);
      tie.health   = 1;
      tie.lastFire = 0;
      tie.active   = true;
    }
    if(!tie.active) continue;

    // Fly toward player
    tie.z -= tieSpeed;

    // Weave side-to-side
    tie.x += tie.vx;
    if(tie.x < 15 || tie.x > 113) tie.vx = -tie.vx;
    // Vertical drift
    tie.y += tie.vy;
    if(tie.y < 0.05f || tie.y > 0.92f) tie.vy = -tie.vy;

    // Project to screen
    int scx, scy;
    trenchTIEProject(tie, scx, scy);

    // Avoidance: steer away from player when close
    if(tie.z < 140.0f) {
      float hdist = scx - playerX;
      if(fabsf(hdist) < 22) tie.vx += (hdist < 0) ? -0.3f : 0.3f;
      tie.vx = constrain(tie.vx, -2.5f, 2.5f);
      float vdist = scy - playerY;
      if(fabsf(vdist) < 18) tie.vy += (vdist < 0) ? -0.006f : 0.006f;
      tie.vy = constrain(tie.vy, -0.025f, 0.025f);
    }

    // Fire at player when in mid-range
    if(tie.z < 200.0f && millis() - tie.lastFire > 1000) {
      float dx = playerX - scx, dy = playerY - scy;
      float dist = sqrt(dx*dx + dy*dy);
      if(dist > 0.5f) {
        float spd = 2.8f;
        fireProjectile(scx-2, scy, (dx/dist)*spd-0.15f, (dy/dist)*spd, false);
        fireProjectile(scx+2, scy, (dx/dist)*spd+0.15f, (dy/dist)*spd, false);
        playSFX(SFX_TIE_FLYBY);
      }
      tie.lastFire = millis();
    }

    // Reached player — damage and despawn
    if(tie.z < TR_NEAR) {
      tie.active    = false;
      tie.nextSpawn = millis() + random(4000, 7000);
      if(millis() - lastDamageTime > INVINCIBILITY_TIME) {
        shields -= 15;
        lastDamageTime = millis();
        damageFlash    = millis() + 400;
        playSFX(SFX_HIT_PLAYER);
        if(shields <= 0) currentState = GAME_OVER;
      }
    }
  }
}

void initializeSurfaceTowers() {
  for(int i = 0; i < 4; i++) {
    surfaceTowers[i].x = random(20, 108);
    surfaceTowers[i].y = 50; // Fixed surface level to match drawDeathStarSurface()
    surfaceTowers[i].z = random(50, 150);
    surfaceTowers[i].health = 2; // Change from 1 to 2
    surfaceTowers[i].active = true;
    surfaceTowers[i].type = random(0, 2);
    surfaceTowers[i].lastHit = 0;
        }
       for(int i = 0; i < 6; i++) {
    backgroundShips[i].x = random(0, 1) ? 10 : 118;  // Start from left or right edge
    backgroundShips[i].y = random(12, 30);  // Higher on screen (was 15-40)
    backgroundShips[i].z = random(100, 180);
    // Set velocity based on starting position - go toward opposite side
    backgroundShips[i].vx = (backgroundShips[i].x < 64) ? random(5, 15) / 10.0 : random(-15, -5) / 10.0;
    backgroundShips[i].vy = 0;
    backgroundShips[i].vz = 0;
    backgroundShips[i].type = random(0, 2);
    backgroundShips[i].active = true;
    backgroundShips[i].lastShot = millis() + random(2000, 5000);
  }
      }

void updateSurfaceTowers() {
  // Update banking angle smoothly
if(currentState == DEATH_STAR_SURFACE) {
  bankingAngle += (targetBankingAngle - bankingAngle) * 0.25;
  
  // Decay banking angle when not moving
  targetBankingAngle *= 0.95;
  
  // Safety: prevent NaN
  if(isnan(bankingAngle)) bankingAngle = 0;
  if(isnan(targetBankingAngle)) targetBankingAngle = 0;
}

// Update background ships
  for(int i = 0; i < 6; i++) {
    if(backgroundShips[i].active) {
      // Safety: validate velocity
      if(isnan(backgroundShips[i].vx) || isinf(backgroundShips[i].vx)) {
        backgroundShips[i].vx = 1.0;
      }
      
      backgroundShips[i].x += backgroundShips[i].vx;
      
      // Safety: validate position
      if(isnan(backgroundShips[i].x) || backgroundShips[i].x < -50 || backgroundShips[i].x > 178) {
        backgroundShips[i].x = random(0, 1) ? 10 : 118;
      }
      // No Y movement - ships fly horizontally only
      // No Z movement - ships stay at constant depth
      
      // Wrap around screen horizontally
      if(backgroundShips[i].x < 10) backgroundShips[i].x = 118;
      if(backgroundShips[i].x > 118) backgroundShips[i].x = 10;
    }
  }
  // Progressive speed increase based on score (starts at 2.0, reaches 3.0 at score 500)
  surfaceSpeed = 2.0 + (score / 500.0) * 1.0;
  if(surfaceSpeed > 3.0) surfaceSpeed = 3.0;
  
  // Safety: validate surface speed
  if(isnan(surfaceSpeed) || surfaceSpeed < 0 || surfaceSpeed > 10) {
    surfaceSpeed = 2.0;
  }
  
  // Move existing towers
  for(int i = 0; i < 4; i++) {
    if(surfaceTowers[i].active) {
      surfaceTowers[i].z -= surfaceSpeed;
      
      // Safety: validate tower position
      if(isnan(surfaceTowers[i].z) || surfaceTowers[i].z < -100 || surfaceTowers[i].z > 200) {
        surfaceTowers[i].active = false;
        continue;
      }
      
      // Check collision and inflict damage when towers get close (z <= 2)
      if(surfaceTowers[i].z <= 2) {
        shields -= 10; // 10% damage per tower
        score -= 10;  // Deduct 10 points when hit
        if(score < 0) score = 0;  // Don't go negative
        playSFX(SFX_HIT_PLAYER);  // Shield impact
        damageFlash = millis() + 400;  // Screen flash effect (same as space battle)
        surfaceTowers[i].active = false; // Destroy tower after collision
        createDebrisExplosion(surfaceTowers[i].x, surfaceTowers[i].y);
        
        if(shields <= 0) {
          currentState = GAME_OVER;
        }
      }
      
      // Remove towers that are too close (cleanup)
      if(surfaceTowers[i].z <= 1) {
        surfaceTowers[i].active = false;
      }
    }
  }
  
  // Spawn new towers - frequency increases more aggressively
  // From 2.5 seconds down to 1.2 seconds minimum
  int spawnDelay = max(1200, 2500 - (score * 3));
  
  // Count active towers before spawning
  int activeTowerCount = 0;
  for(int i = 0; i < 4; i++) {
    if(surfaceTowers[i].active) activeTowerCount++;
  }
  
  // Only spawn if we have less than 3 active towers
  if(millis() - lastRespawn > spawnDelay && activeTowerCount < 3) {
    // Find first inactive tower slot
    for(int i = 0; i < 4; i++) {
      if(!surfaceTowers[i].active) {
        surfaceTowers[i].x = random(20, 108);
        surfaceTowers[i].y = 50;
        surfaceTowers[i].z = random(50, 150);
        surfaceTowers[i].active = true;
        surfaceTowers[i].health = 2;
        surfaceTowers[i].type = random(0, 2);
        surfaceTowers[i].lastHit = 0;
        break; // Only spawn one at a time
      }
    }
    lastRespawn = millis();
  }
}

void drawDeathStarSurface() {
  // Calculate banking effects
  float bankRadians = radians(bankingAngle);
  float bankSin = sin(bankRadians);
  
  // Base horizon with banking tilt
  int baseHorizon = 45;
  
  // Draw tilted horizon line (main Death Star surface line)
  int horizonLeft = baseHorizon - (bankSin * 10);
  int horizonRight = baseHorizon + (bankSin * 10);
  sw_display.drawLine(0, horizonLeft, 128, horizonRight, GREY);
  for(int i = 0; i < 6; i++) {
    if(backgroundShips[i].active) {
      float scale = 100.0 / (backgroundShips[i].z + 50);
      
      if(scale > 0.1 && scale < 0.5) {
        int shipX = backgroundShips[i].x;
        int shipY = backgroundShips[i].y;
        int shipSize = max(1, (int)(3 * scale));
        
        if(backgroundShips[i].type == 0) {
          // X-wing - white/cyan
          sw_display.drawLine(shipX - shipSize, shipY, shipX + shipSize, shipY, CYAN);
          sw_display.drawLine(shipX, shipY - shipSize, shipX, shipY + shipSize, CYAN);
        } else {
          // TIE - grey
          sw_display.drawPixel(shipX - shipSize, shipY, GREY);
          sw_display.drawPixel(shipX + shipSize, shipY, GREY);
          sw_display.drawLine(shipX - shipSize/2, shipY, shipX + shipSize/2, shipY, GREY);
        }
        
        // Laser flash
        if((millis() + i * 100) % 600 < 50) {
          sw_display.drawLine(shipX, shipY, shipX + random(-3, 3), shipY + random(-2, 2), GREEN_LASER);
        }
      }
    }
  }
}

void drawSurfaceTowers() {
  for(int i = 0; i < 4; i++) {
    if(surfaceTowers[i].active && surfaceTowers[i].z > 0) {
      float scale = 100.0 / (surfaceTowers[i].z + 20);
      
      if(scale > 0.3) {
        int towerHeight = max(2, (int)(8 * scale));
        int towerWidth = max(1, (int)(3 * scale));
        
        // Apply banking to tower position
        int baseHorizon = 45;
        float bankRadians = radians(bankingAngle);
        float bankSin = sin(bankRadians);
        
        int towerX = surfaceTowers[i].x;
        int horizonY = baseHorizon;
        
        // Banking affects tower position
        int bankingOffset = bankSin * (horizonY - baseHorizon) * 0.3;
        int bankShiftX = bankSin * (horizonY - baseHorizon) * 0.2;
        
        towerX += bankShiftX;
        horizonY += bankingOffset;
        
        // Only draw if tower is on screen
        if(towerX >= 0 && towerX < 128) {
          // Draw tower with banking tilt
          int tiltOffset = bankSin * towerHeight * 0.2;
          
          sw_display.drawRect(towerX - towerWidth/2, horizonY - towerHeight, 
                          towerWidth, towerHeight, GREY);
          
          // Base line with tilt
          sw_display.drawLine(towerX - towerWidth/2, horizonY, 
                          towerX + towerWidth/2 + tiltOffset, horizonY, GREY);
          
          // Internal details with banking consideration
          if(scale > 0.6) {
            sw_display.drawLine(towerX, horizonY - towerHeight, 
                           towerX + tiltOffset, horizonY, DARK_GREY);
            
            if(towerHeight > 6) {
              int detail1Y = horizonY - towerHeight/3;
              int detail2Y = horizonY - 2*towerHeight/3;
              int detail1Tilt = tiltOffset * 0.33;
              int detail2Tilt = tiltOffset * 0.67;
              
              sw_display.drawLine(towerX - towerWidth/2, detail1Y, 
                             towerX + towerWidth/2 + detail1Tilt, detail1Y, GREY);
              sw_display.drawLine(towerX - towerWidth/2, detail2Y, 
                             towerX + towerWidth/2 + detail2Tilt, detail2Y, GREY);
            }
          }
          
          // Tower type indicators (unchanged)
          bool showDamage = (surfaceTowers[i].health == 1);
          bool isDamageFlashing = (millis() - surfaceTowers[i].lastHit < 500 && (millis() / 100) % 2);
          
          if(!showDamage || !isDamageFlashing) {
            switch(surfaceTowers[i].type % 2) {
              case 0: // Communications tower
                if(towerHeight > 4) {
                  sw_display.drawLine(towerX, horizonY - towerHeight, 
                                 towerX + tiltOffset, horizonY - towerHeight - 3, RED);
                  if(scale > 0.7 && !showDamage) {
                    sw_display.drawLine(towerX - 2, horizonY - towerHeight - 1, 
                                   towerX + 2 + tiltOffset, horizonY - towerHeight - 1, ORANGE);
                  }
                }
                break;
                
              case 1: // Gun turret
                if(towerHeight > 3 && towerWidth > 1) {
                  sw_display.drawLine(towerX + towerWidth/2, horizonY - towerHeight/2, 
                                 towerX + towerWidth/2 + 3 + tiltOffset, horizonY - towerHeight/2, ORANGE);
                  if(scale > 0.7 && !showDamage) {
                    sw_display.drawRect(towerX - 1, horizonY - towerHeight + 1, 3, 2, RED);
                  }
                }
                break;
            }
          }
          
          // Damage sparks
          if(millis() - surfaceTowers[i].lastHit < 300) {
            sw_display.drawPixel(towerX + random(-towerWidth, towerWidth + 1), 
                            horizonY - random(0, towerHeight), ORANGE);
          }
        }
      }
    }
  }
}

// ── Surface TIE fighters: dive from space, fire once, player can shoot them ──

void initializeSurfaceTIEs() {
  for(int i = 0; i < NUM_SURFACE_TIES; i++) {
    surfaceTIEs[i].active     = false;
    surfaceTIEs[i].retreating = false;
  }
  // First TIE arrives 8-12 s into the stage; subsequent ones every 12-18 s.
  // That delivers 4-5 attacks comfortably inside the ~90 s stage window.
  nextTIESpawn    = millis() + 8000 + random(4000);
  tieWavesSpawned = 0;
}

void updateSurfaceTIEs() {
  // ── Timer-based spawning ─────────────────────────────────────────────────────
  // Spawns 4-5 TIE attacks across the ~90 s stage by scheduling each wave with a
  // fixed timer rather than per-frame random chance (which was wildly inconsistent).
  //
  // Wave cadence:
  //   Wave 1:  8-12 s  (set in initializeSurfaceTIEs)
  //   Wave 2+: 12-18 s after the previous wave clears (all active slots free)
  //            Maximum 5 waves so the last one still fires before the stage ends.

  int activeCount = 0;
  for(int i = 0; i < NUM_SURFACE_TIES; i++) if(surfaceTIEs[i].active) activeCount++;

  if(activeCount == 0 && tieWavesSpawned < 7 && millis() >= nextTIESpawn) {
    // Decide wave size: single TIE for waves 1-2, pair for waves 3-5
    int waveSize = (tieWavesSpawned < 2) ? 1 : 2;
    int spawned  = 0;

    for(int i = 0; i < NUM_SURFACE_TIES && spawned < waveSize; i++) {
      if(!surfaceTIEs[i].active) {
        // Spread pair left/right so they come from different angles
        float spawnX;
        if(waveSize == 2) {
          spawnX = (spawned == 0) ? random(20, 55) : random(73, 108);
        } else {
          spawnX = random(30, 98);
        }

        surfaceTIEs[i].x          = spawnX;
        surfaceTIEs[i].y          = 2;
        // Drift toward crosshair so the TIE homes in visibly
        surfaceTIEs[i].vx         = (crosshairX - spawnX) * 0.018f;
        // Dive speed: takes ~1.6 s to travel from y=2 to y=28 at 20 fps
        surfaceTIEs[i].vy         = 0.85f + random(20) / 100.0f;
        surfaceTIEs[i].active     = true;
        surfaceTIEs[i].hasFired   = false;
        surfaceTIEs[i].retreating = false;
        surfaceTIEs[i].health     = 1;
        surfaceTIEs[i].spawnTime  = millis();
        spawned++;
      }
    }

    tieWavesSpawned++;
    // Schedule next wave: 12-18 s from now
    nextTIESpawn = millis() + 12000 + random(6000);
  }

  // ── Per-TIE movement & behaviour ─────────────────────────────────────────────
  for(int i = 0; i < NUM_SURFACE_TIES; i++) {
    if(!surfaceTIEs[i].active) continue;

    if(surfaceTIEs[i].retreating) {
      // Sweep back up — accelerates slightly so the exit feels dynamic
      surfaceTIEs[i].vy  = -1.2f;
      surfaceTIEs[i].y  += surfaceTIEs[i].vy;
      // Keep drifting horizontally so paired TIEs peel away in different directions
      surfaceTIEs[i].x  += surfaceTIEs[i].vx;
      surfaceTIEs[i].vx *= 0.96f;                   // gently straighten out
      // Clamp x
      if(surfaceTIEs[i].x < 10)  surfaceTIEs[i].x = 10;
      if(surfaceTIEs[i].x > 118) surfaceTIEs[i].x = 118;
      // Despawn once fully above frame
      if(surfaceTIEs[i].y < -8) surfaceTIEs[i].active = false;

    } else {
      // Diving toward the player — steer continuously toward crosshair
      float steer = (crosshairX - surfaceTIEs[i].x) * 0.012f;
      surfaceTIEs[i].vx = surfaceTIEs[i].vx * 0.85f + steer * 0.15f;
      surfaceTIEs[i].x += surfaceTIEs[i].vx;
      surfaceTIEs[i].y += surfaceTIEs[i].vy;

      // Clamp x to safe draw range
      if(surfaceTIEs[i].x < 15)  surfaceTIEs[i].x = 15;
      if(surfaceTIEs[i].x > 113) surfaceTIEs[i].x = 113;

      // Fire once at y=28 — large enough on screen to be readable, still in sky
      if(!surfaceTIEs[i].hasFired && surfaceTIEs[i].y >= 28) {
        surfaceTIEs[i].hasFired = true;
        float dx   = crosshairX - surfaceTIEs[i].x;
        float dy   = crosshairY - surfaceTIEs[i].y;
        float dist = sqrt(dx*dx + dy*dy);
        if(dist > 0.1f) {
          float speed = 2.5f;
          fireProjectile(surfaceTIEs[i].x, surfaceTIEs[i].y,
                         (dx/dist)*speed, (dy/dist)*speed, false);
          playSFX(SFX_TIE_FLYBY);
        }
        // Immediately begin retreat; reverse horizontal drift for peel-away look
        surfaceTIEs[i].retreating = true;
        surfaceTIEs[i].vy         = 0;
        surfaceTIEs[i].vx         = -surfaceTIEs[i].vx * 1.5f;  // bank away
      }

      // Safety despawn if it somehow reaches the horizon unfired
      if(surfaceTIEs[i].y > 42) surfaceTIEs[i].active = false;
    }
  }
}

void drawSurfaceTIEs() {
  for(int i = 0; i < NUM_SURFACE_TIES; i++) {
    if(!surfaceTIEs[i].active) continue;

    int x = (int)surfaceTIEs[i].x;
    int y = (int)surfaceTIEs[i].y;

    // Only draw while inside the sky zone (above the horizon)
    if(y < 0 || y > 43) continue;

    // Scale smoothly from 1 (tiny speck at top) to 9 (large threat near horizon)
    // progress 0 = y at top (2), 1 = y at horizon (42)
    float progress = constrain((surfaceTIEs[i].y - 2.0f) / (42.0f - 2.0f), 0.0f, 1.0f);
    float scale = 1.5f + progress * 7.5f;   // 1.5 .. 9.0

    // When retreating after firing, reverse the scale so it shrinks away
    if(surfaceTIEs[i].retreating) {
      // Recompute progress from y going back up: shrink as y drops back toward 0
      progress = constrain((surfaceTIEs[i].y - 2.0f) / (42.0f - 2.0f), 0.0f, 1.0f);
      scale = 1.5f + progress * 7.5f;  // naturally shrinks as y decreases
    }

    // Safety margin: only clip if the panels would be completely off-screen
    // (old code used 2.5*scale+2 which culled valid centre positions)
    int panelHalfSpan = (int)(scale * 2.5f);
    if(x - panelHalfSpan < 0 || x + panelHalfSpan > 128) continue;

    drawVectorTIE(x, y, scale, 0);
  }
}

void handleSurfaceTIEHit() {
  // Called from handleFireButton when in DEATH_STAR_SURFACE
  for(int i = 0; i < NUM_SURFACE_TIES; i++) {
    if(!surfaceTIEs[i].active) continue;
    float dist = sqrt(pow(crosshairX - surfaceTIEs[i].x, 2) +
                      pow(crosshairY - surfaceTIEs[i].y, 2));
    if(dist < 12) {
      surfaceTIEs[i].active = false;
      score += 30;
      createExplosion(surfaceTIEs[i].x, surfaceTIEs[i].y);
      playSFX(SFX_EXPLOSION);
      break;
    }
  }
}

// Helper: draw the Death Star at cx,cy with radius r (reuses approach code)
static void drawDeathStarAt(int cx, int cy, int r) {
  if(r < 2) return;
  sw_display.fillCircle(cx, cy, r, GREY);
  // Lit hemisphere
  if(r > 2) {
    for(int offX = -r; offX < 0; offX++) {
      int chord = (int)sqrt(max(0.0f,(float)(r*r - offX*offX)));
      uint16_t col = (offX > -r/2) ? 0xFFFF : 0x8410;
      sw_display.drawLine(cx+offX, cy-chord, cx+offX, cy+chord, col);
    }
  }
  // Shadow
  if(r > 3) {
    for(int offX = r/3; offX <= r; offX++) {
      int chord = (int)sqrt(max(0.0f,(float)(r*r - offX*offX)));
      sw_display.drawLine(cx+offX, cy-chord, cx+offX, cy+chord, 0x4208);
    }
  }
  // Equatorial trench
  if(r > 6) {
    int tw = max(1, r/20);
    for(int ty2 = cy-tw; ty2 <= cy+tw; ty2++) {
      int dx2 = (int)sqrt(max(0.0f,(float)(r*r-(ty2-cy)*(ty2-cy))));
      sw_display.drawLine(cx-dx2, ty2, cx+dx2, ty2, 0x4208);
    }
    if(r > 14) {
      int dx1 = (int)sqrt(max(0.0f,(float)(r*r-(float)(tw+1)*(tw+1))));
      sw_display.drawLine(cx-dx1, cy-tw-1, cx+dx1, cy-tw-1, 0xFFFF);
      sw_display.drawLine(cx-dx1, cy+tw+1, cx+dx1, cy+tw+1, 0xFFFF);
    }
  }
  // Latitude lines
  if(r > 16) {
    int sp = max(4, r/4);
    for(int latY = cy-sp; latY > cy-r+2; latY -= sp) {
      int dx2 = (int)sqrt(max(0.0f,(float)(r*r-(latY-cy)*(latY-cy))));
      if(dx2 > 2) sw_display.drawLine(cx-dx2+1, latY, cx+dx2-1, latY, 0x8410);
    }
    for(int latY = cy+sp; latY < cy+r-2; latY += sp) {
      int dx2 = (int)sqrt(max(0.0f,(float)(r*r-(latY-cy)*(latY-cy))));
      if(dx2 > 2) sw_display.drawLine(cx-dx2+1, latY, cx+dx2-1, latY, 0x8410);
    }
  }
  // Superlaser dish
  if(r > 8) {
    int dR = max(2, r/4);
    int dX = cx - (int)(r*0.38f);
    int dY = cy - (int)(r*0.35f);
    sw_display.fillCircle(dX, dY, dR, 0x4208);
    sw_display.drawCircle(dX, dY, dR, 0x8410);
    sw_display.drawCircle(dX, dY, dR+1, 0xFFFF);
    sw_display.fillCircle(dX, dY, max(1, dR/3), 0xFFFF);
    if(r > 18) {
      for(int b = 0; b < 8; b++) {
        float ang2 = b * 3.14159f / 4.0f;
        sw_display.drawLine(dX, dY,
          dX+(int)(cos(ang2)*dR), dY+(int)(sin(ang2)*dR), 0x8410);
      }
    }
  }
  sw_display.drawCircle(cx, cy, r, 0x8410);
  if(r > 4) sw_display.drawCircle(cx, cy, r+1, 0x4208);
}

void drawDeathStarExplosion() {
  unsigned long e = millis() - explosionStartTime;
  const int CX = 64, CY = 30, R = 28;

  // Static starfield backdrop
  const uint8_t SX[] = {5,18,35,52,70,88,103,118,12,44,75,110};
  const uint8_t SY[] = {4,14, 6,20, 3, 15,  8, 11,22, 9,18,  5};
  for(int s = 0; s < 12; s++)
    sw_display.drawPixel(SX[s], SY[s], WHITE);

  // ── Phase 0 (0–1000ms): Death Star static, full view ──────────────────
  if(e < 1000) {
    drawDeathStarAt(CX, CY, R);
    return;
  }

  // ── Phase 1 (1000–1600ms): Dish glows — explosion building ────────────
  if(e < 1600) {
    drawDeathStarAt(CX, CY, R);
    float p = (e - 1000) / 600.0f;
    // Dish pulses bright — superlaser detonating
    int dR = max(2, R/4);
    int dX = CX - (int)(R*0.38f);
    int dY = CY - (int)(R*0.35f);
    int glowR = dR + (int)(p * dR * 2);
    sw_display.drawCircle(dX, dY, glowR,   YELLOW);
    sw_display.drawCircle(dX, dY, glowR+2, ORANGE);
    if((millis()/60)%2==0)
      sw_display.fillCircle(dX, dY, glowR-1, YELLOW);
    // Internal cracks emanating from dish
    if(p > 0.4f) {
      for(int c = 0; c < 5; c++) {
        float ang = radians(c * 36 + (int)(p*40));
        int len = (int)(p * 18);
        sw_display.drawLine(dX, dY,
          dX+(int)(cos(ang)*len), dY+(int)(sin(ang)*len),
          (c%2==0) ? YELLOW : ORANGE);
      }
    }
    return;
  }

  // ── Phase 2 (1600–2000ms): BLINDING FLASH ─────────────────────────────
  if(e < 2000) {
    float p = (e - 1600) / 400.0f;
    // Flash starts white, fades to yellow
    if(p < 0.5f) {
      // Full white fill — blindingly bright
      sw_display.fillRect(0, 0, 128, 64, WHITE);
      // DS silhouette just barely visible through the flash
      sw_display.drawCircle(CX, CY, R, 0x8410);
    } else {
      // Flash fading — yellow bloom
      sw_display.fillRect(0, 0, 128, 64, BLACK);
      int bloom = (int)((1.0f - (p-0.5f)*2.0f) * 40);
      sw_display.fillCircle(CX, CY, bloom, YELLOW);
      sw_display.fillCircle(CX, CY, bloom/2, WHITE);
    }
    return;
  }

  // ── Phase 3 (2000–3500ms): DS cracks apart ────────────────────────────
  if(e < 3500) {
    float p = (e - 2000) / 1500.0f;  // 0→1

    // Background — fade to deep orange haze
    sw_display.fillRect(0, 0, 128, 64, BLACK);

    // Core fireball shrinking
    int coreR = (int)(20 * (1.0f - p * 0.6f));
    if(coreR > 0) {
      sw_display.fillCircle(CX, CY, coreR, ORANGE);
      sw_display.fillCircle(CX, CY, coreR/2, YELLOW);
      sw_display.fillCircle(CX, CY, coreR/4, WHITE);
    }

    // 6 large arc fragments flying outward
    // Each fragment is a partial arc of radius R, drifting away from centre
    for(int f = 0; f < 6; f++) {
      float baseAng = f * 60.0f;          // 60° apart
      float drift   = p * (18 + f * 5);  // drift distance
      float tumble  = p * (20 + f * 8);  // tumble angle
      // Fragment centre drifts outward at its angle
      int fx = CX + (int)(cos(radians(baseAng + tumble)) * drift);
      int fy = CY + (int)(sin(radians(baseAng + tumble)) * drift * 0.8f);
      // Draw as a short arc (8 points spanning 50°)
      int fragR = max(3, (int)(R * (1.0f - p * 0.4f)));
      uint16_t fragCol = (f%3==0) ? GREY : (f%3==1) ? DARK_GREY : 0x8410;
      for(int a = 0; a < 8; a++) {
        float aa = radians(baseAng + tumble + a * 6 - 20);
        int ax = fx + (int)(cos(aa) * fragR);
        int ay = fy + (int)(sin(aa) * fragR);
        if(ax>=0 && ax<128 && ay>=0 && ay<64)
          sw_display.drawPixel(ax, ay, fragCol);
      }
      // Fragment inner fill (couple of pixels)
      if(fragR > 6) {
        int ax2 = fx + (int)(cos(radians(baseAng+tumble)) * (fragR-3));
        int ay2 = fy + (int)(sin(radians(baseAng+tumble)) * (fragR-3));
        if(ax2>=0&&ax2<128&&ay2>=0&&ay2<64)
          sw_display.drawPixel(ax2, ay2, DARK_GREY);
      }
    }

    // First shockwave ring expands outward
    int ringR = (int)(p * 55);
    if(ringR > 0 && ringR < 64) {
      sw_display.drawCircle(CX, CY, ringR,   ORANGE);
      if(ringR > 8)
        sw_display.drawCircle(CX, CY, ringR-4, YELLOW);
    }
    // Second ring, delayed
    if(p > 0.3f) {
      int ring2 = (int)((p-0.3f) * 70);
      if(ring2 > 0 && ring2 < 80)
        sw_display.drawCircle(CX, CY, ring2, RED);
    }
    return;
  }

  // ── Phase 4 (3500–6000ms): Total destruction — debris field ───────────
  {
    float p = (e - 3500) / 2500.0f;  // 0→1
    sw_display.fillRect(0, 0, 128, 64, BLACK);

    // Fading debris cloud — 40 particles flying outward
    for(int d = 0; d < 40; d++) {
      float ang  = d * 9.0f + (e / 30.0f);   // slowly rotate
      float dist = p * (20 + (d % 8) * 10);   // spread out
      int dx2 = CX + (int)(cos(radians(ang)) * dist);
      int dy2 = CY + (int)(sin(radians(ang)) * dist * 0.75f);
      if(dx2 < 0 || dx2 >= 128 || dy2 < 0 || dy2 >= 64) continue;

      // Colour fades from orange to dark as explosion cools
      uint16_t dcol;
      if(p < 0.3f)      dcol = (d%3==0) ? ORANGE : (d%3==1) ? YELLOW : RED;
      else if(p < 0.6f) dcol = (d%2==0) ? ORANGE : RED;
      else               dcol = (d%3==0) ? RED : DARK_ORANGE;

      sw_display.drawPixel(dx2, dy2, dcol);
      // Trail
      int tx2 = CX + (int)(cos(radians(ang)) * (dist * 0.85f));
      int ty2 = CY + (int)(sin(radians(ang)) * (dist * 0.75f * 0.85f));
      if(tx2>=0&&tx2<128&&ty2>=0&&ty2<64)
        sw_display.drawPixel(tx2, ty2, DARK_ORANGE);
    }

    // Fading rings
    if(p < 0.5f) {
      int r1 = (int)(55 + p * 30);
      int r2 = (int)(30 + p * 40);
      if(r1<80) sw_display.drawCircle(CX, CY, r1, (p<0.25f)?ORANGE:DARK_ORANGE);
      if(r2<80) sw_display.drawCircle(CX, CY, r2, (p<0.3f)?RED:DARK_ORANGE);
    }


  }
}
    
// ─────────────────────────────────────────────────────────────────────────────
// TRENCH RUN — first-person cockpit perspective
//
// Coordinate system:
//   World X: 0 = trench centre, ±TR_HW = wall edges at near plane
//   World Y: 0 = top of trench (sky), +ve = floor (positive = down in screen)
//            floor is at worldY = TR_HW (square cross-section trench)
//   Z:       TR_FAR (far) → TR_NEAR (player position)
//
// Projection: screenX = VPX + (worldX/z)*FOCAL
//             screenY = VPY + (worldY/z)*FOCAL
// ─────────────────────────────────────────────────────────────────────────────

// Draw speed-lines radiating from vanishing point
// ─────────────────────────────────────────────────────────────────────────────
//  TRENCH DRAW — clean first-person canyon geometry
//
//  Vanishing point: (VP_X=64, VP_Y=45) — low on screen so it feels like
//  you are racing along the floor of a deep canyon, not floating above it.
//
//  Near-plane corners (what you see "around" the ship):
//    Top-left  : (0,  0)   Top-right  : (127, 0)
//    Floor-left: (0, 56)   Floor-right: (127, 56)
//
//  Four rails converge from those four corners to the VP:
//    Rail TL: (0,0)   → (64,45)
//    Rail TR: (127,0) → (64,45)
//    Rail FL: (0,56)  → (64,45)
//    Rail FR: (127,56)→ (64,45)
//
//  Scrolling cross-lines at each segment's projected depth give the "rushing
//  forward" feeling.  Turrets sit ON the wall edges (top-left / top-right
//  rail) and grow as they approach.
// ─────────────────────────────────────────────────────────────────────────────



void drawTrenchSpeedLines() {
  // removed — was drawing through screen centre causing X flicker
}

void drawVaderTIE() {
  for(int ti = 0; ti < NUM_TRENCH_TIES; ti++) {
    const TrenchTIE &tie = trenchTIEs[ti];
    if(!tie.active) continue;
    int scx, scy;
    float scale = trenchTIEProject(tie, scx, scy);
    if(scale < 0.5f) continue;   // too far to see — skip

    // Draw using the same drawVectorTIE used in space battle
    drawVectorTIE(scx, scy, scale, 0);

    // Green laser flash just after firing
    if(millis() - tie.lastFire < 180 && scale > 1.5f) {
      int pOff = max(1,(int)(scale*0.6f)) + max(2,(int)(scale/2)) + 2;
      sw_display.drawLine(scx-pOff, scy, playerX, playerY, GREEN_LASER);
      sw_display.drawLine(scx+pOff, scy, playerX, playerY, GREEN_LASER);
    }
  }
}

void drawTrenchHUD() {
  // Speed bar on left edge
  int speedH = (int)((trenchSpeed / 7.0f) * 24);
  sw_display.drawLine(2, 52 - speedH, 2, 52, CYAN);
  sw_display.drawPixel(2, 52 - speedH, WHITE);

  // Stage progress bar just below score (y=10..11)
  {
    int prog  = score - stageStartScore;
    int barW  = map(constrain(prog, 0, GATE_TRENCH), 0, GATE_TRENCH, 0, 108);
    uint16_t barCol = (prog < GATE_TRENCH/2) ? CYAN : (prog < GATE_TRENCH*3/4) ? YELLOW : GREEN;
    sw_display.drawLine(10, 10, 118, 10, DARK_GREY);
    sw_display.drawLine(10, 11, 118, 11, DARK_GREY);
    if(barW > 0) {
      sw_display.drawLine(10, 10, 10 + barW, 10, barCol);
      sw_display.drawLine(10, 11, 10 + barW, 11, barCol);
    }
  }

  // Lock warning — small red dot on speed bar instead of text
  for(int i = 0; i < 6; i++) {
    if(trenchSegments[i].hasObstacle && trenchSegments[i].turretLocked) {
      if((millis()/200)%2==0) sw_display.drawPixel(2, 52 - speedH - 2, RED);
      break;
    }
  }
}

void drawTrench() {
  // ══════════════════════════════════════════════════════════════
  //  Death Star Trench — simple wireframe corridor
  //
  //  Near plane (screen edges):
  //    Top-left  (0,0)    Top-right  (127,0)
  //    Bot-left  (0,63)   Bot-right  (127,63)
  //
  //  Far rectangle (the opening in the distance) — wide so it
  //  feels like a big corridor, not a pinhole:
  //    Top-left  (FL,FT)  Top-right  (FR,FT)
  //    Bot-left  (FL,FB)  Bot-right  (FR,FB)
  //
  //  8 lines total connect near corners to far corners:
  //    Left wall  top/bottom diagonals  (near TL→far TL, near BL→far BL)
  //    Right wall top/bottom diagonals  (near TR→far TR, near BR→far BR)
  //    Far rectangle 4 sides
  //
  //  A few evenly-spaced intermediate cross-lines on each wall
  //  scroll toward you using segment z-depths.
  // ══════════════════════════════════════════════════════════════

  // Far-end opening corners
  const int FL  = 44;   // far bottom-left  X
  const int FR  = 83;   // far bottom-right X
  const int FTL = 28;   // far top-left     X (much wider = steeper slope)
  const int FTR = 99;   // far top-right    X (much wider = steeper slope)
  const int FT  = 22;   // far top   Y
  const int FB  = 56;   // far bot   Y

  // ── 1. Black background ──────────────────────────────────────
  sw_display.clearDisplay();

  // A few static stars in the upper open area
  const uint8_t SX[]={10,30,55,75,98,115,20,62,88,110, 5,42,80,108};
  const uint8_t SY[]={ 2, 5, 2, 4,  3,  6, 7, 3,  5,  2,12,15, 9,  17};
  for(int s=0;s<10;s++) sw_display.drawPixel(SX[s],SY[s],WHITE);

  // ── 2. The 4 structural rails (near corner → far corner) ─────
  // Top corners at screen edge, bottom corners inset — walls slope outward
  const int NTY = 21;   // near top Y — walls start 1/3 down screen
  const int NBL = 24;   // near bot-left X (inset)
  const int NBR = 103;  // near bot-right X (inset)
  sw_display.drawLine(  0, NTY, FTL, FT, GREY);  // top-left rail
  sw_display.drawLine(NBL,  63, FL,  FB, GREY);   // bot-left rail
  sw_display.drawLine(127, NTY, FTR, FT, GREY);  // top-right rail
  sw_display.drawLine(NBR,  63, FR,  FB, GREY);   // bot-right rail

  // ── 3. Far opening — no top line, sides slope to match walls ──
  sw_display.drawLine(FL,  FB, FR,  FB, GREY);          // far bottom
  sw_display.drawLine(FTL, FT, FL,  FB, GREY);          // far left  (slopes in)
  sw_display.drawLine(FTR, FT, FR,  FB, GREY);          // far right (slopes in)

  // ── 4. Floor — two extra lines from bottom rail to far bottom ─
  // Adds a bit of floor depth without cluttering
  sw_display.drawLine(  0, 63, FL, FB, GREY);   // already drawn (bot-left)
  sw_display.drawLine(127, 63, FR, FB, GREY);   // already drawn (bot-right)
  // Floor centre-line converging to far-bottom-centre


  // ── 5. Scrolling cross-sections at each segment depth ────────
  // Sort far→near so closer ones draw on top
  int order[6]={0,1,2,3,4,5};
  for(int a=0;a<5;a++)
    for(int b=a+1;b<6;b++)
      if(trenchSegments[order[a]].z < trenchSegments[order[b]].z){
        int tmp=order[a]; order[a]=order[b]; order[b]=tmp;
      }

  for(int oi=0;oi<6;oi++){
    int   i = order[oi];
    float z = trenchSegments[i].z;
    if(z <= TR_NEAR || z > TR_FAR) continue;

    // t=0 → near plane (screen edge), t=1 → far rectangle
    float t = zToT(z);

    // Top uses screen edges, bottom uses inset — same slope as structural rails
    int lx_top = (int)(0   + t*(28 - 0  ));
    int rx_top = (int)(127 + t*(99 - 127));
    int lx_bot = (int)(24  + t*(44 - 24 ));
    int rx_bot = (int)(103 + t*(83 - 103));
    int ty = (int)(21  + t*(FT - 21));
    int by = (int)(63  + t*(FB - 63));

    // Wall cross-line — top wide, bottom inset (slopes outward like the rails)
    sw_display.drawLine(lx_top, ty, lx_bot, by, DARK_GREY);
    sw_display.drawLine(rx_top, ty, rx_bot, by, DARK_GREY);
    // Floor cross-line
    sw_display.drawLine(lx_bot, by, rx_bot, by, DARK_GREEN);
    // Seal the bottom corners — Bresenham vs lerp rounding can leave 1px gaps
    sw_display.drawPixel(lx_bot, by, GREY);
    sw_display.drawPixel(rx_bot, by, GREY);

    // ── Turret on the wall ──────────────────────────────────────────────────
    if(!trenchSegments[i].hasObstacle) continue;

    bool onLeft = (trenchSegments[i].turretSide % 2 == 0);
    int tBaseX = onLeft ? (lx_top + lx_bot) / 2 : (rx_top + rx_bot) / 2;
    int tBaseY = (ty + by) / 2;

    float psc = 1.0f - t;
    int tW  = max(2,(int)(psc*10));
    int tW2 = max(1,(int)(psc*5));
    int tH  = max(3,(int)(psc*14));
    int gH  = max(2,(int)(psc*6));

    if(psc < 0.08f || tBaseX < 2 || tBaseX > 125) continue;

    int shaftBot = tBaseY + tH;
    int gunBot   = shaftBot + gH;

    // ── Shaft — solid scanline-filled trapezoid ──────────────────────────────
    // Top width = 2*tW, bottom width = 2*tW2, tapers linearly
    for(int sy = tBaseY; sy <= shaftBot; sy++) {
      float ft = (shaftBot == tBaseY) ? 1.0f
               : (float)(sy - tBaseY) / (float)(shaftBot - tBaseY);
      int hw = (int)(tW + ft * (tW2 - tW));
      if(hw < 1) hw = 1;
      sw_display.fillRect(tBaseX - hw, sy, hw * 2, 1, DARK_GREY);
    }
    // Shaft outline for crispness
    sw_display.drawLine(tBaseX - tW,  tBaseY,   tBaseX - tW2, shaftBot, GREY);
    sw_display.drawLine(tBaseX + tW,  tBaseY,   tBaseX + tW2, shaftBot, GREY);
    sw_display.drawLine(tBaseX - tW,  tBaseY,   tBaseX + tW,  tBaseY,   GREY);

    // ── Gun housing — solid red block ────────────────────────────────────────
    sw_display.fillRect(tBaseX - tW2, shaftBot, tW2 * 2, gH, RED);
    // Dark recess in housing centre
    if(tW2 > 1)
      sw_display.fillRect(tBaseX - tW2 + 1, shaftBot + 1, tW2 * 2 - 2, gH - 2, DARK_GREY);

    // ── Barrel tracking player ───────────────────────────────────────────────
    if(tW2 > 1) {
      int gunCY = shaftBot + gH / 2;
      int bLen  = tW2 + 4;
      float bdx = playerX - tBaseX, bdy = playerY - gunCY;
      float bd  = sqrt(bdx * bdx + bdy * bdy);
      if(bd > 0.5f)
        sw_display.drawLine(tBaseX, gunCY,
          tBaseX + (int)((bdx / bd) * bLen),
          gunCY  + (int)((bdy / bd) * bLen), ORANGE);
    }

    // ── Lock-on indicator ────────────────────────────────────────────────────
    bool locked = trenchSegments[i].turretLocked;
    uint16_t eyeCol = locked ? ((millis()/100)%2==0 ? RED : ORANGE) : ORANGE;
    sw_display.fillRect(tBaseX - 1, shaftBot, 2, 2, eyeCol);
    if(locked && (millis()/200)%2==0 && tW > 3) {
      sw_display.setTextSize(1); sw_display.setTextColor(RED);
      sw_display.setCursor(constrain(tBaseX-8,0,100), constrain(gunBot+1,0,57));
      sw_display.print("LOCK");
    }
  }

  // ── 6. TIE fighters ────────────────────────────────────────────
  drawVaderTIE();
}


void drawTrenchPlayer() {
  // X-wing from behind, centred at (playerX, playerY), near bottom of screen
  int px = (int)playerX;
  int py = (int)playerY;

  // Fuselage
  sw_display.drawLine(px,   py-6, px,   py+1, LIGHT_BLUE);
  sw_display.drawLine(px-1, py-5, px-1, py,   GREY);
  sw_display.drawLine(px+1, py-5, px+1, py,   GREY);

  // Cockpit
  sw_display.drawPixel(px, py-6, WHITE);
  sw_display.drawPixel(px, py-5, CYAN);

  // S-foils open — upper pair spreads more, lower pair less
  sw_display.drawLine(px,   py-3, px-10, py-5, WHITE);   // upper-left
  sw_display.drawLine(px,   py-3, px+10, py-5, WHITE);   // upper-right
  sw_display.drawLine(px,   py-1, px-10, py+1, WHITE);   // lower-left
  sw_display.drawLine(px,   py-1, px+10, py+1, WHITE);   // lower-right

  // Red stripe on wings
  sw_display.drawPixel(px-6,  py-4, RED);
  sw_display.drawPixel(px+6,  py-4, RED);
  sw_display.drawPixel(px-6,  py,   RED);
  sw_display.drawPixel(px+6,  py,   RED);

  // Engine glow at wing tips — flickers
  bool glow = (millis()/90)%2 == 0;
  uint16_t ec = glow ? LIGHT_BLUE : CYAN;
  sw_display.drawPixel(px-10, py-5, ec);
  sw_display.drawPixel(px+10, py-5, ec);
  sw_display.drawPixel(px-10, py+1, ec);
  sw_display.drawPixel(px+10, py+1, ec);
}


void initializeShaft() {
  for(int i = 0; i < 8; i++) {
    shaftSegments[i].z = i * 20;
    shaftSegments[i].width = 40 - (i * 2); // Gets narrower as we go deeper
    shaftSegments[i].hasObstacle = (i > 2 && random(100) < 20); // Sparse obstacles
    shaftSegments[i].obstacleX = random(-10, 10);
  }
}

void drawMissileShaft() {
  // ── Tunnel walls ─────────────────────────────────────────────────────────
  int shaftLeft  = 60;
  int shaftRight = 68;

  sw_display.drawLine(shaftLeft,  8, shaftLeft,  56, GREY);
  sw_display.drawLine(shaftRight, 8, shaftRight, 56, GREY);

  // Scrolling depth/perspective markers
  for(int y = 10; y < 55; y += 8) {
    float depth      = (float)(y - 10) / 45.0;
    int   leftOffset = (int)(depth * 2);
    sw_display.drawLine(shaftLeft  + leftOffset, y, shaftLeft  + leftOffset, y + 6, DARK_GREEN);
    sw_display.drawLine(shaftRight - leftOffset, y, shaftRight - leftOffset, y + 6, DARK_GREEN);
  }

  // ── Reactor core ─────────────────────────────────────────────────────────
  // Scale up slightly as torpedo gets closer
  int coreExtra = (int)(shaftDepth / 55);
  if(coreExtra > 3) coreExtra = 3;

  int cx = 64, cy = 58;

  // Outer glow — slow alternating pulse
  uint16_t outerCol = ((millis() / 200) % 2 == 0) ? ORANGE : DARK_ORANGE;
  sw_display.drawCircle(cx, cy, 12 + coreExtra, outerCol);

  // Mid ring — faster three-phase pulse
  int midPhase = (millis() / 120) % 3;
  int midR     = 7 + coreExtra;
  uint16_t midCol = (midPhase == 0) ? YELLOW : (midPhase == 1 ? ORANGE : RED);
  sw_display.drawCircle(cx, cy, midR,     midCol);
  sw_display.drawCircle(cx, cy, midR - 1, midCol);

  // Bright inner core
  sw_display.fillCircle(cx, cy, 3 + coreExtra, WHITE);
  sw_display.fillCircle(cx, cy, 1 + coreExtra, YELLOW);

  // Energy spikes — cardinal and diagonal alternate each 100 ms
  int spikeLen = 5 + coreExtra;
  if((millis() / 100) % 2 == 0) {
    // Cardinal spikes
    sw_display.drawLine(cx,           cy - midR, cx,           cy - midR - spikeLen, YELLOW);
    sw_display.drawLine(cx,           cy + midR, cx,           cy + midR + spikeLen, YELLOW);
    sw_display.drawLine(cx - midR,    cy,        cx - midR - spikeLen, cy,            YELLOW);
    sw_display.drawLine(cx + midR,    cy,        cx + midR + spikeLen, cy,            YELLOW);
  } else {
    // Diagonal spikes
    int d = midR * 7 / 10;
    sw_display.drawLine(cx - d, cy - d, cx - d - 3, cy - d - 3, ORANGE);
    sw_display.drawLine(cx + d, cy - d, cx + d + 3, cy - d - 3, ORANGE);
    sw_display.drawLine(cx - d, cy + d, cx - d - 3, cy + d + 3, ORANGE);
    sw_display.drawLine(cx + d, cy + d, cx + d + 3, cy + d + 3, ORANGE);
  }

  // ── Missile ──────────────────────────────────────────────────────────────
  // Travels from y=10 (top of shaft) to y=52 (core mouth) as depth 0→215
  int mY = 10 + (int)(shaftDepth * 0.20);
  if(mY > 52) mY = 52;

  // Nose cone (points downward toward the core)
  sw_display.drawLine(63, mY,     65, mY,     WHITE);   // widest nose line
  sw_display.drawPixel(64, mY + 1, WHITE);              // tip

  // Torpedo body
  sw_display.fillRect(62, mY + 2, 5, 7, YELLOW);

  // Fins
  sw_display.drawLine(60, mY + 8, 62, mY + 6, ORANGE);  // left fin
  sw_display.drawLine(67, mY + 8, 65, mY + 6, ORANGE);  // right fin

  // Exhaust trail — grows longer as speed builds
  int trailLen = 4 + (int)(shaftDepth / 22);
  if(trailLen > 12) trailLen = 12;
  for(int t = 1; t <= trailLen; t++) {
    uint16_t trailCol = (t <= 3) ? WHITE : (t <= 7 ? YELLOW : ORANGE);
    sw_display.drawPixel(63,     mY - t,     trailCol);
    sw_display.drawPixel(65,     mY - t,     trailCol);
    if(t % 2 == 0)
      sw_display.drawPixel(64,   mY - t - 1, trailCol);
  }

  // ── Impact flash overlay ─────────────────────────────────────────────────
  if(shaftImpactFlash) {
    unsigned long elapsed = millis() - shaftImpactTime;
    if(elapsed < 80)
      sw_display.fillRect(0, 0, 128, 64, WHITE);
    else if(elapsed < 180)
      sw_display.fillRect(0, 0, 128, 64, YELLOW);
    else if(elapsed < 260)
      sw_display.fillRect(0, 0, 128, 64, ORANGE);
  }
}

void drawSpaceBattlePlayer() {
  // Draw crosshair for targeting
  if(millis() < flashCrosshair) {
    // Rapid pulse effect while firing
    int pulseSize = 6 + ((millis() / 40) % 2) * 2;  // Alternates between 6 and 8
    
    if(hitFlash) {
      // HIT - Green/bright flash with expanding rings
      sw_display.drawCircle(crosshairX, crosshairY, 10, ORANGE);
      sw_display.drawCircle(crosshairX, crosshairY, 12, YELLOW);
      sw_display.drawLine(crosshairX-14, crosshairY, crosshairX+14, crosshairY, YELLOW);
      sw_display.drawLine(crosshairX, crosshairY-14, crosshairX, crosshairY+14, YELLOW);
      sw_display.fillCircle(crosshairX, crosshairY, 4, ORANGE);
      
      // "HIT" text
      sw_display.setTextSize(1);
      sw_display.setTextColor(YELLOW);
      sw_display.setCursor(crosshairX - 9, crosshairY - 18);
      sw_display.print("HIT");
    } else {
      // MISS - Pulsing crosshair shows rapid fire
      sw_display.drawCircle(crosshairX, crosshairY, pulseSize, GREEN_LASER);
      sw_display.drawLine(crosshairX-10, crosshairY, crosshairX+10, crosshairY, GREEN_LASER);
      sw_display.drawLine(crosshairX, crosshairY-10, crosshairX, crosshairY+10, GREEN_LASER);
      sw_display.fillCircle(crosshairX, crosshairY, 2, YELLOW);
    }
  } else {
    // Normal crosshair - green targeting reticle
    sw_display.drawCircle(crosshairX, crosshairY, 6, GREEN_LASER);
    sw_display.drawLine(crosshairX-8, crosshairY, crosshairX+8, crosshairY, GREEN_LASER);
    sw_display.drawLine(crosshairX, crosshairY-8, crosshairX, crosshairY+8, GREEN_LASER);
    sw_display.drawPixel(crosshairX, crosshairY, WHITE);
  }
}

void drawUseTheForce() {
  float forceProgress = (millis() - stateTimer) / 7000.0f;
  if(forceProgress > 1.0f) forceProgress = 1.0f;

  float scale = 2.0f + (forceProgress * 15.0f);
  int   cx    = 64;
  int   cy    = 29 + (int)(sin(forceProgress * PI * 2) * 2); // gentle drift

  // ── Deterministic starfield — no random() so stars don't flicker ──────────
  uint32_t seed = 0xA5F3C1UL;
  for(int i = 0; i < 24; i++) {
    seed = seed * 1664525UL + 1013904223UL;
    int sx = (seed >> 17) & 127;
    seed = seed * 1664525UL + 1013904223UL;
    int sy = (seed >> 26) & 63;
    sw_display.drawPixel(sx, sy, GREY);
  }

  // ── Geometry ──────────────────────────────────────────────────────────────
  // From head-on: fuselage is NARROW and slightly taller than wide.
  // The 4 engines sit at the wing-root junction just outside the fuselage sides.
  // Wings extend outward FROM the engines to thin wingtip laser cannons.
  int   fuseHW  = max(2, (int)(scale * 0.50f)); // narrow
  int   fuseHH  = max(2, (int)(scale * 0.70f)); // taller than wide
  int   split   = max(1, (int)(scale * 0.13f)); // S-foil gap between wing panels
  int   engR    = max(2, (int)(scale * 0.22f)); // moderate engine radius

  // Engine positions — just outside the fuselage flanks, slight vertical offset
  int engOX = fuseHW + engR + 1;               // horizontal: just clear of fuselage
  int engOY = max(1, (int)(fuseHH * 0.28f));   // vertical: upper/lower pair offset

  int tlEng_x = cx - engOX,  tlEng_y = cy - engOY; // top-left  engine
  int trEng_x = cx + engOX,  trEng_y = cy - engOY; // top-right engine
  int blEng_x = cx - engOX,  blEng_y = cy + engOY; // bot-left  engine
  int brEng_x = cx + engOX,  brEng_y = cy + engOY; // bot-right engine

  // Wing tips — far out in the X pattern
  float wingLen = scale * 2.4f;
  float wingSpd = scale * 0.80f;
  int tlTip_x = cx - (int)wingLen,  tlTip_y = cy - (int)wingSpd;
  int trTip_x = cx + (int)wingLen,  trTip_y = cy - (int)wingSpd;
  int blTip_x = cx - (int)wingLen,  blTip_y = cy + (int)wingSpd;
  int brTip_x = cx + (int)wingLen,  brTip_y = cy + (int)wingSpd;

  // ── Wings — root at each engine, tip at each wingtip corner ──────────────
  // White leading edge, grey S-foil shadow offset, red stripe inset
  // Top-left
  sw_display.drawLine(tlEng_x, tlEng_y,         tlTip_x, tlTip_y,         WHITE);
  sw_display.drawLine(tlEng_x, tlEng_y + split, tlTip_x, tlTip_y + split, GREY);
  sw_display.drawLine(tlEng_x - engR/2, tlEng_y + 1,
                      (int)(cx - wingLen*0.70f), (int)(cy - wingSpd*0.70f), RED);
  // Top-right
  sw_display.drawLine(trEng_x, trEng_y,         trTip_x, trTip_y,         WHITE);
  sw_display.drawLine(trEng_x, trEng_y + split, trTip_x, trTip_y + split, GREY);
  sw_display.drawLine(trEng_x + engR/2, trEng_y + 1,
                      (int)(cx + wingLen*0.70f), (int)(cy - wingSpd*0.70f), RED);
  // Bottom-left
  sw_display.drawLine(blEng_x, blEng_y,         blTip_x, blTip_y,         WHITE);
  sw_display.drawLine(blEng_x, blEng_y - split, blTip_x, blTip_y - split, GREY);
  sw_display.drawLine(blEng_x - engR/2, blEng_y - 1,
                      (int)(cx - wingLen*0.70f), (int)(cy + wingSpd*0.70f), RED);
  // Bottom-right
  sw_display.drawLine(brEng_x, brEng_y,         brTip_x, brTip_y,         WHITE);
  sw_display.drawLine(brEng_x, brEng_y - split, brTip_x, brTip_y - split, GREY);
  sw_display.drawLine(brEng_x + engR/2, brEng_y - 1,
                      (int)(cx + wingLen*0.70f), (int)(cy + wingSpd*0.70f), RED);

  // ── 4 Wing-root engines — moderate circles at the wing/fuselage junction ──
  // Layered rings: outer housing → inner face → deep centre → thrust glow
  int ex[4] = { tlEng_x, trEng_x, blEng_x, brEng_x };
  int ey[4] = { tlEng_y, trEng_y, blEng_y, brEng_y };
  for(int i = 0; i < 4; i++) {
    sw_display.fillCircle(ex[i], ey[i], engR + 1, DARK_GREY); // outer housing
    sw_display.fillCircle(ex[i], ey[i], engR,     GREY);       // engine face
    sw_display.drawCircle(ex[i], ey[i], engR,     WHITE);      // rim highlight
    sw_display.fillCircle(ex[i], ey[i], engR/2,   DARK_GREY);  // recessed centre
    if(engR > 3)
      sw_display.fillCircle(ex[i], ey[i], engR/3, LIGHT_BLUE); // thrust glow
  }

  // ── Laser cannon barrels — thin GREY sticks past each wingtip ────────────
  float wLen = sqrt(wingLen*wingLen + wingSpd*wingSpd);
  if(wLen > 0) {
    int gunExt = max(3, (int)(scale * 0.35f));
    int dx = (int)(gunExt * wingLen / wLen);
    int dy = (int)(gunExt * wingSpd / wLen);
    sw_display.drawLine(tlTip_x, tlTip_y, tlTip_x - dx, tlTip_y - dy, GREY);
    sw_display.drawLine(trTip_x, trTip_y, trTip_x + dx, trTip_y - dy, GREY);
    sw_display.drawLine(blTip_x, blTip_y, blTip_x - dx, blTip_y + dy, GREY);
    sw_display.drawLine(brTip_x, brTip_y, brTip_x + dx, brTip_y + dy, GREY);
  }

  // ── Fuselage — heptagon scanline fill ────────────────────────────────────
  // Shape (7 sides, per reference):
  //   Flat top (narrow), upper chamfers angling OUT to full width,
  //   straight full-width sides, lower chamfers angling IN, small flat base.
  //
  // Zone boundaries (all relative to cy, using fuseHH fractions):
  //   Top flat   :  y = -fuseHH             half-width = fuseHW * 0.50
  //   Chamfer end:  y = -fuseHH * 0.42      half-width = fuseHW        (full)
  //   Mid zone   :  y = -fuseHH*0.42  →  +fuseHH*0.40   (full width)
  //   Taper start:  y =  fuseHH * 0.40      half-width = fuseHW
  //   Base flat  :  y =  fuseHH             half-width = fuseHW * 0.35

  float zTopY   = cy - fuseHH;          // top of shape
  float zChamY  = cy - fuseHH * 0.42f;  // end of upper chamfer / start of full width
  float zTaperY = cy + fuseHH * 0.40f;  // start of lower taper
  float zBotY   = cy + fuseHH;          // bottom of shape

  float wTop    = fuseHW * 0.50f;
  float wFull   = fuseHW;
  float wBot    = fuseHW * 0.35f;

  for(int y = (int)zTopY; y <= (int)zBotY; y++) {
    float hw;
    if(y <= (int)zChamY) {
      // Upper chamfer: interpolate from wTop → wFull
      float t = (zChamY == zTopY) ? 1.0f
                                  : (y - zTopY) / (zChamY - zTopY);
      hw = wTop + t * (wFull - wTop);
    } else if(y <= (int)zTaperY) {
      hw = wFull;  // straight sides
    } else {
      // Lower taper: interpolate from wFull → wBot
      float t = (zBotY == zTaperY) ? 1.0f
                                   : (y - zTaperY) / (zBotY - zTaperY);
      hw = wFull + t * (wBot - wFull);
    }
    int ihw = max(1, (int)hw);
    sw_display.fillRect(cx - ihw, y, ihw * 2, 1, WHITE);
  }

  // Spine and equator panel lines
  sw_display.drawLine(cx, (int)zTopY, cx, (int)zBotY, DARK_GREY);
  sw_display.drawLine(cx - fuseHW + 1, cy, cx + fuseHW - 1, cy, GREY);

  // Heptagon outline (drawn over fill so edges are crisp)
  // Top flat
  sw_display.drawLine(cx-(int)wTop, (int)zTopY, cx+(int)wTop, (int)zTopY, DARK_GREY);
  // Upper chamfer edges
  sw_display.drawLine(cx-(int)wTop,  (int)zTopY,  cx-fuseHW, (int)zChamY, DARK_GREY);
  sw_display.drawLine(cx+(int)wTop,  (int)zTopY,  cx+fuseHW, (int)zChamY, DARK_GREY);
  // Full-width sides
  sw_display.drawLine(cx-fuseHW, (int)zChamY,  cx-fuseHW, (int)zTaperY, DARK_GREY);
  sw_display.drawLine(cx+fuseHW, (int)zChamY,  cx+fuseHW, (int)zTaperY, DARK_GREY);
  // Lower taper edges
  sw_display.drawLine(cx-fuseHW, (int)zTaperY, cx-(int)wBot, (int)zBotY, DARK_GREY);
  sw_display.drawLine(cx+fuseHW, (int)zTaperY, cx+(int)wBot, (int)zBotY, DARK_GREY);
  // Base flat
  sw_display.drawLine(cx-(int)wBot, (int)zBotY, cx+(int)wBot, (int)zBotY, DARK_GREY);

  // Red hull stripe — horizontal band in lower-centre third
  if(fuseHW > 3) {
    int stripeW = max(2, (int)(fuseHW * 0.55f));
    int stripeH = max(1, (int)(fuseHH * 0.17f));
    int stripeY = cy + (int)(fuseHH * 0.12f);
    sw_display.fillRect(cx - stripeW, stripeY, stripeW * 2, stripeH, RED);
  }

  // ── Cockpit windscreen — dark slot in upper fuselage ─────────────────────
  {
    int wscW = max(2, (int)(fuseHW * 0.55f));
    int wscH = max(1, (int)(fuseHH * 0.18f));
    int wscY = cy - (int)(fuseHH * 0.38f);
    sw_display.fillRoundRect(cx - wscW, wscY, wscW * 2, wscH, wscH / 2, DARK_GREY);
  }

  // ── "USE THE FORCE" — Obi-Wan ghost blue, fades in at 2 s ────────────────
  if(forceProgress > 0.286f) {
    sw_display.setTextSize(1);
    sw_display.setTextColor(LIGHT_BLUE);
    sw_display.setCursor(25, 57);
    sw_display.println("USE THE FORCE");
  }
}


void drawTrenchEntry() {
  // Calculate animation progress (0.0 to 1.0 over 5 seconds)
  float progress = (millis() - stateTimer) / 5000.0;
  if(progress > 1.0) progress = 1.0;
  
  // Calculate trench dimensions that grow with progress (moved up for scope)
  int baseWidth = 12 + (progress * 30);   // Base width grows from 12 to 42
  int trenchLength = 8 + (progress * 10);  // Length grows from 8 to 18
  
  // Draw curved Death Star horizon line with opening where trench meets surface
  for(int x = 0; x < 128; x++) {
    int curveHeight = 45 + (abs(x - 64)) / 32;  // Less pronounced curve
    
    // Create opening where trench meets horizon - connect to trench sides
    // Find the trench opening edges for the current progress level
    int trenchOpeningLeft = 64 - (baseWidth/2);
    int trenchOpeningRight = 64 + (baseWidth/2);
    
    // Only draw horizon outside trench opening
    if(x < trenchOpeningLeft || x > trenchOpeningRight) {
      sw_display.drawPixel(x, curveHeight, WHITE);
    }
  }
  
  // X-wing starts large and gets smaller as it approaches the trench
  // Use the same detailed front-view X-wing from "Use the Force" scene
  float scale = 15 - (progress * 12); // Scale from 15 down to 3 (reverse of Use the Force)
  int xwingX = 64;
  int xwingY = 32 + sin(progress * PI * 2) * 1; // Slight vertical movement
  
  // Main fuselage nose (7-sided heptagon viewed straight on)
  int noseSize = scale * 1.2;

  // Draw heptagon nose (7-sided polygon viewed from front)
  float cx = xwingX, cy = xwingY;
  for(int i = 0; i < 7; i++) {
    float angle1 = (i * 2 * PI) / 7;
    float angle2 = ((i + 1) * 2 * PI) / 7;
    
    int x1 = cx + cos(angle1) * noseSize/2;
    int y1 = cy + sin(angle1) * noseSize/2;
    int x2 = cx + cos(angle2) * noseSize/2;
    int y2 = cy + sin(angle2) * noseSize/2;
    
    sw_display.drawLine(x1, y1, x2, y2, WHITE);
  }

  // Fill center of nose
  sw_display.fillCircle(xwingX, xwingY, noseSize/3, GREY);

  // X-wing configuration wings (closer together on each side)
  float wingLength = scale * 2.5;
  float wingSpread = scale * 0.4;

  // Top-left wing
  sw_display.drawLine(xwingX-noseSize/3, xwingY-scale*0.2, xwingX-wingLength, xwingY-wingSpread, WHITE); // keep
  sw_display.drawLine(xwingX-noseSize/3-1, xwingY-scale*0.2, xwingX-wingLength-1, xwingY-wingSpread, RED);

  // Top-right wing
  sw_display.drawLine(xwingX+noseSize/3, xwingY-scale*0.2, xwingX+wingLength, xwingY-wingSpread, WHITE); // primary
  sw_display.drawLine(xwingX+noseSize/3+1, xwingY-scale*0.2, xwingX+wingLength+1, xwingY-wingSpread, RED);

  // Bottom-left wing
  sw_display.drawLine(xwingX-noseSize/3, xwingY+scale*0.2, xwingX-wingLength, xwingY+wingSpread, WHITE); // primary
  sw_display.drawLine(xwingX-noseSize/3-1, xwingY+scale*0.2, xwingX-wingLength-1, xwingY+wingSpread, RED);

  // Bottom-right wing
  sw_display.drawLine(xwingX+noseSize/3, xwingY+scale*0.2, xwingX+wingLength, xwingY+wingSpread, WHITE); // primary
  sw_display.drawLine(xwingX+noseSize/3+1, xwingY+scale*0.2, xwingX+wingLength+1, xwingY+wingSpread, RED);

  // Engines near fuselage (4 small circles)
  sw_display.fillCircle(xwingX-noseSize/2-scale*0.1, xwingY-scale*0.3, scale*0.25, ORANGE);
  sw_display.fillCircle(xwingX+noseSize/2+scale*0.1, xwingY-scale*0.3, scale*0.25, ORANGE);
  sw_display.fillCircle(xwingX-noseSize/2-scale*0.1, xwingY+scale*0.3, scale*0.25, ORANGE);
  sw_display.fillCircle(xwingX+noseSize/2+scale*0.1, xwingY+scale*0.3, scale*0.25, ORANGE);

  // Wing tip guns (small circles at wing ends)
  sw_display.fillCircle(xwingX-wingLength, xwingY-wingSpread, scale*0.15, GREY);
  sw_display.fillCircle(xwingX+wingLength, xwingY-wingSpread, scale*0.15, GREY);
  sw_display.fillCircle(xwingX-wingLength, xwingY+wingSpread, scale*0.15, GREY);
  sw_display.fillCircle(xwingX+wingLength, xwingY+wingSpread, scale*0.15, GREY);

  // Engine exhaust glow (flickering)
  if((millis() / 150) % 2 == 0) {
    sw_display.drawCircle(xwingX-noseSize/2-scale*0.1, xwingY-scale*0.3, scale*0.15, LIGHT_BLUE);
    sw_display.drawCircle(xwingX+noseSize/2+scale*0.1, xwingY-scale*0.3, scale*0.15, LIGHT_BLUE);
    sw_display.drawCircle(xwingX-noseSize/2-scale*0.1, xwingY+scale*0.3, scale*0.15, LIGHT_BLUE);
    sw_display.drawCircle(xwingX+noseSize/2+scale*0.1, xwingY+scale*0.3, scale*0.15, LIGHT_BLUE);
  }

  // Now draw the trench with proper perspective - stationary trench opening
  // The trench should be visible immediately as X-wing approaches
  int trenchCenterX = 64;
  
  // No horizontal movement - trench stays centered
  int movementOffset = 0;
  
  // Draw 4 nested rectangles from largest (bottom) to smallest (top/horizon)
  for(int layer = 0; layer < 4; layer++) {
    // Calculate size - largest layer first, each getting 20% smaller
    float layerScale = 1.0 - (layer * 0.2);
    int layerWidth = baseWidth * layerScale;
    int layerLength = trenchLength * layerScale;
    
    // Position each layer higher - largest at bottom, smallest connects to horizon
    int layerY;
    if(layer == 3) {
      // Smallest layer connects to Death Star horizon
      layerY = 45; // Death Star horizon line
    } else {
      // Other layers step upward from bottom
      layerY = 60 - (layer * 5); // Start at Y=60, step up by 5 each layer
    }
    
    // Calculate walls with movement offset
    int leftWall = trenchCenterX - layerWidth/2 + movementOffset;
    int rightWall = trenchCenterX + layerWidth/2 + movementOffset;
    int topWall = layerY - layerLength/2;
    int bottomWall = layerY + layerLength/2;
    
    // Only draw if visible on screen
    if(leftWall < 128 && rightWall > 0 && layerWidth > 2) {
      // Draw U-shaped trench (no top line)
      sw_display.drawLine(leftWall, topWall, leftWall, bottomWall, GREY);   // Left wall
      sw_display.drawLine(rightWall, topWall, rightWall, bottomWall, GREY); // Right wall  
      sw_display.drawLine(leftWall, bottomWall, rightWall, bottomWall, DARK_GREEN); // Bottom
      
      // Connect to next layer for perspective
      if(layer < 3) {
        float nextLayerScale = 1.0 - ((layer + 1) * 0.2);
        int nextLayerWidth = baseWidth * nextLayerScale;
        int nextLayerLength = trenchLength * nextLayerScale;
        
        int nextLayerY;
        if(layer + 1 == 3) {
          nextLayerY = 45; // Next layer is horizon
        } else {
          nextLayerY = 60 - ((layer + 1) * 5);
        }
        
        int nextLeftWall = trenchCenterX - nextLayerWidth/2 + movementOffset;
        int nextRightWall = trenchCenterX + nextLayerWidth/2 + movementOffset;
        int nextTopWall = nextLayerY - nextLayerLength/2;
        int nextBottomWall = nextLayerY + nextLayerLength/2;
        
        // Draw perspective lines connecting corners
        if(nextLeftWall < 128 && nextRightWall > 0) {
          sw_display.drawLine(leftWall, topWall, nextLeftWall, nextTopWall, DARK_GREY);
          sw_display.drawLine(rightWall, topWall, nextRightWall, nextTopWall, DARK_GREY);
          sw_display.drawLine(leftWall, bottomWall, nextLeftWall, nextBottomWall, DARK_GREEN);
          sw_display.drawLine(rightWall, bottomWall, nextRightWall, nextBottomWall, DARK_GREEN);
        }
      }
    }
  }
  
  // Add some trench details that move with the surface
if(progress > 0.4) {
  // Moving surface details - draw only left and right (skip middle i=1)
  for(int i = 0; i < 3; i++) {
    if(i == 1) continue; // Skip the middle detail
    int detailX = 30 + i * 30 + movementOffset;
    if(detailX > 10 && detailX < 118) {
      sw_display.drawLine(detailX, 43, detailX + 4, 43, DARK_GREEN);
      sw_display.drawPixel(detailX + 2, 42, WHITE);
    }
  }
}
  
  // Add Death Star surface details around the trench opening when big enough
  if(progress > 0.5 && baseWidth > 20) { // Fixed: use baseWidth instead of trenchWidth
    int outerLeft = trenchCenterX - baseWidth/2;
    int outerRight = trenchCenterX + baseWidth/2;
    int surfaceY = 45 + (abs(outerLeft - 64) / 64); // Match curved horizon
    
    // Surface panel lines extending outward from trench
    sw_display.drawLine(outerLeft - 3, surfaceY, outerLeft - 8, surfaceY, GREY);
    sw_display.drawLine(outerRight + 3, surfaceY, outerRight + 8, surfaceY, GREY);
    
    // Vertical surface details
    if(progress > 0.7) {
      sw_display.drawLine(outerLeft - 5, surfaceY - 2, outerLeft - 5, surfaceY + 2, DARK_GREY);
      sw_display.drawLine(outerRight + 5, surfaceY - 2, outerRight + 5, surfaceY + 2, DARK_GREY);
    }
  }
  
  // Show "ENTERING TRENCH" text
  if(progress > 0.5) {
    sw_display.setTextSize(1);
    sw_display.setTextColor(WHITE);
    sw_display.setCursor(25, 15);
    sw_display.println("ENTERING TRENCH");
  }
  
  // Add motion lines around the shrinking X-Wing to show forward movement
  if(progress > 0.2) {
    for(int i = 0; i < 8; i++) {
      float angle = i * 45;
      float rad = radians(angle);
      int startX = xwingX + cos(rad) * (wingLength + 5);
      int startY = xwingY + sin(rad) * (wingLength + 5);
      int endX = xwingX + cos(rad) * (wingLength + 8 + progress * 6);
      int endY = xwingY + sin(rad) * (wingLength + 8 + progress * 6);
      
      sw_display.drawLine(startX, startY, endX, endY, GREY);
    }
  }
}

void drawExhaustPort() {
  // Check if crosshair is perfectly aligned and flash the port
  float accuracy = sqrt(pow(crosshairX - 64, 2) + pow(crosshairY - 40, 2));
  if(accuracy <= 3 && targetingActive && !shotFired) {
    if((millis() / 100) % 2 == 0) { // Faster flashing (100ms instead of 200ms)
      // Enhanced flashing with more concentric rings
      sw_display.fillCircle(64, 40, 20, ORANGE);
      sw_display.fillCircle(64, 40, 18, BLACK);
      sw_display.fillCircle(64, 40, 16, YELLOW);
      sw_display.fillCircle(64, 40, 14, BLACK);
      sw_display.fillCircle(64, 40, 12, ORANGE);
      sw_display.fillCircle(64, 40, 10, BLACK);
      sw_display.fillCircle(64, 40, 8, YELLOW);
      sw_display.fillCircle(64, 40, 6, BLACK);
      sw_display.fillCircle(64, 40, 4, ORANGE);
      sw_display.fillCircle(64, 40, 2, BLACK);
      sw_display.fillCircle(64, 40, 1, WHITE);
      
      // Extra outer rings that only appear during perfect targeting
      sw_display.drawCircle(64, 40, 22, YELLOW);
      sw_display.drawCircle(64, 40, 24, ORANGE);
      sw_display.drawCircle(64, 40, 26, YELLOW);
      sw_display.drawCircle(64, 40, 28, ORANGE);
    }
  }
  
  // Move exhaust port down by 8 pixels to avoid UI overlap
  int centerX = 64;
  int centerY = 40; // Changed from 32 to 40
  
  // Draw smaller central exhaust port (reduced from radius 6 to 4)
  sw_display.drawCircle(centerX, centerY, 4, YELLOW);
  sw_display.fillCircle(centerX, centerY, 1, WHITE); // Smaller center dot
  
  // Draw concentric circles for depth
  sw_display.drawCircle(centerX, centerY, 8, GREEN_LASER);
  sw_display.drawCircle(centerX, centerY, 12, GREEN_LASER);
  sw_display.drawCircle(centerX, centerY, 16, YELLOW);
  
  // Draw radial lines from outer edge to center for depth effect
  for(int angle = 0; angle < 360; angle += 15) {
    float rad = radians(angle);
    int outerX = centerX + cos(rad) * 20;
    int outerY = centerY + sin(rad) * 20;
    int innerX = centerX + cos(rad) * 16;
    int innerY = centerY + sin(rad) * 16;
    sw_display.drawLine(outerX, outerY, innerX, innerY, GREEN_LASER);
  }
  
  // Draw structural support beams (like Death Star surface details)
  sw_display.drawLine(centerX-20, centerY, centerX+20, centerY, GREEN_LASER); // Horizontal beam
  sw_display.drawLine(centerX, centerY-20, centerX, centerY+20, GREEN_LASER); // Vertical beam
  sw_display.drawLine(centerX-10, centerY-10, centerX+10, centerY+10, YELLOW); // Diagonal beam 1
  sw_display.drawLine(centerX+10, centerY-10, centerX-10, centerY+10, YELLOW); // Diagonal beam 2
  
  // Draw target zone indicator (flashing ring around the actual target)
  if(targetingActive && !shotFired) {
    if((millis() / 200) % 2) { // Flash every 200ms
      sw_display.drawCircle(centerX, centerY, 6, YELLOW);
      sw_display.drawCircle(centerX, centerY, 7, ORANGE);
    }
  }
  
  sw_display.drawRect(centerX-40, centerY-10, 8, 20, GREY);  // Left panel
  sw_display.drawRect(centerX+32, centerY-10, 8, 20, GREY);  // Right panel
  
  sw_display.drawLine(centerX-36, centerY-6, centerX-36, centerY+6, DARK_GREY);
  sw_display.drawLine(centerX+36, centerY-6, centerX+36, centerY+6, DARK_GREY);
  
  // Draw large countdown timer at bottom right (removed "Time: " text)
  if(targetingActive && !shotFired) {
    int timeLeft = max(0, (int)((exhaustPortTimer - millis()) / 1000));
    sw_display.setTextSize(2);
    sw_display.setTextColor(WHITE);
    sw_display.setCursor(110, 48); // Bottom right position
    sw_display.print(timeLeft);
  }
}



void drawDeathStarApproach() {
  float starSize = map(deathStarDistance, 500, 80, 3, 50);
  if(starSize < 1) starSize = 1;
  int r  = (int)starSize;
  int cx = 64;
  int cy = 28;

  // 1. Base sphere — grey fill
  sw_display.fillCircle(cx, cy, r, GREY);

  // 2. Lit hemisphere — white on left side
  if(r > 2) {
    for(int offX = -r; offX < 0; offX++) {
      int chord = (int)sqrt(max(0.0f, (float)(r*r - offX*offX)));
      uint16_t col = (offX > -r/2) ? 0xFFFF : 0x8410;
      sw_display.drawLine(cx+offX, cy-chord, cx+offX, cy+chord, col);
    }
  }

  // 3. Shadow — dark right side
  if(r > 3) {
    for(int offX = r/3; offX <= r; offX++) {
      int chord = (int)sqrt(max(0.0f, (float)(r*r - offX*offX)));
      sw_display.drawLine(cx+offX, cy-chord, cx+offX, cy+chord, 0x4208);
    }
  }

  // 4. Equatorial trench — wide dark band
  if(r > 6) {
    int tw = max(1, r / 20);
    for(int ty = cy - tw; ty <= cy + tw; ty++) {
      int dx = (int)sqrt(max(0.0f, (float)(r*r - (ty-cy)*(ty-cy))));
      sw_display.drawLine(cx - dx, ty, cx + dx, ty, 0x4208);
    }
    if(r > 14) {
      int dx1 = (int)sqrt(max(0.0f, (float)(r*r - (float)(tw+1)*(tw+1))));
      sw_display.drawLine(cx - dx1, cy - tw - 1, cx + dx1, cy - tw - 1, 0xFFFF);
      sw_display.drawLine(cx - dx1, cy + tw + 1, cx + dx1, cy + tw + 1, 0xFFFF);
    }
  }

  // 5. Surface panel latitude lines
  if(r > 16) {
    int spacing = max(4, r / 4);
    for(int latY = cy - spacing; latY > cy - r + 2; latY -= spacing) {
      int dx = (int)sqrt(max(0.0f, (float)(r*r - (latY-cy)*(latY-cy))));
      if(dx > 2) sw_display.drawLine(cx - dx + 1, latY, cx + dx - 1, latY, 0x8410);
    }
    for(int latY = cy + spacing; latY < cy + r - 2; latY += spacing) {
      int dx = (int)sqrt(max(0.0f, (float)(r*r - (latY-cy)*(latY-cy))));
      if(dx > 2) sw_display.drawLine(cx - dx + 1, latY, cx + dx - 1, latY, 0x8410);
    }
    // Longitude lines
    for(int lx = -1; lx <= 1; lx += 2) {
      int vx = cx + lx * (r / 3);
      int chord = (int)sqrt(max(0.0f, (float)(r*r - (float)(r/3)*(r/3))));
      sw_display.drawLine(vx, cy - chord, vx, cy + chord, 0x8410);
    }
  }

  // 6. Superlaser dish — upper-left hemisphere, large and prominent
  if(r > 8) {
    int dishR = max(2, r / 4);
    int dishX = cx - (int)(r * 0.38f);
    int dishY = cy - (int)(r * 0.35f);
    sw_display.fillCircle(dishX, dishY, dishR, 0x4208);
    sw_display.drawCircle(dishX, dishY, dishR, 0x8410);
    sw_display.drawCircle(dishX, dishY, dishR + 1, 0xFFFF);
    sw_display.fillCircle(dishX, dishY, max(1, dishR / 3), 0xFFFF);
    // 8 tributary beam lines
    if(r > 18) {
      for(int b = 0; b < 8; b++) {
        float ang = b * 3.14159f / 4.0f;
        int lx2 = dishX + (int)(cos(ang) * dishR);
        int ly2 = dishY + (int)(sin(ang) * dishR);
        sw_display.drawLine(dishX, dishY, lx2, ly2, 0x8410);
      }
    }
  }

  // 7. Sphere outline
  sw_display.drawCircle(cx, cy, r, 0x8410);
  if(r > 4) sw_display.drawCircle(cx, cy, r + 1, 0x4208);

  // 8. Northern polar tower detail
  if(r > 20) {
    sw_display.fillCircle(cx, cy - r + 3, 2, 0xFFFF);
  }
}

void drawTargeting() {
  // Draw crosshair for targeting
  sw_display.drawLine(crosshairX - 8, crosshairY, crosshairX + 8, crosshairY, GREEN_LASER);
  sw_display.drawLine(crosshairX, crosshairY - 8, crosshairX, crosshairY + 8, GREEN_LASER);
  sw_display.drawCircle(crosshairX, crosshairY, 4, GREEN_LASER);
  
  // Flash effect when firing
  if(millis() < flashCrosshair) {
    sw_display.fillCircle(crosshairX, crosshairY, 6, GREEN_LASER);
  }
}

bool checkCollisions() {
  // Check trench wall collisions
  for(int i = 0; i < 6; i++) {
    if(abs(trenchSegments[i].z) < 15) { // Close to player
      float scale = 200.0 / (trenchSegments[i].z + 50);
      int leftWall = 64 - (64 - trenchSegments[i].leftX) * scale;
      int rightWall = 64 + (trenchSegments[i].rightX - 64) * scale;
      
       if(playerX < leftWall + 5 || playerX > rightWall - 5 ||
         (trenchSegments[i].hasObstacle && 
          abs(playerX - (64 + (trenchSegments[i].obstacleX - 64) * scale)) < 8)) {
        return true;
      }
    }
  }
  return false;
}

void drawUI() {
  if(currentState == DEATH_STAR_APPROACH) return;
  
  sw_display.setTextSize(1);

  // Health bonus popup - green
  if(millis() < showHealthBonus) {
    sw_display.setTextColor(GREEN);
    sw_display.setCursor(93, 14);
    sw_display.print("+5");
  } 
  
  // Score - yellow
  sw_display.setTextColor(YELLOW);
  sw_display.setCursor(10, 2);
  sw_display.print("SCORE:");
  sw_display.print(score);
  
  // Shield percentage - colour based on health level
  uint16_t shieldColour = (shields > 50) ? GREEN : (shields > 25) ? YELLOW : RED;
  sw_display.setTextColor(shieldColour);
  sw_display.setCursor(73, 2);
  sw_display.print(shields);
  sw_display.print("%");
  
  // Shield bar - outline white, fill colour by health
  sw_display.drawRect(97, 2, 22, 6, WHITE);
  if(shields > 0) {
    sw_display.fillRect(98, 3, map(shields, 0, 100, 0, 20), 4, shieldColour);
  }
  // Invincibility flash border
  if(millis() - lastDamageTime < INVINCIBILITY_TIME) {
    if((millis() / 100) % 2 == 0) {
      sw_display.drawRect(91, 1, 24, 8, CYAN);
    }
  }
}

// drawTitleScreen and drawDevMenu are no longer called from drawGame().
// They draw directly on the physical TFT and are called ONCE when something
// changes, exactly like GoL's showMenu(). No per-frame redraw = no flashing.

void drawTitleScreenDirect() {
  // Title screen is drawn through the canvas each frame (animated stars + text).
  // This function just ensures a clean black screen on first entry / return.
#ifdef GOL_INTEGRATION
  display->fillScreen(0x0000);
#endif
}

// ── Dev menu layout constants (shared by helpers below) ───────────────────
static const int DM_PX     = 20,  DM_PW  = 440;
static const int DM_PY     = 2,   DM_PH  = 316;
static const int DM_SEP_H  = 10;   // separator row height
static const int DM_ITEM_H = 18;   // selectable row height
static const int DM_FIRST_Y = DM_PY + 28; // Y of first item row

// Returns the top-Y of item i, accounting for the separator at index 6.
static int devMenuRowY(int i) {
  int y = DM_FIRST_Y;
  for(int j = 0; j < i; j++)
    y += (j == 6) ? DM_SEP_H : DM_ITEM_H;
  return y;
}

// Repaints a single row. Safe to call at any time — no fillScreen, no flicker.
static void devMenuDrawRow(int i, bool selected) {
#ifdef GOL_INTEGRATION
  int rowY = devMenuRowY(i);
  bool isSep = (i == 6);

  if(isSep) {
    // Separator — always the same, no highlight state
    display->fillRect(DM_PX+2, rowY, DM_PW-4, DM_SEP_H, 0x0000);
    display->drawLine(DM_PX+4, rowY+4, DM_PX+DM_PW-5, rowY+4, 0x4208);
    display->setTextSize(1);
    display->setTextColor(0x632C);
    display->setCursor(DM_PX + 168, rowY);
    display->print("CINEMATICS");
  } else if(selected) {
    display->fillRect(DM_PX+2, rowY, DM_PW-4, DM_ITEM_H-1, 0xFFFF);
    display->setTextSize(2);
    display->setTextColor(0x0000);
    display->setCursor(DM_PX + 8,  rowY + 1);
    display->print(">");
    display->setCursor(DM_PX + 28, rowY + 1);
    display->print(devMenuLabels[i]);
  } else {
    display->fillRect(DM_PX+2, rowY, DM_PW-4, DM_ITEM_H-1, 0x0000);
    display->setTextSize(2);
    display->setTextColor(0xFFFF);
    display->setCursor(DM_PX + 28, rowY + 1);
    display->print(devMenuLabels[i]);
  }
#endif
}

// Full initial draw — called ONCE when the menu opens.
void drawDevMenuDirect() {
#ifdef GOL_INTEGRATION
  display->fillScreen(0x0000);
  display->drawRect(DM_PX, DM_PY, DM_PW, DM_PH, 0xFFE0);   // yellow border

  // Title
  display->setTextSize(2);
  display->setTextColor(0xFFE0);
  display->setCursor(DM_PX + 90, DM_PY + 4);
  display->print("STAGE SELECT");
  display->drawLine(DM_PX+2, DM_PY+23, DM_PX+DM_PW-3, DM_PY+23, 0x4208);

  // All rows
  for(int i = 0; i < DEV_MENU_ITEMS; i++)
    devMenuDrawRow(i, i == devMenuSelection);

  // Footer — drawn after all rows so curY is correct
  int footerY = devMenuRowY(DEV_MENU_ITEMS);
  display->drawLine(DM_PX+2, footerY+3, DM_PX+DM_PW-3, footerY+3, 0x4208);
  display->setTextSize(2);
  display->setTextColor(0x07FF);
  display->setCursor(DM_PX + 60, footerY + 6);
  display->print("A=START   B=CANCEL");
#endif
}

// ── Star Wars logo letter drawing ────────────────────────────────────────
// Each letter drawn as filled rects at a given (cx,cy,sc) — centre + scale.
// sc=1 → letters are ~7px wide, ~10px tall (fits 128px canvas comfortably).
// Letters are drawn in the iconic chunky slab-serif style.

static void swFill(int x, int y, int w, int h, uint16_t col) {
  if(w<=0||h<=0) return;
  // clamp to canvas
  if(x<0){w+=x;x=0;} if(y<0){h+=y;y=0;}
  if(x+w>128) w=128-x; if(y+h>64) h=64-y;
  if(w>0&&h>0) sw_display.fillRect(x,y,w,h,col);
}

// Draw letter at pixel position (ox,oy), scale sc (px per unit).
// Coordinate system: letter fits in a 5-wide × 7-tall unit box.
static void swLetter(char c, float ox, float oy, float sc, uint16_t col) {
  // Helper: fill a unit-rect
  auto R = [&](float x,float y,float w,float h){
    swFill((int)(ox+x*sc),(int)(oy+y*sc),(int)(w*sc+0.5f),(int)(h*sc+0.5f),col);
  };
  switch(c) {
    case 'S':
      R(0,0,5,1.5f);  // top bar
      R(0,0,1.5f,3.5f); // left top
      R(0,2,5,1.5f);  // mid bar
      R(3.5f,2.5f,1.5f,3.5f); // right bot
      R(0,5.5f,5,1.5f); // bot bar
      break;
    case 'T':
      R(0,0,5,1.5f);  // top bar
      R(1.75f,0,1.5f,7); // stem
      break;
    case 'A':
      R(0,0,1.5f,7);  // left leg
      R(3.5f,0,1.5f,7); // right leg
      R(0,0,5,1.5f);  // top bar
      R(0,3,5,1.5f);  // mid bar
      break;
    case 'R':
      R(0,0,1.5f,7);  // left stem
      R(0,0,5,1.5f);  // top bar
      R(3.5f,0,1.5f,3.5f); // right top
      R(0,2,5,1.5f);  // mid bar
      R(2.5f,3.5f,1.5f,3.5f); // right diag (simplified)
      R(3.5f,5,1.5f,2); // leg
      break;
    case 'W':
      R(0,0,1.5f,7);  // left outer
      R(3.5f,0,1.5f,7); // right outer
      R(1.75f,3.5f,1.5f,3.5f); // centre stem
      R(0,5.5f,5,1.5f); // bottom connectors
      break;
    case ' ':
      break;
    default: break;
  }
}

// Draw "STAR WARS" with zoom-in animation.
// t: 0=just started (large), 1=settled (final size)
static void drawSWLogo(float t) {
  // Ease out: fast start, slow settle
  float ease = 1.0f - (1.0f-t)*(1.0f-t)*(1.0f-t);

  // Final: sc=3.6, centred on 64,28 (letters 5 units wide, 3px gap, "STAR WARS" = 2 words)
  // "STAR" = 4 letters × (5+1)=6 units = 24 units wide → 86px at sc=3.6
  // "WARS" = same
  const float scFinal = 3.4f;
  const float scStart = 18.0f;   // starts huge (mostly off canvas)
  float sc = scStart + (scFinal - scStart) * ease;

  // Letter width in pixels at this scale: 5*sc, gap between letters: sc
  float lw = 5.0f * sc;     // letter width
  float gap = sc * 0.8f;    // gap between letters
  float wordW = 4.0f * lw + 3.0f * gap;  // 4 letters per word

  // Centre of canvas
  const float CX = 64, CY = 30;
  float rowH = 7.0f * sc;    // letter height
  float rowGap = sc * 1.0f;  // gap between STAR and WARS rows

  // Row 1: STAR — centred horizontally, above centre
  float y1 = CY - rowH - rowGap * 0.5f;
  float x1 = CX - wordW * 0.5f;
  const char* star = "STAR";
  for(int i=0;i<4;i++) {
    swLetter(star[i], x1 + i*(lw+gap), y1, sc, YELLOW);
  }

  // Row 2: WARS
  float y2 = CY + rowGap * 0.5f;
  float x2 = CX - wordW * 0.5f;
  const char* wars = "WARS";
  for(int i=0;i<4;i++) {
    swLetter(wars[i], x2 + i*(lw+gap), y2, sc, YELLOW);
  }
}

// drawTitleScreen: called every frame through the canvas → animated stars.
void drawTitleScreen() {
  updateStars();
  drawStars();

  // Zoom animation: 0→1 over 2.5 seconds
  float t = (millis() - stateTimer) / 2500.0f;
  if(t > 1.0f) t = 1.0f;
  drawSWLogo(t);

  // Difficulty — centred at bottom
  {
    sw_display.setTextSize(1);
    const char* diffNames[] = {"EASY", "MEDIUM", "HARD"};
    uint16_t diffCols[] = {GREEN, YELLOW, RED};
    // Build string to measure width: "< DIFF:MEDIUM >"
    // textSize(1) = 6px per char
    char buf[24];
    snprintf(buf, sizeof(buf), "< DIFF:%s >", diffNames[swDifficulty]);
    int strW = strlen(buf) * 6;
    int xPos = (128 - strW) / 2;
    sw_display.setCursor(xPos, 57);
    sw_display.setTextColor(DARK_GREY);
    sw_display.print("< DIFF:");
    sw_display.setTextColor(diffCols[swDifficulty]);
    sw_display.print(diffNames[swDifficulty]);
    sw_display.setTextColor(DARK_GREY);
    sw_display.print(" >");
  }
}


void initializeWompRats() {
  for(int i = 0; i < 6; i++) {
    wompRats[i].x = random(20, 108);
    wompRats[i].y = random(-50, 10);
    wompRats[i].vx = random(-15, 16) / 10.0;
    wompRats[i].vy = random(12, 22) / 10.0;  // was 5/15 — faster from the start
    wompRats[i].active = true;
    wompRats[i].lastDirection = millis() + random(500, 1500);
  }
}

void initializeCanyonRocks() {
  for(int i = 0; i < 3; i++) {
    canyonRocks[i].x = random(20, 108);
    canyonRocks[i].y = random(-100, -20);
    canyonRocks[i].size = random(6, 12);
    canyonRocks[i].active = true;
  }
}

void drawWompRatTraining() {
  // Define canyon dimensions FIRST
  int topGap = 25;
  int bottomGap = 50;
  
  // Stars in the top section
  for(int i = 0; i < 70; i += 10) {
    int y = (int)(i + canyonScroll) % 70;
    if(random(100) < 30) {
      sw_display.drawPixel(random(25, 103), y, WHITE);
    }
  }
  
  // Add horizontal surface detail lines on the outer land areas
  for(int i = 0; i < 25; i++) {
    int baseY = i * 8;
    int y = (int)(baseY + canyonScroll) % 80;
    
    if(y >= 5 && y < 64) {
      int leftOuterWall = (64 - topGap) * (64 - y) / 64;
      int rightOuterWall = 128 - ((64 - topGap) * (64 - y) / 64);
      
      if(leftOuterWall > 5) {
        int lineLength = leftOuterWall / 3;
        int startX = leftOuterWall - 2;
        sw_display.drawLine(startX, y, startX - lineLength, y, SAND);
        
        if(i % 3 == 0 && startX - lineLength - 3 > 2) {
          sw_display.drawPixel(startX - lineLength - 3, y, BROWN);
        }
      }
      
      if(rightOuterWall < 123) {
        int lineLength = (128 - rightOuterWall) / 3;
        int startX = rightOuterWall + 2;
        sw_display.drawLine(startX, y, startX + lineLength, y, SAND);
        
        if(i % 3 == 0 && startX + lineLength + 3 < 126) {
          sw_display.drawPixel(startX + lineLength + 3, y, BROWN);
        }
      }
    }
  }
  
  // Draw canyon walls - brownish rock colour
  for(int y = 5; y < 64; y++) {
    int gapAtY = topGap + (bottomGap - topGap) * y / 64;
    int leftInnerWall = 64 - gapAtY;
    sw_display.drawPixel(leftInnerWall, y, BROWN);
  }
  
  for(int y = 5; y < 64; y++) {
    int leftOuterWall = (64 - topGap) * (64 - y) / 64;
    sw_display.drawPixel(leftOuterWall, y, BROWN);
  }
  
  for(int y = 5; y < 64; y++) {
    int gapAtY = topGap + (bottomGap - topGap) * y / 64;
    int rightInnerWall = 64 + gapAtY;
    sw_display.drawPixel(rightInnerWall, y, BROWN);
  }
  
  for(int y = 5; y < 64; y++) {
    int rightOuterWall = 128 - ((64 - topGap) * (64 - y) / 64);
    sw_display.drawPixel(rightOuterWall, y, BROWN);
  }
  
  // Vertical detail lines on left canyon wall
  for(int i = 0; i < 8; i++) {
    int lineOffset = (int)(i * 10 + canyonScroll) % 80;
    
    for(int y = 5; y < 64; y += 2) {
      int gapAtY = topGap + (bottomGap - topGap) * y / 64;
      int leftInnerWall = 64 - gapAtY;
      int leftOuterWall = (64 - topGap) * (64 - y) / 64;
      
      int lineProgress = lineOffset % 10;
      int lineX = leftOuterWall + ((leftInnerWall - leftOuterWall) * lineProgress) / 10;
      
      sw_display.drawPixel(lineX, y, SAND);
    }
  }
  
  // Vertical detail lines on right canyon wall
  for(int i = 0; i < 8; i++) {
    int lineOffset = (int)(i * 10 + canyonScroll) % 80;
    
    for(int y = 5; y < 64; y += 2) {
      int gapAtY = topGap + (bottomGap - topGap) * y / 64;
      int rightInnerWall = 64 + gapAtY;
      int rightOuterWall = 128 - ((64 - topGap) * (64 - y) / 64);
      
      int lineProgress = lineOffset % 10;
      int lineX = rightOuterWall - ((rightOuterWall - rightInnerWall) * lineProgress) / 10;
      
      sw_display.drawPixel(lineX, y, SAND);
    }
  }
  
  // Draw canyon rocks - embedded into floor with natural rock shape
  for(int i = 0; i < 3; i++) {
    if(canyonRocks[i].active && canyonRocks[i].y > 0 && canyonRocks[i].y < 64) {
      // Calculate canyon floor boundaries at this Y position
      int gapAtY = topGap + (bottomGap - topGap) * canyonRocks[i].y / 64;
      int leftFloor = 64 - gapAtY;
      int rightFloor = 64 + gapAtY;
      
      // Only draw if rock is within canyon floor boundaries
      if(canyonRocks[i].x > leftFloor + canyonRocks[i].size/2 && 
         canyonRocks[i].x < rightFloor - canyonRocks[i].size/2) {
        
        int rockX = canyonRocks[i].x;
        int rockY = canyonRocks[i].y;
        int rockSize = canyonRocks[i].size;
        
        // Draw irregular rock shape
        int halfSize = rockSize / 2;
        int topHalf = halfSize * 0.7;
        
        // Left side of rock (jagged) - brown rock colour
        sw_display.drawLine(rockX - halfSize, rockY, rockX - halfSize + 2, rockY - topHalf/2, BROWN);
        sw_display.drawLine(rockX - halfSize + 2, rockY - topHalf/2, rockX - halfSize/2, rockY - topHalf, BROWN);
        
        // Top of rock (rounded but irregular)
        sw_display.drawLine(rockX - halfSize/2, rockY - topHalf, rockX, rockY - topHalf - 2, SAND);
        sw_display.drawLine(rockX, rockY - topHalf - 2, rockX + halfSize/2, rockY - topHalf, SAND);
        
        // Right side of rock (jagged)
        sw_display.drawLine(rockX + halfSize/2, rockY - topHalf, rockX + halfSize - 2, rockY - topHalf/2, BROWN);
        sw_display.drawLine(rockX + halfSize - 2, rockY - topHalf/2, rockX + halfSize, rockY, BROWN);
        
        // Bottom line (embedded in floor)
        sw_display.drawLine(rockX - halfSize, rockY, rockX + halfSize, rockY, BROWN);
        
        // Add rock texture details
        if(rockSize > 8) {
          sw_display.drawLine(rockX - 1, rockY - topHalf + 1, rockX - 2, rockY - 1, DARK_GREY);
          sw_display.drawLine(rockX + 1, rockY - topHalf + 2, rockX + 2, rockY - 1, DARK_GREY);
          sw_display.drawPixel(rockX - halfSize + 1, rockY - 1, DARK_GREY);
          sw_display.drawPixel(rockX - halfSize + 1, rockY - 2, DARK_GREY);
        }
        
        // Small highlights on top
        if(rockSize > 6) {
          sw_display.drawPixel(rockX - 1, rockY - topHalf, SAND);
          sw_display.drawPixel(rockX + 1, rockY - topHalf - 1, SAND);
        }
        
        // Smaller detail rocks
        if(i % 5 == 0 && rockSize > 7) {
          int smallX = rockX - halfSize - 3;
          int smallSize = rockSize / 4;
          if(smallX > leftFloor + 2) {
            sw_display.drawLine(smallX - smallSize, rockY, smallX, rockY - smallSize, BROWN);
            sw_display.drawLine(smallX, rockY - smallSize, smallX + smallSize, rockY, BROWN);
            sw_display.drawLine(smallX - smallSize, rockY, smallX + smallSize, rockY, BROWN);
          }
        }
      }
    }
  }
  
  // Draw womp rats - green alien rodents
  for(int i = 0; i < 6; i++) {
    if(wompRats[i].active && wompRats[i].y > 0 && wompRats[i].y < 58) {
      int gapAtY = topGap + (bottomGap - topGap) * wompRats[i].y / 64;
      int leftFloor2 = 64 - gapAtY;
      int rightFloor2 = 64 + gapAtY;
      
      if(wompRats[i].x > leftFloor2 + 6 && wompRats[i].x < rightFloor2 - 6) {
        int rx = wompRats[i].x;
        int ry = wompRats[i].y;
      
      // Body (green creature)
      sw_display.fillCircle(rx, ry, 3, GREEN);
      sw_display.fillCircle(rx, ry - 1, 2, GREEN);
      sw_display.fillCircle(rx, ry + 1, 2, GREEN);
      
      // Head
      sw_display.fillCircle(rx, ry + 4, 2, GREEN);
      
      // Tail (darker green)
      sw_display.drawLine(rx, ry - 3, rx - 1, ry - 5, DARK_GREEN);
      sw_display.drawPixel(rx - 1, ry - 6, DARK_GREEN);
      
      // Animated legs
      int legOffset = ((millis() / 150) + i) % 2;
      
      if(legOffset == 0) {
        sw_display.drawLine(rx - 2, ry + 2, rx - 3, ry + 4, DARK_GREEN);
        sw_display.drawLine(rx + 2, ry + 2, rx + 1, ry + 3, DARK_GREEN);
      } else {
        sw_display.drawLine(rx - 2, ry + 2, rx - 1, ry + 3, DARK_GREEN);
        sw_display.drawLine(rx + 2, ry + 2, rx + 3, ry + 4, DARK_GREEN);
      }
      
      // Eyes (red dots)
      sw_display.drawPixel(rx - 1, ry + 4, RED);
      sw_display.drawPixel(rx + 1, ry + 4, RED);
      }
    }
  }
  
  int shipY = 57;
  
  sw_display.drawLine(t16X, shipY - 7, t16X - 3, shipY + 4, CYAN);
  sw_display.drawLine(t16X, shipY - 7, t16X + 3, shipY + 4, CYAN);
  sw_display.drawLine(t16X - 3, shipY + 4, t16X + 3, shipY + 4, CYAN);
  
  for(int y = shipY - 6; y <= shipY + 3; y++) {
    int width = (y - (shipY - 7)) * 3 / 11;
    sw_display.drawLine(t16X - width, y, t16X + width, y, LIGHT_BLUE);
  }
  
  sw_display.fillRect(t16X - 1, shipY - 3, 2, 3, BLACK);
  
  sw_display.drawLine(t16X - 3, shipY, t16X - 8, shipY, GREY);
  sw_display.drawLine(t16X - 3, shipY + 1, t16X - 8, shipY + 1, GREY);
  sw_display.drawLine(t16X - 8, shipY, t16X - 8, shipY + 1, GREY);
  
  sw_display.drawLine(t16X + 3, shipY, t16X + 8, shipY, GREY);
  sw_display.drawLine(t16X + 3, shipY + 1, t16X + 8, shipY + 1, GREY);
  sw_display.drawLine(t16X + 8, shipY, t16X + 8, shipY + 1, GREY);
  
  // Engine glow - orange flicker
  if((millis() / 150) % 2 == 0) {
    sw_display.drawPixel(t16X - 2, shipY + 5, ORANGE);
    sw_display.drawPixel(t16X + 2, shipY + 5, ORANGE);
  }
  
  // Draw crosshair - green targeting reticle
  if(millis() < flashCrosshair) {
    int pulseSize = 6 + ((millis() / 40) % 2) * 2;
    sw_display.drawCircle(crosshairX, crosshairY, pulseSize, YELLOW);
    sw_display.drawLine(crosshairX - 10, crosshairY, crosshairX + 10, crosshairY, YELLOW);
    sw_display.drawLine(crosshairX, crosshairY - 10, crosshairX, crosshairY + 10, YELLOW);
    sw_display.fillCircle(crosshairX, crosshairY, 3, YELLOW);
  } else {
    sw_display.drawCircle(crosshairX, crosshairY, 4, GREEN_LASER);
    sw_display.drawLine(crosshairX - 6, crosshairY, crosshairX + 6, crosshairY, GREEN_LASER);
    sw_display.drawLine(crosshairX, crosshairY - 6, crosshairX, crosshairY + 6, GREEN_LASER);
  }
  
  drawExplosions();
}

void updateWompRatTraining() {
  int topGap = 25;
  int bottomGap = 50;

  // Speed ramps 1.5x -> 2.5x across the 200pt stage goal (was 1.0x -> 2.0x)
  float stageProgress = constrain((float)(score - stageStartScore) / (float)GATE_WOMP_RAT, 0.0f, 1.0f);
  float speedMult = 1.5f + stageProgress;

  canyonScroll += 2.0f * speedMult;  // was 1.5f
  t16X = crosshairX;
  
  // Keep T-16 away from top and bottom edges
  if(crosshairY < 15) crosshairY = 15;
  if(crosshairY > 57) crosshairY = 57;
  
  for(int i = 0; i < 6; i++) {
    if(wompRats[i].active) {
      // Check distance to crosshair for evasive behavior
      float distToCrosshair = sqrt(pow(crosshairX - wompRats[i].x, 2) + 
                                   pow(crosshairY - wompRats[i].y, 2));
      
      // Faster, more aggressive evasion (HORIZONTAL ONLY)
      if(distToCrosshair < 35) {
        float evasionStrength = (35 - distToCrosshair) / 35.0;
        evasionStrength *= 0.8;
        
      float dx = wompRats[i].x - crosshairX;
      
      if(abs(dx) > 0.1) {
        wompRats[i].vx += (dx/abs(dx)) * evasionStrength * 1.3;
        wompRats[i].vx = constrain(wompRats[i].vx, -2.8, 2.8);
        
        if(isnan(wompRats[i].vx) || isinf(wompRats[i].vx)) {
          wompRats[i].vx = 0.5;
        }
      }
      }
      
      // Downward velocity — faster base, scales with stage progress
      wompRats[i].vy = constrain(wompRats[i].vy, 1.4f * speedMult, 2.2f * speedMult);  // was 0.8/1.5
      
      // Sine wave movement for natural scurrying
      float sineWave = sin(millis() * 0.003 + i) * 0.8;
      wompRats[i].x += wompRats[i].vx + sineWave;
      wompRats[i].y += wompRats[i].vy;
      
      if(isnan(wompRats[i].x) || wompRats[i].x < -50 || wompRats[i].x > 178) {
        wompRats[i].active = false;
        continue;
      }
      if(isnan(wompRats[i].y) || wompRats[i].y < -100 || wompRats[i].y > 100) {
        wompRats[i].active = false;
        continue;
      }
      
      if(distToCrosshair > 35) {
        wompRats[i].vx *= 0.92;
      }
      
      if(millis() > wompRats[i].lastDirection) {
        wompRats[i].vx += random(-10, 11) / 20.0;
        wompRats[i].vx = constrain(wompRats[i].vx, -1.5, 1.5);
        wompRats[i].lastDirection = millis() + random(800, 2000);
      }
      
      int gapAtY = topGap + (bottomGap - topGap) * wompRats[i].y / 64;
      int leftFloor = 64 - gapAtY;
      int rightFloor = 64 + gapAtY;
      
      if(wompRats[i].x < leftFloor + 8) {
        wompRats[i].x = leftFloor + 8;
        wompRats[i].vx = abs(wompRats[i].vx) * 0.8;
      }
      if(wompRats[i].x > rightFloor - 8) {
        wompRats[i].x = rightFloor - 8;
        wompRats[i].vx = -abs(wompRats[i].vx) * 0.8;
      }
      
      // Respawn rat if it goes off bottom
      if(wompRats[i].y > 64) {
        int spawnGap = topGap + (bottomGap - topGap) * (-20) / 64;
        if(spawnGap < topGap) spawnGap = topGap;
        int spawnLeft = 64 - spawnGap + 8;
        int spawnRight = 64 + spawnGap - 8;
        
        wompRats[i].x = random(spawnLeft, spawnRight);
        wompRats[i].y = random(-30, -10);
        wompRats[i].vx = random(-12, 13) / 10.0;
        wompRats[i].vy = random(12, 20) / 10.0f * speedMult;  // was 8/15
      }
    } else {
      // Respawn inactive rats
      if(random(100) < 2) {
        int spawnGap = topGap + 5;
        int spawnLeft = 64 - spawnGap + 8;
        int spawnRight = 64 + spawnGap - 8;
        
        wompRats[i].x = random(spawnLeft, spawnRight);
        wompRats[i].y = random(-30, -10);
        wompRats[i].vx = random(-12, 13) / 10.0;
        wompRats[i].vy = random(12, 20) / 10.0;  // was 8/15
        wompRats[i].active = true;
        wompRats[i].lastDirection = millis() + random(800, 2000);
      }
    }
  }
  
  for(int i = 0; i < 3; i++) {
    if(canyonRocks[i].active) {
      canyonRocks[i].y += 1.5f * speedMult;
      
      // Calculate canyon floor boundaries at rock position
      int gapAtY = topGap + (bottomGap - topGap) * canyonRocks[i].y / 64;
      int leftFloor = 64 - gapAtY;
      int rightFloor = 64 + gapAtY;
      
      // Keep rocks within canyon floor
      if(canyonRocks[i].x < leftFloor + canyonRocks[i].size) {
        canyonRocks[i].x = leftFloor + canyonRocks[i].size;
      }
      if(canyonRocks[i].x > rightFloor - canyonRocks[i].size) {
        canyonRocks[i].x = rightFloor - canyonRocks[i].size;
      }
      
      // Collision detection against the T-16 ship (centre: t16X, shipY=57).
      //
      // Y-range gate: only test collision while the rock centre is within
      // ±10px of the ship vertically.  Without this, rocks continue to
      // register hits as they scroll off the bottom of the screen.
      int   rockHalfSize  = canyonRocks[i].size / 2;
      int   rockTopHalf   = (int)(rockHalfSize * 0.7f);
      float rockCentreY   = canyonRocks[i].y - rockTopHalf - 1;

      if(rockCentreY > 15 && rockCentreY < 57 + 10) {   // only in the approach zone
        float dist = sqrt(pow(t16X - canyonRocks[i].x, 2) +
                          pow(57.0f - rockCentreY, 2));
        float hitThreshold  = rockHalfSize + 3;
        float clipThreshold = rockHalfSize + 5;  // tightened from +7

        if(dist < hitThreshold) {
          // Direct hit — lose 10 health, no score penalty
          shields -= 10;
          playSFX(SFX_HIT_PLAYER);
          damageFlash = millis() + 500;
          canyonRocks[i].active = false;
          if(shields <= 0) currentState = GAME_OVER;
        } else if(dist < clipThreshold) {
          // Clip — lose 5 health, no score penalty
          shields -= 5;
          playSFX(SFX_HIT_PLAYER);
          damageFlash = millis() + 250;
          canyonRocks[i].active = false;
          if(shields <= 0) currentState = GAME_OVER;
        }
      }
      
      // Respawn rock when it scrolls off the bottom
      if(canyonRocks[i].y > 70) {
        int spawnGap = topGap + 5;
        int spawnLeft = 64 - spawnGap + 6;
        int spawnRight = 64 + spawnGap - 6;
        
        canyonRocks[i].x = random(spawnLeft, spawnRight);
        canyonRocks[i].y = random(-30, -5);
        canyonRocks[i].size = random(6, 12);
        canyonRocks[i].active = true;
      }
    } else {
      // Rock was deactivated (hit the ship) – respawn it above the top of the screen
      int spawnGap = topGap + 5;
      int spawnLeft = 64 - spawnGap + 6;
      int spawnRight = 64 + spawnGap - 6;
      
      canyonRocks[i].x = random(spawnLeft, spawnRight);
      canyonRocks[i].y = random(-60, -10);
      canyonRocks[i].size = random(6, 12);
      canyonRocks[i].active = true;
    }
  }
}

void updateRemoteMovement() {
  int stageScore = score - stageStartScore;

  // Speed multiplier grows smoothly from 1.0 to 2.0 over the full stage.
  // Applied every frame so the ball visibly and smoothly accelerates.
  float speedMult = 1.0f + ((float)stageScore / (float)GATE_JEDI);
  if(speedMult > 2.0f) speedMult = 2.0f;

  // Occasional speed burst — every 3-6 seconds the ball lunges faster
  // to keep the player on their toes
  static unsigned long burstEnd = 0;
  static unsigned long nextBurst = 0;
  if(millis() >= nextBurst && millis() < burstEnd + 600) {
    speedMult *= 1.8f;   // burst speed
  } else if(millis() >= nextBurst) {
    burstEnd  = millis() + 400 + random(300);   // burst lasts 400-700ms
    nextBurst = millis() + 3000 + random(3000); // next burst in 3-6s
  }

  float baseSpeed = 1.4f;

  trainingRemote.x += trainingRemote.vx * speedMult;
  trainingRemote.y += trainingRemote.vy * speedMult;

  // Bounce off all four walls cleanly
  if(trainingRemote.x <= 15) {
    trainingRemote.x = 15;
    trainingRemote.vx = fabsf(trainingRemote.vx);   // always bounce inward
  }
  if(trainingRemote.x >= 113) {
    trainingRemote.x = 113;
    trainingRemote.vx = -fabsf(trainingRemote.vx);
  }
  if(trainingRemote.y <= 12) {
    trainingRemote.y = 12;
    trainingRemote.vy = fabsf(trainingRemote.vy);
  }
  if(trainingRemote.y >= 52) {
    trainingRemote.y = 52;
    trainingRemote.vy = -fabsf(trainingRemote.vy);
  }

  // Occasionally add a small random nudge to direction — this gives
  // unpredictability without any jump in position.
  // Probability rises slightly with stage score so later in the stage
  // the ball changes direction more erratically.
  int nudgeChance = 2 + stageScore / 50;  // 2% → 6% over stage
  if(nudgeChance > 6) nudgeChance = 6;
  if(random(100) < nudgeChance) {
    trainingRemote.vx += (random(2) ? 0.25f : -0.25f);
    trainingRemote.vy += (random(2) ? 0.18f : -0.18f);
  }

  // Occasionally flip one axis for a sharper direction change
  // — gives the ball "personality" without teleporting
  if(random(200) < 1) trainingRemote.vx = -trainingRemote.vx;
  if(random(250) < 1) trainingRemote.vy = -trainingRemote.vy;

  // Keep velocity magnitude sensible — not too slow, not too fast
  float vlen = sqrt(trainingRemote.vx*trainingRemote.vx + trainingRemote.vy*trainingRemote.vy);
  if(vlen < 0.5f && vlen > 0.001f) {
    // Too slow — normalise up to base speed
    trainingRemote.vx = (trainingRemote.vx / vlen) * baseSpeed;
    trainingRemote.vy = (trainingRemote.vy / vlen) * baseSpeed;
  }
  if(vlen > 2.8f) {
    // Too fast — clamp
    trainingRemote.vx = (trainingRemote.vx / vlen) * 2.8f;
    trainingRemote.vy = (trainingRemote.vy / vlen) * 2.8f;
  }
}

void initializeBlindfolding() {
  deflectionsSuccessful = 0;
  deflectionsMissed = 0;

  // Start somewhere on screen, moving at a gentle diagonal
  trainingRemote.x = random(30, 98);
  trainingRemote.y = random(18, 45);
  // Clean diagonal velocity — angle chosen so it heads toward the centre of the screen
  float dirX = (64.0f - trainingRemote.x);
  float dirY = (32.0f - trainingRemote.y);
  float len  = sqrt(dirX*dirX + dirY*dirY);
  if(len < 1.0f) len = 1.0f;
  float baseSpeed = 1.4f;
  trainingRemote.vx = (dirX / len) * baseSpeed + (random(2) ? 0.3f : -0.3f);
  trainingRemote.vy = (dirY / len) * baseSpeed + (random(2) ? 0.2f : -0.2f);

  trainingRemote.active         = true;
  trainingRemote.nextFireTime   = millis() + 3000;
  trainingRemote.showPrediction = false;
  trainingRemote.patternStartTime = millis();
  // Pattern fields unused by new movement but kept to avoid uninitialised reads
  trainingRemote.currentPattern   = PATTERN_RANDOM;
  trainingRemote.centerX          = 64;
  trainingRemote.centerY          = 32;
  trainingRemote.angle            = 0;
  trainingRemote.diveDirection    = 1;
  trainingRemote.zigzagDirection  = 1;

  blindfoldActive   = false;
  dangerZoneRadius  = 50.0;
  crosshairX        = 64;
  crosshairY        = 32;
}

void updateBlindfolding() {
  updateRemoteMovement();
  
  if(millis() - stateTimer > 2000) {
    blindfoldActive = true;
  }
  
  if(blindfoldActive && dangerZoneRadius > dangerZoneMinRadius) {
    dangerZoneRadius -= dangerZoneShrinkRate;
  }
  
  // Warning window is CONSTANT at 2.5s — player always has the same time to aim.
  // Difficulty increases via faster sphere movement and shorter gaps between attacks.
  const int warningTime = 2500;

  int stageScore = score - stageStartScore;
  int minInterval, maxInterval;
  if(stageScore < 50) {
    minInterval = 3000; maxInterval = 4500;
  } else if(stageScore < 100) {
    minInterval = 2500; maxInterval = 3800;
  } else if(stageScore < 150) {
    minInterval = 2000; maxInterval = 3200;
  } else {
    minInterval = 1500; maxInterval = 2800;
  }

  // Warning phase — sphere freezes and flashes
  if(millis() > trainingRemote.nextFireTime && !trainingRemote.showPrediction) {
    trainingRemote.predictX = trainingRemote.x;
    trainingRemote.predictY = trainingRemote.y;
    trainingRemote.showPrediction = true;
    trainingRemote.predictionTime = millis();
  }
  
  // Player missed — sphere fires
  if(trainingRemote.showPrediction && millis() - trainingRemote.predictionTime > warningTime) {
    deflectionsMissed++;
    shields -= 10;
    playSFX(SFX_HIT_PLAYER);
    damageFlash = millis() + 400;
    trainingRemote.showPrediction = false;
    trainingRemote.nextFireTime = millis() + random(minInterval, maxInterval);
    if(shields <= 0) currentState = GAME_OVER;
  }

  updateExplosions();
}

void drawBlindfolding() {
  sw_display.clearDisplay();

  // Sparse starfield background to give a sense of space
  for(int i = 0; i < 8; i++) {
    int sx = (i * 37 + 11) % 118;
    int sy = (i * 19 + 7)  % 50;
    sw_display.drawPixel(sx + 5, sy + 5, DARK_GREY);
  }

  // Floor grid — training room deck plates
  for(int x = 10; x < 118; x += 15)
    sw_display.drawLine(x, 55, x, 63, DARK_GREY);
  for(int y = 55; y < 64; y += 4)
    sw_display.drawLine(10, y, 118, y, DARK_GREY);

  // Wall panels
  sw_display.drawRect(2, 8, 10, 46, DARK_GREY);
  sw_display.drawRect(116, 8, 10, 46, DARK_GREY);
  sw_display.drawLine(2, 31, 12, 31, GREY);
  sw_display.drawLine(116, 31, 126, 31, GREY);

  // Speed / stage progress bar at BOTTOM — shows how fast the remote is now
  int stageScore = score - stageStartScore;
  int barW = map(constrain(stageScore, 0, GATE_JEDI), 0, GATE_JEDI, 0, 80);
  sw_display.drawRect(24, 58, 80, 4, DARK_GREY);
  uint16_t barCol = (stageScore < GATE_JEDI/2) ? CYAN : (stageScore < GATE_JEDI*4/5) ? YELLOW : RED;
  if(barW > 0) sw_display.fillRect(24, 58, barW, 4, barCol);

  // Energy trail — short line behind the remote showing direction
  if(!trainingRemote.showPrediction) {
    float trailX = trainingRemote.x - trainingRemote.vx * 3;
    float trailY = trainingRemote.y - trainingRemote.vy * 3;
    sw_display.drawLine(trailX, trailY, trainingRemote.x, trainingRemote.y, DARK_GREY);
  }

  // Training remote — layered circles for depth, colour shifts when fast
  uint16_t remoteCol = (stageScore > 150) ? ORANGE : (stageScore > 100) ? YELLOW : GREY;
  sw_display.fillCircle(trainingRemote.x, trainingRemote.y, 4, DARK_GREY);
  sw_display.drawCircle(trainingRemote.x, trainingRemote.y, 5, remoteCol);
  sw_display.drawCircle(trainingRemote.x, trainingRemote.y, 3, WHITE);
  // Equator and pole lines (classic remote look)
  sw_display.drawLine(trainingRemote.x - 5, trainingRemote.y,
                      trainingRemote.x + 5, trainingRemote.y, remoteCol);
  sw_display.drawLine(trainingRemote.x, trainingRemote.y - 5,
                      trainingRemote.x, trainingRemote.y + 5, remoteCol);
  // Engine glow
  if((millis() / 150) % 2 == 0) {
    sw_display.drawPixel(trainingRemote.x,     trainingRemote.y + 7, LIGHT_BLUE);
    sw_display.drawPixel(trainingRemote.x - 1, trainingRemote.y + 8, CYAN);
    sw_display.drawPixel(trainingRemote.x + 1, trainingRemote.y + 8, CYAN);
  }

  // Warning — expanding/contracting red rings when about to fire
  if(trainingRemote.showPrediction) {
    unsigned long elapsed = millis() - trainingRemote.predictionTime;
    int flashSpeed = max(40, 220 - (int)(elapsed / 10));
    if((millis() / flashSpeed) % 2 == 0) {
      sw_display.drawCircle(trainingRemote.x, trainingRemote.y,  8, RED);
      sw_display.drawCircle(trainingRemote.x, trainingRemote.y, 11, ORANGE);
      sw_display.drawCircle(trainingRemote.x, trainingRemote.y, 14, RED);
    } else {
      // Fill solid red when close to firing (last 0.8s)
      if(elapsed > 1700)
        sw_display.fillCircle(trainingRemote.x, trainingRemote.y, 6, RED);
    }
    // Countdown bar along BOTTOM edge of screen
    int timeLeft = max(0, 2500 - (int)elapsed);
    int cntW = map(timeLeft, 0, 2500, 0, 128);
    sw_display.fillRect(0, 62, cntW, 2, RED);
  }

  // Lightsaber deflection crosshair
  if(millis() < flashCrosshair) {
    sw_display.drawCircle(crosshairX, crosshairY, 8, YELLOW);
    sw_display.drawLine(crosshairX - 10, crosshairY, crosshairX + 10, crosshairY, YELLOW);
    sw_display.drawLine(crosshairX, crosshairY - 10, crosshairX, crosshairY + 10, YELLOW);
    sw_display.fillCircle(crosshairX, crosshairY, 3, WHITE);
  } else {
    sw_display.drawCircle(crosshairX, crosshairY, 5, LIGHT_BLUE);
    sw_display.drawLine(crosshairX - 7, crosshairY, crosshairX + 7, crosshairY, LIGHT_BLUE);
    sw_display.drawLine(crosshairX, crosshairY - 7, crosshairX, crosshairY + 7, LIGHT_BLUE);
  }
}

void drawForceLesson() {
  sw_display.clearDisplay();
  
  sw_display.setTextSize(1);
  sw_display.setTextColor(LIGHT_BLUE);  // Obi-Wan's ghostly blue presence
  
  sw_display.setCursor(5, 10);
  sw_display.println("Remember, a Jedi can");
  
  sw_display.setTextColor(CYAN);
  sw_display.setCursor(22, 22);
  sw_display.println("feel the Force");
  
  sw_display.setTextColor(LIGHT_BLUE);
  sw_display.setCursor(7, 34);
  sw_display.println("flowing through him");
  
  if((millis() / 500) % 2) {
    sw_display.setTextColor(WHITE);
    sw_display.setCursor(42, 57);
    sw_display.print("PRESS A");
  }
}

void drawTrainingComplete2() {
  sw_display.clearDisplay();
  
  sw_display.setTextSize(1);
  sw_display.setTextColor(YELLOW);
  sw_display.setCursor(10, 15);
  sw_display.println("TRAINING COMPLETE");
  
  sw_display.setTextColor(WHITE);
  sw_display.setCursor(25, 30);
  sw_display.println("READY FOR THE");
  sw_display.setCursor(30, 40);
  sw_display.println("REBELLION!");
  
  if((millis() / 500) % 2) {
    sw_display.setTextColor(CYAN);
    sw_display.setCursor(42, 55);
    sw_display.print("PRESS A");
  }
}

void drawTatooineSunset() {
  sw_display.clearDisplay();
  
  // Calculate animation progress (0 to 1 over 8 seconds)
  float progress = (millis() - stateTimer) / 8000.0;
  if(progress > 1.0) progress = 1.0;
  
  // Horizon line (raised 5 pixels higher)
  int horizonY = 43;
  
  // Binary suns descending (NO RAYS)
  // Slower descent - takes full 8 seconds to set
  int sunDescentOffset = progress * 18;
  
  // Upper sun (higher in sky, moved further right with more spacing)
  int upperSunX = 113;
  int upperSunY = 15 + sunDescentOffset;
sw_display.fillCircle(upperSunX, upperSunY, 10, YELLOW);
  
  // Lower sun (starts fully visible, slowly descends to horizon)
int lowerSunX = 93;
int lowerSunRadius = 7;
int lowerSunY = 30 + sunDescentOffset;

// Always draw the full sun first
sw_display.fillCircle(lowerSunX, lowerSunY, lowerSunRadius, ORANGE);

// If sun extends below horizon, cover the bottom part with black
if(lowerSunY + lowerSunRadius >= horizonY) {
    // Draw black rectangle to cover everything below the horizon
    int coverHeight = 64 - horizonY;
    sw_display.fillRect(0, horizonY, 128, coverHeight, BLACK);
}

// Distant desert mountains in background (rounded, mesa-like) - moved further left
// Mountain 1 - leftmost, rounded peak
int mtn1BaseY = horizonY - 20;
int mtn1Height = 12;

// Draw rounded mesa top (shifted left by 8 pixels total)
sw_display.drawLine(0, mtn1BaseY, 2, mtn1BaseY - mtn1Height/2, SAND);
sw_display.drawLine(2, mtn1BaseY - mtn1Height/2, 7, mtn1BaseY - mtn1Height, SAND);
sw_display.drawLine(7, mtn1BaseY - mtn1Height, 12, mtn1BaseY - mtn1Height, SAND);
sw_display.drawLine(12, mtn1BaseY - mtn1Height, 17, mtn1BaseY - mtn1Height/2, SAND);
sw_display.drawLine(17, mtn1BaseY - mtn1Height/2, 22, mtn1BaseY, SAND);

// Mountain 2 - center-left, also rounded
int mtn2BaseY = horizonY - 17;
int mtn2Height = 10;

sw_display.drawLine(24, mtn2BaseY, 29, mtn2BaseY - mtn2Height/2, SAND);
sw_display.drawLine(29, mtn2BaseY - mtn2Height/2, 32, mtn2BaseY - mtn2Height, SAND);
sw_display.drawLine(32, mtn2BaseY - mtn2Height, 38, mtn2BaseY - mtn2Height, SAND);
sw_display.drawLine(38, mtn2BaseY - mtn2Height, 42, mtn2BaseY - mtn2Height/2, SAND);
sw_display.drawLine(42, mtn2BaseY - mtn2Height/2, 47, mtn2BaseY, SAND);

// Mountain 3 - continuing the range to the right (connects to mountain 2)
int mtn3BaseY = horizonY - 15;
int mtn3Height = 8;

sw_display.drawLine(47, mtn3BaseY, 52, mtn3BaseY - mtn3Height/2, SAND);
sw_display.drawLine(52, mtn3BaseY - mtn3Height/2, 55, mtn3BaseY - mtn3Height, SAND);
sw_display.drawLine(55, mtn3BaseY - mtn3Height, 60, mtn3BaseY - mtn3Height, SAND);
sw_display.drawLine(60, mtn3BaseY - mtn3Height, 64, mtn3BaseY - mtn3Height/2, SAND);
sw_display.drawLine(64, mtn3BaseY - mtn3Height/2, 69, mtn3BaseY, SAND);

// Desert wind effect - animated diagonal lines in warm orange/dusk
int windOffset = (millis() / 100) % 128;
for(int i = 0; i < 8; i++) {
  int x = (windOffset + i * 20) % 128;
  int y = horizonY - 15 + (i * 3);
  
  if(y > 10 && y < horizonY) {
    sw_display.drawLine(x, y, x + 8, y - 2, DUSK_LOW);
    
    if(i % 2 == 0) {
      sw_display.drawLine(x + 15, y + 3, x + 21, y + 1, DUSK_LOW);
    }
  }
}

// Add some smaller dust particles near ground level
for(int i = 0; i < 5; i++) {
  int x = (windOffset * 2 + i * 30) % 128;
  int y = horizonY - 5 + (i % 3);
  if(x > 5 && x < 123) {
    sw_display.drawPixel(x, y, SAND);
    sw_display.drawPixel(x + 2, y + 1, SAND);
  }
}

// Luke's dome home - LEFT side, closer to viewer (below horizon) - LARGER
int domeX = 30;
int domeBaseY = horizonY + 18;
int domeRadius = 18;

// Side sections dimensions - scaled up proportionally
int sideWidth = 10;
int sideHeight = 13;

// Left side section - sitting on same base as dome
int leftSideX = domeX - domeRadius;
sw_display.drawRect(leftSideX - sideWidth, domeBaseY - sideHeight, sideWidth, sideHeight, GREY);

// Right side section - sitting on same base as dome
int rightSideX = domeX + domeRadius;
sw_display.drawRect(rightSideX, domeBaseY - sideHeight, sideWidth, sideHeight, GREY);

// Dome should connect to top corners of side sections
int domeTopY = domeBaseY - sideHeight;

// Draw dome as semi-circle from top of side sections
for(int angle = 180; angle <= 360; angle += 2) {
  float rad = radians(angle);
  int x1 = domeX + cos(rad) * domeRadius;
  int y1 = domeTopY + sin(rad) * sideHeight;
  int x2 = domeX + cos(radians(angle + 2)) * domeRadius;
  int y2 = domeTopY + sin(radians(angle + 2)) * sideHeight;
  sw_display.drawLine(x1, y1, x2, y2, WHITE);
}

// Base line of dome
sw_display.drawLine(domeX - domeRadius, domeBaseY, domeX + domeRadius, domeBaseY, GREY);

// Arched doorway entrance at front center - scaled up
int archX = domeX;
int archY = domeBaseY - 3;
int archWidth = 6;
int archHeight = 9;

// Draw arch (semi-circle doorway) in dark shadow colour
for(int angle = 180; angle <= 360; angle += 5) {
  float rad = radians(angle);
  int x = archX + cos(rad) * archWidth;
  int y = archY + sin(rad) * archHeight;
  sw_display.drawPixel(x, y, DARK_GREY);
}
// Arch sides
sw_display.drawLine(archX - archWidth, archY - archHeight, archX - archWidth, archY, DARK_GREY);
sw_display.drawLine(archX + archWidth, archY - archHeight, archX + archWidth, archY, DARK_GREY);
  
 // Draw horizon line - warm orange glow
int lukeX = 75;
int lukeLeftEdge = lukeX - 5;
int lukeRightEdge = lukeX + 5;

int domeLeftEdge = domeX - domeRadius - sideWidth;
int domeRightEdge = domeX + domeRadius + sideWidth;

sw_display.drawLine(0, horizonY, domeLeftEdge + 8, horizonY, ORANGE);
sw_display.drawLine(domeRightEdge - 9, horizonY, lukeLeftEdge, horizonY, ORANGE);
sw_display.drawLine(lukeRightEdge, horizonY, 128, horizonY, ORANGE);
  
  // Luke - white tunic silhouette
  int lukeY = horizonY - 6;
  
  // Head
  sw_display.fillCircle(lukeX, lukeY - 10, 3, WHITE);
  
  // Neck
  sw_display.drawLine(lukeX, lukeY - 7, lukeX, lukeY - 5, WHITE);
  sw_display.drawLine(lukeX - 1, lukeY - 7, lukeX - 1, lukeY - 5, WHITE);
  sw_display.drawLine(lukeX + 1, lukeY - 7, lukeX + 1, lukeY - 5, WHITE);
  
  // Body/tunic
  sw_display.drawLine(lukeX - 2, lukeY - 5, lukeX - 2, lukeY + 2, WHITE);
  sw_display.drawLine(lukeX - 1, lukeY - 5, lukeX - 1, lukeY + 2, WHITE);
  sw_display.drawLine(lukeX, lukeY - 5, lukeX, lukeY + 2, WHITE);
  sw_display.drawLine(lukeX + 1, lukeY - 5, lukeX + 1, lukeY + 2, WHITE);
  sw_display.drawLine(lukeX + 2, lukeY - 5, lukeX + 2, lukeY + 2, WHITE);
  
  // Belt
  sw_display.drawLine(lukeX - 3, lukeY + 1, lukeX + 3, lukeY + 1, GREY);
  
  // Left arm
  sw_display.drawLine(lukeX - 2, lukeY - 4, lukeX - 4, lukeY - 2, WHITE);
  sw_display.drawLine(lukeX - 4, lukeY - 2, lukeX - 4, lukeY + 2, WHITE);
  
  // Right arm
  sw_display.drawLine(lukeX + 2, lukeY - 4, lukeX + 4, lukeY - 2, WHITE);
  sw_display.drawLine(lukeX + 4, lukeY - 2, lukeX + 4, lukeY + 2, WHITE);
  
  // Legs
  sw_display.drawLine(lukeX - 1, lukeY + 2, lukeX - 2, lukeY + 8, WHITE);
  sw_display.drawLine(lukeX - 2, lukeY + 2, lukeX - 3, lukeY + 8, WHITE);
  
  sw_display.drawLine(lukeX + 1, lukeY + 2, lukeX + 2, lukeY + 8, WHITE);
  sw_display.drawLine(lukeX + 2, lukeY + 2, lukeX + 3, lukeY + 8, WHITE);
  
  // Feet
  sw_display.drawLine(lukeX - 3, lukeY + 8, lukeX - 4, lukeY + 8, WHITE);
  sw_display.drawLine(lukeX + 3, lukeY + 8, lukeX + 4, lukeY + 8, WHITE);
  
  // Desert details - warm sand colour
  sw_display.drawPixel(75, horizonY, SAND);
  sw_display.drawPixel(76, horizonY - 1, SAND);
  
  // "TATOOINE" text - yellow after 2 seconds
  if(progress > 0.25) {
    sw_display.setTextSize(1);
    sw_display.setTextColor(YELLOW);
    sw_display.setCursor(36, 2);
    sw_display.print("TATOOINE");
  }
}

void drawTrainingComplete() {
  sw_display.clearDisplay();
  
  sw_display.setTextSize(1);
  sw_display.setTextColor(YELLOW);
  sw_display.setCursor(10, 15);
  sw_display.println("TRAINING COMPLETE");
  
  sw_display.setTextColor(WHITE);
  sw_display.setCursor(25, 30);
  sw_display.println("READY FOR THE");
  sw_display.setCursor(30, 40);
  sw_display.println("REBELLION!");
  
  if((millis() / 500) % 2) {
    sw_display.setTextColor(CYAN);
    sw_display.setCursor(42, 55);
    sw_display.print("PRESS A");
  }
}

void drawVictory() {
  sw_display.clearDisplay();
  
  // Golden explosion effect
  for(int i = 0; i < 10; i++) {
    int x = 64 + random(-30, 30);
    int y = 32 + random(-20, 20);
    uint16_t col = (random(2) == 0) ? YELLOW : ORANGE;
    sw_display.fillCircle(x, y, random(1, 4), col);
  }
  
  sw_display.setTextSize(2);
  sw_display.setTextColor(YELLOW);
  sw_display.setCursor(5, 5);
  sw_display.println("DEATH");
  sw_display.setTextColor(ORANGE);
  sw_display.setCursor(5, 25);
  sw_display.println("STAR");
  sw_display.setTextColor(WHITE);
  sw_display.setCursor(0, 45);
  sw_display.println("DESTROYED!");
}

void drawCockpitWindow() {
  // Apply banking tilt during Death Star surface battle
  float leftTilt = 0, rightTilt = 0;
  if(currentState == DEATH_STAR_SURFACE) {
    float bankRadians = radians(bankingAngle);
    float bankSin = sin(bankRadians);
    leftTilt  = -(bankSin * 8);
    rightTilt =  (bankSin * 8);
  }

  // ── Cockpit frame — white ─────────────────────────────────────────────────
  sw_display.drawLine(5,   0 + leftTilt,  25,  54 + leftTilt,  WHITE);
  sw_display.drawLine(123, 0 + rightTilt, 103, 54 + rightTilt, WHITE);
  sw_display.drawLine(25, 54 + leftTilt, 103, 54 + rightTilt,  WHITE);
  sw_display.drawLine(0,   64, 25, 54 + leftTilt,  WHITE);
  sw_display.drawLine(128, 64, 103, 54 + rightTilt, WHITE);

  // Subtle inner brace lines for depth
  sw_display.drawLine(8,  0 + leftTilt*0.8f,  26, 54 + leftTilt,  GREY);
  sw_display.drawLine(120,0 + rightTilt*0.8f,102, 54 + rightTilt, GREY);

  // ── Stage progress bar — thin strip just above console (y=52) ───────────
  {
    int gate = (currentState == DEATH_STAR_SURFACE) ? GATE_SURFACE : GATE_SPACE;
    int prog = score - stageStartScore;
    int barW = map(constrain(prog, 0, gate), 0, gate, 0, 76);
    uint16_t barCol = (prog < gate/2) ? CYAN : (prog < gate*3/4) ? YELLOW : GREEN;
    // Background track
    sw_display.drawLine(26, 52, 102, 52, DARK_GREY);
    sw_display.drawLine(26, 53, 102, 53, DARK_GREY);
    // Fill
    if(barW > 0) {
      sw_display.drawLine(26, 52, 26 + barW, 52, barCol);
      sw_display.drawLine(26, 53, 26 + barW, 53, barCol);
    }
  }

  // ── Control panel — white with dark recesses ──────────────────────────────
  int panelLeftY  = 54 + (int)leftTilt;
  int panelRightY = 54 + (int)rightTilt;

  if(currentState == DEATH_STAR_SURFACE && (abs(leftTilt) > 1 || abs(rightTilt) > 1)) {
    // Tilted trapezoid
    for(int px = 25; px <= 103; px++) {
      float prog = (float)(px - 25) / (103 - 25);
      int topY   = panelLeftY + (int)((panelRightY - panelLeftY) * prog);
      sw_display.drawLine(px, topY, px, topY + 8, WHITE);
    }
  } else {
    sw_display.fillRect(25, 54, 78, 8, WHITE);
  }

  int dbY = 55 + (int)((leftTilt + rightTilt) * 0.25f);

  // Left section — navigation box (dark recess, green lines)
  sw_display.fillRect(28, dbY, 10, 6, DARK_GREY);
  sw_display.drawRect(28, dbY, 10, 6, GREY);
  sw_display.drawLine(30, dbY+1, 36, dbY+1, GREEN);
  sw_display.drawLine(33, dbY,   33, dbY+6, GREEN);
  // Blinking yellow light
  if((millis() / 400) % 2 == 0)
    sw_display.fillRect(31, dbY+3, 2, 2, YELLOW);
  else
    sw_display.fillRect(31, dbY+3, 2, 2, DARK_GREY);

  // Centre section — targeting computer (dark recess)
  sw_display.fillRect(55, dbY, 18, 6, DARK_GREY);
  sw_display.drawRect(55, dbY, 18, 6, GREY);
  sw_display.drawLine(57, dbY+1, 71, dbY+1, GREEN);
  sw_display.drawLine(59, dbY+3, 69, dbY+3, GREEN);
  sw_display.drawLine(60, dbY+4, 68, dbY+4, GREEN);
  // Blinking orange/red status light (alternates at different rate)
  uint16_t statusCol = ((millis() / 600) % 3 == 0) ? RED :
                       ((millis() / 600) % 3 == 1) ? ORANGE : DARK_GREY;
  sw_display.fillRect(63, dbY, 2, 2, statusCol);

  // Right section — weapon controls (dark recess)
  sw_display.fillRect(85, dbY, 12, 6, DARK_GREY);
  sw_display.drawRect(85, dbY, 12, 6, GREY);
  sw_display.drawLine(87, dbY+1, 95, dbY+1, RED);
  // Two blinking red fire-ready lights
  bool fireReady = (millis() / 300) % 2 == 0;
  sw_display.fillRect(88, dbY+3, 2, 2, fireReady ? RED    : DARK_GREY);
  sw_display.fillRect(92, dbY+3, 2, 2, fireReady ? ORANGE : DARK_GREY);
  sw_display.drawLine(89, dbY+4, 93, dbY+4, ORANGE);
}

void initializeTurrets() {
  for(int i = 0; i < 4; i++) {
    turrets[i].x = random(25, 103);
    turrets[i].y = random(15, 35);
    turrets[i].z = random(80, 200);
    turrets[i].active = true;
    turrets[i].lastFire = millis() + random(1000, 3000);
    turrets[i].health = 2;
  }
}

void updateTurrets() {
  for(int i = 0; i < 4; i++) {
    if(turrets[i].active) {
      turrets[i].z -= trenchSpeed;
      
      // Respawn turret if it goes behind player
      if(turrets[i].z <= 0) {
        turrets[i].x = random(25, 103);
        turrets[i].y = random(15, 35);
        turrets[i].z = random(150, 250);
        turrets[i].lastFire = millis() + random(1000, 3000);
        turrets[i].health = 2;
      }
      
      // Turret firing
      if(millis() > turrets[i].lastFire && turrets[i].z < 100) {
        float dx = playerX - turrets[i].x;
        float dy = playerY - turrets[i].y;
        float dist = sqrt(dx*dx + dy*dy);
        
        if(dist > 0) {
          fireProjectile(turrets[i].x, turrets[i].y, 
                        (dx/dist) * 2, (dy/dist) * 2, false);
        }
        turrets[i].lastFire = millis() + random(2000, 4000);
      }
    }
  }
}

void drawTurrets() {
  for(int i = 0; i < 4; i++) {
    if(turrets[i].active && turrets[i].z > 0) {
      float scale = 100.0 / turrets[i].z;
      int screenX = turrets[i].x;
      int screenY = turrets[i].y + (turrets[i].z / 8);
      
      if(screenX >= 0 && screenX < 128 && screenY >= 0 && screenY < 64) {
        int size = scale * 3;
        if(size < 2) size = 2;
        
        // Draw turret base
        sw_display.fillRect(screenX - size, screenY - size/2, size*2, size, LIGHT_BLUE);
        
        // Draw turret gun pointing at player
        sw_display.drawLine(screenX, screenY, 
                        screenX + (playerX - screenX) * 0.1, 
                        screenY + (playerY - screenY) * 0.1, ORANGE);
      }
    }
  }
}

void spawnPowerUp(float x, float y, int type) {
  for(int i = 0; i < 3; i++) {
    if(!powerUps[i].active) {
      powerUps[i].x = x;
      powerUps[i].y = y;
      powerUps[i].z = 50;
      powerUps[i].vx = random(-10, 11) / 10.0; // Random float velocity -1.0 to 1.0
      powerUps[i].vy = random(-10, 11) / 10.0;
      powerUps[i].type = type;
      powerUps[i].active = true;
      powerUps[i].spawnTime = millis();
      break;
    }
  }
}

void updatePowerUps() {
  for(int i = 0; i < 3; i++) {
    if(powerUps[i].active) {
      // Different movement for space battle vs trench run
      if(currentState == SPACE_BATTLE || currentState == DEATH_STAR_APPROACH) {
        // Float around in space battle
        powerUps[i].x += powerUps[i].vx;
        powerUps[i].y += powerUps[i].vy;
        
        // Bounce off screen edges
        if(powerUps[i].x < 15 || powerUps[i].x > 113) powerUps[i].vx = -powerUps[i].vx;
        if(powerUps[i].y < 15 || powerUps[i].y > 45) powerUps[i].vy = -powerUps[i].vy;
      } else {
        powerUps[i].z -= trenchSpeed * 0.8; // Move slower than trench
      }
      
      // Remove if too old (4 seconds in space, 8 seconds in trench) or behind player
      int timeout = (currentState == SPACE_BATTLE || currentState == DEATH_STAR_APPROACH) ? 4000 : 8000;
      if(powerUps[i].z <= 0 || millis() - powerUps[i].spawnTime > timeout) {
        powerUps[i].active = false;
        continue;
      }
      
      // Check player collection (use crosshair in space battle, player position in trench)
      float checkX = (currentState == SPACE_BATTLE || currentState == DEATH_STAR_APPROACH) ? crosshairX : playerX;
      float checkY = (currentState == SPACE_BATTLE || currentState == DEATH_STAR_APPROACH) ? crosshairY : playerY;
      float dist = sqrt(pow(checkX - powerUps[i].x, 2) + 
       pow(checkY - powerUps[i].y, 2));
      if(dist < 8) {
        // Apply power-up effect
        switch(powerUps[i].type) {
          case 0: // Shield boost
            shields += 25;
            if(shields > 100) shields = 100;
            break;
          case 1: // Rapid fire
            rapidFire = true;
            rapidFireEnd = millis() + 5000;
            break;
        }
        powerUps[i].active = false;
        score += 20;
        playSFX(SFX_POWER_UP);  // Power-up collected!
      }
    }
  }
}

void drawGameOver() {
  sw_display.clearDisplay();
  
  sw_display.setTextSize(2);
  sw_display.setTextColor(RED);
  sw_display.setCursor(12, 10);
  sw_display.println("GAME OVER");
  
  sw_display.setTextSize(1);
  sw_display.setTextColor(YELLOW);
  
  int scoreDigits = (score == 0) ? 1 : 0;
  int tempScore = score;
  while(tempScore > 0) {
    scoreDigits++;
    tempScore /= 10;
  }
  
  int scoreTextWidth = 42 + (scoreDigits * 6);
  int scoreCursorX = (128 - scoreTextWidth) / 2;
  
  sw_display.setCursor(scoreCursorX, 50);
  sw_display.print("SCORE: ");
  sw_display.print(score);
}


void drawPowerUps() {
  for(int i = 0; i < 3; i++) {
    if(powerUps[i].active) {
      int screenX = powerUps[i].x;
      int screenY = powerUps[i].y + (powerUps[i].z / 8);
      
      if(screenX >= 0 && screenX < 128 && screenY >= 0 && screenY < 64) {
        // Flash effect - blink every 200ms to be visible
        bool isFlashing = (millis() / 200) % 2 == 0;
        
        // Draw different power-up types
        switch(powerUps[i].type) {
          case 0: // Shield power-up
            if(isFlashing) {
              // Large flashing outline
              sw_display.drawRect(screenX-4, screenY-4, 8, 8, CYAN);
              sw_display.drawRect(screenX-3, screenY-3, 6, 6, WHITE);
              sw_display.fillRect(screenX-1, screenY-1, 2, 2, CYAN);
            } else {
              // Smaller inner square
              sw_display.drawRect(screenX-3, screenY-3, 6, 6, CYAN);
              sw_display.drawPixel(screenX, screenY, WHITE);
            }
            break;
          case 1: // Rapid fire (triangle)
            sw_display.drawLine(screenX, screenY-3, screenX-3, screenY+2, ORANGE);
            sw_display.drawLine(screenX-3, screenY+2, screenX+3, screenY+2, ORANGE);
            sw_display.drawLine(screenX+3, screenY+2, screenX, screenY-3, ORANGE);
            break;
          case 2: // Extra life (cross)
            sw_display.drawLine(screenX-3, screenY, screenX+3, screenY, YELLOW);
            sw_display.drawLine(screenX, screenY-3, screenX, screenY+3, YELLOW);
            break;
        }
      }
    }
  }
}
// ============================================================
// BUTTON FUNCTIONS  (digital GPIO, active LOW + INPUT_PULLUP)
// ============================================================
// 'val' parameter kept for API compatibility with existing
// handleInput() calls – we just ignore it and read pins directly.
int readButtonValue() {
  // Returns a bitmask: UP=1 DOWN=2 LEFT=4 RIGHT=8 FIRE=16 B=32
  // (unused by the original code – isBtnXxx() read the pins directly)
  return 0;
}

bool sw_isBtnUp(int /*val*/)    { return digitalRead(BTN_UP)    == LOW; }
bool sw_isBtnDown(int /*val*/)  { return digitalRead(BTN_DOWN)  == LOW; }
bool sw_isBtnLeft(int /*val*/)  { return digitalRead(BTN_LEFT)  == LOW; }
bool sw_isBtnRight(int /*val*/) { return digitalRead(BTN_RIGHT) == LOW; }
bool sw_isBtnSet(int /*val*/)   { return digitalRead(BTN_FIRE)  == LOW; }
bool sw_isBtnB(int /*val*/)     { return digitalRead(BTN_B)     == LOW; }