#pragma once

#include <cstdint>
#include <limits>

using byte = std::uint8_t;
using word = std::uint16_t;

constexpr word START_ADDRESS = 0x200;
constexpr std::size_t MEMORY_SIZE = 4096;

constexpr int DISPLAY_WIDTH = 64;
constexpr int DISPLAY_HEIGHT = 32;
constexpr int SCALE = 10;

constexpr byte BIGGEST_BYTE_VALUE = std::numeric_limits<byte>::max();
