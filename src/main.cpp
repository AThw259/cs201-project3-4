// main.cpp
// Entry point for the SFML version of the Nota Hospital simulation.
//
//   ./hospital                                  uses data/doctors.csv and data/patients.csv
//   ./hospital my_doctors.csv my_patients.csv   uses other data files
#include <iostream>

#include "Gui.h"
#include "Hospital.h"

int main(int argc, char* argv[]) {
    std::string doctorsCsv = argc > 1 ? argv[1] : "data/doctors.csv";
    std::string patientsCsv = argc > 2 ? argv[2] : "data/patients.csv";

    std::unique_ptr<Hospital> hospital;
    try {
        hospital = Hospital::fromFiles(doctorsCsv, patientsCsv);
    } catch (const std::exception& e) {
        std::cerr << "Error loading data: " << e.what() << '\n';
        return 1;
    }

    HospitalGui gui(*hospital);
    return gui.run();
}
