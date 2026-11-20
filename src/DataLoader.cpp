// DataLoader.cpp
#include "DataLoader.h"

#include <cstdio>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include "SimTime.h"

namespace dataloader {
namespace {

std::vector<std::string> splitCsv(const std::string& line) {
    std::vector<std::string> fields;
    std::stringstream ss(line);
    std::string field;
    while (std::getline(ss, field, ',')) fields.push_back(trim(field));
    return fields;
}

int toInt(const std::string& text, const char* what) {
    std::size_t used = 0;
    int value = 0;
    try {
        value = std::stoi(text, &used);
    } catch (const std::exception&) {
        used = 0;
    }
    if (used == 0 || used != text.size())
        throw std::invalid_argument(std::string(what) + " must be a whole number, got \"" + text + "\"");
    return value;
}

void checkRange(int value, int lo, int hi, const char* what) {
    if (value < lo || value > hi)
        throw std::invalid_argument(std::string(what) + " must be between " + std::to_string(lo) + " and " +
                                    std::to_string(hi));
}

void checkDate(const std::string& dob) {
    int y, m, d;
    char extra;
    if (dob.size() != 10 || std::sscanf(dob.c_str(), "%d-%d-%d%c", &y, &m, &d, &extra) != 3 || m < 1 || m > 12 ||
        d < 1 || d > 31)
        throw std::invalid_argument("date of birth must look like YYYY-MM-DD, got \"" + dob + "\"");
}

// Calls handleRow(fields) for each data row, adding file/line info to errors.
template <typename Fn>
void readRows(const std::string& path, std::size_t expectedFields, Fn handleRow) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("could not open " + path);
    std::string line;
    int lineNo = 0;
    bool headerSeen = false;
    while (std::getline(in, line)) {
        ++lineNo;
        std::string t = trim(line);
        if (t.empty() || t[0] == '#') continue;
        if (!headerSeen) {
            headerSeen = true;
            continue;
        }
        std::vector<std::string> fields = splitCsv(t);
        try {
            if (fields.size() != expectedFields)
                throw std::invalid_argument("expected " + std::to_string(expectedFields) + " fields, found " +
                                            std::to_string(fields.size()));
            handleRow(fields);
        } catch (const std::exception& e) {
            throw std::runtime_error(path + ":" + std::to_string(lineNo) + ": " + e.what());
        }
    }
}

}  // namespace

std::vector<std::unique_ptr<Doctor>> loadDoctors(const std::string& path) {
    std::vector<std::unique_ptr<Doctor>> doctors;
    readRows(path, 6, [&](const std::vector<std::string>& f) {
        int age = toInt(f[1], "age");
        checkRange(age, 18, 100, "age");
        checkDate(f[2]);
        int skill = toInt(f[3], "skill");
        checkRange(skill, Doctor::kMinSkill, Doctor::kMaxSkill, "skill");
        doctors.emplace_back(new Doctor(f[0], age, f[2], skill, simtime::parseHHMM(f[4]), simtime::parseHHMM(f[5])));
    });
    return doctors;
}

std::vector<std::unique_ptr<Patient>> loadPatients(const std::string& path) {
    std::vector<std::unique_ptr<Patient>> patients;
    readRows(path, 8, [&](const std::vector<std::string>& f) {
        int age = toInt(f[1], "age");
        checkRange(age, 0, 120, "age");
        checkDate(f[2]);
        int condition = toInt(f[3], "condition");
        checkRange(condition, Patient::kHealthy, Patient::kCritical, "condition");
        int day = toInt(f[5], "arrival_day");
        checkRange(day, 1, 365, "arrival_day");
        int arrival = simtime::toTick(day, simtime::parseHHMM(f[6]));
        int limitMinutes = toInt(f[7], "time_limit_min");
        checkRange(limitMinutes, 1, 7 * simtime::kMinutesPerDay, "time_limit_min");
        patients.emplace_back(
            new Patient(f[0], age, f[2], condition, f[4], arrival, simtime::minutesToTicks(limitMinutes)));
    });
    return patients;
}

}  // namespace dataloader
