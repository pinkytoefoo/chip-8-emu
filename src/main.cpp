#include <SDL3/SDL.h>

#include <array>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <limits>
#include <span>
#include <string_view>
#include <vector>

using byte = std::uint8_t;
using word = std::uint16_t;

constexpr word START_ADDRESS = 0x200;
constexpr size_t MEMORY_SIZE = 4096;

constexpr int DISPLAY_WIDTH = 64;
constexpr int DISPLAY_HEIGHT = 32;
constexpr int SCALE = 10;

constexpr byte BIGGEST_BYTE_VALUE = std::numeric_limits<byte>::max();

class CPU {
public:
    CPU()
        : pc_{START_ADDRESS}
    {
    }

    void pushInstructions(std::span<const word> instructions)
    {
        for (size_t i{}; i < instructions.size(); ++i) {
            // split 2 byte rom instruction into addressable memory bytes
            memory_[START_ADDRESS + (i * 2)] = (instructions[i] & 0xFF00) >> 8;
            memory_[START_ADDRESS + (i * 2) + 1] = instructions[i] & 0x00FF;
        }

        program_size_ = instructions.size() * 2;
    }

    bool loadInstructionsFromDisk(std::string_view path)
    {
        std::ifstream file(path.data(), std::ios::binary | std::ios::ate);

        if (!file.is_open()) {
            return false;
        }

        const size_t size = file.tellg();

        // reset file ptr after finding size
        file.seekg(0, std::ios::beg);

        std::vector<char> buffer(size);

        file.read(buffer.data(), size);

        for (size_t i{}; i < size; ++i) {
            memory_[START_ADDRESS + i] =
                static_cast<byte>(buffer[i]);
        }

        program_size_ = size;

        return true;
    }
    
    // chip-8 reference
    // http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
    void cycle()
    {
        // fetch
        opcode_ = (static_cast<word>(memory_[pc_]) << 8) | memory_[pc_ + 1];
        pc_ += 2;

        const byte x =
            (opcode_ & 0x0F00u) >> 8;

        const byte y =
            (opcode_ & 0x00F0u) >> 4;

        const byte imm =
            opcode_ & 0x00FFu;

        const byte type =
            opcode_ & 0x000Fu;

        const word addr =
            opcode_ & 0x0FFFu;

        // execute
        switch (opcode_ & 0xF000u) {

        // 0xxx
        // instruction match
        case 0x0000:
            switch (opcode_ & 0x00FFu) {

            // CLS
            case 0x00E0:
                display_.fill(0);
                break;

            // RET
            case 0x00EE:
                --sp_;
                pc_ = stack_[sp_];
                break;

            default:
                break;
            }
            break;

        // JP addr
        case 0x1000:
            pc_ = addr;
            break;

        // CALL addr
        case 0x2000:
            stack_[sp_++] = pc_;
            pc_ = addr;
            break;

        // SE Vx, byte
        case 0x3000:
            if (registers_[x] == imm) {
                pc_ += 2;
            }
            break;
        
        // SNE Vx, byte
        case 0x4000:
            if(registers_[x] == imm) {
                pc_ += 2;
            }
            break;
        
        // SE Vx, Vy
        case 0x5000:
            if(registers_[x] == registers_[y]) {
                pc_ += 2;
            }
            break;

        // LD Vx, byte
        case 0x6000:
            registers_[x] = imm;
            break;

        // ADD Vx, byte
        case 0x7000:
            registers_[x] += imm;
            break;

        // 8xy0
        case 0x8000:
            switch (type) {
            // LD Vx, Vy
            case 0x1:
                registers_[x] = registers_[y];
                break;

            // ADD Vx, Vy
            case 0x4: {
                const word sum =
                    registers_[x] + registers_[y];

                registers_[0xF] =
                    sum > BIGGEST_BYTE_VALUE ? 1 : 0;

                registers_[x] =
                    sum & 0x00FFu;

                break;
            }

            default:
                break;
            }
            break;
        
        // LD I, addr
        case 0xA000: {
            index_ = addr;
        } break;

        // DxyN - draw (http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#Dxyn)
        case 0xD000: {
            const byte height = opcode_ & 0x000Fu;
            const byte start_x = registers_[x];
            const byte start_y = registers_[y];
            
            registers_[0xF] = 0;

            for (byte row{}; row < height; ++row) {
                const byte sprite = memory_[index_ + row];

                for (byte col{}; col < 8; ++col) {
                    if ((sprite & (0x80u >> col)) == 0) {
                        continue;
                    }

                    const size_t px = (start_x + col) % DISPLAY_WIDTH;

                    const size_t py = (start_y + row) % DISPLAY_HEIGHT;

                    const size_t pixel = py * DISPLAY_WIDTH + px;

                    // collision
                    if (display_[pixel]) {
                        registers_[0xF] = 1;
                    }

                    // XOR the pixel
                    display_[pixel] ^= 1;
                }
            }

            break;
        }

        default:
            std::cout << "Unknown opcode: 0x" << std::hex << opcode_ << '\n';
            break;
        }
    }

    const std::array<byte, DISPLAY_WIDTH * DISPLAY_HEIGHT>& display() const
    {
        return display_;
    }

    void printActiveRegisters() const
    {
        for (int i{}; i < 16; ++i) {
            std::cout << "V[" << i << "]: " << static_cast<int>(registers_[i]) << '\n';
        }
    }

private:
    word pc_{}; // program counter
    word index_{};
    word opcode_{};
    
    // loaded rom size
    size_t program_size_{};

    std::array<byte, 16> registers_{};
    std::array<byte, DISPLAY_WIDTH * DISPLAY_HEIGHT> display_{};

    byte sp_{};

    std::array<word, 16> stack_{};
    std::array<byte, MEMORY_SIZE> memory_{};
};

int main()
{
    CPU cpu;
    Display display;

    if (!cpu.loadInstructionsFromDisk("test_opcode.ch8")) {
        std::cerr << "failed to load ROM.\n";
        return 1;
    }

    if (!display.initialize()) {
        return 1;
    }

    bool running = true;

    while (running) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        cpu.cycle();

        display.render(cpu.display());

        SDL_Delay(2);
    }

    display.shutdown();

    return 0;
}
