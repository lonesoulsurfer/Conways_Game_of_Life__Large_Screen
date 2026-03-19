// sw_types.h — shared types for StarWars.ino
// The Arduino IDE auto-generates function prototypes from .ino files but NOT from .h files.
// Putting TuneID and Note here prevents the "does not name a type" prototype error.
#pragma once

enum TuneID {
  TUNE_NONE = 0,
  TUNE_MAIN_THEME,
  TUNE_BINARY_SUNSET,
  TUNE_IMPERIAL_MARCH,
  TUNE_VICTORY_FANFARE,
  TUNE_GAME_OVER_JINGLE,
  SFX_LASER,
  SFX_EXPLOSION,
  SFX_HIT_PLAYER,
  SFX_POWER_UP,
  SFX_FORCE,
  SFX_EXHAUST_LOCK,
  SFX_PROTON_TORPEDO,
  SFX_WOMP_RAT,
  SFX_TIE_FLYBY,
  SFX_SHAFT_WHOOSH,   // Proton torpedo screaming down the exhaust shaft
};

struct Note { uint16_t freq; uint16_t dur; };
