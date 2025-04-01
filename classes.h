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

	uClass() {
		for (int i = 0; i < 50; i++) {
			nodeBase node = {0, node_hex64};
			nodes.push_back(node);
		}
		memset(name, 0, sizeof(name));
		memset(addressInput, 0, sizeof(addressInput));
		addressInput[0] = '0';

		std::string newName = "Class_" + std::to_string(nameCounter++);
		memcpy(name, newName.data(), newName.size());
	}

	void drawNodes(BYTE* data, int len);
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
			sizeDiff = sizeDiff % 4;
			nodes.insert(nodes.begin() + i + inserted++, { 0, node_hex32, selectNew });
		}
		else if (sizeDiff >= 2) {
			sizeDiff = sizeDiff % 2;
			nodes.insert(nodes.begin() + i + inserted++, { 0, node_hex16, selectNew });
		}
		else if (sizeDiff >= 1) {
			sizeDiff = sizeDiff - 1;
			nodes.insert(nodes.begin() + i + inserted++, { 0, node_hex8, selectNew });
		}
	}

	if (newNodes) {
		*newNodes = inserted - 1;
	}
}

void uClass::drawHexNumber(int i, uintptr_t num, int pad) {
	pad += 15;

	ImGui::SetCursorPos(ImVec2(455 + pad, 10 + 12 * i));
	ImGui::PushStyleColor(ImGuiCol_Text, ImColor(255, 162, 0).Value);
	ImGui::Text(("0x" + ui::toHexString(num, 0)).c_str());
	ImGui::PopStyleColor();
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

void uClass::drawNodes(BYTE* data, int len) {
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
}

std::vector<uClass> g_Classes = { uClass{}, uClass{}, uClass{} };