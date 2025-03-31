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
	node_hex64
};

class nodeBase {
public:
	char name[64];
	nodeType type;
	uint8_t size;

	nodeBase(const char* aName, nodeType aType, uint8_t aSize) {
		memset(name, 0, sizeof(name));
		if (aName) {
			memcpy(name, aName, sizeof(aName));
		}

		type = aType;
		size = aSize;
	}
};

int nameCounter = 0;

class uClass {
public:
	char name[64];
	std::vector<nodeBase> nodes;
	uintptr_t address = 0;

	uClass() {
		for (int i = 0; i < 50; i++) {
			nodeBase node = {0, node_hex64, 8};
			nodes.push_back(node);
		}
		memset(name, 0, sizeof(name));

		std::string newName = "Class_" + std::to_string(nameCounter++);
		memcpy(name, newName.data(), newName.size());
	}

	void drawNodes(BYTE* data, int len);
	void drawStringBytes(int i, BYTE* data, int pos, int size);
	void drawOffset(int i, int pos);
	void drawBytes(int i, BYTE* data, int pos, int size);
	void drawNumber(int i, uintptr_t num, int* pad);
	void drawFloat(int i, float num, int* pad = 0);
	void drawDouble(int i, double num, int* pad = 0);
	void drawHexNumber(int i, uintptr_t num, int pad);
};

void uClass::drawHexNumber(int i, uintptr_t num, int pad) {
	pad += 15;

	ImGui::SetCursorPos(ImVec2(320 + pad, 10 + 12 * i));
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

	ImGui::SetCursorPos(ImVec2(320, 10 + 12 * i));
	ImGui::PushStyleColor(ImGuiCol_Text, ImColor(163, 255, 240).Value);
	ImGui::Text(toDraw.c_str());
	ImGui::PopStyleColor();
}

void uClass::drawFloat(int i, float num, int* pad) {
	std::string toDraw = std::format("{:.3f}", num);
	if (toDraw.size() > 20) {
		toDraw = "###";
	}

	if (pad) {
		*pad += ImGui::CalcTextSize(toDraw.c_str()).x;
	}

	ImGui::SetCursorPos(ImVec2(320, 10 + 12 * i));
	ImGui::PushStyleColor(ImGuiCol_Text, ImColor(163, 255, 240).Value);
	ImGui::Text(toDraw.c_str());
	ImGui::PopStyleColor();
}

void uClass::drawNumber(int i, uintptr_t num, int* pad) {
	if (*pad > 0) {
		*pad += 15;
	}

	std::string toDraw = std::to_string(num);

	ImGui::SetCursorPos(ImVec2(320 + *pad, 10 + 12 * i));
	ImGui::PushStyleColor(ImGuiCol_Text, ImColor(255, 218, 133).Value);
	ImGui::Text(toDraw.c_str());
	ImGui::PopStyleColor();

	*pad += ImGui::CalcTextSize(toDraw.c_str()).x;
}

void uClass::drawOffset(int i, int pos) {
	ImGui::SetCursorPos(ImVec2(0, 10 + 12 * i));
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(.7f, .7f, .7f, 1.f));
	ImGui::Text(ui::toHexString(pos, 4).c_str());
	ImGui::PopStyleColor();
}

void uClass::drawBytes(int i, BYTE* data, int pos, int size) {
	for (int j = 0; j < size; j++) {
		BYTE byte = data[pos];
		ImGui::SetCursorPos(ImVec2(150 + j * 20, 10 + i * 12));
		ImGui::Text(ui::toHexString(byte, 2).c_str());
		pos++;
	}
}

void uClass::drawStringBytes(int i, BYTE* data, int pos, int size) {
	for (int j = 0; j < size; j++) {
		BYTE byte = data[pos];
		ImGui::SetCursorPos(ImVec2(45 + j * 12, 10 + i * 12));
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

void uClass::drawNodes(BYTE* data, int len) {
	size_t counter = 0;
	for (int i = 0; i < nodes.size(); i++) {
		auto& node = nodes[i];

		drawOffset(i, counter);

		size_t lCounter = 0;
		uintptr_t num = 0;
		float floatNum;
		double doubleNum;
		int pad = 0;

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
		}
	}
}

std::vector<uClass> g_Classes = { uClass{}, uClass{}, uClass{} };