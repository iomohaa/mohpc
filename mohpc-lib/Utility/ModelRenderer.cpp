#include <Shared.h>
#include <MOHPC/Utility/ModelRenderer.h>
#include <MOHPC/Utility/Managers/SkeletorManager.h>
#include <MOHPC/Assets/Managers/ShaderManager.h>
#include <MOHPC/Assets/Formats/TIKI.h>
#include <MOHPC/Assets/Formats/Skel.h>
#include <MOHPC/Assets/Formats/Skel/Skeletor.h>
#include <MOHPC/Assets/Formats/Skel/SkeletonBones.h>

using namespace MOHPC;

MOHPC_OBJECT_DEFINITION(ModelRenderer);

ModelRenderer::ModelRenderer(const MOHPC::AssetManagerPtr& AssetManager)
	: AssetObject(AssetManager)
{
	skelBones = nullptr;
}

ModelRenderer::~ModelRenderer()
{
	ClearBonesCache();
}

void ModelRenderer::CacheBones()
{
	if (!skelBones)
	{
		const size_t numBones = boneList.NumChannels();

		skelBones = new skelBone_Base*[numBones]();

		// Load bonesTransform from all meshes
		const size_t numMeshes = meshes.size();
		for (size_t i = 0; i < numMeshes; i++)
		{
			const SkeletonPtr Skel = meshes[i].lock();
			if(Skel) {
				Skel->LoadBonesFromBuffer(&boneList, skelBones);
			}
		}

		bones.reserve(numBones);

		for (size_t i = 0; i < numBones; i++)
		{
			skelBone_Base* parent = nullptr;

			if (skelBones[i])
			{
				skelBones[i]->m_controller = nullptr;
				parent = skelBones[i]->Parent();
			}

			intptr_t parentIndex = -1;
			if (parent)
			{
				for (size_t j = 0; j < numBones; j++)
				{
					if (skelBones[j] == parent)
					{
						parentIndex = j;
						break;
					}
				}
			}

			ModelBone bone;
			bone.boneName = boneList.ChannelName(GetManager<SkeletorManager>()->GetBoneNamesTable(), i);
			bone.parentIndex = parentIndex;
			bones.AddObject(bone);
		}
	}
}

void ModelRenderer::MarkAllBonesAsDirty()
{
	if (skelBones)
	{
		const size_t numBones = boneList.NumChannels();
		for (size_t i = 0; i < numBones; i++)
		{
			if (skelBones[i])
			{
				skelBones[i]->m_isDirty = true;
			}
		}
	}
}

void ModelRenderer::ClearBonesCache()
{
	if (skelBones)
	{
		const size_t numBones = boneList.NumChannels();
		for (size_t i = 0; i < numBones; i++)
		{
			if (skelBones[i])
			{
				delete skelBones[i];
			}
		}
		delete[] skelBones;
		skelBones = nullptr;
	}
}

void ModelRenderer::AddModel(const TIKI* Tiki)
{
	const size_t numMeshes = Tiki->GetNumMeshes();
	if (numMeshes)
	{
		ClearBonesCache();

		// Add all channels from the TIKI
		const SkeletonChannelList* modelBoneList = Tiki->GetBoneList();
		const size_t numBones = modelBoneList->NumChannels();
		for (size_t i = 0; i < numBones; i++)
		{
			intptr_t channelNum = modelBoneList->GetGlobalFromLocal(i);
			boneList.AddChannel(channelNum);
		}

		boneList.PackChannels();

		ShaderManagerPtr shaderMan = GetAssetManager()->GetManager<ShaderManager>();

		meshes.reserve(meshes.size() + numMeshes);
		materials.reserve(materials.size() + numMeshes);

		for (size_t i = 0; i < numMeshes; i++)
		{
			const SkeletonPtr Skel = Tiki->GetMesh(i);
			meshes.AddObject(Skel);
			LoadMorphTargetNames(Skel.get());
		}

		const size_t numSurfaces = Tiki->GetNumSurfaces();
		for (size_t i = 0; i < numSurfaces; i++)
		{
			const TIKI::TIKISurface* Surface = Tiki->GetSurface(i);

			if (!FindMaterialByName(Surface->name))
			{
				ModelSurfaceMaterial Mat;
				Mat.name = Surface->name;

				const size_t numShaders = Surface->shaders.size();
				Mat.shaders.reserve(numShaders);

				for (size_t j = 0; j < numShaders; j++)
				{
					const str& shaderName = Surface->shaders[j];
					Mat.shaders.AddObject(shaderMan->GetShader(shaderName.c_str()));
				}

				materials.AddObject(Mat);
			}
		}
	}

	m_morphTargetList.PackChannels();

	if (poses[MAX_ANIM_MOVEMENTS_POSES].animation && Tiki->GetNumAnimations())
	{
		SetActionPose(Tiki->GetAnimation(0), 0, 0, 1.f);
	}
}

intptr_t ModelRenderer::AddModel(const SkeletonPtr& Skel)
{
	ClearBonesCache();

	// Add all channels from the skeleton
	const size_t numBones = Skel->GetNumBones();
	for (size_t i = 0; i < numBones; i++)
	{
		intptr_t channelNum = Skel->GetBone(i)->channel;
		boneList.AddChannel(channelNum);
	}

	intptr_t index = meshes.size();
	meshes.AddObject(Skel);
	// Add a material for the mesh
	materials.AddObject(ModelSurfaceMaterial());
	return index;
}

void ModelRenderer::ClearPoses()
{
	const size_t numPoses = sizeof(poses) / sizeof(poses[0]);
	for (size_t i = 0; i < numPoses; i++)
	{
		Pose* pose = &poses[i];
		pose->animation.reset();
		pose->frameNum = 0;
		pose->weight = 0.f;
	}
}

void ModelRenderer::SetMovementPose(const ConstSkeletonAnimationPtr& Animation, uint32_t PoseIndex, uintptr_t FrameNumber, float Weight)
{
	if (PoseIndex < 0 || PoseIndex > MAX_ANIM_MOVEMENTS_POSES)
	{
		return;
	}

	Pose* pose = &poses[PoseIndex];
	pose->animation = Animation;
	pose->frameNum = FrameNumber;
	pose->weight = Weight;
}

void ModelRenderer::SetActionPose(const ConstSkeletonAnimationPtr& Animation, uint32_t PoseIndex, uintptr_t FrameNumber, float Weight)
{
	if (PoseIndex < 0 || PoseIndex > MAX_ANIM_ACTIONS_POSES)
	{
		return;
	}

	Pose* pose = &poses[PoseIndex + MAX_ANIM_MOVEMENTS_POSES];
	pose->animation = Animation;
	pose->frameNum = FrameNumber;
	pose->weight = Weight;
}

void ModelRenderer::AdvancePosesFrame()
{
	for (size_t i = 0; i < MAX_ANIM_POSES; i++)
	{
		Pose* pose = &poses[i];
		if (pose->animation)
		{
			if (pose->frameNum < pose->animation->GetNumFrames())
			{
				pose->frameNum++;
			}
		}
	}
}

void ModelRenderer::BuildBonesTransform()
{
	skelAnimStoreFrameList_c frameList;
	frameList.numActionFrames = 0;
	frameList.numMovementFrames = 0;
	frameList.actionWeight = 1.f;

	delta = vec_zero;

	for (size_t i = 0; i < MAX_ANIM_MOVEMENTS_POSES; i++)
	{
		if (SetPose(i, frameList)) {
			frameList.numMovementFrames++;
		}
	}

	for (size_t i = 0; i < MAX_ANIM_ACTIONS_POSES; i++)
	{
		if (SetPose(MAX_ANIM_MOVEMENTS_POSES + i, frameList)) {
			frameList.numActionFrames++;
		}
	}

	// Retrieve all skeleton bones to be later used
	CacheBones();

	// Must guarantee that all bones transform are calculated accordingly
	MarkAllBonesAsDirty();

	bonesTransform.FreeObjectList();

	const size_t numBones = boneList.NumChannels();
	bonesTransform.Resize(numBones);

	// Calculate all bones transform
	for (size_t i = 0; i < numBones; i++)
	{
		// Get the matrix bone transform
		SkelMat4 transform = skelBones[i]->GetTransform(&frameList);

		ModelBoneTransform* boneTransform = new(bonesTransform) ModelBoneTransform();
		boneTransform->baseBone = &bones[i];
		boneTransform->offset = transform[3];
		Matrix4_3Copy(transform.val, boneTransform->matrix);
		MatToQuat(boneTransform->matrix, boneTransform->quat);
	}
}

void ModelRenderer::BuildRenderData()
{
	if (!meshes.size()) {
		return;
	}

	size_t numSurfaces = 0;
	//size_t numTotalMorphs = 0;
	const size_t numMeshes = meshes.size();
	for (size_t i = 0; i < numMeshes; i++)
	{
		const SkeletonPtr Skel = meshes[i].lock();
		if(Skel) {
			numSurfaces += Skel->GetNumSurfaces();
		}
		//numTotalMorphs += Skel->GetNumMorphTargets();
	}

	surfaces.clear();
	surfaces.reserve(numSurfaces);

	// This code was used to calculate morph, but it doesn't seem to work
	/*
	std::vector<float> morphData;
	morphData.insert(morphData.begin(), numTotalMorphs, 0);

	size_t morphDataNum = 0;

	for (size_t i = 0; i < numMeshes; i++)
	{
		const Skeleton* Skel = meshes[i];
		
		for (uint32_t blendNum = 0; blendNum < MAX_ANIM_MOVEMENTS_POSES; blendNum++)
		{
			const Pose& pose = poses[blendNum];
			const float weight = pose.weight;

			if (weight > 0.001)
			{
				for (uint32_t modelChannelNum = 0; modelChannelNum < m_morphTargetList.NumChannels(); modelChannelNum++)
				{
					uint32_t animChannelNum = m_morphTargetList.GlobalChannel(modelChannelNum);
					animChannelNum = m_morphTargetList.GetLocalFromGlobal(animChannelNum);

					if (animChannelNum >= 0)
					{
						const vec4_t* channelData = DecodeFrameValue(pose.animation->GetAryChannel(animChannelNum), pose.frameNum);
						morphData[modelChannelNum] += (int)((*channelData)[0] * weight);
					}
				}
			}
		}

		for (uint32_t blendNum = 32; blendNum < MAX_ANIM_ACTIONS_POSES + 32; blendNum++)
		{
			const Pose& pose = poses[blendNum];
			const float weight = pose.weight;

			if (weight > 0.001)
			{
				for (uint32_t modelChannelNum = 0; modelChannelNum < m_morphTargetList.NumChannels(); modelChannelNum++)
				{
					uint32_t animChannelNum = m_morphTargetList.GlobalChannel(modelChannelNum);
					animChannelNum = m_morphTargetList.GetLocalFromGlobal(animChannelNum);

					if (animChannelNum >= 0)
					{
						const vec4_t* channelData = DecodeFrameValue(pose.animation->GetAryChannel(animChannelNum), pose.frameNum);
						morphData[modelChannelNum] += (int)((*channelData)[0] * weight);
					}
				}
			}
		}
	}
	*/

	size_t materialIdx = 0;

	// Calculate vertices and weights
	for (size_t meshIdx = 0; meshIdx < numMeshes; meshIdx++)
	{
		const SkeletonPtr Skel = meshes[meshIdx].lock();
		if (!Skel) {
			continue;
		}

		const size_t numMeshSurfaces = Skel->GetNumSurfaces();
		for (size_t surfIdx = 0; surfIdx < numMeshSurfaces; surfIdx++)
		{
			const Skeleton::Surface* skelSurface = Skel->GetSurface(surfIdx);

			const size_t numVertices = skelSurface->Vertices.size();
			const size_t numIndexes = skelSurface->Triangles.size();

			ModelSurface Surface;
			Surface.material = FindMaterialByName(skelSurface->name);
			Surface.vertices.reserve(numVertices);
			Surface.indexes.reserve(numIndexes);

			for (size_t vertIdx = 0; vertIdx < numVertices; vertIdx++)
			{
				const Skeleton::SkeletorVertex* skelVertex = &skelSurface->Vertices[vertIdx];
				const Skeleton::SkeletorWeight* skelWeight = skelVertex->Weights.data();
				const size_t numWeights = skelVertex->Weights.size();
				const size_t numMorphs = skelVertex->Morphs.size();
				intptr_t boneNum;
				const ModelBoneTransform* bone;

				ModelVertice Vertice;
				Vertice.weights.reserve(numWeights);
				Vertice.morphs.reserve(numMorphs);

				Vertice.st[0] = skelVertex->textureCoords[0];
				Vertice.st[1] = skelVertex->textureCoords[1];

				intptr_t channelNum = Skel->GetBone(skelWeight->boneIndex)->channel;
				boneNum = boneList.GetLocalFromGlobal(channelNum);
				bone = &bonesTransform[boneNum];

				SkelVertGetNormal(skelVertex, bone, Vertice.normal);

				if (numMorphs > 0)
				{
					Vector totalMorph;
					const Skeleton::SkeletorMorph* skelMorph = skelVertex->Morphs.data();

					/*
					for (size_t morphNum = 0; morphNum < numMorphs; morphNum++)
					{
						morphcache = &morphs[morph->morphIndex];

						if (*morphcache)
						{
							SkelMorphGetXyz(morph, morphcache, totalmorph);
						}

						morph++;
					}
					*/

					for (size_t morphNum = 0; morphNum < numMorphs; morphNum++, skelMorph++)
					{
						ModelMorph morph;
						morph.morphIndex = skelMorph->morphIndex;
						morph.offset = skelMorph->offset;

						Vertice.morphs.AddObject(morph);
					}

					for (size_t weightNum = 0; weightNum < numWeights; weightNum++, skelWeight++)
					{
						channelNum = Skel->GetBone(skelWeight->boneIndex)->channel;
						boneNum = boneList.GetLocalFromGlobal(channelNum);
						bone = &bonesTransform[boneNum];

						if (!weightNum) {
							SkelWeightMorphGetXyz(skelWeight, bone, totalMorph, Vertice.xyz);
						}
						else {
							SkelWeightGetXyz(skelWeight, bone, Vertice.xyz);
						}

						ModelWeight weight;
						weight.boneIndex = boneNum;
						weight.boneWeight = skelWeight->boneWeight;
						weight.offset = skelWeight->offset;

						Vertice.weights.AddObject(weight);
					}
				}
				else
				{
					for (size_t weightNum = 0; weightNum < numWeights; weightNum++, skelWeight++)
					{
						channelNum = Skel->GetBone(skelWeight->boneIndex)->channel;
						boneNum = boneList.GetLocalFromGlobal(channelNum);
						bone = &bonesTransform[boneNum];

						SkelWeightGetXyz(skelWeight, bone, Vertice.xyz);

						ModelWeight weight;
						weight.boneIndex = (int32_t)boneNum;
						weight.boneWeight = skelWeight->boneWeight;
						weight.offset = skelWeight->offset;

						Vertice.weights.AddObject(weight);
					}
				}

				Surface.vertices.AddObject(Vertice);
			}

			for (size_t indiceIdx = 0; indiceIdx < numIndexes; indiceIdx++)
			{
				Surface.indexes.AddObject(skelSurface->Triangles[indiceIdx]);
			}

			surfaces.AddObject(Surface);
		}
	}
}

const SkeletonChannelList* ModelRenderer::GetBonesList() const
{
	return &boneList;
}

size_t ModelRenderer::GetNumBones() const
{
	return bones.size();
}

const ModelBone* ModelRenderer::GetBone(size_t index) const
{
	if (index >= 0 && index < bones.size())
	{
		return &bones[index];
	}
	return nullptr;
}

size_t ModelRenderer::FindBoneIndex(const char* boneName) const
{
	size_t numBones = bones.size();
	for (size_t i = 0; i < numBones; i++)
	{
		if (!stricmp(bones[i].boneName, boneName))
		{
			return i;
		}
	}
	return -1;
}

size_t ModelRenderer::GetNumBonesTransform() const
{
	return bonesTransform.size();
}

const ModelBoneTransform* ModelRenderer::GetBoneTransform(size_t index) const
{
	if (index >= 0 && index < bonesTransform.size())
	{
		return &bonesTransform[index];
	}
	return nullptr;
}

size_t ModelRenderer::GetNumMaterials() const
{
	return materials.size();
}

const ModelSurfaceMaterial* ModelRenderer::GetMaterial(size_t index) const
{
	if (index >= 0 && index < materials.size())
	{
		return &materials[index];
	}
	return nullptr;
}


size_t ModelRenderer::GetNumSurfaces() const
{
	return surfaces.size();
}

const ModelSurface* ModelRenderer::GetSurface(size_t index) const
{
	if (index >= 0 && index < surfaces.size())
	{
		return &surfaces[index];
	}
	return nullptr;
}

void ModelRenderer::LoadMorphTargetNames(const Skeleton* skelmodel)
{
	size_t numTargets = skelmodel->GetNumMorphTargets();

	for (size_t i = 0; i < numTargets; i++)
	{
		const char* newTargetName = skelmodel->GetMorphTarget(i);

		size_t newChannel = GetAssetManager()->GetManager<SkeletorManager>()->GetChannelNamesTable()->RegisterChannel(newTargetName);
		size_t morphTargetIndex = m_morphTargetList.AddChannel(newChannel);

		/*
		if (!strncmp(newTargetName, "EYES_left", 9))
		{
			m_targetLookLeft = morphTargetIndex;
		}
		else if (!strncmp(newTargetName, "EYES_right", 10))
		{
			m_targetLookRight = morphTargetIndex;
		}
		else if (!strncmp(newTargetName, "EYES_up", 70))
		{
			m_targetLookUp = morphTargetIndex;
		}
		else if (!strncmp(newTargetName, "EYES_down", 9))
		{
			m_targetLookDown = morphTargetIndex;
		}
		else if (!strncmp(newTargetName, "EYES_blink", 9))
		{
			m_targetBlink = morphTargetIndex;
		}
		*/
	}
}

void ModelRenderer::SkelVertGetNormal(const Skeleton::SkeletorVertex *vert, const ModelBoneTransform *bone, Vector& out)
{
	out[0] = vert->normal[0] * bone->matrix[0][0] +
		vert->normal[1] * bone->matrix[1][0] +
		vert->normal[2] * bone->matrix[2][0];

	out[1] = vert->normal[0] * bone->matrix[0][1] +
		vert->normal[1] * bone->matrix[1][1] +
		vert->normal[2] * bone->matrix[2][1];

	out[2] = vert->normal[0] * bone->matrix[0][2] +
		vert->normal[1] * bone->matrix[1][2] +
		vert->normal[2] * bone->matrix[2][2];
}

void ModelRenderer::SkelWeightGetXyz(const Skeleton::SkeletorWeight *weight, const ModelBoneTransform *bone, Vector& out)
{
	out[0] += ((weight->offset[0] * bone->matrix[0][0] +
		weight->offset[1] * bone->matrix[1][0] +
		weight->offset[2] * bone->matrix[2][0]) +
		bone->offset[0]) * weight->boneWeight;

	out[1] += ((weight->offset[0] * bone->matrix[0][1] +
		weight->offset[1] * bone->matrix[1][1] +
		weight->offset[2] * bone->matrix[2][1]) +
		bone->offset[1]) * weight->boneWeight;

	out[2] += ((weight->offset[0] * bone->matrix[0][2] +
		weight->offset[1] * bone->matrix[1][2] +
		weight->offset[2] * bone->matrix[2][2]) +
		bone->offset[2]) * weight->boneWeight;
}

void ModelRenderer::SkelWeightMorphGetXyz(const Skeleton::SkeletorWeight *weight, const ModelBoneTransform *bone, const Vector& totalmorph, Vector& out)
{
	const Vector point = totalmorph + weight->offset;

	out[0] += ((point[0] * bone->matrix[0][0] +
		point[1] * bone->matrix[1][0] +
		point[2] * bone->matrix[2][0]) +
		bone->offset[0]) * weight->boneWeight;

	out[1] += ((point[0] * bone->matrix[0][1] +
		point[1] * bone->matrix[1][1] +
		point[2] * bone->matrix[2][1]) +
		bone->offset[1]) * weight->boneWeight;

	out[2] += ((point[0] * bone->matrix[0][2] +
		point[1] * bone->matrix[1][2] +
		point[2] * bone->matrix[2][2]) +
		bone->offset[2]) * weight->boneWeight;
}

void ModelRenderer::SkelMorphGetXyz(const Skeleton::SkeletorMorph *morph, int *morphcache, Vector& out)
{
	out[0] += morph->offset[0] * *morphcache +
		morph->offset[1] * *morphcache +
		morph->offset[2] * *morphcache;

	out[1] += morph->offset[0] * *morphcache +
		morph->offset[1] * *morphcache +
		morph->offset[2] * *morphcache;

	out[2] += morph->offset[0] * *morphcache +
		morph->offset[1] * *morphcache +
		morph->offset[2] * *morphcache;
}

const MOHPC::Vector& MOHPC::ModelRenderer::GetDelta() const
{
	return delta;
}

bool ModelRenderer::SetPose(uintptr_t poseIndex, skelAnimStoreFrameList_c& frameList)
{
	Pose* pose = &poses[poseIndex];
	SkanBlendInfo& blendInfo = frameList.m_blendInfo[poseIndex];
	if (pose->animation)
	{
		for (size_t j = 0; j <= pose->frameNum; j++)
		{
			const SkeletonAnimation::AnimFrame* animFrame = pose->animation->GetFrame(j);
			delta += (float*)animFrame->delta;
		}

		blendInfo.pAnimationData = pose->animation;
		blendInfo.frame = pose->frameNum;
		blendInfo.weight = pose->weight;
	}
	else
	{
		blendInfo.pAnimationData.reset();
		blendInfo.frame = 0;
		blendInfo.weight = 0.f;
		// No animation
		return false;
	}

	return true;
}

const MOHPC::ModelSurfaceMaterial* MOHPC::ModelRenderer::FindMaterialByName(const str& name)
{
	for (const ModelSurfaceMaterial& material : materials)
	{
		if (!stricmp(material.name.c_str(), name.c_str()))
		{
			return &material;
		}
	}
	return nullptr;
}
