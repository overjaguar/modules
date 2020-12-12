#include <iostream>

[[noreturn]] void print_and_exit(std::string msg, int exit_code)
{
  std::cout << msg << std::endl;
  exit(exit_code);
}
