#include "Bela_Wire.h"
#include <Bela.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#define SEND_STOP_SCTRICT

TwoWire::TwoWire(uint8_t bus_num)
    :num(bus_num)
    ,bufferSize(I2C_BUFFER_LENGTH) // default Wire Buffer Size
    ,rxBuffer(NULL)
    ,rxIndex(0)
    ,rxLength(0)
    ,txBuffer(NULL)
    ,txLength(0)
    ,txAddress(0)
    ,nonStop(false)
{}

TwoWire::~TwoWire()
{
    end();
}

bool TwoWire::allocateWireBuffer(void)
{
    // or both buffer can be allocated or none will be
    if (rxBuffer == NULL) {
            rxBuffer = (uint8_t *)malloc(bufferSize);
            if (rxBuffer == NULL) {
                rt_printf("Can't allocate memory for I2C_%d rxBuffer", num);
                return false;
            }
    }
    if (txBuffer == NULL) {
            txBuffer = (uint8_t *)malloc(bufferSize);
            if (txBuffer == NULL) {
                rt_printf("Can't allocate memory for I2C_%d txBuffer", num);
                freeWireBuffer();  // free rxBuffer for safety!
                return false;
            }
    }
    // in case both were allocated before, they must have the same size. All good.
    return true;
}

void TwoWire::freeWireBuffer(void)
{
    if (rxBuffer != NULL) {
        free(rxBuffer);
        rxBuffer = NULL;
    }
    if (txBuffer != NULL) {
        free(txBuffer);
        txBuffer = NULL;
    }
}

bool TwoWire::begin()
{
    rt_printf("TwoWire::begin\n");
    if (!allocateWireBuffer()) {
        freeWireBuffer();
        return false;
    }
    return true;
}
bool TwoWire::end()
{
    rt_printf("TwoWire::end\n");
    freeWireBuffer();
    return true;
}

void TwoWire::beginTransmission(uint8_t address){
    rt_printf("TwoWire::beginTransmission - bus: %d, addr: 0x%X\n", num, address);
    nonStop = false;
    txAddress = address;
    txLength = 0;
    int status = i2cBus.initI2C_RW(num, address, 0);
    if (status != 0){
        rt_printf("TwoWire::beginTransmission - strerror(errno): %s \n", strerror(errno));
    }
}

uint8_t TwoWire::endTransmission(bool sendStop)
{
    rt_printf("TwoWire::endTransmission - sendStop: %d\n", sendStop);
    if (txBuffer == NULL){
        rt_printf("NULL TX buffer pointer");
        return 4;
    }
    if(sendStop){
        rt_printf("TwoWire::endTransmission - i2cBus.writeBytes failed with %s, txLength %d\n", txBuffer, txLength);
        int status = i2cBus.writeBytes(txBuffer, txLength);
        int errsv = errno;
        if (status != 0) {
            rt_printf("TwoWire::endTransmission - i2cBus.writeBytes failed with %d, strerror(status): %s, errno: %d, strerror(errno): %s \n", status, strerror(status), errsv, strerror(errsv));
            return 1; // Return code from https://www.arduino.cc/reference/en/language/functions/communication/wire/endtransmission/
        }
        status = i2cBus.closeI2C();
        if (status != 0) {
            rt_printf("TwoWire::endTransmission - i2cBus.closeI2C failed with status: %d, strerror(errno): %s\n", status, strerror(errno));
            return 4; // Return code from https://www.arduino.cc/reference/en/language/functions/communication/wire/endtransmission/
        }
    } else {
        //mark as non-stop
        nonStop = true;
    }
    return 0;
}

uint8_t TwoWire::requestFrom(uint8_t address, uint8_t size, uint8_t stop)
{
    rt_printf("TwoWire::requestFrom - size: %d\n", size);
    if (rxBuffer == NULL || txBuffer == NULL){
        rt_printf("NULL buffer pointer");
        return 0;
    }
    if(nonStop){
        if(address != txAddress){
            rt_printf("Unfinished Repeated Start transaction! Expected address do not match! %u != %u", address, txAddress);
            return 0;
        }
        nonStop = false;
        rxIndex = 0;
        rxLength = 0;
        int writtenBytes = i2cBus.writeBytes(txBuffer, txLength);
        if (writtenBytes != txLength) {
            rt_printf("TwoWire::endTransmission - i2cBus.writeBytes failed with %d written bytes (expected %d)\n", writtenBytes, txLength);
            return 0;
        }
        int rxLength = i2cBus.readBytes(rxBuffer, size);
        if (size != rxLength) {
            rt_printf("TwoWire::endTransmission - i2cBus.readBytes failed with %d read bytes (expected %d)\n", rxLength, size);
            return 0;
        }
    } else {
        rxIndex = 0;
        rxLength = 0;
        int rxLength = i2cBus.readBytes(rxBuffer, size);
        if (size != rxLength) {
            rt_printf("TwoWire::endTransmission - i2cBus.readBytes failed with %d read bytes (expected %d)\n", rxLength, size);
            return 0;
        }
    }
    // TODO Close? in all cases?
    return rxLength;
}

size_t TwoWire::write(uint8_t data)
{
    rt_printf("TwoWire::write - data: %d\n", data);
    if (txBuffer == NULL){
        rt_printf("NULL TX buffer pointer");
        return 0;
    }
    if(txLength >= bufferSize) {
        return 0;
    }
    txBuffer[txLength++] = data;
    return 1;
}

size_t TwoWire::write(const uint8_t *data, size_t quantity)
{
    rt_printf("TwoWire::write - quantity: %d\n", quantity);
    for(size_t i = 0; i < quantity; ++i) {
        if(!write(data[i])) {
            return i;
        }
    }
    return quantity;
}

int TwoWire::available(void)
{
    rt_printf("TwoWire::available\n");
    int result = rxLength - rxIndex;
    return result;
}

int TwoWire::read(void)
{
    rt_printf("TwoWire::read\n");
    int value = -1;
    if (rxBuffer == NULL){
        rt_printf("NULL RX buffer pointer");
        return value;
    }
    if(rxIndex < rxLength) {
        value = rxBuffer[rxIndex++];
    }
    return value;
}

int TwoWire::peek(void)
{
    rt_printf("TwoWire::peek\n");
    int value = -1;
    if (rxBuffer == NULL){
        rt_printf("NULL RX buffer pointer");
        return value;
    }
    if(rxIndex < rxLength) {
        value = rxBuffer[rxIndex];
    }
    return value;
}

void TwoWire::flush(void)
{
    rt_printf("TwoWire::flush\n");
    rxIndex = 0;
    rxLength = 0;
    txLength = 0;
}

TwoWire Wire = TwoWire(1);
