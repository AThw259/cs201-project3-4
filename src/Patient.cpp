// Patient.cpp
#include "Patient.h"

#include <algorithm>

const char* toString(PatientStatus status) {
    switch (status) {
        case PatientStatus::NotArrived: return "Not arrived";
        case PatientStatus::Waiting: return "Waiting";
        case PatientStatus::InTreatment: return "In treatment";
        case PatientStatus::Discharged: return "Discharged";
    }
    return "?";
}

Patient::Patient(std::string name, int age, std::string dob, int condition, std::string ailment,
                 int arrivalTick, int criticalLimitTicks)
    : Person(std::move(name), age, std::move(dob)),
      condition_(std::max(kHealthy, std::min(kCritical, condition))),
      initialCondition_(condition_),
      ailment_(std::move(ailment)),
      arrivalTick_(arrivalTick),
      criticalLimitTicks_(std::max(1, criticalLimitTicks)) {}

int Patient::ticksWaited(int now) const {
    if (admitTick_ < 0) return 0;
    int end = (treatmentStartTick_ >= 0) ? treatmentStartTick_ : now;
    return std::max(0, end - admitTick_);
}

int Patient::ticksUntilCritical(int now) const {
    if (isCritical() || status_ != PatientStatus::Waiting) return 0;
    return std::max(0, criticalLimitTicks_ - ticksWaited(now));
}

void Patient::admit(int tick) {
    status_ = PatientStatus::Waiting;
    admitTick_ = tick;
}

void Patient::becomeCritical() {
    if (!isCritical()) wentCritical_ = true;
    condition_ = kCritical;
}

void Patient::startTreatment(Doctor* doctor, int tick) {
    status_ = PatientStatus::InTreatment;
    doctor_ = doctor;
    treatmentStartTick_ = tick;
}

void Patient::discharge(int tick) {
    status_ = PatientStatus::Discharged;
    condition_ = kHealthy;
    dischargeTick_ = tick;
}
