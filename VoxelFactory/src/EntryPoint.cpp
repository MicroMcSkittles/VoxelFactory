#include "Game/Game.h"

int main(int argc, char** argv) {

	Unique<Game> game = CreateUnique<Game>();
	game->Run();

	return 0;
}