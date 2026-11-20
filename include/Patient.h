// Patient.h
// A patient with a condition level from 0 (healthy) to 10 (critical).
// If a waiting patient is not seen within their time limit they enter a
// critical state (condition 10).
#pragma once

#include <string>

#include "Person.h"

class Doctor;

enum class PatientStatus {
    NotArrived,    // listed in the dataset but has not come in yet
    Waiting,       // in the waiting room / triage queue
    InTreatment,   // being seen by a doctor
    Discharged     // treated and has left the hospital
};

const char* toString(PatientStatus status);

class Patient : public Person {
public:
    static constexpr int kHealthy = 0;
    static constexpr int kCritical = 10;

    Patient(std::string name, int age, std::string dob, int condition, std::string ailment,
            int arrivalTick, int criticalLimitTicks);

    int condition() const { return condition_; }
    int initialCondition() const { return initialCondition_; }
    bool isCritical() const { return condition_ >= kCritical; }
    bool wentCritical() const { return wentCritical_; }   // became critical while waiting
    const std::string& ailment() const { return ailment_; }
    int arrivalTick() const { return arrivalTick_; }
    int criticalLimitTicks() const { return criticalLimitTicks_; }

    PatientStatus status() const { return status_; }
    Doctor* doctor() const { return doctor_; }
    int treatmentStartTick() const { return treatmentStartTick_; }
    int dischargeTick() const { return dischargeTick_; }

    // Ticks spent waiting: up to now if still waiting, else until treatment began.
    int ticksWaited(int now) const;
    // Ticks left before this waiting patient becomes critical (0 if already critical).
    int ticksUntilCritical(int now) const;

    // State changes, driven by Hospital.
    void admit(int tick);
    void becomeCritical();
    void startTreatment(Doctor* doctor, int tick);
    void discharge(int tick);

private:
    int condition_;
    int initialCondition_;
    std::string ailment_;
    int arrivalTick_;
    int criticalLimitTicks_;

    PatientStatus status_ = PatientStatus::NotArrived;
    bool wentCritical_ = false;
    Doctor* doctor_ = nullptr;
    int admitTick_ = -1;
    int treatmentStartTick_ = -1;
    int dischargeTick_ = -1;
};
