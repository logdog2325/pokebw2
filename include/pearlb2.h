#ifndef POKEBW2_PEARLB2_H
#define POKEBW2_PEARLB2_H

// PearlB2 (Pokémon Pearl rebuilt inside Black 2): its changes to the game's code, on the pearlb2 branch. The ROM
// hack's tools build these binaries, then edit them further with binary patches at fixed addresses (PearlB2's
// notes/DECOMP_WORKFLOW.md). So a change here must keep every function where it is:
// - a change in place keeps its function's size (a constant, a table value, the function a call goes to), and the
//   original stays beside it, `#if PEARLB2 ... #else <the original> #endif`;
// - new code goes to src/pearlb2/*_itcm.c, which configure.py links after the game's own ITCM code, the end of the
//   memory map (tools/pearlb2/link_itcm.py). Each file's code is inside #if PEARLB2.
// - data that PearlB2's tools generate from its world (zone tables) isn't compiled in: the code reads it through a
//   pointer in ITCM that the hack's build fills in (NULL: the game's behaviour).
// With PEARLB2 0 every file compiles to the game's code and the build matches the original ROM again (PearlB2's
// `python3 b2tools/decomp.py vanilla`).
#define PEARLB2 1

#include "types.h"
#include "struct_decls.h"

// Sinnoh's weather by season (src/pearlb2/season_weather_itcm.c), asked first by GetWeatherAll in place of
// GetDefaultWeatherValue: the weather a Sinnoh zone has today, or WEATHER_NONE for the game's rules
u32 PearlB2_SeasonWeather(GameData *gameData, u16 zoneId);

#endif // POKEBW2_PEARLB2_H
