// =================================================================================
// Filename:    Game/GameFactory.cpp
// Author:      SalixGameStudio
// Description: Implements the factory function for creating game-specific states.
// =================================================================================
#include <Game/GameAPI.h>
#include <Game/states/GameState.h>

Salix::IAppState* create_game_state(Salix::AppStateType state_type) {
    switch (state_type) {
        case Salix::AppStateType::Game:
            // Now the DLL will create the GameState.
            return new Salix::GameState();

        // Other game-specific states, can be added here.
        // case Salix::AppStateType::MyOtherGameState:
        //     return new Salix::MyOtherGameState();

        default:
            // If the engine asks for a state this DLL doesn't know about, return null.
            return nullptr;
    }
}