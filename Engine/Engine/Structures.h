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
struct ColoredVertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT4 Color;
};
using uint16 = std::uint16_t;
using uint32 = std::uint32_t;
struct MeshData
{
	std::vector<Vertex> Vertices;
	std::vector<uint32> Indices32;

	std::vector<uint16>& GetIndices16()
	{
		if (mIndices16.empty())
		{
			mIndices16.resize(Indices32.size());
			for (size_t i = 0; i < Indices32.size(); ++i)
				mIndices16[i] = static_cast<uint16>(Indices32[i]);
		}

		return mIndices16;
	}

private:
	std::vector<uint16> mIndices16;
};
struct SubmeshGeometry
{
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	INT BaseVertexLocation = 0;

	// Bounding box of the geometry defined by this submesh. 
	// This is used in later chapters of the book.
	DirectX::BoundingBox Bounds;
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

	// A MeshGeometry may store multiple geometries in one vertex/index buffer.
	// Use this container to define the Submesh geometries so we can draw
	// the Submeshes individually.
	std::unordered_map<std::string, SubmeshGeometry> DrawArgs;

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
	//TODO PhysicsObject(const PhysicsObject& rhs) = delete;
	float mass = 5.0f;
	DirectX::BoundingBox bounds;
	float restitution = .65f;
	float friction = 1.0f;
	DirectX::XMFLOAT3 velocity = DirectX::XMFLOAT3(0, 0, 0);
	bool isAtRest = false;
	bool isConstant = true;
	bool applied = false;
};

struct RenderItem
{
	RenderItem(const int numFrameDirty)
	{
		//TODO mNumframesdirty
		NumFramesDirty = numFrameDirty;
	}
	//RenderItem(const RenderItem& rhs) = delete; TODODODODO

	// World matrix of the shape that describes the object's local space
	// relative to the world space, which defines the position, orientation,
	// and scale of the object in the world.
	DirectX::XMFLOAT4X4 World = MathUtils::Identity4x4();

	DirectX::XMFLOAT4X4 TexTransform = MathUtils::Identity4x4();

	// Dirty flag indicating the object data has changed and we need to update the constant buffer.
	// Because we have an object cbuffer for each FrameResource, we have to apply the
	// update to each FrameResource.  Thus, when we modify obect data we should set 
	// NumFramesDirty = gNumFrameResources so that each frame resource gets the update.
	int NumFramesDirty = 3;

	// Index into GPU constant buffer corresponding to the ObjectCB for this render item.
	UINT ObjCBIndex = -1;

	Material* Mat = nullptr;
	MeshGeometry* Geo = nullptr;
	// Primitive topology.
	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// DrawIndexedInstanced parameters.
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	int BaseVertexLocation = 0;
};
struct Object {
	Object() {

	}
	std::unique_ptr<RenderItem> ri = nullptr;
	std::unique_ptr<PhysicsObject> po = nullptr;
	void InitPhysicsBounds() {
		BoundingBox bounds = ri->Geo->Bounds;
		XMMATRIX transform = XMLoadFloat4x4(&ri->World);
		bounds.Transform(bounds, transform);

		po->bounds = std::move(bounds);
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
	int numTextrues = 0;
	std::vector<string> modelNames;
	std::vector<Material> materials;
	std::vector<ObjectSkeleton> objects;
};