#include "pearlb2.h"
#include "types.h"
#include "field/encounter.h"
#include "field/zone.h"
#include "nitro/rtc.h"
#include "system/game_data.h"
#include "system/season.h"
#include "system/zone_weather.h"

// PearlB2: Sinnoh's weather by season (PearlB2's patches/season_weather.py fills in gPearlB2SeasonWeather).
//
// GetWeatherAll asks this first, for every zone change and warp, in place of GetDefaultWeatherValue (a stub that
// says WEATHER_NONE). Its answer for a Sinnoh zone keeps the Unova zone it reuses from taking that zone's Entralink,
// Route 7, birthday or calendar weather (Sandgem had Lentimas Town's sandstorm on about half the days):
//   kind 1 (rooms, caves, buildings, zones with a Pearl weather of their own): the zone header's weather;
//   kind 2 (outdoors): in winter, on the snow days of a Black 2 zone's calendar year (snowDays, 2 bits a day), its
//     snow; every other day and season the header's weather;
//   kind 3 (the north with Pearl's day-by-day weather: Route 216, Acuity Lakefront, Snowpoint): Pearl's weather of the
//     day (northExc*: the days that differ from the header's), in every season;
//   kind 0 (not Sinnoh's): WEATHER_NONE, so the game goes on as before.
// Winter is GameData_GetSeason's season (spring all year with PearlB2's seasons off) and the clock's both 3, as the
// game uses its calendar only while they agree. The day is the cached RTC date counted as the game's calendar counts
// it (February always 29 days; Pearl's table counts the same way). Without the table, the game's behaviour.

#if PEARLB2

#define WINTER 3

// What the hack's build writes (2 bits a zone or day, the low bits first)
typedef struct {
    u16 zoneCount;
    u16 northExcCount;
    const u8 *zoneKind;
    const u8 *snowDays;
    const u16 *snowWeather;
    const u16 *northExcZone;
    const u16 *northExcDay;
    const u16 *northExcWeather;
} PearlB2SeasonWeather;

#pragma explicit_zero_data on
PearlB2SeasonWeather *gPearlB2SeasonWeather = NULL;
#pragma explicit_zero_data reset

static const u16 sMonthStart[13] = { 0, 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335 };

static u32 DayOfYear(void) {
    RTCDate date;

    RTC_GetCachedDate(&date);
    if (date.month < 1 || date.month > 12 || date.day < 1) {
        return 0;
    }
    return sMonthStart[date.month] + date.day - 1;
}

u32 PearlB2_SeasonWeather(GameData *gameData, u16 zoneId) {
    const PearlB2SeasonWeather *table = gPearlB2SeasonWeather;
    u32 kind, day, i, code;

    if (table == NULL) {
        return GetDefaultWeatherValue(gameData, zoneId);
    }
    if (zoneId >= table->zoneCount) {
        return WEATHER_NONE;
    }
    kind = (table->zoneKind[zoneId >> 2] >> ((zoneId & 3) * 2)) & 3;
    if (kind == 0) {
        return WEATHER_NONE;
    }
    if (kind == 3) {
        day = DayOfYear();
        for (i = 0; i < table->northExcCount; i++) {
            if (table->northExcZone[i] == zoneId && table->northExcDay[i] == day) {
                return table->northExcWeather[i];
            }
        }
    } else if (kind == 2 && GameData_GetSeason(gameData) == WINTER && Season_GetRealTime() == WINTER) {
        day = DayOfYear();
        code = (table->snowDays[day >> 2] >> ((day & 3) * 2)) & 3;
        if (code) {
            return table->snowWeather[code];
        }
    }
    return GetZoneEnvFlagsWeather(zoneId);
}

#endif
