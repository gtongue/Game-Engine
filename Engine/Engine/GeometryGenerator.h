#pragma once
#include "D3DUtils.h"

class GeometryGenerator
{
public:
	GeometryGenerator();
	~GeometryGenerator();

	static void GenerateGridMesh(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, uint16 width, uint16 height, float offset);
	static void GenerateGridMeshPoly(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, uint16 width, uint16 height, float offset);
	static void GenerateBoundingBoxRender(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, BoundingBox box);
	static void SetNormalsSame(Vertex& v1, Vertex& v2, Vertex& v3)
	{
		DirectX::XMFLOAT3 U(v2.Pos.x - v1.Pos.x, v2.Pos.y - v1.Pos.y, v2.Pos.z - v1.Pos.z);
		DirectX::XMFLOAT3 V(v3.Pos.x - v1.Pos.x, v3.Pos.y - v1.Pos.y, v3.Pos.z - v1.Pos.z);
		//Cross product
		float nX = (U.y*V.z) - (U.z*V.y);
		float nY = -((U.x*V.z) - (U.z*V.x));
		float nZ = (U.x*V.y) - (U.y*V.x);
		float normalize = sqrt(pow(nX,2) + pow(nY,2) + pow(nZ,2));
		DirectX::XMFLOAT3 normal(nX/normalize, nY/ normalize, nZ/ normalize);
		v1.Normal = normal;
		v2.Normal = normal;
		v3.Normal = normal;
	}
};

