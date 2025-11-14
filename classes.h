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

inline nodeTypeInfo nodeData[] = {
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

inline bool g_HoveringPointer = false;
inline bool g_InPopup = false;
inline size_t g_SelectedClass = 0;

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

inline int g_nameCounter = 0;

class uClass {
public:
	char name[64];
	char addressInput[256];
	std::vector<nodeBase> nodes;
	uintptr_t address = 0;
	int varCounter = 0;
	size_t size;
	BYTE* data = 0;
	float cur_pad = 0;

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

		std::string newName = "Class_" + std::to_string(g_nameCounter);
		memcpy(name, newName.data(), newName.size());

		data = (BYTE*)malloc(size);

		if (data) {
			memset(data, 0, size);
		}
		else {
			MessageBoxA(0, "Failed to allocate memory!", "ERROR", MB_ICONERROR);
		}

		if (incrementCounter) {
			g_nameCounter++;
		}
	}

	void sizeToNodes();
	void resize(int size);
	void drawNodes();
	void drawStringBytes(int i, const BYTE* data, int pos, int size);
	void drawOffset(int i, int pos);
	void drawAddress(int i, int pos) const;
	void drawBytes(int i, BYTE* data, int pos, int size);
	void drawNumber(int i, int64_t num);
	void drawFloat(int i, float num);
	void drawDouble(int i, double num);
	void drawHexNumber(int i, uintptr_t num, uintptr_t* ptrOut = 0);
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
	template<typename MatrixType>
	void drawMatrixText(float xPadding, int rows, int columns, const MatrixType& value);
	void drawMatrix4x4(int i, const Matrix4x4& value);
	void drawMatrix3x4(int i, const Matrix3x4& value);
	void drawMatrix3x3(int i, const Matrix3x3& value);
	void drawBool(int i, bool value);

	std::string exportClass();
};

inline uClass g_PreviewClass(15);
inline std::vector<uClass> g_Classes = { uClass(50) };

inline std::string uClass::exportClass() {
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

inline void uClass::sizeToNodes() {
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

inline void uClass::resize(int mod) {
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
				nodes.emplace_back(nodeBase(nullptr, node_hex64, false));
			}
			else if (remaining >= 4) {
				remaining = remaining - 4;
				nodes.emplace_back(nodeBase(nullptr, node_hex32, false));
			}
			else if (remaining >= 2) {
				remaining = remaining - 2;
				nodes.emplace_back(nodeBase(nullptr, node_hex16, false));
			}
			else if (remaining >= 1) {
				remaining = remaining - 1;
				nodes.emplace_back(nodeBase(nullptr, node_hex8, false));
			}
		}
		sizeToNodes();
		memset(data, 0, size);
	}
}

inline void uClass::copyPopup(int i, std::string toCopy, std::string id) {
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


// TODO: cleanup the magic number hell below this point
inline int uClass::drawVariableName(int i, nodeType type) {
	auto& node = nodes[i];
	ImVec2 nameSize = ImGui::CalcTextSize(node.name);
	
	auto& lData = nodeData[type];
	auto typeName = lData.name;
	ImVec2 typenameSize = ImGui::CalcTextSize(typeName);

	ImGui::PushStyleColor(ImGuiCol_Text, lData.color.Value);
	ImGui::SetCursorPos(ImVec2(180, 0));
	ImGui::Text("%s", typeName);
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

inline void uClass::drawBool(int i, bool value) {

	float xPad = static_cast<float>(drawVariableName(i, node_bool));

	ImGui::SetCursorPos(ImVec2(180 + xPad + 30, 0));
	ImGui::Text(value ? "=  true" : "=  false");
}

template<typename MatrixType>
inline void uClass::drawMatrixText(float xPadding, int rows, int columns, const MatrixType& value)
{
	// for matrices, use the largest rendered number to decide the sizing for all
	// it may not look perfect in all scenarios, but it is the most logical solution for most use cases

	float mPad = 0;
	float maxWidth = 0.f;
	float y = 0;

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < columns; j++) {
			float val = value.m[i][j];
			float currentWidth = ImGui::CalcTextSize(std::format("{:.3f}", val).c_str()).x;
			maxWidth = max(currentWidth, maxWidth);
		}
	}

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < columns; j++) {
			float val = value.m[i][j];
			ImGui::SetCursorPos(ImVec2(180 + xPadding + 30 + mPad, y));
			ImGui::Text("%.3f", val);
			mPad += maxWidth;
		}

		y += 15;
		mPad = 0;
	}
}


inline void uClass::drawMatrix4x4(int i, const Matrix4x4& value) {

	float xPad = static_cast<float>(drawVariableName(i, node_matrix3x4));
	drawMatrixText(xPad, 4, 4, value);
}

inline void uClass::drawMatrix3x4(int i, const Matrix3x4& value) {

	float xPad = static_cast<float>(drawVariableName(i, node_matrix3x4));
	drawMatrixText(xPad, 3, 4, value);
}

inline void uClass::drawMatrix3x3(int i, const Matrix3x3& value) {

	float xPad = static_cast<float>(drawVariableName(i, node_matrix3x3));
	drawMatrixText(xPad, 3, 3, value);
}

inline void uClass::drawVector4(int i, Vector4& value) {

	const float xPad = static_cast<float>(drawVariableName(i, node_vector4));

	std::string vec = std::format("{:.3f}, {:.3f}, {:.3f}, {:.3f}", value.x, value.y, value.z, value.w);
	std::string toDraw = std::format("=  ({})", vec);
	ImGui::SetCursorPos(ImVec2(180.f + xPad + 30.f, 0));
	ImGui::Text("%s", toDraw.c_str());

	copyPopup(i, vec, "vec4");
}

inline void uClass::drawVector3(int i, Vector3& value) {

	const float xPad = static_cast<float>(drawVariableName(i, node_vector3));

	std::string vec = std::format("{:.3f}, {:.3f}, {:.3f}", value.x, value.y, value.z);
	std::string toDraw = std::format("=  ({})", vec);
	ImGui::SetCursorPos(ImVec2(180 + xPad + 30, 0));
	ImGui::Text("%s", toDraw.c_str());

	copyPopup(i, vec, "vec3");
}

inline void uClass::drawVector2(int i, Vector2& value) {

	const float xPad = static_cast<float>(drawVariableName(i, node_vector2));

	std::string vec = std::format("{:.3f}, {:.3f}", value.x, value.y);
	std::string toDraw = std::format("=  ({})", vec);
	ImGui::SetCursorPos(ImVec2(180 + xPad + 30, 0));
	ImGui::Text("%s", toDraw.c_str());

	copyPopup(i, vec, "vec2");
}

inline void uClass::drawFloatVar(int i, float value) {

	const float xPad = static_cast<float>(drawVariableName(i, node_float));

	ImGui::SetCursorPos(ImVec2(180.f + xPad + 30, 0));
	ImGui::Text("=  %.3f", value);

	copyPopup(i, std::to_string(value), "float");
}

inline void uClass::drawDoubleVar(int i, double value) {

	const float xPad = static_cast<float>(drawVariableName(i, node_double));

	std::string toDraw = std::format("=  {:.6f}", value);
	ImGui::SetCursorPos(ImVec2(180.f + xPad + 30, 0));
	ImGui::Text("%s", toDraw.c_str());

	copyPopup(i, std::to_string(value), "double");
}

inline void uClass::drawInteger(int i, int64_t value, nodeType type) {

	const float xPad = static_cast<float>(drawVariableName(i, type));

	ImGui::SetCursorPos(ImVec2(180.f + xPad + 30.f, 0));
	ImGui::Text("=  %lld", value);

	copyPopup(i, std::to_string(value), "int");
}

inline void uClass::drawUInteger(int i, uint64_t value, nodeType type) {

	const float xPad = static_cast<float>(drawVariableName(i, type));

	ImGui::SetCursorPos(ImVec2(180.f + xPad + 30.f, 0));
	ImGui::Text("=  %llu", value);

	copyPopup(i, std::to_string(value), "uint");
}

inline void uClass::changeType(nodeType newType) {
	for (int i = 0; i < static_cast<int>(nodes.size()); i++) {
		auto& node = nodes[i];
		if (node.selected) {
			int newNodes;
			changeType(i, newType, true, &newNodes);
			i += newNodes;
		}
	}
}

inline void uClass::changeType(int i, nodeType newType, bool selectNew, int* newNodes) {
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
			nodes.insert(nodes.begin() + i + inserted++, { nullptr, node_hex32, selectNew });
		}
		else if (sizeDiff >= 2) {
			sizeDiff = sizeDiff - 2;
			nodes.insert(nodes.begin() + i + inserted++, { nullptr, node_hex16, selectNew });
		}
		else if (sizeDiff >= 1) {
			sizeDiff = sizeDiff - 1;
			nodes.insert(nodes.begin() + i + inserted++, { nullptr, node_hex8, selectNew });
		}
	}

	sizeToNodes();

	if (newNodes) {
		*newNodes = inserted - 1;
	}
}

inline void uClass::drawHexNumber(int i, uintptr_t num, uintptr_t* ptrOut) {
	cur_pad += 15;

	ImColor color = ImColor(255, 162, 0);

	std::string numText = ui::toHexString(num, 0);

	std::string targetAddress = ("0x" + numText);
	std::string toDraw;

	pointerInfo info;
	bool isPointer = mem::isPointer(num, &info);
	if (isPointer) {
		color = ImColor(255, 0, 0);

		if (info.moduleName == "") {
			toDraw = "[heap] " + targetAddress;
		}
		else {
			auto exportIt = mem::g_ExportMap.find(num);
			if (exportIt != mem::g_ExportMap.end()) {
				color = ImColor(0, 255, 0);
				toDraw = "[EXPORT] " + exportIt->second + " " + targetAddress;
			}
			else {
				toDraw = std::format("[{}] {} {}", info.section, info.moduleName, targetAddress);
			}
		}

		std::string rttiNames;
		if (mem::rttiInfo(num, rttiNames)) {
			toDraw += rttiNames;
		}
	}

	ImVec2 textSize = ImGui::CalcTextSize(toDraw.c_str());

	ImGui::SetCursorPos(ImVec2(455 + cur_pad, 0));
	ImGui::PushStyleColor(ImGuiCol_Text, color.Value);
	ImGui::Text("%s", toDraw.c_str());
	ImGui::PopStyleColor();

	if (isPointer) {
		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
			bool found = false;
			for (size_t j = 0; j < g_Classes.size(); j++) {
				auto& lClass = g_Classes[j];
				if (lClass.address == num) {
					g_SelectedClass = j;
					found = true;
					break;
				}
			}

			if (!found) {
				if (ptrOut) {
					*ptrOut = num;
				}
			}
		}
	}

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
	for (int it = 0; it < 4; it++) {
		if (buf.data[it] < 21 || buf.data[it] > 126) {
			isString = false;
			break;
		}
	}

	if (isString) {
		std::string stringDraw = std::format("'{}'", (char*)buf.data);
		ImGui::SetCursorPos(ImVec2(455.f + cur_pad + textSize.x + 15, 0));
		ImGui::PushStyleColor(ImGuiCol_Text, ImColor(3, 252, 140).Value);
		ImGui::Text("%s", stringDraw.c_str());
		ImGui::PopStyleColor();
	}
}

inline void uClass::drawDouble(int i, double num) {
	std::string toDraw = std::format("{:.3f}", num);
	if (toDraw.size() > 20) {
		toDraw = "#####";
	}

	cur_pad += ImGui::CalcTextSize(toDraw.c_str()).x;

	ImGui::SetCursorPos(ImVec2(455, 0));
	ImGui::PushStyleColor(ImGuiCol_Text, ImColor(163, 255, 240).Value);
	ImGui::Text("%s", toDraw.c_str());
	ImGui::PopStyleColor();

	copyPopup(i, toDraw, "dbl");
}

inline void uClass::drawFloat(int i, float num) {
	std::string toDraw = std::format("{:.3f}", num);
	if (toDraw.size() > 20) {
		toDraw = "#####";
	}

	cur_pad += ImGui::CalcTextSize(toDraw.c_str()).x;

	ImGui::SetCursorPos(ImVec2(455, 0));
	ImGui::PushStyleColor(ImGuiCol_Text, ImColor(163, 255, 240).Value);
	ImGui::Text("%s", toDraw.c_str());
	ImGui::PopStyleColor();

	copyPopup(i, toDraw, "flt");
}

inline void uClass::drawNumber(int i, int64_t num) {
	if (cur_pad > 0) {
		cur_pad += 15;
	}

	std::string toDraw = std::to_string(num);

	ImGui::SetCursorPos(ImVec2(455 + cur_pad, 0));
	ImGui::PushStyleColor(ImGuiCol_Text, ImColor(255, 218, 133).Value);
	ImGui::Text("%s", toDraw.c_str());
	ImGui::PopStyleColor();

	copyPopup(i, toDraw, "num");

	cur_pad += ImGui::CalcTextSize(toDraw.c_str()).x;
}

inline void uClass::drawOffset(int i, int pos) {
	ImGui::SetCursorPos(ImVec2(0, 0));
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(.9f, .9f, .9f, 1.f));
	ImGui::Text(ui::toHexString(pos, 4).c_str());
	ImGui::PopStyleColor();
}

inline void uClass::drawAddress(int i, int pos) const {
	ImGui::SetCursorPos(ImVec2(50, 0));
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(.7f, .7f, .7f, 1.f));
	ImGui::Text("%s", ui::toHexString(this->address + pos, 16).c_str());
	ImGui::PopStyleColor();
}

inline void uClass::drawBytes(int i, BYTE* data, int pos, int size) {
	for (int j = 0; j < size; j++) {
		BYTE byte = data[pos];
		ImGui::SetCursorPos(ImVec2(285 + j * 20, 0));
		ImGui::Text("%s", ui::toHexString(byte, 2).c_str());
		pos++;
	}
}

inline void uClass::drawStringBytes(int i, const BYTE* lData, int pos, int lSize) {
	for (int j = 0; j < lSize; j++) {
		BYTE byte = lData[pos];
		ImGui::SetCursorPos(ImVec2(180.f + static_cast<float>(j) * 12.f, 0));
		if (byte > 32 && byte < 127) {
			ImGui::Text("%c", byte);
		}
		else if (byte != 0) {
			ImGui::Text(".");
		}
		else {
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(.3f, .3f, .3f, 1.f));
			ImGui::Text(".");
			ImGui::PopStyleColor();
		}
		pos++;
	}
}

inline void uClass::drawControllers(int i, int counter) {
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
			for (size_t j = 0; j < nodes.size(); j++) {
				if (nodes[j].selected) {
					min = min(min, static_cast<int>(j));
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
			for (auto& lNode : nodes) {
				lNode.selected = false;
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
			for (auto& lNode : nodes) {
				lNode.selected = false;
			}
			nodes[i].selected = true;
		}

		if (ImGui::BeginMenu("Change Type")) {
			// the ImVec2 just ensures the width of this menu
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
			for (int j = static_cast<int>(nodes.size()) - 1; j >= 0; j--) {
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

inline void uClass::drawNodes() {
	mem::read(this->address, this->data, this->size);

	ImVec2 parentSize = ImGui::GetContentRegionAvail();

	uintptr_t clickedPointer = 0;

	ImGuiListClipper clipper;
	clipper.Begin( static_cast<int>(nodes.size()) );
	while (clipper.Step()) {
		int counter = 0;
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
			cur_pad = 0;

			// this kinda sucks
			auto dataPos = reinterpret_cast<std::uint8_t*>(data) + counter;

			switch (node.type) {
			case node_hex8:
			{
				drawStringBytes(i, data, counter, 1);
				drawBytes(i, data, counter, 1);

				auto num = *reinterpret_cast<int8_t*>(dataPos);
				drawNumber(i, num);
				drawHexNumber(i, num);
				break;
			}
			case node_hex16:
			{
				drawStringBytes(i, data, counter, 2);
				drawBytes(i, data, counter, 2);

				auto num = *reinterpret_cast<int16_t*>(dataPos);
				drawNumber(i, num);
				drawHexNumber(i, num);
				break;
			}
			case node_hex32:
			{
				drawStringBytes(i, data, counter, 4);
				drawBytes(i, data, counter, 4);

				auto fNum = *reinterpret_cast<float*>(dataPos);
				drawFloat(i, fNum);

				auto num = *reinterpret_cast<int32_t*>(dataPos);
				drawNumber(i, num);
				drawHexNumber(i, num, &clickedPointer);
				break;
			}
			case node_hex64:
			{
				drawStringBytes(i, data, counter, 8);
				drawBytes(i, data, counter, 8);

				auto dNum = *reinterpret_cast<double*>(dataPos);
				drawDouble(i, dNum);

				auto num = *reinterpret_cast<int64_t*>(dataPos);
				drawNumber(i, num);
				drawHexNumber(i, num, &clickedPointer);
				break;
			}
			case node_int64:
				drawInteger(i, *reinterpret_cast<int64_t*>(dataPos), node_int64);
				break;
			case node_int32:
				drawInteger(i, *reinterpret_cast<int32_t*>(dataPos), node_int32);
				break;
			case node_int16:
				drawInteger(i, *reinterpret_cast<int16_t*>(dataPos), node_int16);
				break;
			case node_int8:
				drawInteger(i, *reinterpret_cast<int8_t*>(dataPos), node_int8);
				break;
			case node_uint64:
				drawUInteger(i, *reinterpret_cast<uint64_t*>(dataPos), node_uint64);
				break;
			case node_uint32:
				drawUInteger(i, *reinterpret_cast<uint32_t*>(dataPos), node_uint32);
				break;
			case node_uint16:
				drawUInteger(i, *reinterpret_cast<uint16_t*>(dataPos), node_uint16);
				break;
			case node_uint8:
				drawUInteger(i, *reinterpret_cast<uint8_t*>(dataPos), node_uint8);
				break;
			case node_float:
				drawFloatVar(i, *reinterpret_cast<float*>(dataPos));
				break;
			case node_double:
				drawDoubleVar(i, *reinterpret_cast<double*>(dataPos));
				break;
			case node_vector4:
				drawVector4(i, *reinterpret_cast<Vector4*>(dataPos));
				break;
			case node_vector3:
				drawVector3(i, *reinterpret_cast<Vector3*>(dataPos));
				break;
			case node_vector2:
				drawVector2(i, *reinterpret_cast<Vector2*>(dataPos));
				break;
			case node_matrix4x4:
				drawMatrix4x4(i, *reinterpret_cast<Matrix4x4*>(dataPos));
				break;
			case node_matrix3x4:
				drawMatrix3x4(i, *reinterpret_cast<Matrix3x4*>(dataPos));
				break;
			case node_matrix3x3:
				drawMatrix3x3(i, *reinterpret_cast<Matrix3x3*>(dataPos));
				break;
			case node_bool:
				drawBool(i, *reinterpret_cast<bool*>(dataPos));
				break;
			}

			drawControllers(i, counter);

			ImGui::EndChild();

			if (node.type >= node_max) {
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

	if (clickedPointer) {
		uClass newClass(50);
		newClass.address = clickedPointer;
		g_Classes.push_back(newClass);
		g_SelectedClass = g_Classes.size() - 1;
	}
}

inline void initClasses(bool isX32) {
	if (g_Classes.empty() || isX32 != mem::x32) {
		mem::x32 = isX32;
		g_Classes = { uClass(50) };
	}
}