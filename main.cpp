#include <Windows.h>
#include <iostream>

#include <directx.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    WNDCLASS wc = { CS_CLASSDC, WndProc, 0, 0, hInstance, nullptr, nullptr, nullptr, nullptr, L"ImClassWnd" };
    RegisterClass(&wc);
    HWND hwnd = CreateWindow(wc.lpszClassName, L"ImClass", WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, nullptr, nullptr, hInstance, nullptr);

    if (!CreateDeviceD3D(hwnd)) {
        CleanupDeviceD3D();
        UnregisterClass(wc.lpszClassName, hInstance);
        return 1;
    }

    ShowWindow(hwnd, SW_HIDE);
    UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    
    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Hello, world!");
        ImGui::Text("This is an ImGui window with docking enabled.");
        ImGui::End();

        ImGui::Render();
        const float clear_color[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();

        g_pSwapChain->Present(1, 0);
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupDeviceD3D();
    DestroyWindow(hwnd);
    UnregisterClass(wc.lpszClassName, hInstance);

	return 0;
}