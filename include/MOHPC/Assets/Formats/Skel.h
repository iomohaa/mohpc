#pragma once

#include "Skel/SkelVec3.h"
#include "Skel/SkelVec4.h"
#include "Skel/SkelMat3.h"
#include "Skel/SkelMat4.h"
#include "Skel/SkelQuat.h"
#include "../Asset.h"
#include "../Managers/AssetManager.h"
#include "../Managers/SkeletorManager.h"
#include "../../Common/Vector.h"
#include "../../Common/str.h"
#include "../../Utility/SharedPtr.h"

#include <vector>

namespace MOHPC
{
	MOHPC_ASSETS_EXPORTS void AddToBounds(SkelVec3 *bounds, const SkelVec3 *newBounds);

	using SkeletonPtr = SharedPtr<class Skeleton>;
	using ConstSkeletonPtr = SharedPtr<const class Skeleton>;

	struct SkeletonChannelName;
	class SkeletonChannelNameTable;
	class SkeletonChannelList;

	struct File_SkelHeader;
	struct File_SkelBoneName;
	struct File_Surface;
	struct File_Weight;
	struct File_Morph;
	struct File_SKB_Vertex;
	struct File_SKD_Vertex;
	struct BoneFileData;
	struct BoneData;

	enum BoneType
	{
		SKELBONE_ROTATION,
		SKELBONE_POSROT,
		SKELBONE_IKSHOULDER,
		SKELBONE_IKELBOW,
		SKELBONE_IKWRIST,
		SKELBONE_HOSEROT,
		SKELBONE_AVROT,
		SKELBONE_ZERO,
		SKELBONE_NUMBONETYPES,
		SKELBONE_WORLD,
		SKELBONE_HOSEROTBOTH,
		SKELBONE_HOSEROTPARENT
	};

	enum HoseRotType
	{
		HRTYPE_PLAIN,
		HRTYPE_ROTATEPARENT180Y,
		HRTYPE_ROTATEBOTH180Y
	};

	struct BoneData
	{
		intptr_t channel;
		BoneType boneType;
		intptr_t parent;
		int16_t numChannels;
		int16_t numRefs;
		intptr_t channelIndex[2];
		intptr_t refIndex[2];
		vec3_t offset;
		float length;
		float weight;
		float bendRatio;
		float bendMax;
		float spinRatio;
	};

	struct SkeletorMorph
	{
		int32_t morphIndex;
		vec3_t offset;
	};

	struct SkeletorWeight
	{
		int32_t boneIndex;
		float boneWeight;
		vec3_t offset;
	};

	struct SkeletorVertex
	{
		vec3_t normal;
		float textureCoords[2];
		std::vector<SkeletorWeight> Weights;
		std::vector<SkeletorMorph> Morphs;
	};

	struct Surface
	{
		str name;
		std::vector<int32_t> Triangles;
		std::vector<SkeletorVertex> Vertices;
		std::vector<int32_t> Collapse;
		std::vector<int32_t> CollapseIndex;
	};

	struct LodControl
	{
		struct LodCurvePoint
		{
			float Pos;
			float Val;
		};

		struct LodCurvePointConstants
		{
			float Base;
			float Scale;
			float Cutoff;
		};

		float MinMetric;
		float MaxMetric;
		LodCurvePoint Curve[5];
		LodCurvePointConstants Consts[4];
	};

	class Skeleton : public Asset2
	{
		MOHPC_ASSET_OBJECT_DECLARATION(Skeleton);

		friend class TIKI;

	public:
		MOHPC_ASSETS_EXPORTS Skeleton(
			const fs::path& fileName,
			std::vector<BoneData>&& bonesData,
			std::vector<Surface>&& surfacesData,
			uint32_t* lodIndexPtr,
			std::vector<int32_t>&& boxesData,
			std::vector<str>&& morphsData
		);

		/** Return the number of surfaces. */
		MOHPC_ASSETS_EXPORTS size_t GetNumSurfaces() const;
		/**
		 * Return the surface at the specified index.
		 *
		 * @param index The index to get the surface from.
		 */
		MOHPC_ASSETS_EXPORTS const Surface* GetSurface(size_t index) const;

		/** Return the number of bones. */
		MOHPC_ASSETS_EXPORTS size_t GetNumBones() const;
		/**
		 * Return the bone at the specified index.
		 *
		 * @param index The index to get the bone from.
		 */
		MOHPC_ASSETS_EXPORTS const BoneData* GetBone(size_t index) const;

		/** Return an array of bones. */
		const std::vector<BoneData>& getBones() const;

		/** Return the number of morph targets. */
		MOHPC_ASSETS_EXPORTS size_t GetNumMorphTargets() const;
		/**
		 * Return the morph target at the specified index.
		 *
		 * @param index The index to get the morph target from.
		 */
		MOHPC_ASSETS_EXPORTS const char* GetMorphTarget(size_t index) const;

	private:
		std::vector<BoneData> bones;
		std::vector<Surface> surfaces;
		uint32_t lodIndex[10];
		std::vector<int32_t> boxes;
		LodControl* pLOD;
		std::vector<str> morphTargets;
	};

	class SkeletonReader : public AssetReader
	{
		MOHPC_ASSET_OBJECT_DECLARATION(SkeletonReader);

	public:
		using AssetType = Skeleton;

	public:
		MOHPC_ASSETS_EXPORTS SkeletonReader();
		MOHPC_ASSETS_EXPORTS ~SkeletonReader();
		MOHPC_ASSETS_EXPORTS Asset2Ptr read(const IFilePtr& file) override;

	private:
		SkeletonPtr LoadModel(const IFilePtr& file);
		void LoadCollapses(const File_SkelHeader* pHeader, size_t length, std::vector<Surface>& surfaces);
		void LoadSKBBones(const File_SkelHeader* pHeader, size_t length, std::vector<BoneData>& bones);
		void LoadSKDBones(const File_SkelHeader* pHeader, size_t length, std::vector<BoneData>& bones);
		void LoadBoxes(const IFilePtr& file, const File_SkelHeader* pHeader, size_t length, std::vector<int32_t>& boxes);
		void LoadMorphs(const IFilePtr& file, const File_SkelHeader* pHeader, size_t length, std::vector<str>& morphs);
		void LoadSKBSurfaces(const File_SkelHeader* pHeader, size_t length, std::vector<Surface>& surfaces);
		void LoadSKDSurfaces(const File_SkelHeader* pHeader, size_t length, std::vector<Surface>& surfaces);

		void CreatePosRotBoneData(const char* newBoneName, const char* newBoneParentName, BoneData* boneData);
		uint32_t CreateRotationBoneFileData(const char* newBoneName, const char* newBoneParentName, SkelVec3 basePos, BoneFileData* fileData);
		uint32_t CreatePosRotBoneFileData(char* newBoneName, char* newBoneParentName, BoneFileData* fileData);
		uint32_t CreateIKShoulderBoneFileData(const char* newBoneName, const char* newBoneParentName, SkelQuat baseOrient, SkelVec3 basePos, BoneData* boneData);
		uint32_t CreateIKElbowBoneFileData(const char* newBoneName, const char* newBoneParentName, SkelVec3 basePos, BoneData* boneData);
		uint32_t CreateIKWristBoneFileData(const char* newBoneName, const char* newBoneParentName, const char* shoulderBoneName, SkelVec3 basePos, BoneFileData* fileData);
		uint32_t CreateHoseRotBoneFileData(char* newBoneName, char* newBoneParentName, char* targetBoneName, float bendRatio, float bendMax, float spinRatio,
			HoseRotType hoseRotType, SkelVec3 basePos, BoneFileData* fileData);
		uint32_t CreateAvRotBoneFileData(char* newBoneName, char* newBoneParentName, char* baseBoneName, char* targetBoneName, float rotRatio,
			SkelVec3 basePos, BoneFileData* fileData);
	};

	/**
	 * Gather bones from this mesh.
	 *
	 * @boneList Channel list used to load specific bones.
	 * @bone List of bone to read to. The list must be the size of the number of channels.
	 */
	MOHPC_ASSETS_EXPORTS void LoadBonesFromBuffer(const SkeletorManagerPtr& skeletorManager, const SkeletonChannelList* boneList, class skelBone_Base** bone, const std::vector<BoneData>& bones);
	void LoadBoneFromBuffer(const SkeletorManagerPtr& skeletorManager, const SkeletonChannelList* boneList, const BoneData* boneData, class skelBone_Base** bone);
	void LoadBoneFromBuffer2(const SkeletorManagerPtr& skeletorManager, const BoneFileData* fileData, BoneData* boneData);

	namespace SkelError
	{
		class Base : public std::exception {};

		class BadExtension : public Base
		{
		public:
			BadExtension(const fs::path& extension);

			MOHPC_ASSETS_EXPORTS const fs::path& getExtension() const;

		public:
			const char* what() const noexcept override;

		private:
			fs::path extension;
		};

		class NoExtension : public Base
		{
		public:
			NoExtension() = default;

		public:
			const char* what() const noexcept override;
		};
	}
};

#include "Skel/SkeletonNameLists.h"
#include "SkelAnim.h"
