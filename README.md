# espfuchs
Steuerung von Fuchsjagdsendern für das Amateurfunkpeilen (ARDF) auf Basis von ESP8266-Modulen

-- Funktionen --

    - WLAN
    - Webinterface zur Konfiguration über das WLAN
    - Übernahme der Einstellungen von allen Fuechsen
    - Aktuelle Uhrzeit
    - Einstellbare Startzeit und Endezeit
    - Modus fuer Fuchsjagd, Foxoring, Sprint
    - Einstellbare Anzahl der Füchse, Sendezeit und Pausezeit
    - Frequenz direkt über GPIO erzeugen: Fuchsfrequenz 3,579MHz, 3,550MHz, 3,600MHz, 3,650MHz

## Generating frequencies with the GPIO
Inspired by the https://github.com/cnlohr/channel3 project, which uses the I2S bus to directly 
generate a video signal, I thought, it should be possible to directly generate the transmitter 
frequencies for the fox hunting transmitter.

It works as follows:
* At start-up, a DMA buffer is filled with a bit pattern, that generates the wanted frequency.
* The I2S port is used to shift out the bit pattern at 80MHz or a fraciton of that frequency.

Easy to generate are frequencies, that are fractions of 80MHz:

* 80MHz / 22 = 3.636MHz -> bit pattern: 11 bits high, 11 bits low
* 80MHz / 23 = 3.478MHz

With different pre-divider settings, this frequencies can also be generated:
* 160MHz / 3 /15 = 3.555MHz

As the ESP generates square waves, the spectrum shuld contain the fundamental and every third harmonic,
but the square waves are not perfect and therefore the real spectrum shows more harmonics:


## Mixing frequencies
According to http://www.ardf-r1.org/files/Rules_V2.11B_2013.pdf you need four frequencies between
3510kHz and 3600kHz for e.g. an ARDF sprint event, but only two of the directly generated frequencies
fall in this range.

This software tires to mix two frequencies to get the desired frequency.
The table https://github.com/jokrug/espfuchs/blob/master/FrequenzBerechnung.ods helps to find usefull values.
It lists all values of 80 MHz devided by 2 down to 150. 
I chose 5MHz (80/16) and mixed it with 
* 1.600MHz (80/50) = 5MHz - 1.600 MHz = 3.400Hz
* 1.568MHz (80/51) = 5MHz - 1.568 MHz = 3.431Hz
* 1.538MHz (80/52) = 5MHz - 1.538 MHz = 3.461Hz
and so on.

The mixing of the frequencies is alredy done, when the bit pattern for the I2S bus is calculated at start-up.
In the calculation loop the bit streams for both frequencies are generated and multiplied by a simple logic-AND operation.

This method actually works and generates the desired frequencies, but it also generates a lot of harmonics.
Also the level of the two mixed fundamentals is higher than the level of the desired frequency.


Even if the demands for a low power fox signal with a relatively short range are not so high, I'm not yet sure, 
if I can filter out the harmonics with reasonable efforts.