#include "driver.h"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <locale>
#include <spdlog/spdlog.h>
#include <sys/stat.h>
#include <sys/types.h>

namespace {
inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
                return !std::isspace(ch);
            }));
}

inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
                return !std::isspace(ch);
            }).base(),
            s.end());
}

std::string trim(const char *buffer, std::size_t size) {
    auto value = std::string{buffer, buffer + size};
    ltrim(value);
    rtrim(value);
    return value;
}

const char *IN_DIRECTION = "in";
const char *OUT_DIRECTION = "out";
} // namespace

namespace gpio {
::gpio::Direction from_string(const char *buffer, std::size_t size) {
    if (auto value = trim(buffer, size); value == IN_DIRECTION) {
        return ::gpio::Direction::Input;
    } else if (value == OUT_DIRECTION) {
        return ::gpio::Direction::Output;
    }
    throw std::runtime_error("Wrong value!");
}

std::string to_string(::gpio::Direction direction) {
    switch (direction) {
    case ::gpio::Direction::Input:
        return IN_DIRECTION;
    case ::gpio::Direction::Output:
    default:
        break;
    }

    return OUT_DIRECTION;
}

void Pin::set_direction(Direction direction) {
    m_direction = direction;
}

Direction Pin::get_direction() const {
    return m_direction;
}

void Pin::set_value(bool value) {
    if (get_direction() == Direction::Input) {
        throw std::runtime_error("Pin is configured as input");
    }
    m_value = value;
}

bool Pin::get_value() const {
    return m_value;
}

Driver *Driver::s_instance = nullptr;
std::once_flag Driver::s_once;

Driver &Driver::get() {
    std::call_once(s_once, &Driver::create);
    return *s_instance;
}

void Driver::create() {
    s_instance = new Driver{};
}

Driver::Driver() = default;

Driver::~Driver() {
    for (auto [number, pin] : m_pins) {
        unexport_pin(number);
    }
}

void Driver::export_pin(uint32_t pin) {
    m_pins[pin] = Pin{};
    set_direction(pin, Direction::Output);
    set_value(pin, false);
}

void Driver::unexport_pin(uint32_t pin) {
    m_pins.erase(pin);
}

void Driver::set_direction(uint32_t pin, Direction direction) {
    m_pins.at(pin).set_direction(direction);
}

Direction Driver::get_direction(uint32_t pin) const {
    return m_pins.at(pin).get_direction();
}

void Driver::set_value(uint32_t pin, bool value) {
    m_pins.at(pin).set_value(value);
}

bool Driver::get_value(uint32_t pin) const {
    return m_pins.at(pin).get_value();
}

std::vector<uint32_t> Driver::get_pins() const {
    std::vector<uint32_t> ret;
    for (auto [pin, data] : m_pins) {
        ret.push_back(pin);
    }
    return ret;
}
} // namespace gpio
