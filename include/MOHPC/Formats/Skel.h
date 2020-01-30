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
#include <string>
#include <vector>
#include <memory>

namespace MOHPC
{
	MOHPC_EXPORTS void AddToBounds(SkelVec3 *bounds, const SkelVec3 *newBounds);

	typedef std::shared_ptr<class Skeleton> SkeletonPtr;
	typedef std::shared_ptr<const class Skeleton> ConstSkeletonPtr;

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
			std::vector<SkeletorWeight> Weights;
			std::vector<SkeletorMorph> Morphs;
		};

		struct Surface
		{
			std::string name;
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

	private:
		std::string name;
		std::vector<BoneData> Bones;
		std::vector<Surface> Surfaces;
		int32_t lodIndex[10];
		std::vector<int32_t> Boxes;
		LodControl* pLOD;
		std::vector<std::string> MorphTargets;

	public:
		MOHPC_EXPORTS Skeleton();
		virtual bool Load() override;

		MOHPC_EXPORTS bool LoadModel(const char *path);

		MOHPC_EXPORTS const char* GetName() const;

		MOHPC_EXPORTS size_t GetNumSurfaces() const;
		MOHPC_EXPORTS const Surface* GetSurface(size_t index) const;

		MOHPC_EXPORTS size_t GetNumBones() const;
		MOHPC_EXPORTS const BoneData* GetBone(size_t index) const;

		MOHPC_EXPORTS size_t GetNumMorphTargets() const;
		MOHPC_EXPORTS const std::string& GetMorphTarget(size_t index) const;

		MOHPC_EXPORTS void LoadBoneFromBuffer(const SkeletonChannelList *boneList, const BoneData *boneData, class skelBone_Base **bone) const;
		MOHPC_EXPORTS void LoadBonesFromBuffer(const SkeletonChannelList *boneList, class skelBone_Base **bone) const;
		MOHPC_EXPORTS void LoadBoneFromBuffer2(const BoneFileData *fileData, BoneData *boneData) const;

	private:
		int32_t CreateRotationBoneFileData(const char *newBoneName, const char *newBoneParentName, SkelVec3 basePos, Skeleton::BoneFileData *fileData);
		int32_t CreatePosRotBoneFileData(char *newBoneName, char *newBoneParentName, Skeleton::BoneFileData *fileData);
		void CreatePosRotBoneData(const char *newBoneName, const char *newBoneParentName, Skeleton::BoneData *boneData);
		int32_t CreateIKShoulderBoneFileData(const char *newBoneName, const char *newBoneParentName, SkelQuat baseOrient, SkelVec3 basePos, Skeleton::BoneData *boneData);
		int32_t CreateIKElbowBoneFileData(const char *newBoneName, const char *newBoneParentName, SkelVec3 basePos, Skeleton::BoneData *boneData);
		int32_t CreateIKWristBoneFileData(const char *newBoneName, const char *newBoneParentName, const char *shoulderBoneName, SkelVec3 basePos, Skeleton::BoneFileData *fileData);
		int32_t CreateHoseRotBoneFileData(char *newBoneName, char *newBoneParentName, char *targetBoneName, float bendRatio, float bendMax, float spinRatio,
			Skeleton::HoseRotType hoseRotType, SkelVec3 basePos, Skeleton::BoneFileData *fileData);
		int32_t CreateAvRotBoneFileData(char *newBoneName, char *newBoneParentName, char *baseBoneName, char *targetBoneName, float rotRatio,
				SkelVec3 basePos, Skeleton::BoneFileData *fileData);
	};
};

#include <MOHPC/Formats/Skel/SkeletonNameLists.h>
#include <MOHPC/Formats/SkelAnim.h>
