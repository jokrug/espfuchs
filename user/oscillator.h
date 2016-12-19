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

const char* frequencySelectOptions =
"  <option value=\"1\">3478</option>"
"  <option value=\"2\">3555</option>"
"  <option value=\"3\">3636</option>"
"  <option value=\"4\">3400</option>"
"  <option value=\"5\">3431</option>"
"  <option value=\"6\">3462</option>"
"  <option value=\"7\">3491</option>"
"  <option value=\"8\">3519</option>"
"  <option value=\"9\">3545</option>"
"  <option value=\"10\">3571</option>"
"  <option value=\"11\">3597</option>"
"  <option value=\"12\">3621</option>"
"  <option value=\"13\">3644</option>";

void initI2S(enum Frequencies freq);

int setBitPattern();
void printBitField();
void setBit(int bitNr, bool val);
int  getBit(int bitNr);
void setFrequency(enum Frequencies freq);
void frequencyEnumToString(enum Frequencies freq, char* buf);

#endif
