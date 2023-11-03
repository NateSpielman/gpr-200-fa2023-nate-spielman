#include "procGen.h"

namespace ns {
	/*
	ew::MeshData createSphere(float radius, int numSegments)
	{
		
	}

	ew::MeshData createCylinder(float height, float radius, int numSegments)
	{

	}*/

	ew::MeshData createPlane(float width, float height, int subdivisions)
	{
		ew::MeshData mesh;
		mesh.vertices.reserve((subdivisions + 1) * (subdivisions + 1));

		//Vertices
		for (int row = 0; row <= subdivisions; row++) {
			for (int col = 0; col <= subdivisions; col++) {
				ew::Vertex vertex;
				vertex.pos.x = width * (col / (float)subdivisions);
				vertex.pos.z = -height * (row / (float)subdivisions);

				vertex.normal = ew::Vec3(0.0f, 1.0f, 0.0f);

				vertex.uv.x = col / (float)subdivisions;
				vertex.uv.y = row / (float)subdivisions;

				mesh.vertices.push_back(vertex);
			}
		}

		//Indicies
		int columns = subdivisions + 1;
		for (int row = 0; row < subdivisions; row++) {
			for (int col = 0; col < subdivisions; col++) {
				int start = row * columns + col;
				//Bottom right triangle
				mesh.indices.push_back(start);
				mesh.indices.push_back(start + 1);
				mesh.indices.push_back(start + columns + 1);
				//Top left triangle
				mesh.indices.push_back(start);
				mesh.indices.push_back(start + columns + 1);
				mesh.indices.push_back(start + columns);
			}
		}

		return mesh;
	}
}