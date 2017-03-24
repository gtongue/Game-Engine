#include "FBXExporter.h"
#include <fstream>


FBXExporter::FBXExporter() : mManager(NULL), mScene(NULL), mImporter(NULL)
{
	mManager = FbxManager::Create();
	if (!mManager)
	{
		std::cout << "Unable to create FBX Manager \n";
		exit(1);
	}
	else
		std::cout << "Autodesk sdk version %s\n", mManager->GetVersion();

	FbxIOSettings* IOSettings = FbxIOSettings::Create(mManager, IOSROOT);
	mManager->SetIOSettings(IOSettings);
	mScene = FbxScene::Create(mManager, "Scene");
	if (!mScene)
	{
		std::cout << "Unable to create Scene \n";
		exit(1);
	}
	mImporter = FbxImporter::Create(mManager, "");
	int lFileFormat = -1;
	mFileName = "suitcase.fbx";
	if (!mManager->GetIOPluginRegistry()->DetectReaderFileFormat(mFileName, lFileFormat))
	{
		lFileFormat = mManager->GetIOPluginRegistry()->FindReaderIDByDescription("FBX binary (*.fbx)");;
	}
	if (mImporter->Initialize(mFileName, lFileFormat) == true)
	{
		std::cout << "Loading file " << mFileName << std::endl;
		LoadScene();
		ExportSceneStart();
	}
	else {
		std::cout << "Unable to read file " << mFileName << std::endl;
		exit(1);
	}
}


FBXExporter::~FBXExporter()
{
}

bool FBXExporter::LoadScene()
{
	bool lResult = false;
	if (mImporter->Import(mScene) == true)
	{
		FbxAxisSystem SceneAxisSystem = mScene->GetGlobalSettings().GetAxisSystem();
		FbxAxisSystem OurAxisSystem(FbxAxisSystem::eDirectX);
		if(SceneAxisSystem != OurAxisSystem)
			OurAxisSystem.ConvertScene(mScene);

		FbxSystemUnit SceneSystemUnit = mScene->GetGlobalSettings().GetSystemUnit();
		if (SceneSystemUnit.GetScaleFactor() != 1.0)
		{
			FbxSystemUnit::cm.ConvertScene(mScene);
		}
		FbxGeometryConverter lGeomConverter(mManager);
		lGeomConverter.Triangulate(mScene, true);
		
		LoadCacheRecursive(mScene->GetRootNode());
	}
	return true;
}

void FBXExporter::LoadCacheRecursive(FbxNode * pNode)
{
	//TODO Cache Materials
	//TODO Cache Textures
	//TODO Cache Lights
	FbxNodeAttribute* lNodeAttribute = pNode->GetNodeAttribute();
	if (lNodeAttribute)
	{
		FbxMesh* lMesh = pNode->GetMesh();
		if (lMesh && !lMesh->GetUserDataPtr())
		{
			FbxAutoPtr<MeshCache> lMeshCache(new MeshCache);
			if (lMeshCache->Initialize(lMesh))
				lMesh->SetUserDataPtr(lMeshCache.Release());
		}
	}
	const int lChildCount = pNode->GetChildCount();
	for (int lChildIndex = 0; lChildIndex < lChildCount; ++lChildIndex)
	{
		LoadCacheRecursive(pNode->GetChild(lChildIndex));
	}
}

void FBXExporter::ExportSceneStart()
{
	FbxPose* lPose = NULL;
	lPose = mScene->GetPose(0);
	FbxAMatrix lDummyGlobalPosition;
	std::string outputMeshName = "facetest.txt";
	std::ofstream meshOutput(outputMeshName);
	ExportSceneRecursive(mScene->GetRootNode(), lDummyGlobalPosition, lPose, meshOutput);
}

void FBXExporter::ExportScene(FbxNode * pNode, FbxAMatrix & pParentGlobalPosition, FbxAMatrix & pGlobalPosition, FbxPose * pPose, std::ostream& inStream)
{
	FbxNodeAttribute* lNodeAttribute = pNode->GetNodeAttribute();

	if (lNodeAttribute)
	{
		if (lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh)
		{
			ExportMesh(pNode, pGlobalPosition, pPose, inStream);
		}
	}
}

void FBXExporter::ExportMesh(FbxNode * pNode, FbxAMatrix & pGlobalPosition, FbxPose * pPose, std::ostream& inStream)
{
	FbxMesh* lMesh = pNode->GetMesh();
	const int lVertexCount = lMesh->GetControlPointsCount();
	if (lVertexCount == 0)
		return;
	const MeshCache * lMeshCache = static_cast<const MeshCache *>(lMesh->GetUserDataPtr());
	const bool lHasVertexCache = lMesh->GetDeformerCount(FbxDeformer::eVertexCache) &&
		(static_cast<FbxVertexCacheDeformer*>(lMesh->GetDeformer(0, FbxDeformer::eVertexCache)))->Active.Get();
	const bool lHasShape = lMesh->GetShapeCount() > 0;
	const bool lHasSkin = lMesh->GetDeformerCount(FbxDeformer::eSkin) > 0;
	const bool lHasDeformation = lHasVertexCache || lHasShape || lHasSkin;
	//APPLY TRANSFORMATIONS TODO
	FbxVector4 fbxTranslation = pGlobalPosition.GetT();
	FbxVector4 fbxRotation = pGlobalPosition.GetR();
	FbxVector4 fbxScale = pGlobalPosition.GetS();
	FbxQuaternion fbxQuaternion = pGlobalPosition.GetQ();

	XMVECTOR translation = XMLoadFloat3(&XMFLOAT3(
		static_cast<float>(fbxTranslation.mData[0]),
		static_cast<float>(fbxTranslation.mData[1]),
		static_cast<float>(fbxTranslation.mData[2])));
	XMVECTOR scale = XMLoadFloat3(&XMFLOAT3(
		static_cast<float>(fbxScale.mData[0]),
		static_cast<float>(fbxScale.mData[1]),
		static_cast<float>(fbxScale.mData[2])));;

	XMVECTOR quaternion = XMLoadFloat4(&XMFLOAT4(
		static_cast<float>(fbxQuaternion.mData[0]),
		static_cast<float>(fbxQuaternion.mData[1]),
		static_cast<float>(fbxQuaternion.mData[2]),
		static_cast<float>(fbxQuaternion.mData[3])));
	XMVECTOR origin = XMLoadFloat3(&XMFLOAT3(0,0,0));
	XMMATRIX transform;
	
	/*XMStoreFloat4x4(&transform, XMMatrixRotationRollPitchYaw(
		static_cast<float>(rotation.mData[0]),
		static_cast<float>(rotation.mData[1]),
		static_cast<float>(rotation.mData[2]))*XMMatrixTransformation();*/
	transform = XMMatrixTransformation(origin, origin, scale, origin, quaternion, translation);
	
	if (lMeshCache)
	{
		const int lSubMeshCount = lMeshCache->GetSubMeshCount();
		//Count triangles (inneficient but good for now)
		unsigned int lTriangleCount = 0;
		for (int lIndex = 0; lIndex < lSubMeshCount; ++lIndex)
		{
			//Load Material should be done here
			lTriangleCount += lMeshCache->mSubMeshes[lIndex]->TriangleCount;
		}
		inStream << "VertexCount: 1" << std::endl;
		inStream << "TriangleCount: 1" << std::endl;
		inStream << "VertexList (pos, normal)" << std::endl;
		inStream << "{" << std::endl;
		for (int lIndex = 0; lIndex < lMeshCache->mPolygonVertexCount; ++lIndex)
		{
			XMVECTOR pos = XMLoadFloat3(&XMFLOAT3(lMeshCache->mVertices[3 * lIndex], lMeshCache->mVertices[3 * lIndex+1], lMeshCache->mVertices[3 * lIndex+2]));
			XMVECTOR norm = XMLoadFloat3(&XMFLOAT3(lMeshCache->mNormals[3 * lIndex], lMeshCache->mNormals[3 * lIndex + 1], lMeshCache->mNormals[3 * lIndex + 2]));
			pos = XMVector3TransformCoord(pos, transform);
			norm = XMVector3TransformNormal(norm, transform);
			XMFLOAT3 lPos, lNorm;
			XMStoreFloat3(&lPos, pos);
			XMStoreFloat3(&lNorm, norm);

			inStream << lPos.x << " " << lPos.y << " " << lPos.z << " " <<
			lNorm.x << " " << lNorm.y << " " << lNorm.z << std::endl;
		}
		inStream << "}" << std::endl;
		inStream << "TriangleList" << std::endl;
		inStream << "{" << std::endl;
		for (int lIndex = 0; lIndex < lTriangleCount*3; ++lIndex)
		{
			inStream << lMeshCache->mIndices[lIndex] << " " << lMeshCache->mIndices[lIndex+1] << " " << lMeshCache->mIndices[lIndex+2] << std::endl;
			lIndex += 2;
		}
		inStream << "}" << std::endl;
	}
}

void FBXExporter::ExportSceneRecursive(FbxNode * pNode, FbxAMatrix & pParentGlobalPosition, FbxPose * pPose, std::ostream& inStream)
{
	FbxAMatrix lGlobalPosition = GetGlobalPosition(pNode, pPose, &pParentGlobalPosition);
	if (pNode->GetNodeAttribute())
	{
		FbxAMatrix lGeometryOffset = GetGeometry(pNode);
		FbxAMatrix lGlobalOffPosition = lGlobalPosition * lGeometryOffset;
		ExportScene(pNode, pParentGlobalPosition, lGlobalOffPosition, pPose, inStream);
	}
	const int lChildCount = pNode->GetChildCount();
	for (int lChildIndex = 0; lChildIndex < lChildCount; ++lChildIndex)
	{
		ExportSceneRecursive(pNode->GetChild(lChildIndex), lGlobalPosition, pPose, inStream);
	}
}

FbxAMatrix FBXExporter::GetGlobalPosition(FbxNode * pNode, FbxPose * pPose, FbxAMatrix * pParentGlobalPosition)
{
	FbxAMatrix lGlobalPosition;
	bool        lPositionFound = false;
	if (pPose)
	{
		int lNodeIndex = pPose->Find(pNode);
		if (lNodeIndex > -1)
		{
			if (pPose->IsBindPose() || !pPose->IsLocalMatrix(lNodeIndex))
			{
				lGlobalPosition = GetPoseMatrix(pPose, lNodeIndex);
			}
			else
			{
				FbxAMatrix lParentGlobalPosition;
				if (pParentGlobalPosition)
				{
					lParentGlobalPosition = *pParentGlobalPosition;
				}
				else
				{
					if (pNode->GetParent())
					{
						lParentGlobalPosition = GetGlobalPosition(pNode->GetParent(), pPose);
					}
				}

				FbxAMatrix lLocalPosition = GetPoseMatrix(pPose, lNodeIndex);
				lGlobalPosition = lParentGlobalPosition * lLocalPosition;
			}

			lPositionFound = true;
		}
	}
	if (!lPositionFound)
	{
		lGlobalPosition = pNode->EvaluateGlobalTransform();
	}

	return lGlobalPosition;
}
FbxAMatrix FBXExporter::GetPoseMatrix(FbxPose* pPose, int pNodeIndex)
{
	FbxAMatrix lPoseMatrix;
	FbxMatrix lMatrix = pPose->GetMatrix(pNodeIndex);

	memcpy((double*)lPoseMatrix, (double*)lMatrix, sizeof(lMatrix.mData));

	return lPoseMatrix;
}
FbxAMatrix FBXExporter::GetGeometry(FbxNode* pNode)
{
	const FbxVector4 lT = pNode->GetGeometricTranslation(FbxNode::eSourcePivot);
	const FbxVector4 lR = pNode->GetGeometricRotation(FbxNode::eSourcePivot);
	const FbxVector4 lS = pNode->GetGeometricScaling(FbxNode::eSourcePivot);

	return FbxAMatrix(lT, lR, lS);
}

MeshCache::MeshCache() : mHasNormal(false), mHasUV(false), mAllByControlPoint(true)
{
}

MeshCache::~MeshCache()
{
	for (int i = 0; i < mSubMeshes.GetCount(); i++)
	{
		delete mSubMeshes[i];
	}
	mSubMeshes.Clear();
}

bool MeshCache::Initialize(const FbxMesh * pMesh)
{
	if (!pMesh->GetNode())
		return false;
	const int lPolygonCount = pMesh->GetPolygonCount();

	FbxLayerElementArrayTemplate<int>* lMaterialIndice = NULL;
	FbxGeometryElement::EMappingMode lMaterialMappingMode = FbxGeometryElement::eNone;
	if (pMesh->GetElementMaterial())
	{
		lMaterialIndice = &pMesh->GetElementMaterial()->GetIndexArray();
		lMaterialMappingMode = pMesh->GetElementMaterial()->GetMappingMode();
		if (lMaterialIndice && lMaterialMappingMode == FbxGeometryElement::eByPolygon)
		{
			FBX_ASSERT(lMaterialIndice->GetCount() == lPolygonCount);
			if (lMaterialIndice->GetCount() == lPolygonCount)
			{
				for (int lPolygonIndex = 0; lPolygonIndex < lPolygonCount; ++lPolygonIndex)
				{
					const int lMaterialIndex = lMaterialIndice->GetAt(lPolygonIndex);
					if (mSubMeshes.GetCount() < lMaterialIndex + 1)
					{
						mSubMeshes.Resize(lMaterialIndex + 1);
					}
					if (mSubMeshes[lMaterialIndex] == NULL)
					{
						mSubMeshes[lMaterialIndex] = new SubMesh;
					}
					mSubMeshes[lMaterialIndex]->TriangleCount += 1;
				}
				for (int i = 0; i < mSubMeshes.GetCount(); i++)
				{
					if (mSubMeshes[i] == NULL)
						mSubMeshes[i] = new SubMesh;
				}
				const int lMaterialCount = mSubMeshes.GetCount();
				int lOffset = 0;
				for (int lIndex = 0; lIndex < lMaterialCount; ++lIndex)
				{
					mSubMeshes[lIndex]->IndexOffset = lOffset;
					lOffset += mSubMeshes[lIndex]->TriangleCount * 3;
					mSubMeshes[lIndex]->TriangleCount = 0;
				}
				FBX_ASSERT(lOffset == lPolygonCount * 3);
			}
		}
	}
	if (mSubMeshes.GetCount() == 0)
	{
		mSubMeshes.Resize(1);
		mSubMeshes[0] = new SubMesh();
	}
	mHasNormal = pMesh->GetElementNormalCount() > 0;
	mHasUV = pMesh->GetElementUVCount() > 0;
	FbxGeometryElement::EMappingMode lNormalMappingMode = FbxGeometryElement::eNone;
	FbxGeometryElement::EMappingMode lUVMappingMode = FbxGeometryElement::eNone;
	if (mHasNormal)
	{
		lNormalMappingMode = pMesh->GetElementNormal(0)->GetMappingMode();
		if (lNormalMappingMode == FbxGeometryElement::eNone)
		{
			mHasNormal = false;
		}
		if (mHasNormal && lNormalMappingMode != FbxGeometryElement::eByControlPoint)
		{
			mAllByControlPoint = false;
		}
	}
	if (mHasUV)
	{
		lUVMappingMode = pMesh->GetElementUV(0)->GetMappingMode();
		if (lUVMappingMode == FbxGeometryElement::eNone)
		{
			mHasUV = false;
		}
		if (mHasUV && lUVMappingMode != FbxGeometryElement::eByControlPoint)
		{
			mAllByControlPoint = false;
		}
	}
	mPolygonVertexCount = pMesh->GetControlPointsCount();
	if (!mAllByControlPoint)
	{
		mPolygonVertexCount = lPolygonCount * 3;
	}
	mVertices = new float[mPolygonVertexCount * 3];

	mIndices = new unsigned int[lPolygonCount * 3];

	mNormals = NULL;
	if (mHasNormal)
	{
		mNormals = new float[mPolygonVertexCount * 3];
	}
	mUVs = NULL;
	FbxStringList lUVNames;
	pMesh->GetUVSetNames(lUVNames);
	const char * lUVName = NULL;
	if (mHasUV && lUVNames.GetCount())
	{
		mUVs = new float[mPolygonVertexCount * 2];
		lUVName = lUVNames[0];
	}

	const FbxVector4 * lControlPoints = pMesh->GetControlPoints();
	FbxVector4 lCurrentVertex;
	FbxVector4 lCurrentNormal;
	FbxVector2 lCurrentUV;
	if (mAllByControlPoint)
	{
		const FbxGeometryElementNormal * lNormalElement = NULL;
		const FbxGeometryElementUV * lUVElement = NULL;
		if (mHasNormal)
		{
			lNormalElement = pMesh->GetElementNormal(0);
		}
		if (mHasUV)
		{
			lUVElement = pMesh->GetElementUV(0);
		}
		for (int lIndex = 0; lIndex < mPolygonVertexCount; ++lIndex)
		{
			// Save the vertex position.
			lCurrentVertex = lControlPoints[lIndex];
			mVertices[lIndex * 3] = static_cast<float>(lCurrentVertex[0]);
			mVertices[lIndex * 3 + 1] = static_cast<float>(lCurrentVertex[1]);
			mVertices[lIndex * 3 + 2] = static_cast<float>(lCurrentVertex[2]);

			// Save the normal.
			if (mHasNormal)
			{
				int lNormalIndex = lIndex;
				if (lNormalElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
				{
					lNormalIndex = lNormalElement->GetIndexArray().GetAt(lIndex);
				}
				lCurrentNormal = lNormalElement->GetDirectArray().GetAt(lNormalIndex);
				mNormals[lIndex * 3] = static_cast<float>(lCurrentNormal[0]);
				mNormals[lIndex * 3 + 1] = static_cast<float>(lCurrentNormal[1]);
				mNormals[lIndex * 3 + 2] = static_cast<float>(lCurrentNormal[2]);
			}

			// Save the UV.
			if (mHasUV)
			{
				int lUVIndex = lIndex;
				if (lUVElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
				{
					lUVIndex = lUVElement->GetIndexArray().GetAt(lIndex);
				}
				lCurrentUV = lUVElement->GetDirectArray().GetAt(lUVIndex);
				mUVs[lIndex * 2] = static_cast<float>(lCurrentUV[0]);
				mUVs[lIndex * 2 + 1] = static_cast<float>(lCurrentUV[1]);
			}
		}
	}
	int lVertexCount = 0;
	for (int lPolygonIndex = 0; lPolygonIndex < lPolygonCount; ++lPolygonIndex)
	{
		// The material for current face.
		int lMaterialIndex = 0;
		if (lMaterialIndice && lMaterialMappingMode == FbxGeometryElement::eByPolygon)
		{
			lMaterialIndex = lMaterialIndice->GetAt(lPolygonIndex);
		}

		// Where should I save the vertex attribute index, according to the material
		const int lIndexOffset = mSubMeshes[lMaterialIndex]->IndexOffset +
			mSubMeshes[lMaterialIndex]->TriangleCount * 3;
		for (int lVerticeIndex = 0; lVerticeIndex < 3; ++lVerticeIndex)
		{
			const int lControlPointIndex = pMesh->GetPolygonVertex(lPolygonIndex, lVerticeIndex);

			if (mAllByControlPoint)
			{
				mIndices[lIndexOffset + lVerticeIndex] = static_cast<unsigned int>(lControlPointIndex);
			}
			// Populate the array with vertex attribute, if by polygon vertex.
			else
			{
				mIndices[lIndexOffset + lVerticeIndex] = static_cast<unsigned int>(lVertexCount);

				lCurrentVertex = lControlPoints[lControlPointIndex];
				mVertices[lVertexCount * 3] = static_cast<float>(lCurrentVertex[0]);
				mVertices[lVertexCount * 3 + 1] = static_cast<float>(lCurrentVertex[1]);
				mVertices[lVertexCount * 3 + 2] = static_cast<float>(lCurrentVertex[2]);
				if (mHasNormal)
				{
					pMesh->GetPolygonVertexNormal(lPolygonIndex, lVerticeIndex, lCurrentNormal);
					mNormals[lVertexCount * 3] = static_cast<float>(lCurrentNormal[0]);
					mNormals[lVertexCount * 3 + 1] = static_cast<float>(lCurrentNormal[1]);
					mNormals[lVertexCount * 3 + 2] = static_cast<float>(lCurrentNormal[2]);
				}

				if (mHasUV)
				{
					bool lUnmappedUV;
					pMesh->GetPolygonVertexUV(lPolygonIndex, lVerticeIndex, lUVName, lCurrentUV, lUnmappedUV);
					mUVs[lVertexCount * 2] = static_cast<float>(lCurrentUV[0]);
					mUVs[lVertexCount * 2 + 1] = static_cast<float>(lCurrentUV[1]);
				}
			}
			++lVertexCount;
		}
		mSubMeshes[lMaterialIndex]->TriangleCount += 1;
	}
	return true;
}
