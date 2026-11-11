/*
 * LLD Design Patterns
 * Oct 29, 2026 · @Akanksha Gupta
 *
 * 3. Singleton Pattern
 *
 * Definition
 * Singleton ensures a class has exactly one instance and gives everyone a
 * single global access point to it.
 *
 * How this example enforces that:
 * 1. Private constructor: nobody outside the class can create a Logger.
 * 2. Copy constructor and copy assignment are deleted: nobody can duplicate it.
 * 3. getInstance() is the only way to get the object. The function-local
 *    static is created once, the first time getInstance() runs, and every
 *    later call returns that same object.
 */

#include <iostream>
#include <string>
using namespace std;

class Logger {
private:
    Logger() {}                                  // 1. private constructor: nobody can create one

public:
    Logger(const Logger&) = delete;              // 2. no copying
    Logger& operator=(const Logger&) = delete;

    static Logger& getInstance() {               // 3. the only way to get the object
        static Logger instance;                  // created once, the first time this runs
        return instance;
    }

    void log(const string& msg) {
        cout << "[LOG] " << msg << "\n";
    }
};

int main() {
    Logger::getInstance().log("App started");
    Logger::getInstance().log("Slot 5 allocated");

    Logger& a = Logger::getInstance();
    Logger& b = Logger::getInstance();
    cout << (&a == &b) << "\n";                  // 1: same object
}
