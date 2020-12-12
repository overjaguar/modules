#include "test.h"

Test::Test()
{
  eEvent = IEE_EmoEngineEventCreate();
  eState = IEE_EmoStateCreate();
}

Test::~Test()
{
  IEE_EngineDisconnect();
  IEE_EmoStateFree(eState);
  IEE_EmoEngineEventFree(eEvent);
}

int Test::run()
{
  try {
    if (IEE_EngineConnect() != EDK_OK)
      throw std::runtime_error("Emotiv Driver start up failed.");
    unsigned int userId = 0;
    unsigned long pHwVersionOut;
    std::cout << "Start receiving EmoState! Press any key to stop logging..." << std::endl;
   std::cout << IEE_HardwareGetVersion(2, &pHwVersionOut) << std::endl;
   std::cout << pHwVersionOut << std::endl;
    return 0;
    while (!_kbhit()) {
      state = IEE_EngineGetNextEvent(eEvent);

      if (state == EDK_OK) {
        IEE_Event_t eventType = IEE_EmoEngineEventGetType(eEvent);
        if (eventType == IEE_EmoStateUpdated) {
          IEE_EmoEngineEventGetEmoState(eEvent, eState);
          const float timestamp = IS_GetTimeFromStart(eState);
          std::cout << timestamp << ": " << "New EmoState from user " << static_cast<int>(IS_GetWirelessSignalStatus(eState)) << std::endl;

          for (int i = 0; i < int(sizeof(channels)/sizeof(channels[0])); i++) {
            switch(IS_GetContactQuality(eState, channels[i])){
                case IEEG_CQ_NO_SIGNAL:
                std::cout << "No Signal: " << channels[i] << std::endl;
                break;
              case IEEG_CQ_VERY_BAD:
                std::cout << "Very Bad Signal: " << channels[i] << std::endl;
                break;
              case IEEG_CQ_POOR:
                std::cout << "Poor Signal: " << channels[i] << std::endl;
                break;
              case IEEG_CQ_FAIR:
                // do something
                break;
              case IEEG_CQ_GOOD:
                // do something
                break;
            }
          }
        }
      } else if (state != EDK_NO_EVENT) {
        std::cout << "Internal error in Emotiv Engine!" << std::endl;
        break;
      }
#ifdef _WIN32
      Sleep(1);
#endif
#if __linux__ || __APPLE__
      usleep(10000);
#endif
    }
  }
  catch (const std::runtime_error& e) {
    std::cerr << e.what() << std::endl;
    std::cout << "Press any key to exit..." << std::endl;
    getchar();
  }
  return 0;
}

#ifdef __linux__
int Test::_kbhit(void)
{
    struct timeval tv;
    fd_set read_fd;

    tv.tv_sec=0;
    tv.tv_usec=0;

    FD_ZERO(&read_fd);
    FD_SET(0,&read_fd);

    if(select(1, &read_fd, NULL, NULL, &tv) == -1)
    return 0;

    if(FD_ISSET(0,&read_fd))
        return 1;

    return 0;
}
#endif
#ifdef __APPLE__
int Test::_kbhit(void)
{
    struct timeval tv;
    fd_set rdfs;

    tv.tv_sec = 0;
    tv.tv_usec = 0;

    FD_ZERO(&rdfs);
    FD_SET (STDIN_FILENO, &rdfs);

    select(STDIN_FILENO+1, &rdfs, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &rdfs);
}
#endif
