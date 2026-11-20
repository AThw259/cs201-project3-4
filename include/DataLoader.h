// DataLoader.h
// Reads the CSV datasets in data/. Lines that are blank or start with '#'
// are ignored; the first non-comment line is the header row.
//
// doctors.csv : name,age,dob,skill,shift_start,shift_end
// patients.csv: name,age,dob,condition,ailment,arrival_day,arrival_time,time_limit_min
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Doctor.h"
#include "Patient.h"

namespace dataloader {

// Both functions throw std::runtime_error with the file name and line number
// if the file cannot be opened or a row is malformed.
std::vector<std::unique_ptr<Doctor>> loadDoctors(const std::string& path);
std::vector<std::unique_ptr<Patient>> loadPatients(const std::string& path);

}  // namespace dataloader
