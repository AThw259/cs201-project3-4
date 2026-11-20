// SimTime.cpp
#include "SimTime.h"

#include <cstdio>
#include <stdexcept>

namespace simtime {

std::string formatHHMM(int minuteOfDay) {
    char buf[8];
    std::snprintf(buf, sizeof buf, "%02d:%02d", (minuteOfDay / 60) % 24, minuteOfDay % 60);
    return buf;
}

std::string formatClock(int tick) {
    return "Day " + std::to_string(dayOf(tick)) + " " + formatHHMM(minuteOfDay(tick));
}

std::string formatDuration(int ticks) {
    int minutes = ticks * kMinutesPerTick;
    if (minutes < 60) return std::to_string(minutes) + "m";
    std::string s = std::to_string(minutes / 60) + "h";
    if (minutes % 60) s += " " + std::to_string(minutes % 60) + "m";
    return s;
}

int parseHHMM(const std::string& text) {
    int h = -1, m = -1;
    char extra = 0;
    if (std::sscanf(text.c_str(), "%d:%d%c", &h, &m, &extra) != 2 || h < 0 || h > 23 || m < 0 || m > 59)
        throw std::invalid_argument("expected a time like 07:30 but got \"" + text + "\"");
    return h * 60 + m;
}

}  // namespace simtime
