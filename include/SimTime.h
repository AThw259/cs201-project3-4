// SimTime.h
// Simulation clock helpers. Time only moves when the user presses a button;
// each press is one "tick" of kMinutesPerTick minutes of hospital time.
#pragma once

#include <string>

namespace simtime {

constexpr int kMinutesPerTick = 15;
constexpr int kMinutesPerDay = 24 * 60;
constexpr int kTicksPerDay = kMinutesPerDay / kMinutesPerTick;   // 96
constexpr int kTicksPerHour = 60 / kMinutesPerTick;              // 4

// Tick 0 is Day 1, 00:00.
inline int minuteOfDay(int tick) { return (tick % kTicksPerDay) * kMinutesPerTick; }
inline int dayOf(int tick) { return tick / kTicksPerDay + 1; }
inline int toTick(int day, int minuteOfDay) { return (day - 1) * kTicksPerDay + minuteOfDay / kMinutesPerTick; }

// Rounds a duration in minutes up to whole ticks (at least 1).
inline int minutesToTicks(int minutes) {
    int t = (minutes + kMinutesPerTick - 1) / kMinutesPerTick;
    return t < 1 ? 1 : t;
}

std::string formatHHMM(int minuteOfDay);   // 375 -> "06:15"
std::string formatClock(int tick);         // "Day 1 06:15"
std::string formatDuration(int ticks);     // 5 -> "1h 15m"
int parseHHMM(const std::string& text);    // "06:15" -> 375, throws on bad input

}  // namespace simtime
