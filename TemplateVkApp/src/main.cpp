#include "application.h"

//#define  DEBUG_MEM_LEAKS
#ifdef   DEBUG_MEM_LEAKS
# define _CRTDBG_MAP_ALLOC
# include <stdlib.h>
# include <crtdbg.h>
#endif //DEBUG_MEM_LEAKS

Application app;

int main() {
#ifdef DEBUG_MEM_LEAKS
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
  //_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
#endif

  app.init(800, 600);

  app.run();
    
  app.shutdown();

#ifdef DEBUG_MEM_LEAKS
  _CrtDumpMemoryLeaks();
#endif

  return 0;
} 