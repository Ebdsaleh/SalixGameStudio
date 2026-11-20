#include <Salix/states/IAppState.h>

extern "C" __declspec(dllexport) Salix::IAppState* create_editor_state(Salix::AppStateType) {
    return nullptr;
}
