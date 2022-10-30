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

#include "BinaryIO.h"
#include "Utils.h"
#include "Half.h"
using namespace Math;

struct Vector2
{
	float x, y;

	Vector2(float x, float y) {
		this->x = x;
		this->y = y;
	}

	Vector2() {};
};

struct Vector3
{
	float x, y, z;

	Vector3(float x, float y, float z) {
		this->x = x;
		this->y = y;
		this->z = z;
	}

	Vector3() {};
};

struct Quaternion
{
	float x, y, z, w;

	Quaternion(float x, float y, float z, float w) {
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
	}

	Quaternion() {};
};

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