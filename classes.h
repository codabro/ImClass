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
	node_int64,
	node_uint8,
	node_uint16,
	node_uint32,
	node_uint64,
	node_float,
	node_double,
	node_vector4,
	node_vector3,
	node_vector2,
	node_matrix4x4,
	node_matrix3x4,
	node_matrix3x3,
	node_bool,
	node_max
};

struct Vector4 {
	float x, y, z, w;
};

struct Vector3 {
	float x, y, z;
};

struct Vector2 {
	float x, y;
};

struct Matrix4x4 {
	float m[4][4];
};

struct Matrix3x4 {
	float m[3][4];
};

struct Matrix3x3 {
	float m[3][3];
};

struct nodeTypeInfo {
	nodeType type;
	uint8_t size;
	char name[21];
	ImColor color;
	char codeName[16];
};

nodeTypeInfo nodeData[] = {
	{node_hex8, sizeof(uint8_t), "Hex8", ImColor(255, 255, 255)},
	{node_hex16, sizeof(uint16_t), "Hex16", ImColor(255, 255, 255)},
	{node_hex32, sizeof(uint32_t), "Hex32", ImColor(255, 255, 255)},
	{node_hex64, sizeof(uint64_t), "Hex64", ImColor(255, 255, 255)},
	{node_int8, sizeof(int8_t), "Int8", ImColor(255, 200, 0), "int8_t"},
	{node_int16, sizeof(int16_t), "Int16", ImColor(255, 200, 0), "int16_t"},
	{node_int32, sizeof(int32_t), "Int32", ImColor(255, 200, 0), "int"},
	{node_int64, sizeof(int64_t), "Int64", ImColor(255, 200, 0), "int64_t"},
	{node_uint8, sizeof(uint8_t), "UInt8", ImColor(7, 247, 163), "uint8_t"},
	{node_uint16, sizeof(uint16_t), "UInt16", ImColor(7, 247, 163), "uint16_t"},
	{node_uint32, sizeof(uint32_t), "UInt32", ImColor(7, 247, 163), "uint32_t"},
	{node_uint64, sizeof(uint64_t), "UInt64", ImColor(7, 247, 163), "uint64_t"},
	{node_float, sizeof(float), "Float", ImColor(225, 143, 255), "float"},
	{node_double, sizeof(double), "Double", ImColor(187, 0, 255), "double"},
	{node_vector4, sizeof(Vector4), "Vector4", ImColor(115, 255, 124), "Vector4"},
	{node_vector3, sizeof(Vector3), "Vector3", ImColor(115, 255, 124), "Vector3"},
	{node_vector2, sizeof(Vector2), "Vector2", ImColor(115, 255, 124), "Vector2"},
	{node_matrix4x4, sizeof(Matrix4x4), "Matrix4x4", ImColor(3, 252, 144), "matrix4x4_t"},
	{node_matrix3x4, sizeof(Matrix3x4), "Matrix3x4", ImColor(3, 252, 144), "matrix3x4_t"},
	{node_matrix3x3, sizeof(Matrix3x3), "Matrix3x3", ImColor(3, 252, 144), "matrix3x3_t"},
	{node_bool, sizeof(bool), "Bool", ImColor(0, 183, 255), "bool"}
};

bool g_HoveringPointer = false;
bool g_InPopup = false;

class nodeBase {
public:
	char name[64];
	nodeType type;
	uint8_t size;
	bool selected = false;

	nodeBase(const char* aName, nodeType aType, bool aSelected = false) {
		memset(name, 0, sizeof(name));
		if (aName) {
			memcpy(name, aName, sizeof(name));
		}

		type = aType;
		size = nodeData[aType].size;
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

	uClass(int nodeCount, bool incrementCounter = true) {
		size = 0;

		for (int i = 0; i < nodeCount; i++) {
			nodeBase node = {0, mem::x32 ? node_hex32 : node_hex64};
			nodes.push_back(node);
			size += nodeData[node.type].size;
		}
		memset(name, 0, sizeof(name));
		memset(addressInput, 0, sizeof(addressInput));
		addressInput[0] = '0';

		std::string newName = "Class_" + std::to_string(nameCounter);
		memcpy(name, newName.data(), newName.size());

		data = (BYTE*)malloc(size);

		if (data) {
			memset(data, 0, size);
		}
		else {
			MessageBoxA(0, "Failed to allocate memory!", "ERROR", MB_ICONERROR);
		}

		if (incrementCounter) {
			nameCounter++;
		}
	}

	void sizeToNodes();
	void resize(int size);
	void drawNodes();
	void drawStringBytes(int i, BYTE* data, int pos, int size);
	void drawOffset(int i, int pos);
	void drawAddress(int i, int pos);
	void drawBytes(int i, BYTE* data, int pos, int size);
	void drawNumber(int i, int64_t num, int* pad);
	void drawFloat(int i, float num, int* pad = 0);
	void drawDouble(int i, double num, int* pad = 0);
	void drawHexNumber(int i, uintptr_t num, int pad);
	void drawControllers(int i, int counter);
	void changeType(int i, nodeType newType, bool selectNew = false, int* newNodes = 0);
	void changeType(nodeType newType);

	int drawVariableName(int i, nodeType type);
	void copyPopup(int i, std::string toCopy, std::string id);
	void drawInteger(int i, int64_t value, nodeType type);
	void drawUInteger(int i, uint64_t value, nodeType type);
	void drawFloatVar(int i, float value);
	void drawDoubleVar(int i, double value);
	void drawVector4(int i, Vector4& value);
	void drawVector3(int i, Vector3& value);
	void drawVector2(int i, Vector2& value);
	void drawMatrix4x4(int i, Matrix4x4& value);
	void drawMatrix3x4(int i, Matrix3x4& value);
	void drawMatrix3x3(int i, Matrix3x3& value);
	void drawBool(int i, bool value);

	std::string exportClass();
};

uClass g_PreviewClass(15);

std::string uClass::exportClass() {
	std::string exportedClass = std::format("class {} {{\npublic:", name);
	int pad = 0;
	for (auto& node : nodes) {
		if (node.type <= node_hex64) {
			pad += node.size;
			continue;
		}
		else {
			if (pad > 0) {
				std::string var = std::format("\tBYTE pad[{}];", pad);
				exportedClass = exportedClass + "\n" + var;
				pad = 0;
			}
		}
		auto& data = nodeData[node.type];
		std::string var = std::format("\t{} {};", data.codeName, node.name);
		exportedClass = exportedClass + "\n" + var;
	}
	exportedClass += "\n};";

	return exportedClass;
}

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
	size = szNodes;
}

void uClass::resize(int mod) {
	assert(mod > 0 || mod == -8); // not intended

	int newSize = size + mod;
	if (newSize < 1) {
		return;
	}

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
		sizeToNodes();
		memset(data, 0, size);
	}
}

void uClass::copyPopup(int i, std::string toCopy, std::string id) {
	std::string sID = "copyvar_" + id + std::to_string(i);

	if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1)) {
		g_InPopup = true;
		ImGui::OpenPopup(sID.c_str());
	}

	if (ImGui::BeginPopup(sID.c_str())) {
		g_InPopup = true;
		if (ImGui::Selectable("Copy value")) {
			ImGui::SetClipboardText(toCopy.c_str());
		}
		ImGui::EndPopup();
	}
}

int uClass::drawVariableName(int i, nodeType type) {
	auto& node = nodes[i];
	ImVec2 nameSize = ImGui::CalcTextSize(node.name);
	
	auto& data = nodeData[type];
	auto typeName = data.name;
	ImVec2 typenameSize = ImGui::CalcTextSize(typeName);

	ImGui::PushStyleColor(ImGuiCol_Text, data.color.Value);
	ImGui::SetCursorPos(ImVec2(180, 0));
	ImGui::Text(typeName);
	ImGui::PopStyleColor();

	static char renameBuf[64] = { 0 };
	static int renamedNode = -1;

	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(.9f, .9f, .9f, 1.f));
	ImGui::SetCursorPos(ImVec2(180 + typenameSize.x + 15, 0));
	if (renamedNode != i) {
		ImGui::Text(node.name);

		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
			renamedNode = i;
			memcpy(renameBuf, node.name, sizeof(renameBuf));
		}
	}
	else {
		ImGui::SetKeyboardFocusHere();
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 1));
		ImGui::SetNextItemWidth(200);
		nameSize.x = 200;
		if (ImGui::InputText("##RenameNode", renameBuf, sizeof(renameBuf), ImGuiInputTextFlags_EnterReturnsTrue)) {
			memcpy(node.name, renameBuf, sizeof(renameBuf));
			renamedNode = -1;
		}
		ImGui::PopStyleVar();

		if (!ImGui::IsItemActive() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
			renamedNode = -1;
		}
	}
	ImGui::PopStyleColor();

	return typenameSize.x + nameSize.x;
}

void uClass::drawBool(int i, bool value) {
	auto& node = nodes[i];

	int xPad = drawVariableName(i, node_bool);

	ImGui::SetCursorPos(ImVec2(180 + xPad + 30, 0));
	ImGui::Text(value ? "=  true" : "=  false");
}

void uClass::drawMatrix4x4(int i, Matrix4x4& value) {
	auto& node = nodes[i];

	int xPad = drawVariableName(i, node_matrix4x4);
	int mPad = 0;
	int y = 0;

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			float val = value.m[i][j];
			ImGui::SetCursorPos(ImVec2(180 + xPad + 30 + mPad, y));
			ImGui::Text("%.3f", val);
			mPad += 60;
		}

		y += 15;
		mPad = 0;
	}
}

void uClass::drawMatrix3x4(int i, Matrix3x4& value) {
	auto& node = nodes[i];

	int xPad = drawVariableName(i, node_matrix3x4);
	int mPad = 0;
	int y = 0;

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 4; j++) {
			float val = value.m[i][j];
			ImGui::SetCursorPos(ImVec2(180 + xPad + 30 + mPad, y));
			ImGui::Text("%.3f", val);
			mPad += 60;
		}

		y += 15;
		mPad = 0;
	}
}

void uClass::drawMatrix3x3(int i, Matrix3x3& value) {
	auto& node = nodes[i];

	int xPad = drawVariableName(i, node_matrix3x3);
	int mPad = 0;
	int y = 0;

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			float val = value.m[i][j];
			ImGui::SetCursorPos(ImVec2(180 + xPad + 30 + mPad, y));
			ImGui::Text("%.3f", val);
			mPad += 60;
		}

		y += 15;
		mPad = 0;
	}
}

void uClass::drawVector4(int i, Vector4& value) {
	auto& node = nodes[i];

	int xPad = drawVariableName(i, node_vector4);

	std::string vec = std::format("{:.3f}, {:.3f}, {:.3f}, {:.3f}", value.x, value.y, value.z, value.w);
	std::string toDraw = std::format("=  ({})", vec);
	ImGui::SetCursorPos(ImVec2(180 + xPad + 30, 0));
	ImGui::Text(toDraw.c_str());

	copyPopup(i, vec, "vec4");
}

void uClass::drawVector3(int i, Vector3& value) {
	auto& node = nodes[i];

	int xPad = drawVariableName(i, node_vector3);

	std::string vec = std::format("{:.3f}, {:.3f}, {:.3f}", value.x, value.y, value.z);
	std::string toDraw = std::format("=  ({})", vec);
	ImGui::SetCursorPos(ImVec2(180 + xPad + 30, 0));
	ImGui::Text(toDraw.c_str());

	copyPopup(i, vec, "vec3");
}

void uClass::drawVector2(int i, Vector2& value) {
	auto& node = nodes[i];

	int xPad = drawVariableName(i, node_vector2);

	std::string vec = std::format("{:.3f}, {:.3f}", value.x, value.y);
	std::string toDraw = std::format("=  ({})", vec);
	ImGui::SetCursorPos(ImVec2(180 + xPad + 30, 0));
	ImGui::Text(toDraw.c_str());

	copyPopup(i, vec, "vec2");
}

void uClass::drawFloatVar(int i, float value) {
	auto& node = nodes[i];

	int xPad = drawVariableName(i, node_float);

	ImGui::SetCursorPos(ImVec2(180 + xPad + 30, 0));
	ImGui::Text("=  %.3f", value);

	copyPopup(i, std::to_string(value), "float");
}

void uClass::drawDoubleVar(int i, double value) {
	auto& node = nodes[i];

	int xPad = drawVariableName(i, node_double);

	std::string toDraw = std::format("=  {:.6f}", value);
	ImGui::SetCursorPos(ImVec2(180 + xPad + 30, 0));
	ImGui::Text(toDraw.c_str());

	copyPopup(i, std::to_string(value), "double");
}

void uClass::drawInteger(int i, int64_t value, nodeType type) {
	auto& node = nodes[i];
	
	int xPad = drawVariableName(i, type);

	ImGui::SetCursorPos(ImVec2(180 + xPad + 30, 0));
	ImGui::Text("=  %lld", value);

	copyPopup(i, std::to_string(value), "int");
}

void uClass::drawUInteger(int i, uint64_t value, nodeType type) {
	auto& node = nodes[i];

	int xPad = drawVariableName(i, type);

	ImGui::SetCursorPos(ImVec2(180 + xPad + 30, 0));
	ImGui::Text("=  %llu", value);

	copyPopup(i, std::to_string(value), "uint");
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
	auto typeSize = nodeData[newType].size;

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

	ImGui::SetCursorPos(ImVec2(455 + pad, 0));
	ImGui::PushStyleColor(ImGuiCol_Text, color.Value);
	ImGui::Text(toDraw.c_str());
	ImGui::PopStyleColor();
	copyPopup(i, numText, "hex");

	if (isPointer) {
		ImGui::SetItemKeyOwner(ImGuiKey_MouseWheelY);
		if (ImGui::IsItemHovered()) {
			g_HoveringPointer = true;
		}

		if (ImGui::BeginItemTooltip()) {
			ImGui::BeginChild("MemPreview_Child", ImVec2(0, 0), ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);

			float mWheel = ImGui::GetIO().MouseWheel;
			if (mWheel > 0) {
				g_PreviewClass.resize(-8);
			}
			else if (mWheel < 0) {
				g_PreviewClass.resize(8);
			}

			g_PreviewClass.address = num;
			g_PreviewClass.drawNodes();

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

	if (isString) {
		std::string stringDraw = std::format("'{}'", (char*)buf.data);
		ImGui::SetCursorPos(ImVec2(455 + pad + textSize.x + 15, 0));
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

	ImGui::SetCursorPos(ImVec2(455, 0));
	ImGui::PushStyleColor(ImGuiCol_Text, ImColor(163, 255, 240).Value);
	ImGui::Text(toDraw.c_str());
	ImGui::PopStyleColor();

	copyPopup(i, toDraw, "dbl");
}

void uClass::drawFloat(int i, float num, int* pad) {
	std::string toDraw = std::format("{:.3f}", num);
	if (toDraw.size() > 20) {
		toDraw = "#####";
	}

	if (pad) {
		*pad += ImGui::CalcTextSize(toDraw.c_str()).x;
	}

	ImGui::SetCursorPos(ImVec2(455, 0));
	ImGui::PushStyleColor(ImGuiCol_Text, ImColor(163, 255, 240).Value);
	ImGui::Text(toDraw.c_str());
	ImGui::PopStyleColor();

	copyPopup(i, toDraw, "flt");
}

void uClass::drawNumber(int i, int64_t num, int* pad) {
	if (*pad > 0) {
		*pad += 15;
	}

	std::string toDraw = std::to_string(num);

	ImGui::SetCursorPos(ImVec2(455 + *pad, 0));
	ImGui::PushStyleColor(ImGuiCol_Text, ImColor(255, 218, 133).Value);
	ImGui::Text(toDraw.c_str());
	ImGui::PopStyleColor();

	copyPopup(i, toDraw, "num");

	*pad += ImGui::CalcTextSize(toDraw.c_str()).x;
}

void uClass::drawOffset(int i, int pos) {
	ImGui::SetCursorPos(ImVec2(0, 0));
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(.9f, .9f, .9f, 1.f));
	ImGui::Text(ui::toHexString(pos, 4).c_str());
	ImGui::PopStyleColor();
}

void uClass::drawAddress(int i, int pos) {
	ImGui::SetCursorPos(ImVec2(50, 0));
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(.7f, .7f, .7f, 1.f));
	ImGui::Text(ui::toHexString(this->address + pos, 16).c_str());
	ImGui::PopStyleColor();
}

void uClass::drawBytes(int i, BYTE* data, int pos, int size) {
	for (int j = 0; j < size; j++) {
		BYTE byte = data[pos];
		ImGui::SetCursorPos(ImVec2(285 + j * 20, 0));
		ImGui::Text(ui::toHexString(byte, 2).c_str());
		pos++;
	}
}

void uClass::drawStringBytes(int i, BYTE* data, int pos, int size) {
	for (int j = 0; j < size; j++) {
		BYTE byte = data[pos];
		ImGui::SetCursorPos(ImVec2(180 + j * 12, 0));
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

	ImVec2 parentSize = ImGui::GetContentRegionAvail();
	float h = ImGui::GetCursorPosY() - 2;

	ImGui::SetCursorPos(ImVec2(0, 0));
	if (ImGui::Selectable(("##Controller_" + std::to_string(i) + std::to_string(h)).c_str(), node.selected, 0, ImVec2(parentSize.x, h))) {
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

	std::string sID = "##NodePopup_" + std::to_string(i);
	if (!g_InPopup && ImGui::IsItemHovered() && ImGui::IsMouseClicked(1)) {
		ImGui::OpenPopup(sID.c_str());
	}

	if (ImGui::BeginPopup(sID.c_str())) {
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

			ImGui::Separator();

			if (ImGui::Selectable("UInt64")) {
				changeType(node_uint64);
			}
			if (ImGui::Selectable("UInt32")) {
				changeType(node_uint32);
			}
			if (ImGui::Selectable("UInt16")) {
				changeType(node_uint16);
			}
			if (ImGui::Selectable("UInt8")) {
				changeType(node_uint8);
			}

			ImGui::Separator();

			if (ImGui::Selectable("Float")) {
				changeType(node_float);
			}
			if (ImGui::Selectable("Double")) {
				changeType(node_double);
			}
			if (ImGui::Selectable("Bool")) {
				changeType(node_bool);
			}

			ImGui::Separator();

			if (ImGui::Selectable("Vector4")) {
				changeType(node_vector4);
			}
			if (ImGui::Selectable("Vector3")) {
				changeType(node_vector3);
			}
			if (ImGui::Selectable("Vector2")) {
				changeType(node_vector2);
			}

			ImGui::Separator();

			if (ImGui::Selectable("Matrix4x4")) {
				changeType(node_matrix4x4);
			}
			if (ImGui::Selectable("Matrix3x4")) {
				changeType(node_matrix3x4);
			}
			if (ImGui::Selectable("Matrix3x3")) {
				changeType(node_matrix3x3);
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

	ImVec2 parentSize = ImGui::GetContentRegionAvail();

	ImGuiListClipper clipper;
	clipper.Begin(nodes.size());
	while (clipper.Step()) {
		size_t counter = 0;
		for (int i = 0; i < clipper.DisplayStart; i++) {
			auto& node = nodes[i];
			counter += node.size;
		}

		for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
			auto& node = nodes[i];

			ImVec2 startPos = ImGui::GetCursorPos();
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
			ImGui::BeginChild(("Node_" + std::to_string(i)).c_str(), ImVec2((this == &g_PreviewClass) ? 1100 : parentSize.x, 0), ImGuiChildFlags_AutoResizeY, ImGuiWindowFlags_AlwaysUseWindowPadding);
			ImGui::PopStyleVar();

			drawOffset(i, counter);
			drawAddress(i, counter);

			size_t lCounter = 0;
			uintptr_t num = 0;
			float floatNum;
			double doubleNum;
			int pad = 0;

			// this kinda sucks
			uintptr_t dataPos = (uintptr_t)data + counter;
			switch (node.type) {
			case node_hex8:
				drawStringBytes(i, data, counter, 1);
				drawBytes(i, data, counter, 1);

				num = *(int8_t*)dataPos;
				drawNumber(i, num, &pad);
				drawHexNumber(i, num, pad);
				break;
			case node_hex16:
				drawStringBytes(i, data, counter, 2);
				drawBytes(i, data, counter, 2);

				num = *(int16_t*)dataPos;
				drawNumber(i, num, &pad);
				drawHexNumber(i, num, pad);
				break;
			case node_hex32:
				drawStringBytes(i, data, counter, 4);
				drawBytes(i, data, counter, 4);

				floatNum = *(float*)dataPos;
				drawFloat(i, floatNum, &pad);

				num = *(int32_t*)dataPos;
				drawNumber(i, num, &pad);
				drawHexNumber(i, num, pad);
				break;
			case node_hex64:
				drawStringBytes(i, data, counter, 8);
				drawBytes(i, data, counter, 8);

				doubleNum = *(double*)dataPos;
				drawDouble(i, doubleNum, &pad);

				num = *(int64_t*)dataPos;
				drawNumber(i, num, &pad);
				drawHexNumber(i, num, pad);
				break;
			case node_int64:
				drawInteger(i, *(int64_t*)dataPos, node_int64);
				break;
			case node_int32:
				drawInteger(i, *(int32_t*)dataPos, node_int32);
				break;
			case node_int16:
				drawInteger(i, *(int16_t*)dataPos, node_int16);
				break;
			case node_int8:
				drawInteger(i, *(int8_t*)dataPos, node_int8);
				break;
			case node_uint64:
				drawUInteger(i, *(uint64_t*)dataPos, node_uint64);
				break;
			case node_uint32:
				drawUInteger(i, *(uint32_t*)dataPos, node_uint32);
				break;
			case node_uint16:
				drawUInteger(i, *(uint16_t*)dataPos, node_uint16);
				break;
			case node_uint8:
				drawUInteger(i, *(uint8_t*)dataPos, node_uint8);
				break;
			case node_float:
				drawFloatVar(i, *(float*)dataPos);
				break;
			case node_double:
				drawDoubleVar(i, *(double*)dataPos);
				break;
			case node_vector4:
				drawVector4(i, *(Vector4*)dataPos);
				break;
			case node_vector3:
				drawVector3(i, *(Vector3*)dataPos);
				break;
			case node_vector2:
				drawVector2(i, *(Vector2*)dataPos);
				break;
			case node_matrix4x4:
				drawMatrix4x4(i, *(Matrix4x4*)dataPos);
				break;
			case node_matrix3x4:
				drawMatrix3x4(i, *(Matrix3x4*)dataPos);
				break;
			case node_matrix3x3:
				drawMatrix3x3(i, *(Matrix3x3*)dataPos);
				break;
			case node_bool:
				drawBool(i, *(bool*)dataPos);
				break;
			}

			drawControllers(i, counter);

			ImGui::EndChild();

			ImVec2 endPos = ImGui::GetCursorPos();

			if (node.type < 0 || node.type >= node_max) {
				continue;
			}

			counter += node.size;
		}
	}

	static bool oHoveringPointer = false;
	if (oHoveringPointer && !g_HoveringPointer) {
		free(g_PreviewClass.data);
		g_PreviewClass = uClass(15, false);
	}
	oHoveringPointer = g_HoveringPointer;
}

std::vector<uClass> g_Classes = { uClass(50) };

void initClasses(bool isX32) {
	if (g_Classes.empty() || isX32 != mem::x32) {
		mem::x32 = isX32;
		g_Classes = { uClass(50) };
	}
}