#include "research.h"

Research::Research(bool* flags)
{
  eEvent = IEE_EmoEngineEventCreate();
  eState = IEE_EmoStateCreate();
  std::cout << "INIT RESEARCH" << std::endl;
  this->flags = flags;
  std::string time(filename_time());

  if (flags[MOTION]) {
    filenames[MOTION] = time+".EmotivEEG-motion.csv";
    ofs[MOTION] = std::ofstream(filenames[MOTION].c_str(), std::ios::trunc);
  }

  if (flags[EEG]) {
    filenames[EEG] = time+".EmotivEEG-eeg.csv";
    ofs[EEG] = std::ofstream(filenames[EEG].c_str(), std::ios::trunc);
  }

  if (flags[FACIAL]){
    filenames[FACIAL] = time+".EmotivEEG-facial.csv";
    ofs[FACIAL] = std::ofstream(filenames[FACIAL].c_str(), std::ios::trunc);
  }

  write_headers();

  std::cout << "Motion: " << this->flags[MOTION] << std::endl;
  std::cout << "Waves: " << this->flags[EEG] << std::endl;
  std::cout << "Expression: " << this->flags[FACIAL] << std::endl;
  std::cout << "Test: " << this->flags[TEST] << std::endl;
}

Research::~Research()
{
  IEE_EngineDisconnect();
  IEE_EmoStateFree(eState);
  IEE_EmoEngineEventFree(eEvent);

  if (flags[MOTION]) ofs[MOTION].close();
  if (flags[EEG]) ofs[EEG].close();
  if (flags[FACIAL]) ofs[FACIAL].close();
}

int Research::run()
{
  try {
    if (IEE_EngineConnect() != EDK_OK)
      throw std::runtime_error("Emotiv Driver start up failed.");

    std::cout << "Start receiving EmoState! Press any key to stop logging..." << std::endl;

    std::unique_ptr<void, void (*)(DataHandle)> hMotionData(IEE_MotionDataCreate(), &IEE_MotionDataFree);
    IEE_MotionDataSetBufferSizeInSec(secs);

    while (!_kbhit()) {
      state = IEE_EngineGetNextEvent(eEvent);

      if (state == EDK_OK) {
        eventType = IEE_EmoEngineEventGetType(eEvent);
        std::cout << IEE_EmoEngineEventGetUserId(eEvent, &userID) << std::endl;

        logEEG();
        logFacial();
        logMotion(&hMotionData);
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

std::string Research::filename_time() {
  std::time_t t = std::time(nullptr);
  char mbstr[100];
  if (std::strftime(mbstr, sizeof(mbstr), "%d_%m_%Y_%H_%M_%S", std::localtime(&t))) {
     std::cout << mbstr << std::endl;
  }
  return std::string(mbstr);
}

void Research::write_headers() {
  if(flags[FACIAL]) {
    ofs[FACIAL] << "Time,";
    ofs[FACIAL] << "Blink,";
    ofs[FACIAL] << "Wink Left,";
    ofs[FACIAL] << "Wink Right,";
    ofs[FACIAL] << "Surprise,";
    ofs[FACIAL] << "Frown,";
    ofs[FACIAL] << "Smile,";
    ofs[FACIAL] << "Clench";
    ofs[FACIAL] << std::endl;
  }
  if(flags[MOTION]) {
    const char header[] = "Time, COUNTER, GYROX, GYROY, GYROZ, ACCX, ACCY, ACCZ, MAGX, "
        "MAGY, MAGZ, TIMESTAMP";
    ofs[MOTION] << header << std::endl;
  }
  if(flags[EEG]) {
    const char header[] = "Time, Theta, Alpha, Low_beta, High_beta, Gamma";
    ofs[EEG] << header << std::endl;
  }
}

void Research::logFacial() {
  if(!flags[FACIAL]) return;
  if (eventType != IEE_EmoStateUpdated) return;

  IEE_EmoEngineEventGetEmoState(eEvent, eState);
  const float timestamp = IS_GetTimeFromStart(eState);
  std::cout << timestamp << ": " << "New EmoState from user " << static_cast<int>(IS_GetWirelessSignalStatus(eState)) << std::endl;

  ofs[FACIAL] << time(NULL) << ",";

  // FacialExpression Suite results
  ofs[FACIAL] << IS_FacialExpressionIsBlink(eState) << ",";
  ofs[FACIAL] << IS_FacialExpressionIsLeftWink(eState) << ",";
  ofs[FACIAL] << IS_FacialExpressionIsRightWink(eState) << ",";

  std::map<IEE_FacialExpressionAlgo_t, float> expressivStates;

  IEE_FacialExpressionAlgo_t upperFaceAction = IS_FacialExpressionGetUpperFaceAction(eState);
  float upperFacePower = IS_FacialExpressionGetUpperFaceActionPower(eState);

  IEE_FacialExpressionAlgo_t lowerFaceAction = IS_FacialExpressionGetLowerFaceAction(eState);
  float lowerFacePower = IS_FacialExpressionGetLowerFaceActionPower(eState);

  expressivStates[ upperFaceAction ] = upperFacePower;
  expressivStates[ lowerFaceAction ] = lowerFacePower;

  ofs[FACIAL] << expressivStates[ FE_SURPRISE] << ","; // eyebrow
  ofs[FACIAL] << expressivStates[ FE_FROWN   ] << ","; // furrow
  ofs[FACIAL] << expressivStates[ FE_SMILE   ] << ","; // smile
  ofs[FACIAL] << expressivStates[ FE_CLENCH  ] << ","; // clench
//  FE_NEUTRAL    = 0x0001,
//  FE_BLINK      = 0x0002,
//  FE_WINK_LEFT  = 0x0004,
//  FE_WINK_RIGHT = 0x0008,
//  FE_HORIEYE    = 0x0010,
//  FE_SURPRISE   = 0x0020,
//  FE_FROWN      = 0x0040,
//  FE_SMILE      = 0x0080,
//  FE_CLENCH     = 0x0100,

//FE_LAUGH      = 0x0200,
//FE_SMIRK_LEFT = 0x0400,
//FE_SMIRK_RIGHT= 0x0800

  ofs[FACIAL] << std::endl;
  ofs[FACIAL].flush();
}

void Research::logMotion(std::unique_ptr<void, void (*)(DataHandle)>* hMotionData) {
  if(!flags[MOTION]) return;
  if (eventType != IEE_UserAdded) return;
  std::cout << "Get motion data" << std::endl;
  const IEE_MotionDataChannel_t targetChannelList[] = {
      IMD_COUNTER,
      IMD_GYROX,
      IMD_GYROY,
      IMD_GYROZ,
      IMD_ACCX,
      IMD_ACCY,
      IMD_ACCZ,
      IMD_MAGX,
      IMD_MAGY,
      IMD_MAGZ,
      IMD_TIMESTAMP
  };

  IEE_MotionDataUpdateHandle(userID, hMotionData->get());
  unsigned int nSamplesTaken=0;
  IEE_MotionDataGetNumberOfSample(hMotionData->get(), &nSamplesTaken);

  if (nSamplesTaken != 0) {
    std::cout << "Updated " << nSamplesTaken << std::endl;

    std::vector<double> data(nSamplesTaken);
    ofs[MOTION] << time(NULL) << ",";
    for(int sampleIdx=0 ; sampleIdx<(int)nSamplesTaken ; ++sampleIdx) {
      for(int i = 0; i<sizeof(targetChannelList)/sizeof(IEE_MotionDataChannel_t); i++) {
        IEE_MotionDataGet(hMotionData->get(), targetChannelList[i], data.data(), data.size());
        ofs[MOTION] << data[sampleIdx] << ",";
      }
      ofs[MOTION] << std::endl;
    }
  }
}

void Research::logEEG() {
  if(!flags[EEG]) return;
//  if (eventType != IEE_UserAdded) return;

  std::cout << "Get the average band power" << std::endl;
  IEE_FFTSetWindowingType(userID, IEE_HAMMING);

  IEE_DataChannel_t channelList[] = { IED_AF3, IED_AF4, IED_T7, IED_T8, IED_Pz };
  double alpha, low_beta, high_beta, gamma, theta;
  alpha = low_beta = high_beta = gamma = theta = 0;

  for(int i=0 ; i< sizeof(channelList)/sizeof(channelList[0]) ; ++i)
  {
    int result = IEE_GetAverageBandPowers(userID, channelList[i], &theta, &alpha,
                                     &low_beta, &high_beta, &gamma);
    if(result != EDK_OK) continue;
    ofs[EEG] << time(NULL) << ",";
    ofs[EEG] << theta << ",";
    ofs[EEG] << alpha << ",";
    ofs[EEG] << low_beta << ",";
    ofs[EEG] << high_beta << ",";
    ofs[EEG] << gamma << ",";
    ofs[EEG] << std::endl;
  }
}

#ifdef __linux__
int Research::_kbhit(void)
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
int Research::_kbhit(void)
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
