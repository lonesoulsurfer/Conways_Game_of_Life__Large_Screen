/*
 * =====================================================================
 *   ArcadeGames.ino — Breakout + Gyruss for Game_of_Life_Colour
 *   Ported from RetroArcade (320×240 Adafruit_ST7789)
 *   Target: ST7796S 480×320, Arduino_GFX_Library
 *
 *   Place in the same sketch folder as Game_of_Life_Colour.ino
 *   and StarWars.ino.
 *
 *   Entry points (called from Game_of_Life_Colour.ino):
 *     void runBreakout()    — runs until player quits, then returns
 *     bool gyrussLoop()     — call in a loop; returns true when player exits
 * =====================================================================
 *
 *  BREAKOUT BEYOND CONTROLS
 *    LEFT / RIGHT → move paddle         B → speed boost
 *    
 *    FIRE (SET)  → launch ball / pause
 *    SET + B      → exit to arcade menu
 *
 *  GYRUSS CONTROLS
 *    LEFT / RIGHT  → rotate ship around ring
 *    SET or B      → fire (not both — both = exit combo)
 *    SET + B held  → exit to arcade menu
 *    UP (hold)     → smart bomb
 * =====================================================================
 */

// ── File ordering note ────────────────────────────────────────────────────
// Arduino IDE merges .ino files alphabetically before compiling.
// "ArcadeGames" sorts before "Game_of_Life_Colour", so Arduino_GFX types
// and isBtnXxx() helpers aren't yet visible here.  We work around this by:
//   1. Including the Arduino_GFX header directly (harmless duplicate include).
//   2. Forward-declaring every cross-file symbol we need.
//   3. Forward-declaring Gyruss functions called before their definition.

#include <Arduino_GFX_Library.h>

// display and playTone() are declared in Game_of_Life_Colour.ino
extern Arduino_GFX *display;
extern void playTone(int frequency, int duration);
extern bool soundEnabled;
extern bool arcadeExitToMain;  // set true when A+B pressed to go to main menu

// Button helpers (defined in Game_of_Life_Colour.ino)
extern bool isBtnUp();
extern bool isBtnDown();
extern bool isBtnLeft();
extern bool isBtnRight();
extern bool isBtnSet();
extern bool isBtnB();

// ── Redirect RetroArcade's blocking beepTone to GoL's playTone ─────────────
// playTone already guards soundEnabled, so no extra check needed here.
static inline void beepTone(int freq, int durationMs) {
  playTone(freq, durationMs);
}

// ── Shared sound helpers (only menu-navigation sounds shared here) ──────────
// Game-specific sounds are defined in each game's section below.
static void sndMenuMove()  { beepTone(800,  25); }
static void sndSelect()    { beepTone(600,  35); beepTone(900, 55); }

// ── Colour palette (RGB565) — prefixed ARC_ to avoid collision with GoL ────
#define ARC_BLACK    0x0000
#define ARC_WHITE    0xFFFF
#define ARC_RED      0xF800
#define ARC_GREEN    0x07E0
#define ARC_BLUE     0x001F
#define ARC_CYAN     0x07FF
#define ARC_MAGENTA  0xF81F
#define ARC_YELLOW   0xFFE0
#define ARC_ORANGE   0xFD20
#define ARC_DKGRAY   0x39E7
#define ARC_LTGRAY   0xCE79
#define ARC_DKBLUE   0x000F
#define ARC_TEAL     0x03EF
#define ARC_GOLD     0xFEA0


// ══════════════════════════════════════════════════════════════════════════
//
//                   B R E A K O U T   B E Y O N D
//
//  Inspired by Atari / Choice Provisions Breakout Beyond (2025).
//  Paddle on the LEFT, bricks on the RIGHT.
//  Goal: get the ball past the right edge to advance — don't clear every brick.
//  Combo multiplier, spin physics, multiball, shield, bomb powerups.
//  Neon aesthetic: glowing ball trail, bright colours, dark background.

// ══════════════════════════════════════════════════════════════════════════
//
//                   B R E A K O U T   B E Y O N D
//
//  Classic Breakout orientation (paddle at bottom, bricks at top) with
//  Breakout Beyond-inspired features: combo multiplier, spin physics,
//  multiball, shield, bomb bricks, hard bricks, neon aesthetic.
//
//  CONTROLS
//    LEFT / RIGHT → move paddle         B → speed boost
//    FIRE (SET)   → launch ball / pause
//    SET + B      → exit to arcade menu
//
// ══════════════════════════════════════════════════════════════════════════

// ── Layout (480×320 landscape) ────────────────────────────────────────────
#define BRK_STATUS_H      20          // top status bar

// Play field
#define BRK_PLAY_TOP      (BRK_STATUS_H + 1)
#define BRK_PLAY_BOTTOM   318
#define BRK_PLAY_LEFT     1
#define BRK_PLAY_RIGHT    478

// Paddle at bottom, moves left/right
#define BRK_PADDLE_Y      304
#define BRK_PADDLE_H      11
#define BRK_PADDLE_W_BASE 70

// Ball
#define BRK_BALL_R        6

// Bricks — 12 columns × 7 rows filling the upper portion
#define BRK_COLS          12
#define BRK_ROWS           7
#define BRK_W             36          // (480 - 2*margin) / 12 - gap ≈ 36
#define BRK_H             20
#define BRK_GAP_X          3
#define BRK_GAP_Y          3
// Centre the grid: 12*(36+3)=468, margin=(480-468)/2=6
#define BRK_X0             6
#define BRK_Y0            (BRK_PLAY_TOP + 4)

// ── EEPROM ────────────────────────────────────────────────────────────────
#define EEPROM_BRK_HI 4

// ── Neon colour themes — 4 palettes cycling every 5 levels ───────────────
static const uint16_t BRK_THEMES[4][7] = {
  // Theme 0 — Classic Neon (levels 1-5)
  { 0xF81F, 0xF800, 0xFD20, 0xFFE0, 0x07E0, 0x07FF, 0x001F },
  // Theme 1 — Solar Flare (levels 6-10)
  { 0xFFE0, 0xFD20, 0xFB00, 0xF800, 0xF81F, 0xC81F, 0x801F },
  // Theme 2 — Ice Storm (levels 11-15)
  { 0xFFFF, 0xAFFF, 0x07FF, 0x055F, 0x001F, 0x601F, 0xF81F },
  // Theme 3 — Toxic (levels 16-20)
  { 0x07E0, 0x4FE0, 0xAFE0, 0xFFE0, 0xFD20, 0xF800, 0x07FF },
};
static const int BRICK_PTS[7] = { 120, 100, 90, 80, 70, 60, 50 };
// Mutable active palette — set at the start of each level
static uint16_t BRICK_COL[7];

// ── Brick types ───────────────────────────────────────────────────────────
#define BTYPE_EMPTY   0
#define BTYPE_NORMAL  1
#define BTYPE_HARD    2   // 2 hits; drawn with double border
#define BTYPE_BOMB    3   // clears 3×3 region on break
#define BTYPE_MULTI   4   // spawns a second ball on break
#define BTYPE_SHIELD  5   // grants a bottom safety net for a few seconds
#define BTYPE_TURBO   6   // speeds ball up for 10 seconds
#define BTYPE_SHRINK  7   // shrinks paddle by a third for 8 seconds

// ── Game state ────────────────────────────────────────────────────────────
static uint8_t brk_bricks[BRK_ROWS][BRK_COLS];
static int     brk_bricksLeft;

#define BRK_MAX_BALLS 3
struct BrkBall {
  float x, y, vx, vy;
  bool  active;
  float trailX[8], trailY[8];
  int   trailHead;
};
static BrkBall brk_balls[BRK_MAX_BALLS];

static float brk_px;          // paddle left edge
static int   brk_pw;          // paddle width
static bool  brk_launched;
static int   brk_lives;
static int   brk_score;
static int   brk_hiScore;
static int   brk_level;
static int   brk_combo;       // consecutive brick hits
static int   brk_mult;        // score multiplier 1–8
static bool  brk_speedBoost;  // B toggles fast paddle speed

// Shield (safety net at bottom)
static bool  brk_shieldActive;
static int   brk_shieldTimer;
#define BRK_SHIELD_FRAMES 420

// Wide paddle timer
static int   brk_widenFrames;

// Turbo (ball speed boost) timer — frames at 14ms/frame ≈ 714 frames = 10s
static int   brk_turboFrames;
#define BRK_TURBO_FRAMES  714
#define BRK_TURBO_SPEED_MUL 1.55f   // multiplier applied once on activation

// Shrink paddle timer — frames at 14ms/frame ≈ 571 frames = 8s
static int   brk_shrinkFrames;
#define BRK_SHRINK_FRAMES 571

// Particles
#define BRK_MAX_PART 80
struct BrkPart { float x, y, vx, vy; uint8_t life; uint16_t col; bool active; };
static BrkPart brk_parts[BRK_MAX_PART];

// Hard-brick flash counter per cell
static int brk_hardFlash[BRK_ROWS][BRK_COLS];

// ── Forward declarations ──────────────────────────────────────────────────
static void brkEraseBall(BrkBall& b);
static void brkDrawBall(BrkBall& b);
static bool brkCheckBricks(BrkBall& ball);

// ── Helpers ───────────────────────────────────────────────────────────────
static inline int brkBX(int col) { return BRK_X0 + col*(BRK_W+BRK_GAP_X); }
static inline int brkBY(int row) { return BRK_Y0 + row*(BRK_H+BRK_GAP_Y); }

static int brkReadHi() {
  int hi=((int)EEPROM.read(EEPROM_BRK_HI)<<8)|EEPROM.read(EEPROM_BRK_HI+1);
  return (hi>=0&&hi<9999999)?hi:0;
}
static void brkSaveHi(int s) {
  EEPROM.write(EEPROM_BRK_HI,  (s>>8)&0xFF);
  EEPROM.write(EEPROM_BRK_HI+1, s    &0xFF);
  EEPROM.commit();
}

// ── Sound ─────────────────────────────────────────────────────────────────
static void sndBrick()     { beepTone(500, 14); }
static void sndHardBrick() { beepTone(320,20); beepTone(420,14); }
static void sndWall()      { beepTone(260, 18); }
static void sndPaddle()    { beepTone(380, 18); }
static void sndLostLife()  { beepTone(220,60); beepTone(150,80); beepTone(100,120); }
static void sndPowerup()   { beepTone(600,22); beepTone(900,22); beepTone(1200,40); }
static void sndMulti()     { beepTone(800,18); beepTone(1100,18); beepTone(1500,35); }
static void sndBomb()      { beepTone(300,30); beepTone(200,40); beepTone(150,50); }
static void sndTurbo()     { beepTone(400,12); beepTone(600,12); beepTone(900,12); beepTone(1400,25); }
static void sndShrink()    { beepTone(700,20); beepTone(500,20); beepTone(300,20); beepTone(200,30); }
static void sndExit()      { beepTone(700,20); beepTone(500,20); beepTone(300,30); }
static void sndCombo()     { beepTone(400+brk_mult*80, 12); }
static void sndLevelUp()   {
  int n[]={400,550,700,900,1100,1400};
  for(int i=0;i<6;i++){beepTone(n[i],55);delay(8);}
}
static void sndGameOver()  { for(int f=500;f>=80;f-=25) beepTone(f,28); }

// ── Particles ─────────────────────────────────────────────────────────────
static void brkSpawnBurst(int cx, int cy, uint16_t col, int n=12) {
  int spawned=0;
  for(int i=0;i<BRK_MAX_PART&&spawned<n;i++){
    if(brk_parts[i].active) continue;
    float a=(float)random(6283)/1000.0f;
    float spd=1.2f+(float)random(30)*0.1f;
    brk_parts[i]={(float)cx,(float)cy,cosf(a)*spd,sinf(a)*spd,
                  (uint8_t)(16+random(16)),col,true};
    spawned++;
  }
}
static void brkUpdateParticles() {
  for(int i=0;i<BRK_MAX_PART;i++){
    if(!brk_parts[i].active) continue;
    display->drawPixel((int)brk_parts[i].x,(int)brk_parts[i].y,ARC_BLACK);
    brk_parts[i].x+=brk_parts[i].vx;
    brk_parts[i].y+=brk_parts[i].vy;
    brk_parts[i].life--;
    if(!brk_parts[i].life||
       brk_parts[i].x<0||brk_parts[i].x>SCREEN_WIDTH||
       brk_parts[i].y<BRK_PLAY_TOP||brk_parts[i].y>BRK_PLAY_BOTTOM){
      brk_parts[i].active=false;
    } else {
      uint16_t c=(brk_parts[i].life>10)?brk_parts[i].col:(brk_parts[i].col>>1)&0x7BEF;
      display->drawPixel((int)brk_parts[i].x,(int)brk_parts[i].y,c);
    }
  }
}

// ── Bomb: destroy 3×3 region ──────────────────────────────────────────────
static void brkBombBlast(int cr, int cc) {
  sndBomb();
  for(int dr=-1;dr<=1;dr++) for(int dc=-1;dc<=1;dc++){
    int r2=cr+dr, c2=cc+dc;
    if(r2<0||r2>=BRK_ROWS||c2<0||c2>=BRK_COLS) continue;
    if(brk_bricks[r2][c2]==BTYPE_EMPTY) continue;
    brkSpawnBurst(brkBX(c2)+BRK_W/2, brkBY(r2)+BRK_H/2, BRICK_COL[r2], 6);
    display->fillRect(brkBX(c2),brkBY(r2),BRK_W,BRK_H,ARC_BLACK);
    brk_bricks[r2][c2]=BTYPE_EMPTY;
    brk_bricksLeft--;
    brk_score+=BRICK_PTS[r2]*brk_mult;
    if(brk_score>brk_hiScore) brk_hiScore=brk_score;
  }
}

// ── Add a ball (multiball) ────────────────────────────────────────────────
static void brkAddBall(float x, float y, float vx, float vy) {
  for(int i=0;i<BRK_MAX_BALLS;i++){
    if(!brk_balls[i].active){
      brk_balls[i].x=x; brk_balls[i].y=y;
      brk_balls[i].vx=vx; brk_balls[i].vy=vy;
      brk_balls[i].active=true;
      brk_balls[i].trailHead=0;
      for(int t=0;t<8;t++){brk_balls[i].trailX[t]=x; brk_balls[i].trailY[t]=y;}
      return;
    }
  }
}

// ── Status bar ────────────────────────────────────────────────────────────
static void brkDrawStatus() {
  display->fillRect(0,0,SCREEN_WIDTH,BRK_STATUS_H,0x0010);
  display->drawFastHLine(0,BRK_STATUS_H-1,SCREEN_WIDTH,0x39E7);
  display->setTextSize(1);
  char buf[32];

  display->setTextColor(ARC_WHITE);
  sprintf(buf,"SCR:%d",brk_score);
  display->setCursor(4,6); display->print(buf);

  // Multiplier — colour shifts with combo
  static const uint16_t multCols[]={ARC_WHITE,ARC_CYAN,ARC_GREEN,ARC_YELLOW,
                                     ARC_ORANGE,ARC_RED,ARC_MAGENTA,0xFFFF};
  display->setTextColor(multCols[min(brk_mult-1,7)]);
  sprintf(buf,"x%d",brk_mult);
  display->setCursor(140,6); display->print(buf);

  display->setTextColor(0xAD75);
  sprintf(buf,"HI:%d",brk_hiScore);
  display->setCursor(190,6); display->print(buf);

  display->setTextColor(ARC_CYAN);
  sprintf(buf,"LVL:%d",brk_level);
  display->setCursor(330,6); display->print(buf);

  display->setCursor(390,6);
  for(int i=0;i<3;i++){
    display->setTextColor(i<brk_lives?ARC_RED:0x39E7);
    display->print("* ");
  }
  if(brk_shieldActive){
    display->setTextColor(ARC_CYAN);
    display->setCursor(454,6); display->print("S");
  }
}

// ── Draw one brick ────────────────────────────────────────────────────────
static void brkDrawBrick(int r, int c) {
  int x=brkBX(c), y=brkBY(r);
  if(brk_bricks[r][c]==BTYPE_EMPTY){
    display->fillRect(x,y,BRK_W,BRK_H,ARC_BLACK);
    return;
  }
  uint16_t col=BRICK_COL[r];
  uint16_t dim=(col>>1)&0x7BEF;

  if(brk_hardFlash[r][c]>0){ col=ARC_WHITE; brk_hardFlash[r][c]--; }

  display->fillRect(x,y,BRK_W,BRK_H,dim);
  // Neon border
  display->drawFastHLine(x,y,BRK_W,col);
  display->drawFastHLine(x,y+BRK_H-1,BRK_W,col);
  display->drawFastVLine(x,y,BRK_H,col);
  display->drawFastVLine(x+BRK_W-1,y,BRK_H,col);

  if(brk_bricks[r][c]==BTYPE_HARD){
    display->drawRect(x+2,y+2,BRK_W-4,BRK_H-4,col);
  } else if(brk_bricks[r][c]==BTYPE_BOMB){
    display->setTextColor(ARC_ORANGE); display->setTextSize(1);
    display->setCursor(x+BRK_W/2-3,y+BRK_H/2-4); display->print("B");
  } else if(brk_bricks[r][c]==BTYPE_MULTI){
    display->setTextColor(ARC_YELLOW); display->setTextSize(1);
    display->setCursor(x+BRK_W/2-3,y+BRK_H/2-4); display->print("M");
  } else if(brk_bricks[r][c]==BTYPE_SHIELD){
    display->setTextColor(ARC_CYAN); display->setTextSize(1);
    display->setCursor(x+BRK_W/2-3,y+BRK_H/2-4); display->print("S");
  } else if(brk_bricks[r][c]==BTYPE_TURBO){
    // Turbo: bright red fill + "T"
    display->fillRect(x+1,y+1,BRK_W-2,BRK_H-2,0xF800);
    display->setTextColor(ARC_WHITE); display->setTextSize(1);
    display->setCursor(x+BRK_W/2-3,y+BRK_H/2-4); display->print("T");
  } else if(brk_bricks[r][c]==BTYPE_SHRINK){
    // Shrink: deep purple fill + "<>"
    display->fillRect(x+1,y+1,BRK_W-2,BRK_H-2,0x600F);
    display->setTextColor(ARC_WHITE); display->setTextSize(1);
    display->setCursor(x+BRK_W/2-6,y+BRK_H/2-4); display->print("<>");
  }
}

static void brkDrawAllBricks() {
  for(int r=0;r<BRK_ROWS;r++) for(int c=0;c<BRK_COLS;c++) brkDrawBrick(r,c);
}

// ── Paddle ────────────────────────────────────────────────────────────────
static void brkDrawPaddle(uint16_t col) {
  int px=(int)brk_px;
  if(col==ARC_BLACK){
    display->fillRect(px-1,BRK_PADDLE_Y-1,brk_pw+2,BRK_PADDLE_H+2,ARC_BLACK);
    return;
  }
  display->fillRect(px,BRK_PADDLE_Y,brk_pw,BRK_PADDLE_H,(col>>1)&0x7BEF);
  display->drawRect(px,BRK_PADDLE_Y,brk_pw,BRK_PADDLE_H,col);
  // Inner glow line
  display->drawFastHLine(px+2,BRK_PADDLE_Y+BRK_PADDLE_H/2,brk_pw-4,col);
}

// ── Shield (safety net at bottom, just below paddle) ──────────────────────
static void brkDrawShield(bool erase=false) {
  uint16_t col=erase?ARC_BLACK:ARC_CYAN;
  if(!erase && brk_shieldTimer<120 && (brk_shieldTimer/8)%2==0) col=0x39E7;
  display->drawFastHLine(BRK_PLAY_LEFT, BRK_PLAY_BOTTOM-1, BRK_PLAY_RIGHT-BRK_PLAY_LEFT, col);
  display->drawFastHLine(BRK_PLAY_LEFT, BRK_PLAY_BOTTOM,   BRK_PLAY_RIGHT-BRK_PLAY_LEFT, col);
}

// ── Ball ──────────────────────────────────────────────────────────────────
static void brkEraseBall(BrkBall& b) {
  for(int t=0;t<8;t++) display->drawPixel((int)b.trailX[t],(int)b.trailY[t],ARC_BLACK);
  display->fillCircle((int)b.x,(int)b.y,BRK_BALL_R+1,ARC_BLACK);
}

static void brkDrawBall(BrkBall& b) {
  // Neon trail — cyan fade
  for(int t=0;t<7;t++){
    int idx=(b.trailHead+t)%8;
    if(b.trailY[idx]>BRK_PLAY_TOP&&b.trailY[idx]<BRK_PLAY_BOTTOM){
      uint8_t bright=(uint8_t)(t*20);
      uint16_t tc=((uint16_t)(bright>>3)<<6)|((uint16_t)(bright>>2));
      display->drawPixel((int)b.trailX[idx],(int)b.trailY[idx],tc);
    }
  }
  display->fillCircle((int)b.x,(int)b.y,BRK_BALL_R,ARC_WHITE);
  display->fillCircle((int)b.x,(int)b.y,BRK_BALL_R-2,ARC_CYAN);
  display->drawPixel((int)b.x-1,(int)b.y-1,ARC_WHITE);
}

// ── Borders ───────────────────────────────────────────────────────────────
static void brkDrawBorders() {
  display->drawFastHLine(BRK_PLAY_LEFT, BRK_PLAY_TOP, BRK_PLAY_RIGHT-BRK_PLAY_LEFT, 0x39E7);
  display->drawFastVLine(BRK_PLAY_LEFT, BRK_PLAY_TOP, BRK_PLAY_BOTTOM-BRK_PLAY_TOP, 0x39E7);
  display->drawFastVLine(BRK_PLAY_RIGHT,BRK_PLAY_TOP, BRK_PLAY_BOTTOM-BRK_PLAY_TOP, 0x39E7);
  // No bottom wall — ball falls off the bottom = life lost
}

// ── Overlay ───────────────────────────────────────────────────────────────
static void brkOverlay(const char* big, const char* sm1, const char* sm2, uint16_t col) {
  int mx=SCREEN_WIDTH/2, my=SCREEN_HEIGHT/2;
  display->fillRect(40,my-50,SCREEN_WIDTH-80,100,0x0020);
  display->drawRect(40,my-50,SCREEN_WIDTH-80,100,col);
  display->setTextSize(2); display->setTextColor(col);
  display->setCursor(mx-(int)strlen(big)*6, my-36); display->print(big);
  display->setTextSize(1); display->setTextColor(ARC_WHITE);
  display->setCursor(mx-(int)strlen(sm1)*3, my-8);  display->print(sm1);
  display->setTextColor(0xAD75);
  display->setCursor(mx-(int)strlen(sm2)*3, my+8);  display->print(sm2);
}

// ── Full screen redraw ────────────────────────────────────────────────────
static void brkFullDraw() {
  display->fillScreen(ARC_BLACK);
  brkDrawStatus();
  brkDrawAllBricks();
  brkDrawBorders();
  brkDrawPaddle(ARC_CYAN);
  for(int i=0;i<BRK_MAX_BALLS;i++) if(brk_balls[i].active) brkDrawBall(brk_balls[i]);
  if(brk_shieldActive) brkDrawShield();
}

// ── Brick collision for one ball ──────────────────────────────────────────
static bool brkCheckBricks(BrkBall& ball) {
  for(int r=0;r<BRK_ROWS;r++) for(int c=0;c<BRK_COLS;c++){
    if(brk_bricks[r][c]==BTYPE_EMPTY) continue;
    int bx=brkBX(c), by=brkBY(r);
    if(ball.x+BRK_BALL_R<bx||ball.x-BRK_BALL_R>bx+BRK_W) continue;
    if(ball.y+BRK_BALL_R<by||ball.y-BRK_BALL_R>by+BRK_H) continue;

    float ol=(ball.x+BRK_BALL_R)-bx;
    float or_=(bx+BRK_W)-(ball.x-BRK_BALL_R);
    float ot=(ball.y+BRK_BALL_R)-by;
    float ob=(by+BRK_H)-(ball.y-BRK_BALL_R);
    if(min(ol,or_)<min(ot,ob)) ball.vx=-ball.vx;
    else                       ball.vy=-ball.vy;

    uint8_t type=brk_bricks[r][c];

    if(type==BTYPE_HARD){
      brk_bricks[r][c]=BTYPE_NORMAL;
      brk_hardFlash[r][c]=8;
      brkSpawnBurst(bx+BRK_W/2, by+BRK_H/2, ARC_WHITE, 5);
      sndHardBrick();
    } else {
      brk_bricks[r][c]=BTYPE_EMPTY;
      brk_bricksLeft--;
      brk_combo++;
      brk_mult=min(1+(brk_combo/4), 8);
      brk_score+=BRICK_PTS[r]*brk_mult;
      if(brk_score>brk_hiScore) brk_hiScore=brk_score;
      brkSpawnBurst(bx+BRK_W/2, by+BRK_H/2, BRICK_COL[r]);
      display->fillRect(bx,by,BRK_W,BRK_H,ARC_BLACK);
      brkDrawStatus();
      sndBrick(); sndCombo();

      if(type==BTYPE_BOMB){
        brkBombBlast(r,c);
      } else if(type==BTYPE_MULTI){
        sndMulti();
        brkAddBall(ball.x, ball.y, ball.vx, -ball.vy);
      } else if(type==BTYPE_SHIELD){
        sndPowerup();
        brk_shieldActive=true;
        brk_shieldTimer=BRK_SHIELD_FRAMES;
        brkDrawShield();
      } else if(type==BTYPE_TURBO){
        sndTurbo();
        brk_turboFrames=BRK_TURBO_FRAMES;
        // Boost all active balls
        for(int bi=0;bi<BRK_MAX_BALLS;bi++){
          if(!brk_balls[bi].active) continue;
          float s=sqrtf(brk_balls[bi].vx*brk_balls[bi].vx+brk_balls[bi].vy*brk_balls[bi].vy);
          float maxSpd=3.0f+brk_level*0.15f;
          float newSpd=min(s*BRK_TURBO_SPEED_MUL, maxSpd*2.2f);
          if(s>0.01f){ brk_balls[bi].vx=brk_balls[bi].vx/s*newSpd; brk_balls[bi].vy=brk_balls[bi].vy/s*newSpd; }
        }
      } else if(type==BTYPE_SHRINK){
        sndShrink();
        brk_shrinkFrames=BRK_SHRINK_FRAMES;
        // Paddle width will be applied in the main loop timer block
      }
    }
    return true;
  }
  return false;
}

// ── Level / game init ─────────────────────────────────────────────────────
// ── Sprinkle special bricks over a filled grid ───────────────────────────
// Call after filling brk_bricks with BTYPE_NORMAL/BTYPE_EMPTY.
// lv is 1-based level; higher levels get more specials.
static void brkAddSpecials(int lv) {
  // Rates scale from level 1 (low) to level 20 (high)
  int hardPct   = 4  + lv/2;          //  4% … 14%
  int bombPct   = 3  + lv/4;          //  3% …  8%
  int multiPct  = 3  + lv/4;          //  3% …  8%
  int shieldPct = 2  + lv/6;          //  2% …  5%
  int turboPct  = 2  + lv/5;          //  2% …  6%
  int shrinkPct = 2  + lv/5;          //  2% …  6%
  for(int r=0;r<BRK_ROWS;r++) for(int c=0;c<BRK_COLS;c++){
    if(brk_bricks[r][c]!=BTYPE_NORMAL) continue;
    int rng=random(100);
    if     (rng < hardPct)                                         brk_bricks[r][c]=BTYPE_HARD;
    else if(rng < hardPct+bombPct)                                 brk_bricks[r][c]=BTYPE_BOMB;
    else if(rng < hardPct+bombPct+multiPct)                        brk_bricks[r][c]=BTYPE_MULTI;
    else if(rng < hardPct+bombPct+multiPct+shieldPct)              brk_bricks[r][c]=BTYPE_SHIELD;
    else if(rng < hardPct+bombPct+multiPct+shieldPct+turboPct)     brk_bricks[r][c]=BTYPE_TURBO;
    else if(rng < hardPct+bombPct+multiPct+shieldPct+turboPct+shrinkPct) brk_bricks[r][c]=BTYPE_SHRINK;
  }
}

// ── Layout builders — each fills brk_bricks then calls brkAddSpecials ────
// Helper: set one cell, keeping count
static void brkSetBrick(int r, int c, uint8_t t=BTYPE_NORMAL){
  if(r<0||r>=BRK_ROWS||c<0||c>=BRK_COLS) return;
  bool wasEmpty=(brk_bricks[r][c]==BTYPE_EMPTY);
  bool nowEmpty=(t==BTYPE_EMPTY);
  brk_bricks[r][c]=t;
  if(wasEmpty&&!nowEmpty) brk_bricksLeft++;
  if(!wasEmpty&&nowEmpty) brk_bricksLeft--;
}

static void brkLayoutFull()   { // solid grid
  for(int r=0;r<BRK_ROWS;r++) for(int c=0;c<BRK_COLS;c++){
    brk_bricks[r][c]=BTYPE_NORMAL; brk_bricksLeft++;
  }
}
static void brkLayoutChecker(){ // checkerboard
  for(int r=0;r<BRK_ROWS;r++) for(int c=0;c<BRK_COLS;c++){
    if((r+c)%2==0){ brk_bricks[r][c]=BTYPE_NORMAL; brk_bricksLeft++; }
    else           brk_bricks[r][c]=BTYPE_EMPTY;
  }
}
static void brkLayoutStripes(){ // vertical stripes (every other column)
  for(int r=0;r<BRK_ROWS;r++) for(int c=0;c<BRK_COLS;c++){
    if(c%2==0){ brk_bricks[r][c]=BTYPE_NORMAL; brk_bricksLeft++; }
    else       brk_bricks[r][c]=BTYPE_EMPTY;
  }
}
static void brkLayoutHStripes(){ // horizontal stripes (every other row)
  for(int r=0;r<BRK_ROWS;r++) for(int c=0;c<BRK_COLS;c++){
    if(r%2==0){ brk_bricks[r][c]=BTYPE_NORMAL; brk_bricksLeft++; }
    else       brk_bricks[r][c]=BTYPE_EMPTY;
  }
}
static void brkLayoutDiamond(){ // diamond / rhombus shape
  int cr=BRK_ROWS/2, cc=BRK_COLS/2;
  for(int r=0;r<BRK_ROWS;r++) for(int c=0;c<BRK_COLS;c++){
    if(abs(r-cr)*2+abs(c-cc)<=BRK_COLS/2+1){
      brk_bricks[r][c]=BTYPE_NORMAL; brk_bricksLeft++;
    } else brk_bricks[r][c]=BTYPE_EMPTY;
  }
}
static void brkLayoutX(){ // X cross
  for(int r=0;r<BRK_ROWS;r++) for(int c=0;c<BRK_COLS;c++){
    // Two diagonals
    float fc=((float)r/(BRK_ROWS-1))*(BRK_COLS-1);
    float fc2=(BRK_COLS-1)-fc;
    if(abs(c-fc)<1.5f||abs(c-fc2)<1.5f){
      brk_bricks[r][c]=BTYPE_NORMAL; brk_bricksLeft++;
    } else brk_bricks[r][c]=BTYPE_EMPTY;
  }
}
static void brkLayoutBorder(){ // outer border + inner cross
  for(int r=0;r<BRK_ROWS;r++) for(int c=0;c<BRK_COLS;c++){
    bool border=(r==0||r==BRK_ROWS-1||c==0||c==BRK_COLS-1);
    bool cross=(r==BRK_ROWS/2||c==BRK_COLS/2);
    if(border||cross){ brk_bricks[r][c]=BTYPE_NORMAL; brk_bricksLeft++; }
    else              brk_bricks[r][c]=BTYPE_EMPTY;
  }
}
static void brkLayoutTunnel(){ // two horizontal tunnels (empty rows 2 and 4)
  for(int r=0;r<BRK_ROWS;r++) for(int c=0;c<BRK_COLS;c++){
    if(r==2||r==4) brk_bricks[r][c]=BTYPE_EMPTY;
    else { brk_bricks[r][c]=BTYPE_NORMAL; brk_bricksLeft++; }
  }
}
static void brkLayoutPyramid(){ // right-pointing triangle from left
  for(int r=0;r<BRK_ROWS;r++) for(int c=0;c<BRK_COLS;c++){
    int minC=abs(r-BRK_ROWS/2); // fewer bricks near top/bottom
    if(c>=minC){ brk_bricks[r][c]=BTYPE_NORMAL; brk_bricksLeft++; }
    else        brk_bricks[r][c]=BTYPE_EMPTY;
  }
}
static void brkLayoutZigzag(){ // zigzag rows
  for(int r=0;r<BRK_ROWS;r++) for(int c=0;c<BRK_COLS;c++){
    int offset=(r%2)*2;
    if((c+offset)%4<3){ brk_bricks[r][c]=BTYPE_NORMAL; brk_bricksLeft++; }
    else               brk_bricks[r][c]=BTYPE_EMPTY;
  }
}
static void brkLayoutFort(){ // fortress — solid outer, hollow inner
  for(int r=0;r<BRK_ROWS;r++) for(int c=0;c<BRK_COLS;c++){
    bool wall=(r<=1||r>=BRK_ROWS-2||c<=1||c>=BRK_COLS-2);
    if(wall){ brk_bricks[r][c]=BTYPE_NORMAL; brk_bricksLeft++; }
    else    brk_bricks[r][c]=BTYPE_EMPTY;
  }
}
static void brkLayoutWaves(){ // sine-wave shaped rows
  for(int r=0;r<BRK_ROWS;r++) for(int c=0;c<BRK_COLS;c++){
    float wave=sinf((float)c*0.6f+(float)r*0.8f);
    if(wave>-0.3f){ brk_bricks[r][c]=BTYPE_NORMAL; brk_bricksLeft++; }
    else           brk_bricks[r][c]=BTYPE_EMPTY;
  }
}
static void brkLayoutArrows(){ // right-pointing arrows
  for(int r=0;r<BRK_ROWS;r++) for(int c=0;c<BRK_COLS;c++){
    int dc=abs(r-BRK_ROWS/2);
    if(c>=dc&&c<dc+5||c>=dc+6&&c<dc+11){
      brk_bricks[r][c]=BTYPE_NORMAL; brk_bricksLeft++;
    } else brk_bricks[r][c]=BTYPE_EMPTY;
  }
}
static void brkLayoutDots(){ // 2x2 dot clusters
  for(int r=0;r<BRK_ROWS;r++) for(int c=0;c<BRK_COLS;c++){
    if(r%3<2&&c%3<2){ brk_bricks[r][c]=BTYPE_NORMAL; brk_bricksLeft++; }
    else             brk_bricks[r][c]=BTYPE_EMPTY;
  }
}
static void brkLayoutCross(){ // plus sign in centre
  for(int r=0;r<BRK_ROWS;r++) for(int c=0;c<BRK_COLS;c++){
    bool inCross=(r>=BRK_ROWS/2-1&&r<=BRK_ROWS/2+1)||(c>=BRK_COLS/2-2&&c<=BRK_COLS/2+2);
    if(inCross){ brk_bricks[r][c]=BTYPE_NORMAL; brk_bricksLeft++; }
    else        brk_bricks[r][c]=BTYPE_EMPTY;
  }
}
static void brkLayoutSpiral(){ // spiral from outside in
  for(int r=0;r<BRK_ROWS;r++) for(int c=0;c<BRK_COLS;c++){
    int shell=min(min(r,BRK_ROWS-1-r),min(c,BRK_COLS-1-c));
    if(shell%2==0){ brk_bricks[r][c]=BTYPE_NORMAL; brk_bricksLeft++; }
    else           brk_bricks[r][c]=BTYPE_EMPTY;
  }
}
static void brkLayoutInvader(){ // space-invader silhouette, tiled
  // 6-wide pattern, repeated twice across 12 cols
  static const uint8_t pat[7][6]={
    {0,0,1,1,0,0},
    {0,1,1,1,1,0},
    {1,1,0,0,1,1},
    {1,1,1,1,1,1},
    {0,1,0,0,1,0},
    {1,0,1,1,0,1},
    {0,1,0,0,1,0},
  };
  for(int r=0;r<BRK_ROWS;r++) for(int c=0;c<BRK_COLS;c++){
    if(pat[r][c%6]){ brk_bricks[r][c]=BTYPE_NORMAL; brk_bricksLeft++; }
    else            brk_bricks[r][c]=BTYPE_EMPTY;
  }
}
static void brkLayoutStagger(){ // offset rows (brick-wall pattern)
  for(int r=0;r<BRK_ROWS;r++) for(int c=0;c<BRK_COLS;c++){
    int shift=(r%2)?1:0;
    if((c+shift)%3!=2){ brk_bricks[r][c]=BTYPE_NORMAL; brk_bricksLeft++; }
    else               brk_bricks[r][c]=BTYPE_EMPTY;
  }
}
static void brkLayoutHourglass(){ // wide at top/bottom, narrow in middle
  for(int r=0;r<BRK_ROWS;r++) for(int c=0;c<BRK_COLS;c++){
    int dist=abs(r-BRK_ROWS/2); // 0=centre, 3=edge
    int margin=BRK_COLS/2-dist*2;
    if(margin<0) margin=0;
    if(c>=margin&&c<BRK_COLS-margin){ brk_bricks[r][c]=BTYPE_NORMAL; brk_bricksLeft++; }
    else                              brk_bricks[r][c]=BTYPE_EMPTY;
  }
}
static void brkLayoutGauntlet(){ // almost full, 4 small gaps
  for(int r=0;r<BRK_ROWS;r++) for(int c=0;c<BRK_COLS;c++){
    bool gap=(c==2&&r>=2&&r<=4)||(c==5&&r>=0&&r<=2)||
             (c==8&&r>=4&&r<=6)||(c==10&&r>=2&&r<=4);
    if(!gap){ brk_bricks[r][c]=BTYPE_NORMAL; brk_bricksLeft++; }
    else    brk_bricks[r][c]=BTYPE_EMPTY;
  }
}

// ── Main level initialiser ────────────────────────────────────────────────
static void brkInitLevel() {
  brk_bricksLeft=0;
  memset(brk_hardFlash,0,sizeof(brk_hardFlash));
  for(int i=0;i<BRK_MAX_PART;i++) brk_parts[i].active=false;
  // Clear grid
  memset(brk_bricks, BTYPE_EMPTY, sizeof(brk_bricks));

  // Choose colour theme (one per 5-level block)
  int theme=min((brk_level-1)/5, 3);
  for(int r=0;r<BRK_ROWS;r++) BRICK_COL[r]=BRK_THEMES[theme][r];

  // Choose layout for this level (20 distinct patterns)
  switch(brk_level){
    case  1: brkLayoutFull();      break;
    case  2: brkLayoutChecker();   break;
    case  3: brkLayoutStripes();   break;
    case  4: brkLayoutHStripes();  break;
    case  5: brkLayoutDiamond();   break;
    case  6: brkLayoutFull();      break;  // theme 1 starts
    case  7: brkLayoutBorder();    break;
    case  8: brkLayoutTunnel();    break;
    case  9: brkLayoutPyramid();   break;
    case 10: brkLayoutZigzag();    break;
    case 11: brkLayoutFort();      break;  // theme 2 starts
    case 12: brkLayoutX();         break;
    case 13: brkLayoutWaves();     break;
    case 14: brkLayoutArrows();    break;
    case 15: brkLayoutDots();      break;
    case 16: brkLayoutCross();     break;  // theme 3 starts
    case 17: brkLayoutSpiral();    break;
    case 18: brkLayoutInvader();   break;
    case 19: brkLayoutStagger();   break;
    case 20: brkLayoutGauntlet();  break;
    default: brkLayoutFull();      break;
  }

  // Overlay specials (scales with level)
  brkAddSpecials(brk_level);

  // Reset balls — one ball sitting on paddle
  for(int i=0;i<BRK_MAX_BALLS;i++) brk_balls[i].active=false;
  float bx=brk_px+brk_pw/2.0f;
  float by=(float)(BRK_PADDLE_Y-BRK_BALL_R-2);
  float spd=3.0f+brk_level*0.15f;   // gentler speed ramp over 20 levels
  float ang=-0.4f;
  brkAddBall(bx, by, spd*sinf(ang), -spd*cosf(ang));

  brk_launched=false;
  brk_combo=0; brk_mult=1;
  brk_shieldActive=false; brk_shieldTimer=0;
  brk_widenFrames=0;
  brk_turboFrames=0;
  brk_shrinkFrames=0;
  brk_pw=BRK_PADDLE_W_BASE;
}

static void brkInitGame() {
  brk_lives=3; brk_score=0; brk_level=1;
  brk_hiScore=brkReadHi();
  brk_pw=BRK_PADDLE_W_BASE;
  brk_px=(SCREEN_WIDTH-brk_pw)/2.0f;
  brk_speedBoost=false;
  brkInitLevel();
}

// ── Wait for launch ───────────────────────────────────────────────────────
static void brkWaitLaunch(const char* msg) {
  display->setTextColor(ARC_YELLOW); display->setTextSize(2);
  display->setCursor(SCREEN_WIDTH/2-(int)strlen(msg)*6, BRK_PADDLE_Y-54);
  display->print(msg);
  display->setTextSize(1); display->setTextColor(ARC_LTGRAY);
  display->setCursor(SCREEN_WIDTH/2-66, BRK_PADDLE_Y-30);
  display->print("LEFT/RIGHT  FIRE to launch");

  while(!isBtnSet()){
    brkEraseBall(brk_balls[0]);
    brkDrawPaddle(ARC_BLACK);

    float spd=brk_speedBoost?9.0f:5.0f;
    if(isBtnLeft()){
      brk_px-=spd;
      if(brk_px<BRK_PLAY_LEFT) brk_px=BRK_PLAY_LEFT;
    }
    if(isBtnRight()){
      brk_px+=spd;
      if(brk_px+brk_pw>BRK_PLAY_RIGHT) brk_px=BRK_PLAY_RIGHT-brk_pw;
    }
    // Sync ball to paddle centre
    float bx=brk_px+brk_pw/2.0f;
    float by=(float)(BRK_PADDLE_Y-BRK_BALL_R-2);
    brk_balls[0].x=bx; brk_balls[0].y=by;
    for(int t=0;t<8;t++){brk_balls[0].trailX[t]=bx; brk_balls[0].trailY[t]=by;}

    brkDrawPaddle(ARC_CYAN);
    brkDrawBall(brk_balls[0]);
    delay(16);
  }
  while(isBtnSet()) delay(10);

  // Clear prompt — rect must cover both text lines (big at -54, small at -30)
  // plus a few px margin below the small text. Height 60 covers -60 to 0 (paddle top).
  display->fillRect(0, BRK_PADDLE_Y-60, SCREEN_WIDTH, 60, ARC_BLACK);
  brkDrawAllBricks(); brkDrawBorders();
  if(brk_shieldActive) brkDrawShield();
  brk_launched=true;
}

// ══════════════════════════════════════════════════════════════════════════
//  MAIN ENTRY POINT
// ══════════════════════════════════════════════════════════════════════════
void runBreakout() {
  brkInitGame();
  brkFullDraw();
  brkWaitLaunch("  READY!");

  const int FRAME_MS=14;
  unsigned long lastFrame=0;
  int  exitHoldCt=0;
  bool lastSetState=false;
  bool bWasHeld=false;        // for B toggle detection
  float prevPx=brk_px;

  while(true){
    unsigned long now=millis();
    if(now-lastFrame<FRAME_MS) continue;
    lastFrame=now;

    // ── Exit ──────────────────────────────────────────────────────────
    if(isBtnSet()&&isBtnB()){
      if(++exitHoldCt>=4){ sndExit(); arcadeExitToMain=true; display->fillScreen(ARC_BLACK); return; }
    } else { exitHoldCt=0; }

    // ── Pause ─────────────────────────────────────────────────────────
    bool setNow=isBtnSet()&&!isBtnB();
    if(setNow&&!lastSetState&&brk_launched){
      brkOverlay("PAUSED","FIRE to Resume","A+B  Main Menu",ARC_YELLOW);
      delay(200);
      while(true){
        if(isBtnSet()&&!isBtnB()){delay(200);break;}
        if(isBtnSet()&&isBtnB()){arcadeExitToMain=true; display->fillScreen(ARC_BLACK);return;}
        delay(40);
      }
      brkFullDraw();
    }
    lastSetState=setNow;

    // ── B button toggles speed boost ─────────────────────────────────
    bool bNow = isBtnB() && !isBtnSet();
    if(bNow && !bWasHeld) {
      brk_speedBoost = !brk_speedBoost;
      beepTone(brk_speedBoost ? 900 : 500, 15);
    }
    bWasHeld = bNow;

    // ── Paddle movement (LEFT/RIGHT) ──────────────────────────────────
    prevPx=brk_px;
    brkDrawPaddle(ARC_BLACK);

    float padSpd = brk_speedBoost ? 10.0f : 5.5f+brk_level*0.2f;
    if(isBtnLeft()){
      brk_px-=padSpd;
      if(brk_px<BRK_PLAY_LEFT) brk_px=BRK_PLAY_LEFT;
    }
    if(isBtnRight()){
      brk_px+=padSpd;
      if(brk_px+brk_pw>BRK_PLAY_RIGHT) brk_px=BRK_PLAY_RIGHT-brk_pw;
    }
    if(!brk_launched){
      brk_balls[0].x=brk_px+brk_pw/2.0f;
      for(int t=0;t<8;t++){brk_balls[0].trailX[t]=brk_balls[0].x; brk_balls[0].trailY[t]=brk_balls[0].y;}
    }

    // Wide paddle timer
    if(brk_widenFrames>0){
      brk_pw=BRK_PADDLE_W_BASE+40;
      if(--brk_widenFrames==0) brk_pw=BRK_PADDLE_W_BASE;
    }

    // Turbo timer — when active ball speed is already boosted; just count down
    if(brk_turboFrames>0){
      if(--brk_turboFrames==0){
        // Restore normal speed on all active balls
        float baseSpd=3.0f+brk_level*0.15f;
        for(int bi=0;bi<BRK_MAX_BALLS;bi++){
          if(!brk_balls[bi].active) continue;
          float s=sqrtf(brk_balls[bi].vx*brk_balls[bi].vx+brk_balls[bi].vy*brk_balls[bi].vy);
          if(s>baseSpd*1.2f && s>0.01f){
            brk_balls[bi].vx=brk_balls[bi].vx/s*baseSpd;
            brk_balls[bi].vy=brk_balls[bi].vy/s*baseSpd;
          }
        }
      }
    }

    // Shrink timer — paddle width is set here each frame
    if(brk_shrinkFrames>0){
      brk_pw=max(BRK_PADDLE_W_BASE/3, BRK_PADDLE_W_BASE*2/3);  // shrink to 2/3
      if(--brk_shrinkFrames==0) brk_pw=BRK_PADDLE_W_BASE;
    }

    // Pick paddle colour: shrink=red, turbo=magenta, widen=orange, speedBoost=yellow, normal=cyan
    uint16_t padCol = brk_shrinkFrames>0 ? ARC_RED :
                      brk_turboFrames>0  ? 0xF81F  :
                      brk_widenFrames>0  ? ARC_ORANGE :
                      brk_speedBoost     ? ARC_YELLOW : ARC_CYAN;
    brkDrawPaddle(padCol);

    // Shield timer
    if(brk_shieldActive){
      brkDrawShield();
      if(--brk_shieldTimer<=0){brkDrawShield(true);brk_shieldActive=false;}
    }

    if(!brk_launched){ brkDrawBall(brk_balls[0]); continue; }

    // ── Update all balls ───────────────────────────────────────────────
    int activeBalls=0;
    for(int bi=0;bi<BRK_MAX_BALLS;bi++){
      BrkBall& b=brk_balls[bi];
      if(!b.active) continue;
      activeBalls++;

      // Erase old trail tip + ball
      int oldIdx=b.trailHead%8;
      display->drawPixel((int)b.trailX[oldIdx],(int)b.trailY[oldIdx],ARC_BLACK);
      brkEraseBall(b);

      b.trailX[b.trailHead]=b.x; b.trailY[b.trailHead]=b.y;
      b.trailHead=(b.trailHead+1)%8;

      b.x+=b.vx; b.y+=b.vy;

      // ── Wall bounces ─────────────────────────────────────────────
      if(b.x-BRK_BALL_R<=BRK_PLAY_LEFT){
        b.x=BRK_PLAY_LEFT+BRK_BALL_R+0.5f; b.vx=fabsf(b.vx); sndWall();
      }
      if(b.x+BRK_BALL_R>=BRK_PLAY_RIGHT){
        b.x=BRK_PLAY_RIGHT-BRK_BALL_R-0.5f; b.vx=-fabsf(b.vx); sndWall();
      }
      if(b.y-BRK_BALL_R<=BRK_PLAY_TOP){
        b.y=BRK_PLAY_TOP+BRK_BALL_R+0.5f; b.vy=fabsf(b.vy); sndWall();
      }

      // ── Shield catches ball at bottom ─────────────────────────────
      if(brk_shieldActive && b.y+BRK_BALL_R>=BRK_PLAY_BOTTOM-2){
        b.y=BRK_PLAY_BOTTOM-2-BRK_BALL_R; b.vy=-fabsf(b.vy);
        sndWall();
        // Shield consumed by first save
        brkDrawShield(true);
        brk_shieldActive=false;
      }

      // ── Paddle collision ──────────────────────────────────────────
      if(b.vy>0 &&
         b.y+BRK_BALL_R>=BRK_PADDLE_Y &&
         b.y-BRK_BALL_R<=BRK_PADDLE_Y+BRK_PADDLE_H &&
         b.x+BRK_BALL_R>=brk_px &&
         b.x-BRK_BALL_R<=brk_px+brk_pw){

        b.y=BRK_PADDLE_Y-BRK_BALL_R-1;
        float t=(b.x-brk_px)/brk_pw;      // 0.0 left … 1.0 right
        float angleDeg=-55.0f+t*110.0f;
        // Spin: paddle movement adds up to ±15°
        float spin=(brk_px-prevPx)*1.2f;
        spin=constrain(spin,-15.0f,15.0f);
        angleDeg+=spin;
        angleDeg=constrain(angleDeg,-70.0f,70.0f);
        float angleRad=angleDeg*(float)M_PI/180.0f;
        float spd=sqrtf(b.vx*b.vx+b.vy*b.vy);
        b.vx=spd*sinf(angleRad);
        b.vy=-spd*cosf(angleRad);
        brk_combo=0; brk_mult=1;
        brkDrawStatus();
        sndPaddle();
      }

      // ── Brick collisions ──────────────────────────────────────────
      brkCheckBricks(b);

      // ── Ball lost (past bottom) ───────────────────────────────────
      if(b.y-BRK_BALL_R>BRK_PLAY_BOTTOM){
        brkEraseBall(b);
        b.active=false;
        activeBalls--;
        // Count remaining balls
        int rem=0;
        for(int j=0;j<BRK_MAX_BALLS;j++) if(brk_balls[j].active) rem++;
        if(rem>0) continue;   // other balls still alive
        // All balls gone
        goto ball_lost;
      }

      brkDrawBall(b);
    } // end ball loop

    // Hard-brick flash update
    for(int r=0;r<BRK_ROWS;r++) for(int c=0;c<BRK_COLS;c++){
      if(brk_hardFlash[r][c]>0) brkDrawBrick(r,c);
    }
    brkUpdateParticles();

    // ── Level complete ─────────────────────────────────────────────────
    if(brk_bricksLeft<=0){
      if(brk_score>brkReadHi()) brkSaveHi(brk_score);
      sndLevelUp();
      for(int burst=0;burst<8;burst++)
        brkSpawnBurst(random(60,420),random(50,260),BRICK_COL[random(BRK_ROWS)],14);
      for(int f=0;f<50;f++){brkUpdateParticles();delay(16);}

      brk_level++;
      if(brk_level>20){
        brkOverlay("YOU WIN!","All levels cleared!","FIRE Again  B Menu",ARC_GREEN);
        while(true){
          if(isBtnSet()&&!isBtnB()){ brk_level=1; brkInitGame(); brkFullDraw(); brkWaitLaunch("READY!"); break; }
          if(isBtnB()&&!isBtnSet()){ display->fillScreen(ARC_BLACK); return; }
          delay(40);
        }
      } else {
        char lm[28]; sprintf(lm,"Level %d!",brk_level);
        brkOverlay("LEVEL CLEAR!",lm,"FIRE to continue",ARC_CYAN);
        delay(2200);
        brkInitLevel();
        brkFullDraw();
        brkWaitLaunch("Next Level!");
      }
      exitHoldCt=0; lastSetState=false; bWasHeld=false;
    }
    continue;  // normal frame end

    // ── All balls gone ─────────────────────────────────────────────────
    ball_lost: {
      sndLostLife();
      brk_lives--; brk_combo=0; brk_mult=1;
      brkDrawStatus();

      if(brk_lives<=0){
        if(brk_score>brkReadHi()) brkSaveHi(brk_score);
        sndGameOver();
        brkOverlay("GAME OVER","FIRE  Play Again","B  Menu",ARC_RED);
        while(true){
          if(isBtnSet()&&!isBtnB()){ brkInitGame(); brkFullDraw(); brkWaitLaunch("  READY!"); break; }
          if(isBtnB()&&!isBtnSet()){ display->fillScreen(ARC_BLACK); return; }
          delay(40);
        }
        exitHoldCt=0; lastSetState=false; bWasHeld=false; continue;
      }

      brkOverlay("","Ball lost!","FIRE to continue",ARC_ORANGE);
      display->setTextColor(ARC_RED); display->setTextSize(3);
      for(int i=0;i<brk_lives;i++){
        display->setCursor(170+i*38, SCREEN_HEIGHT/2+24); display->print("*");
      }
      delay(1400);

      for(int i=0;i<BRK_MAX_BALLS;i++) brk_balls[i].active=false;
      brk_px=(SCREEN_WIDTH-brk_pw)/2.0f;
      float bx2=brk_px+brk_pw/2.0f;
      float by2=(float)(BRK_PADDLE_Y-BRK_BALL_R-2);
      float spd2=3.0f+brk_level*0.15f;   // same speed ramp as level start — no penalty speed-up
      brkAddBall(bx2, by2, spd2*sinf(-0.4f), -spd2*cosf(-0.4f));
      brk_turboFrames=0; brk_shrinkFrames=0; brk_pw=BRK_PADDLE_W_BASE;
      brk_launched=false;

      brkFullDraw();
      brkWaitLaunch("Continue...");
      exitHoldCt=0; lastSetState=false; bWasHeld=false; continue;
    }
  } // end main while
}



// ══════════════════════════════════════════════════════════════════════════
//
//                         G Y R U S S
//
//  Faithful port of RetroArcade.ino Gyruss to Arduino_GFX 480×320.
//  All gameplay, sprite drawing, and logic matches the original exactly.
//  Only changes: display-> instead of display., scaled geometry constants,
//  beepTone() redirects to playTone(), appState exits → return.
//
// ══════════════════════════════════════════════════════════════════════════

// ── Screen geometry — scaled from 320×240 to 480×320 ─────────────────────
//   GYR_CX:       160 → 240    GYR_CY:      112 → 149
//   GYR_PLAYER_R:  98 → 130    GYR_FORM_R:   60 →  80
//   GYR_STAR_MAX: 118 → 157    GYR_HUD_Y:   228 → 300 (unused; HUD at top)

#define GYR_CX          240
#define GYR_CY          149
#define GYR_PLAYER_R    130
#define GYR_FORM_R       80
#define GYR_STAR_MAX    157

// ── Pool sizes ────────────────────────────────────────────────────────────
#define GYR_NUM_STARS      80
#define GYR_MAX_ENEMIES    24
#define GYR_MAX_PBULLETS    6
#define GYR_MAX_EBULLETS   14
#define GYR_MAX_PARTS      64
#define GYR_NUM_SATS        3
#define GYR_MAX_ASTEROIDS   4
#define GYR_MAX_LASERS      3

// ── Palette ───────────────────────────────────────────────────────────────
#define GYR_STAR_DIM    0x8410
#define GYR_STAR_MID    0xC618
#define GYR_STAR_BRT    0xFFFF
#define GYR_PLAYER_COL  0x07FF
#define GYR_BULLET_COL  0xFFE0
#define GYR_BULL2_COL   0x07FF
#define GYR_ENE_COL0    0x07E0
#define GYR_ENE_COL1    0xFFE0
#define GYR_ENE_COL2    0xF800
#define GYR_EBUL_COL    0xF81F
#define GYR_EXPL_A      0xFD20
#define GYR_EXPL_B      0xFFE0
#define GYR_SAT_COL     0x07FF
#define GYR_SUN_COL     0xFFE0
#define GYR_AST_COL     0x8C71
#define GYR_LASER_COL   0xF81F
#define GYR_BEAM_COL    0xFC10
#define GYR_HUD_LABEL   0xFB20
#define GYR_HUD_SCORE   0xFFE0
#define GYR_HUD_STAGE   0xFFFF
#define GYR_GOLD        0xFEA0

// ── EEPROM ────────────────────────────────────────────────────────────────
#define EEPROM_GYR_HI   6

// ── Weapon ────────────────────────────────────────────────────────────────
#define GYR_WPN_SINGLE  0
#define GYR_WPN_DOUBLE  1

// ── Spawn radius (just outside star field) ────────────────────────────────
static const float GYR_SPAWN_RADIUS = 168.0f;  // 135 * 480/320 * ~1.0 = ~168

// ══════════════════════════════════════════════════════════════════════════
//  DATA STRUCTURES  (identical to original)
// ══════════════════════════════════════════════════════════════════════════

struct GyrStar { float angle, r, speed; };

struct GyrPBullet {
  float angle, r, offset;
  bool  active;
};

struct GyrEBullet {
  float x, y, vx, vy;
  bool  active;
};

struct GyrEnemy {
  float   angle;
  float   r;
  float   formAngle;
  float   vAngle;
  float   entryT;
  bool    spiralCW;
  uint8_t type;
  uint8_t state;    // 0=entering(side) 1=formation 2=diving 3=returning 4=entering(centre)
  uint8_t hp;
  bool    active;
  int     diveCountdown;
  int     shootCountdown;
  bool    hasFired;
};

struct GyrPart {
  float x, y, vx, vy;
  uint8_t life;
  uint16_t col;
  bool active;
};

struct GyrSatellite {
  float angle, r;
  float orbitSpd;
  bool  isSun;
  bool  active;
};

struct GyrAsteroid {
  float x, y, vx, vy;
  uint8_t size;
  bool  active;
};

struct GyrLaser {
  float angle, r, vr;
  uint8_t phase;     // 0=flying 1=charging 2=firing 3=done
  int   phaseTimer;
  bool  active;
};

// ── Static pools ──────────────────────────────────────────────────────────
static GyrStar      gyrStars    [GYR_NUM_STARS];
static GyrPBullet   gyrPBullets [GYR_MAX_PBULLETS];
static GyrEBullet   gyrEBullets [GYR_MAX_EBULLETS];
static GyrEnemy     gyrEnemies  [GYR_MAX_ENEMIES];
static GyrPart      gyrParts    [GYR_MAX_PARTS];
static GyrSatellite gyrSats     [GYR_NUM_SATS];
static GyrAsteroid  gyrAsteroids[GYR_MAX_ASTEROIDS];
static GyrLaser     gyrLasers   [GYR_MAX_LASERS];

// ── Game state ────────────────────────────────────────────────────────────
static float  gyrAngle;
static float  gyrSpeed;
static int    gyrLives;
static int    gyrScore;
static int    gyrHiScore;
static int    gyrWave;
static int    gyrPlanet;
static int    gyrWarpInPlanet;
static uint8_t gyrWeapon;
static int    gyrSmartBombs;
static int    gyrNextLifeScore;
static int    gyrWaveTotal;
static int    gyrSpawned;
static int    gyrAlive;
static unsigned long gyrSpawnAt;
static bool   gyrDead;
static bool   gyrChanceMode;
static unsigned long gyrDeadAt;
static bool   gyrInvincible;
static unsigned long gyrInvincibleUntil;
static unsigned long gyrLastShot;
static unsigned long gyrPrevFrame;
static bool   gyrSatsActive;
static bool   gyrSatsDone;
static int    gyrAsteroidTimer;
static int    gyrLaserTimer;

// ── Forward declarations ──────────────────────────────────────────────────
static void gyrDrawEnemy(GyrEnemy& e, bool erase);
static void gyrSpawnEnemy(int slot);
static void gyrBurst(float a, float r, uint16_t col, int n);
static void gyrFireEBullet(float fx, float fy, float bx, float by, float spread);
static void gyrBurstXY(float x, float y, uint16_t col, int n);
static void gyrTickParts();
static void gyrDrawShip(float a, uint16_t col);
static void gyrDrawHUD();
static void gyrAnnounceWave();
static void gyrInitWave();
static void gyrInitGame();
static void gyrRespawn();
static void gyrFirePlayerBullets();
static void gyrDrawSatellite(int i, bool erase);
static void gyrTickSatellites();
static void gyrSpawnSatellites();
static void gyrTickAsteroids();
static void gyrTickLasers();
static void gyrSpawnAsteroid();
static void gyrSpawnLaser();
static void gyrRunChanceStage();
static void gyrSmartBomb();
static uint16_t gyrEneColor(uint8_t type);

// ══════════════════════════════════════════════════════════════════════════
//  SOUND
// ══════════════════════════════════════════════════════════════════════════
static void gyrSndShoot()      { beepTone(880,10); beepTone(660,8); }
static void gyrSndShoot2()     { beepTone(1100,8); beepTone(880,8); beepTone(660,6); }
static void gyrSndHit()        { beepTone(350,18); beepTone(220,18); }
static void gyrSndExplode()    { for(int f=480;f>=60;f-=35) beepTone(f,7); }
static void gyrSndPlayerDie()  { beepTone(440,70); beepTone(330,70); beepTone(220,90); beepTone(110,140); }
static void gyrSndWaveStart()  { int n[]={440,550,660,880,1100}; for(int i=0;i<5;i++){beepTone(n[i],45);delay(5);} }
static void gyrSndWarp()       { for(int f=180;f<=1400;f+=25) beepTone(f,5); for(int f=1400;f>=600;f-=20) beepTone(f,4); }
static void gyrSndLevelUp()    { int n[]={440,550,660,770,880,1100,1320}; int d[]={55,55,55,55,80,100,160}; for(int i=0;i<7;i++){beepTone(n[i],d[i]);delay(8);} }
static void gyrSndPowerup()    { beepTone(600,30); beepTone(800,30); beepTone(1000,30); beepTone(1200,60); }
static void gyrSndDoubleShot() { beepTone(500,20); beepTone(700,20); beepTone(1000,30); beepTone(1400,60); }
static void gyrSndSmartBomb()  { for(int f=1200;f>=100;f-=40) beepTone(f,6); }
static void gyrSndExtraLife()  { int n[]={660,880,1100,880,1320}; for(int i=0;i<5;i++){beepTone(n[i],70);delay(10);} }
static void gyrSndAsteroid()   { beepTone(180,30); }
static void gyrSndLaser()      { for(int f=200;f<=1800;f+=60) beepTone(f,4); }
static void gyrSndChanceIn()   { int n[]={660,770,880,1100,1320,1760}; int d[]={60,60,60,80,100,160}; for(int i=0;i<6;i++){beepTone(n[i],d[i]);delay(6);} }

// ══════════════════════════════════════════════════════════════════════════
//  COORDINATE HELPERS
// ══════════════════════════════════════════════════════════════════════════
static inline int gyrPX(float a, float r) { return GYR_CX + (int)(r * cosf(a)); }
static inline int gyrPY(float a, float r) { return GYR_CY + (int)(r * sinf(a)); }

static inline float gyrWrapAngle(float a) {
  while (a > 2.0f*(float)M_PI) a -= 2.0f*(float)M_PI;
  while (a < 0)                 a += 2.0f*(float)M_PI;
  return a;
}

static float gyrAngleDiff(float a, float b) {
  float d = b - a;
  while (d >  (float)M_PI) d -= 2.0f*(float)M_PI;
  while (d < -(float)M_PI) d += 2.0f*(float)M_PI;
  return d;
}

// ══════════════════════════════════════════════════════════════════════════
//  STARS
// ══════════════════════════════════════════════════════════════════════════
static void gyrResetStar(int i) {
  gyrStars[i].angle = (float)random(6283) / 1000.0f;
  gyrStars[i].r     = (float)random(4, 22);
  gyrStars[i].speed = 0.35f + (float)random(30) * 0.04f;
}

static void gyrInitStars() {
  for (int i = 0; i < GYR_NUM_STARS; i++) {
    gyrResetStar(i);
    gyrStars[i].r = (float)random(4, GYR_STAR_MAX);
  }
}

static void gyrTickStars(float mult) {
  for (int i = 0; i < GYR_NUM_STARS; i++) {
    display->drawPixel(gyrPX(gyrStars[i].angle, gyrStars[i].r),
                       gyrPY(gyrStars[i].angle, gyrStars[i].r), ARC_BLACK);
    gyrStars[i].r += gyrStars[i].speed * (1.0f + gyrStars[i].r * 0.022f) * mult;
    if (gyrStars[i].r > GYR_STAR_MAX) { gyrResetStar(i); continue; }
    uint16_t c = (gyrStars[i].r > 104) ? GYR_STAR_BRT    // scaled: 78*1.333
               : (gyrStars[i].r >  56) ? GYR_STAR_MID    // scaled: 42*1.333
               :                          GYR_STAR_DIM;
    display->drawPixel(gyrPX(gyrStars[i].angle, gyrStars[i].r),
                       gyrPY(gyrStars[i].angle, gyrStars[i].r), c);
  }
}

// ══════════════════════════════════════════════════════════════════════════
//  PLAYER SHIP
// ══════════════════════════════════════════════════════════════════════════
static void gyrDrawShip(float a, uint16_t col) {
  // Scaled from original: distances *1.4 (between 1.333 and 1.5)
  int tx = gyrPX(a, GYR_PLAYER_R - 18);   // nose tip (inward)
  int ty = gyrPY(a, GYR_PLAYER_R - 18);
  int bx = gyrPX(a, GYR_PLAYER_R + 5);    // tail base
  int by = gyrPY(a, GYR_PLAYER_R + 5);
  float wa = a + (float)M_PI * 0.5f;
  int lx = bx + (int)(11.0f * cosf(wa));  // left wing
  int ly = by + (int)(11.0f * sinf(wa));
  int rx = bx - (int)(11.0f * cosf(wa));  // right wing
  int ry = by - (int)(11.0f * sinf(wa));

  if (col == ARC_BLACK) {
    int cx = gyrPX(a, GYR_PLAYER_R - 1);
    int cy = gyrPY(a, GYR_PLAYER_R - 1);
    display->fillCircle(cx, cy, 20, ARC_BLACK);
  } else {
    display->fillTriangle(tx, ty, lx, ly, rx, ry, col);
    // Cockpit
    int mx = (tx + bx) / 2, my = (ty + by) / 2;
    uint16_t cockpitCol = (gyrWeapon == GYR_WPN_DOUBLE) ? ARC_YELLOW : GYR_PLAYER_COL;
    display->fillCircle(mx, my, 2, cockpitCol);
    display->drawLine(lx, ly, rx, ry, ARC_WHITE);
    // Engine nozzle
    int ex = gyrPX(a, GYR_PLAYER_R + 9);
    int ey = gyrPY(a, GYR_PLAYER_R + 9);
    display->fillCircle(ex, ey, 2, 0xFD20);
    // Double-shot wing dots
    if (gyrWeapon == GYR_WPN_DOUBLE) {
      display->drawPixel(ex + (int)(4*cosf(wa)), ey + (int)(4*sinf(wa)), ARC_YELLOW);
      display->drawPixel(ex - (int)(4*cosf(wa)), ey - (int)(4*sinf(wa)), ARC_YELLOW);
    }
    // Smart bomb indicator dots on wings
    for (int s = 0; s < min(gyrSmartBombs, 3); s++) {
      float sa = a + (float)M_PI * 0.5f;
      int sdx = (int)((6 + s*4)*cosf(sa));
      int sdy = (int)((6 + s*4)*sinf(sa));
      display->fillCircle(mx + sdx, my + sdy, 1, ARC_RED);
    }
  }
}

// ══════════════════════════════════════════════════════════════════════════
//  PLAYER BULLETS
// ══════════════════════════════════════════════════════════════════════════
static void gyrFirePlayerBullets() {
  for (int i = 0; i < GYR_MAX_PBULLETS; i++) {
    if (!gyrPBullets[i].active) {
      gyrPBullets[i] = { gyrAngle, (float)GYR_PLAYER_R - 6, 0.0f, true };
      if (gyrWeapon == GYR_WPN_DOUBLE) {
        for (int j = i+1; j < GYR_MAX_PBULLETS; j++) {
          if (!gyrPBullets[j].active) {
            gyrPBullets[j] = { gyrAngle, (float)GYR_PLAYER_R - 6, 0.14f, true };
            break;
          }
        }
        gyrSndShoot2();
      } else {
        gyrSndShoot();
      }
      break;
    }
  }
}

// ══════════════════════════════════════════════════════════════════════════
//  ENEMIES
// ══════════════════════════════════════════════════════════════════════════
static uint16_t gyrEneColor(uint8_t type) {
  if (type == 0) return GYR_ENE_COL0;
  if (type == 1) return GYR_ENE_COL1;
  return GYR_ENE_COL2;
}

static void gyrDrawEnemy(GyrEnemy& e, bool erase) {
  int x = gyrPX(e.angle, e.r);
  int y = gyrPY(e.angle, e.r);

  float wa  = e.angle + (float)M_PI * 0.5f;
  float cwa = cosf(wa), swa = sinf(wa);
  float cra = cosf(e.angle + (float)M_PI);  // inward
  float sra = sinf(e.angle + (float)M_PI);

  if (erase) {
    display->fillCircle(x, y, 12, ARC_BLACK);  // scaled: 10*1.2
    return;
  }

  uint16_t c    = gyrEneColor(e.type);
  uint16_t cDim = (c >> 1) & 0x7BEF;
  uint16_t cBrt = ARC_WHITE;

  // Body (elongated capsule along radial)
  int bfx = x + (int)(4.0f * cra), bfy = y + (int)(4.0f * sra);  // nose
  int bbx = x - (int)(4.0f * cra), bby = y - (int)(4.0f * sra);  // tail
  display->drawLine(bfx, bfy, bbx, bby, c);
  display->drawPixel(x + (int)(1.5f*cwa), y + (int)(1.5f*swa), c);
  display->drawPixel(x - (int)(1.5f*cwa), y - (int)(1.5f*swa), c);

  // Main wings
  int w1x = x + (int)(9.0f * cwa), w1y = y + (int)(9.0f * swa);
  int w2x = x - (int)(9.0f * cwa), w2y = y - (int)(9.0f * swa);
  int wr1x = bfx + (int)(4.0f * cwa), wr1y = bfy + (int)(4.0f * swa);
  int wr2x = bfx - (int)(4.0f * cwa), wr2y = bfy - (int)(4.0f * swa);
  display->drawLine(wr1x, wr1y, w1x, w1y, c);
  display->drawLine(wr2x, wr2y, w2x, w2y, c);

  // Swept trailing edge
  display->drawLine(w1x, w1y, bbx + (int)(2.5f*cwa), bby + (int)(2.5f*swa), cDim);
  display->drawLine(w2x, w2y, bbx - (int)(2.5f*cwa), bby - (int)(2.5f*swa), cDim);

  // Rear fins
  int rf1x = bbx + (int)(4.0f*cwa), rf1y = bby + (int)(4.0f*swa);
  int rf2x = bbx - (int)(4.0f*cwa), rf2y = bby - (int)(4.0f*swa);
  display->drawLine(bbx, bby, rf1x, rf1y, cDim);
  display->drawLine(bbx, bby, rf2x, rf2y, cDim);

  // Cockpit / nose dot
  uint16_t cockpitCol = (e.type == 0) ? 0x07FF
                      : (e.type == 1) ? ARC_WHITE
                      :                 0xFC00;
  display->fillCircle(bfx, bfy, 2, cockpitCol);

  // HP ring for heavy type
  if (e.type == 2 && e.hp == 2) {
    display->drawCircle(x, y, 8, c);
  }
}

static void gyrSpawnEnemy(int slot) {
  float formAngle = (float)gyrSpawned * (2.0f*(float)M_PI / (float)gyrWaveTotal);

  uint8_t type = 0;
  if (gyrWave >= 2 && gyrSpawned % 3 == 2) type = 1;
  if (gyrWave >= 3 && gyrSpawned % 5 == 4) type = 2;
  if (gyrWave >= 4 && gyrSpawned % 2 == 1) type = 1;
  if (gyrWave >= 6 && gyrSpawned % 3 == 2) type = 2;

  GyrEnemy& e   = gyrEnemies[slot];
  e.formAngle   = formAngle;
  e.vAngle      = 0.007f + gyrWave * 0.0005f;
  e.entryT      = 0;
  e.type        = type;
  e.hp          = (type == 2) ? 2 : 1;
  e.active      = true;
  e.diveCountdown  = max(80, 260 + random(200) - gyrWave * 10);
  int baseShoot    = (type == 0) ? 90 : (type == 1) ? 70 : 50;
  e.shootCountdown = baseShoot + (gyrSpawned * 23) % 90 + random(40);
  e.hasFired       = false;

  // From wave 2 onward, every 4th enemy bursts from the centre
  bool fromCentre = (gyrWave >= 2 && gyrSpawned % 4 == 3);

  if (fromCentre) {
    // Centre-spawn: start at r=0, spiral outward to formation
    e.state     = 4;
    e.r         = 0.0f;
    e.angle     = formAngle;   // will radiate straight outward
    e.spiralCW  = (gyrSpawned % 2 == 0);
    // Flash burst at centre so the player sees it coming
    gyrBurstXY((float)GYR_CX, (float)GYR_CY, gyrEneColor(type), 8);
  } else {
    // Normal side-entry
    e.state     = 0;
    bool fromRight = (gyrSpawned % 2 == 0);
    int  groupIdx  = gyrSpawned / 2;
    float spawnFan = (float)(groupIdx % 3 - 1) * 0.35f;
    float sideAngle = fromRight ? (0.0f + spawnFan) : ((float)M_PI + spawnFan);
    e.r         = GYR_SPAWN_RADIUS;
    e.angle     = sideAngle;
    e.spiralCW  = fromRight;
  }
}

// ══════════════════════════════════════════════════════════════════════════
//  SATELLITE POWER-UP PODS
// ══════════════════════════════════════════════════════════════════════════
static void gyrSpawnSatellites() {
  for (int i = 0; i < GYR_NUM_SATS; i++) {
    float offset = (i - 1) * 0.28f;
    gyrSats[i].angle    = gyrAngle + offset;
    gyrSats[i].r        = GYR_PLAYER_R - 28;   // scaled: 22*1.27
    gyrSats[i].orbitSpd = 0.018f * (i % 2 == 0 ? 1 : -1);
    gyrSats[i].isSun    = (i == 1);
    gyrSats[i].active   = true;
  }
  gyrSatsActive = true;
}

static void gyrDrawSatellite(int i, bool erase) {
  if (!gyrSats[i].active) return;
  int x = gyrPX(gyrSats[i].angle, gyrSats[i].r);
  int y = gyrPY(gyrSats[i].angle, gyrSats[i].r);
  if (erase) {
    display->fillCircle(x, y, 10, ARC_BLACK);
    return;
  }
  if (gyrSats[i].isSun) {
    display->fillCircle(x, y, 5, GYR_SUN_COL);
    display->fillCircle(x, y, 2, ARC_WHITE);
    for (int r = 0; r < 8; r++) {
      float ra = r * (float)M_PI / 4.0f;
      display->drawPixel(x + (int)(8*cosf(ra)), y + (int)(8*sinf(ra)), GYR_SUN_COL);
    }
  } else {
    display->fillCircle(x, y, 4, GYR_SAT_COL);
    display->drawCircle(x, y, 7, GYR_SAT_COL);
  }
}

static void gyrTickSatellites() {
  if (!gyrSatsActive) return;

  static int satLifetime = 0;
  satLifetime++;

  bool anyActive = false;
  for (int i = 0; i < GYR_NUM_SATS; i++) {
    if (!gyrSats[i].active) continue;
    anyActive = true;

    gyrDrawSatellite(i, true);
    gyrSats[i].angle = gyrWrapAngle(gyrSats[i].angle + gyrSats[i].orbitSpd);

    for (int b = 0; b < GYR_MAX_PBULLETS; b++) {
      if (!gyrPBullets[b].active) continue;
      float drawA = gyrPBullets[b].angle + gyrPBullets[b].offset;
      float bx = (float)gyrPX(drawA, gyrPBullets[b].r);
      float by = (float)gyrPY(drawA, gyrPBullets[b].r);
      float sx = (float)gyrPX(gyrSats[i].angle, gyrSats[i].r);
      float sy = (float)gyrPY(gyrSats[i].angle, gyrSats[i].r);
      float dx = bx - sx, dy = by - sy;
      if (dx*dx + dy*dy < 64) {
        gyrPBullets[b].active = false;
        gyrSats[i].active = false;
        gyrBurstXY(sx, sy, GYR_SAT_COL, 10);

        if (gyrSats[i].isSun) {
          if (gyrWeapon == GYR_WPN_SINGLE) {
            gyrWeapon = GYR_WPN_DOUBLE;
            gyrSndDoubleShot();
            display->setTextSize(1);
            display->setTextColor(ARC_YELLOW);
            display->setCursor(GYR_CX - 54, 22);
            display->print("** DOUBLE SHOT! **");
            delay(800);
            display->fillRect(GYR_CX - 58, 22, 120, 9, ARC_BLACK);
          } else {
            if (gyrLives < 5) { gyrLives++; gyrSndExtraLife(); gyrDrawHUD(); }
          }
        } else {
          gyrScore += 1000;
          if (gyrScore > gyrHiScore) gyrHiScore = gyrScore;
          if (random(100) < 40 && gyrSmartBombs < 3) {
            gyrSmartBombs++;
            gyrSndPowerup();
          } else if (random(100) < 25 && gyrLives < 5) {
            gyrLives++;
            gyrSndExtraLife();
            gyrDrawHUD();
          }
        }
        gyrDrawHUD();
        break;
      }
    }

    if (gyrSats[i].active) gyrDrawSatellite(i, false);
  }

  if (!anyActive || satLifetime > 150) {
    for (int i = 0; i < GYR_NUM_SATS; i++) {
      if (gyrSats[i].active) { gyrDrawSatellite(i, true); gyrSats[i].active = false; }
    }
    gyrSatsActive = false;
    satLifetime = 0;
  }
}

// ══════════════════════════════════════════════════════════════════════════
//  ASTEROIDS  — spawn from centre, travel outward
// ══════════════════════════════════════════════════════════════════════════
static void gyrSpawnAsteroid() {
  for (int i = 0; i < GYR_MAX_ASTEROIDS; i++) {
    if (!gyrAsteroids[i].active) {
      gyrAsteroids[i].x = GYR_CX;
      gyrAsteroids[i].y = GYR_CY;
      float angle = (float)random(6283) / 1000.0f;
      float spd   = 1.8f + (float)random(8) * 0.15f;
      gyrAsteroids[i].vx     = cosf(angle) * spd;
      gyrAsteroids[i].vy     = sinf(angle) * spd;
      gyrAsteroids[i].size   = 5 + random(4);   // scaled: 4+3
      gyrAsteroids[i].active = true;
      gyrSndAsteroid();
      break;
    }
  }
}

static void gyrTickAsteroids() {
  for (int i = 0; i < GYR_MAX_ASTEROIDS; i++) {
    if (!gyrAsteroids[i].active) continue;

    display->fillCircle((int)gyrAsteroids[i].x, (int)gyrAsteroids[i].y,
                        gyrAsteroids[i].size + 1, ARC_BLACK);

    gyrAsteroids[i].x += gyrAsteroids[i].vx;
    gyrAsteroids[i].y += gyrAsteroids[i].vy;

    int nx = (int)gyrAsteroids[i].x;
    int ny = (int)gyrAsteroids[i].y;

    float distFromCentre = sqrtf((nx-GYR_CX)*(float)(nx-GYR_CX) +
                                 (ny-GYR_CY)*(float)(ny-GYR_CY));
    if (distFromCentre > GYR_PLAYER_R + 15) {
      gyrAsteroids[i].active = false;
      continue;
    }

    // Draw asteroid — orange-brown with jagged outline
    uint16_t astInner = 0xCA60;
    uint16_t astOuter = 0xFD00;
    int sz = gyrAsteroids[i].size;
    display->fillCircle(nx, ny, sz, astInner);
    display->drawCircle(nx, ny, sz, astOuter);
    display->fillCircle(nx+sz, ny,   1, astOuter);
    display->fillCircle(nx-sz, ny,   1, astOuter);
    display->fillCircle(nx,   ny+sz, 1, astOuter);
    display->fillCircle(nx,   ny-sz, 1, astOuter);
    display->fillCircle(nx-1, ny-1,  1, 0x2000);  // crater

    // Bullet hits asteroid (slows it, 100 pts, doesn't destroy)
    for (int b = 0; b < GYR_MAX_PBULLETS; b++) {
      if (!gyrPBullets[b].active) continue;
      float drawA = gyrPBullets[b].angle + gyrPBullets[b].offset;
      float bx = (float)gyrPX(drawA, gyrPBullets[b].r);
      float by = (float)gyrPY(drawA, gyrPBullets[b].r);
      float dx = bx - nx, dy = by - ny;
      if (dx*dx + dy*dy < (float)(sz*sz + 16)) {
        gyrPBullets[b].active = false;
        gyrScore += 100;
        if (gyrScore > gyrHiScore) gyrHiScore = gyrScore;
        gyrBurstXY(bx, by, 0xFD00, 3);
        gyrDrawHUD();
        gyrAsteroids[i].vx *= 0.8f;
        gyrAsteroids[i].vy *= 0.8f;
        break;
      }
    }

    // Player collision — only kills if NOT actively firing
    if (!gyrDead && !gyrChanceMode && !gyrInvincible) {
      float px = (float)gyrPX(gyrAngle, GYR_PLAYER_R);
      float py = (float)gyrPY(gyrAngle, GYR_PLAYER_R);
      float dx = nx - px, dy = ny - py;
      bool playerFiring = false;
      for (int b = 0; b < GYR_MAX_PBULLETS; b++)
        if (gyrPBullets[b].active) { playerFiring = true; break; }
      if (!playerFiring && sqrtf(dx*dx+dy*dy) < (float)(sz + 11)) {
        gyrAsteroids[i].active = false;
        gyrDead   = true; gyrDeadAt = millis(); gyrLives--;
        gyrDrawShip(gyrAngle, ARC_BLACK);
        gyrBurst(gyrAngle, GYR_PLAYER_R, GYR_PLAYER_COL, 22);
        gyrBurst(gyrAngle, GYR_PLAYER_R, ARC_WHITE, 10);
        gyrSndPlayerDie(); gyrDrawHUD();
      }
    }
  }
}

// ══════════════════════════════════════════════════════════════════════════
//  LASER BEAM GENERATORS
// ══════════════════════════════════════════════════════════════════════════
static void gyrSpawnLaser() {
  for (int i = 0; i < GYR_MAX_LASERS; i++) {
    if (!gyrLasers[i].active) {
      gyrLasers[i].angle      = gyrAngle + gyrAngleDiff(0, (float)random(628)/100.0f - (float)M_PI);
      gyrLasers[i].r          = 10;
      gyrLasers[i].vr         = 3.0f;
      gyrLasers[i].phase      = 0;
      gyrLasers[i].phaseTimer = 0;
      gyrLasers[i].active     = true;
      break;
    }
  }
}

static void gyrTickLasers() {
  for (int i = 0; i < GYR_MAX_LASERS; i++) {
    if (!gyrLasers[i].active) continue;

    int lx = gyrPX(gyrLasers[i].angle, gyrLasers[i].r);
    int ly = gyrPY(gyrLasers[i].angle, gyrLasers[i].r);
    display->fillCircle(lx, ly, 9, ARC_BLACK);   // scaled: 7*1.3

    if (gyrLasers[i].phase == 2) {
      // Erase beam — clear the whole ring area
      display->fillCircle(GYR_CX, GYR_CY, GYR_PLAYER_R + 3, ARC_BLACK);
      gyrDrawHUD();
      if (!gyrDead) gyrDrawShip(gyrAngle, GYR_PLAYER_COL);
    }

    gyrLasers[i].phaseTimer++;

    if (gyrLasers[i].phase == 0) {
      gyrLasers[i].r += gyrLasers[i].vr;
      if (gyrLasers[i].r >= GYR_FORM_R + 14) {   // scaled: 10*1.4
        gyrLasers[i].phase = 1;
        gyrLasers[i].phaseTimer = 0;
      }
    } else if (gyrLasers[i].phase == 1) {
      if (gyrLasers[i].phaseTimer >= 30) {
        gyrLasers[i].phase = 2;
        gyrLasers[i].phaseTimer = 0;
        gyrSndLaser();
      }
    } else if (gyrLasers[i].phase == 2) {
      // Draw beam radially from generator toward player ring
      float ba = gyrLasers[i].angle;
      for (float br = gyrLasers[i].r; br <= GYR_PLAYER_R + 6; br += 1.5f) {
        int bx = gyrPX(ba, br);
        int by = gyrPY(ba, br);
        uint16_t bc = (gyrLasers[i].phaseTimer % 4 < 2) ? GYR_LASER_COL : GYR_BEAM_COL;
        display->drawPixel(bx, by, bc);
        float pa = ba + (float)M_PI * 0.5f;
        display->drawPixel(bx + (int)cosf(pa), by + (int)sinf(pa), bc);
        display->drawPixel(bx - (int)cosf(pa), by - (int)sinf(pa), bc);
      }

      // Player hit check
      if (!gyrDead && !gyrChanceMode && !gyrInvincible) {
        float da = fabsf(gyrAngleDiff(gyrLasers[i].angle, gyrAngle));
        if (da < 0.12f) {
          gyrDead = true; gyrDeadAt = millis(); gyrLives--;
          gyrDrawShip(gyrAngle, ARC_BLACK);
          gyrBurst(gyrAngle, GYR_PLAYER_R, GYR_PLAYER_COL, 22);
          gyrBurst(gyrAngle, GYR_PLAYER_R, ARC_WHITE, 10);
          gyrSndPlayerDie(); gyrDrawHUD();
        }
      }

      if (gyrLasers[i].phaseTimer >= 20) {
        gyrLasers[i].phase = 3;
        gyrLasers[i].phaseTimer = 0;
      }
    } else {
      // Fly back to centre
      gyrLasers[i].r -= 4.0f;
      if (gyrLasers[i].r <= 5) { gyrLasers[i].active = false; continue; }
    }

    // Redraw generator (flashes during charge)
    if (gyrLasers[i].phase != 3) {
      lx = gyrPX(gyrLasers[i].angle, gyrLasers[i].r);
      ly = gyrPY(gyrLasers[i].angle, gyrLasers[i].r);
      uint16_t gc = (gyrLasers[i].phase == 1 && gyrLasers[i].phaseTimer % 6 < 3)
                    ? ARC_WHITE : GYR_LASER_COL;
      display->fillCircle(lx, ly, 6, gc);
      display->fillCircle(lx, ly, 2, ARC_WHITE);
      float wa = gyrLasers[i].angle + (float)M_PI * 0.5f;
      display->drawLine(lx+(int)(10*cosf(wa)), ly+(int)(10*sinf(wa)),
                        lx-(int)(10*cosf(wa)), ly-(int)(10*sinf(wa)), gc);
    }

    // Bullet destroys laser generator
    for (int b = 0; b < GYR_MAX_PBULLETS; b++) {
      if (!gyrPBullets[b].active) continue;
      float drawA = gyrPBullets[b].angle + gyrPBullets[b].offset;
      float bx2 = (float)gyrPX(drawA, gyrPBullets[b].r);
      float by2 = (float)gyrPY(drawA, gyrPBullets[b].r);
      int lxc = gyrPX(gyrLasers[i].angle, gyrLasers[i].r);
      int lyc = gyrPY(gyrLasers[i].angle, gyrLasers[i].r);
      float dx = bx2-lxc, dy = by2-lyc;
      if (dx*dx + dy*dy < 64) {
        gyrPBullets[b].active = false;
        gyrLasers[i].active   = false;
        gyrScore += 300;
        if (gyrScore > gyrHiScore) gyrHiScore = gyrScore;
        gyrBurstXY((float)lxc, (float)lyc, GYR_LASER_COL, 12);
        gyrSndExplode(); gyrDrawHUD();
        break;
      }
    }
  }
}

// ══════════════════════════════════════════════════════════════════════════
//  PARTICLES
// ══════════════════════════════════════════════════════════════════════════
static void gyrBurst(float a, float r, uint16_t col, int n) {
  gyrBurstXY((float)gyrPX(a, r), (float)gyrPY(a, r), col, n);
}

static void gyrFireEBullet(float fromX, float fromY, float aimBiasX, float aimBiasY, float extraSpread) {
  for (int _b = 0; _b < GYR_MAX_EBULLETS; _b++) {
    if (!gyrEBullets[_b].active) {
      float px = (float)gyrPX(gyrAngle, GYR_PLAYER_R);
      float py = (float)gyrPY(gyrAngle, GYR_PLAYER_R);
      float dx = (px + aimBiasX) - fromX;
      float dy = (py + aimBiasY) - fromY;
      float len = sqrtf(dx*dx + dy*dy);
      if (len < 1.0f) len = 1.0f;
      float spd = 4.5f + gyrWave * 0.12f;
      float cs = cosf(extraSpread), sn = sinf(extraSpread);
      float vx = (dx/len*spd)*cs - (dy/len*spd)*sn;
      float vy = (dx/len*spd)*sn + (dy/len*spd)*cs;
      gyrEBullets[_b] = { fromX, fromY, vx, vy, true };
      break;
    }
  }
}

static void gyrBurstXY(float x, float y, uint16_t col, int n) {
  int spawned = 0;
  for (int i = 0; i < GYR_MAX_PARTS && spawned < n; i++) {
    if (gyrParts[i].active) continue;
    float pa  = (float)random(6283) / 1000.0f;
    float spd = 0.7f + (float)random(20) * 0.13f;
    gyrParts[i] = { x, y, cosf(pa)*spd, sinf(pa)*spd,
                    (uint8_t)(14 + random(14)), col, true };
    spawned++;
  }
}

static void gyrTickParts() {
  for (int i = 0; i < GYR_MAX_PARTS; i++) {
    if (!gyrParts[i].active) continue;
    display->drawPixel((int)gyrParts[i].x, (int)gyrParts[i].y, ARC_BLACK);
    gyrParts[i].x += gyrParts[i].vx;
    gyrParts[i].y += gyrParts[i].vy;
    if (!--gyrParts[i].life) { gyrParts[i].active = false; continue; }
    uint16_t c = (gyrParts[i].life > 8) ? gyrParts[i].col
                                        : (gyrParts[i].col >> 1) & 0x7BEF;
    display->drawPixel((int)gyrParts[i].x, (int)gyrParts[i].y, c);
  }
}

// ══════════════════════════════════════════════════════════════════════════
//  HUD
// ══════════════════════════════════════════════════════════════════════════
static void gyrDrawHUD() {
  display->fillRect(0, 0, SCREEN_WIDTH, 18, ARC_BLACK);
  display->setTextSize(1);
  char buf[32];

  // Left: 1-UP + score
  display->setTextColor(GYR_HUD_LABEL);
  display->setCursor(4, 1);
  display->print("1-UP");
  display->setTextColor(GYR_HUD_SCORE);
  sprintf(buf, "%d", gyrScore);
  display->setCursor(4, 10);
  display->print(buf);

  // Centre: HI-SCORE (centred at GYR_CX=240; original centred at 134 of 320)
  display->setTextColor(GYR_HUD_LABEL);
  display->setCursor(GYR_CX - 24, 1);   // "HI-SCORE" = 8 chars * 6px / 2 = 24
  display->print("HI-SCORE");
  display->setTextColor(GYR_HUD_SCORE);
  sprintf(buf, "%d", gyrHiScore);
  int hiW = (int)strlen(buf) * 6;
  display->setCursor(GYR_CX - hiW/2, 10);
  display->print(buf);

  // DBL / smart bombs (right of centre)
  if (gyrWeapon == GYR_WPN_DOUBLE) {
    display->setTextColor(GYR_HUD_SCORE);
    display->setCursor(GYR_CX + 40, 1);
    display->print("DBL");
  }
  if (gyrSmartBombs > 0) {
    display->setTextColor(ARC_RED);
    display->setCursor(GYR_CX + 40, 10);
    for (int s = 0; s < gyrSmartBombs; s++) display->print("*");
  }

  // Right: STAGE N
  display->setTextColor(GYR_HUD_STAGE);
  sprintf(buf, "STAGE %d", gyrWave);
  int stageW = (int)strlen(buf) * 6;
  display->setCursor(SCREEN_WIDTH - 2 - stageW, 1);
  display->print(buf);

  // Lives as ship glyphs
  display->setTextColor(GYR_PLAYER_COL);
  display->setCursor(SCREEN_WIDTH - 2 - min(gyrLives,4)*7, 10);
  for (int i = 0; i < min(gyrLives, 4); i++) display->print("^ ");
}

// ══════════════════════════════════════════════════════════════════════════
//  WAVE ANNOUNCEMENT
// ══════════════════════════════════════════════════════════════════════════
static const char* const GYR_PLANETS[] = {
  "HEADING TO NEPTUNE", "HEADING TO URANUS",  "HEADING TO SATURN",
  "HEADING TO JUPITER", "HEADING TO MARS",    "HEADING TO EARTH",
  "HEADING TO THE SUN",
};
static const char* const GYR_WARPS[] = {
  "3  WARPS  TO  GO", "2  WARPS  TO  GO",
  "1  WARP  TO  GO",  "ARRIVING!"
};

static void gyrAnnounceWave() {
  gyrSndWarp();
  unsigned long ws = millis();
  while (millis() - ws < 1800) {
    float mult = 1.0f + (float)(millis() - ws) / 450.0f;
    gyrTickStars(mult);
    delay(18);
  }

  display->fillRect(0, 80, SCREEN_WIDTH, 80, ARC_BLACK);

  display->setTextSize(2);
  display->setTextColor(GYR_GOLD);
  const char* warpStr = GYR_WARPS[min(gyrWarpInPlanet - 1, 3)];
  int warpW = (int)strlen(warpStr) * 12;
  display->setCursor(GYR_CX - warpW/2, 88);
  display->print(warpStr);

  display->setTextSize(1);
  display->setTextColor(GYR_HUD_STAGE);
  const char* planet = GYR_PLANETS[min(gyrPlanet, 6)];
  int planetW = (int)strlen(planet) * 6;
  display->setCursor(GYR_CX - planetW/2, 114);
  display->print(planet);

  if (gyrWeapon == GYR_WPN_DOUBLE) {
    display->setTextColor(GYR_HUD_SCORE);
    display->setCursor(GYR_CX - 72, 128);
    display->print("** DOUBLE SHOT ACTIVE **");
  }

  gyrSndWaveStart();
  delay(1400);
  display->fillRect(0, 80, SCREEN_WIDTH, 80, ARC_BLACK);
  gyrDrawHUD();
}

// ══════════════════════════════════════════════════════════════════════════
//  CHANCE STAGE
// ══════════════════════════════════════════════════════════════════════════
static void gyrRunChanceStage() {
  gyrSndChanceIn();
  display->fillRect(0, 95, SCREEN_WIDTH, 50, ARC_BLACK);
  display->setTextSize(2);
  display->setTextColor(GYR_GOLD);
  display->setCursor(GYR_CX - 78, 104);
  display->print("CHANCE STAGE");
  display->setTextSize(1);
  display->setTextColor(GYR_HUD_STAGE);
  display->setCursor(GYR_CX - 84, 126);
  display->print("DESTROY ALL FOR 10,000 PTS");
  delay(2200);
  display->fillRect(0, 95, SCREEN_WIDTH, 50, ARC_BLACK);
  gyrDrawHUD();

  gyrChanceMode = true;

  for (int i = 0; i < GYR_MAX_ENEMIES;  i++) gyrEnemies[i].active   = false;
  for (int i = 0; i < GYR_MAX_PBULLETS; i++) gyrPBullets[i].active  = false;
  for (int i = 0; i < GYR_MAX_EBULLETS; i++) gyrEBullets[i].active  = false;
  for (int i = 0; i < GYR_MAX_PARTS;    i++) gyrParts[i].active     = false;
  for (int i = 0; i < GYR_MAX_ASTEROIDS;i++) gyrAsteroids[i].active = false;
  for (int i = 0; i < GYR_MAX_LASERS;   i++) gyrLasers[i].active    = false;
  gyrSatsActive = false;

  const int CS_GROUPS    = 4;
  const int CS_PER_GROUP = 5;
  const int CS_TOTAL     = CS_GROUPS * CS_PER_GROUP;
  const int CS_HOLD_FRAMES = 90;

  int  csKilled   = 0;
  int  csGroupIdx = 0;
  bool fromRight  = true;

  uint8_t groupState = 1;
  int     holdTimer  = 0;
  int     exitTimer  = 0;

  int groupSlots[CS_PER_GROUP];
  int slotsUsed = 0;

  // Spawn first group
  {
    slotsUsed = 0;
    float baseAngle = 0;
    for (int k = 0; k < CS_PER_GROUP; k++) {
      for (int s = 0; s < GYR_MAX_ENEMIES; s++) {
        if (!gyrEnemies[s].active) {
          float formA = baseAngle + (float)k * (2.0f*(float)M_PI / (float)CS_TOTAL);
          GyrEnemy& e  = gyrEnemies[s];
          e.formAngle  = formA;
          e.angle      = fromRight ? 0.0f : (float)M_PI;
          e.r          = GYR_SPAWN_RADIUS;
          e.vAngle     = 0.010f;
          e.entryT     = 0;
          e.spiralCW   = fromRight;
          e.type       = 0; e.state = 0; e.hp = 1; e.active = true;
          e.diveCountdown = 99999; e.shootCountdown = 99999; e.hasFired = false;
          groupSlots[slotsUsed++] = s;
          break;
        }
      }
    }
  }
  fromRight = !fromRight;

  unsigned long chanceFrame = millis();
  unsigned long stageStart  = millis();

  while (true) {
    unsigned long now = millis();
    if (now - chanceFrame < 33) continue;
    chanceFrame = now;

    if (isBtnSet() && isBtnB()) { gyrChanceMode = false; display->fillScreen(ARC_BLACK); return; }

    float tspd = 0;
    if (isBtnLeft())  tspd = -0.092f;
    if (isBtnRight()) tspd =  0.092f;
    gyrSpeed += (tspd - gyrSpeed) * 0.42f;
    gyrDrawShip(gyrAngle, ARC_BLACK);
    gyrAngle = gyrWrapAngle(gyrAngle + gyrSpeed);

    bool shootBtn = (isBtnSet() && !isBtnB()) || (isBtnB() && !isBtnSet());
    if (shootBtn && now - gyrLastShot > 160) { gyrFirePlayerBullets(); gyrLastShot = now; }

    gyrTickStars(1.0f);

    // Player bullets
    for (int i = 0; i < GYR_MAX_PBULLETS; i++) {
      GyrPBullet& b = gyrPBullets[i];
      if (!b.active) continue;
      float drawA = b.angle + b.offset;
      int ox = gyrPX(drawA, b.r), oy = gyrPY(drawA, b.r);
      display->fillRect(ox-1, oy-1, 3, 3, ARC_BLACK);
      b.r -= 7.5f;
      if (b.r < 3) { b.active = false; continue; }
      int nx = gyrPX(b.angle + b.offset, b.r), ny = gyrPY(b.angle + b.offset, b.r);
      display->fillRect(nx-1, ny-1, 3, 3, (gyrWeapon==GYR_WPN_DOUBLE)?GYR_BULL2_COL:GYR_BULLET_COL);
    }

    // Enemy update
    bool anyEntering = false, anyInFormation = false;
    for (int i = 0; i < GYR_MAX_ENEMIES; i++) {
      GyrEnemy& e = gyrEnemies[i];
      if (!e.active) continue;
      gyrDrawEnemy(e, true);

      if (e.state == 0) {
        e.entryT += 0.011f;
        if (e.entryT >= 1.0f) {
          e.entryT = 1.0f; e.state = 1; e.angle = e.formAngle; e.r = GYR_FORM_R;
        } else {
          float t = e.entryT;
          float ease = (t<0.5f)?(4.0f*t*t*t):(1.0f-powf(-2.0f*t+2.0f,3.0f)*0.5f);
          float sa = e.spiralCW ? 0.0f : (float)M_PI;
          float sx = GYR_SPAWN_RADIUS*cosf(sa), sy = GYR_SPAWN_RADIUS*sinf(sa);
          float ex2 = GYR_FORM_R*cosf(e.formAngle), ey2 = GYR_FORM_R*sinf(e.formAngle);
          float mx = (sx+ex2)*0.5f, my2 = (sy+ey2)*0.5f;
          float plen = sqrtf(mx*mx+my2*my2);
          float scl = (e.spiralCW?1.0f:-1.0f)*90.0f;
          float cpx = (plen>0.5f)?mx+(-my2/plen)*scl:0;
          float cpy = (plen>0.5f)?my2+(mx/plen)*scl:0;
          float u = 1.0f-ease;
          float bx2 = u*u*sx+2.0f*u*ease*cpx+ease*ease*ex2;
          float by2 = u*u*sy+2.0f*u*ease*cpy+ease*ease*ey2;
          e.r = sqrtf(bx2*bx2+by2*by2); e.angle = atan2f(by2,bx2);
        }
        anyEntering = true;
      } else if (e.state == 1) {
        e.formAngle = gyrWrapAngle(e.formAngle + e.vAngle);
        e.angle = e.formAngle; e.r = GYR_FORM_R;
        anyInFormation = true;
      } else if (e.state == 2) {
        e.r -= 3.5f;
        e.formAngle = gyrWrapAngle(e.formAngle + 0.06f);
        e.angle = e.formAngle;
        if (e.r < -GYR_SPAWN_RADIUS) { gyrDrawEnemy(e,true); e.active=false; continue; }
      }

      // Bullet collision
      for (int b = 0; b < GYR_MAX_PBULLETS; b++) {
        if (!gyrPBullets[b].active) continue;
        float drawA = gyrPBullets[b].angle + gyrPBullets[b].offset;
        float bxf = (float)gyrPX(drawA, gyrPBullets[b].r);
        float byf = (float)gyrPY(drawA, gyrPBullets[b].r);
        float exf = (float)gyrPX(e.angle, e.r);
        float eyf = (float)gyrPY(e.angle, e.r);
        float ddx = bxf-exf, ddy = byf-eyf;
        if (ddx*ddx+ddy*ddy < 144.0f) {
          display->fillRect((int)bxf-1,(int)byf-1,3,3,ARC_BLACK);
          gyrPBullets[b].active = false;
          gyrBurstXY(exf, eyf, GYR_ENE_COL0, 10);
          gyrSndExplode();
          e.active = false; csKilled++;
          gyrScore += 200; if (gyrScore>gyrHiScore) gyrHiScore=gyrScore;
          gyrDrawHUD(); break;
        }
      }
      if (e.active) gyrDrawEnemy(e, false);
    }

    gyrTickParts();
    gyrDrawShip(gyrAngle, GYR_PLAYER_COL);

    // Group state machine
    int groupActive = 0;
    for (int k = 0; k < slotsUsed; k++)
      if (gyrEnemies[groupSlots[k]].active) groupActive++;

    if (groupState == 1) {
      if (!anyEntering) { groupState = 2; holdTimer = CS_HOLD_FRAMES; }
    } else if (groupState == 2) {
      if (--holdTimer <= 0) {
        for (int k = 0; k < slotsUsed; k++) {
          int s = groupSlots[k];
          if (gyrEnemies[s].active && gyrEnemies[s].state == 1) gyrEnemies[s].state = 2;
        }
        groupState = 3; exitTimer = 80;
        csGroupIdx++;
        if (csGroupIdx < CS_GROUPS) {
          fromRight = !fromRight;
          slotsUsed = 0;
          float baseAngle2 = (float)csGroupIdx * (2.0f*(float)M_PI / (float)CS_GROUPS);
          for (int k2 = 0; k2 < CS_PER_GROUP; k2++) {
            for (int s2 = 0; s2 < GYR_MAX_ENEMIES; s2++) {
              if (!gyrEnemies[s2].active) {
                float formA2 = baseAngle2 + (float)k2*(2.0f*(float)M_PI/(float)CS_TOTAL);
                GyrEnemy& e2 = gyrEnemies[s2];
                e2.formAngle=formA2; e2.angle=fromRight?0.0f:(float)M_PI;
                e2.r=GYR_SPAWN_RADIUS; e2.vAngle=0.010f; e2.entryT=0;
                e2.spiralCW=fromRight; e2.type=0; e2.state=0; e2.hp=1; e2.active=true;
                e2.diveCountdown=99999; e2.shootCountdown=99999; e2.hasFired=false;
                groupSlots[slotsUsed++]=s2; break;
              }
            }
          }
          groupState = 1;
        }
      }
    } else if (groupState == 3) {
      if (--exitTimer <= 0 || groupActive == 0) {
        if (csGroupIdx >= CS_GROUPS) {
          bool anyLeft = false;
          for (int i = 0; i < GYR_MAX_ENEMIES; i++)
            if (gyrEnemies[i].active) { anyLeft=true; break; }
          if (!anyLeft) break;
        }
      }
    }
    if (now - stageStart > 90000UL) break;
  }

  // Clear remaining
  for (int i = 0; i < GYR_MAX_ENEMIES;  i++) {
    if (gyrEnemies[i].active) { gyrDrawEnemy(gyrEnemies[i],true); gyrEnemies[i].active=false; }
  }
  for (int i = 0; i < GYR_MAX_PBULLETS; i++) gyrPBullets[i].active = false;
  for (int i = 0; i < GYR_MAX_PARTS;    i++) gyrParts[i].active    = false;

  // Scoring
  if (csKilled == CS_TOTAL) {
    gyrScore += 10000; if (gyrScore>gyrHiScore) gyrHiScore=gyrScore;
    gyrSndLevelUp();
    display->fillRect(0, 96, SCREEN_WIDTH, 50, ARC_BLACK);
    display->setTextSize(2); display->setTextColor(GYR_GOLD);
    display->setCursor(GYR_CX - 90, 105);
    display->print("PERFECT! +10000");
    gyrDrawHUD(); delay(2000);
    display->fillRect(0, 96, SCREEN_WIDTH, 50, ARC_BLACK);
  } else {
    int bonus = csKilled * 500;
    gyrScore += bonus; if (gyrScore>gyrHiScore) gyrHiScore=gyrScore;
    display->fillRect(0, 96, SCREEN_WIDTH, 40, ARC_BLACK);
    display->setTextSize(1); display->setTextColor(GYR_HUD_STAGE);
    char tmpBuf[48];
    sprintf(tmpBuf, "DESTROYED: %d / %d  +%d PTS", csKilled, CS_TOTAL, bonus);
    int dW = (int)strlen(tmpBuf) * 6;
    display->setCursor(GYR_CX - dW/2, 108);
    display->print(tmpBuf);
    gyrDrawHUD(); delay(2000);
    display->fillRect(0, 96, SCREEN_WIDTH, 40, ARC_BLACK);
  }
  gyrChanceMode = false;
}

// ══════════════════════════════════════════════════════════════════════════
//  SMART BOMB
// ══════════════════════════════════════════════════════════════════════════
static void gyrSmartBomb() {
  if (gyrSmartBombs <= 0) return;
  gyrSmartBombs--;
  gyrSndSmartBomb();
  display->fillScreen(ARC_WHITE); delay(60);
  display->fillScreen(ARC_BLACK);
  gyrDrawHUD(); gyrDrawShip(gyrAngle, GYR_PLAYER_COL);
  for (int i = 0; i < GYR_MAX_ENEMIES; i++) {
    if (!gyrEnemies[i].active) continue;
    gyrBurst(gyrEnemies[i].angle, gyrEnemies[i].r, gyrEneColor(gyrEnemies[i].type), 8);
    gyrScore += 50;
    gyrEnemies[i].active = false; gyrAlive--;
    if (gyrAlive < 0) gyrAlive = 0;
  }
  for (int i = 0; i < GYR_MAX_EBULLETS;  i++) gyrEBullets[i].active  = false;
  for (int i = 0; i < GYR_MAX_ASTEROIDS; i++) gyrAsteroids[i].active = false;
  for (int i = 0; i < GYR_MAX_LASERS;    i++) gyrLasers[i].active    = false;
  if (gyrScore > gyrHiScore) gyrHiScore = gyrScore;
  gyrDrawHUD();
}

// ══════════════════════════════════════════════════════════════════════════
//  OVERLAY
// ══════════════════════════════════════════════════════════════════════════
static void gyrOverlay(const char* big, const char* sm1, const char* sm2, uint16_t col) {
  display->fillRect(0, 86, SCREEN_WIDTH, 70, ARC_BLACK);
  display->setTextSize(2); display->setTextColor(col);
  int bigW = (int)strlen(big)*12;
  display->setCursor(GYR_CX - bigW/2, 95); display->print(big);
  display->setTextSize(1); display->setTextColor(GYR_HUD_STAGE);
  int sm1W = (int)strlen(sm1)*6;
  display->setCursor(GYR_CX - sm1W/2, 122); display->print(sm1);
  display->setTextColor(0xAD75);
  int sm2W = (int)strlen(sm2)*6;
  display->setCursor(GYR_CX - sm2W/2, 136); display->print(sm2);
}

// ══════════════════════════════════════════════════════════════════════════
//  EEPROM HI-SCORE
// ══════════════════════════════════════════════════════════════════════════
static int gyrReadHi() {
  int hi = ((int)EEPROM.read(EEPROM_GYR_HI)<<8) | EEPROM.read(EEPROM_GYR_HI+1);
  return (hi>=0 && hi<99999) ? hi : 0;
}
static void gyrSaveHi(int s) {
  EEPROM.write(EEPROM_GYR_HI,   (s>>8)&0xFF);
  EEPROM.write(EEPROM_GYR_HI+1,  s    &0xFF);
  EEPROM.commit();
}

// ══════════════════════════════════════════════════════════════════════════
//  WAVE INIT
// ══════════════════════════════════════════════════════════════════════════
static void gyrInitWave() {
  for (int i = 0; i < GYR_MAX_ENEMIES;  i++) gyrEnemies[i].active   = false;
  for (int i = 0; i < GYR_MAX_PBULLETS; i++) gyrPBullets[i].active  = false;
  for (int i = 0; i < GYR_MAX_EBULLETS; i++) gyrEBullets[i].active  = false;
  for (int i = 0; i < GYR_MAX_PARTS;    i++) gyrParts[i].active     = false;
  for (int i = 0; i < GYR_MAX_ASTEROIDS;i++) gyrAsteroids[i].active = false;
  for (int i = 0; i < GYR_MAX_LASERS;   i++) gyrLasers[i].active    = false;
  gyrSatsActive = false; gyrSatsDone = false;
  gyrWaveTotal = min(10 + (gyrWave-1)*3, GYR_MAX_ENEMIES);
  gyrSpawned = 0; gyrAlive = 0;
  gyrSpawnAt = millis() + 600;
  gyrAsteroidTimer = 200 + random(120);
  gyrLaserTimer    = (gyrWave >= 3) ? (240 + random(150)) : 99999;
}

// ══════════════════════════════════════════════════════════════════════════
//  GAME INIT
// ══════════════════════════════════════════════════════════════════════════
static void gyrInitGame() {
  gyrLives = 3; gyrScore = 0; gyrWave = 1; gyrPlanet = 0;
  gyrWarpInPlanet = 1; gyrWeapon = GYR_WPN_SINGLE; gyrSmartBombs = 0;
  gyrNextLifeScore = 50000;
  gyrAngle = -(float)M_PI/2.0f; gyrSpeed = 0;
  gyrDead = false; gyrChanceMode = false;
  gyrInvincible = false; gyrInvincibleUntil = 0;
  gyrHiScore = gyrReadHi();
  gyrInitStars(); gyrInitWave();
  display->fillScreen(ARC_BLACK);
  gyrDrawHUD(); gyrAnnounceWave();
}

// ══════════════════════════════════════════════════════════════════════════
//  RESPAWN
// ══════════════════════════════════════════════════════════════════════════
static void gyrRespawn() {
  gyrAngle = -(float)M_PI/2.0f; gyrSpeed = 0; gyrDead = false;
  gyrWeapon = GYR_WPN_SINGLE;
  for (int i = 0; i < GYR_MAX_EBULLETS; i++) gyrEBullets[i].active = false;
  for (int i = 0; i < GYR_MAX_LASERS;   i++) gyrLasers[i].active   = false;
  // Blink 5 times to signal respawn
  for (int f = 0; f < 5; f++) {
    gyrDrawShip(gyrAngle, GYR_PLAYER_COL); delay(80);
    gyrDrawShip(gyrAngle, ARC_BLACK);      delay(60);
  }
  gyrDrawShip(gyrAngle, GYR_PLAYER_COL);
  // 1.5 s post-spawn invincibility
  gyrInvincible = true;
  gyrInvincibleUntil = millis() + 1500;
}

// ══════════════════════════════════════════════════════════════════════════
//  MAIN GYRUSS LOOP  — call in a while() loop; returns true when player exits
// ══════════════════════════════════════════════════════════════════════════
bool gyrussLoop() {
  static bool initialized = false;
  if (!initialized) {
    initialized = true;
    gyrPrevFrame = 0;
    gyrInitGame();
  }

  // Exit combo: hold SET + B for ~4 frames
  static int exitCombo = 0;
  if (isBtnSet() && isBtnB()) {
    if (++exitCombo >= 4) {
      exitCombo = 0; initialized = false;
      arcadeExitToMain = true;
      display->fillScreen(ARC_BLACK);
      return true;
    }
  } else { exitCombo = 0; }

  // 30 fps cap
  unsigned long now = millis();
  if (now - gyrPrevFrame < 33) return false;
  gyrPrevFrame = now;

  // ── Death sequence ────────────────────────────────────────────────────
  if (gyrDead) {
    gyrTickStars(1.0f); gyrTickParts();
    if (now - gyrDeadAt > 2200) {
      if (gyrLives <= 0) {
        if (gyrScore > gyrHiScore) { gyrHiScore = gyrScore; gyrSaveHi(gyrScore); }
        gyrOverlay("GAME OVER", "FIRE  Play Again", "B  Arcade Menu", ARC_RED);
        // Wait for all buttons to be released first (player may be holding fire)
        while (isBtnSet() || isBtnB()) delay(20);
        while (true) {
          if (isBtnSet() && !isBtnB()) { initialized = false; return false; }
          if (isBtnB()  && !isBtnSet()) { initialized=false; display->fillScreen(ARC_BLACK); return true; }
          delay(40);
        }
      }
      gyrDrawHUD(); gyrRespawn();
    }
    return false;
  }

  // ── Invincibility tick (post-respawn) ────────────────────────────────
  if (gyrInvincible) {
    if (now >= gyrInvincibleUntil) {
      gyrInvincible = false;
    } else {
      // Blink ship: visible for 2 frames, hidden for 1 frame (20-frame period ≈ 10 Hz)
      bool visible = ((now / 100) % 2 == 0);
      gyrDrawShip(gyrAngle, visible ? GYR_PLAYER_COL : ARC_BLACK);
    }
  }

  // ── Input ─────────────────────────────────────────────────────────────
  float targetSpd = 0;
  if (isBtnLeft())  targetSpd =  0.092f;   // reversed: LEFT now rotates clockwise
  if (isBtnRight()) targetSpd = -0.092f;   // reversed: RIGHT now rotates counter-clockwise
  gyrSpeed += (targetSpd - gyrSpeed) * 0.42f;
  gyrDrawShip(gyrAngle, ARC_BLACK);
  gyrAngle = gyrWrapAngle(gyrAngle + gyrSpeed);

  bool shootBtn = (isBtnSet() && !isBtnB()) || (isBtnB() && !isBtnSet());
  if (shootBtn && now - gyrLastShot > 160) { gyrFirePlayerBullets(); gyrLastShot = now; }

  static bool upWasHeld = false;
  bool upNow = isBtnUp();
  if (upNow && !upWasHeld && gyrSmartBombs > 0) gyrSmartBomb();
  upWasHeld = upNow;

  // ── Spawn next enemy ──────────────────────────────────────────────────
  if (gyrSpawned < gyrWaveTotal && now >= gyrSpawnAt) {
    for (int s = 0; s < GYR_MAX_ENEMIES; s++) {
      if (!gyrEnemies[s].active) {
        gyrSpawnEnemy(s);
        gyrSpawned++; gyrAlive++;
        gyrSpawnAt = now + max(180UL, 380UL - (unsigned long)(gyrWave * 22));
        break;
      }
    }
  }

  // ── Satellite spawn (after all enemies have entered formation) ────────
  if (!gyrSatsDone && gyrSpawned >= gyrWaveTotal) {
    bool allInFormation = true;
    for (int i = 0; i < GYR_MAX_ENEMIES; i++) {
      if (gyrEnemies[i].active && gyrEnemies[i].state == 0) { allInFormation=false; break; }
    }
    if (allInFormation && gyrAlive > 0) { gyrSpawnSatellites(); gyrSatsDone = true; }
  }

  // ── Timers ────────────────────────────────────────────────────────────
  if (--gyrAsteroidTimer <= 0) {
    gyrSpawnAsteroid();
    gyrAsteroidTimer = max(80, 180 + random(80) - gyrWave * 5);
  }
  if (gyrWave >= 3 && --gyrLaserTimer <= 0) {
    gyrSpawnLaser();
    gyrLaserTimer = max(120, 320 + random(150) - gyrWave * 10);
  }

  // ── Stars ─────────────────────────────────────────────────────────────
  gyrTickStars(1.0f);

  // ── Player bullets ────────────────────────────────────────────────────
  for (int i = 0; i < GYR_MAX_PBULLETS; i++) {
    GyrPBullet& b = gyrPBullets[i];
    if (!b.active) continue;
    float drawA = b.angle + b.offset;
    int ox = gyrPX(drawA, b.r), oy = gyrPY(drawA, b.r);
    display->fillRect(ox-1, oy-1, 3, 3, ARC_BLACK);
    b.r -= 7.5f;
    if (b.r < 3) { b.active = false; continue; }
    int nx = gyrPX(b.angle+b.offset, b.r), ny = gyrPY(b.angle+b.offset, b.r);
    uint16_t bc = (gyrWeapon==GYR_WPN_DOUBLE) ? GYR_BULL2_COL : GYR_BULLET_COL;
    display->fillRect(nx-1, ny-1, 3, 3, bc);
  }

  // ── Enemies ───────────────────────────────────────────────────────────
  for (int i = 0; i < GYR_MAX_ENEMIES; i++) {
    GyrEnemy& e = gyrEnemies[i];
    if (!e.active) continue;
    gyrDrawEnemy(e, true);

    if (e.state == 0) {
      // ENTERING: Bezier arc from side of screen to formation slot
      e.entryT += 0.011f + gyrWave * 0.0004f;
      if (e.entryT >= 1.0f) {
        e.entryT=1.0f; e.state=1; e.angle=e.formAngle; e.r=GYR_FORM_R; e.hasFired=false;
      } else {
        float t = e.entryT;
        float ease = (t<0.5f)?(4.0f*t*t*t):(1.0f-powf(-2.0f*t+2.0f,3.0f)*0.5f);
        float startAngle = e.spiralCW ? 0.0f : (float)M_PI;
        float sx = GYR_SPAWN_RADIUS*cosf(startAngle);
        float sy = GYR_SPAWN_RADIUS*sinf(startAngle);
        float ex2 = GYR_FORM_R*cosf(e.formAngle);
        float ey2 = GYR_FORM_R*sinf(e.formAngle);
        float mx = (sx+ex2)*0.5f, my = (sy+ey2)*0.5f;
        float plen = sqrtf(mx*mx+my*my);
        float cpx=0, cpy=0;
        if (plen > 0.5f) {
          float scl = (e.spiralCW?1.0f:-1.0f)*90.0f;
          cpx = mx + (-my/plen)*scl;
          cpy = my + ( mx/plen)*scl;
        }
        float u = 1.0f-ease;
        float bx = u*u*sx + 2.0f*u*ease*cpx + ease*ease*ex2;
        float by = u*u*sy + 2.0f*u*ease*cpy + ease*ease*ey2;
        e.r = sqrtf(bx*bx+by*by); e.angle = atan2f(by,bx);
      }

    } else if (e.state == 1) {
      // IN FORMATION: orbit, shoot, dive countdown
      e.formAngle = gyrWrapAngle(e.formAngle + e.vAngle);
      e.angle = e.formAngle; e.r = GYR_FORM_R;

      if (--e.shootCountdown <= 0) {
        int activeBullets = 0;
        for (int _b=0; _b<GYR_MAX_EBULLETS; _b++)
          if (gyrEBullets[_b].active) activeBullets++;
        if (activeBullets < 5) {
          float ex2 = (float)gyrPX(e.formAngle, e.r);
          float ey2 = (float)gyrPY(e.formAngle, e.r);
          int shots = (e.type == 2) ? 2 : 1;
          for (int s = 0; s < shots; s++) {
            float spread = (shots>1) ? ((s==0)?-0.15f:0.15f) : 0.0f;
            gyrFireEBullet(ex2, ey2, 0, 0, spread);
          }
          gyrSndShoot();
        }
        int baseShoot = (e.type==0)?100:(e.type==1)?75:55;
        e.shootCountdown = baseShoot + random(50) - gyrWave*2;
        if (e.shootCountdown < 30) e.shootCountdown = 30;
      }
      if (--e.diveCountdown <= 0) { e.state=2; e.hasFired=false; }

    } else if (e.state == 2) {
      // DIVING outward — steers toward player, fires once mid-dive
      float diveSpd = 2.8f + gyrWave * 0.14f;
      e.r += diveSpd;
      float diff = gyrAngleDiff(e.angle, gyrAngle);
      e.angle = gyrWrapAngle(e.angle + diff * 0.055f);

      if (!e.hasFired && e.r > GYR_FORM_R + 20) {
        e.hasFired = true;
        int activeBullets = 0;
        for (int _b=0; _b<GYR_MAX_EBULLETS; _b++)
          if (gyrEBullets[_b].active) activeBullets++;
        if (activeBullets < 4) {
          float ex2=(float)gyrPX(e.angle,e.r), ey2=(float)gyrPY(e.angle,e.r);
          gyrFireEBullet(ex2, ey2, 0, 0, 0.0f);
          gyrSndShoot();
        }
      }

      // Collision with player ring
      if (e.r >= GYR_PLAYER_R-8 && e.r <= GYR_PLAYER_R+4) {
        float angDist = fabsf(gyrAngleDiff(e.angle, gyrAngle));
        if (angDist < 0.28f && !gyrDead && !gyrChanceMode && !gyrInvincible) {
          bool playerFiring = false;
          for (int _b=0; _b<GYR_MAX_PBULLETS; _b++)
            if (gyrPBullets[_b].active) { playerFiring=true; break; }
          if (playerFiring) {
            gyrBurst(e.angle, e.r, gyrEneColor(e.type), 12);
            gyrBurst(e.angle, e.r, GYR_EXPL_A, 6);
            gyrSndExplode(); e.active=false; gyrAlive--;
            int pts=(e.type==0)?200:(e.type==1)?400:800;
            gyrScore+=pts; if(gyrScore>gyrHiScore) gyrHiScore=gyrScore;
            gyrDrawHUD();
          } else {
            gyrDead=true; gyrDeadAt=now; gyrLives--;
            gyrDrawShip(gyrAngle, ARC_BLACK);
            gyrBurst(gyrAngle,GYR_PLAYER_R,GYR_PLAYER_COL,22);
            gyrBurst(gyrAngle,GYR_PLAYER_R,ARC_WHITE,10);
            gyrSndPlayerDie(); gyrDrawHUD();
          }
        }
      }

      // Off-screen → return arc from centre
      if (e.r > GYR_SPAWN_RADIUS + 10) {
        e.state=3; e.r=4.0f; e.spiralCW=!e.spiralCW; e.entryT=0; e.hasFired=false;
      }

    } else if (e.state == 3) {
      // RETURNING: Bezier arc from near-centre back to formation slot
      e.entryT += 0.016f + gyrWave * 0.0004f;
      if (e.entryT >= 1.0f) {
        e.state=1; e.r=GYR_FORM_R; e.angle=e.formAngle;
        e.diveCountdown = max(80, 260+random(180)-gyrWave*8);
      } else {
        float t = e.entryT;
        float ease = (t<0.5f)?(4.0f*t*t*t):(1.0f-powf(-2.0f*t+2.0f,3.0f)*0.5f);
        float startAngle = e.spiralCW ? 0.0f : (float)M_PI;
        float sx = 4.0f*cosf(startAngle), sy = 4.0f*sinf(startAngle);
        float ex2=GYR_FORM_R*cosf(e.formAngle), ey2=GYR_FORM_R*sinf(e.formAngle);
        float mx=(sx+ex2)*0.5f, my=(sy+ey2)*0.5f;
        float plen=sqrtf(mx*mx+my*my);
        float cpx=(plen>0.5f)?mx+(-my/plen)*(e.spiralCW?50.0f:-50.0f):0;
        float cpy=(plen>0.5f)?my+( mx/plen)*(e.spiralCW?50.0f:-50.0f):0;
        float u=1.0f-ease;
        float bx=u*u*sx+2.0f*u*ease*cpx+ease*ease*ex2;
        float by=u*u*sy+2.0f*u*ease*cpy+ease*ease*ey2;
        e.r=sqrtf(bx*bx+by*by); e.angle=atan2f(by,bx);
      }

    } else if (e.state == 4) {
      // CENTRE-SPAWN: enemy radiates straight out from the centre to formation
      // Uses a simple linear radius expansion with a clockwise spiral offset
      e.entryT += 0.018f + gyrWave * 0.0004f;
      if (e.entryT >= 1.0f) {
        e.state=1; e.r=GYR_FORM_R; e.angle=e.formAngle;
        e.diveCountdown = max(80, 260+random(200)-gyrWave*10);
      } else {
        float t = e.entryT;
        // Ease out: fast start, gentle landing
        float ease = 1.0f - powf(1.0f - t, 2.5f);
        // Radius grows from 0 to GYR_FORM_R
        e.r = ease * (float)GYR_FORM_R;
        // Angle spirals from opposite side of formation to formAngle
        float startA = gyrWrapAngle(e.formAngle + (float)M_PI);
        float diff   = gyrAngleDiff(startA, e.formAngle);
        e.angle = gyrWrapAngle(startA + diff * ease);
      }
    }

    // Player bullet vs enemy
    for (int b=0; b<GYR_MAX_PBULLETS; b++) {
      if (!gyrPBullets[b].active) continue;
      float drawA = gyrPBullets[b].angle + gyrPBullets[b].offset;
      float bx=(float)gyrPX(drawA,gyrPBullets[b].r);
      float by=(float)gyrPY(drawA,gyrPBullets[b].r);
      float ex2=(float)gyrPX(e.angle,e.r), ey2=(float)gyrPY(e.angle,e.r);
      float dx=bx-ex2, dy=by-ey2;
      if (dx*dx+dy*dy < 144.0f) {
        display->fillRect((int)bx-1,(int)by-1,3,3,ARC_BLACK);
        gyrPBullets[b].active = false;
        e.hp--;
        if (e.hp > 0) {
          gyrBurst(e.angle,e.r,gyrEneColor(e.type),5); gyrSndHit();
        } else {
          gyrBurst(e.angle,e.r,gyrEneColor(e.type),14);
          gyrBurst(e.angle,e.r,GYR_EXPL_A,8);
          gyrSndExplode(); e.active=false; gyrAlive--;
          int pts=(e.type==0)?100:(e.type==1)?200:400;
          if (e.state==2) pts*=2;
          gyrScore+=pts; if(gyrScore>gyrHiScore) gyrHiScore=gyrScore;
          if (gyrScore >= gyrNextLifeScore) {
            gyrNextLifeScore += 50000;
            if (gyrLives<5) { gyrLives++; gyrSndExtraLife(); }
          }
          gyrDrawHUD();
        }
        break;
      }
    }
    if (e.active) gyrDrawEnemy(e, false);
  }

  // ── Enemy bullets ─────────────────────────────────────────────────────
  for (int i = 0; i < GYR_MAX_EBULLETS; i++) {
    GyrEBullet& b = gyrEBullets[i];
    if (!b.active) continue;
    display->fillRect((int)b.x-1,(int)b.y-1,3,3,ARC_BLACK);
    b.x += b.vx; b.y += b.vy;
    float distC = sqrtf((b.x-GYR_CX)*(b.x-GYR_CX)+(b.y-GYR_CY)*(b.y-GYR_CY));
    if (b.x<0||b.x>SCREEN_WIDTH||b.y<0||b.y>SCREEN_HEIGHT||distC>GYR_PLAYER_R+14) {
      b.active=false; continue;
    }
    if (!gyrDead && !gyrChanceMode && !gyrInvincible) {
      float px=(float)gyrPX(gyrAngle,GYR_PLAYER_R), py=(float)gyrPY(gyrAngle,GYR_PLAYER_R);
      float dx=b.x-px, dy=b.y-py;
      if (dx*dx+dy*dy < 144.0f) {
        b.active=false; gyrDead=true; gyrDeadAt=now; gyrLives--;
        gyrDrawShip(gyrAngle,ARC_BLACK);
        gyrBurst(gyrAngle,GYR_PLAYER_R,GYR_PLAYER_COL,22);
        gyrBurst(gyrAngle,GYR_PLAYER_R,ARC_WHITE,10);
        gyrSndPlayerDie(); gyrDrawHUD(); continue;
      }
    }
    display->fillRect((int)b.x-1,(int)b.y-1,3,3,GYR_EBUL_COL);
  }

  // ── Satellites / asteroids / lasers ──────────────────────────────────
  if (gyrSatsActive) gyrTickSatellites();
  gyrTickAsteroids();
  gyrTickLasers();

  // ── Draw player ship ──────────────────────────────────────────────────
  if (!gyrDead) gyrDrawShip(gyrAngle, GYR_PLAYER_COL);

  // ── Particles ─────────────────────────────────────────────────────────
  gyrTickParts();

  // ── Redraw HUD strip (ship erase circle can reach into top 18px) ──────
  gyrDrawHUD();

  // ── Wave clear ────────────────────────────────────────────────────────
  // If all enemies are gone, dismiss any lingering satellites immediately
  // so the wave-clear condition triggers without waiting for them to expire.
  if (gyrSpawned >= gyrWaveTotal && gyrAlive == 0 && gyrSatsActive) {
    for (int i = 0; i < GYR_NUM_SATS; i++) {
      if (gyrSats[i].active) { gyrDrawSatellite(i, true); gyrSats[i].active = false; }
    }
    gyrSatsActive = false;
  }

  if (gyrSpawned >= gyrWaveTotal && gyrAlive == 0 && !gyrDead && !gyrSatsActive) {
    gyrSndLevelUp();
    gyrWarpInPlanet++; gyrWave++;

    // Bonus life on reaching stage 7 and stage 15
    if ((gyrWave == 7 || gyrWave == 15) && gyrLives < 5) {
      gyrLives++; gyrSndExtraLife(); gyrDrawHUD();
    }
    if (gyrWarpInPlanet > 3) {
      gyrWarpInPlanet = 1;
      gyrPlanet = (gyrPlanet + 1) % 7;
      display->fillScreen(ARC_BLACK);
      gyrDrawHUD(); gyrDrawShip(gyrAngle, GYR_PLAYER_COL);
      gyrRunChanceStage();
    }
    if (gyrWarpInPlanet==1 && gyrWave%9==1 && gyrLives<5) { gyrLives++; gyrSndPowerup(); }
    gyrInitWave();
    display->fillScreen(ARC_BLACK);
    gyrDrawHUD(); gyrDrawShip(gyrAngle, GYR_PLAYER_COL);
    gyrAnnounceWave();
  }

  return false;
}
