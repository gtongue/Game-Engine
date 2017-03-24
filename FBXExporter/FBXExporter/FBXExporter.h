#pragma once
#include <fbxsdk.h>
#include <iostream>
#include <DirectXMath.h>

using namespace DirectX;
class MeshCache
{
public:
	MeshCache();
	~MeshCache();
	// Save up data into GPU buffers.
	bool Initialize(const FbxMesh * pMesh);
	// Get the count of material groups
	int GetSubMeshCount() const { return mSubMeshes.GetCount(); }
	float* mVertices;
	float* mNormals;
	unsigned int* mIndices;
	float* mUVs;
	int mPolygonVertexCount;
	struct SubMesh
	{
		SubMesh() : IndexOffset(0), TriangleCount(0) {}

		int IndexOffset;
		int TriangleCount;
	};
	FbxArray<SubMesh*> mSubMeshes;
private:
	bool mHasNormal;
	bool mHasUV;
	bool mAllByControlPoint;
};
class FBXExporter
{
public:
	FBXExporter();
	~FBXExporter();
private:

	const char * mFileName;
	FbxManager* mManager;
	FbxScene* mScene;
	FbxImporter* mImporter;
	
	bool LoadScene();
	void LoadCacheRecursive(FbxNode* pNode);
	void ExportSceneStart();
	void ExportScene(FbxNode* pNode, FbxAMatrix& pParentGlobalPosition, FbxAMatrix & pGlobalPosition, FbxPose* pPose, std::ostream& inStream);
	void ExportMesh(FbxNode* pNode,FbxAMatrix& pGlobalPosition, FbxPose* pPose, std::ostream& inStream);
	void ExportSceneRecursive(FbxNode* pNode, FbxAMatrix& pParentGlobalPosition, FbxPose* pPose, std::ostream& inStream);
	FbxAMatrix GetGlobalPosition(FbxNode* pNode, FbxPose* pPose = NULL, FbxAMatrix* pParentGlobalPosition = NULL);
	FbxAMatrix GetPoseMatrix(FbxPose* pPose, int pNodeIndex);
	FbxAMatrix GetGeometry(FbxNode* pNode);
};

