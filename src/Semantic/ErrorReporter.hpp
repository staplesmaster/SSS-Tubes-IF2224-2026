#pragma once

#include <string>
#include <vector>

class ErrorReporter {
    private:
        std::vector<std::string> errors;

    public:
        ErrorReporter() = default;

        void addErrors(int line, const std::string& message);

        bool hasErrors() const;

        const std::vector<std::string>& getErrors() const;

        void clear();
};