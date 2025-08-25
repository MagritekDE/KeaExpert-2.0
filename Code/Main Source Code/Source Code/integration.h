#ifndef INTEGRATION_H
#define INTEGRATION_H

class Interface;

int SimpsonsIntegration(Interface* itfc ,char args[]);
int TrapezoidalIntegration(Interface* itfc ,char args[]);

#endif // define INTEGRATION_H
