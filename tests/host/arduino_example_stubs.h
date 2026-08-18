#ifndef ASTRUINO_EXAMPLE_STUBS_H
#define ASTRUINO_EXAMPLE_STUBS_H

#include <stdint.h>
#include <string>

#ifndef F
#define F(value) value
#endif
#ifndef LED_BUILTIN
#define LED_BUILTIN 13
#endif
#ifndef HIGH
#define HIGH 1
#endif
#ifndef LOW
#define LOW 0
#endif
#ifndef OUTPUT
#define OUTPUT 1
#endif

inline uint32_t millis() { static uint32_t value = 0; value += 10; return value; }
inline void yield() { ; }
inline void pinMode(int, int) { ; }
inline void digitalWrite(int, int) { ; }

class ExampleSerial {
public:
    void begin(unsigned long) { ; }
    explicit operator bool() const { return true; }

    void print(const char *) { ; }
    void print(const std::string &) { ; }
    void print(char) { ; }
    void print(int) { ; }
    void print(unsigned int) { ; }
    void print(long) { ; }
    void print(unsigned long) { ; }
    void print(long long) { ; }
    void print(unsigned long long) { ; }
    void print(float) { ; }
    void print(double) { ; }
    void print(float, int) { ; }
    void print(double, int) { ; }

    void println() { ; }
    template<class T> void println(const T &value) { print(value); }
    void println(float value, int digits) { print(value, digits); }
    void println(double value, int digits) { print(value, digits); }
};

static ExampleSerial Serial;

#endif
