#include "procGen.h"

namespace ns {
	/*
	ew::MeshData createSphere(float radius, int numSegments)
	{
		
	}
	*/
	ew::MeshData createCylinder(float height, float radius, int numSegments)
	{
		ew::MeshData mesh;
		mesh.vertices.reserve((numSegments + 1) * 2);

		//Vertices
		float topY = height / 2.0f;
		float bottomY = -topY;
		float thetaStep = (ew::PI * 2.0f) / (float)numSegments;

		//Top Center Vertex
		ew::Vertex topVertex;
		topVertex.pos = ew::Vec3(0.0f, topY, 0.0f);
		topVertex.normal = ew::Vec3(0.0f, 1.0f, 0.0f);
		topVertex.uv = ew::Vec2(0.5f, 0.5f);
		mesh.vertices.push_back(topVertex);

		//Top Ring Cap
		for (int i = 0; i <= numSegments; i++) {
			ew::Vertex vertex;
			float theta;
			theta = (float)i * thetaStep;

			vertex.pos.x = cos(theta) * radius;
			vertex.pos.z = sin(theta) * radius;
			vertex.pos.y = topY;

			vertex.normal = ew::Vec3(0.0f, 1.0f, 0.0f);

			vertex.uv.x = (cos(theta) + 1) * 0.5;
			vertex.uv.y = (sin(theta) + 1) * 0.5;

			mesh.vertices.push_back(vertex);
		}

		//Top Ring Sides
		for (int i = 0; i <= numSegments; i++) {
			ew::Vertex vertex;
			float theta;
			theta = (float)i * thetaStep;

			vertex.pos.x = cos(theta) * radius;
			vertex.pos.z = sin(theta) * radius;
			vertex.pos.y = topY;

			vertex.normal = ew::Normalize(ew::Vec3(cos(theta), 0.0f, sin(theta)));

			vertex.uv.x = (cos(theta) + 1) * 0.5;
			vertex.uv.y = 1;

			mesh.vertices.push_back(vertex);
		}

		//Bottom Ring Sides
		for (int i = 0; i <= numSegments; i++) {
			ew::Vertex vertex;
			float theta;
			theta = (float)i * thetaStep;

			vertex.pos.x = cos(theta) * radius;
			vertex.pos.z = sin(theta) * radius;
			vertex.pos.y = bottomY;

			vertex.normal = ew::Normalize(ew::Vec3(cos(theta), 0.0f, sin(theta)));

			vertex.uv.x = (cos(theta) + 1) * 0.5;
			vertex.uv.y = 0;

			mesh.vertices.push_back(vertex);
		}

		//Bottom Ring Cap
		for (int i = 0; i <= numSegments; i++) {
			ew::Vertex vertex;
			float theta;
			theta = (float)i * thetaStep;

			vertex.pos.x = cos(theta) * radius;
			vertex.pos.z = sin(theta) * radius;
			vertex.pos.y = bottomY;

			vertex.normal = ew::Vec3(0.0f, -1.0f, 0.0f);

			vertex.uv.x = (cos(theta) + 1) * 0.5;
			vertex.uv.y = (sin(theta) + 1) * 0.5;

			mesh.vertices.push_back(vertex);
		}

		//Bottom Center Vertex
		ew::Vertex bottomVertex;
		bottomVertex.pos = ew::Vec3(0.0f, bottomY, 0.0f);
		bottomVertex.normal = ew::Vec3(0.0f, -1.0f, 0.0f);
		bottomVertex.uv = ew::Vec2(0.5f, 0.5f);
		mesh.vertices.push_back(bottomVertex);

		return mesh;
	}

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