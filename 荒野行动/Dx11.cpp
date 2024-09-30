#include "dx11.h"
#include "detours.h"
#include <algorithm>

const char* decryptedText1 = VMProtectDecryptStringA("Free Version - %s %s");
const char* decryptedText2 = VMProtectDecryptStringA(u8"软件完全公益免费！！！");
const char* decryptedTitle = VMProtectDecryptStringA(u8"Motna --- v1.4 --- 公益软件不曾收费!");
const char* decryptedLinkText = VMProtectDecryptStringA(u8"点击订阅我们的TG频道(#^.^#)");
const char* decryptedLinkURL = VMProtectDecryptStringA("https://t.me/Motna_wuzha");
char formattedText1[256];
static bool needsUpdate = true;
uint32_t initialCRC_decryptedText2;
uint32_t initialCRC_decryptedTitle;
uint32_t initialCRC_decryptedLinkText;
uint32_t initialCRC_decryptedLinkURL;
uint32_t crc32(const void* data, size_t length) {
	static uint32_t table[256];
	static bool have_table = false;

	if (!have_table) {
		for (uint32_t i = 0; i < 256; ++i) {
			uint32_t crc = i;
			for (uint32_t j = 0; j < 8; ++j) {
				if (crc & 1)
					crc = (crc >> 1) ^ 0xEDB88320;
				else
					crc = crc >> 1;
			}
			table[i] = crc;
		}
		have_table = true;
	}

	uint32_t crc = 0xFFFFFFFF;
	const uint8_t* current = reinterpret_cast<const uint8_t*>(data);
	while (length--)
		crc = (crc >> 8) ^ table[(crc & 0xFF) ^ *current++];
	return ~crc;
}
void InitializeCRCValues() {
	
	initialCRC_decryptedText2 = crc32(decryptedText2, strlen(decryptedText2));
	initialCRC_decryptedTitle = crc32(decryptedTitle, strlen(decryptedTitle));
	initialCRC_decryptedLinkText = crc32(decryptedLinkText, strlen(decryptedLinkText));
	initialCRC_decryptedLinkURL = crc32(decryptedLinkURL, strlen(decryptedLinkURL));
	
}
void CheckCRCValues() {
	VMProtectBegin("CRC check thread");
	while (true) {
		std::this_thread::sleep_for(std::chrono::minutes(10)); // 延长检查间隔时间

		uint32_t currentCRC_decryptedText2 = crc32(decryptedText2, strlen(decryptedText2));
		uint32_t currentCRC_decryptedTitle = crc32(decryptedTitle, strlen(decryptedTitle));
		uint32_t currentCRC_decryptedLinkText = crc32(decryptedLinkText, strlen(decryptedLinkText));
		uint32_t currentCRC_decryptedLinkURL = crc32(decryptedLinkURL, strlen(decryptedLinkURL));

		if (currentCRC_decryptedText2 != initialCRC_decryptedText2 ||
			currentCRC_decryptedTitle != initialCRC_decryptedTitle ||
			currentCRC_decryptedLinkText != initialCRC_decryptedLinkText ||
			currentCRC_decryptedLinkURL != initialCRC_decryptedLinkURL) {

			// 检测到CRC值变化,立即终止进程
			ExitProcess(0);
		}
	}
	VMProtectEnd();
}


template<typename T>
T clamp(T value, T min, T max) {
	return (value < min) ? min : ((value > max) ? max : value);
}
std::string ToUpper(const std::string& str) {
	std::string upperStr = str;
	std::transform(upperStr.begin(), upperStr.end(), upperStr.begin(), ::toupper);
	return upperStr;
}


enum AimTarget {
	HEAD,
	CHEST,
	NEAREST
};
AimTarget aimTarget = HEAD;

PDWORD64 SwapChainVtb;
PPresent oPresent;
PWndProc oWndProc;

Tables::Table GoodsHead;
Tables::Table PlayerHead;

LPVOID GoodsThis;
LPVOID ClassName;

static float SMOOTH_FACTOR = 0.2f; // 平滑因子,可以根据需要调整
static float MIN_DISTANCE = 0.8f; // 最小移动距离阈值
static float MAX_SPEED = 6.0f; //最大移动速度

bool 骨骼 = false;
bool 玩家 = false;
bool 武器 = false;
bool 自瞄 = false;
bool 世界地址 = false;
bool 方框 = false;
bool 真人 = false;
bool 射线 = false;
bool 自瞄倒地 = false;
int 自瞄热键1 = 0x45; // 默认自瞄热键1为 'E'
int 自瞄热键2 = 0x51; // 默认自瞄热键2为 'Q'
bool capturingKey1 = false; // 是否正在捕捉热键1
bool capturingKey2 = false; // 是否正在捕捉热键2
bool  车辆 = false;
bool  空投 = false;
static float 范围 = 100.0f;

Vector2 AimCoor;

DWORD64 currentTargetId = 0;
float targetLockTime = 0.0f;
static float TARGET_LOCK_DURATION = 1.0f; // 1秒的锁定时间,可以根据需要调整

const float EPSILON = 0.0001f;//计算误差
bool FloatEqual(float a, float b, float epsilon = EPSILON) {
	return std::fabs(a - b) < epsilon;
}


void SaveConfig()
{
	char configPath[MAX_PATH];
	GetModuleFileNameA(NULL, configPath, MAX_PATH);
	strcpy(strrchr(configPath, '\\'), "\\config.ini");

	char buffer[32];

	WritePrivateProfileStringA("Settings", "骨骼", 骨骼 ? "1" : "0", configPath);
	WritePrivateProfileStringA("Settings", "玩家", 玩家 ? "1" : "0", configPath);
	WritePrivateProfileStringA("Settings", "自瞄", 自瞄 ? "1" : "0", configPath);
	WritePrivateProfileStringA("Settings", "世界地址", 世界地址 ? "1" : "0", configPath);
	WritePrivateProfileStringA("Settings", "方框", 方框 ? "1" : "0", configPath);
	WritePrivateProfileStringA("Settings", "射线", 射线 ? "1" : "0", configPath);
	WritePrivateProfileStringA("Settings", "自瞄倒地", 自瞄倒地 ? "1" : "0", configPath);
	WritePrivateProfileStringA("Settings", "车辆", 车辆 ? "1" : "0", configPath);
	WritePrivateProfileStringA("Settings", "空投", 空投 ? "1" : "0", configPath);
	WritePrivateProfileStringA("Settings", "真人", 真人 ? "1" : "0", configPath);
	WritePrivateProfileStringA("Settings", "武器", 武器 ? "1" : "0", configPath);

	sprintf(buffer, "%d", 自瞄热键1);
	WritePrivateProfileStringA("Settings", "自瞄热键1", buffer, configPath);

	sprintf(buffer, "%d", 自瞄热键2);
	WritePrivateProfileStringA("Settings", "自瞄热键2", buffer, configPath);

	sprintf(buffer, "%d", aimTarget);
	WritePrivateProfileStringA("Settings", "自瞄部位", buffer, configPath);

	sprintf(buffer, "%.2f", SMOOTH_FACTOR);
	WritePrivateProfileStringA("Settings", "SMOOTH_FACTOR", buffer, configPath);

	sprintf(buffer, "%.2f", MIN_DISTANCE);
	WritePrivateProfileStringA("Settings", "MIN_DISTANCE", buffer, configPath);

	sprintf(buffer, "%.2f", MAX_SPEED);
	WritePrivateProfileStringA("Settings", "MAX_SPEED", buffer, configPath);

	sprintf(buffer, "%.2f", TARGET_LOCK_DURATION);
	WritePrivateProfileStringA("Settings", "TARGET_LOCK_DURATION", buffer, configPath);

	sprintf(buffer, "%.2f", 范围);
	WritePrivateProfileStringA("Settings", "范围", buffer, configPath);
}
void LoadConfig()
{
	char configPath[MAX_PATH];
	GetModuleFileNameA(NULL, configPath, MAX_PATH);
	strcpy(strrchr(configPath, '\\'), "\\config.ini");

	char buffer[32];

	骨骼 = GetPrivateProfileIntA("Settings", "骨骼", 骨骼, configPath) != 0;
	玩家 = GetPrivateProfileIntA("Settings", "玩家", 玩家, configPath) != 0;
	自瞄 = GetPrivateProfileIntA("Settings", "自瞄", 自瞄, configPath) != 0;
	世界地址 = GetPrivateProfileIntA("Settings", "世界地址", 世界地址, configPath) != 0;
	方框 = GetPrivateProfileIntA("Settings", "方框", 方框, configPath) != 0;
	射线 = GetPrivateProfileIntA("Settings", "射线", 射线, configPath) != 0;
	自瞄倒地 = GetPrivateProfileIntA("Settings", "自瞄倒地", 自瞄倒地, configPath) != 0;
	空投 = GetPrivateProfileIntA("Settings", "空投", 空投, configPath) != 0;
	车辆 = GetPrivateProfileIntA("Settings", "车辆", 车辆, configPath) != 0;
	真人 = GetPrivateProfileIntA("Settings", "真人", 真人, configPath) != 0;
	武器 = GetPrivateProfileIntA("Settings", "武器", 武器, configPath) != 0;

	GetPrivateProfileStringA("Settings", "自瞄热键1", "69", buffer, sizeof(buffer), configPath);
	自瞄热键1 = atoi(buffer);

	GetPrivateProfileStringA("Settings", "自瞄热键2", "81", buffer, sizeof(buffer), configPath);
	自瞄热键2 = atoi(buffer);

	GetPrivateProfileStringA("Settings", "自瞄部位", "0", buffer, sizeof(buffer), configPath);
	aimTarget = static_cast<AimTarget>(atoi(buffer));

	GetPrivateProfileStringA("Settings", "SMOOTH_FACTOR", "0.2", buffer, sizeof(buffer), configPath);
	SMOOTH_FACTOR = atof(buffer);

	GetPrivateProfileStringA("Settings", "MIN_DISTANCE", "0.8", buffer, sizeof(buffer), configPath);
	MIN_DISTANCE = atof(buffer);

	GetPrivateProfileStringA("Settings", "MAX_SPEED", "6.0", buffer, sizeof(buffer), configPath);
	MAX_SPEED = atof(buffer);

	GetPrivateProfileStringA("Settings", "TARGET_LOCK_DURATION", "1.0", buffer, sizeof(buffer), configPath);
	TARGET_LOCK_DURATION = atof(buffer);

	GetPrivateProfileStringA("Settings", "范围", "100.0", buffer, sizeof(buffer), configPath);
	范围 = atof(buffer);
}

void CleanupRenderTarget()
{
	if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}
void CreateRenderTarget()
{
	ID3D11Texture2D* pBackBuffer;
	g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
	pBackBuffer->Release();
}

std::unordered_map<int, const char*> keyNames = {
	{0x41, "A"}, {0x42, "B"}, {0x43, "C"}, {0x44, "D"}, {0x45, "E"}, {0x46, "F"}, {0x47, "G"},
	{0x48, "H"}, {0x49, "I"}, {0x4A, "J"}, {0x4B, "K"}, {0x4C, "L"}, {0x4D, "M"}, {0x4E, "N"},
	{0x4F, "O"}, {0x50, "P"}, {0x51, "Q"}, {0x52, "R"}, {0x53, "S"}, {0x54, "T"}, {0x55, "U"},
	{0x56, "V"}, {0x57, "W"}, {0x58, "X"}, {0x59, "Y"}, {0x5A, "Z"},
	{VK_LBUTTON, "Left Mouse"}, {VK_RBUTTON, "Right Mouse"}, {VK_MBUTTON, "Middle Mouse"},
	{VK_XBUTTON1, "X1 Mouse"}, {VK_XBUTTON2, "X2 Mouse"},
	{VK_BACK, "Backspace"}, {VK_TAB, "Tab"}, {VK_RETURN, "Enter"}, {VK_SHIFT, "Shift"},
	{VK_CONTROL, "Ctrl"}, {VK_MENU, "Alt"}, {VK_PAUSE, "Pause"}, {VK_CAPITAL, "Caps Lock"},
	{VK_ESCAPE, "Esc"}, {VK_SPACE, "Space"}, {VK_PRIOR, "Page Up"}, {VK_NEXT, "Page Down"},
	{VK_END, "End"}, {VK_HOME, "Home"}, {VK_LEFT, "Left Arrow"}, {VK_UP, "Up Arrow"},
	{VK_RIGHT, "Right Arrow"}, {VK_DOWN, "Down Arrow"}, {VK_INSERT, "Insert"}, {VK_DELETE, "Delete"},
	{VK_NUMPAD0, "Numpad 0"}, {VK_NUMPAD1, "Numpad 1"}, {VK_NUMPAD2, "Numpad 2"}, {VK_NUMPAD3, "Numpad 3"},
	{VK_NUMPAD4, "Numpad 4"}, {VK_NUMPAD5, "Numpad 5"}, {VK_NUMPAD6, "Numpad 6"}, {VK_NUMPAD7, "Numpad 7"},
	{VK_NUMPAD8, "Numpad 8"}, {VK_NUMPAD9, "Numpad 9"}, {VK_MULTIPLY, "Numpad *"}, {VK_ADD, "Numpad +"},
	{VK_SEPARATOR, "Numpad Enter"}, {VK_SUBTRACT, "Numpad -"}, {VK_DECIMAL, "Numpad ."}, {VK_DIVIDE, "Numpad /"},
	{VK_F1, "F1"}, {VK_F2, "F2"}, {VK_F3, "F3"}, {VK_F4, "F4"}, {VK_F5, "F5"}, {VK_F6, "F6"},
	{VK_F7, "F7"}, {VK_F8, "F8"}, {VK_F9, "F9"}, {VK_F10, "F10"}, {VK_F11, "F11"}, {VK_F12, "F12"},
	{VK_NUMLOCK, "Num Lock"}, {VK_SCROLL, "Scroll Lock"},
	// 其他键位可以根据需要添加
};
const char* GetKeyName(int keyCode) {
	auto it = keyNames.find(keyCode);
	return it != keyNames.end() ? it->second : "Unknown";
}


void RenderDynamicWatermark() {
	// 设置水印窗口的位置和透明度
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(0.35f); // 半透明背景

	// 开始一个无标题栏、不可调整大小、自动调整大小、不可移动且不保存设置的窗口
	if (ImGui::Begin("Watermark", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings)) {
		// 显示动态信息
		static char formattedText1[256];
		static std::string lastFormattedText1;
		static std::string lastDecryptedText2;
		

		// 检查是否需要更新文本
		if (needsUpdate) {
			snprintf(formattedText1, sizeof(formattedText1), decryptedText1, __DATE__, __TIME__);
			lastFormattedText1 = formattedText1;
			lastDecryptedText2 = decryptedText2;
			needsUpdate = false;
		}

		ImGui::Text(lastFormattedText1.c_str());
		ImGui::Text(lastDecryptedText2.c_str());
		ImGui::End();
	}
}


HRESULT STDMETHODCALLTYPE MyPresent(IDXGISwapChain* This, UINT SyncInterval, UINT Flags)
{
	CleanupRenderTarget();
	CreateRenderTarget();

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin(decryptedTitle);

	ImGui::Separator();
	ImGui::Text(u8"视觉设置");

	ImGui::Checkbox(u8"方框", &方框);
	ImGui::SameLine();
	ImGui::Checkbox(u8"骨骼", &骨骼);
	ImGui::SameLine();
	ImGui::Checkbox(u8"射线", &射线);
	ImGui::SameLine();
	ImGui::Checkbox(u8"手持", &武器);
	ImGui::SameLine();
	ImGui::Checkbox(u8"显示真人", &真人);

	ImGui::Checkbox(u8"车辆透视", &车辆);
	ImGui::SameLine();
	ImGui::Checkbox(u8"空投透视", &空投);

	ImGui::Separator();
	ImGui::Text(u8"自瞄设置");

	ImGui::Checkbox(u8"自瞄", &自瞄);
	ImGui::SameLine();
	ImGui::Checkbox(u8"是否自瞄倒地", &自瞄倒地);

	ImGui::Separator();
	ImGui::Text(u8"自瞄部位");

	ImGui::RadioButton(u8"头部", (int*)&aimTarget, HEAD);
	ImGui::SameLine();
	ImGui::RadioButton(u8"胸口", (int*)&aimTarget, CHEST);
	ImGui::SameLine();
	ImGui::RadioButton(u8"最近部位", (int*)&aimTarget, NEAREST);

	ImGui::Separator();
	ImGui::Text(u8"热键设置");

	ImGui::AlignTextToFramePadding();
	ImGui::Text(u8"当前自瞄热键1: %s", GetKeyName(自瞄热键1));
	ImGui::SameLine();
	if (capturingKey1)
	{
		ImGui::Text(u8"按下任意键以设置自瞄热键1...");
	}
	else
	{
		if (ImGui::Button(u8"设置自瞄热键1"))
		{
			capturingKey1 = true;
		}
	}

	ImGui::SameLine(0, 20); // 添加一些间距
	ImGui::AlignTextToFramePadding();
	ImGui::Text(u8"当前自瞄热键2: %s", GetKeyName(自瞄热键2));
	ImGui::SameLine();
	if (capturingKey2)
	{
		ImGui::Text(u8"按下任意键以设置自瞄热键2...");
	}
	else
	{
		if (ImGui::Button(u8"设置自瞄热键2"))
		{
			capturingKey2 = true;
		}
	}

	ImGui::Separator();
	ImGui::Text(u8"自瞄参数");

	ImGui::SliderFloat(u8"自瞄平滑", &SMOOTH_FACTOR, 0.00f, 1.00f);
	ImGui::SliderFloat(u8"最小移动距离阈值", &MIN_DISTANCE, 0.00f, 5.00f);
	ImGui::SliderFloat(u8"最大移动速度", &MAX_SPEED, 5.00f, 50.00f);
	ImGui::SliderFloat(u8"目标锁定时间", &TARGET_LOCK_DURATION, 0.2f, 2.0f);
	ImGui::SliderFloat(u8"范围", &范围, 0.0f, 1000.0f);

	ImGui::Separator();
	ImGui::Text(u8"配置管理");

	if (ImGui::Button(u8"保存配置"))
	{
		SaveConfig();
	}
	ImGui::SameLine();
	if (ImGui::Button(u8"加载配置"))
	{
		LoadConfig();
	}

	ImGui::Separator();
	RenderClickableLink(decryptedLinkText, decryptedLinkURL);

	ImGui::Separator();
	ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), u8"免责声明\n\n"
		"1. 使用风险\n"
		"   本软件为免费提供的游戏修改工具，具有透视和自瞄等功能。用户在使用本软件时，需自行承担由此产生的所有风险和责任。我们不对任何因使用本软件而导致的账号封禁、游戏数据丢失或其他任何形式的损失负责。\n\n"
		"2. 法律责任\n"
		"   本软件仅供学习和娱乐用途。用户需确保其使用行为符合所在国家或地区的法律法规。任何违反法律法规的行为，均由用户自行承担法律责任。\n\n"
		"3. 第三方责任\n"
		"   本软件与任何游戏开发商、发行商或平台无关。用户在使用本软件时，应遵守相关游戏的用户协议和条款。因违反游戏协议而导致的后果，由用户自行承担。\n\n"
		"4. 软件保证\n"
		"   本软件按“现状”提供，不提供任何形式的明示或暗示保证。我们不保证软件的功能、性能或适用性，也不保证软件的持续可用性。\n\n"
		"5. 更新和支持\n"
		"   我们不承诺对本软件进行任何更新或提供技术支持。用户在使用过程中遇到的任何问题，需自行解决。\n\n"
		"通过使用本软件，您即表示已阅读并同意上述条款。");
	ImGui::End();

	Cheat::RenderGoods();
	Cheat::RenderPlayer();

	RenderDynamicWatermark();//水印

	ImGui::Render();

	g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	return oPresent(This, SyncInterval, Flags);
}

LRESULT WINAPI MyWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	// 捕捉键盘按键
	if (capturingKey1 && msg == WM_KEYDOWN) {
		自瞄热键1 = wParam;
		capturingKey1 = false;
	}
	else if (capturingKey2 && msg == WM_KEYDOWN) {
		自瞄热键2 = wParam;
		capturingKey2 = false;
	}

	// 捕捉鼠标按键
	if (capturingKey1 && (msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_XBUTTONDOWN)) {
		if (msg == WM_LBUTTONDOWN) {
			自瞄热键1 = VK_LBUTTON;
		}
		else if (msg == WM_RBUTTONDOWN) {
			自瞄热键1 = VK_RBUTTON;
		}
		else if (msg == WM_XBUTTONDOWN) {
			自瞄热键1 = (HIWORD(wParam) == XBUTTON1) ? VK_XBUTTON1 : VK_XBUTTON2;
		}
		capturingKey1 = false;
	}
	else if (capturingKey2 && (msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_XBUTTONDOWN)) {
		if (msg == WM_LBUTTONDOWN) {
			自瞄热键2 = VK_LBUTTON;
		}
		else if (msg == WM_RBUTTONDOWN) {
			自瞄热键2 = VK_RBUTTON;
		}
		else if (msg == WM_XBUTTONDOWN) {
			自瞄热键2 = (HIWORD(wParam) == XBUTTON1) ? VK_XBUTTON1 : VK_XBUTTON2;
		}
		capturingKey2 = false;
	}

	return CallWindowProcA(oWndProc, hWnd, msg, wParam, lParam);
}

void GetSwapChain(void)
{
	//0x00
	//0x10
	//0x28
	//0x30
	//0x08
	//hyxd.exe + 5F77A40
	
	PDWORD64 Buffer = NULL;
	
	Buffer = (PDWORD64) ((DWORD64)*BaseSwapChain + 0x08);
	
	Buffer = (PDWORD64) ((DWORD64)*Buffer + 0x30);
	Buffer = (PDWORD64) ((DWORD64)*Buffer + 0x28);
	Buffer = (PDWORD64) ((DWORD64)*Buffer + 0x10);
	Buffer = (PDWORD64) ((DWORD64)*Buffer + 0x00);
	g_pSwapChain = (IDXGISwapChain*)Buffer;
	
	
#if  DEBUG_DX11 
	printf("g_pSwapChain:%p\n", g_pSwapChain);
#endif

}

void GetDx11Ptr(void) {
	if (!g_pSwapChain) return;
	HRESULT hr = g_pSwapChain->GetDevice(__uuidof(g_pd3dDevice), (LPVOID*)&g_pd3dDevice);
	if (FAILED(hr) || !g_pd3dDevice) return; // 检查设备获取是否成功

	g_pd3dDevice->GetImmediateContext(&g_pd3dDeviceContext);
	if (!g_pd3dDeviceContext) return; // 检查上下文是否获取成功

	ID3D11Texture2D* pBackBuffer = nullptr;
	hr = g_pSwapChain->GetBuffer(0, __uuidof(pBackBuffer), (LPVOID*)&pBackBuffer);
	if (FAILED(hr) || !pBackBuffer) return; // 检查缓冲区是否获取成功

	g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
	pBackBuffer->Release();

#if DEBUG_DX11
	printf("g_pd3dDevice:%p\n", g_pd3dDevice);
	printf("g_pd3dDeviceContext:%p\n", g_pd3dDeviceContext);
	printf("g_mainRenderTargetView:%p\n", g_mainRenderTargetView);

#endif

}
void InitImGui(void) {
	if (g_pd3dDevice == nullptr || g_pd3dDeviceContext == nullptr) return;
	HWND hWnd = *(HWND*)BasehWnd;
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\msyh.ttc", 16.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);
	oWndProc = (PWndProc)SetWindowLongPtrA(hWnd, GWLP_WNDPROC, (LONG_PTR)MyWndProc);
}

void Dx11Hook(void) {
	VMProtectBegin("Dx11Hook");
	// 启动一个线程来定期检查CRC32值
	InitializeCRCValues();
	std::thread crcChecker(CheckCRCValues);
	crcChecker.detach();

	
	GetSwapChain();
	GetDx11Ptr();
	InitImGui();
	SwapChainVtb = *(PDWORD64*)g_pSwapChain;
	oPresent = (PPresent)SwapChainVtb[8];
	HookBegin((LPVOID*)&oPresent, MyPresent);

	VMProtectEnd();
}
void HookBegin(LPVOID* oFuncAddr, LPVOID pDetour)
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(oFuncAddr, pDetour);
	DetourTransactionCommit();
}

void Render::Line(Vector2 Begine, Vector2 End, ImColor Color, float Width)
{
	ImGui::GetBackgroundDrawList()->AddLine(ImVec2(Begine.X, Begine.Y), ImVec2(End.X, End.Y), Color, Width);
}
void Render::Text(Vector2 Screen, ImColor Color, const char* fmt, ...)
{
	char Buffer[256] = { 0 };
	va_list args;
	va_start(args, fmt);
	vsprintf(Buffer, fmt,args );
	va_end(args);

	ImGui::GetBackgroundDrawList()->AddText(ImVec2(Screen.X, Screen.Y), Color, Buffer);
	//ImGui::GetBackgroundDrawList()->AddText(ImVec2(Screen.X+1, Screen.Y+1), Color, Buffer);
	

}
void Render::Rect(Vector2 Begine, Vector2 End, ImColor Color)
{
	ImGui::GetForegroundDrawList()->AddRect(ImVec2(Begine.X, Begine.Y), ImVec2(End.X, End.Y), Color);
}
void RenderClickableLink(const char* label, const char* url) {
	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 120, 255, 255)); // 蓝色文本
	ImGui::Text(label);
	ImGui::PopStyleColor();

	if (ImGui::IsItemHovered()) {
		ImGui::SetMouseCursor(ImGuiMouseCursor_Hand); // 鼠标悬停时显示手型光标
	}

	if (ImGui::IsItemClicked()) {
		// 使用 ShellExecute 打开默认浏览器并导航到指定 URL
		ShellExecuteA(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
	}
}

 
void Cheat::RenderPlayer(void) {
	GetController(PlayerHead);
	Tables::Table* Ptr = PlayerHead.Node;

	ViewMatrix* PViewMatrix = *(ViewMatrix**)BaseView;
	

	float currentTime = ImGui::GetTime(); // 获取当前时间
	DWORD64 bestTargetId = 0;
	float bestDistance = FLT_MAX;
	Vector2 bestAimCoor;


	while (Ptr) {
		IEntity* PIEntity = (IEntity*)Ptr->Data;


		if (!IsPtr(PIEntity)
			|| !IsPtr(PIEntity->PIEntityTable)
			|| !IsPtr(PIEntity->PIEntityTable->PActorComponent)
			|| !IsPtr(PIEntity->PIEntityTable->PActorComponent->PActor)
			|| !IsPtr(PIEntity->PIEntityTable->PActorComponent->PActor->PPlayerStatus)
			|| PIEntity->PIEntityTable->PActorComponent->PActor->PPlayerStatus->Status1 == FALSE) {

			Tables::SubTable(PlayerHead, Ptr->Data);
			Ptr = Ptr->Node;
			continue;
		}

		Actor* PActor = PIEntity->PIEntityTable->PActorComponent->PActor;
		if (strcmp((const char*)&PActor->MyPlayerUStr, "player_avatar") &&
			PIEntity->Camp != 2139062150 &&
			PActor->PPlayerStatus->Status1 >= 1 &&
			PActor->PPlayerStatus->Status1 <= 1000
			) 
		{
			Vector3 WorldCoor;
			Vector2 Screen;

			MeshToVector3(PIEntity->Mesh, WorldCoor);
			if (!WorldToScreen(Screen, WorldCoor)) {
				Ptr = Ptr->Node;
				continue;
			}
			
			if (玩家) {
				Render::Text(Screen, ImColor(255, 0, 0, 255), u8"IEntity:%p Status1:%d Status2:%d 阵营:%s", PIEntity,
					PActor->PPlayerStatus->Status1,
					PActor->PPlayerStatus->Status2,
					(const char*)&PActor->MyPlayerUStr);
			}

			
			
			//if (PActor->PPose->PBoneBegine == PActor->PPose->PBoneEnd) 
			{//骨骼
			
					
					//for (size_t i = 0; i < PActor->PPose->GeTBoneCount(); i++)
					//{
					//double BoneFlag = PActor->PPose->Round(PActor->PPose->PBoneBegine->PBoneBlock[i]._83,2);
					//if (BoneFlag == 0.0f)
					//	continue;
					//Vector2 BoneScreen;
					//Vector3 BoneWorld =  PActor->PPose->BoneVectorToWorld(PIEntity->Mesh, PActor->PPose->PBoneBegine->PBoneBlock[i]);
					//WorldToScreen(BoneScreen, BoneWorld);
					//Render::Text(BoneScreen, ImColor(255, 255, 0, 255), "%f", BoneFlag);
					//}
					
				BONEDATA BoneData;
				for (size_t i = 0; i < PActor->PPose->GeTBoneCount(); i++) {
						double BoneFlag = PActor->PPose->Round(PActor->PPose->PBoneBegine->PBoneBlock[i]._83, 2);
						if (BoneData.BoneMax == BoneData.BoneMin)
							break;

						//脖子
						if (BoneFlag == -0.07 && BoneData._Block[0].PBoneInfoMation == NULL){
							BoneData._Block[0] = PActor->PPose->PBoneBegine->PBoneBlock[i];
							BoneData.BoneMin++;
							continue;
						}

						//盆骨
						if (BoneData._Block[1].PBoneInfoMation == NULL){
							BoneData._Block[1] = PActor->PPose->PBoneBegine->PBoneBlock[0];
							BoneData.BoneMin++;
							continue;
						}

						//胳膊1
						if (BoneFlag == -0.27 && BoneData._Block[2].PBoneInfoMation == NULL){
							BoneData._Block[2] = PActor->PPose->PBoneBegine->PBoneBlock[i];
							BoneData.BoneMin++;
							continue;
						}

						//胳膊2
						if (BoneFlag == -0.27 && BoneData._Block[3].PBoneInfoMation == NULL){
							BoneData._Block[3] = PActor->PPose->PBoneBegine->PBoneBlock[i];
							BoneData.BoneMin++;
							continue;
						}
						//脊柱
						if (BoneFlag == -0.21 && BoneData._Block[10].PBoneInfoMation == NULL){
							BoneData._Block[10] = PActor->PPose->PBoneBegine->PBoneBlock[i];
							BoneData.BoneMin++;
							continue;
						}

						//手1
						if (BoneFlag == -0.21 && BoneData._Block[4].PBoneInfoMation == NULL){
							BoneData._Block[4] = PActor->PPose->PBoneBegine->PBoneBlock[i];
							BoneData.BoneMin++;
							continue;
						}

						//手2
						if (BoneFlag == -0.21 && BoneData._Block[5].PBoneInfoMation == NULL){
							BoneData._Block[5] = PActor->PPose->PBoneBegine->PBoneBlock[i];
							BoneData.BoneMin++;
							continue;
						}

						//膝盖1
						if (BoneFlag == -0.38 && BoneData._Block[6].PBoneInfoMation == NULL){
							BoneData._Block[6] = PActor->PPose->PBoneBegine->PBoneBlock[i];
							BoneData.BoneMin++;
							continue;
						}

						//膝盖2
						if (BoneFlag == -0.38 && BoneData._Block[7].PBoneInfoMation == NULL){
							BoneData._Block[7] = PActor->PPose->PBoneBegine->PBoneBlock[i];
							BoneData.BoneMin++;
							continue;
						}

						//脚1
						if (BoneFlag == -0.40 && BoneData._Block[8].PBoneInfoMation == NULL){
							BoneData._Block[8] = PActor->PPose->PBoneBegine->PBoneBlock[i];
							BoneData.BoneMin++;
							continue;
						}

						//脚2
						if (BoneFlag == -0.40 && BoneData._Block[9].PBoneInfoMation == NULL){
							BoneData._Block[9] = PActor->PPose->PBoneBegine->PBoneBlock[i];
							BoneData.BoneMin++;
							continue;
						}

						



				}
					//BoneData.BoneMin = 0;

					for (size_t i = 0; i < BoneData.BoneMax; i++) {
						BoneData.BoneWorld[i] = PActor->PPose->BoneVectorToWorld(PIEntity->Mesh, BoneData._Block[i]);
						if (!WorldToScreen(BoneData.BoneScreen[i], BoneData.BoneWorld[i])) {
							continue;
						}
					}
					
					if (BoneData.BoneMax == BoneData.BoneMin) {
						if (骨骼) {
							Render::Line(BoneData.BoneScreen[0], BoneData.BoneScreen[1], ImColor(255, 0, 0, 255), 1);		//脖子到盆骨

							Render::Line(BoneData.BoneScreen[0], BoneData.BoneScreen[2], ImColor(255, 0, 0, 255), 1);	//脖子到胳膊1
							Render::Line(BoneData.BoneScreen[0], BoneData.BoneScreen[3], ImColor(255, 0, 0, 255), 1);	//脖子到胳膊2

							Render::Line(BoneData.BoneScreen[2], BoneData.BoneScreen[4], ImColor(255, 0, 0, 255), 1);	//胳膊1到手1
							Render::Line(BoneData.BoneScreen[3], BoneData.BoneScreen[5], ImColor(255, 0, 0, 255), 1);	//胳膊2到手2

							Render::Line(BoneData.BoneScreen[1], BoneData.BoneScreen[6], ImColor(255, 0, 0, 255), 1);	//盆骨到腿1
							Render::Line(BoneData.BoneScreen[1], BoneData.BoneScreen[7], ImColor(255, 0, 0, 255), 1);	//盆骨到腿2

							Render::Line(BoneData.BoneScreen[6], BoneData.BoneScreen[8], ImColor(255, 0, 0, 255), 1);	//腿1到脚1
							Render::Line(BoneData.BoneScreen[7], BoneData.BoneScreen[9], ImColor(255, 0, 0, 255), 1);	//腿2到脚2
						}


						Vector2 Box[2];
						BoneData.GetLeftIndex();
						BoneData.GetRightIndex();
						BoneData.GetLowestIndex();
						BoneData.GetHighesIndex();

						if (方框) {

							float boneHeight = BoneData.BoneScreen[8].Y - BoneData.BoneScreen[0].Y;

							float heightOffset = boneHeight * 0.1f;

							Box[0].X = BoneData.BoneScreen[BoneData.LeftIndex].X;
							Box[0].Y = BoneData.BoneScreen[BoneData.HighesIndex].Y - heightOffset;

							Box[1].X = BoneData.BoneScreen[BoneData.RightIndex].X;
							Box[1].Y = BoneData.BoneScreen[BoneData.LowestIndex].Y + heightOffset;

							Render::Rect(Box[0], Box[1], ImColor(255, 0, 0, 255));


						}

						if (武器) {
							Vector2 weaponTextPos = Box[0];
							weaponTextPos.Y -= 15; // 在方框顶部上方留出 15 像素的偏移量
							if (!IsPtr(PActor->Weapon1) || !IsPtr(PActor->Weapon1->Weapon)) {
								Render::Text(weaponTextPos, ImColor(255, 255, 255, 255), u8"空手"); // 白色
							}
							else {
								std::string weaponPath(PActor->Weapon1->Weapon->WeaponNameclass.WeaponName->WeaponName);
								// 检查是否为特定的手雷类型
								if (weaponPath == "Char/graph/empty.graph") {
									//float grenadeID = PActor->Weapon1->Weapon->GrenadeID;

									//if (FloatEqual(grenadeID, -0.032856f)) {
									//	Render::Text(weaponTextPos, ImColor(255, 0, 0, 255), u8"手榴弹"); // 红色
									//}
									//else if (FloatEqual(grenadeID, -0.035027f)) {
									//	Render::Text(weaponTextPos, ImColor(255, 0, 0, 255), u8"烟雾弹"); // 红色
									//}
									//else if (FloatEqual(grenadeID, -0.042450f)) {
									//	Render::Text(weaponTextPos, ImColor(255, 0, 0, 255), u8"闪光弹"); // 红色
									//}
									//else if (FloatEqual(grenadeID, -0.035315f)) {
									//	Render::Text(weaponTextPos, ImColor(255, 0, 0, 255), u8"毒气弹"); // 红色
									//}
									//else if (FloatEqual(grenadeID, -0.069347f)) {
									//	Render::Text(weaponTextPos, ImColor(255, 0, 0, 255), u8"C4"); // 红色
									//}
									//else if (FloatEqual(grenadeID, -0.017459f)) {
									//	Render::Text(weaponTextPos, ImColor(255, 0, 0, 255), u8"地形镭"); // 红色
									//}
									//else {
									//	Render::Text(weaponTextPos, ImColor(255, 255, 255, 255), u8"空手"); // 白色
									//}
									Render::Text(weaponTextPos, ImColor(255, 0, 0, 255), u8"投掷物！！！"); // 红色
								}
								else if (weaponPath == "Char/graph/item/aircraft.graph") {
									Render::Text(weaponTextPos, ImColor(255, 255, 0, 255), u8"烟幕无人机"); // 黄色
								}
								else {
									// 查找并删除指定的前缀和后缀
									const std::string prefix = "Char/graph/gun/";
									const std::string suffix = ".graph";
									size_t startPos = weaponPath.find(prefix);
									if (startPos != std::string::npos) {
										weaponPath.erase(startPos, prefix.length());
									}
									size_t endPos = weaponPath.find(suffix);
									if (endPos != std::string::npos) {
										weaponPath.erase(endPos, suffix.length());
									}
									// 转换为大写
									std::string weaponNameUpper = ToUpper(weaponPath);
									// 绘制处理后的武器名称
									Render::Text(weaponTextPos, ImColor(255, 255, 0, 255), u8"手握: %s", weaponNameUpper.c_str()); // 黄色
								}
							}
						}

						if (射线) {
							Vector2 RayLine[2];
							// 确保起始点在上方
							RayLine[0].X = BoneData.BoneScreen[1].X;
							RayLine[0].Y = BoneData.BoneScreen[BoneData.LowestIndex].Y; // 使用最低点

							// 终点在屏幕底部中央
							RayLine[1].X = PViewMatrix->Screen.X / 2;
							RayLine[1].Y = PViewMatrix->Screen.Y;
							Render::Line(RayLine[0], RayLine[1], ImColor(255, 0, 0, 255), 1);
						}

						if (自瞄) {
							DWORD64 currentPlayerId = (DWORD64)PIEntity; // 使用IEntity指针作为唯一ID
							FLOAT Distance = sqrtf(powf((PViewMatrix->Screen.X / 2) - BoneData.BoneScreen[0].X, 2) + powf((PViewMatrix->Screen.Y / 2) - BoneData.BoneScreen[0].Y, 2));

							bool shouldConsiderTarget = true;
							if (PActor->PPlayerStatus->Status1 == 26 && !自瞄倒地) {
								shouldConsiderTarget = false;
							}
							if (shouldConsiderTarget && Distance < 范围) {
								Vector2 targetCoor;
								switch (aimTarget) {
								case HEAD:
									targetCoor = BoneData.BoneScreen[0]; // 自瞄头部
									break;
								case CHEST:
									targetCoor = BoneData.BoneScreen[10]; // 自瞄胸口
									break;
								case NEAREST:
									float distances[] = {
											sqrtf(powf((PViewMatrix->Screen.X / 2) - BoneData.BoneScreen[0].X, 2) + powf((PViewMatrix->Screen.Y / 2) - BoneData.BoneScreen[0].Y, 2)),
											sqrtf(powf((PViewMatrix->Screen.X / 2) - BoneData.BoneScreen[10].X, 2) + powf((PViewMatrix->Screen.Y / 2) - BoneData.BoneScreen[10].Y, 2)),
											sqrtf(powf((PViewMatrix->Screen.X / 2) - BoneData.BoneScreen[6].X, 2) + powf((PViewMatrix->Screen.Y / 2) - BoneData.BoneScreen[6].Y, 2)),
											sqrtf(powf((PViewMatrix->Screen.X / 2) - BoneData.BoneScreen[7].X, 2) + powf((PViewMatrix->Screen.Y / 2) - BoneData.BoneScreen[7].Y, 2)),
											sqrtf(powf((PViewMatrix->Screen.X / 2) - BoneData.BoneScreen[8].X, 2) + powf((PViewMatrix->Screen.Y / 2) - BoneData.BoneScreen[8].Y, 2)),
											sqrtf(powf((PViewMatrix->Screen.X / 2) - BoneData.BoneScreen[9].X, 2) + powf((PViewMatrix->Screen.Y / 2) - BoneData.BoneScreen[9].Y, 2))
									};

									int indices[] = { 0, 10, 6, 7, 8, 9 };
									auto minElement = std::min_element(std::begin(distances), std::end(distances));
									int minIndex = std::distance(std::begin(distances), minElement);
									targetCoor = BoneData.BoneScreen[indices[minIndex]];
									break;
								}

								if (currentPlayerId == currentTargetId &&
									currentTime - targetLockTime < TARGET_LOCK_DURATION) {
									// 当前锁定目标仍然有效
									bestTargetId = currentPlayerId;
									bestDistance = Distance;
									bestAimCoor = targetCoor;
								}
								else if (bestTargetId == 0 || Distance < bestDistance) {
									// 更新最佳目标
									bestTargetId = currentPlayerId;
									bestDistance = Distance;
									bestAimCoor = targetCoor;
								}
							}

						}
					}
			}

		}
		Ptr = Ptr->Node;
	}
	if (bestTargetId != 0) {
		if (bestTargetId != currentTargetId) {
			currentTargetId = bestTargetId;
			targetLockTime = currentTime;
		}

		AimCoor = bestAimCoor;
		

		if (GetAsyncKeyState(自瞄热键1) & 0x8000 || GetAsyncKeyState(自瞄热键2) & 0x8000) {
			// 执行自瞄逻辑
			Vector2 currentMousePos = { PViewMatrix->Screen.X / 2, PViewMatrix->Screen.Y / 2 };
			float deltaX = AimCoor.X - currentMousePos.X;
			float deltaY = AimCoor.Y - currentMousePos.Y;

			// 使用平滑因子计算移动距离
			float moveX = deltaX * SMOOTH_FACTOR;
			float moveY = deltaY * SMOOTH_FACTOR;

			// 应用非线性曲线以提高精度
			moveX = (fabs(moveX) < 1.0f) ? moveX * fabs(moveX) : moveX;
			moveY = (fabs(moveY) < 1.0f) ? moveY * fabs(moveY) : moveY;

			// 限制最大移动速度

			moveX = std::clamp(moveX, -MAX_SPEED, MAX_SPEED);
			moveY = std::clamp(moveY, -MAX_SPEED, MAX_SPEED);

			// 如果移动距离太小,直接移动到目标位置
			if (fabs(deltaX) < MIN_DISTANCE && fabs(deltaY) < MIN_DISTANCE) {
				moveX = deltaX;
				moveY = deltaY;
			}

			// 移动鼠标
			if (fabs(moveX) > 0.01f || fabs(moveY) > 0.01f) {
				mouse_event(MOUSEEVENTF_MOVE,static_cast<DWORD>(moveX), static_cast<DWORD>(moveY), 0, 0);
			}

		}
	}
	else {
		// 如果没有找到合适的目标,重置当前目标
		currentTargetId = 0;
	}

	

}

float Lerp(float a, float b, float t) {
	return a + t * (b - a);
}

void Cheat::RenderGoods(void) {
	HookGoods(GoodsThis);
	Tables::AddTable(GoodsHead, *(PDWORD64)GoodsThis);
	Tables::Table* Ptr = GoodsHead.Node;

	while (Ptr) {
		area_impl* Parea_impl = (area_impl*)Ptr->Data;
		if (!IsPtr(Parea_impl) || !IsVector3(Parea_impl->WorldCoor)) {
			Tables::SubTable(GoodsHead, Ptr->Data);
			Ptr = Ptr->Node;
			continue;
		}

		GetGoodsGetClassName(Parea_impl, ClassName);
		const char* classNameStr = reinterpret_cast<const char*>(ClassName);

		if (classNameStr == nullptr || strlen(classNameStr) == 0) {
			Ptr = Ptr->Node;
			continue;
		}

		if (strcmp((const char*)ClassName, "PlayerAvatar") ) {
			Vector2 Screen;
			if (!WorldToScreen(Screen, Parea_impl->WorldCoor)) {
				Ptr = Ptr->Node;
				continue;
			}
			//物品ID* 物品id = Parea_impl->wupin1->wupin2->wupin3->wupin4->wupin5;
			


			if (车辆 && !strcmp((const char*)ClassName, "Vehicle")) {

				Render::Text(Screen, ImColor(255, 255, 0, 255), u8"车辆");

			}

			if (空投 && !strcmp((const char*)ClassName, "AirDropBox")) {

				Render::Text(Screen, ImColor(255, 0, 0, 255), u8"空投");

			}

			if (真人 && !strcmp((const char*)ClassName, "Avatar")) {

				Render::Text(Screen, ImColor(127, 255, 212, 255), u8"真人");

			}

		}

		Ptr = Ptr->Node;
	}
}



