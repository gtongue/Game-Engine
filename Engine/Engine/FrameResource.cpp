#include "FrameResource.h"



FrameResource::FrameResource(ID3D12Device* device, UINT passCount)
{
	ThrowIfFailed(device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(CmdListAlloc.GetAddressOf())));

	//  FrameCB = std::make_unique<UploadBuffer<FrameConstants>>(device, 1, true);

	UINT maxObjects = 9999;
	UINT maxMaterials = 9999;

	PassCB = std::make_unique<UploadBuffer<PassConstants>>(device, passCount, true);
	//MaterialCB = std::make_unique<UploadBuffer<MaterialConstants>>(device, materialCount, true);
	//ObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(device, objectCount, true);

	MaterialCB = std::make_unique<UploadBuffer<MaterialConstants>>(device, maxMaterials, true);
	ObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(device, maxObjects, true);
}


FrameResource::~FrameResource()
{
}
