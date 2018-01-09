#include "BoardController.h"

class Light {
public:
    Light() : lightState(LIGHT_OFF), brightnessMax(0xFF), brightnessMin(30), dimTime(1000), dim(DIM1) {}
    void onDimDown();
    void onMaximum();
    void onMinimum();
    void off();
    void setBrightnessMax(int);
    void setBrightnessMin(int);
    void setDimTime(int);

private:
    int lightState;
    int brightnessMax;
    int brightnessMin;
    int dimTime;
    int dim;
    unsigned char command[18];
    BoardController boardcontroller;

    static const unsigned char DIM1 = 0x00;
    static const unsigned char DIM2 = 0x01;
    static const unsigned char LIGHT_OFF = 0x00;
    static const unsigned char LIGHT_ON_MINIMUM = 0x01;
    static const unsigned char LIGHT_ON_MAXIMUM = 0x02;
    static const unsigned char LIGHT_ON_DIM_DOWN = 0x03;
    static const unsigned char DATA_SIZE = 0x0C;
    static const unsigned char COMMAND_NUMBER = 0x03;
    static const unsigned char COMMAND_ELEMENT_RESERVED = 0x00;
    static const int DATA_START_POSITION = 1;
    static const int DATA_END_POSITION = 15;

    void generateCommand(unsigned char newLightState);
    void getDefaultCommand();
    void sendCommand();
    static int checkProper8BitValue(int value);
    static int checkProper16BitValue(int value);
};


