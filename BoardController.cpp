#include "BoardController.h"

using namespace std;

BoardController::BoardController() {
	cout << endl << "----------<<< BoardController.cpp in  >>>----------" << endl << endl;
	USB = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY);
//	USB = open("/dev/tty0", O_RDWR | O_NOCTTY | O_NDELAY);
	if (USB == -1 ) {
	    perror("open_port: Unable to open /dev/ttyUSB0 – ");
	}
	else {
	    fcntl(USB, F_SETFL, 0);
	    cout << "Port 1 has been sucessfully opened and " << USB <<" is the file description" << endl;
	}
}

BoardController::~BoardController() {
	close(USB);
}

void BoardController::writeCommand(unsigned char *command) {
	write(USB,command,18);	
}

unsigned char BoardController::calculateChecksum(unsigned char* fullCommand, int startPosition, int endPosition) {
	unsigned char checksum = 0x00;
	for (int i = startPosition; i <= endPosition; i++) {
	    checksum ^= fullCommand[i];
	}
	return checksum;
}

