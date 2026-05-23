#include "ErrorReporter.hpp"

void ErrorReporter::addErrors(int line, const std::string& message) {
    errors.push_back("[SEMANTIC ERROR] Baris " + std::to_string(line) + ": " + message);
};

bool ErrorReporter::hasErrors() const {
    return !errors.empty();
};

const std::vector<std::string>& ErrorReporter::getErrors() const {
    return errors;
}

void ErrorReporter::clear() {
    errors.clear();
};

