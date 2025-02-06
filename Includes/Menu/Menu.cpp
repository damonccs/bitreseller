#include "Menu.h"
#include <d3d11.h>
#include <dwmapi.h>
#include <thread>
#include <iostream>
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
#include "Menu/imgui/imgui_internal.h"
#include "LoaderThreads/LoaderThreads.h"
#include "Server/Server.h"
#include "fileutils/fileutils.h"
#define STB_IMAGE_IMPLEMENTATION
#include "Menu/imgui/stb_image.h"
#include <main.h>
#include <Menu/Gui/Gui.h>
#include <Security/xor.h>
#include <Menu/Gui/bytes.hpp>

ID3D11Device* g_pd3dDevice = NULL;
ID3D11DeviceContext* g_pd3dDeviceContext = NULL;
IDXGISwapChain* g_pSwapChain = NULL;
ID3D11RenderTargetView* g_mainRenderTargetView = NULL;
HWND hwnd = NULL;
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
bool LoadTextureFromMemory(unsigned char* Buffer, size_t Size, ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
ImFont* BIGBIG;
ImFont* MEDIUM;
ID3D11ShaderResourceView* Logo;
int LogoX;
int LogoY;

int my_image_width = 0;
int my_image_height = 0;
ID3D11ShaderResourceView* my_texture = NULL;

bool LoadTextureFromBytes(const unsigned char* image_data, size_t image_size, ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height)
{
    int image_width = 0;
    int image_height = 0;
    int channels = 4; // RGBA
    unsigned char* decoded_image_data = stbi_load_from_memory(image_data, image_size, &image_width, &image_height, &channels, STBI_rgb_alpha);
    if (decoded_image_data == NULL)
        return false;

    // Create texture
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = image_width;
    desc.Height = image_height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;

    ID3D11Texture2D* pTexture = NULL;
    D3D11_SUBRESOURCE_DATA subResource;
    subResource.pSysMem = decoded_image_data;
    subResource.SysMemPitch = desc.Width * 4;
    subResource.SysMemSlicePitch = 0;
    g_pd3dDevice->CreateTexture2D(&desc, &subResource, &pTexture);

    // Create texture view
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = desc.MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;
    g_pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, out_srv);
    pTexture->Release();

    *out_width = image_width;
    *out_height = image_height;
    stbi_image_free(decoded_image_data);

    return true;
}
bool LoadTextureFromFile(const char* file_path, ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height) {
    // Load image from file using stbi
    int image_width = 0, image_height = 0, channels = 4; // RGBA
    unsigned char* image_data = stbi_load(file_path, &image_width, &image_height, &channels, STBI_rgb_alpha);
    if (image_data == NULL) {
        return false;
    }

    // Create texture description
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = image_width;
    desc.Height = image_height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;

    // Create subresource
    D3D11_SUBRESOURCE_DATA subResource;
    subResource.pSysMem = image_data;
    subResource.SysMemPitch = desc.Width * 4;
    subResource.SysMemSlicePitch = 0;

    ID3D11Texture2D* texture = nullptr;
    HRESULT hr = g_pd3dDevice->CreateTexture2D(&desc, &subResource, &texture);
    if (FAILED(hr)) {
        stbi_image_free(image_data);
        return false;
    }

    // Create shader resource view
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = desc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = desc.MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;

    hr = g_pd3dDevice->CreateShaderResourceView(texture, &srvDesc, out_srv);
    texture->Release();

    if (FAILED(hr)) {
        stbi_image_free(image_data);
        return false;
    }

    // Set output parameters
    *out_width = image_width;
    *out_height = image_height;

    // Cleanup
    stbi_image_free(image_data);
    return true;
}

bool tryed = false;

//manualy add your logo
void LoadLogo(std::string url) {
    tryed = true;
    bool ret = LoadTextureFromBytes(image_data.data(), image_data.size(), &my_texture, &my_image_width, &my_image_height);
    if (!ret) {
        my_texture = NULL;
    }
}
void LoadLogoFromServer(std::string url) {
    tryed = true;
    BitAuth::AuthApi Load;
    std::string downloadedFilePath = Load.LoadLogoAuto(url);
    bool ret = LoadTextureFromFile(downloadedFilePath.c_str(), &my_texture, &my_image_width, &my_image_height);
    if (!ret) {
        my_texture = NULL;
    }
}


RECT center_window(HWND parent_window, int width, int height)
{
    RECT rect;
    GetClientRect(parent_window, &rect);
    rect.left = (rect.right / 2) - (width / 2);
    rect.top = (rect.bottom / 2) - (height / 2);
    return rect;
}
WNDCLASSEX wc;


bool  Menu::Start() {
    wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, "LOADER__", NULL };
    wc.hInstance = instance;
    ::RegisterClassEx(&wc);
    int width = 575, height = 500;
    RECT rect;
    GetClientRect(GetDesktopWindow(), &rect);
    auto center = center_window(GetDesktopWindow(), width, height);
    hwnd = CreateWindowExA(0, wc.lpszClassName, fileutils::RandomString(8).c_str(), WS_POPUP | WS_VISIBLE, center.left, center.top, width, height, NULL, NULL, instance, NULL);
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
        return false;
    }
    MARGINS Margin = { -1 };
    DwmExtendFrameIntoClientArea(hwnd, &Margin);
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
    ImFontConfig icons_config; icons_config.MergeMode = true; icons_config.PixelSnapH = true;
    io.IniFilename = nullptr;

    FIRSTFONT = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(Roboto_compressed_data, Roboto_compressed_size, 19.f, nullptr, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
    SECONDFONT = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(Roboto_compressed_data, Roboto_compressed_size, 24.f, nullptr, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
    BUTTONFONT = io.Fonts->AddFontFromMemoryTTF(icons_button, sizeof(icons_button), 25.0f);
    GUIFONT = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(Awesome, Awesome_size, 16.f, &icons_config, icons_ranges);

    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    return true;
}

void LoginKey(std::string key) {
    BitAuth::AuthApi BitAuthApp;
    try {
        BitAuthApp.Authenticate(key);
        if (BitAuthApp.data.login_success) {
            //MessageBoxW(NULL, L"Authentication Successfuly", L"Debug Log", MB_OK);
            Menu::Get().store_name = BitAuthApp.data.store_name;
            Menu::Get().cheat_name = BitAuthApp.data.cheat_name;
            Menu::Get().time_left = BitAuthApp.data.time_left;
            Menu::Get().hasSpoofer = BitAuthApp.HasSpoofer;
            Menu::Get().store_logo = BitAuthApp.data.store_logo;
            Menu::Get().store_discord_link = BitAuthApp.data.store_discord_link;
            Menu::Get().store_youtube_link = BitAuthApp.data.store_youtube_link;
            Menu::Get().store_website_link = BitAuthApp.data.store_website_link;

            /* DisplayResponseMessageBoxss(BitAuthApp.data.store_discord_link);
             DisplayResponseMessageBoxss(Menu::Get().store_youtube_link);*/

            Menu::Get().Loading = false;
            Menu::Get().page = 1;

        }
        else {
            //MessageBoxW(NULL, L"Authentication Failed", L"Debug Log", MB_OK);
            Menu::Get().page = 0;
        }
    }
    catch (const std::exception& e) {
        Menu::Get().message = BitAuthApp.data.error_message;
        Menu::Get().Loading = false;
        Menu::Get().isError = true;
        Sleep(5000); // Wait for 5 seconds before exiting
        fileutils::Terminate();

    }
    catch (...) {
        Menu::Get().message = _xor_("An unknown error occurred during login.").c_str();
        Menu::Get().Loading = false;
        Menu::Get().isError = true;
        Sleep(5000); // Wait for 5 seconds before exiting
        fileutils::Terminate();
    }
}
void Load_Product() {
    Menu& menu = Menu::Get();
    BitAuth::AuthApi BitAuthApp;
    try {
        BitAuthApp.Load_Product();
        if (BitAuthApp.data.product_loaded_success) {
            //example
            //LoaderThreads::SuccessClose("Product Loaded Successfuly");
            //example

            Menu::Get().message = BitAuthApp.data.error_message;
            menu.Loading = false;
        }
        else {
            //example
            //LoaderThreads::ErrorClose("Product Loading Failed");
            //example

            Menu::Get().message = BitAuthApp.data.error_message;
            menu.Loading = false;
        }
    }
    catch (const std::exception& e) {
        Menu::Get().message = BitAuthApp.data.error_message;
        Menu::Get().Loading = false;
        Menu::Get().isError = true;
        Sleep(5000); // Wait for 5 seconds before exiting
        fileutils::Terminate();

    }
    catch (...) {
        Menu::Get().message = _xor_("An unknown error occurred during load cheat.").c_str();
        Menu::Get().Loading = false;
        Menu::Get().isError = true;
        Sleep(5000); // Wait for 5 seconds before exiting
        fileutils::Terminate();
    }
}

void Menu::Loop() {
    bool done = true;
    bool firstTime = true;
    bool configureSpoofer = false;
    int selectedCheat = 0;
    BitAuth::AuthApi BitAuthApp;

    while (done)
    {
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = false;
        }

        if (!done)
            break;

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        if (firstTime) {
            ImGui::SetNextWindowPos({ 0, 0 });
            firstTime = false;
        }

        ImGuiStyle& style = ImGui::GetStyle();
        style.Colors[ImGuiCol_WindowBg] = ImColor(13, 13, 13);
        style.Colors[ImGuiCol_ChildBg] = ImColor(15, 15, 15);
        style.Colors[ImGuiCol_Border] = ImColor(55, 65, 81);
        style.Colors[ImGuiCol_TextSelectedBg] = ImColor(255, 255, 255, 50);
        style.Colors[ImGuiCol_FrameBg] = ImColor(29, 29, 29);
        style.Colors[ImGuiCol_FrameBgActive] = ImColor(29, 29, 29);
        style.Colors[ImGuiCol_FrameBgHovered] = ImColor(29, 29, 29);
        style.Colors[ImGuiCol_Button] = ImColor(29, 29, 29);
        style.Colors[ImGuiCol_ButtonHovered] = ImColor(30, 30, 40);
        style.Colors[ImGuiCol_ButtonActive] = ImColor(30, 30, 40);
        style.Colors[ImGuiCol_Text] = ImColor(255, 255, 255);
        style.WindowRounding = 4;
        style.WindowBorderSize = 1.5;
        style.FrameBorderSize = 1;
        style.ChildBorderSize = 1.5;
        style.ChildRounding = 7;
        style.FrameRounding = 4;
        style.ScrollbarSize = 0;
        style.Colors[ImGuiCol_WindowBg] = ImColor(13, 13, 13, 255);
        ImGui::SetNextWindowSize(ImVec2(575, 500)); // format = width height
        if (ImGui::Begin(_xor_("Loader").c_str(), &done, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar))
        {
            style.Colors[ImGuiCol_Button] = ImColor(13, 13, 13);
            ImGui::SetCursorPos({ 540.f, 5.f });
            if (ImGui::Button(_xor_("X").c_str(), ImVec2(30, 30)))
            {
                done = false;
                ExitProcess(0);

            }
            ImVec2 windowSize = ImGui::GetWindowSize();

            if (page == 0)
            {
                BitAuthApp.LoadSettings();

                ImGui::SetCursorPos({ 240.f, 40.f });
                float circleDiameter = 100.0f;
                ImVec2 uv0(0, 0);
                ImVec2 uv1(1, 1);
                ImVec4 tintCol(1, 1, 1, 1);
                ImVec4 outlineCol(accent_color[0], accent_color[1], accent_color[2], 1.f);
                float outlineThickness = 2.0f;

                if (!Menu::Get().store_logo.empty()) {
                    if (!my_texture && !tryed) {
                        LoadLogoFromServer(Menu::Get().store_logo);
                    }

                    if (my_texture) {
                        CircleImageWithOutline(my_texture, circleDiameter, uv0, uv1, tintCol, outlineCol, outlineThickness);
                    }
                }
                if (!Menu::Get().store_name.empty()) {
                    const char* textToDisplay = Menu::Get().store_name.c_str();

                    ImVec2 textSize = ImGui::CalcTextSize(textToDisplay);


                    float posX = (windowSize.x - textSize.x) / 2.0f;

                    float posY = 160.0f;

                    ImGui::SetCursorPos({ posX, posY });
                    ImGui::PushFont(SECONDFONT);
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(200.0f / 255.0f, 200.0f / 255.0f, 200.0f / 255.0f, 1.0f));
                    ImGui::Text("%s", textToDisplay);
                    ImGui::PopStyleColor();
                    ImGui::PopFont();
                }


                ImGui::SetCursorPos({ 80.f, 220.f });
                ImGui::PushFont(SECONDFONT);
                ImGui::InputTextCPP(_xor_("##Key").c_str(), &key);
                ImGui::PopFont();

                ImGui::SetCursorPos({ 78.f, 280 });
                render_status_message(_xor_("Logs").c_str(), Menu::Get().message.c_str(), ImVec2(420, 40));

                ImGui::SetCursorPos({ 78.f, 350.f });
                static bool buttonPressed = false;

                if (Loading)
                    ImGui::BeginDisabled();

                if (inject_btn(Loading ? _xor_("Logging...").c_str() : _xor_("Login").c_str(), ImVec2(420, 42), 0))
                {
                    Loading = true;
                    message = "";
                    if (key.size() < 15) {
                        isError = true;
                        Loading = false;
                        message = std::string(_xor_("Invalid key").c_str());
                    }
                    else {
                        BitAuthApp.SaveSettings();
                        std::thread(LoginKey, key).detach();
                    }
                }

                if (Loading)
                    ImGui::EndDisabled();

            }
            else  if (page == 1) {
                float circleDiameter = 80.0f; 
                ImVec2 uv0(0, 0);
                ImVec2 uv1(1, 1);
                ImVec4 tintCol(1, 1, 1, 1);
                ImVec4 outlineCol(accent_color[0], accent_color[1], accent_color[2], 1.0f);
                float outlineThickness = 2.0f;
                if (!Menu::Get().store_logo.empty()) {
                    if (!my_texture && !tryed) {
                        LoadLogoFromServer(Menu::Get().store_logo);
                    }

                    if (my_texture) {

                        float posX = (windowSize.x - circleDiameter) / 2.0f;
                        float posY = (windowSize.y - circleDiameter) / 2.0f; 
                        ImGui::SetCursorPos({ posX, 10.f });

                        CircleImageWithOutline(my_texture, circleDiameter, uv0, uv1, tintCol, outlineCol, outlineThickness);
                    }
                }

                if (!Menu::Get().store_name.empty()) {
                    const char* textToDisplay = Menu::Get().store_name.c_str();

                    ImVec2 textSize = ImGui::CalcTextSize(textToDisplay);


                    float posX = (windowSize.x - textSize.x) / 2.0f;

                    float posY = 95.f;

                    ImGui::SetCursorPos({ posX, posY });
                    ImGui::PushFont(SECONDFONT);
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(200.0f / 255.0f, 200.0f / 255.0f, 200.0f / 255.0f, 1.0f));
                    ImGui::Text("%s", textToDisplay);
                    ImGui::PopStyleColor();
                    ImGui::PopFont();
                }


                float elementWidth = 380.0f;

                float posX = (windowSize.x - elementWidth) / 2.0f;

                ImGui::SetCursorPos({ posX, 120 });
                render_game(_xor_("Game Cheat").c_str(), Menu::Get().cheat_name, ImVec2(elementWidth, 40));

                ImGui::SetCursorPos({ posX, 170 });
                render_status(_xor_("Cheat Status").c_str(), message == std::string(_xor_("Cheat is under maintenance").c_str()) ? 1 : 0, ImVec2(elementWidth, 40));

                ImGui::SetCursorPos({ posX, 220 });
                render_time(_xor_("  Time Left    ").c_str(), Menu::Get().time_left, ImVec2(elementWidth, 40));

                ImGui::SetCursorPos({ posX, 310 });
                render_status_message(_xor_("Logs").c_str(), message, ImVec2(elementWidth, 40));



                ImGui::PushFont(FIRSTFONT);
                if (hasSpoofer) {
                    ImGui::SetCursorPos({ 98, 270 });
                    if (ImGui::Checkbox2(_xor_("Spoofer").c_str(), &BitAuthApp.wantSpoofer)) {
                        BitAuthApp.SaveSettings();
                    }
                    ImGui::SameLine();
                    if (ImGui::Button(_xor_("Configure Spoofer").c_str(), ImVec2(170.000f, 30.000f))) {
                        configureSpoofer = !configureSpoofer;
                    }
                }
                ImGui::PopFont();

                ImGui::SetCursorPos({ posX, 370.f });

                if (inject_btn(Loading ? _xor_("Loading...").c_str() : _xor_("Load Cheat").c_str(), ImVec2(380, 42), 0))
                {
                    Loading = true;
                    message = "";
                    if (key.size() < 15) {
                        isError = true;
                        Loading = false;
                        message = std::string(_xor_("Invalid key").c_str());
                    }
                    else {
                        std::thread(Load_Product).detach();
                    }
                }

            }

            const float buttonSize = 40.0f;
            ImGui::SetCursorPos({ 218.f, 440.f });

            if (!Menu::Get().store_discord_link.empty() || !Menu::Get().store_youtube_link.empty() || !Menu::Get().store_website_link.empty()) {
                if (!Menu::Get().store_discord_link.empty()) {
                    if (Discord_btn(BUTTONFONT, _xor_("D").c_str(), ImVec2(buttonSize, buttonSize), 0)) {
                        OpenLinkInWebBrowser(Menu::Get().store_discord_link.c_str());
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip(_xor_("Discord Server").c_str());
                    }
                    ImGui::SameLine();
                }

                if (!Menu::Get().store_website_link.empty()) {
                    if (Website_btn(BUTTONFONT, _xor_("W").c_str(), ImVec2(buttonSize, buttonSize), 0)) {
                        OpenLinkInWebBrowser(Menu::Get().store_website_link.c_str());
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip(_xor_("Official Website").c_str());
                    }
                    ImGui::SameLine();
                }

                if (!Menu::Get().store_youtube_link.empty()) {
                    if (Youtube_btn(BUTTONFONT, _xor_("Y").c_str(), ImVec2(buttonSize, buttonSize), 0)) {
                        OpenLinkInWebBrowser(Menu::Get().store_youtube_link.c_str());
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip(_xor_("YouTube Channel").c_str());
                    }
                }
            }

          
            ImGui::End();
            if (configureSpoofer) {
                float elementWidth = 250.0f;
                float elementHight = 380.0f;

                float posX = (windowSize.x - elementWidth) / 2.0f;
                float posY = (windowSize.y - elementHight) / 2.0f;

                ImGui::SetNextWindowPos({ posX, posY });
                ImGui::SetNextWindowSize(ImVec2(250, 380)); // format = width height

                if (ImGui::Begin(_xor_("Spoofer").c_str(), &done, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar))
                {
                    style.FrameRounding = 2.f;
                    ImGui::SetCursorPos(ImVec2(210, 10));
                    if (ImGui::Button(_xor_("X").c_str(), { 30, 30 }))
                    {
                        configureSpoofer = !configureSpoofer;
                    }
                    ImGui::SetCursorPos(ImVec2(35, 50));
                    if (ImGui::Checkbox2(_xor_("Spoof Disks").c_str(), &BitAuthApp.spoofDisk)) {
                        BitAuthApp.SaveSettings();
                    }
                    ImGui::SetCursorPos(ImVec2(35, 80));
                    if (ImGui::Checkbox2(_xor_("Null Disk").c_str(), &BitAuthApp.spoofDiskType)) {
                        BitAuthApp.SaveSettings();
                    }
                    ImGui::SetCursorPos(ImVec2(35, 110));
                    if (ImGui::Checkbox2(_xor_("Spoof MOBO").c_str(), &BitAuthApp.spoofMobo)) {
                        BitAuthApp.SaveSettings();
                    }
                    ImGui::SetCursorPos(ImVec2(35, 140));
                    if (ImGui::Checkbox2(_xor_("Spoof Boot").c_str(), &BitAuthApp.spoofBoot)) {
                        BitAuthApp.SaveSettings();
                    }
                    ImGui::SetCursorPos(ImVec2(35, 170));
                    if (ImGui::Checkbox2(_xor_("Spoof MAC").c_str(), &BitAuthApp.spoofMAC)) {
                        BitAuthApp.SaveSettings();
                    }
                    ImGui::SetCursorPos(ImVec2(35, 200));
                    if (ImGui::Checkbox2(_xor_("Spoof Monitor").c_str(), &BitAuthApp.spoofMonitor)) {
                        BitAuthApp.SaveSettings();
                    }
                    ImGui::SetCursorPos(ImVec2(35, 230));
                    if (ImGui::Checkbox2(_xor_("Spoof GPU").c_str(), &BitAuthApp.spoofGPU)) {
                        BitAuthApp.SaveSettings();
                    }
                    ImGui::SetCursorPos(ImVec2(35, 260));

                    if (ImGui::Checkbox2(_xor_("Spoof TPM").c_str(), &BitAuthApp.spoofTPM)) {
                        BitAuthApp.SaveSettings();
                    }
                    ImGui::SetCursorPos(ImVec2(35, 290));

                    if (ImGui::Checkbox2(_xor_("Spoof Registry").c_str(), &BitAuthApp.spoofFileRegistry)) {
                        BitAuthApp.SaveSettings();
                    }
                    ImGui::SetCursorPos(ImVec2(35, 320));
                    if (ImGui::Checkbox2(_xor_("Enable Serial Seed").c_str(), &BitAuthApp.spoofStaticSerial)) {
                        BitAuthApp.SaveSettings();
                    }
                    ImGui::SetCursorPos(ImVec2(35, 350));
                    if (BitAuthApp.spoofStaticSerial) {
                        ImGui::Checkbox2(_xor_("Reset Serial Seed").c_str(), &BitAuthApp.spoofStaticSerialRandomSeed);
                    }
                }
                ImGui::End();
            }
        }
        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = { 0.f, 0.f, 0.f, 0.f };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0); // Present with vsync
    }
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);
}


bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}


bool LoadTextureFromMemory(unsigned char* Buffer, size_t Size, ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height)
{
    // Load from disk into a raw RGBA buffer
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load_from_memory(Buffer, Size, &image_width, &image_height, NULL, 4);
    if (image_data == NULL)
        return false;

    // Create texture
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = image_width;
    desc.Height = image_height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;

    ID3D11Texture2D* pTexture = NULL;
    D3D11_SUBRESOURCE_DATA subResource;
    subResource.pSysMem = image_data;
    subResource.SysMemPitch = desc.Width * 4;
    subResource.SysMemSlicePitch = 0;
    g_pd3dDevice->CreateTexture2D(&desc, &subResource, &pTexture);

    // Create texture view
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = desc.MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;
    g_pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, out_srv);
    pTexture->Release();

    *out_width = image_width;
    *out_height = image_height;
    stbi_image_free(image_data);

    return true;
}