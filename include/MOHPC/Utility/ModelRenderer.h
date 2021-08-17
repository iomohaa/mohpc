#pragma once

#include "UtilityGlobal.h"
#include "UtilityObject.h"
#include "../Assets/Formats/Skel.h"
#include "../Assets/Managers/ShaderManager.h"

#include "../Common/str.h"

namespace MOHPC
{
	class Shader;
	class TIKI;
	class Skeleton;
	class skelAnimStoreFrameList_c;

	struct ModelSurfaceMaterial
	{
		str name;
		std::vector<ShaderPtr> shaders;
	};

	struct ModelMorph
	{
		int32_t morphIndex;
		vec3_t offset;
	};

	struct ModelWeight
	{
		uintptr_t boneIndex;
		float boneWeight;
		vec3_t offset;
	};

	struct ModelVertice
	{
		vec3_t normal;
		float st[2];
		vec3_t xyz;
		std::vector<ModelWeight> weights;
		std::vector<ModelMorph> morphs;
	};

	struct ModelSurface
	{
		const ModelSurfaceMaterial* material;
		std::vector<ModelVertice> vertices;
		std::vector<intptr_t> indexes;
	};

	struct ModelBone
	{
		const char *boneName;
		intptr_t parentIndex;
	};

	struct ModelBoneTransform
	{
		const ModelBone* baseBone;
		vec3_t offset;
		float matrix[3][3];
		quat_t quat;
	};

	/*
	 * Class used to render models with animations support
	 */
	class ModelRenderer : public AssetObject
	{
		MOHPC_UTILITY_OBJECT_DECLARATION(ModelRenderer);

	private:
		struct Pose
		{
			ConstSkeletonAnimationPtr animation;
			uintptr_t frameNum;
			float weight;

			Pose()
				: frameNum(0)
				, weight(0.f)
			{
			}
		};

	private:
		std::vector<WeakPtr<Skeleton>> meshes;
		std::vector<ModelSurfaceMaterial> materials;
		std::vector<ModelBoneTransform> bonesTransform;
		std::vector<ModelBone> bones;
		std::vector<ModelSurface> surfaces;
		skelBone_Base** skelBones;
		SkeletonChannelList boneList;
		SkeletonChannelList m_morphTargetList;
		Pose poses[MAX_ANIM_POSES];
		vec3_t delta;

	private:
		/** Requires AssetManager in order to access skeleton names table. */
		MOHPC_UTILITY_EXPORTS ModelRenderer(const MOHPC::AssetManagerPtr& AssetManager);
		MOHPC_UTILITY_EXPORTS ~ModelRenderer();

	public:
		/** Adds all skeletons to be rendered from the tiki. */
		MOHPC_UTILITY_EXPORTS void AddModel(const TIKI* Tiki);

		/** Adds a skeleton to be rendered and return the index to the skeleton (used for materials). */
		MOHPC_UTILITY_EXPORTS intptr_t AddModel(const SkeletonPtr& Skel);

		/** Clears all poses. */
		MOHPC_UTILITY_EXPORTS void ClearPoses();

		/**
		 * Sets the model's movement pose to a corresponding animation.
		 * @param Animation : The animation to set pose from
		 * @param PoseIndex : A pose index in the interval [0-31]
		 * @param FrameNumber : The frame number from the animation
		 * @param Weight : The animation weight
		 */
		MOHPC_UTILITY_EXPORTS void SetMovementPose(const ConstSkeletonAnimationPtr& Animation, uint32_t PoseIndex, uintptr_t FrameNumber = 0, float Weight = 1.f);

		/**
		 * Sets the model's action pose to a corresponding animation.
		 * @param Animation : The animation to set pose from
		 * @param PoseIndex : A pose index in the interval [0-31]
		 * @param FrameNumber : The frame number from the animation
		 * @param Weight : The animation weight
		 */
		MOHPC_UTILITY_EXPORTS void SetActionPose(const ConstSkeletonAnimationPtr& Animation, uint32_t PoseIndex, uintptr_t FrameNumber = 0, float Weight = 1.f);

		/** Increments the frame number of all poses. */
		MOHPC_UTILITY_EXPORTS void AdvancePosesFrame();

		/** Builds bonesTransform transform data. */
		MOHPC_UTILITY_EXPORTS void BuildBonesTransform();

		/** Builds the render data (BuildBonesTransform must be called first). */
		MOHPC_UTILITY_EXPORTS void BuildRenderData();

		/** Return the bones list. */
		MOHPC_UTILITY_EXPORTS const SkeletonChannelList* GetBonesList() const;

		/** Returns the model of bones this model has captured. */
		MOHPC_UTILITY_EXPORTS size_t GetNumBones() const;

		/** Returns the bone at the specified index. */
		MOHPC_UTILITY_EXPORTS const ModelBone* GetBone(size_t index) const;

		/** Finds the index of the specified bone name. */
		MOHPC_UTILITY_EXPORTS size_t FindBoneIndex(const char* boneName) const;

		/** Returns the number of calculated bones transform. */
		MOHPC_UTILITY_EXPORTS size_t GetNumBonesTransform() const;

		/** Returns the bone transform at the specified index. */
		MOHPC_UTILITY_EXPORTS const ModelBoneTransform* GetBoneTransform(size_t index) const;

		/** Returns the delta since frame 0. */
		MOHPC_UTILITY_EXPORTS const_vec3p_t GetDelta() const;

		/** Returns the number of material this model has captured. */
		MOHPC_UTILITY_EXPORTS size_t GetNumMaterials() const;

		/** Returns the material at the specified index. */
		MOHPC_UTILITY_EXPORTS const ModelSurfaceMaterial* GetMaterial(size_t index) const;

		/** Returns the number of surfaces this model has captured. */
		MOHPC_UTILITY_EXPORTS size_t GetNumSurfaces() const;

		/** Returns the surface at the specified index. */
		MOHPC_UTILITY_EXPORTS const ModelSurface* GetSurface(size_t index) const;

	private:
		bool SetPose(uintptr_t poseIndex, skelAnimStoreFrameList_c& frameList);
		void CacheBones();
		void MarkAllBonesAsDirty();
		void ClearBonesCache();
		void LoadMorphTargetNames(const Skeleton* skelmodel);

		void SkelVertGetNormal(const SkeletorVertex *vert, const ModelBoneTransform *bone, vec3r_t out);
		void SkelWeightGetXyz(const SkeletorWeight *weight, const ModelBoneTransform *bone, vec3r_t out);
		void SkelWeightMorphGetXyz(const SkeletorWeight *weight, const ModelBoneTransform *bone, const vec3r_t totalmorph, vec3r_t out);
		void SkelMorphGetXyz(const SkeletorMorph *morph, int *morphcache, vec3r_t out);
		const ModelSurfaceMaterial* FindMaterialByName(const str& name);
	};
	using ModelRendererPtr = SharedPtr<ModelRenderer>;
}
