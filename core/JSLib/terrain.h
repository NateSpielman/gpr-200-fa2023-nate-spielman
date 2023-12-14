/*
	Author: Jake Sanderson
	File:terrain.h
*/

#pragma once
#include "../ew/mesh.h"
#include "../ew/external/stb_image.h"
#include "../ew/external/glad.h"
namespace JSLib
{
	ew::MeshData createTerrain(char* heightMap);
}