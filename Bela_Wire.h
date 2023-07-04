// https://github.com/espressif/arduino-esp32/tree/master/libraries/Wire
#ifndef TwoWire_h
#define TwoWire_h

#include <stdint.h>
#include <I2c.h>

#ifndef I2C_BUFFER_LENGTH
    #define I2C_BUFFER_LENGTH 128  // Default size, if none is set using Wire::setBuffersize(size_t)
#endif

class TwoWire {

private:
    uint8_t num;
    I2c i2cBus;

    size_t bufferSize;
    uint8_t *rxBuffer;
    size_t rxIndex;
    size_t rxLength;

    uint8_t *txBuffer;
    size_t txLength;
    uint16_t txAddress;

    bool nonStop;

    bool allocateWireBuffer();
    void freeWireBuffer();

public:
    TwoWire(uint8_t bus_num);
    TwoWire(const TwoWire& other) : num(other.num) {};
    ~TwoWire();

    bool begin();
    bool end();

    void beginTransmission(uint8_t address);
    uint8_t endTransmission(bool sendStop = true);

    uint8_t requestFrom(uint8_t address, uint8_t size, uint8_t stop);

    size_t write(uint8_t data);
    size_t write(const uint8_t * buf, size_t count);
    int read();

    int available();
    int peek();
    void flush();

};

extern TwoWire Wire;

#endif