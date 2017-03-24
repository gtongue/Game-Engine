#include "Loader.h"



Loader::Loader()
{
}

Loader::~Loader()
{
}

void Loader::LoadModel(vector<Vertex>& vertices, vector<uint32_t>& indices, int& vertexCount, int& triangleCount, string filename)
{
	ifstream fin;
	GetModelFilename(filename, fin);
	int currOffset = 0;
	while (!fin.eof()) {
		ReadFileCounts(vertexCount, triangleCount, fin);
		ReadFileVertices(vertices, fin);
		ReadFileIndices(indices, fin, currOffset);
	}
	fin.close();
}

void Loader::LoadObj(vector<Vertex>& vertices, vector<uint32_t>& indices, int &vertexCount, int & triangleCount, string filename)
{
	ifstream fin;
	GetModelFilename(filename, fin);
	ReadOBJ(vertices, indices, fin);
	fin.close();
}

Scene Loader::LoadScene(string filename)
{
	ifstream fin;
	GetModelFilename(filename, fin);
	string line = "";
	Scene scene;
	getline(fin, line); //Name
	scene.Name = line;
	getline(fin, line);//Textures
	getline(fin, line);//Textures
	while (line[0] != '}')
	{
		scene.texNames.push_back(line);
		scene.numTextrues++;
		getline(fin, line);
	}
	getline(fin, line);//Models
	getline(fin, line);//Models
	while (line[0] != '}')
	{
		scene.modelNames.push_back(line);
		getline(fin, line);
	}
	getline(fin, line);//Materials
	getline(fin, line);//Materials
	while (line[0] != '}')
	{
		Material mat;
		float x, y, z, w;

		mat.Name = line.substr(0, line.find(','));
		line = line.substr(line.find(',')+1);
		mat.MatCBIndex = stoi(line.substr(0, line.find(',')));
		line = line.substr(line.find(',') + 1);
		mat.DiffuseSrvHeapIndex = stoi(line.substr(0, line.find(',')));
		line = line.substr(line.find(',') + 1);


		x = stof(line.substr(0, line.find(',')));
		line = line.substr(line.find(',') + 1);
		y = stof(line.substr(0, line.find(',')));
		line = line.substr(line.find(',') + 1);
		z = stof(line.substr(0, line.find(',')));
		line = line.substr(line.find(',') + 1);
		w = stof(line.substr(0, line.find(',')));
		line = line.substr(line.find(',') + 1);
		mat.DiffuseAlbedo = XMFLOAT4(x, y, z, w);

		x = stof(line.substr(0, line.find(',')));
		line = line.substr(line.find(',') + 1);
		y = stof(line.substr(0, line.find(',')));
		line = line.substr(line.find(',') + 1);
		z = stof(line.substr(0, line.find(',')));
		line = line.substr(line.find(',') + 1);
		mat.FresnelR0 = XMFLOAT3(x, y, z);
	
		mat.Roughness = stof(line);

		scene.materials.push_back(mat); //RIP MAYBE

		getline(fin, line);
	}
	getline(fin, line);//Objects
	getline(fin, line);//Objects
	while (line[0] != '}')
	{
		ObjectSkeleton obj;
		float x, y, z;

		x = stof(line.substr(0, line.find(',')));
		line = line.substr(line.find(',') + 1);
		y = stof(line.substr(0, line.find(',')));
		line = line.substr(line.find(',') + 1);
		z = stof(line.substr(0, line.find(',')));
		line = line.substr(line.find(',') + 1);
		obj.pos = XMFLOAT3(x, y, z);

		x = stof(line.substr(0, line.find(',')));
		line = line.substr(line.find(',') + 1);
		y = stof(line.substr(0, line.find(',')));
		line = line.substr(line.find(',') + 1);
		z = stof(line.substr(0, line.find(',')));
		line = line.substr(line.find(',') + 1);
		obj.scale = XMFLOAT3(x, y, z);

		x = stof(line.substr(0, line.find(',')));
		line = line.substr(line.find(',') + 1);
		y = stof(line.substr(0, line.find(',')));
		line = line.substr(line.find(',') + 1);
		z = stof(line.substr(0, line.find(',')));
		line = line.substr(line.find(',') + 1);
		obj.rotation = XMFLOAT3(x, y, z);

		obj.geoName = line.substr(0, line.find(','));
		line = line.substr(line.find(',') + 1);
		obj.matName = line.substr(0, line.find(','));
		line = line.substr(line.find(',') + 1);
		obj.drawArgs = line.substr(0, line.find(','));
		line = line.substr(line.find(',') + 1);

		PhysicsObject po;
		po.mass = stof(line.substr(0, line.find(',')));
		line = line.substr(line.find(',') + 1);

		po.restitution = stof(line.substr(0, line.find(',')));
		line = line.substr(line.find(',') + 1);

		po.friction = stof(line.substr(0, line.find(',')));
		line = line.substr(line.find(',') + 1);

		po.mass = stof(line.substr(0, line.find(',')));
		line = line.substr(line.find(',') + 1);

		po.isConstant = stoi(line.substr(0, line.find(',')));
		line = line.substr(line.find(',') + 1);

		po.isAtRest = stoi(line.substr(0, line.find(',')));
		line = line.substr(line.find(',') + 1);

		x = stof(line.substr(0, line.find(',')));
		line = line.substr(line.find(',') + 1);
		y = stof(line.substr(0, line.find(',')));
		line = line.substr(line.find(',') + 1);
		z = stof(line);
		po.velocity = XMFLOAT3(x, y, z);

		obj.po = po;

		scene.objects.push_back(obj);

		getline(fin, line);
	}
	return scene;
}

void Loader::ReadFileIndices(vector<uint32_t>& indices, ifstream& fin, int& indexOffset)
{
	string line;
	uint32_t X, Y, Z;
	int largest = indexOffset;

	while (getline(fin, line))
	{
		if (line.find("T") != string::npos || line.find("{") != string::npos) {
			continue;
		}
		else if (line.find("}") != string::npos) {
			break;
		}
		else {
			X = stoi(line.substr(0, line.find(' '))) + indexOffset;
			line = line.substr(line.find(' ') + 1);
			Y = stoi(line.substr(0, line.find(' '))) + indexOffset;
			line = line.substr(line.find(' ') + 1);
			Z = stoi(line) + indexOffset;
			if (X > largest) {
				largest = X;
			}
			if (Y > largest) {
				largest = Y;
			}
			if (Z > largest) {
				largest = Z;
			}
			indices.push_back(X);
			indices.push_back(Y);
			indices.push_back(Z);
		}
	}
	indexOffset = largest+1;
}

void Loader::ReadOBJ(vector<Vertex>& vertices, vector<uint32_t>& indices, ifstream & fin)
{
	string line;
	float x, y, z;
	int one, two, three;
	vector<XMFLOAT3> verts;
	vector<XMFLOAT3> normals;
	vector<XMFLOAT2> uvs;
	std::unordered_map<int, bool> vertex;

	//read verts
	while (line[0] != 'v')
	{
		getline(fin, line);
	}
	int numVert = 1;
	while (line[0] == 'v')
	{
		//find first number index
		int n = 0;
		while (!isdigit(line[n]) || line[n] == '-')
			n++;
		line = line.substr(n );
		x = stof(line.substr(0, line.find(' ')));
		line = line.substr(line.find(' ') + 1);
		y = stof(line.substr(0, line.find(' ')));
		line = line.substr(line.find(' ') + 1);
		z = stof(line);
		verts.push_back(XMFLOAT3(x, y, z));
		vertex[numVert] = false;
		numVert++;
		getline(fin, line);
	}
	while (line[0] != 'v')
	{
		getline(fin, line);
	}
	//read normals
	if (line[1] == 'n') {
		while (line[0] == 'v')
		{
			//find first number index
			int n = 0;
			while (!isdigit(line[n]) || line[n] == '-')
				n++;
			line = line.substr(n);
			x = stof(line.substr(0, line.find(' ')));
			line = line.substr(line.find(' ') + 1);
			y = stof(line.substr(0, line.find(' ')));
			line = line.substr(line.find(' ') + 1);
			z = stof(line);
			normals.push_back(XMFLOAT3(x, y, z));
			getline(fin, line);
		}
		while (line[0] != 'v')
		{
			getline(fin, line);
		}
		//read uvs
		while (line[0] == 'v')
		{
			//find first number index
			int n = 0;
			while (!isdigit(line[n]) || line[n] == '-')
				n++;
			line = line.substr(n);
			x = stof(line.substr(0, line.find(' ')));
			line = line.substr(line.find(' ') + 1);
			y = stof(line);
			uvs.push_back(XMFLOAT2(x, y));
			getline(fin, line);
		}
	}
	else {
		//read uvs
		while (line[0] == 'v')
		{
			//find first number index
			int n = 0;
			while (!isdigit(line[n]) || line[n] == '-')
				n++;
			line = line.substr(n);
			x = stof(line.substr(0, line.find(' ')));
			line = line.substr(line.find(' ') + 1);
			y = stof(line);
			uvs.push_back(XMFLOAT2(x, y));
			getline(fin, line);
		}
		while (line[0] != 'v')
		{
			getline(fin, line);
		}
		while (line[0] == 'v')
		{
			//find first number index
			int n = 0;
			while (!isdigit(line[n]) || line[n] == '-')
				n++;
			line = line.substr(n);
			x = stof(line.substr(0, line.find(' ')));
			line = line.substr(line.find(' ') + 1);
			y = stof(line.substr(0, line.find(' ')));
			line = line.substr(line.find(' ') + 1);
			z = stof(line);
			normals.push_back(XMFLOAT3(x, y, z));
			getline(fin, line);
		}
	}
	//read faces
	while (line[0] != 'f')
	{
		getline(fin, line);
	}
	while (line[0] == 'f' || line[0] == 's')
	{
		if (line[0] == 's')
		{
			getline(fin, line);
			continue;
		}
		int n = 0;
		while (!isdigit(line[n]) || line[n] == '-')
			n++;
		line = line.substr(n);
		one = stoi(line.substr(0, line.find('/')));
		line = line.substr(line.find('/') + 1);
		two = stoi(line.substr(0, line.find('/')));
		line = line.substr(line.find('/') + 1);
		three = stoi(line.substr(0, line.find(' ')));
		line = line.substr(line.find(' ') + 1);
		if (vertex[one] == false)
		{
			vertex[one] = true;
			vertices.push_back(Vertex({ verts[one - 1], normals[three - 1], uvs[two - 1] }));
		}
		indices.push_back(one);

		one = stoi(line.substr(0, line.find('/')));
		line = line.substr(line.find('/') + 1);
		two = stoi(line.substr(0, line.find('/')));
		line = line.substr(line.find('/') + 1);
		three = stoi(line.substr(0, line.find(' ')));
		line = line.substr(line.find(' ') + 1);
		if (vertex[one] == false)
		{
			vertex[one] = true;
			vertices.push_back(Vertex({ verts[one - 1], normals[three - 1], uvs[two - 1] }));
		}
		indices.push_back(one);

		one = stoi(line.substr(0, line.find('/')));
		line = line.substr(line.find('/') + 1);
		two = stoi(line.substr(0, line.find('/')));
		line = line.substr(line.find('/') + 1);
		three = stoi(line);
		if (vertex[one] == false)
		{
			vertex[one] = true;
			vertices.push_back(Vertex({ verts[one - 1], normals[three - 1], uvs[two - 1] }));
		}
		indices.push_back(one);

		getline(fin, line);
	}
}

void Loader::ReadFileVertices(vector<Vertex>& vertices, ifstream& fin)
{
	string line;
	float X, Y, Z;
	float nX, nY, nZ;
	float U, V;

	while (getline(fin, line))
	{
		if (line.find("V") != string::npos || line.find("{") != string::npos) {
			continue;
		}
		else if (line.find("}") != string::npos) {
			break;
		}
		else {
			X = stof(line.substr(0, line.find(' ')));
			line = line.substr(line.find(' ') + 1);
			Y = stof(line.substr(0, line.find(' ')));
			line = line.substr(line.find(' ') + 1);
			Z = stof(line.substr(0, line.find(' ')));
			line = line.substr(line.find(' ') + 1);
			nX = stof(line.substr(0, line.find(' ')));
			line = line.substr(line.find(' ') + 1);
			nY = stof(line.substr(0, line.find(' ')));
			line = line.substr(line.find(' ') + 1);
			nZ = stof(line.substr(0, line.find(' ')));
			line = line.substr(line.find(' ') + 1);
			try {
				U = stof(line.substr(0, line.find(' ')));
				line = line.substr(line.find(' ') + 1);
				V = stof(line);
				vertices.push_back(Vertex({ XMFLOAT3(X,Y,Z), XMFLOAT3(nX,nY,nZ),XMFLOAT2(U,V) }));
			}
			catch (exception e) {
				vertices.push_back(Vertex({ XMFLOAT3(X,Y,Z), XMFLOAT3(nX,nY,nZ),XMFLOAT2(0,0)}));
			}
		}
	}
}
void Loader::ReadFileCounts(int& vertexCount, int& triangleCount, ifstream& fin)
{
	string line;
	while (getline(fin, line))
	{
		string findVert = "VertexCount: ";
		string findTriangle = "TriangleCount: ";

		if (line.find(findVert) != string::npos) {
			try {
				vertexCount = std::stoi(line.substr(findVert.length()));
			}
			catch (exception e) {
				cout << "vertexCount RIP";
				return;
			}
		}
		else if (line.find(findTriangle) != string::npos) {
			try {
				triangleCount = std::stoi(line.substr(findTriangle.length()));
			}
			catch (exception e) {
				cout << "triangleCount RIP";
				return;
			}
		}
		else {
			break;
		}
	}
}

void Loader::GetModelFilename(string filename, ifstream& fin)
{
	bool done = false;
	fin.open(filename);
	if (fin.good())
	{
		// If the file exists and there are no problems then exit since we have the file name.
		done = true;
	}
	else
	{
		// If the file does not exist or there was an issue opening it then notify the user and repeat the process.
		fin.clear();
		fin.close();
	}
	return;
}