#pragma once
#include "预编译.h"
#include "Struct.h"

//class 物品ID
//{
//public:
//    char PTR00[0x030]; int 物品id;                                //0x030
//    //char PTR01[0x00C]; int 子弹数量;                            //0x05C
//};
//
//class 物品id4
//{
//public:
//    char PTR00[0x018]; 物品ID* wupin5;                //0x018
//};
//
//class 物品id3
//{
//public:
//    char PTR00[0x018]; 物品id4* wupin4;                //0x018
//};
//
//class 物品id2
//{
//public:
//    char PTR00[0x010]; 物品id3* wupin3;                //0x010
//};
//
//class 物品id1
//{
//public:
//    char PTR00[0x020]; 物品id2* wupin2;                //0x020
//};

class area_impl
{
public:
    char PTR00[0x0A8]; LPVOID ClassName;        //0x0A8
    char PTR01[0x008]; INT Len;                            //0x0B8
    char PTR02[0x00C]; Vector3 WorldCoor;            //0x0C8
    //char PTR03[0x124]; 物品id1* wupin1;                //0x1F8
};

class ViewMatrix
{
public:
    char PTR00[0x014]; Vector2 Screen;        //0x014
    char PTR01[0x554]; MATRIX Matrix;        //0x570
};

class PlayerStatus
{
public:
    char PTR00[0x048]; INT Status1;            //0x048        状态
    char PTR01[0x03C]; INT Status2;            //0x088        抓timing
};

class BoneInfoMation
{
public:
    // 添加必要的成员变量和函数
};

class BoneBlock
{
public:
    BoneInfoMation* PBoneInfoMation;

    float _00, _01, _02, _03;
    float _10, _11, _12, _13;
    float _20, _21, _22, _23;
    float _30, _31, _32, _33;
    float _40, _41, _42, _43;
    float _50, _51, _52, _53;
    float _60, _61, _62, _63;
    float _70, _71, _72, _73;
    float _80, _81, _82, _83;
    float _90, _91;
};

class BoneArray
{
public:
    char PTR00[0x088]; BoneBlock PBoneBlock[1];            //0x088
};

class Pose
{
public:
    char PTR00[0x010];
    BoneArray* PBoneBegine;                                            //0x010
    BoneArray* PBoneEnd;                                                //0x018
    size_t GeTBoneCount();
    Vector3 BoneVectorToWorld(MESH Mesh, BoneBlock Block);
    DOUBLE Round(DOUBLE number, INT digits);
};

class WeaponName
{
public:
    char* WeaponName;

};


class WeaponNameclass
{
public:
    WeaponName* WeaponName;                                                 //0x00
};

class WeaponID
{
public:
    char PTR00[0x0E0]; FLOAT GrenadeID;                                   //0xE0
    char PTR01[0x1A4]; WeaponNameclass WeaponNameclass;     //0x288
};

class Weapon
{
public:
    char PTR00[0x08]; WeaponID* Weapon;                                  //0x08
};

class Actor
{
public:
    char PTR00[0x018]; Pose* PPose;                                     //0x018
    char PTR01[0x1B0]; LPVOID MyPlayerUStr;                      //0x1D0
    char PTR02[0x070]; Weapon* Weapon1;                          //0x248   武器数据
    char PTR03[0x038]; PlayerStatus* PPlayerStatus;            //0x288
};

class ActorComponent
{
public:
    char PTR00[0x0D0]; Actor* PActor;                            //0x0D0
};

class IStorboard
{
public:
    // 添加必要的成员变量和函数
};

class IEntityTable
{
public:
    IStorboard* PIStorboard;
    ActorComponent* PActorComponent;
};

class IEntity
{
public:
    char PTR00[0x040]; IEntityTable* PIEntityTable;            //0x040
    char PTR01[0x010]; MESH Mesh;                                //0x058
    char PTR02[0x036]; INT Camp;                                    //0x0A4 阵营
};

class CharCtrlComponent
{
public:
    char PTR00[0x040]; IEntity* PIEntity;                        //0x040        人物结构
};

class ControllerCallBack
{
public:
    char PTR00[0x030]; CharCtrlComponent* PCharCtrlComponent;//0x030
};

class Controller
{
public:
    char PTR00[0x0B0]; ControllerCallBack* PControllerCallBack; //0x0B0
};

class ControllerArray
{
public:
    Controller* PController[1];
};

class ControllerManager
{
public:
    char PTR00[0x060];
    ControllerArray* PControllerArray;    //0x060
    INT Count;                                    //0x068
};

class BONEDATA
{
public:
    Vector3 BoneWorld[20];
    Vector2 BoneScreen[20];
    BoneBlock _Block[20];
    size_t BoneMax;
    size_t BoneMin;

    size_t LeftIndex;
    size_t RightIndex;
    size_t LowestIndex;
    size_t HighesIndex;
    BONEDATA()
    {
        this->LeftIndex = 0;
        this->RightIndex = 0;
        this->LowestIndex = 0;
        this->HighesIndex = 0;

        this->BoneMax = 11;
        this->BoneMin = 0;
        for (size_t i = 0; i < this->BoneMax; i++)
            this->_Block[i].PBoneInfoMation = NULL;
    }
    void GetLeftIndex();
    void GetRightIndex();
    void GetLowestIndex();
    void GetHighesIndex();
};
