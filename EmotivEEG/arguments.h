#ifndef ARGUMENTS_H
#define ARGUMENTS_H

#include <array>

class Arguments
{
  private:
    bool flags[4] = {false, false, false, false};
  public:
    Arguments(int argc, char *argv[]);
    enum flags_enum {TEST, MOTION, EEG, FACIAL};

    void set_flags(char c);
    void print_flags();

    bool get_motion() { return flags[MOTION]; }
    bool get_eeg() { return flags[EEG]; }
    bool get_facial() { return flags[FACIAL]; }
    bool get_test() { return flags[TEST]; }
    bool* get_flags() { return flags; }
};

#endif // ARGUMENTS_H
