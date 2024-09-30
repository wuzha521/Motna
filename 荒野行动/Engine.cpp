#include "Engine.h"
#include "Table.h"
 HMODULE ImageBase = GetModuleHandle(0);
 PDWORD64 BaseSwapChain	= (PDWORD64)((DWORD64)ImageBase + 0x5F77A80);//dx11
 PDWORD64 BasehWnd			= (PDWORD64)((DWORD64)ImageBase + 0x5F711F0);//hWnd
 PDWORD64 BaseHookGoods		= (PDWORD64)((DWORD64)ImageBase + 0x5ADE1D);//世界物品基址
 PDWORD64 BaseView			= (PDWORD64)((DWORD64)ImageBase + 0x5F77E80);//矩阵
 PDWORD64 BaseConroller		= (PDWORD64)((DWORD64)ImageBase + 0x62A1E08);//玩家控制器
 



 BOOL IsVector3(Vector3 Vec3) {
	 return Vec3.X != 0 && Vec3.Y != 0 && Vec3.Z != 0;
 }

 BOOL MeshToVector3(MESH Mesh, Vector3& Vec3) {
	 Vec3.X = Mesh._21;
	 Vec3.Y = Mesh._22;
	 Vec3.Z = Mesh._23;
	 return IsVector3(Vec3);
 }


 BOOL WorldToScreen(Vector2& Screen, Vector3 WorldCoor) {
	 Vector2 ForseSight;
	 ViewMatrix* PViewMatrix = *(ViewMatrix**)BaseView;
	 if (!IsPtr(PViewMatrix)) return false; // 检查指针有效性

	 ForseSight.X = PViewMatrix->Screen.X / 2;
	 ForseSight.Y = PViewMatrix->Screen.Y / 2;
	 float SightZ = PViewMatrix->Matrix._30 * WorldCoor.X + PViewMatrix->Matrix._31 * WorldCoor.Y + PViewMatrix->Matrix._32 * WorldCoor.Z + PViewMatrix->Matrix._33;
	 if (SightZ > 0.01 && SightZ < 800.0f) {
		 SightZ = 1 / SightZ;
		 Screen.X = ForseSight.X + (PViewMatrix->Matrix._00 * WorldCoor.X + PViewMatrix->Matrix._01 * WorldCoor.Y + PViewMatrix->Matrix._02 * WorldCoor.Z + PViewMatrix->Matrix._03) * SightZ * ForseSight.X;
		 Screen.Y = ForseSight.Y - (PViewMatrix->Matrix._10 * WorldCoor.X + PViewMatrix->Matrix._11 * WorldCoor.Y + PViewMatrix->Matrix._12 * WorldCoor.Z + PViewMatrix->Matrix._13) * SightZ * ForseSight.Y;
		 return true;
	 }
	 Screen.X = 10000;
	 Screen.Y = 10000;
	 return false;
 }

 BOOL GetController(Tables::Table& PlayerHead) {
	 PDWORD64 Buffer = nullptr;
	 Buffer = (PDWORD64)((DWORD64)*BaseConroller + 0x08);

	 if (IsPtr(Buffer)) {
		 Buffer = (PDWORD64)((DWORD64)*Buffer + 0x00);

		 if (IsPtr(Buffer)) {
			 Buffer = (PDWORD64)((DWORD64)*Buffer + 0x18);
			 if (IsPtr(Buffer) && IsPtr(*(LPVOID*)Buffer)) {
				 ControllerManager* PControllerManager = *(ControllerManager**)Buffer;
				 for (INT i = 0; i < PControllerManager->Count; i++) {
					 Controller* PController = PControllerManager->PControllerArray->PController[i];

					 if (IsPtr(PController) &&
						 IsPtr(PController->PControllerCallBack) &&
						 IsPtr(PController->PControllerCallBack->PCharCtrlComponent) &&
						 IsPtr(PController->PControllerCallBack->PCharCtrlComponent->PIEntity) &&
						 IsPtr(PController->PControllerCallBack->PCharCtrlComponent->PIEntity->PIEntityTable) &&
						 IsPtr(PController->PControllerCallBack->PCharCtrlComponent->PIEntity->PIEntityTable->PActorComponent)) {

						 Tables::AddTable(PlayerHead, (Tables::DataType)PController->PControllerCallBack->PCharCtrlComponent->PIEntity);
					 }
				 }
				 return true;
			 }
		 }
	 }

	 return false;
 }

	

 BOOL IsPtr(LPVOID Ptr) {
	 return !IsBadReadPtr((LPVOID)Ptr, 8);
 }
 
 BOOL GetGoodsGetClassName(area_impl* area_impl, LPVOID& ClassName) {
	 if (!IsPtr(area_impl) || area_impl->Len > 50)
		 return false;

	 if (!IsPtr(area_impl->ClassName))
		 ClassName = &area_impl->ClassName;
	 else ClassName = area_impl->ClassName;

	 return true;
 }

 BOOL HookGoods(LPVOID& This)
 {
	 char ShellCode[]
	 {


		 0xFF,0x25,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,					//jmp MyAddress
		 0x90,																																//nop
		 0x90,																																//nop
		 0x90,																																//nop
		 0x90,																																//nop
		 0x48,0x8B,0x01,																												//mov rax,[rcx]
		 0x49,0xB8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00	,													//mov r8,MyAddress+100
		 0x49,0x89,0x00,																												//mov[r8],rax
		 0x4D,0x31,0xC0,																												//xor r8,r8
		 0x48,0x8B,0xB0,0xF8,0x01,0x00,0x00,																				//mov rsi,[rax + 000001F8]
		 0x48,0x8B,0xCE,																												// mov rcx,rsi
		 0xFF,0x15,0x02,0x00,0x00,0x00,0xEB,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//call hyxd.exe + 2500230
		 0xFF,0x25,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00					// jmp hyxd.exe + 5ADE2F
		 
	 };


	 if (*(DWORD*)BaseHookGoods == 0x000025FF)
		 return false;

	 LPVOID  MyAddress = new char[0x1000];
	 memset(MyAddress, 0, 0x1000);

	 DWORD GoodsProtext;
	 DWORD MyPortext;

	 if (!VirtualProtect(MyAddress, 0x1000, PAGE_EXECUTE_READWRITE, &MyPortext)) {
		 delete[] MyAddress;
		 return false;
	 }
	 if (!VirtualProtect(BaseHookGoods, 0x1000, PAGE_EXECUTE_READWRITE, &GoodsProtext)) {
		 VirtualProtect(MyAddress, 0x1000, MyPortext, &MyPortext);
		 delete[] MyAddress;
		 return false;
	 }

	 LPVOID TmpMyaddress			= (LPVOID)((DWORD64)MyAddress + 0x100);
	 LPVOID TmpCall					= (LPVOID)((DWORD64)ImageBase + 0x2500230);
	 LPVOID TmpJmpGoods			= (LPVOID)((DWORD64)BaseHookGoods + 0x12);
	 
	 

	 memcpy(&ShellCode[6], &MyAddress, 8);
	 memcpy(&ShellCode[23], &TmpMyaddress, 8);
	 memcpy(&ShellCode[55], &TmpCall, 8);
	 memcpy(&ShellCode[69], &TmpJmpGoods, 8);

	 memcpy(BaseHookGoods, &ShellCode[0], 18);
	 memcpy(MyAddress, &ShellCode[18], 59);
	 
	 VirtualProtect(BaseHookGoods, 0x1000, PAGE_EXECUTE_READWRITE, &GoodsProtext);

	 This = TmpMyaddress;
	 



	 return true;


 }
