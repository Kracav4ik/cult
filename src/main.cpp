#include <iostream>
#include <vector>
#include <unordered_map>

FILE* out;

class UniversalMachine{
private:
    uint32_t registers[8];
    uint32_t free_index;
    uint32_t exec_finger;
    typedef std::vector<uint32_t> ArrType;
    std::unordered_map<uint32_t, ArrType*> arrays;

    uint32_t create_array(uint32_t size){
        uint32_t index = free_index;
        arrays[index] = new ArrType(size);
        free_index++;
        return index;
   }
    void delete_array(uint32_t index){
        delete arrays[index];
        arrays.erase(index);
    }
public:
    UniversalMachine(const char* file) :
            exec_finger(0),
            free_index(0)
    {
        for (int i = 0; i < 8; i++){
            registers[i] = 0;
        }

        std::vector<uint32_t> program;
        FILE* f = fopen(file,"rb");
        if (!f){
            printf("Cannot open file \"%s\"\n",file);
            return;
        }
        uint8_t p[4];
        while (fread(&p, sizeof(p), 1, f)){
            uint32_t x = (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3];
            program.push_back(x);
        }
        create_array(program.size());
        *arrays[0] = program;
        fclose(f);
    }

    void run(){
        while (true){
            uint32_t word = (*arrays[0])[exec_finger];
            exec_finger++;
            const int opcode = word>>28;
            const int A = (word >> 6) & 7;
            const int B = (word >> 3) & 7;
            const int C = (word >> 0) & 7;
            switch (opcode) {
                case 0:
                    if(registers[C]){
                        registers[A] = registers[B];
                    }
                    break;
                case 1:
                    registers[A] = (*arrays[registers[B]])[registers[C]];
                    break;
                case 2:
                    (*arrays[registers[A]])[registers[B]] = registers[C];
                    break;
                case 3:
                    registers[A] = registers[B] + registers[C];
                    break;
                case 4:
                    registers[A] = registers[B] * registers[C];
                    break;
                case 5:
                    registers[A] = registers[B] / registers[C];
                    break;
                case 6:
                    registers[A] = ~(registers[B] & registers[C]);
                    break;
                case 7:
                    return; // HALT
                case 8:
                    registers[B] = create_array(registers[C]);
                    break;
                case 9:
                    delete_array(registers[C]);
                    break;
                case 10:
                    fprintf(out, "%c", char(registers[C]));
                    fflush(out);
                    break;
                case 11:
                    registers[C] = (uint32_t) getchar();
                    break;
                case 12:
                    *arrays[0] = *arrays[registers[B]];
                    exec_finger = registers[C];
                    break;
                case 13: {
                    const int D = (word>>25) & 7;
                    const uint32_t value = word & ((1 << 25) - 1);
                    registers[D] = value;
                }
                    break;
                default:
                    printf("INVALID OPCODE %d FROM WORD %x\n",opcode,word);
                    return;
            }
        }
    }
};

int main(int argc, const char* argv[]) {
    const char* file = "sandmark.umz";
//    const char* file = "codex.umz";
//    const char* file = "out.um";
    if (argc > 1) {
        file = argv[1];
    }
    if (argc > 2) {
        out = fopen(argv[2], "wb");
    } else {
        out = stdout;
    }
    UniversalMachine um(file);
    um.run();
    if (argc > 2) {
        fclose(out);
    }
    return 0;
}
