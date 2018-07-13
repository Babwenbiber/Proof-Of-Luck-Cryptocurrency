
#include "console.h"

Console::Console()
{

}

void Console::run()
{
  while (true)
  {
    char key = getchar();
    emit KeyPressed(key);
  }
  return;
}




