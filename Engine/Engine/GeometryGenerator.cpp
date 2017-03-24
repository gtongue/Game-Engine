#include "GeometryGenerator.h"



GeometryGenerator::GeometryGenerator()
{
}


GeometryGenerator::~GeometryGenerator()
{
}
void GeometryGenerator::GenerateGridMesh(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, uint16 width, uint16 height, float offset)
{
	for (int i = 0; i < width; ++i)
	{
		for (int j = 0; j < height; ++j)
		{
			Vertex v;
			v.Pos = XMFLOAT3(i*offset,0,j*offset);
			v.Normal = XMFLOAT3(1, .75f, -1);
			v.TexC = XMFLOAT2((float)i/width * 50, (float)j/height * 50);
			vertices.push_back(std::move(v));
		}
	}

	for (int i = 0; i < height-1; ++i)
	{
		for (int j = width*i; j < width*i + (width - 1); ++j)
		{
			indices.push_back(j);
			indices.push_back(j+1);
			indices.push_back(j+width);
			indices.push_back(j+1);
			indices.push_back(j+1+width);
			indices.push_back(j+width);
		}
	}
}

void GeometryGenerator::GenerateGridMeshPoly(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, uint16 width, uint16 height, float offset)
{
	int vertCount = 0;
	for (int i = 0; i < width-1; ++i)
	{
		for (int j = 0; j < height - 1; ++j)
		{
			Vertex v1, v2, v3, v4, v5, v6;
			v1.Pos = XMFLOAT3(i*offset, std::cos(i), j*offset);
			v1.TexC = XMFLOAT2(.5f, .5f);
			v2.Pos = XMFLOAT3((i)*offset, std::cos(i), (j + 1)*offset);
			v2.TexC = XMFLOAT2(.5f, .5f);
			v3.Pos = XMFLOAT3((i+1)*offset, std::cos(i+1), j*offset);
			v3.TexC = XMFLOAT2(.5f, .5f);

			v4.Pos = XMFLOAT3((i+1)*offset, std::cos(i+1), j*offset);
			v4.TexC = XMFLOAT2(.5f, .5f);
			v5.Pos = XMFLOAT3((i)*offset, std::cos(i), (j+1)*offset);
			v5.TexC = XMFLOAT2(.5f, .5f);
			v6.Pos = XMFLOAT3((i+1)*offset, std::cos(i+1), (j + 1)*offset);
			v6.TexC = XMFLOAT2(.5f, .5f);

			SetNormalsSame(v1, v2, v3);
			SetNormalsSame(v4, v5, v6);

			vertices.push_back(std::move(v1));
			vertices.push_back(std::move(v2));
			vertices.push_back(std::move(v3));
			vertices.push_back(std::move(v4));
			vertices.push_back(std::move(v5));
			vertices.push_back(std::move(v6));
			for (int c = 0; c < 6; ++c)
			{
				indices.push_back(vertCount);
				vertCount++;
			}
		}
	}
}

void GeometryGenerator::GenerateBoundingBoxRender(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, BoundingBox box)
{
	Vertex topFarLeft; //0
	Vertex topFarRight; //1
	Vertex topCloseLeft; //2
	Vertex topCloseRight; //3
	Vertex bottomFarLeft; //4
	Vertex bottomFarRight; //5 
	Vertex bottomCloseLeft; //6 
	Vertex bottomCloseRight; //7
	XMFLOAT3 c = box.Center;
	XMFLOAT3 e = box.Extents;
	topFarLeft.Pos = XMFLOAT3(c.x-e.x, c.y+e.y, c.z+e.z);
	topFarRight.Pos = XMFLOAT3(c.x + e.x, c.y+e.y, c.z+e.z);
	topCloseLeft.Pos = XMFLOAT3(c.x - e.x, c.y + e.y, c.z - e.z);
	topCloseRight.Pos = XMFLOAT3(c.x + e.x, c.y + e.y, c.z - e.z);
	bottomFarLeft.Pos = XMFLOAT3(c.x - e.x, c.y - e.y, c.z + e.z);
	bottomFarRight.Pos = XMFLOAT3(c.x + e.x, c.y - e.y, c.z + e.z);
	bottomCloseLeft.Pos = XMFLOAT3(c.x - e.x, c.y - e.y, c.z - e.z);
	bottomCloseRight.Pos = XMFLOAT3(c.x + e.x, c.y - e.y, c.z - e.z);
	vertices.push_back(std::move(topFarLeft));
	vertices.push_back(std::move(topFarRight));
	vertices.push_back(std::move(topCloseLeft));
	vertices.push_back(std::move(topCloseRight));
	vertices.push_back(std::move(bottomFarLeft));
	vertices.push_back(std::move(bottomFarRight));
	vertices.push_back(std::move(bottomCloseLeft));
	vertices.push_back(std::move(bottomCloseRight));
	
	indices.push_back(0); indices.push_back(3); indices.push_back(2); indices.push_back(0); indices.push_back(1); indices.push_back(3); //top
	indices.push_back(4); indices.push_back(6); indices.push_back(7); indices.push_back(4); indices.push_back(7); indices.push_back(5); //bottom
	indices.push_back(3); indices.push_back(1); indices.push_back(7); indices.push_back(1); indices.push_back(5); indices.push_back(7); //right
	indices.push_back(0); indices.push_back(2); indices.push_back(4); indices.push_back(2); indices.push_back(6); indices.push_back(4); //left
	indices.push_back(2); indices.push_back(3); indices.push_back(7); indices.push_back(2); indices.push_back(7); indices.push_back(6); //front
	indices.push_back(1); indices.push_back(0); indices.push_back(5); indices.push_back(0); indices.push_back(4); indices.push_back(5); //back
}
