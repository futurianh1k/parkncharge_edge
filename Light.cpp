#include "Light.h"

using namespace std;

void Light::onDimDown() {
	lightState = LIGHT_ON_DIM_DOWN;
	generateCommand(lightState);
	sendCommand();
}
	
void Light::onMaximum() {
	lightState = LIGHT_ON_MAXIMUM;
	generateCommand(lightState);
	sendCommand();
}
	
void Light::onMinimum() {
	lightState = LIGHT_ON_MINIMUM;
	generateCommand(lightState);
	sendCommand();
}
	
void Light::off() {
	lightState = LIGHT_OFF;
	generateCommand(lightState);
	sendCommand();
}

void Light::setBrightnessMax(int newBrightnessMax) {
	brightnessMax = newBrightnessMax;
	if (lightState == LIGHT_ON_MAXIMUM) onMaximum();
}

void Light::setBrightnessMin(int newBrightnessMin) {
	brightnessMin = newBrightnessMin;
	if (lightState == LIGHT_ON_MINIMUM) onMinimum();
}

void Light::setDimTime(int newDimTime) {
	dimTime = newDimTime;
}

void Light::generateCommand(unsigned char newLightState) {
	getDefaultCommand();
		
	command[4] = dim;
	command[5] = newLightState;
		
	command[6] = (unsigned char)checkProper8BitValue(brightnessMax);
	command[7] = (unsigned char)checkProper8BitValue(brightnessMin);
		
	int properDimTime = checkProper16BitValue(dimTime);
		
	command[8] = (unsigned char)(properDimTime & 0xFF);
	command[9] = (unsigned char)((properDimTime >> 8) & 0xFF);
}
	
void Light::getDefaultCommand() {
	command[0] = BoardController::COMMAND_ELEMENT_START;
	command[1] = BoardController::COMMAND_ELEMENT_OPTION;
	command[2] = DATA_SIZE;
	command[3] = COMMAND_NUMBER;
	command[10] = COMMAND_ELEMENT_RESERVED;
	command[11] = COMMAND_ELEMENT_RESERVED;
	command[12] = COMMAND_ELEMENT_RESERVED;
	command[13] = COMMAND_ELEMENT_RESERVED;
	command[14] = COMMAND_ELEMENT_RESERVED;
	command[15] = COMMAND_ELEMENT_RESERVED;
	command[17] = BoardController::COMMAND_ELEMENT_END;
}
	
void Light::sendCommand() {
	    command[16] = BoardController::calculateChecksum(command, DATA_START_POSITION, DATA_END_POSITION);
	    boardcontroller.writeCommand(command);	
}

int Light::checkProper8BitValue(int value) {
	if (value > 0xFF) {
	    return 0xFF;
	}
	else {
	    return value;
	}
}
	
int Light::checkProper16BitValue(int value) {
	if (value > 0xFFFF) {
	    return 0xFFFF;
	}
	else {
	    return value;
	}
}


