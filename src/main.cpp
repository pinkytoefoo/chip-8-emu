#include <iostream>
#include <format>
#include <cstdint>
#include <span>
#include <array>
#include <utility>
#include <new>
#include <memory>
#include <string>
#include <fstream>
#include <vector>

using byte = std::uint8_t;
using word = std::uint16_t;

constexpr uint32_t START_ADDRESS = 0x200;
constexpr size_t MEMORY_SIZE = 4096;
constexpr byte BIGGEST_BYTE_VALUE = std::numeric_limits<byte>::max();

class CPU {
public:
    CPU()
        : pc_{START_ADDRESS}
    {
    }

    void pushInstructions(std::span<const word> instructions) {
        for(size_t i{}; i < instructions.size(); ++i) {
            memory_[START_ADDRESS + (i*2)] = (instructions[i] & 0xFF00) >> 8;
            memory_[START_ADDRESS + (i*2) + 1] = (instructions[i] & 0x00FF);
        }

        program_size_ = instructions.size() * 2;
    }

    bool loadInstructionsFromDisk(std::string_view path) {
        std::ifstream file(path.data(), std::ios::binary | std::ios::ate);

        if(!file.is_open()) {
            return false;
        }
        
        size_t size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<char> buffer(size, 0);
        file.read(buffer.data(), size);
        file.close();

        for(uint64_t i{}; i < size; ++i) {
            memory_[START_ADDRESS + i] = static_cast<byte>(buffer[i]);
        }

        return true;
    }

    void executeInstructions() {
        while(pc_ < START_ADDRESS + program_size_) {
            opcode_ = (static_cast<word>(memory_[pc_]) << 8) | memory_[pc_ + 1];

            pc_ += 2;

            const byte x = (opcode_ & 0x0F00u) >> 8;
            const byte y = (opcode_ & 0x00F0u) >> 4;
            const byte imm = opcode_ & 0x00FFu;
            const byte type = opcode_ & 0x000Fu;
            const byte addr = opcode_ & 0x0FFFu;
            
            switch(opcode_ & 0xF000) {
                case 0x0000: {
                    switch(opcode_ & 0x00FF) {
                        case 0x00E0: display_.fill(0);
                        case 0x00EE: --sp_; pc_ = stack_[sp_]; break;
                    }
                } break;

                case 0x1000: pc_ = addr; break;
                case 0x2000: stack_[sp_++] = pc_; pc_ = addr; break;
                case 0x3000: {
                    if(registers_[x] == imm) {
                        pc_ += 2;
                    }
                } break;
                case 0x6000: registers_[x] = imm; break;
                case 0x7000: registers_[x] += imm; break;

                case 0x8: {
                    switch(type) {
                        case 1: {
                            registers_[x] = registers_[y];
                        } break;

                        case 4: {
                            uint16_t sum = registers_[x] + registers_[y];
                            std::cout << (int)registers_[x] << '\n' << (int)registers_[y] << '\n';
                            if(sum > BIGGEST_BYTE_VALUE) {
                                registers_[0xF] = 1;
                            } else {
                                registers_[0xF] = 0;
                            }
                            registers_[x] = sum & 0x00FFu;
                        } break;
                    }
                    registers_[x] += imm;
                } break;

                default: {
                    std::cout << "Unknown opcode: 0x" << std::hex << opcode_ << '\n';
                } break;
            }
        }
    }

    void printActiveRegisters() {
        for(int i{}; i < 16; ++i) {
            std::cout << "V[" << i << "]: " << static_cast<int>(registers_[i]) << "\n";
        }
    }

private:
    word pc_{};
    word index_{};
    word opcode_{};
    size_t program_size_{};
    std::array<word, 16> registers_{};
    std::array<word, 64 * 32> display_{};

    byte sp_{};
    std::array<byte, 16> stack_{};
    std::array<byte, 4096> memory_{};
};

struct Test
{
    Test() = default;

    Test(const Test&) {
        std::cout << "copy ctor\n";
    }

    Test(Test&& test) {
        std::cout << "move ctor\n";
    }
};

void printa(Test test) {
    std::cout << "tests address is " << &test << '\n';
}

int main() {
    std::array<word, 3> starter_program{
        0x602Au,
        0x6131u,
        0x8014u
    };

    Test a;
    printa(a);

    CPU cpu;
    if(!cpu.loadInstructionsFromDisk("logo.ch8"))
        return 1;
    
    cpu.executeInstructions();
    cpu.printActiveRegisters();
}