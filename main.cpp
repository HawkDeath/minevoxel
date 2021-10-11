#include "MineVoxelGame.h"
#include "Log.h"
#include <memory>

int main(int argc, char* argv[]) {
 
  try {
    std::unique_ptr<mv::MineVoxelGame> game = std::make_unique<mv::MineVoxelGame>();
    game->run();
  }
  catch (std::exception& e) {
    ELOG(e.what());
    std::getchar();
    return -1;
  }
  return 0;
}