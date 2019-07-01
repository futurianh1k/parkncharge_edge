#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <iostream>

class BoardController {
    int USB;
public:
    static const unsigned char COMMAND_ELEMENT_START = 0x02;
    static const unsigned char COMMAND_ELEMENT_OPTION = 0x00;
    static const unsigned char COMMAND_ELEMENT_END = 0x03;
    BoardController();
    ~BoardController(); 
    void writeCommand(unsigned char*);
    static unsigned char calculateChecksum(unsigned char*, int, int); 
};


