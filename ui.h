#pragma once

#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cstdint>
#include <regex>
#include <imgui/imgui_internal.h>

namespace ui {
    bool open = true;
    bool processWindow = false;
    char addressInput[256] = "0";

    ImVec2 mainPos;

    void init(HWND hwnd);
	void renderProcessWindow();
	void renderMain();
	void render();
    bool searchMatches(std::string str, std::string term);
    uintptr_t toAddress(std::string address);
    std::string toHexString(uintptr_t address, int width = 0);
    bool isValidHex(std::string& str);
    void updateAddress(uintptr_t newAddress, uintptr_t* dest = 0);
    void updateAddressBox(char* dest, char* src);
}

void ui::updateAddressBox(char* dest, char* src) {
    memset(dest, 0, sizeof(addressInput));
    memcpy(dest, src, strlen(src));
}

void ui::updateAddress(uintptr_t newAddress, uintptr_t* dest) {
    if (newAddress != 0 && strcmp(addressInput, "0")) {
        if (dest) {
            *dest = newAddress;
        }
    }
}

void ui::renderMain() {
    ImGui::Begin("ImClass", &open, ImGuiWindowFlags_MenuBar);
    mainPos = ImGui::GetWindowPos();
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Attach to process")) {
                processWindow = true;
                mem::getProcessList();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Sigma")) {
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();

        ImGui::Columns(2);

        ImGui::SetColumnOffset(1, 150);

        float columnOffset = ImGui::GetColumnOffset(1);

        ImVec2 wndSize = ImGui::GetWindowSize();

        ImGui::BeginChild("ClassesChild", ImVec2(columnOffset - 15, wndSize.y - 54), 1);
        static int selectedClass = 0;
        for (int i = 0; i < g_Classes.size(); i++) {
            auto& lClass = g_Classes[i];
            if (ImGui::Selectable(lClass.name, (i == selectedClass))) {
                selectedClass = i;
                updateAddress(lClass.address);
                updateAddressBox(addressInput, lClass.addressInput);
            }
        }
        ImGui::EndChild();

        uClass& sClass = g_Classes[selectedClass];

        ImGui::NextColumn();

        ImGui::BeginChild("MemoryViewChild", ImVec2(wndSize.x - columnOffset - 16, wndSize.y - 54), 1);
        ImGui::SetNextItemWidth(150);
        if (ImGui::InputText("Address", addressInput, sizeof(addressInput), ImGuiInputTextFlags_EnterReturnsTrue)) {
            uintptr_t newAddress = addressParser::parseInput(addressInput);
            updateAddress(newAddress, &sClass.address);
            updateAddressBox(sClass.addressInput, addressInput);
        }

        static bool oInputFocused = false;

        // ImGui::IsItemFocused() doesn't really work for losing focus from the inputtext
        // Idk why, so I'm taking the logic from InputText itself, it works...
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        ImGuiID id = window->GetID("Address");

        bool inputFocused = (GImGui->ActiveId == id);
        if (oInputFocused && !inputFocused) {
            updateAddress(sClass.address);
            updateAddressBox(addressInput, sClass.addressInput);
        }

        oInputFocused = inputFocused;

        //auto drawList = ImGui::GetWindowDrawList();
        ImGui::BeginChild("MemView");
        auto buf = Read<readBuf<4096>>(sClass.address);
        sClass.drawNodes(buf.data, 4096);
        ImGui::EndChild();

        ImGui::EndChild();
    }
    ImGui::End();
}

void ui::renderProcessWindow() {
    static bool oProcessWindow = false;

    if (!processWindow) {
        oProcessWindow = processWindow;
        return;
    }

    if (processWindow != oProcessWindow) {
        ImGui::SetNextWindowPos(ImVec2(mainPos.x + 50, mainPos.y + 50), ImGuiCond_Always);
    }
    oProcessWindow = processWindow;

    ImGui::Begin("Attach to process", &processWindow);

    static DWORD selected = 0;
    static char searchTerm[64] = { 0 };
    ImGui::InputText("Search", searchTerm, sizeof(searchTerm));

    ImVec2 wndSize = ImGui::GetWindowSize();
    ImGui::BeginChild("##ProcessList", ImVec2(0, wndSize.y - 81));
    for (int i = 0; i < mem::processes.size(); i++) {
        auto& proc = mem::processes[i];
        std::string procName = std::string(proc.name.begin(), proc.name.end());
        if (searchTerm[0] != 0 && !searchMatches(procName, searchTerm)) {
            continue;
        }

        if (ImGui::Selectable((procName + " " + std::to_string(proc.pid)).c_str(), (proc.pid == selected))) {
            selected = proc.pid;
        }
    }
    ImGui::EndChild();

    if (ImGui::Button("Confirm")) {
        processWindow = false;
        mem::initProcess(selected);
    }

    ImGui::End();
}

bool ui::searchMatches(std::string str, std::string term) {
    std::transform(str.begin(), str.end(), str.begin(), tolower);
    std::transform(term.begin(), term.end(), term.begin(), tolower);
    return str.find(term) != std::string::npos;
}

void ui::render() {
    renderMain();
    renderProcessWindow();
}

void ui::init(HWND hwnd) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigViewportsNoDefaultParent = true;

    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);
}

uintptr_t ui::toAddress(std::string address) {
    address.erase(std::remove(address.begin(), address.end(), ' '), address.end());

    if (!isValidHex(address)) {
        return 0;
    }

    if (address.size() > 2 && (address[0] == '0') && (address[1] == 'x' || address[1] == 'X')) {
        address = address.substr(2);
    }

    std::uintptr_t result = 0;
    std::istringstream(address) >> std::hex >> result;

    return result;
}

std::string ui::toHexString(uintptr_t address, int width) {
    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setw(width) << std::setfill('0') << address;
    return ss.str();
}

bool ui::isValidHex(std::string& str) {
    static std::regex hexRegex("^(0x|0X)?[0-9a-fA-F]+$");
    return std::regex_match(str, hexRegex);
}