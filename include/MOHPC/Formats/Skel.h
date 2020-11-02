#pragma once

#include "../Global.h"
#include "../Asset.h"
#include "../Managers/AssetManager.h"
#include "../Vector.h"
#include "Skel/SkelVec3.h"
#include "Skel/SkelVec4.h"
#include "Skel/SkelMat3.h"
#include "Skel/SkelMat4.h"
#include "Skel/SkelQuat.h"
#include "../Common/Container.h"
#include "../Common/str.h"
#include "../Utilities/SharedPtr.h"

namespace MOHPC
{
	MOHPC_EXPORTS void AddToBounds(SkelVec3 *bounds, const SkelVec3 *newBounds);

	using SkeletonPtr = SharedPtr<class Skeleton>;
	using ConstSkeletonPtr = SharedPtr<const class Skeleton>;

	struct SkeletonChannelName;
	class SkeletonChannelNameTable;
	class SkeletonChannelList;

	class Skeleton : public Asset
	{
		CLASS_BODY(Skeleton);

		friend class TIKI;

	public:
		struct File_SkelHeader;
		struct File_SkelBoneName;
		struct File_Surface;
		struct File_Weight;
		struct File_Morph;
		struct File_SKB_Vertex;
		struct File_SKD_Vertex;
		struct BoneFileData;

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
			Vector offset;
		};

		struct SkeletorWeight
		{
			int32_t boneIndex;
			float boneWeight;
			Vector offset;
		};

		struct SkeletorVertex
		{
			Vector normal;
			float textureCoords[2];
			Container<SkeletorWeight> Weights;
			Container<SkeletorMorph> Morphs;
		};

		struct Surface
		{
			str name;
			Container<int32_t> Triangles;
			Container<SkeletorVertex> Vertices;
			Container<int32_t> Collapse;
			Container<int32_t> CollapseIndex;
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

	private:
		str name;
		Container<BoneData> Bones;
		Container<Surface> Surfaces;
		uint32_t lodIndex[10];
		Container<int32_t> Boxes;
		LodControl* pLOD;
		Container<str> MorphTargets;
	public:
		MOHPC_EXPORTS Skeleton();
		virtual bool Load() override;

		/** Return the model name. */
		MOHPC_EXPORTS const char* GetName() const;

		/** Return the number of surfaces. */
		MOHPC_EXPORTS size_t GetNumSurfaces() const;
		/**
		 * Return the surface at the specified index.
		 *
		 * @param index The index to get the surface from.
		 */
		MOHPC_EXPORTS const Surface* GetSurface(size_t index) const;

		/** Return the number of bones. */
		MOHPC_EXPORTS size_t GetNumBones() const;
		/**
		 * Return the bone at the specified index.
		 *
		 * @param index The index to get the bone from.
		 */
		MOHPC_EXPORTS const BoneData* GetBone(size_t index) const;

		/** Return the number of morph targets. */
		MOHPC_EXPORTS size_t GetNumMorphTargets() const;
		/**
		 * Return the morph target at the specified index.
		 *
		 * @param index The index to get the morph target from.
		 */
		MOHPC_EXPORTS const char* GetMorphTarget(size_t index) const;

		/**
		 * Gather bones from this mesh.
		 *
		 * @boneList Channel list used to load specific bones.
		 * @bone List of bone to read to. The list must be the size of the number of channels.
		 */
		MOHPC_EXPORTS void LoadBonesFromBuffer(const SkeletonChannelList* boneList, class skelBone_Base** bone) const;

	private:
		void LoadBoneFromBuffer(const SkeletonChannelList* boneList, const BoneData* boneData, class skelBone_Base** bone) const;
		void LoadBoneFromBuffer2(const BoneFileData* fileData, BoneData* boneData) const;
		void CreatePosRotBoneData(const char* newBoneName, const char* newBoneParentName, Skeleton::BoneData* boneData);
		uint32_t CreateRotationBoneFileData(const char* newBoneName, const char* newBoneParentName, SkelVec3 basePos, Skeleton::BoneFileData* fileData);
		uint32_t CreatePosRotBoneFileData(char* newBoneName, char* newBoneParentName, Skeleton::BoneFileData* fileData);
		uint32_t CreateIKShoulderBoneFileData(const char* newBoneName, const char* newBoneParentName, SkelQuat baseOrient, SkelVec3 basePos, Skeleton::BoneData* boneData);
		uint32_t CreateIKElbowBoneFileData(const char* newBoneName, const char* newBoneParentName, SkelVec3 basePos, Skeleton::BoneData* boneData);
		uint32_t CreateIKWristBoneFileData(const char* newBoneName, const char* newBoneParentName, const char* shoulderBoneName, SkelVec3 basePos, Skeleton::BoneFileData* fileData);
		uint32_t CreateHoseRotBoneFileData(char* newBoneName, char* newBoneParentName, char* targetBoneName, float bendRatio, float bendMax, float spinRatio,
			Skeleton::HoseRotType hoseRotType, SkelVec3 basePos, Skeleton::BoneFileData *fileData);
		uint32_t CreateAvRotBoneFileData(char *newBoneName, char *newBoneParentName, char *baseBoneName, char *targetBoneName, float rotRatio,
				SkelVec3 basePos, Skeleton::BoneFileData *fileData);

		bool LoadModel(const char* path);
		void LoadCollapses(const File_SkelHeader* pHeader, size_t length);
		void LoadSKBBones(const File_SkelHeader* pHeader, size_t length);
		void LoadSKDBones(const File_SkelHeader* pHeader, size_t length);
		void LoadBoxes(const File_SkelHeader* pHeader, size_t length);
		void LoadMorphs(const File_SkelHeader* pHeader, size_t length);
		void LoadSKBSurfaces(const File_SkelHeader* pHeader, size_t length);
		void LoadSKDSurfaces(const File_SkelHeader* pHeader, size_t length);
	};
};

#include <MOHPC/Formats/Skel/SkeletonNameLists.h>
#include <MOHPC/Formats/SkelAnim.h>
