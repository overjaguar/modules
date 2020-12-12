#include <arguments.h>
#include <test.h>
#include <research.h>
#include <iostream>

int main(int argc, char *argv[])
{
  Arguments args(argc, argv);
//  args.print_flags();
  bool* flags = args.get_flags();
  if(flags[Arguments::TEST]) {
    Test test;
    test.run();
    return 0;
  } else {
    Research research(flags);
    research.run();
  }
  return 0;
}
