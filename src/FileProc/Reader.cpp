#include "Reader.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

string readFile(const string& filepath) {
    ifstream inputFile(filepath);
    if (!inputFile.is_open()) {
        throw runtime_error("Gagal membuka file input: " + filepath);
    }

    stringstream buffer;
    buffer << inputFile.rdbuf();
    inputFile.close();
    
    return buffer.str();
}