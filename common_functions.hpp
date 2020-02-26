#include<iostream>
#include<fstream>
#include<string>

int get_buffer_fullness() {
    int fullness = 0;
    std::ifstream fp;
    fp.open("store.txt");
    if (fp) {
        fp >> fullness;
        fp.close();
    }
    return fullness;
}
void set_buffer_fullness(int fullness) {
    std::ofstream fp;
    fp.open("store.txt", std::fstream::trunc);
    if (fp) {
        fp << fullness << std::endl;
        fp.flush();
        fp.close();
    } else {
        std::cerr << "Could not open buffer file in setter\n";
    }
}
enum class Type{PRODUCER, CONSUMER};
void log_to_file(std::string message, Type t, unsigned int id) {
    std::ofstream fp;
    std::string filename = (t == Type::PRODUCER ? "producer" : "consumer") + std::to_string(id) + ".txt";
    fp.open(filename, std::fstream::app);
    if (fp) {
        fp << message << '\n';
        fp.close();
    } else {
        std::cerr << "Could not open log file " << (t == Type::PRODUCER ? "producer" : "consumer") << id << '\n';
    }
}