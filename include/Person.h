// Person.h
// Shared base for patients and doctors: name, age and date of birth, which
// together form the PersonKey used by the B-Trees.
#pragma once

#include <string>

#include "PersonKey.h"

class Person {
public:
    Person(std::string name, int age, std::string dob) : key_(std::move(name), age, std::move(dob)) {}
    virtual ~Person() = default;

    const PersonKey& key() const { return key_; }
    const std::string& name() const { return key_.name(); }
    int age() const { return key_.age(); }
    const std::string& dob() const { return key_.dob(); }

private:
    PersonKey key_;
};
