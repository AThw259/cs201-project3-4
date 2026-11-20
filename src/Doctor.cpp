// Doctor.cpp
#include "Doctor.h"

#include <algorithm>

#include "Patient.h"
#include "SimTime.h"

Doctor::Doctor(std::string name, int age, std::string dob, int skill, int shiftStartMinute, int shiftEndMinute)
    : Person(std::move(name), age, std::move(dob)),
      skill_(std::max(kMinSkill, std::min(kMaxSkill, skill))),
      shiftStart_(shiftStartMinute),
      shiftEnd_(shiftEndMinute) {}

std::string Doctor::shiftString() const {
    return simtime::formatHHMM(shiftStart_) + "-" + simtime::formatHHMM(shiftEnd_);
}

bool Doctor::isOnShift(int minuteOfDay) const {
    if (shiftStart_ == shiftEnd_) return true;   // 24-hour shift
    if (shiftStart_ < shiftEnd_) return minuteOfDay >= shiftStart_ && minuteOfDay < shiftEnd_;
    return minuteOfDay >= shiftStart_ || minuteOfDay < shiftEnd_;   // overnight
}

int Doctor::treatmentTicksFor(const Patient& patient) const {
    // One tick of setup plus work that grows with the condition and shrinks
    // with skill. Level 10 with skill 10 -> 3 ticks (45 m); with skill 3 -> 9
    // ticks (2 h 15 m). Level 2 with skill 9 -> 2 ticks (30 m).
    int work = std::max(1, patient.condition()) * (12 - skill_);
    return 1 + (work + 11) / 12;
}

void Doctor::beginTreatment(Patient* patient, int ticks) {
    patient_ = patient;
    ticksRemaining_ = std::max(1, ticks);
}

Patient* Doctor::advanceTreatment() {
    if (!patient_) return nullptr;
    if (--ticksRemaining_ > 0) return nullptr;
    Patient* done = patient_;
    patient_ = nullptr;
    ticksRemaining_ = 0;
    ++patientsSeen_;
    return done;
}
