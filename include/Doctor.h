// Doctor.h
// A doctor with a skill level (1-10) and a daily shift. Higher skill means a
// shorter treatment time. Doctors only start new patients while on shift.
#pragma once

#include <string>

#include "Person.h"

class Patient;

class Doctor : public Person {
public:
    static constexpr int kMinSkill = 1;
    static constexpr int kMaxSkill = 10;

    // Shift times are minutes after midnight. A shift whose end is earlier
    // than its start runs overnight (e.g. 22:00-06:00).
    Doctor(std::string name, int age, std::string dob, int skill, int shiftStartMinute, int shiftEndMinute);

    int skill() const { return skill_; }
    int shiftStart() const { return shiftStart_; }
    int shiftEnd() const { return shiftEnd_; }
    std::string shiftString() const;   // "07:00-19:00"
    bool isOnShift(int minuteOfDay) const;

    bool isBusy() const { return patient_ != nullptr; }
    Patient* currentPatient() const { return patient_; }
    int ticksRemaining() const { return ticksRemaining_; }
    int patientsSeen() const { return patientsSeen_; }

    // How many ticks this doctor needs for a patient: sicker patients take
    // longer and more skilled doctors work faster.
    int treatmentTicksFor(const Patient& patient) const;

    void beginTreatment(Patient* patient, int ticks);
    // Advances the current treatment by one tick. Returns the patient when
    // the treatment finishes (the doctor is then free), otherwise nullptr.
    Patient* advanceTreatment();

private:
    int skill_;
    int shiftStart_;
    int shiftEnd_;

    Patient* patient_ = nullptr;
    int ticksRemaining_ = 0;
    int patientsSeen_ = 0;
};
