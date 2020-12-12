#ifndef TEST_H
#define TEST_H

#include <iostream>

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


class Test
{
  private:
    IEE_InputChannels_t channels[14] = {IEE_CHAN_AF3, IEE_CHAN_F7, IEE_CHAN_F3, IEE_CHAN_FC5,
                                      IEE_CHAN_T7, IEE_CHAN_P7, IEE_CHAN_O1, IEE_CHAN_O2,
                                      IEE_CHAN_P8, IEE_CHAN_T8, IEE_CHAN_FC6, IEE_CHAN_F4, IEE_CHAN_F8, IEE_CHAN_AF4};
    EmoEngineEventHandle eEvent;
    EmoStateHandle eState;
    int state  = 0;

    int _kbhit(void);

  public:
    Test();
    ~Test();
    int run();
};

#endif // TEST_H
