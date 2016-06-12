#include "ComFtdiFpc1011.h"

#include <string>
#include <iostream>
#include <fstream>

using namespace std;

int main() {
    unsigned char* img = new unsigned char[152 * 200];


    if( open() != 0 ){
        cout << "Failed to open FTDI device\n" ;
    }
    else{
        cout << "FTDI device successfully opened\n";
    }

    if( resetSensor() != 0 ){
        cout << "Faild to reset the FTDI device\n";
        return -1;
    }
    else{
        cout << "FTDI device successfully reset\n";
    }

    captureImage(2, 127, 0, img);
    
    ofstream out("out.raw");
    out << img;
    out.close();

    return 0;
}
