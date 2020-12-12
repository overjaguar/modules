#include "arguments.h"
#include "emotiveeg.cpp"

Arguments::Arguments(int argc, char *argv[])
{
  if (argc > 1) {
    for (int count=1; count < argc; count++) {
      if (argv[count][0] != '-')
        print_and_exit("WRONG ARGUMENTS!!!", EXIT_FAILURE);

      std::string arg(argv[count]);
      arg = arg.substr(1, arg.length());

      if (!arg.compare("h") || !arg.compare("help"))
        print_and_exit("\t-h \t\tshow this help message and exit.\n\t-a\t\tstart of testing and save the results into file.\n\t-t\t\tcalibration of device.\n", EXIT_SUCCESS);

      if (!arg.compare("t") || !arg.compare("test")) {
        flags[TEST] = true;
        break;
      }

      if (arg.length() > 1)
        for(unsigned long c = 0; c < arg.size(); c++)
          set_flags(arg[c]);
      else
        set_flags(arg[0]);
    }
  }
  else {
    print_and_exit("WRONG ARGUMENTS!!!\ntry --help param", EXIT_FAILURE);
  }
}

void Arguments::set_flags(char c)
{
  switch(c){
    case 'a':
      flags[MOTION] = flags[EEG] = flags[FACIAL] = true;
      break;
    case 'm':
      flags[MOTION] = true;
      break;
    case 'e':
      flags[EEG] = true;
      break;
    case 'f':
      flags[FACIAL] = true;
      break;
    default:
      print_and_exit("WRONG ARGUMENTS!!!", EXIT_FAILURE);
  }
}

void Arguments::print_flags()
{
  std::cout << "Motion: " << flags[MOTION] << std::endl;
  std::cout << "Waves: " << flags[EEG] << std::endl;
  std::cout << "Expression: " << flags[FACIAL] << std::endl;
  std::cout << "Test: " << flags[TEST] << std::endl;
}

