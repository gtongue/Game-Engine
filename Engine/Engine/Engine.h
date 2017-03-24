#pragma once

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "D3DUtils.h"
#include "Loader.h"
#include "Structures.h"
#include <codecvt>
#include <locale.h>
#include <iostream>
#include "Camera.h"
#include "GeometryGenerator.h"

class Engine
{
protected:

	Engine(const Engine& rhs) = delete;
	Engine& operator=(const Engine& rhs) = delete;
	virtual void OnMouseDown(WPARAM btnState, int x, int y);
	virtual void OnMouseUp(WPARAM btnState, int x, int y);
	virtual void OnMouseMove(WPARAM btnState, int x, int y);
	virtual void OnKeyboardInput();

	POINT mLastMousePos;


public:
	Engine(HINSTANCE hInstance);
	~Engine();
	static Engine* GetApp();
	int Run();
	bool Initialize();
	float Engine::AspectRatio()const
	{
		return static_cast<float>(mClientWidth) / mClientHeight;
	}

	LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	bool InitMainWindow();
	bool InitDirect3D();
	void OnResize();
	void CreateCommandObjects();
	void CreateSwapChain();
	void CreateRtvAndDsvDescriptorHeaps();
	void FlushCommandQueue();
	void CalculateFrameStats();

	void Pick(int x, int y);

	
	bool InitNewScene(Scene& scene);
	bool ResetScene();

	void BuildRootSignature(Scene& scene);
	void BuildDescriptorHeaps(Scene& scene);
	void BuildShadersAndInputLayout(Scene& scene);
	void BuildFrameResources(Scene& scene);
	void BuildPSOs(Scene& scene);
	void BuildWorldObjects(Scene& scene);
	std::unique_ptr<RenderItem> BuildRenderItem(ObjectSkeleton skel, int ObjNumber);
	void BuildGeometry(Scene& scene);
	void BuildMaterials(Scene& scene);
	void BuildTextures(Scene& scene);

	void AddRenderItem();

	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 7> GetStaticSamplers();
	virtual void Update();
	virtual void UpdateCamera();
	void UpdateObjectCBs();
	void UpdateMainPassCB();
	void UpdateMaterialPassCBs();
	virtual void Draw();
	void DrawWorldItems(ID3D12GraphicsCommandList* cmdList);

	ID3D12Resource* CurrentBackBuffer()const;
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView()const;
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView()const;


	static Engine* mApp;

	HINSTANCE mhAppInst = nullptr; // application instance handle
	HWND      mhMainWnd = nullptr; // main window handle
	bool      mAppPaused = false;  // is the application paused?
	bool      mMinimized = false;  // is the application minimized?
	bool      mMaximized = false;  // is the application maximized?
	bool      mResizing = false;   // are the resize bars being dragged?
	bool      mFullscreenState = false;// fullscreen enabled


	bool      m4xMsaaState = false;    // 4X MSAA enabled
	UINT      m4xMsaaQuality = 0;      // quality level of 4X MSAA
	UINT64	  tempNum = 0;
			
	Camera mCamera;

	std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;
	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;
	std::unordered_map<std::string, std::unique_ptr<Material>> mMaterials;
	std::unordered_map<std::string, std::unique_ptr<Texture>> mTextures;

	// List of all the render items.
/*	std::vector<std::unique_ptr<RenderItem>> mAllRitems;
	std::vector<std::unique_ptr<PhysicsObject>> mAllPhysicsObjects;
	std::vector<std::unique_ptr<Object>> mAllObjects; //TODO 
	std::vector<RenderItem*> mOpaqueRitems;
	std::vector<Object*> mCurrentObjects;
	std::vector<RenderItem*> mBoundingBoxes;

	Object* mPickedObject = nullptr;*/
	std::vector<std::unique_ptr<WorldObject>> mWorldObjects;

	std::unique_ptr<Scene> mScene = nullptr;

	// Render items divided by PSO. Right now Only opaque

	const int mNumFrameResources = 3;
	std::vector<std::unique_ptr<FrameResource>> mFrameResources;
	FrameResource* mCurrFrameResource = nullptr;
	int mCurrFrameResourceIndex = 0;

	//std::vector<D3D12_INPUT_ELEMENT_DESC> mSkinnedInputLayout;  TODO when implement animation

	ComPtr<IDXGIFactory4> mdxgiFactory;
	ComPtr<IDXGISwapChain> mSwapChain;
	ComPtr<ID3D12Device> md3dDevice;
	ComPtr<ID3D12RootSignature> mRootSignature = nullptr;

	ComPtr<ID3D12Fence> mFence;
	UINT64 mCurrentFence = 0;

	ComPtr<ID3D12CommandQueue> mCommandQueue;
	ComPtr<ID3D12CommandAllocator> mDirectCmdListAlloc;
	ComPtr<ID3D12GraphicsCommandList> mCommandList;

	static const int SwapChainBufferCount = 2;
	int mCurrBackBuffer = 0;
	ComPtr<ID3D12Resource> mSwapChainBuffer[SwapChainBufferCount];
	ComPtr<ID3D12Resource> mDepthStencilBuffer;

	ComPtr<ID3D12DescriptorHeap> mCbvHeap = nullptr;
	ComPtr<ID3D12DescriptorHeap> mRtvHeap = nullptr;
	ComPtr<ID3D12DescriptorHeap> mDsvHeap = nullptr;
	ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap = nullptr;
	bool temp = false;

	D3D12_VIEWPORT mScreenViewport;
	D3D12_RECT mScissorRect;

	UINT mRtvDescriptorSize = 0;
	UINT mDsvDescriptorSize = 0;
	UINT mCbvSrvUavDescriptorSize = 0;
	UINT mCbvSrvDescriptorSize = 0;
	UINT mPassCbvOffset = 0;

	bool mIsWireframe = false;
	bool mRenderBoundingBoxes = false;

	PassConstants mMainPassCB;

	XMFLOAT3 mEyePos = { 0.0f, 0.0f, 0.0f };
	XMFLOAT4X4 mView = MathUtils::Identity4x4();
	XMFLOAT4X4 mProj = MathUtils::Identity4x4();
	GameTimer mTimer;
//	PhysicsEngine mPhysicsEngine;

	float mTheta = 1.5f*XM_PI;
	float mPhi = XM_PIDIV2 - 0.1f;
	float mRadius = 50.0f;

	//SUN
	float mSunTheta = 1.25f*XM_PI;
	float mSunPhi = XM_PIDIV4;

	// Derived class should set these in derived constructor to customize starting values.
	D3D_DRIVER_TYPE md3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
	DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	int mClientWidth = 1680;
	int mClientHeight = 720;
	std::wstring mMainWndCaption = L"d3d App";
};