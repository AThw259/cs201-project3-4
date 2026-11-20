// PersonKey.h
// The key used to store people in the B-Trees. Two people can share a name
// and age, but the assignment guarantees the date of birth is unique for each
// (name, age) pair, so (name, age, date of birth) identifies one person.
#pragma once

#include <string>
#include <utility>

class PersonKey {
public:
    PersonKey() = default;
    PersonKey(std::string name, int age, std::string dob);

    const std::string& name() const { return name_; }
    int age() const { return age_; }
    const std::string& dob() const { return dob_; }   // ISO "YYYY-MM-DD"

    // Ordered by name (case-insensitive), then age, then date of birth.
    // ISO dates sort correctly as plain strings.
    bool operator<(const PersonKey& other) const;
    bool operator==(const PersonKey& other) const;
    bool operator!=(const PersonKey& other) const { return !(*this == other); }

    // "Sam Lee, 34, 1992-02-14" - the same format the lookup box accepts.
    std::string toString() const;

    // Smallest and largest possible keys whose name starts with prefix.
    // Passing them to BTree::rangeSearch finds every person with that prefix.
    static std::pair<PersonKey, PersonKey> prefixRange(const std::string& prefix);

private:
    std::string name_;
    std::string folded_;   // lower-case name, used for comparisons
    int age_ = 0;
    std::string dob_;
};

// Helpers shared by the loader and the lookup box.
std::string toLower(std::string s);
std::string trim(const std::string& s);
