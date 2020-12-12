#ifndef RESEARCH_H
#define RESEARCH_H

#include <fstream>
#include <iostream>
#include <map>
#include <ctime>
#include <memory>
#include <vector>

#include "IEmoStateDLL.h"
#include "Iedk.h"
#include "IedkErrorCode.h"

#ifdef _WIN32
    #include <conio.h>
    #include <windows.h>
#endif
#if __linux__ || __APPLE__
    #include <unistd.h>
#endif



class Research
{
  private:
    EmoEngineEventHandle eEvent;
    EmoStateHandle eState;
    unsigned int userID = 0;
    float secs = 1;
    IEE_Event_t eventType;
    int state  = 0;
    enum flags_enum {TEST, MOTION, EEG, FACIAL};
    bool* flags;
    std::string filenames[4];
    std::ofstream ofs[4];

    std::string filename_time();

    void write_headers();
    void logFacial();
    void logMotion(std::unique_ptr<void, void (*)(DataHandle)>* hMotionData);
    void logEEG();

    int _kbhit(void);

  public:
    Research(bool* flags);
    ~Research();
    int run();
};

#endif // RESEARCH_H
