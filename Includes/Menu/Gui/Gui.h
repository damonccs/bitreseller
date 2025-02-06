#pragma once
#include <map>
#include <unordered_map>
#include <d3d9.h>
#include <Menu/imgui/imgui.h>
#include <Menu/imgui/imgui_internal.h>
#include <Menu/Gui/Icons.h>

using namespace ImGui;

// Forward declarations
void DrawTextCentered(const char* text);
bool inject_btn(const char* label, const ImVec2& size_arg, bool selected);
bool Discord_btn(ImFont* button_icons_font, const char* label, const ImVec2& size_arg, bool selected);
bool Website_btn(ImFont* button_icons_font, const char* label, const ImVec2& size_arg, bool selected);
bool Youtube_btn(ImFont* button_icons_font, const char* label, const ImVec2& size_arg, bool selected);
void render_status(const char* label, int status, const ImVec2& size_arg);
void render_game(const char* label, std::string cheat, const ImVec2& size_arg);
void render_time(const char* label, std::string time_left, const ImVec2& size_arg);
void render_status_message(const char* label, std::string status, const ImVec2& size_arg);
void CircleImageWithOutline(ImTextureID user_texture_id, float diameter, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1), const ImVec4& tint_col = ImVec4(1, 1, 1, 1), const ImVec4& outline_col = ImVec4(1, 0, 1, 1), float outline_thickness = 1.0f);
void OpenLinkInWebBrowser(const char* url);

// External variables
extern LPDIRECT3D9 g_pD3D;
extern D3DPRESENT_PARAMETERS g_d3dpp;
extern LPDIRECT3DTEXTURE9 g_pBackBuffer;
extern LPDIRECT3DSURFACE9 g_pBackBufferSurface;
extern LPDIRECT3DSURFACE9 g_pDepthStencilSurface;

struct checbox_anim {
    int active_outline_anim;
    int active_text_anim;
    int active_rect_alpha;
};

// Font and texture declarations
extern ImFont* FIRSTFONT;
extern ImFont* SECONDFONT;
extern ImFont* BUTTONFONT;
extern ImFont* GUIFONT;

extern IDirect3DTexture9* logotype;
extern IDirect3DTexture9* logomain;


extern std::map <ImGuiID, checbox_anim> anim;
extern std::map<ImGuiID, float> nameanim_animation;
