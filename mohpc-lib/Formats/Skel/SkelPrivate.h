#pragma once

#include <MOHPC/Formats/Skel.h>
#include <MOHPC/Formats/Skel/SkeletonNameLists.h>
#include "SkeletonAnimationFileFormat.h"
#include "SkeletonModelFileFormat.h"

namespace MOHPC
{
	class skelAnimStoreFrameList_c;

	class skelBone_Base {
	public:
		bool m_isDirty;

	protected:
		skelBone_Base *m_parent;
		SkelMat4 m_cachedValue;

	public:
		float *m_controller;

	public:
		skelBone_Base();

		SkelMat4				GetTransform(const skelAnimStoreFrameList_c *frames);
		virtual SkelMat4		GetDirtyTransform(const skelAnimStoreFrameList_c *frames);
		void					SetParent(skelBone_Base *parent);
		virtual void			SetBaseValue(const Skeleton::BoneData *boneData);
		virtual size_t			GetChannelIndex(size_t num);
		virtual skelBone_Base	*GetBoneRef(size_t num);
		static int				GetNumChannels(Skeleton::BoneType boneType);
		static int				GetNumBoneRefs(Skeleton::BoneType boneType);
		skelBone_Base			*Parent() const;
		bool					OnGround(const skelAnimStoreFrameList_c *frames, float threshold);
	};

	class skelBone_World : public skelBone_Base {
	public:
		skelBone_World();

	private:
		SkelMat4		GetDirtyTransform(const skelAnimStoreFrameList_c *frames);
		virtual void	SetBaseValue(const Skeleton::BoneData *boneData) override;
		size_t			GetChannelIndex(size_t num);
		skelBone_Base	*GetBoneRef(size_t num);
	};

	class skelBone_Zero : public skelBone_Base {
	private:
		SkelMat4			GetDirtyTransform(const skelAnimStoreFrameList_c *frames);
		virtual void		SetBaseValue(const Skeleton::BoneData *boneData) override;
		size_t				GetChannelIndex(size_t num);
		skelBone_Base		*GetBoneRef(size_t num);
	};

	class skelBone_Rotation : public skelBone_Base {
	protected:
		SkelVec3 m_baseValue;
		size_t m_quatChannel;

	protected:
		SkelMat4		GetDirtyTransform(const skelAnimStoreFrameList_c *frames);

	public:
		void			SetChannels(size_t num);
		virtual void	SetBaseValue(const Skeleton::BoneData *boneData) override;
		size_t			GetChannelIndex(size_t num);
		skelBone_Base	*GetBoneRef(size_t num);
	};

	class skelBone_PosRot : public skelBone_Base {
	protected:
		size_t m_quatChannel;
		size_t m_offsetChannel;

	public:
	protected:
		SkelMat4		GetDirtyTransform(const skelAnimStoreFrameList_c *frames);
	public:
		void			SetChannels(size_t quatChannel, size_t offsetChannel);
		virtual void	SetBaseValue(const Skeleton::BoneData *boneData) override;
		size_t			GetChannelIndex(size_t num);
		skelBone_Base	*GetBoneRef(size_t num);
	};

	class skelBone_Root : public skelBone_PosRot {
	public:
		SkelMat4	GetDirtyTransform(const skelAnimStoreFrameList_c *frames);
	};

	class skelBone_IKshoulder : public skelBone_Base {
	public:
		class skelBone_IKwrist *m_wrist;
		float m_upperLength;
		float m_lowerLength;
		SkelVec3 m_baseValue;
		float m_cosElbowAngle;
		SkelVec3 m_wristPos;
		SkelQuat m_wristAngle;

		skelBone_IKshoulder();

		SkelMat4		GetDirtyTransform(const skelAnimStoreFrameList_c *frames);
		virtual void	SetBaseValue(const Skeleton::BoneData *boneData) override;
		size_t			GetChannelIndex(size_t num);
		skelBone_Base	*GetBoneRef(size_t num);
		void			SetElbowValue(float elbowOffset);
		void			SetWristValue(float wristOffset);
		void			SetWristBone(skelBone_IKwrist *wrist);
		float			GetUpperLength();
		float			GetLowerLength();
	};

	class skelBone_IKelbow : public skelBone_Base {
	public:
		skelBone_IKshoulder *m_shoulder;

	public:
		SkelMat4		GetDirtyTransform(const skelAnimStoreFrameList_c *frames);
		void			SetBoneRefs(skelBone_IKshoulder *shoulder);
		virtual void	SetBaseValue(const Skeleton::BoneData *boneData) override;
		size_t			GetChannelIndex(size_t num);
		skelBone_Base	*GetBoneRef(size_t num);
	};

	class skelBone_IKwrist : public skelBone_Base {
	public:
		skelBone_IKshoulder *m_shoulder;
		size_t m_quatChannel;
		size_t m_offsetChannel;

	public:
		SkelMat4		GetDirtyTransform(const skelAnimStoreFrameList_c *frame);
		void			SetChannels(size_t quatChannel, size_t offsetChannel);
		void			SetBoneRefs(skelBone_IKshoulder *shoulder);
		virtual void	SetBaseValue(const Skeleton::BoneData *boneData) override;
		size_t			GetChannelIndex(size_t num);
		skelBone_Base	*GetBoneRef(size_t num);
	};

	class skelBone_AvRot : public skelBone_Base {
	public:
		SkelVec3 m_basePos;
		SkelQuat m_cachedQuat;
		skelBone_Base *m_reference1;
		skelBone_Base *m_reference2;
		float m_bone2weight;

	public:
		skelBone_AvRot();

		SkelMat4		GetDirtyTransform(const skelAnimStoreFrameList_c *frames);
		void			SetBoneRefs(skelBone_Base *ref1, skelBone_Base *ref2);
		virtual void	SetBaseValue(const Skeleton::BoneData *boneData) override;
		size_t			GetChannelIndex(size_t num);
		skelBone_Base	*GetBoneRef(size_t num);
	};

	class skelBone_HoseRot : public skelBone_Base {
	public:
		SkelVec3 m_basePos;
		SkelQuat m_cachedQuat;
		skelBone_Base *m_target;
		float m_bendRatio;
		float m_bendMax;
		float m_spinRatio;

	public:
		SkelMat4		GetDirtyTransform(const skelAnimStoreFrameList_c *frames);
		SkelMat4		GetDirtyTransform(SkelMat4& myParentTM, SkelMat4& targetTM);
		void			SetBoneRefs(skelBone_Base *ref);
		virtual void	SetBaseValue(const Skeleton::BoneData *boneData) override;
		size_t			GetChannelIndex(size_t num);
		skelBone_Base	*GetBoneRef(size_t num);
	};

	class skelBone_HoseRotBoth : public skelBone_HoseRot {
	public:
		SkelMat4		GetDirtyTransform(const skelAnimStoreFrameList_c *frames);
		virtual void	SetBaseValue(const Skeleton::BoneData *boneData) override;
	};

	class skelBone_HoseRotParent : public skelBone_HoseRot {
	public:
		SkelMat4	GetDirtyTransform(const skelAnimStoreFrameList_c *frames);
		virtual void	SetBaseValue(const Skeleton::BoneData *boneData) override;
	};

	typedef char skelChannelName_t[32];
};

#include "Skeletor.h"
#include <MOHPC/Managers/SkeletorManager.h>
