// PersonKey.cpp
#include "PersonKey.h"

#include <cctype>
#include <climits>
#include <tuple>

std::string toLower(std::string s) {
    for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

std::string trim(const std::string& s) {
    std::size_t b = 0, e = s.size();
    while (b < e && std::isspace(static_cast<unsigned char>(s[b]))) ++b;
    while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1]))) --e;
    return s.substr(b, e - b);
}

PersonKey::PersonKey(std::string name, int age, std::string dob)
    : name_(std::move(name)), folded_(toLower(name_)), age_(age), dob_(std::move(dob)) {}

bool PersonKey::operator<(const PersonKey& other) const {
    return std::tie(folded_, age_, dob_) < std::tie(other.folded_, other.age_, other.dob_);
}

bool PersonKey::operator==(const PersonKey& other) const {
    return folded_ == other.folded_ && age_ == other.age_ && dob_ == other.dob_;
}

std::string PersonKey::toString() const {
    return name_ + ", " + std::to_string(age_) + ", " + dob_;
}

std::pair<PersonKey, PersonKey> PersonKey::prefixRange(const std::string& prefix) {
    // '\x7f' sorts after every printable ASCII character, so any name that
    // starts with the prefix falls between these two keys.
    PersonKey lo(prefix, INT_MIN, "");
    PersonKey hi(prefix + "\x7f", INT_MAX, "\x7f");
    return {lo, hi};
}
