#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <filesystem>
#include <iostream>
#include <unordered_map>
#include <sysinfoapi.h>
#include <vector>
#include <cstdint>
#include <string>
#include <fstream>
#include <cstdint>
#include <stddef.h>

#include <BinaryIO.h>
#include <Utils.h>
#include <Half.h>
#include <Quaternion.h>
#include <Vector2.h>
#include <Vector3.h>
#include <structs_VG.h>

using namespace Math;

// packed vector
struct Vector32
{
	uint32_t x : 10;
	uint32_t y : 10;
	uint32_t z : 10;
	uint32_t exp : 2;
};


struct Vector48
{
	uint16_t x;
	uint16_t y;
	uint16_t z;
};

struct Quat32
{
	uint32_t x : 11;
	uint32_t y : 10;
	uint32_t z : 10;
	uint32_t wneg : 1;
};

struct Quat48
{
	uint16_t x : 16;
	uint16_t y : 16;
	uint16_t z : 15; // broken
	uint16_t wneg : 1;
};

struct Quat64
{
	uint64_t x : 21;
	uint64_t y : 21;
	uint64_t z : 21;
	uint64_t wneg : 1;
};

struct RadianEuler
{
	// for when you want radians in float.
	float x;
	float y;
	float z;
};

struct matrix3x4_t
{       //    c0         c1         c2         c3
	float m00; float m01; float m02; float m03; // r0
	float m10; float m11; float m12; float m13; // r1
	float m20; float m21; float m22; float m23; // r2
};