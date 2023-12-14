/*
	Author: Jake Sanderson
	File:terrain.cpp
*/

#include "terrain.h"
namespace JSLib
{
	ew::MeshData createTerrain(char* heightMap)
	{
		int row, col, start;
		ew::Vertex v;
		ew::MeshData mesh;

		int width, height, numComponents;
		unsigned char* data = stbi_load(heightMap, &width, &height, &numComponents, 0);

		float yScale = 64.0f / 256.0f;

		//Vertices
		for (row = 0; row < height; row++)
		{
			for (col = 0; col < width; col++)
			{
				//Get texel for where current vertex is at and get its pixel data
				unsigned char* texel = data + (col + width * row) * numComponents;
				unsigned char y = texel[0];

				v.pos.x = -height / 2.0f + row;
				v.pos.y = (int)y * yScale;
				v.pos.z = -width / 2.0f + col;

				v.normal = ew::Vec3(0.0f, 0.0f, 0.0f);

				v.uv.x = col / (float)height;
				v.uv.y = row / (float)width;

				mesh.vertices.push_back(v);
			}
		}

		stbi_image_free(data);

		//Indices
		int indBottomLeft, indTopLeft, indTopRight, indBottomRight;

		for (row = 0; row < height - 1; row++)
		{
			for (col = 0; col < width - 1; col++)
			{
				//Indices for each quad
				indBottomLeft = row * width + col;
				indTopLeft = (row + 1) * width + col;
				indTopRight = (row + 1) * width + col + 1;
				indBottomRight = row * width + col + 1;

				//Top left triangle
				mesh.indices.push_back(indBottomLeft);
				mesh.indices.push_back(indTopRight);
				mesh.indices.push_back(indTopLeft);

				//Bottom right triangle
				mesh.indices.push_back(indBottomLeft);
				mesh.indices.push_back(indBottomRight);
				mesh.indices.push_back(indTopRight);
			}
		}

		//Normals
		for (int i = 0; i < mesh.indices.size(); i += 3)
		{
			//Gets indices of the three points of a triangle
			int ind0 = mesh.indices[i];
			int ind1 = mesh.indices[i + 1];
			int ind2 = mesh.indices[i + 2];

			//Makes two vectors from the three points
			ew::Vec3 v1 = mesh.vertices[ind1].pos - mesh.vertices[ind0].pos;
			ew::Vec3 v2 = mesh.vertices[ind2].pos - mesh.vertices[ind0].pos;

			//Calculates their norm
			ew::Vec3 normal = ew::Normalize(ew::Cross(v1, v2));

			//Adds to vertices normals
			mesh.vertices[ind0].normal += normal;
			mesh.vertices[ind1].normal += normal;
			mesh.vertices[ind2].normal += normal;
		}

		for (int i = 0; i < mesh.vertices.size(); i++)
		{
			mesh.vertices[i].normal = ew::Normalize(mesh.vertices[i].normal);
		}

		return mesh;
	}
}