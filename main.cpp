#include "MineVoxelGame.h"
#include "Log.h"


int main(int argc, char* argv[]) {
 
  try {
    mv::MineVoxelGame game;
    game.run();
  }
  catch (std::exception& e) {
    ELOG(e.what());
    std::getchar();
    return -1;
  }
  return 0;
}