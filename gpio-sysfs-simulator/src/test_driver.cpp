#include "driver.h"
#include <gtest/gtest.h>
#include <algorithm>

using namespace gpio;

TEST(PinTest, DirectionSetGet) {
    Pin pin;
    pin.set_direction(Direction::Input);
    EXPECT_EQ(pin.get_direction(), Direction::Input);
    pin.set_direction(Direction::Output);
    EXPECT_EQ(pin.get_direction(), Direction::Output);
}

TEST(PinTest, ValueSetGet) {
    Pin pin;
    pin.set_direction(Direction::Output);
    pin.set_value(true);
    EXPECT_TRUE(pin.get_value());
    pin.set_value(false);
    EXPECT_FALSE(pin.get_value());
}

TEST(PinTest, ValueSetThrowsIfInput) {
    Pin pin;
    pin.set_direction(Direction::Input);
    EXPECT_THROW(pin.set_value(true), std::runtime_error);
}

TEST(DriverTest, ExportUnexportPin) {
    Driver &drv = Driver::get();
    drv.export_pin(5);
    auto pins = drv.get_pins();
    EXPECT_NE(std::find(pins.begin(), pins.end(), static_cast<uint32_t>(5)), pins.end());
    drv.unexport_pin(5);
    pins = drv.get_pins();
    EXPECT_EQ(std::find(pins.begin(), pins.end(), static_cast<uint32_t>(5)), pins.end());
}

TEST(DriverTest, SetGetDirectionValue) {
    Driver &drv = Driver::get();
    drv.export_pin(7);
    drv.set_direction(7, Direction::Input);
    EXPECT_EQ(drv.get_direction(7), Direction::Input);
    drv.set_direction(7, Direction::Output);
    EXPECT_EQ(drv.get_direction(7), Direction::Output);
    drv.set_value(7, true);
    EXPECT_TRUE(drv.get_value(7));
    drv.set_value(7, false);
    EXPECT_FALSE(drv.get_value(7));
    drv.unexport_pin(7);
}

TEST(DriverTest, SetValueThrowsIfInput) {
    Driver &drv = Driver::get();
    drv.export_pin(8);
    drv.set_direction(8, Direction::Input);
    EXPECT_THROW(drv.set_value(8, true), std::runtime_error);
    drv.unexport_pin(8);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
