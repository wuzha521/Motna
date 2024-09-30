#pragma once
#include"‘§±‡“Î.h"
#include "Class.h"
#include "Table.h"

extern HMODULE ImageBase;
extern PDWORD64 BaseSwapChain;
extern PDWORD64 BasehWnd;
extern PDWORD64 BaseView;
extern PDWORD64 BaseConroller;


BOOL MeshToVector3(MESH Mesh, Vector3& Vec3);
BOOL GetController(Tables::Table& PlayerHead);
BOOL IsVector3(Vector3 Vec3);
BOOL WorldToScreen(Vector2& Screen, Vector3 WorldCoor);
BOOL HookGoods(LPVOID& This);
BOOL IsPtr(LPVOID Ptr);
BOOL GetGoodsGetClassName(area_impl* area_impl, LPVOID& ClassName);