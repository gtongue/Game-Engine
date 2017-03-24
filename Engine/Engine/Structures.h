#pragma once
#include <dxgi1_4.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
#include "d3dx12.h"
#include "MathUtils.h"
#include <vector>
#include <string>
#include <windowsx.h>
#include <windows.h>
#include <wrl.h>
#include <unordered_map>
#include <memory>

using namespace DirectX;
using namespace std;

class Structures
{
public:
	Structures();
	~Structures();
};

struct Vertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT2 TexC;
};

struct MeshData
{
	std::vector<Vertex> Vertices;
	std::vector<uint32_t> Indices32;
};

struct MeshGeometry
{
	// Give it a name so we can look it up by name.
	std::string Name;
	BoundingBox Bounds;

	// System memory copies.  Use Blobs because the vertex/index format can be generic.
	// It is up to the client to cast appropriately.  
	Microsoft::WRL::ComPtr<ID3DBlob> VertexBufferCPU = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> IndexBufferCPU = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferGPU = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferGPU = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferUploader = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferUploader = nullptr;

	// Data about the buffers.
	UINT VertexByteStride = 0;
	UINT VertexBufferByteSize = 0;
	DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT;
	UINT IndexBufferByteSize = 0;
	UINT IndexCount = 0;

	D3D12_VERTEX_BUFFER_VIEW VertexBufferView()const
	{
		D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation = VertexBufferGPU->GetGPUVirtualAddress();
		vbv.StrideInBytes = VertexByteStride;
		vbv.SizeInBytes = VertexBufferByteSize;
		return vbv;
	}
	D3D12_INDEX_BUFFER_VIEW IndexBufferView()const
	{
		D3D12_INDEX_BUFFER_VIEW ibv;
		ibv.BufferLocation = IndexBufferGPU->GetGPUVirtualAddress();
		ibv.Format = IndexFormat;
		ibv.SizeInBytes = IndexBufferByteSize;
		return ibv;
	}
	// We can free this memory after we finish upload to the GPU.
	void DisposeUploaders()
	{
		VertexBufferUploader = nullptr;
		IndexBufferUploader = nullptr;
	}
};
struct Texture {
	std::string Name;
	std::wstring Filename;
	Microsoft::WRL::ComPtr<ID3D12Resource> Resource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadHeap = nullptr;
};
struct Material {
	std::string Name;
	int MatCBIndex = -1;
	int DiffuseSrvHeapIndex = -1;
	int NormalSrvHeapIndex = -1;
	int NumFramesDirty = 3; //TODO
	DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
	float Roughness = .25f;
	DirectX::XMFLOAT4X4 MatTransform = MathUtils::Identity4x4();
};

struct Light
{
	DirectX::XMFLOAT3 Strength = { 0.5f, 0.5f, 0.5f };
	float FalloffStart = 1.0f;                          // point/spot light only
	DirectX::XMFLOAT3 Direction = { 0.0f, -1.0f, 0.0f };// directional/spot light only
	float FalloffEnd = 10.0f;                           // point/spot light only
	DirectX::XMFLOAT3 Position = { 0.0f, 0.0f, 0.0f };  // point/spot light only
	float SpotPower = 64.0f;                            // spot light only
};

#define MaxLights 16

struct MaterialConstants
{
	DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
	float Roughness = 0.25f;
	// Used in texture mapping.
	DirectX::XMFLOAT4X4 MatTransform = MathUtils::Identity4x4();
};
struct PhysicsObject
{
	PhysicsObject() {}
	float Mass = 5.0f;
	DirectX::BoundingBox Bounds;
	float Restitution = .65f;
	float Friction = 1.0f;
	DirectX::XMFLOAT3 Velocity = DirectX::XMFLOAT3(0, 0, 0);
	bool isAtRest = false;
	bool isConstant = true;
};

struct RenderItem
{
	RenderItem(const int numFrameDirty)
	{
		NumFrameResources = numFrameDirty;
		NumFramesDirty = NumFrameResources;
	}
	int NumFramesDirty = 3;
	int NumFrameResources = 0;
	UINT ObjCBIndex = -1;
	Material* Mat = nullptr;
	MeshGeometry* Geo = nullptr;
	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
};

struct WorldObject {
	XMFLOAT3 pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
	XMFLOAT3 rotation = XMFLOAT3(0, 0, 0);
	XMFLOAT3 texScale = XMFLOAT3(1, 1, 1);
	DirectX::XMFLOAT4X4 World = MathUtils::Identity4x4();
	DirectX::XMFLOAT4X4 TexTransform = MathUtils::Identity4x4();
	std::unique_ptr<RenderItem> ri = nullptr;
	std::unique_ptr<PhysicsObject> po = nullptr;
	void UpdateWorld()
	{
		XMStoreFloat4x4(&World, 
		XMMatrixRotationRollPitchYaw(rotation.x*XM_PI/180, rotation.y*XM_PI / 180, rotation.z*XM_PI / 180)
		*XMMatrixScaling(scale.x,scale.y,scale.z)
		*XMMatrixTranslation(pos.x,pos.y,pos.z));
	}
	void UpdateTexTransform()
	{
		XMStoreFloat4x4(&TexTransform, XMMatrixScaling(texScale.x, texScale.y, texScale.z));
	}
};

struct ObjectSkeleton {
	XMFLOAT3 pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
	XMFLOAT3 rotation = XMFLOAT3(0, 0, 0);
	XMFLOAT3 texTrans = XMFLOAT3(1.0f,1.0f,1.0f);
	string geoName = "";
	string matName = "";
	string drawArgs = "";
	PhysicsObject po;
};

struct Scene {
	string Name;
	std::vector<string> texNames;
	std::vector<string> modelNames;
	std::vector<Material> materials;
	std::vector<ObjectSkeleton> objects;
	int numTextrues = 0;
};