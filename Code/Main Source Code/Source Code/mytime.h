#ifndef MYTIME_H
#define MYTIME_H

class Interface;

int GetorSetElapsedTime(Interface* itfc ,char args[]);
int Pause(Interface* itfc ,char args[]);
int GetDate(Interface *itfc, char[]);
int GetTimeOfDay(Interface *itfc, char[]);
double GetMsTime(void);

#endif // define MYTIME_H