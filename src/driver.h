#pragma once

#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace gpio {
enum class Direction {
    Input,
    Output
};

::gpio::Direction from_string(const char *buffer, std::size_t size);
std::string to_string(::gpio::Direction direction);

class Pin {
public:
    void set_direction(Direction direction);
    Direction get_direction() const;

    void set_value(bool value);
    bool get_value() const;

private:
    Direction m_direction = Direction::Output;
    bool m_value = false;
};

class Driver {
public:
    static Driver &get();

    ~Driver();

    void export_pin(uint32_t pin);
    void unexport_pin(uint32_t pin);

    void set_direction(uint32_t pin, Direction direction);
    Direction get_direction(uint32_t pin) const;

    void set_value(uint32_t pin, bool value);
    bool get_value(uint32_t pin) const;

    std::vector<uint32_t> get_pins() const;

private:
    Driver();

    Driver(const Driver &) = delete;
    Driver &operator=(const Driver &) = delete;

    static void create();

    static std::once_flag s_once;
    static Driver *s_instance;

    mutable std::unordered_map<uint32_t, Pin> m_pins;
};
} // namespace gpio