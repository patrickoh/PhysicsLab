// You can copy and use unmodified imgui_impl_* files in your project. See main.cpp for an example of using this.
// If you use this binding you'll need to call 4 functions: 
//ImGui_ImplXXXX_Init(), 
//ImGui_ImplXXXX_NewFrame(), 
//ImGui::Render()
//and ImGui_ImplXXXX_Shutdown().

#include "imgui.h"

IMGUI_API bool        ImGui_Init();
IMGUI_API void        ImGui_Shutdown();
IMGUI_API void        ImGui_NewFrame();

// Use if you want to reset your rendering device without losing ImGui state.
IMGUI_API void        ImGui_InvalidateDeviceObjects();
IMGUI_API bool        ImGui_CreateDeviceObjects();