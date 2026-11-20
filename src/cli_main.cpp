// cli_main.cpp
// Console version of the simulation. It uses the same Hospital engine as the
// SFML GUI and is handy for quick testing or on machines without SFML.
//
//   ./hospital_cli                      interactive (press Enter to advance)
//   ./hospital_cli --run 96             run 96 ticks, print the log and a summary
//   ./hospital_cli doctors.csv patients.csv [--run N]
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "Hospital.h"

namespace {

std::size_t printedLines = 0;   // how much of the event log is already on screen

void printNewLogLines(const Hospital& h) {
    const auto& log = h.log();
    // The log is capped, so if lines were dropped just print what is left.
    if (printedLines > log.size()) printedLines = 0;
    for (std::size_t i = printedLines; i < log.size(); ++i) std::cout << log[i] << '\n';
    printedLines = log.size();
}

void printWaiting(const Hospital& h) {
    auto waiting = h.waitingInPriorityOrder();
    std::cout << "Waiting room (" << waiting.size() << ", in the order they will be seen):\n";
    for (const Patient* p : waiting) {
        std::cout << "  level " << std::setw(2) << p->condition() << "  " << std::left << std::setw(18) << p->name()
                  << std::right << " waited " << std::setw(7) << simtime::formatDuration(p->ticksWaited(h.tick()));
        if (p->isCritical()) std::cout << "  CRITICAL";
        else std::cout << "  critical in " << simtime::formatDuration(p->ticksUntilCritical(h.tick()));
        std::cout << "  (" << p->ailment() << ")\n";
    }
}

void printDoctors(const Hospital& h) {
    const int now = simtime::minuteOfDay(h.tick());
    std::cout << "Doctors:\n";
    for (const auto& d : h.doctors()) {
        std::cout << "  Dr. " << std::left << std::setw(15) << d->name() << std::right << " skill " << std::setw(2)
                  << d->skill() << "  " << d->shiftString() << "  ";
        if (d->isBusy())
            std::cout << "treating " << d->currentPatient()->name() << " ("
                      << simtime::formatDuration(d->ticksRemaining()) << " left)";
        else if (d->isOnShift(now))
            std::cout << "available";
        else
            std::cout << "off shift";
        std::cout << '\n';
    }
}

void printStats(const Hospital& h) {
    HospitalStats s = h.stats();
    std::cout << h.clock() << " | waiting " << s.waiting << " | in treatment " << s.inTreatment << " | discharged "
              << s.discharged << " | not arrived " << s.notYetArrived << " | went critical " << s.wentCritical
              << " | avg wait " << std::fixed << std::setprecision(0)
              << s.averageWaitTicks * simtime::kMinutesPerTick << " min"
              << " | doctors on shift " << s.doctorsOnShift << '\n';
    std::cout << "Patient B-Tree: " << h.patientTree().size() << " keys, height " << h.patientTree().height()
              << ", " << h.patientTree().nodeCount() << " nodes.  Doctor B-Tree: " << h.doctorTree().size()
              << " keys, height " << h.doctorTree().height() << ".\n";
}

void printLookup(const Hospital& h, const std::string& query) {
    LookupResult r = h.lookup(query);
    std::cout << r.message;
    if (r.exact) std::cout << " (patient tree: " << r.patientNodesVisited << " node(s) visited)";
    std::cout << '\n';
    for (const Patient* p : r.patients) {
        std::cout << "  Patient " << p->key().toString() << " - " << toString(p->status()) << ", level "
                  << p->condition() << ", " << p->ailment() << ", arrived "
                  << simtime::formatClock(p->arrivalTick());
        if (p->doctor()) std::cout << ", seen by Dr. " << p->doctor()->name();
        std::cout << '\n';
    }
    for (const Doctor* d : r.doctors)
        std::cout << "  Doctor  " << d->key().toString() << " - skill " << d->skill() << ", shift "
                  << d->shiftString() << ", patients seen " << d->patientsSeen() << '\n';
}

void printHelp() {
    std::cout << "Commands:\n"
                 "  [Enter] or n     advance one tick (15 minutes)\n"
                 "  n <k>            advance k ticks\n"
                 "  w                show the waiting room in priority order\n"
                 "  d                show doctors\n"
                 "  s                show statistics\n"
                 "  f <query>        look up a patient/doctor: a name prefix, or name, age, YYYY-MM-DD\n"
                 "  h                help\n"
                 "  q                quit\n";
}

}  // namespace

int main(int argc, char* argv[]) {
    std::string doctorsCsv = "data/doctors.csv";
    std::string patientsCsv = "data/patients.csv";
    int runTicks = -1;

    int positional = 0;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--run" && i + 1 < argc) {
            runTicks = std::stoi(argv[++i]);
        } else if (positional == 0) {
            doctorsCsv = arg;
            ++positional;
        } else if (positional == 1) {
            patientsCsv = arg;
            ++positional;
        }
    }

    std::unique_ptr<Hospital> hospital;
    try {
        hospital = Hospital::fromFiles(doctorsCsv, patientsCsv);
    } catch (const std::exception& e) {
        std::cerr << "Error loading data: " << e.what() << '\n';
        return 1;
    }
    Hospital& h = *hospital;

    if (runTicks >= 0) {   // batch mode
        h.advance(runTicks);
        printNewLogLines(h);
        std::cout << '\n';
        printStats(h);
        return 0;
    }

    std::cout << "Nota Hospital ER simulation (console). Each tick is " << simtime::kMinutesPerTick
              << " minutes.\n";
    printHelp();
    printNewLogLines(h);

    std::string line;
    while (true) {
        std::cout << '\n' << h.clock() << " > " << std::flush;
        if (!std::getline(std::cin, line)) break;
        std::istringstream in(line);
        std::string cmd;
        in >> cmd;
        if (cmd.empty() || cmd == "n") {
            int k = 1;
            in >> k;
            h.advance(k < 1 ? 1 : k);
            printNewLogLines(h);
            if (h.finished()) std::cout << "All patients have been treated and discharged.\n";
        } else if (cmd == "w") {
            printWaiting(h);
        } else if (cmd == "d") {
            printDoctors(h);
        } else if (cmd == "s") {
            printStats(h);
        } else if (cmd == "f") {
            std::string rest;
            std::getline(in, rest);
            printLookup(h, rest);
        } else if (cmd == "q") {
            break;
        } else {
            printHelp();
        }
    }
    return 0;
}
