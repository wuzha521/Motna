#include "Class.h"

size_t  Pose::GeTBoneCount()
{
	return(size_t)(((DWORD64)this->PBoneEnd - (DWORD64)this->PBoneBegine) / sizeof(BoneBlock));


}

Vector3 Pose::BoneVectorToWorld(MESH Mesh, BoneBlock Block)
{
	Vector3 WorldCoor;
	Vector3 BoneDot;
	BoneDot.X = Block._61;
	BoneDot.Y = Block._62;
	BoneDot.Z = Block._63;
	WorldCoor.X = Mesh._20 * BoneDot.X + Mesh._13 * BoneDot.Y + Mesh._12 * BoneDot.Z + Mesh._21;
	WorldCoor.Y = Mesh._11 * BoneDot.X + Mesh._10 * BoneDot.Y + Mesh._03 * BoneDot.Z + Mesh._22;
	WorldCoor.Z = Mesh._02 * BoneDot.X + Mesh._01 * BoneDot.Y + Mesh._00 * BoneDot.Z + Mesh._23;
	return WorldCoor;
}

DOUBLE Pose::Round(DOUBLE number, INT digits)
{
	DOUBLE multiplier = pow(10.0, digits);
	DOUBLE rounded = round(number * multiplier) / multiplier;
	return rounded;

}

void BONEDATA::GetLeftIndex()
{
	size_t Ret = 0;
		for (int i = 0; i < this->BoneMax; i++)
		{
			if (this->BoneScreen[i].X < this->BoneScreen[Ret].X)
				Ret = i;
		}
		this->LeftIndex = Ret;
}

void BONEDATA::GetRightIndex()
{
	size_t Ret = 0;
	for (int i = 0; i < this->BoneMax; i++)
	{
		if (this->BoneScreen[i].X > this->BoneScreen[Ret].X)
			Ret = i;
	}
	this->RightIndex = Ret;

}

void BONEDATA::GetLowestIndex()
{
	size_t Ret = 0;
	for (int i = 0; i < this->BoneMax; i++)
	{
		if (this->BoneScreen[i].Y > this->BoneScreen[Ret].Y)
			Ret = i;
	}
	this->LowestIndex = Ret;
}



void BONEDATA::GetHighesIndex()
{
	size_t Ret = 0;
	for (int i = 0; i < this->BoneMax; i++)
	{
		if (this->BoneScreen[i].Y < this->BoneScreen[Ret].Y) // 寻找最小的 Y 值
			Ret = i;
	}
	this->HighesIndex = Ret;
}
