#ifndef OUTPUTPROFILE_H
#define OUTPUTPROFILE_H

// Sound per output (an R1 addition): wired headphones and Bluetooth keep
// their own EQ, PEQ, MSEB and crossfeed. See outputprofile.c.
void outputprofile_init(void);
// What is playing out of now: "Wired" or "Bluetooth".
const char *outputprofile_name(void);
// For the simulator's probe: 0 wired, 1 Bluetooth, -1 back to detection.
void outputprofile_force(int output);

#endif
