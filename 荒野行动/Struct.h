#pragma once

#include "预编译.h"

struct Config {
	bool 骨骼;
	bool 玩家;
	bool 自瞄;
	bool 世界地址;
	bool 方框;
	bool 射线;
	bool 自瞄倒地;
	float SMOOTH_FACTOR;
	float MIN_DISTANCE;
	float MAX_SPEED;
	float TARGET_LOCK_DURATION;
	float 范围;
};

struct Vector2
{
	float X, Y;

	// 默认构造函数
	Vector2() : X(0), Y(0) {}

	// 带参数的构造函数
	Vector2(float x, float y) : X(x), Y(y) {}
};



struct Vector3 : public Vector2
{
	float Z;
	Vector3()
	{
		this->Z = 0;
	}



};

struct Vector4 : public Vector3
{
	float W;

	Vector4()
	{
		this->W = 0;
	}

};



struct MESH
{

	float _00, _01, _02, _03; 
	float _10, _11, _12, _13;
	float _20, _21, _22, _23;
	MESH()
	{
		this->_00 = 0, this->_01 = 0, this->_02 = 0, this->_03 = 0;
		this->_10 = 0, this->_11 = 0, this->_12 = 0, this->_13 = 0;
		this->_20 = 0, this->_21 = 0, this->_22 = 0, this->_23 = 0;
	}

}; 

struct MATRIX : public MESH
{

	float _30, _31, _32, _33;
	MATRIX()
	{
		this->_30 = 0, this->_31 = 0, this->_32 = 0, this->_33 = 0;
	}

};


