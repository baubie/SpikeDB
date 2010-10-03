
#include <iostream>
#include "spikedata.h"
#include "gui.h"

int main(int argc, char** argv)
{
    SpikeData sd;
    sd.parse("/home/brandon/MU DATA/Riziq/MU10/MU10.1.002");
    sd.printfile();
    return 0;
}
