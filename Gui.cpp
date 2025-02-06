#include "../Includes/Menu/Gui/Gui.h"
#include <Security/xor.h>
#include <main.h>

// Direct3D global variables
LPDIRECT3D9 g_pD3D = NULL;
LPDIRECT3DDEVICE9 g_pd3dDevice = NULL;
D3DPRESENT_PARAMETERS g_d3dpp;
LPDIRECT3DSWAPCHAIN9 g_pSwapChain = NULL;
LPDIRECT3DTEXTURE9 g_pBackBuffer = NULL;
LPDIRECT3DSURFACE9 g_pBackBufferSurface = NULL;
LPDIRECT3DSURFACE9 g_pDepthStencilSurface = NULL;

// Font and texture variables
ImFont* FIRSTFONT;
ImFont* SECONDFONT;
ImFont* BUTTONFONT;
ImFont* GUIFONT;

IDirect3DTexture9* logotype = nullptr;
IDirect3DTexture9* logomain = nullptr;


std::map<ImGuiID, checbox_anim> anim;
std::map<ImGuiID, float> nameanim_animation;

int youtube_color[4] = { 255, 0, 0, 255 };
int website_color[4] = { 0, 145, 255, 255 };
int discord_color[4] = { 88, 101, 242, 255 };

void OpenLinkInWebBrowser(const char* url) {
    ShellExecuteA(NULL, _xor_("open").c_str(), url, NULL, NULL, SW_SHOWNORMAL);
}

ImColor get_accent_color(float a = 1.f) {
    return ImColor(accent_color[0], accent_color[1], accent_color[2], a);
}

ImColor get_youtube_color(float a = 1.f) {

    return to_vec4(youtube_color[0], youtube_color[1], youtube_color[2], a);
}
ImColor get_discord_color(float a = 1.f) {

    return to_vec4(discord_color[0], discord_color[1], discord_color[2], a);
}
ImColor get_website_color(float a = 1.f) {

    return to_vec4(website_color[0], website_color[1], website_color[2], a);
}


void DrawTextCentered(const char* text) {
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize(text).x) / 2.f);
    ImGui::Text(text);
}

bool inject_btn(const char* label, const ImVec2& size_arg, bool selected) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
    auto draw = window->DrawList;

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);
    const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
    ImGui::ItemSize(size, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id))
        return false;

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, ImGuiButtonFlags_MouseButtonLeft);

    static std::unordered_map<ImGuiID, bool> buttonClicked;
    if (buttonClicked.find(id) == buttonClicked.end()) {
        buttonClicked[id] = false; // Initialize as not clicked
    }

    if (buttonClicked[id]) {
        pressed = false; // Prevent further clicks
    }
    else if (pressed) {
        buttonClicked[id] = true; // Mark as clicked
    }

    static std::unordered_map<ImGuiID, float> values;
    auto value = values.find(id);
    if (value == values.end()) {
        values.insert({ id, { 0.f } });
        value = values.find(id);
    }

    value->second = ImLerp(value->second, (selected ? 1.f : hovered ? 0.5f : 0.f), 0.05f);
    draw->AddRectFilled(bb.Min, bb.Max, ImColor(17, 17, 17), 5);
    draw->AddRect(bb.Min, bb.Max, get_accent_color(GetStyle().Alpha * value->second), 5);

    ImGui::PushFont(FIRSTFONT);
    window->DrawList->AddText(ImVec2(bb.Min.x + size_arg.x / 2 - ImGui::CalcTextSize(label).x / 2, bb.Min.y + size_arg.y / 2 + 1 - ImGui::CalcTextSize(label).y / 1.6 + 2), ImColor(238, 230, 236), label);
    ImGui::PopFont();

    return pressed;
}
bool Discord_btn(ImFont* button_icons_font, const char* label, const ImVec2& size_arg, bool selected) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = button_icons_font->CalcTextSizeA(25.0f, FLT_MAX, 0.0f, label);
    auto draw = window->DrawList;

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);
    const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
    ImGui::ItemSize(size, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id))
        return false;

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, ImGuiButtonFlags_MouseButtonLeft);

    static std::unordered_map<ImGuiID, float> values;
    auto value = values.find(id);
    if (value == values.end()) {
        values.insert({ id, { 0.f } });
        value = values.find(id);
    }

    value->second = ImLerp(value->second, (selected ? 1.f : hovered ? 0.5f : 0.f), 0.05f);
    draw->AddRectFilled(bb.Min, bb.Max, ImColor(17, 17, 17), 5);
    draw->AddRect(bb.Min, bb.Max, get_discord_color(GetStyle().Alpha * value->second), 5);

    ImGui::PushFont(button_icons_font);

    // Calculate the position for centered text
    ImVec2 text_pos = ImVec2((bb.Min.x + bb.Max.x - label_size.x) / 2.0f, (bb.Min.y + bb.Max.y - label_size.y) / 2.0f);

    // Draw the text
    window->DrawList->AddText(text_pos, ImColor(88, 101, 242), label);

    ImGui::PopFont();

    return pressed;
}
bool Website_btn(ImFont* button_icons_font, const char* label, const ImVec2& size_arg, bool selected) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = button_icons_font->CalcTextSizeA(25.0f, FLT_MAX, 0.0f, label);
    auto draw = window->DrawList;

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);
    const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
    ImGui::ItemSize(size, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id))
        return false;

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, ImGuiButtonFlags_MouseButtonLeft);

    static std::unordered_map<ImGuiID, float> values;
    auto value = values.find(id);
    if (value == values.end()) {
        values.insert({ id, { 0.f } });
        value = values.find(id);
    }

    value->second = ImLerp(value->second, (selected ? 1.f : hovered ? 0.5f : 0.f), 0.05f);
    draw->AddRectFilled(bb.Min, bb.Max, ImColor(17, 17, 17), 5);
    draw->AddRect(bb.Min, bb.Max, get_website_color(GetStyle().Alpha * value->second), 5);

    ImGui::PushFont(button_icons_font);

    // Calculate the position for centered text
    ImVec2 text_pos = ImVec2((bb.Min.x + bb.Max.x - label_size.x) / 2.0f, (bb.Min.y + bb.Max.y - label_size.y) / 2.0f);

    // Draw the text
    window->DrawList->AddText(text_pos, ImColor(0, 145, 255), label);

    ImGui::PopFont();

    return pressed;
}


bool Youtube_btn(ImFont* button_icons_font, const char* label, const ImVec2& size_arg, bool selected)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    //const ImVec2 label_size = ImGui::CalcTextSize(label);  // Calculate the size of the label
    const ImVec2 label_size = button_icons_font->CalcTextSizeA(25.0f, FLT_MAX, 0.0f, label);
    auto draw = window->DrawList;

    ImVec2 pos = window->DC.CursorPos;

    ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);
    const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
    ImGui::ItemSize(size, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id))
        return false;

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, ImGuiButtonFlags_MouseButtonLeft);

    static std::unordered_map<ImGuiID, float> values;
    auto value = values.find(id);
    if (value == values.end()) {
        values.insert({ id, { 0.f } });
        value = values.find(id);
    }

    value->second = ImLerp(value->second, (selected ? 1.f : hovered ? 0.5f : 0.f), 0.05f);
    draw->AddRectFilled(bb.Min, bb.Max, ImColor(17, 17, 17), 5);
    draw->AddRect(bb.Min, bb.Max, get_youtube_color(GetStyle().Alpha * value->second), 5);

    ImGui::PushFont(button_icons_font);  // Push the custom font

    // Calculate the position for centered text
    ImVec2 text_pos = ImVec2((bb.Min.x + bb.Max.x - label_size.x) / 2.0f, (bb.Min.y + bb.Max.y - label_size.y) / 2.0f);

    // Draw the text
    window->DrawList->AddText(text_pos, ImColor(255, 0, 0), label);

    ImGui::PopFont();  // Pop the custom font to restore the previous font

    return pressed;
}




void render_status(const char* label, int status, const ImVec2& size_arg) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
    auto draw = window->DrawList;

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);
    const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
    ImGui::ItemSize(size, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id))
        return;

    ImColor c_bg = ImColor(17, 17, 17);
    ImColor c_badge_bg = ImColor(13, 13, 13);
    ImColor c_badge_txt = ImColor(7, 171, 2);

    ImColor c_badge_bg2 = ImColor(13, 13, 13);
    ImColor c_badge_txt2 = ImColor(234, 88, 12);

    window->DrawList->AddRectFilled(ImVec2(bb.Min.x, bb.Min.y), ImVec2(bb.Max.x, bb.Max.y), c_bg, 6, 15);

    int cur_pos = 6;
    auto s_1 = GUIFONT->CalcTextSizeA(20.f, FLT_MAX, 0.f, (char*)ICON_FA_INFO_CIRCLE);
    window->DrawList->AddText(GUIFONT, 20.f, ImVec2(bb.Min.x + cur_pos, bb.Min.y + size_arg.y / 2.5 + 1 - s_1.y / 2 + 2), ImGui::ColorConvertFloat4ToU32(ImColor(180, 180, 180)), (char*)ICON_FA_INFO_CIRCLE);
    cur_pos += s_1.x + 12.f;
    auto s_2 = FIRSTFONT->CalcTextSizeA(20.f, FLT_MAX, 0.f, label);
    window->DrawList->AddText(FIRSTFONT, 20.f, ImVec2(bb.Min.x + cur_pos, bb.Min.y + size_arg.y / 2 - s_1.y / 2 - 1), ImGui::ColorConvertFloat4ToU32(ImColor(180, 180, 180)), label);
    cur_pos += s_2.x + 10.f;

    auto text_to = std::string(_xor_("Under Maintenance"));
    c_bg = c_badge_bg2;
    if (status == 0) {
        text_to = std::string(_xor_("Undetected"));
        c_bg = c_badge_bg;
        c_badge_txt2 = c_badge_txt;
    }

    window->DrawList->AddRectFilled(ImVec2(bb.Min.x + cur_pos, bb.Min.y + 6), ImVec2(bb.Max.x - 6, bb.Max.y - 6), c_bg, 6, 15);
    auto s_3 = FIRSTFONT->CalcTextSizeA(20.f, FLT_MAX, 0.f, text_to.c_str());
    auto center_ = (cur_pos + bb.Max.x - 6 - (bb.Min.x)) / 2.f;
    window->DrawList->AddText(FIRSTFONT, 20.f, ImVec2(bb.Min.x + center_ - s_3.x / 2.f, bb.Min.y + size_arg.y / 1.8 - ImGui::CalcTextSize(text_to.c_str()).y / 2 - 2), c_badge_txt2, text_to.c_str());
}

void render_game(const char* label, std::string cheat, const ImVec2& size_arg) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);
    const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
    ImGui::ItemSize(size, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id))
        return;

    ImColor c_bg = ImColor(17, 17, 17);
    ImColor c_game_bg = ImColor(13, 13, 13);

    ImColor c_fn_txt = ImColor(180, 180, 180);
    ImColor c_valo_txt = ImColor(117, 2, 250);
    ImColor c_apex_txt = ImColor(0, 133, 8);
    ImColor c_war_txt = ImColor(196, 104, 4);
    int cur_pos = 6;

    window->DrawList->AddRectFilled(ImVec2(bb.Min.x, bb.Min.y), ImVec2(bb.Max.x, bb.Max.y), c_bg, 6, 15);

    auto s_1 = GUIFONT->CalcTextSizeA(20.f, FLT_MAX, 0.f, (char*)ICON_FA_GAMEPAD);
    window->DrawList->AddText(GUIFONT, 20.f, ImVec2(bb.Min.x + cur_pos, bb.Min.y + size_arg.y / 2.5 + 1 - s_1.y / 2 + 2), ImGui::ColorConvertFloat4ToU32(ImColor(180, 180, 180)), (char*)ICON_FA_GAMEPAD);
    cur_pos += s_1.x + 10.f;
    window->DrawList->AddText(FIRSTFONT, 20.f, ImVec2(bb.Min.x + cur_pos, bb.Min.y + size_arg.y / 2 - s_1.y / 2 - 1), ImGui::ColorConvertFloat4ToU32(ImColor(180, 180, 180)), label);

    auto s_2 = FIRSTFONT->CalcTextSizeA(20.f, FLT_MAX, 0.f, label);
    cur_pos += s_2.x + 10.f;
    window->DrawList->AddRectFilled(ImVec2(bb.Min.x + cur_pos, bb.Min.y + 6), ImVec2(bb.Max.x - 6, bb.Max.y - 6), c_game_bg, 6, 15);

    auto text_to = cheat.c_str();
    auto s_3 = FIRSTFONT->CalcTextSizeA(20.f, FLT_MAX, 0.f, text_to);
    auto center_ = (cur_pos + bb.Max.x - 6 - (bb.Min.x)) / 2.f;
    window->DrawList->AddText(FIRSTFONT, 20.f, ImVec2(bb.Min.x + center_ - s_3.x / 2.f, bb.Min.y + size_arg.y / 1.8 - ImGui::CalcTextSize(text_to).y / 2 - 2), c_fn_txt, text_to);
}

void render_time(const char* label, std::string time_left, const ImVec2& size_arg) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);
    const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
    ImGui::ItemSize(size, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id))
        return;

    ImColor c_bg = ImColor(17, 17, 17);
    ImColor c_badge_bg = ImColor(13, 13, 13);
    ImColor c_badge_txt = ImColor(150, 150, 150);

    window->DrawList->AddRectFilled(ImVec2(bb.Min.x, bb.Min.y), ImVec2(bb.Max.x, bb.Max.y), c_bg, 6, 15);
    int cur_pos = 6;
    auto s_1 = GUIFONT->CalcTextSizeA(20.f, FLT_MAX, 0.f, (char*)ICON_FA_CLOCK);
    window->DrawList->AddText(GUIFONT, 20.f, ImVec2(bb.Min.x + cur_pos, bb.Min.y + size_arg.y / 2.5 + 1 - s_1.y / 2 + 2), ImGui::ColorConvertFloat4ToU32(ImColor(180, 180, 180)), (char*)ICON_FA_CLOCK);
    cur_pos += s_1.x + 12.f;
    auto s_2 = FIRSTFONT->CalcTextSizeA(20.f, FLT_MAX, 0.f, label);
    window->DrawList->AddText(FIRSTFONT, 20.f, ImVec2(bb.Min.x + cur_pos, bb.Min.y + size_arg.y / 2 - s_1.y / 2 - 1), ImGui::ColorConvertFloat4ToU32(ImColor(180, 180, 180)), label);
    cur_pos += s_2.x + 10.f;

    auto text_to = time_left.c_str();
    window->DrawList->AddRectFilled(ImVec2(bb.Min.x + cur_pos, bb.Min.y + 6), ImVec2(bb.Max.x - 6, bb.Max.y - 6), c_badge_bg, 6, 15);
    auto s_3 = FIRSTFONT->CalcTextSizeA(20.f, FLT_MAX, 0.f, text_to);
    auto center_ = (cur_pos + bb.Max.x - 6 - (bb.Min.x)) / 2.f;
    window->DrawList->AddText(FIRSTFONT, 20.f, ImVec2(bb.Min.x + center_ - s_3.x / 2.f, bb.Min.y + size_arg.y / 1.8 - ImGui::CalcTextSize(text_to).y / 2 - 2), c_badge_txt, text_to);
}

void render_status_message(const char* label, std::string status, const ImVec2& size_arg) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);
    const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
    ImGui::ItemSize(size, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id))
        return;

    ImColor c_bg = ImColor(17, 17, 17);
    ImColor c_badge_bg = ImColor(13, 13, 13);
    ImColor c_badge_txt = ImColor(181, 49, 49);

    window->DrawList->AddRectFilled(ImVec2(bb.Min.x, bb.Min.y), ImVec2(bb.Max.x, bb.Max.y), c_bg, 6, 15);
    int cur_pos = 6;
    auto s_2 = FIRSTFONT->CalcTextSizeA(20.f, FLT_MAX, 0.f, label);
    window->DrawList->AddText(FIRSTFONT, 20.f, ImVec2(bb.Min.x + cur_pos, bb.Min.y + size_arg.y / 2 - 10), ImGui::ColorConvertFloat4ToU32(ImColor(180, 180, 180)), label);
    cur_pos += s_2.x + 10.f;

    auto text_to = status.c_str();
    window->DrawList->AddRectFilled(ImVec2(bb.Min.x + cur_pos, bb.Min.y + 6), ImVec2(bb.Max.x - 6, bb.Max.y - 6), c_badge_bg, 6, 15);
    auto s_3 = FIRSTFONT->CalcTextSizeA(20.f, FLT_MAX, 0.f, text_to);
    auto center_ = (cur_pos + bb.Max.x - 6 - (bb.Min.x)) / 2.f;
    window->DrawList->AddText(FIRSTFONT, 20.f, ImVec2(bb.Min.x + center_ - s_3.x / 2.f, bb.Min.y + size_arg.y / 1.8 - ImGui::CalcTextSize(text_to).y / 2 - 2), c_badge_txt, text_to);
}

void CircleImageWithOutline(ImTextureID user_texture_id, float diameter, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& outline_col, float outline_thickness) {
    ImVec2 p_min = ImGui::GetCursorScreenPos();
    ImVec2 p_max = ImVec2(p_min.x + diameter, p_min.y + diameter);

    float rounding = diameter * 0.5f;
    ImGui::GetWindowDrawList()->AddImageRounded(user_texture_id, p_min, p_max, uv0, uv1, ImGui::GetColorU32(tint_col), rounding);

    float outline_radius = diameter * 0.5f;
    ImVec2 outline_center = ImVec2(p_min.x + diameter * 0.5f, p_min.y + diameter * 0.5f);
    ImGui::GetWindowDrawList()->AddCircle(outline_center, outline_radius, ImGui::GetColorU32(outline_col), 64, outline_thickness);

    ImGui::Dummy(ImVec2(diameter, diameter));
}


