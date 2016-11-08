#ifndef _OSCILLATOR_H
#define _OSCILLATOR_H

enum Frequencies {
    khz3478 = 1, //direct generated frequencies, no mix products
    khz3555,
    khz3636,
    khz3400,
    khz3431,
    khz3462,
    khz3491,
    khz3519,
    khz3545,
    khz3571,
    khz3597,
    khz3621,
    khz3644
};

void initI2S(enum Frequencies freq);

int setBitPattern();
void printBitField();
void setBit(int bitNr, bool val);
int  getBit(int bitNr);
void setFrequency(enum Frequencies freq);

#endif
