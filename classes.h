#pragma once

#include <vector>

namespace ui {
	extern std::string toHexString(uintptr_t address, int width);
}

template <typename T>
T Read(uintptr_t address);

enum nodeType {
	node_hex8,
	node_hex16,
	node_hex32,
	node_hex64,
	node_int8,
	node_int16,
	node_int32,
	node_int64
};

uint8_t typeSizes[] = {
	1,
	2,
	4,
	8,
	1,
	2,
	4,
	8
};

char typeNames[][21] = {
	"Hex8",
	"Hex16",
	"Hex32",
	"Hex64",
	"Int8",
	"Int16",
	"Int32",
	"Int64"
};

bool g_HoveringPointer = false;
class uClass;
uClass* g_PreviewClass = 0;

class nodeBase {
public:
	char name[64];
	nodeType type;
	uint8_t size;
	bool selected = false;

	nodeBase(const char* aName, nodeType aType, bool aSelected = false) {
		memset(name, 0, sizeof(name));
		if (aName) {
			memcpy(name, aName, sizeof(aName));
		}

		type = aType;
		size = typeSizes[aType];
		selected = aSelected;
	}
};

int nameCounter = 0;

class uClass {
public:
	char name[64];
	char addressInput[256];
	std::vector<nodeBase> nodes;
	uintptr_t address = 0;
	int varCounter = 0;
	size_t size;
	BYTE* data = 0;

	uClass(int nodeCount) {
		size = 0;

		for (int i = 0; i < nodeCount; i++) {
			nodeBase node = {0, mem::x32 ? node_hex32 : node_hex64};
			nodes.push_back(node);
			size += typeSizes[node.type];
		}
		memset(name, 0, sizeof(name));
		memset(addressInput, 0, sizeof(addressInput));
		addressInput[0] = '0';

		std::string newName = "Class_" + std::to_string(nameCounter++);
		memcpy(name, newName.data(), newName.size());

		data = (BYTE*)malloc(size);

		if (data) {
			memset(data, 0, size);
		}
		else {
			MessageBoxA(0, "Failed to allocate memory!", "ERROR", MB_ICONERROR);
		}
	}

	void sizeToNodes();
	void resize(int size);
	void drawNodes();
	void drawStringBytes(int i, BYTE* data, int pos, int size);
	void drawOffset(int i, int pos);
	void drawAddress(int i, int pos);
	void drawBytes(int i, BYTE* data, int pos, int size);
	void drawNumber(int i, uintptr_t num, int* pad);
	void drawFloat(int i, float num, int* pad = 0);
	void drawDouble(int i, double num, int* pad = 0);
	void drawHexNumber(int i, uintptr_t num, int pad);
	void drawControllers(int i, int counter);
	void changeType(int i, nodeType newType, bool selectNew = false, int* newNodes = 0);
	void changeType(nodeType newType);

	void drawInteger(int i, int64_t value, nodeType type);
};

void uClass::sizeToNodes() {
	size_t szNodes = 0;
	for (auto& node : nodes) {
		szNodes += node.size;
	}

	auto newData = (BYTE*)realloc(data, szNodes);
	if (!newData) {
		MessageBoxA(0, "Failed to reallocate memory!", "ERROR", MB_ICONERROR);
		return;
	}

	data = newData;
}

void uClass::resize(int mod) {
	assert(mod > 0 || mod == -8); // not intended

	int newSize = size + mod;
	if (newSize < 1) {
		return;
	}

	auto newData = (BYTE*)realloc(data, newSize);
	if (!newData) {
		MessageBoxA(0, "Failed to reallocate memory!", "ERROR", MB_ICONERROR);
		return;
	}

	data = newData;
	size = newSize;

	if (mod < 0) {
		nodes.erase(nodes.begin() + nodes.size() - 1);
		sizeToNodes();
	}
	else {
		int remaining = mod;
		while (remaining > 0) {
			if (remaining >= 8) {
				remaining = remaining - 8;
				nodes.push_back({ 0, node_hex64, false });
			}
			else if (remaining >= 4) {
				remaining = remaining - 4;
				nodes.push_back({ 0, node_hex32, false });
			}
			else if (remaining >= 2) {
				remaining = remaining - 2;
				nodes.push_back({ 0, node_hex16, false });
			}
			else if (remaining >= 1) {
				remaining = remaining - 1;
				nodes.push_back({ 0, node_hex8, false });
			}
		}
	}
}

void uClass::drawInteger(int i, int64_t value, nodeType type) {
	int y = 10 + 12 * i;
	auto& node = nodes[i];
	ImVec2 nameSize = ImGui::CalcTextSize(node.name);
	ImVec2 typenameSize = ImGui::CalcTextSize(typeNames[type]);

	ImGui::PushStyleColor(ImGuiCol_Text, ImColor(255, 218, 133).Value);
	ImGui::SetCursorPos(ImVec2(180, y));
	ImGui::Text(typeNames[type]);
	ImGui::PopStyleColor();

	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(.9f, .9f, .9f, 1.f));
	ImGui::SetCursorPos(ImVec2(180 + typenameSize.x + 15, y));
	ImGui::Text(node.name);
	ImGui::PopStyleColor();

	ImGui::SetCursorPos(ImVec2(180 + typenameSize.x + nameSize.x + 30, y));
	ImGui::Text("=  %lld", value);

	if (ImGui::BeginPopupContextItem(("copyintvar_" + std::to_string(i)).c_str())) {
		if (ImGui::Selectable("Copy value")) {
			ImGui::SetClipboardText(std::to_string(value).c_str());
		}
		ImGui::EndPopup();
	}
}

void uClass::changeType(nodeType newType) {
	for (int i = 0; i < nodes.size(); i++) {
		auto& node = nodes[i];
		if (node.selected) {
			int newNodes;
			changeType(i, newType, true, &newNodes);
			i += newNodes;
		}
	}
}

void uClass::changeType(int i, nodeType newType, bool selectNew, int* newNodes) {
	auto node = this->nodes[i];
	auto oldSize = node.size;
	auto typeSize = typeSizes[newType];

	nodes.erase(nodes.begin() + i);
	nodes.insert(nodes.begin() + i, { (newType > node_hex64) ? ("Var_" + std::to_string(varCounter++)).c_str() : 0, newType, selectNew});
	int inserted = 1;

	int sizeDiff = oldSize - typeSize;
	while (sizeDiff > 0) {
		if (sizeDiff >= 4) {
			sizeDiff = sizeDiff - 4;
			nodes.insert(nodes.begin() + i + inserted++, { 0, node_hex32, selectNew });
		}
		else if (sizeDiff >= 2) {
			sizeDiff = sizeDiff - 2;
			nodes.insert(nodes.begin() + i + inserted++, { 0, node_hex16, selectNew });
		}
		else if (sizeDiff >= 1) {
			sizeDiff = sizeDiff - 1;
			nodes.insert(nodes.begin() + i + inserted++, { 0, node_hex8, selectNew });
		}
	}

	sizeToNodes();

	if (newNodes) {
		*newNodes = inserted - 1;
	}
}

void uClass::drawHexNumber(int i, uintptr_t num, int pad) {
	pad += 15;

	ImColor color = ImColor(255, 162, 0);

	std::string numText = ui::toHexString(num, 0);

	std::string toDraw = ("0x" + numText);

	pointerInfo info;
	bool isPointer = mem::isPointer(num, &info);
	if (isPointer) {
		color = ImColor(255, 0, 0);

		if (info.moduleName == "") {
			toDraw = "[heap] " + toDraw;
		}
		else {
			toDraw = std::format("[{}] {} {}", info.section, info.moduleName.c_str(), toDraw.c_str());
		}

		std::string rttiNames;
		if (mem::rttiInfo(num, rttiNames)) {
			toDraw += rttiNames;
		}
	}

	ImVec2 textSize = ImGui::CalcTextSize(toDraw.c_str());

	ImGui::SetCursorPos(ImVec2(455 + pad, 10 + 12 * i));
	ImGui::PushStyleColor(ImGuiCol_Text, color.Value);
	ImGui::Text(toDraw.c_str());
	ImGui::PopStyleColor();
	if (ImGui::BeginPopupContextItem(("copyhex_" + std::to_string(i)).c_str())) {
		if (ImGui::Selectable("Copy value")) {
			ImGui::SetClipboardText(numText.c_str());
		}
		ImGui::EndPopup();
	}

	if (isPointer) {
		ImGui::SetItemKeyOwner(ImGuiKey_MouseWheelY);
		if (ImGui::IsItemHovered()) {
			g_HoveringPointer = true;
		}

		if (ImGui::BeginItemTooltip()) {
			ImGui::BeginChild("MemPreview_Child", ImVec2(0, 0), ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
			if (!g_PreviewClass) {
				g_PreviewClass = new uClass(15);

			}

			float mWheel = ImGui::GetIO().MouseWheel;
			if (mWheel > 0) {
				g_PreviewClass->resize(-8);
			}
			else if (mWheel < 0) {
				g_PreviewClass->resize(8);
			}

			g_PreviewClass->address = num;
			g_PreviewClass->drawNodes();

			ImGui::EndChild();
			ImGui::EndTooltip();
		}
	}

	auto buf = Read<readBuf<64>>(num);
	bool isString = true;
	for (int i = 0; i < 4; i++) {
		if (buf.data[i] < 21 || buf.data[i] > 126) {
			isString = false;
			break;
		}
	}

	std::string stringDraw = std::format("'{}'", (char*)buf.data);

	if (isString) {
		ImGui::SetCursorPos(ImVec2(455 + pad + textSize.x + 15, 10 + 12 * i));
		ImGui::PushStyleColor(ImGuiCol_Text, ImColor(3, 252, 140).Value);
		ImGui::Text(stringDraw.c_str());
		ImGui::PopStyleColor();
	}
}

void uClass::drawDouble(int i, double num, int* pad) {
	std::string toDraw = std::format("{:.3f}", num);
	if (toDraw.size() > 20) {
		toDraw = "#####";
	}

	if (pad) {
		*pad += ImGui::CalcTextSize(toDraw.c_str()).x;
	}

	ImGui::SetCursorPos(ImVec2(455, 10 + 12 * i));
	ImGui::PushStyleColor(ImGuiCol_Text, ImColor(163, 255, 240).Value);
	ImGui::Text(toDraw.c_str());
	ImGui::PopStyleColor();

	if (ImGui::BeginPopupContextItem(("copydouble_" + std::to_string(i)).c_str())) {
		if (ImGui::Selectable("Copy value")) {
			ImGui::SetClipboardText(toDraw.c_str());
		}
		ImGui::EndPopup();
	}
}

void uClass::drawFloat(int i, float num, int* pad) {
	std::string toDraw = std::format("{:.3f}", num);
	if (toDraw.size() > 20) {
		toDraw = "#####";
	}

	if (pad) {
		*pad += ImGui::CalcTextSize(toDraw.c_str()).x;
	}

	ImGui::SetCursorPos(ImVec2(455, 10 + 12 * i));
	ImGui::PushStyleColor(ImGuiCol_Text, ImColor(163, 255, 240).Value);
	ImGui::Text(toDraw.c_str());
	ImGui::PopStyleColor();

	if (ImGui::BeginPopupContextItem(("copyfloat_" + std::to_string(i)).c_str())) {
		if (ImGui::Selectable("Copy value")) {
			ImGui::SetClipboardText(toDraw.c_str());
		}
		ImGui::EndPopup();
	}
}

void uClass::drawNumber(int i, uintptr_t num, int* pad) {
	if (*pad > 0) {
		*pad += 15;
	}

	std::string toDraw = std::to_string(num);

	ImGui::SetCursorPos(ImVec2(455 + *pad, 10 + 12 * i));
	ImGui::PushStyleColor(ImGuiCol_Text, ImColor(255, 218, 133).Value);
	ImGui::Text(toDraw.c_str());
	ImGui::PopStyleColor();

	if (ImGui::BeginPopupContextItem(("copyint_" + std::to_string(i)).c_str())) {
		if (ImGui::Selectable("Copy value")) {
			ImGui::SetClipboardText(toDraw.c_str());
		}
		ImGui::EndPopup();
	}

	*pad += ImGui::CalcTextSize(toDraw.c_str()).x;
}

void uClass::drawOffset(int i, int pos) {
	ImGui::SetCursorPos(ImVec2(0, 10 + 12 * i));
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(.9f, .9f, .9f, 1.f));
	ImGui::Text(ui::toHexString(pos, 4).c_str());
	ImGui::PopStyleColor();
}

void uClass::drawAddress(int i, int pos) {
	ImGui::SetCursorPos(ImVec2(50, 10 + 12 * i));
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(.7f, .7f, .7f, 1.f));
	ImGui::Text(ui::toHexString(this->address + pos, 16).c_str());
	ImGui::PopStyleColor();
}

void uClass::drawBytes(int i, BYTE* data, int pos, int size) {
	for (int j = 0; j < size; j++) {
		BYTE byte = data[pos];
		ImGui::SetCursorPos(ImVec2(285 + j * 20, 10 + i * 12));
		ImGui::Text(ui::toHexString(byte, 2).c_str());
		pos++;
	}
}

void uClass::drawStringBytes(int i, BYTE* data, int pos, int size) {
	for (int j = 0; j < size; j++) {
		BYTE byte = data[pos];
		ImGui::SetCursorPos(ImVec2(180 + j * 12, 10 + i * 12));
		if (byte > 32 && byte < 127) {
			ImGui::Text("%c", byte);
		}
		else if (byte != 0) {
			ImGui::Text(".", byte);
		}
		else {
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(.3f, .3f, .3f, 1.f));
			ImGui::Text(".", byte);
			ImGui::PopStyleColor();
		}
		pos++;
	}
}

void uClass::drawControllers(int i, int counter) {
	auto& node = nodes[i];

	ImGui::SetCursorPos(ImVec2(0, 14 + i * 12));
	if (ImGui::Selectable(("##Controller_" + std::to_string(i)).c_str(), node.selected, 0, ImVec2(0, 6))) {
		if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
			node.selected = !node.selected;
		}
		else if (ImGui::IsKeyDown(ImGuiKey_LeftShift)) {
			int min = INT_MAX;
			for (int j = 0; j < nodes.size(); j++) {
				if (nodes[j].selected) {
					if (j < min) {
						min = j;
					}
				}
			}

			if (min > i) {
				for (int j = min; j >= i; j--) {
					nodes[j].selected = true;
				}
			}
			else {
				for (int j = min; j <= i; j++) {
					nodes[j].selected = true;
				}
			}
		}
		else {
			for (int j = 0; j < nodes.size(); j++) {
				nodes[j].selected = false;
			}
			nodes[i].selected = true;
		}
	}

	if (ImGui::BeginPopupContextItem(("##NodePopup_" + std::to_string(i)).c_str())) {
		if (!node.selected) {
			for (int j = 0; j < nodes.size(); j++) {
				nodes[j].selected = false;
			}
			nodes[i].selected = true;
		}

		if (ImGui::BeginMenu("Change Type")) {
			// the imvec2 just ensures the width of this menu
			if (ImGui::Selectable("Hex8", false, 0, ImVec2(100, 0))) {
				changeType(node_hex8);
			}
			if (ImGui::Selectable("Hex16")) {
				changeType(node_hex16);
			}
			if (ImGui::Selectable("Hex32")) {
				changeType(node_hex32);
			}
			if (ImGui::Selectable("Hex64")) {
				changeType(node_hex64);
			}

			ImGui::Separator();
			
			if (ImGui::Selectable("Int64")) {
				changeType(node_int64);
			}
			if (ImGui::Selectable("Int32")) {
				changeType(node_int32);
			}
			if (ImGui::Selectable("Int16")) {
				changeType(node_int16);
			}
			if (ImGui::Selectable("Int8")) {
				changeType(node_int8);
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Add Bytes")) {
			if (ImGui::Selectable("4 Bytes")) {
				resize(4);
			}
			if (ImGui::Selectable("8 Bytes")) {
				resize(8);
			}
			if (ImGui::Selectable("64 Bytes")) {
				resize(64);
			}
			if (ImGui::Selectable("512 Bytes")) {
				resize(512);
			}
			if (ImGui::Selectable("1024 Bytes")) {
				resize(1024);
			}
			if (ImGui::Selectable("4096 Bytes")) {
				resize(4096);
			}
			if (ImGui::Selectable("8192 Bytes")) {
				resize(8192);
			}
			ImGui::EndMenu();
		}

		if (ImGui::Selectable("Delete")) {
			for (int j = nodes.size() - 1; j >= 0; j--) {
				if (nodes[j].selected) {
					nodes.erase(nodes.begin() + j);
				}
			}
		}
		if (ImGui::Selectable("Copy Address")) {
			ImGui::SetClipboardText(ui::toHexString(this->address + counter, 0).c_str());
		}
		ImGui::EndPopup();
	}
}

void uClass::drawNodes() {
	mem::read(this->address, this->data, this->size);
	
	size_t counter = 0;
	for (int i = 0; i < nodes.size(); i++) {
		auto& node = nodes[i];

		drawControllers(i, counter);
		drawOffset(i, counter);
		drawAddress(i, counter);

		size_t lCounter = 0;
		uintptr_t num = 0;
		float floatNum;
		double doubleNum;
		int pad = 0;

		// this kinda sucks
		switch (node.type) {
		case node_hex8:
			drawStringBytes(i, data, counter, 1);
			drawBytes(i, data, counter, 1);

			num = *(uint8_t*)((uintptr_t)data + counter);
			drawNumber(i, num, &pad);
			drawHexNumber(i, num, pad);

			counter += 1;
			break;
		case node_hex16:
			drawStringBytes(i, data, counter, 2);
			drawBytes(i, data, counter, 2);

			num = *(uint16_t*)((uintptr_t)data + counter);
			drawNumber(i, num, &pad);
			drawHexNumber(i, num, pad);

			counter += 2;
			break;
		case node_hex32:
			drawStringBytes(i, data, counter, 4);
			drawBytes(i, data, counter, 4);

			floatNum = *(float*)((uintptr_t)data + counter);
			drawFloat(i, floatNum, &pad);

			num = *(uint32_t*)((uintptr_t)data + counter);
			drawNumber(i, num, &pad);
			drawHexNumber(i, num, pad);

			counter += 4;
			break;
		case node_hex64:
			drawStringBytes(i, data, counter, 8);
			drawBytes(i, data, counter, 8);

			doubleNum = *(double*)((uintptr_t)data + counter);
			drawDouble(i, doubleNum, &pad);

			num = *(uint64_t*)((uintptr_t)data + counter);
			drawNumber(i, num, &pad);
			drawHexNumber(i, num, pad);

			counter += 8;
			break;
		case node_int64:
			drawInteger(i, *(int64_t*)((uintptr_t)data + counter), node_int64);
			counter += 8;
			break;
		case node_int32:
			drawInteger(i, *(int32_t*)((uintptr_t)data + counter), node_int32);
			counter += 4;
			break;
		case node_int16:
			drawInteger(i, *(int16_t*)((uintptr_t)data + counter), node_int16);
			counter += 2;
			break;
		case node_int8:
			drawInteger(i, *(int8_t*)((uintptr_t)data + counter), node_int8);
			counter += 1;
			break;
		}
	}

	if (!g_HoveringPointer) {
		if (g_PreviewClass) {
			free(g_PreviewClass->data);
			delete g_PreviewClass;
			g_PreviewClass = 0;
		}
	}
}

std::vector<uClass> g_Classes = { uClass(50), uClass(50), uClass(50) };

void initClasses(bool isX32) {
	if (g_Classes.empty() || isX32 != mem::x32) {
		mem::x32 = isX32;
		g_Classes = { uClass(50), uClass(50), uClass(50) };
	}
}