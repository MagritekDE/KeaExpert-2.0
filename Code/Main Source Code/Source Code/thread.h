#ifndef THREAD_H
#define THREAD_H

#include <vector>

class Interface;

extern DWORD dwTlsIndex; // Thread local storage for current gui window
extern bool threadAbortPresent;
extern std::vector<long> threadAbortIDs;

int CountThreads();
int Thread(Interface* itfc ,char args[]);
int AbortThread(Interface* itfc ,char args[]);
int ThreadStatus(Interface *itfc, char args[]);

#endif // define THREAD_H