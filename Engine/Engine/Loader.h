#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <basetsd.h>

#include "D3DUtils.h"

using namespace DirectX;
using namespace std;

class Loader
{
public:
	Loader();
	static void LoadModel(vector<Vertex>&, vector<uint32_t>&, string);
	static void LoadObj(vector<Vertex>&, vector<uint32_t>&, int&, int&, string);
	static Scene LoadScene(string filename);
//	static HRESULT LoadFBX(std::vector* pOutVertexVector);
	~Loader();
private:
	static void GetModelFilename(string, ifstream&);
	static void ReadFileCounts(ifstream&);
	static void ReadFileVertices(vector<Vertex>&, ifstream&);
	static void ReadFileIndices(vector<uint32_t>&, ifstream&, int&);
	static void ReadOBJ(vector<Vertex>&, vector<uint32_t>&, ifstream&);
};

