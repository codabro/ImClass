#pragma once

#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cstdint>
#include <regex>
#include <imgui/imgui_internal.h>

#include "patterns.h"

namespace ui {
    bool open = true;
    bool processWindow = false;
    bool signaturesWindow = false;
    bool sigScanWindow = false;
    bool exportWindow = false;
    std::string exportedClass;
    inline std::optional<PatternScanResult> patternResults;
    char addressInput[256] = "0";
	char module[512] = { 0 };
	char signature[512] = { 0 };

    ImVec2 mainPos;
    ImVec2 signaturePos = {0, 0};

    void init(HWND hwnd);
	void renderProcessWindow();
	void renderMain();
    void renderExportWindow();
	void render();
    bool searchMatches(std::string str, std::string term);
    uintptr_t toAddress(std::string address);
    std::string toHexString(uintptr_t address, int width = 0);
    bool isValidHex(std::string& str);
    void updateAddress(uintptr_t newAddress, uintptr_t* dest = 0);
    void renderSignatureResults();
    void renderSignatureScan();
    void updateAddressBox(char* dest, char* src);
}

void ui::updateAddressBox(char* dest, char* src) {
    memset(dest, 0, sizeof(addressInput));
    memcpy(dest, src, strlen(src));
}

inline void ui::renderSignatureScan()
{
	static bool oSigScanWindow = false;
	if (!sigScanWindow) {
		oSigScanWindow = sigScanWindow;
		return;
	}

	const float entryHeight = ImGui::GetTextLineHeightWithSpacing();
	constexpr float headerHeight = 30.0f;
	constexpr float footerHeight = 30.0f;
	constexpr float minWidth = 300.0f;
	constexpr float padding = 20.0f;

	constexpr int numElements = 3;
	const float contentHeight = (entryHeight * numElements) + padding;
	const float windowHeight = min(headerHeight + contentHeight + footerHeight, 300.0f);
    static bool hasSetPos = false;

	if (sigScanWindow != oSigScanWindow) {
        if (hasSetPos) {
            ImGui::SetNextWindowPos(signaturePos, ImGuiCond_Always);
        }
        else
        {
            ImVec2 defaultPos = ImVec2(mainPos.x + 50, mainPos.y + 50);
            ImGui::SetNextWindowPos(defaultPos, ImGuiCond_Always);
			signaturePos = ImVec2(defaultPos);
            hasSetPos = true;
        }
		ImGui::SetNextWindowSize(ImVec2(minWidth, windowHeight), ImGuiCond_Always);
	}
	oSigScanWindow = sigScanWindow;

	ImGui::Begin("Signature Scanner", &sigScanWindow);
	ImGui::InputText("Module", module, sizeof(module));
	ImGui::InputText("Signature", signature, sizeof(signature));
	if (ImGui::Button("Scan")) {
		PatternInfo pattern;
		pattern.pattern = signature;
		patternResults = pattern::scanPattern(pattern, module);
		if (patternResults != std::nullopt && !patternResults.value().matches.empty()) {
			signaturesWindow = true;
		}
		sigScanWindow = false;
	}
	ImGui::SameLine();
	if (ImGui::Button("Cancel")) {
		sigScanWindow = false;
	}

    signaturePos = ImGui::GetWindowPos();

	ImGui::End();
}


void ui::renderSignatureResults() {
	static bool oSignaturesWindow = false;
	if (!signaturesWindow) {
		oSignaturesWindow = signaturesWindow;
		return;
	}

	const float entryHeight = ImGui::GetTextLineHeightWithSpacing();
	constexpr float headerHeight = 30.0f;
	constexpr float footerHeight = 30.0f;
	constexpr float minWidth = 200.0f;

	const size_t numEntries = patternResults.has_value() && !patternResults.value().matches.empty()
		? patternResults.value().matches.size()
		: 1;

	const float windowHeight = min(headerHeight + (entryHeight * numEntries) + footerHeight, 500.0f);

	if (signaturesWindow != oSignaturesWindow) {
        ImGui::SetNextWindowPos(signaturePos, ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(minWidth, windowHeight), ImGuiCond_Always);
	}
	oSignaturesWindow = signaturesWindow;

	ImGui::Begin("Signatures", &signaturesWindow);
	const ImVec2 wndSize = ImGui::GetWindowSize();
	ImGui::BeginChild("##SignaturesList", ImVec2(0, wndSize.y - 30));

	if (patternResults.has_value() && !patternResults.value().matches.empty()) {

        PatternScanResult& results = patternResults.value();

		for (uintptr_t match : results.matches) {
			const std::string address = toHexString(match);
            const char* cAddr = address.c_str();
			if (ImGui::Selectable(cAddr)) {
                if (g_Classes.size() >= g_SelectedClass) {
                    uClass& cClass = g_Classes[g_SelectedClass];
                    updateAddressBox(addressInput, (char*)(cAddr));
                    updateAddressBox(cClass.addressInput, (char*)(cAddr));
                    updateAddress(match, &cClass.address);
                    signaturesWindow = false;
                }
			}
		}
	}
	else {
		ImGui::Text("No signatures found.");
	}

	ImGui::EndChild();
	ImGui::End();
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
        if (ImGui::BeginMenu("Tools")) {
            if (ImGui::MenuItem("Signature Scanner"))
            {
                sigScanWindow = true;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();

        ImGui::Columns(2);

        ImGui::SetColumnOffset(1, 150);

        float columnOffset = ImGui::GetColumnOffset(1);

        ImVec2 wndSize = ImGui::GetWindowSize();

        ImGui::BeginChild("ClassesChild", ImVec2(columnOffset - 15, wndSize.y - 54), 1, ImGuiWindowFlags_NoScrollbar);

        static char renameBuf[64] = { 0 };
		static int renamedClass = -1;

        for (int i = 0; i < g_Classes.size(); i++) {
            auto& lClass = g_Classes[i];

            if (renamedClass != i) {
                if (ImGui::Selectable(lClass.name, (i == g_SelectedClass))) {
                    g_SelectedClass = i;
                    updateAddress(lClass.address);
                    updateAddressBox(addressInput, lClass.addressInput);
                }

                if (ImGui::BeginPopupContextItem(("##ClassContext" + std::to_string(i)).c_str())) {
                    if (ImGui::MenuItem("Export Class")) {
                        uClass& sClass = g_Classes[i];
                        std::string exported = sClass.exportClass();
                        exportedClass = exported;
                        exportWindow = true;
                    }
                    if (ImGui::MenuItem("New Class")) {
                        g_Classes.push_back({ uClass(50) });
                    }
                    if (ImGui::MenuItem("Delete")) {
                        if (g_Classes.size() > 0) {
                            uClass& sClass = g_Classes[i];
                            free(sClass.data);
                            g_Classes.erase(g_Classes.begin() + i);
                            if (g_SelectedClass > 0 && g_SelectedClass > g_Classes.size() - 1) {
                                g_SelectedClass--;
                            }
                        }
                    }
                    ImGui::EndPopup();
                }

                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                    renamedClass = i;
                    memset(renameBuf, 0, sizeof(renameBuf));
                    memcpy(renameBuf, lClass.name, sizeof(renameBuf));
                }
            }
            else {
                ImGui::SetKeyboardFocusHere();

                if (ImGui::InputText("##RenameClass", renameBuf, sizeof(renameBuf), ImGuiInputTextFlags_EnterReturnsTrue)) {
                    memcpy(lClass.name, renameBuf, sizeof(renameBuf));
                    renamedClass = -1;
                }

                if (!ImGui::IsItemActive() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                    renamedClass = -1;
                }
            }
        }

        ImGui::EndChild();

        if (g_Classes.empty()) {
            if (ImGui::BeginPopupContextItem("##ClassesContext")) {
                if (ImGui::MenuItem("New Class")) {
                    g_Classes.push_back({ uClass(50) });
                }
                ImGui::EndPopup();
            }
            ImGui::End();
            return;
        }

        uClass& sClass = g_Classes[g_SelectedClass];

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

        ImGui::BeginChild("MemView", ImVec2(0, 0), 0, g_HoveringPointer? ImGuiWindowFlags_NoScrollWithMouse : 0);
        g_HoveringPointer = false;
        g_InPopup = false;
        //auto buf = Read<readBuf<4096>>(sClass.address);
        sClass.drawNodes();
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

void ui::renderExportWindow() {
    if (!exportWindow) {
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(437, 305));
    ImGui::Begin("Exported class", &exportWindow, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::InputTextMultiline("##exportedclass", exportedClass.data(), exportedClass.size(), ImVec2(420, 250));
    if (ImGui::Button("Copy")) {
        ImGui::SetClipboardText(exportedClass.c_str());
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
    renderExportWindow();
    renderSignatureScan();
    renderSignatureResults();    
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
    style.Colors[ImGuiCol_Header] = ImColor(66, 135, 245, 75);
    style.Colors[ImGuiCol_HeaderActive] = ImColor(66, 135, 245, 75);
    style.Colors[ImGuiCol_HeaderHovered] = ImColor(66, 135, 245, 50);

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