#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

class Interface;

int MemoryStatus(Interface* itfc ,char args[]);
int GetEnvironmentVariable(Interface *itfc, char args[]);

#endif //define ENVIRONMENT_H