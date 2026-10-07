#include "Game.h"

int main() {
    Game game;
    if (!game.init(1200, 600)) {
        return -1;
    }
    game.run();
    return 0;
}
