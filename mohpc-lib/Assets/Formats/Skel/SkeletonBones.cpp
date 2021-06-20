#include <Shared.h>
#include "SkelPrivate.h"

using namespace MOHPC;

skelBone_World::skelBone_World()
{
	m_isDirty = false;
}

void Skeleton::LoadBoneFromBuffer(const SkeletonChannelList *boneList, const Skeleton::BoneData *boneData, skelBone_Base **bone) const
{
	const char *boneName;
	skelBone_Base *parentBone;

	const intptr_t newBoneIndex = boneList->GetLocalFromGlobal(boneData->channel);

	if (!bone[newBoneIndex])
	{
		const SkeletorManagerPtr skeletorManager = GetManager<SkeletorManager>();
		const SkeletonChannelNameTable* const nameTable = skeletorManager->GetBoneNamesTable();
		boneName = nameTable->FindName(boneData->channel);

		intptr_t newBoneParent;
		if (boneData->parent < 0)
		{
			newBoneParent = -1;
		}
		else
		{
			newBoneParent = boneList->GetLocalFromGlobal(boneData->parent);
		}

		switch (boneData->boneType)
		{
		case Skeleton::SKELBONE_ZERO:
		{
			skelBone_Zero *newBone = new skelBone_Zero;
			bone[newBoneIndex] = newBone;
			break;
		}
		case Skeleton::SKELBONE_ROTATION:
		{
			skelBone_Rotation *newBone = new skelBone_Rotation;
			bone[newBoneIndex] = newBone;
			newBone->SetChannels(boneData->channelIndex[0]);
			break;
		}
		case Skeleton::SKELBONE_POSROT:
		{
			skelBone_PosRot *newBone;

			if (!str::cmpn(boneName, "Bip01", 5))
			{
				newBone = new skelBone_Root;
				bone[newBoneIndex] = newBone;
			}
			else
			{
				newBone = new skelBone_PosRot;
				bone[newBoneIndex] = newBone;
			}

			newBone->SetChannels(boneData->channelIndex[0], boneData->channelIndex[1]);
			break;
		}
		case Skeleton::SKELBONE_IKSHOULDER:
		{
			skelBone_IKshoulder *newBone = new skelBone_IKshoulder;
			bone[newBoneIndex] = newBone;
			break;
		}
		case Skeleton::SKELBONE_IKELBOW:
		{
			skelBone_IKelbow *newBone = new skelBone_IKelbow;
			bone[newBoneIndex] = newBone;
			newBone->SetBoneRefs((skelBone_IKshoulder *)bone[boneList->GetLocalFromGlobal(boneData->refIndex[0])]);
			break;
		}
		case Skeleton::SKELBONE_IKWRIST:
		{
			skelBone_IKwrist *newBone = new skelBone_IKwrist;
			skelBone_IKshoulder *shoulder;
			bone[newBoneIndex] = newBone;
			shoulder = (skelBone_IKshoulder *)bone[boneList->GetLocalFromGlobal(boneData->refIndex[0])];
			newBone->SetChannels(boneData->channelIndex[0], boneData->channelIndex[1]);
			newBone->SetBoneRefs(shoulder);
			shoulder->SetWristBone(newBone);
			break;
		}
		case Skeleton::SKELBONE_AVROT:
		{
			skelBone_AvRot *newBone = new skelBone_AvRot;
			skelBone_AvRot *ref1, *ref2;
			bone[newBoneIndex] = newBone;
			ref1 = (skelBone_AvRot *)bone[boneList->GetLocalFromGlobal(boneData->refIndex[0])];
			ref2 = (skelBone_AvRot *)bone[boneList->GetLocalFromGlobal(boneData->refIndex[1])];
			newBone->SetBoneRefs(ref1, ref2);
			break;
		}
		case Skeleton::SKELBONE_HOSEROT:
		{
			skelBone_HoseRot *newBone = new skelBone_HoseRot;
			bone[newBoneIndex] = newBone;
			newBone->SetBoneRefs(bone[boneList->GetLocalFromGlobal(boneData->refIndex[0])]);
			break;
		}
		case Skeleton::SKELBONE_HOSEROTBOTH:
		{
			skelBone_HoseRotBoth *newBone = new skelBone_HoseRotBoth;
			bone[newBoneIndex] = newBone;
			newBone->SetBoneRefs(bone[boneList->GetLocalFromGlobal(boneData->refIndex[0])]);
			break;
		}
		case Skeleton::SKELBONE_HOSEROTPARENT:
		{
			skelBone_HoseRotParent *newBone = new skelBone_HoseRotParent;
			bone[newBoneIndex] = newBone;
			newBone->SetBoneRefs(bone[boneList->GetLocalFromGlobal(boneData->refIndex[0])]);
			break;
		}
		default:
			break;
		}

		bone[newBoneIndex]->SetBaseValue(boneData);

		if (newBoneParent < 0)
		{
			parentBone = skeletorManager->GetWorldBone();
		}
		else
		{
			parentBone = bone[newBoneParent];
		}

		bone[newBoneIndex]->SetParent(parentBone);
	}
}

void Skeleton::LoadBonesFromBuffer(const SkeletonChannelList *boneList, skelBone_Base **bone) const
{
	const size_t numBones = Bones.size();
	for(size_t boneNum = 0; boneNum < numBones; boneNum++)
	{
		LoadBoneFromBuffer(boneList, &Bones[boneNum], bone);
	}
}

const float *DecodeRLEValue( const SkeletonAnimation::SkanChannelHdr *channelFrames, size_t desiredFrameNum )
{
	const SkeletonAnimation::SkanGameFrame *foundFrame = &channelFrames->ary_frames[0];

	size_t nFramesInChannel = channelFrames->ary_frames.size();
	for(size_t i = 0; i < nFramesInChannel; i++)
	{
		if( foundFrame->nFrameNum >= desiredFrameNum )
		{
			break;
		}

		foundFrame++;
	}

	if (foundFrame->nFrameNum > desiredFrameNum)
	{
		foundFrame = &channelFrames->ary_frames[foundFrame->nPrevFrameIndex];
	}

	return foundFrame->pChannelData;
}

SkelQuat skelAnimStoreFrameList_c::GetSlerpValue(size_t globalChannelNum) const
{
	SkelQuat actionQuat, movementQuat;
	SkelQuat outQuat;
	float totalWeight;
	SkelQuat *pIncomingQuat;
	float incomingWeight;
	intptr_t localChannelNum;
	float channelActionWeight;
	//float movementWeight;
	int nTotal;
	int i;
	const SkanBlendInfo *pFrame;
	float t;

	actionQuat.Set( 0, 0, 0, 0 );
	movementQuat.Set( 0, 0, 0, 0 );

	nTotal = 0;
	totalWeight = 0.0;

	if( actionWeight > 0.001 )
	{
		for( i = 0; i < numActionFrames; i++ )
		{
			pFrame = &m_blendInfo[ i + 32 ];

			localChannelNum = pFrame->pAnimationData->GetChannelList()->GetLocalFromGlobal( globalChannelNum );
			if( localChannelNum >= 0 )
			{
				incomingWeight = pFrame->weight;
				if( incomingWeight == 0.0 ) {
					continue;
				}
				pIncomingQuat = ( SkelQuat * )DecodeRLEValue(pFrame->pAnimationData->GetAryChannel(localChannelNum), pFrame->frame);
				totalWeight += incomingWeight;
				nTotal++;

				if( DotProduct4( *pIncomingQuat, actionQuat ) >= 0.0 )
				{
					actionQuat.x += pIncomingQuat->x * incomingWeight;
					actionQuat.y += pIncomingQuat->y * incomingWeight;
					actionQuat.z += pIncomingQuat->z * incomingWeight;
					actionQuat.w += pIncomingQuat->w * incomingWeight;
				}
				else
				{
					actionQuat.x -= pIncomingQuat->x * incomingWeight;
					actionQuat.y -= pIncomingQuat->y * incomingWeight;
					actionQuat.z -= pIncomingQuat->z * incomingWeight;
					actionQuat.w -= pIncomingQuat->w * incomingWeight;
				}
			}
		}
	}

	if( nTotal )
	{
		channelActionWeight = actionWeight;
		if( nTotal > 1 ) {
			t = 1.0f / actionQuat.Length();
		} else {
			t = 1.0f / totalWeight;
		}

		actionQuat.x = actionQuat.x * t;
		actionQuat.y = actionQuat.y * t;
		actionQuat.z = actionQuat.z * t;
		actionQuat.w = actionQuat.w * t;
	} else {
		channelActionWeight = 0.0;
	}

	nTotal = 0;
	totalWeight = 0.0;

	if( channelActionWeight < 0.999 )
	{
		for( i = 0; i < numMovementFrames; i++ )
		{
			pFrame = &m_blendInfo[ i ];

			localChannelNum = pFrame->pAnimationData->GetChannelList()->GetLocalFromGlobal( globalChannelNum );
			if( localChannelNum >= 0 )
			{
				incomingWeight = pFrame->weight;
				if( incomingWeight == 0.0 ) {
					continue;
				}
				pIncomingQuat = ( SkelQuat * )DecodeRLEValue(pFrame->pAnimationData->GetAryChannel(localChannelNum), pFrame->frame);
				totalWeight += incomingWeight;
				nTotal++;

				if( DotProduct4( *pIncomingQuat, movementQuat ) >= 0.0 )
				{
					movementQuat.x += pIncomingQuat->x * incomingWeight;
					movementQuat.y += pIncomingQuat->y * incomingWeight;
					movementQuat.z += pIncomingQuat->z * incomingWeight;
					movementQuat.w += pIncomingQuat->w * incomingWeight;
				}
				else
				{
					movementQuat.x -= pIncomingQuat->x * incomingWeight;
					movementQuat.y -= pIncomingQuat->y * incomingWeight;
					movementQuat.z -= pIncomingQuat->z * incomingWeight;
					movementQuat.w -= pIncomingQuat->w * incomingWeight;
				}
			}
		}
	}

	if( nTotal )
	{
		if( nTotal > 1 ) {
			t = 1.0f / movementQuat.Length();
		} else {
			t = 1.0f / totalWeight;
		}

		movementQuat.x = movementQuat.x * t;
		movementQuat.y = movementQuat.y * t;
		movementQuat.z = movementQuat.z * t;
		movementQuat.w = movementQuat.w * t;
	}
	else {
		movementQuat.w = 1.0;
	}

	if( channelActionWeight < 0.001 )
	{
		outQuat.x = movementQuat.x;
		outQuat.y = movementQuat.y;
		outQuat.z = movementQuat.z;
		outQuat.w = movementQuat.w;
		return outQuat;
	}
	else if( channelActionWeight >= 0.999 )
	{
		outQuat.x = actionQuat.x;
		outQuat.y = actionQuat.y;
		outQuat.z = actionQuat.z;
		outQuat.w = actionQuat.w;
		return outQuat;
	}

	t = 1.0f - channelActionWeight;

	if( DotProduct4( actionQuat, movementQuat ) >= 0.0 )
	{
		outQuat.x = movementQuat.x * t + actionQuat.x * channelActionWeight;
		outQuat.y = movementQuat.y * t + actionQuat.y * channelActionWeight;
		outQuat.z = movementQuat.z * t + actionQuat.z * channelActionWeight;
		outQuat.w = movementQuat.w * t + actionQuat.w * channelActionWeight;
	}
	else
	{
		outQuat.x = movementQuat.x * t - actionQuat.x * channelActionWeight;
		outQuat.y = movementQuat.y * t - actionQuat.y * channelActionWeight;
		outQuat.z = movementQuat.z * t - actionQuat.z * channelActionWeight;
		outQuat.w = movementQuat.w * t - actionQuat.w * channelActionWeight;
	}

	t = 1.0f / outQuat.Length();
	outQuat.x = outQuat.x * t;
	outQuat.y = outQuat.y * t;
	outQuat.z = outQuat.z * t;
	outQuat.w = outQuat.w * t;

	return outQuat;
}

void skelAnimStoreFrameList_c::GetLerpValue3(size_t globalChannelNum, SkelVec3 *outVec ) const
{
	float totalWeight;
	SkelVec3 incomingVec;
	float incomingWeight;
	intptr_t localChannelNum;
	vec3_t result;
	float t;
	float channelActionWeight;
	int i;
	const SkanBlendInfo *pFrame;

	totalWeight = 0.0;

	if( actionWeight > 0.001 )
	{
		VectorClear( result );

		for( i = 0; i < numActionFrames; i++ )
		{
			pFrame = &m_blendInfo[ i + 32 ];

			localChannelNum = pFrame->pAnimationData->GetChannelList()->GetLocalFromGlobal( globalChannelNum );
			if( localChannelNum >= 0 )
			{
				incomingWeight = pFrame->weight;
				incomingVec = DecodeRLEValue(pFrame->pAnimationData->GetAryChannel(localChannelNum), pFrame->frame);
				totalWeight += incomingWeight;
				result[ 0 ] += incomingVec[ 0 ] * incomingWeight;
				result[ 1 ] += incomingVec[ 1 ] * incomingWeight;
				result[ 2 ] += incomingVec[ 2 ] * incomingWeight;
			}
		}
	}

	if( totalWeight != 0.0 )
	{
		t = 1.0f / totalWeight;
		VectorScale( result, t, *outVec );
		channelActionWeight = actionWeight;
	}
	else
	{
		VectorClear( *outVec );
		channelActionWeight = 0.0;
	}

	if( channelActionWeight >= 0.999 || !numMovementFrames ) {
		return;
	}

	totalWeight = 0.0;
	VectorClear( result );

	for( i = 0; i < numMovementFrames; i++ )
	{
		pFrame = &m_blendInfo[ i ];

		localChannelNum = pFrame->pAnimationData->GetChannelList()->GetLocalFromGlobal( globalChannelNum );
		if( localChannelNum >= 0 )
		{
			incomingWeight = pFrame->weight;
			incomingVec = DecodeRLEValue(pFrame->pAnimationData->GetAryChannel(localChannelNum), pFrame->frame);
			totalWeight += incomingWeight;
			result[ 0 ] += incomingVec[ 0 ] * incomingWeight;
			result[ 1 ] += incomingVec[ 1 ] * incomingWeight;
			result[ 2 ] += incomingVec[ 2 ] * incomingWeight;
		}
	}

	if( totalWeight != 0.0 )
	{
		t = 1.0f / totalWeight * ( 1.0f - channelActionWeight );
		outVec->x = outVec->x * channelActionWeight + result[ 0 ] * t;
		outVec->y = outVec->y * channelActionWeight + result[ 1 ] * t;
		outVec->z = outVec->z * channelActionWeight + result[ 2 ] * t;
	}
}

skelBone_Base::skelBone_Base()
{
	m_parent = NULL;
	m_isDirty = true;
	m_controller = NULL;
}

int skelBone_Base::GetNumChannels(Skeleton::BoneType boneType)
{
	switch( boneType )
	{
	case Skeleton::SKELBONE_ROTATION:
		return 1;
	case Skeleton::SKELBONE_POSROT:
	case Skeleton::SKELBONE_IKWRIST:
		return 2;
	default:
		return 0;
	}
}

int skelBone_Base::GetNumBoneRefs( Skeleton::BoneType boneType )
{
	switch( boneType )
	{
	case Skeleton::SKELBONE_AVROT:
		return 2;
	case Skeleton::SKELBONE_IKELBOW:
	case Skeleton::SKELBONE_IKWRIST:
	case Skeleton::SKELBONE_HOSEROT:
	case Skeleton::SKELBONE_HOSEROTBOTH:
	case Skeleton::SKELBONE_HOSEROTPARENT:
		return 1;
	default:
		return 0;
	}
}

SkelMat4 skelBone_Base::GetTransform( const skelAnimStoreFrameList_c *frames )
{
	if( m_isDirty )
	{
		return GetDirtyTransform( frames );
	}
	else
	{
		return m_cachedValue;
	}
}

SkelMat4 skelBone_Base::GetDirtyTransform( const skelAnimStoreFrameList_c *frames )
{
	return 0;
}

void skelBone_Base::SetParent( skelBone_Base *parent )
{
	m_parent = parent;
}

void skelBone_Base::SetBaseValue( const Skeleton::BoneData *boneData )
{
}

size_t skelBone_Base::GetChannelIndex( size_t num )
{
	return 0;
}

skelBone_Base *skelBone_Base::GetBoneRef( size_t num )
{
	return NULL;
}

skelBone_Base *skelBone_Base::Parent() const
{
	return m_parent;
}

SkelMat4 skelBone_Zero::GetDirtyTransform( const skelAnimStoreFrameList_c *frames )
{
	if( m_parent )
	{
		m_cachedValue = m_parent->GetTransform( frames );
	}

	m_isDirty = false;
	return m_cachedValue;
}

void skelBone_Zero::SetBaseValue( const Skeleton::BoneData *boneData )
{
}

size_t skelBone_Zero::GetChannelIndex( size_t num )
{
	return 0;
}

skelBone_Base *skelBone_Zero::GetBoneRef( size_t num )
{
	return NULL;
}

SkelMat4 skelBone_Rotation::GetDirtyTransform( const skelAnimStoreFrameList_c *frames )
{
	SkelMat4 incomingValue;
	SkelQuat incomingQuat;

	incomingQuat = frames->GetSlerpValue( m_quatChannel );
	//incomingQuat.Normalize();
	incomingQuat.GetMat4( incomingValue );
	VecCopy( m_baseValue, incomingValue[ 3 ] );

	if( m_parent )
	{
		m_cachedValue.Multiply( incomingValue, m_parent->GetTransform( frames ) );
	}
	else
	{
		m_cachedValue = incomingValue;
	}

	if( m_controller )
	{
		SkelMat3 m;

		incomingQuat.Set( m_controller );
		incomingQuat.GetMat3( m );
		m_cachedValue.RotateBy( m );
	}

	m_isDirty = false;

	return m_cachedValue;
}


void skelBone_Rotation::SetChannels( size_t num )
{
	m_quatChannel = num;
}

void skelBone_Rotation::SetBaseValue(const Skeleton::BoneData *data)
{
	m_baseValue = data->offset;
}

size_t skelBone_Rotation::GetChannelIndex( size_t num )
{
	return m_quatChannel;
}

skelBone_Base *skelBone_Rotation::GetBoneRef(size_t bone )
{
	return 0;
}

SkelMat4 skelBone_PosRot::GetDirtyTransform( const skelAnimStoreFrameList_c *frames )
{
	SkelVec3 incomingOffset;
	SkelMat4 incomingValue;
	SkelQuat incomingQuat;

	incomingQuat = frames->GetSlerpValue( m_quatChannel );
	frames->GetLerpValue3(m_offsetChannel, &incomingOffset);
	//incomingQuat.Normalize();
	incomingQuat.GetMat4( incomingValue );
	VecCopy( incomingOffset, incomingValue[ 3 ] );

	if( m_parent )
	{
		m_cachedValue.Multiply( incomingValue, m_parent->GetTransform( frames ) );
	}
	else
	{
		m_cachedValue = incomingValue;
	}

	if( m_controller )
	{
		SkelMat3 m;

		incomingQuat.Set( m_controller );
		incomingQuat.GetMat3( m );
		m_cachedValue.RotateBy( m );
	}

	m_isDirty = false;

	return m_cachedValue;
}

void skelBone_PosRot::SetChannels(size_t quatChannel, size_t offsetChannel)
{
	m_quatChannel = quatChannel;
	m_offsetChannel = offsetChannel;
}

void skelBone_PosRot::SetBaseValue( const Skeleton::BoneData *boneData )
{
}

size_t skelBone_PosRot::GetChannelIndex( size_t num )
{
	if( num )
	{
		return m_quatChannel;
	}
	else
	{
		return m_offsetChannel;
	}
}

skelBone_Base *skelBone_PosRot::GetBoneRef( size_t num )
{
	return NULL;
}

SkelMat4 skelBone_Root::GetDirtyTransform( const skelAnimStoreFrameList_c *frames )
{
	SkelVec3 incomingOffset;
	SkelMat4 incomingValue;
	SkelQuat incomingQuat;

	incomingQuat = frames->GetSlerpValue( m_quatChannel );
	frames->GetLerpValue3(m_offsetChannel, &incomingOffset);
	//incomingQuat.Normalize();
	incomingQuat.GetMat4( incomingValue );
	VecCopy( incomingOffset, incomingValue[ 3 ] );

	if( m_parent )
	{
		m_cachedValue.Multiply( incomingValue, m_parent->GetTransform( frames ) );
	}
	else
	{
		m_cachedValue = incomingValue;
	}

	if( m_controller )
	{
		SkelMat3 m;

		incomingQuat.Set( m_controller );
		incomingQuat.GetMat3( m );
		m_cachedValue.RotateBy( m );
	}

	m_isDirty = false;

	return m_cachedValue;
}

skelBone_IKshoulder::skelBone_IKshoulder()
{
	m_wrist = NULL;
}

SkelMat4 skelBone_IKshoulder::GetDirtyTransform( const skelAnimStoreFrameList_c *frames )
{
	SkelMat4 baseMatrix, targetMatrix;
	float desiredLength;
	SkelVec3 newBaseY;
	SkelVec3 newBaseZ;
	SkelVec3 cross;
	float cosUpperAngle;
	float sinUpperAngle;
	float maxLength;
	float length;
	float cosA;
	float sinA;

	if( !m_wrist )
	{
		m_cachedValue.MakeIdentity();
		return m_cachedValue;
	}

	if( m_parent )
	{
		baseMatrix = m_parent->GetTransform( frames );
	}

	baseMatrix[ 3 ][ 0 ] += m_baseValue[ 0 ] * baseMatrix[ 0 ][ 0 ] + m_baseValue[ 1 ] * baseMatrix[ 1 ][ 0 ] + m_baseValue[ 2 ] * baseMatrix[ 2 ][ 0 ];
	baseMatrix[ 3 ][ 1 ] += m_baseValue[ 0 ] * baseMatrix[ 0 ][ 1 ] + m_baseValue[ 1 ] * baseMatrix[ 1 ][ 1 ] + m_baseValue[ 2 ] * baseMatrix[ 2 ][ 1 ];
	baseMatrix[ 3 ][ 2 ] += m_baseValue[ 0 ] * baseMatrix[ 0 ][ 2 ] + m_baseValue[ 1 ] * baseMatrix[ 1 ][ 2 ] + m_baseValue[ 2 ] * baseMatrix[ 2 ][ 2 ];
	baseMatrix.InvertAxis( 0 );
	baseMatrix.InvertAxis( 2 );

	m_wristAngle = frames->GetSlerpValue(m_wrist->m_quatChannel);
	//m_wristAngle.Normalize();
	m_wristAngle.GetMat4( targetMatrix );

	frames->GetLerpValue3( m_wrist->m_offsetChannel, &m_wristPos );
	m_cachedValue = baseMatrix;
	VecCopy( m_wristPos, targetMatrix[ 3 ] );
	VecSubtract( targetMatrix[ 3 ], baseMatrix[ 3 ], m_cachedValue[ 0 ] );

	sinUpperAngle = VectorNormalize( m_cachedValue[ 0 ] );

	newBaseY = baseMatrix[ 2 ];
	newBaseZ = m_cachedValue[ 0 ];

	CrossProduct( newBaseY, newBaseZ, cross );
	VecCopy( cross, m_cachedValue[ 1 ] );

	newBaseY = targetMatrix[ 2 ];
	newBaseZ = m_cachedValue[ 0 ];

	CrossProduct( newBaseY, newBaseZ, cross );
	VecAdd( m_cachedValue[ 1 ], cross, m_cachedValue[ 1 ] );

	desiredLength = VectorNormalize( m_cachedValue[ 1 ] );

	if( !desiredLength ) {
		m_cachedValue[ 1 ][ 1 ] = 1.0;
	}

	newBaseY = m_cachedValue[ 0 ];
	newBaseZ = m_cachedValue[ 1 ];

	CrossProduct( newBaseY, newBaseZ, cross );
	VecCopy( cross, m_cachedValue[ 2 ] );

	cosUpperAngle = m_upperLength + m_lowerLength - 0.001f;

	if( sinUpperAngle > cosUpperAngle )
	{
		sinUpperAngle = cosUpperAngle;

		m_wristPos = m_cachedValue[ 3 ];

		m_wristPos[ 0 ] += m_cachedValue[ 0 ][ 0 ] * cosUpperAngle;
		m_wristPos[ 1 ] += m_cachedValue[ 0 ][ 1 ] * cosUpperAngle;
		m_wristPos[ 2 ] += m_cachedValue[ 0 ][ 2 ] * cosUpperAngle;
	}

	maxLength = ( sinUpperAngle * sinUpperAngle + m_upperLength * m_upperLength - m_lowerLength * m_lowerLength )
		/ ( sinUpperAngle * m_upperLength + sinUpperAngle * m_upperLength );

	if( maxLength > 1.0 ) {
		maxLength = 1.0;
	}

	m_cosElbowAngle = -( ( m_lowerLength * m_lowerLength + m_upperLength * m_upperLength - sinUpperAngle * sinUpperAngle ) /
		( m_upperLength * m_lowerLength + m_upperLength * m_lowerLength ) );

	length = -sqrtf( 1.0f - maxLength * maxLength );

	cosA = m_cachedValue[ 0 ][ 0 ];
	sinA = m_cachedValue[ 1 ][ 0 ];
	m_cachedValue[ 0 ][ 0 ] = cosA * maxLength - sinA * length;
	m_cachedValue[ 1 ][ 0 ] = sinA * maxLength + cosA * length;

	cosA = m_cachedValue[ 0 ][ 1 ];
	sinA = m_cachedValue[ 1 ][ 1 ];
	m_cachedValue[ 0 ][ 1 ] = cosA * maxLength - sinA * length;
	m_cachedValue[ 1 ][ 1 ] = sinA * maxLength + cosA * length;

	cosA = m_cachedValue[ 0 ][ 2 ];
	sinA = m_cachedValue[ 1 ][ 2 ];
	m_cachedValue[ 0 ][ 2 ] = cosA * maxLength - sinA * length;
	m_cachedValue[ 1 ][ 2 ] = sinA * maxLength + cosA * length;

	m_isDirty = false;

	return m_cachedValue;
}

void skelBone_IKshoulder::SetBaseValue( const Skeleton::BoneData *boneData )
{
	m_baseValue = boneData->offset;
}

size_t skelBone_IKshoulder::GetChannelIndex( size_t num )
{
	return -1;
}

skelBone_Base *skelBone_IKshoulder::GetBoneRef( size_t num )
{
	return m_wrist;
}

void skelBone_IKshoulder::SetElbowValue( float elbowOffset )
{
	m_upperLength = elbowOffset;
}

void skelBone_IKshoulder::SetWristValue( float wristOffset )
{
	m_lowerLength = wristOffset;
}

void skelBone_IKshoulder::SetWristBone( skelBone_IKwrist *wrist )
{
	this->m_wrist = wrist;
}

float skelBone_IKshoulder::GetUpperLength()
{
	return m_upperLength;
}

float skelBone_IKshoulder::GetLowerLength()
{
	return m_lowerLength;
}

SkelMat4 skelBone_IKwrist::GetDirtyTransform( const skelAnimStoreFrameList_c *frames )
{
	if( m_shoulder->m_isDirty ) {
		m_shoulder->GetDirtyTransform( frames );
	}

	m_shoulder->m_wristAngle.GetMat4(m_cachedValue);
	//m_shoulder->m_wristAngle.Normalize();
	VecCopy( m_shoulder->m_wristPos, m_cachedValue[ 3 ] );
	m_isDirty = false;

	return m_cachedValue;
}

void skelBone_IKwrist::SetChannels(size_t quatChannel, size_t offsetChannel)
{
	m_quatChannel = quatChannel;
	m_offsetChannel = offsetChannel;
}

void skelBone_IKwrist::SetBoneRefs( skelBone_IKshoulder *shoulder )
{
	m_shoulder = shoulder;
}

void skelBone_IKwrist::SetBaseValue( const Skeleton::BoneData *boneData )
{
	m_shoulder->SetWristValue( boneData->length );
}

size_t skelBone_IKwrist::GetChannelIndex( size_t num )
{
	return -1;
}

skelBone_Base *skelBone_IKwrist::GetBoneRef( size_t num )
{
	return m_shoulder;
}

SkelMat4 skelBone_IKelbow::GetDirtyTransform( const skelAnimStoreFrameList_c *frames )
{
	float fLength;
	float cosA, sinA;

	m_cachedValue = m_shoulder->GetTransform( frames );

	m_cachedValue[ 3 ][ 0 ] += m_cachedValue[ 0 ][ 0 ] * m_shoulder->m_upperLength;
	m_cachedValue[ 3 ][ 1 ] += m_cachedValue[ 0 ][ 1 ] * m_shoulder->m_upperLength;
	m_cachedValue[ 3 ][ 2 ] += m_cachedValue[ 0 ][ 2 ] * m_shoulder->m_upperLength;

	fLength = sqrtf( 1.0f - m_shoulder->m_cosElbowAngle * m_shoulder->m_cosElbowAngle );

	cosA = m_cachedValue[ 0 ][ 0 ];
	sinA = m_cachedValue[ 1 ][ 0 ];
	m_cachedValue[ 0 ][ 0 ] = cosA * m_shoulder->m_cosElbowAngle - sinA * fLength;
	m_cachedValue[ 1 ][ 0 ] = sinA * m_shoulder->m_cosElbowAngle + cosA * fLength;

	cosA = m_cachedValue[ 0 ][ 1 ];
	sinA = m_cachedValue[ 1 ][ 1 ];
	m_cachedValue[ 0 ][ 1 ] = cosA * m_shoulder->m_cosElbowAngle - sinA * fLength;
	m_cachedValue[ 1 ][ 1 ] = sinA * m_shoulder->m_cosElbowAngle + cosA * fLength;

	cosA = m_cachedValue[ 0 ][ 2 ];
	sinA = m_cachedValue[ 1 ][ 2 ];
	m_cachedValue[ 0 ][ 2 ] = cosA * m_shoulder->m_cosElbowAngle - sinA * fLength;
	m_cachedValue[ 1 ][ 2 ] = sinA * m_shoulder->m_cosElbowAngle + cosA * fLength;

	m_isDirty = false;

	return m_cachedValue;
}

void skelBone_IKelbow::SetBoneRefs( skelBone_IKshoulder *shoulder )
{
	m_shoulder = shoulder;
}

void skelBone_IKelbow::SetBaseValue( const Skeleton::BoneData *boneData )
{
	m_shoulder->SetElbowValue( boneData->length );
}

size_t skelBone_IKelbow::GetChannelIndex( size_t num )
{
	return -1;
}

skelBone_Base *skelBone_IKelbow::GetBoneRef( size_t num )
{
	return m_shoulder;
}

skelBone_AvRot::skelBone_AvRot()
{
	m_reference1 = NULL;
	m_reference2 = NULL;
}

SkelMat4 skelBone_AvRot::GetDirtyTransform( const skelAnimStoreFrameList_c *frames )
{
	SkelQuat temp1, temp2;
	SkelMat4 temp;

	temp1 = m_reference1->GetTransform( frames );
	temp2 = m_reference2->GetTransform( frames );

	// lerp values
	Slerp( temp1, temp2, m_bone2weight, &m_cachedQuat );

	VecCopy( m_basePos, m_cachedValue[ 3 ] );

	if( m_parent )
	{
		temp.Multiply( m_cachedValue, m_parent->GetTransform( frames ) );
		m_cachedValue = temp;
	}

	m_cachedQuat.GetMat4( m_cachedValue );

	m_isDirty = false;

	return m_cachedValue;
}

void skelBone_AvRot::SetBoneRefs( skelBone_Base *ref1, skelBone_Base *ref2 )
{
	m_reference1 = ref1;
	m_reference2 = ref2;
}

void skelBone_AvRot::SetBaseValue( const Skeleton::BoneData *boneData )
{
	m_basePos = boneData->offset;
	m_bone2weight = boneData->weight;
}

size_t skelBone_AvRot::GetChannelIndex( size_t num )
{
	return -1;
}

skelBone_Base *skelBone_AvRot::GetBoneRef( size_t num )
{
	if( num )
	{
		return m_reference2;
	}
	else
	{
		return m_reference1;
	}
}

SkelMat4 skelBone_HoseRot::GetDirtyTransform(const skelAnimStoreFrameList_c *frames)
{
	SkelMat4 mat;

	if (m_parent)
	{
		mat = m_parent->GetTransform(frames);
	}

	SkelMat4 targetMat = m_target->GetTransform(frames);
	return GetDirtyTransform(mat, targetMat);
}

SkelMat4 skelBone_HoseRot::GetDirtyTransform(SkelMat4& myParentTM, SkelMat4& targetTM)
{
	SkelMat4 m;
	SkelMat4 invParentTM;
	SkelMat4 temp;
	SkelVec3 rotaxis;
	SkelVec3 targetup, targetaim;
	SkelVec3 upaxis, aimaxis;
	SkelVec3 tmp;
	SkelQuat targetQuat;
	float l, s, c;
	float angle, vScale;

	aimaxis = myParentTM[ 0 ];
	targetaim = targetTM[ 0 ];

	CrossProduct( targetaim, aimaxis, rotaxis );

	s = VectorLengthSquared( rotaxis );

	if( s == 0.0 ) {
		rotaxis.x = 1.0f;
	} else if( s != 1.0f ) {
		l = 1.0f / sqrtf( s );
		VectorScale( rotaxis, l, rotaxis );
	}

	s = DotProduct( aimaxis, targetaim );
	if( s < 1.0f ) {
		if( s > -0.999f ) {
			angle = acosf( s );
		} else {
			angle = 6.2831855f;
		}
	} else {
		angle = 0.0;
	}

	vScale = angle * m_bendRatio;
	if( vScale > m_bendMax ) {
		vScale = m_bendMax;
	}

	temp[ 0 ][ 0 ] = myParentTM[ 0 ][ 0 ];
	temp[ 0 ][ 1 ] = myParentTM[ 1 ][ 0 ];
	temp[ 0 ][ 2 ] = myParentTM[ 2 ][ 0 ];
	temp[ 1 ][ 0 ] = myParentTM[ 0 ][ 1 ];
	temp[ 1 ][ 1 ] = myParentTM[ 1 ][ 1 ];
	temp[ 1 ][ 2 ] = myParentTM[ 2 ][ 1 ];
	temp[ 2 ][ 0 ] = myParentTM[ 0 ][ 2 ];
	temp[ 2 ][ 1 ] = myParentTM[ 1 ][ 2 ];
	temp[ 2 ][ 2 ] = myParentTM[ 2 ][ 2 ];
	temp[ 3 ][ 0 ] = -( myParentTM[ 0 ][ 0 ] * myParentTM[ 3 ][ 0 ] + myParentTM[ 0 ][ 1 ] * myParentTM[ 3 ][ 1 ] + myParentTM[ 0 ][ 2 ] * myParentTM[ 3 ][ 2 ] );
	temp[ 3 ][ 1 ] = -( myParentTM[ 1 ][ 0 ] * myParentTM[ 3 ][ 0 ] + myParentTM[ 1 ][ 1 ] * myParentTM[ 3 ][ 1 ] + myParentTM[ 1 ][ 2 ] * myParentTM[ 3 ][ 2 ] );
	temp[ 3 ][ 2 ] = -( myParentTM[ 2 ][ 0 ] * myParentTM[ 3 ][ 0 ] + myParentTM[ 2 ][ 1 ] * myParentTM[ 3 ][ 1 ] + myParentTM[ 2 ][ 2 ] * myParentTM[ 3 ][ 2 ] );

	m.Multiply( temp, myParentTM );

	VecCopy( rotaxis, tmp );
	rotaxis[ 0 ] = tmp[ 0 ] * temp[ 0 ][ 0 ] + tmp[ 1 ] * temp[ 1 ][ 0 ] + tmp[ 2 ] * temp[ 2 ][ 0 ];
	rotaxis[ 1 ] = tmp[ 0 ] * temp[ 0 ][ 1 ] + tmp[ 1 ] * temp[ 1 ][ 1 ] + tmp[ 2 ] * temp[ 2 ][ 1 ];
	rotaxis[ 2 ] = tmp[ 0 ] * temp[ 0 ][ 2 ] + tmp[ 1 ] * temp[ 1 ][ 2 ] + tmp[ 2 ] * temp[ 2 ][ 2 ];

	targetaim.y = cosf( vScale * 0.5f );
	targetup.y = rotaxis.x * targetaim.y;
	targetup.z = rotaxis.y * targetaim.y;
	targetaim.x = rotaxis.z * targetaim.y;

	c = cosf( vScale * 0.5f );
	l = sqrtf( 1.0f - c * c );

	m_cachedQuat[ 0 ] = rotaxis[ 0 ] * l;
	m_cachedQuat[ 1 ] = rotaxis[ 1 ] * l;
	m_cachedQuat[ 2 ] = rotaxis[ 2 ] * l;
	m_cachedQuat[ 3 ] = c;

	if( m_spinRatio < 1.0f )
	{
		m.Multiply( targetTM, temp );
		MatToQuat( m.val, targetQuat.val );
		Slerp( targetQuat, m_cachedQuat, m_spinRatio, &m_cachedQuat );
	}

	m_cachedQuat.GetMat4( m_cachedValue );
	VecCopy( m_basePos, m_cachedValue[ 3 ] );

	m.Multiply( m_cachedValue, myParentTM );
	m_cachedValue = m;

	m_isDirty = false;

	return m_cachedValue;
}

void skelBone_HoseRot::SetBoneRefs( skelBone_Base *ref )
{
	m_target = ref;
}

void skelBone_HoseRot::SetBaseValue( const Skeleton::BoneData *boneData )
{
	m_bendRatio = boneData->bendRatio;
	m_bendMax = boneData->bendMax;
	m_spinRatio = boneData->spinRatio;
	m_basePos = boneData->offset;
}

size_t skelBone_HoseRot::GetChannelIndex( size_t num )
{
	return -1;
}

skelBone_Base *skelBone_HoseRot::GetBoneRef( size_t num )
{
	return m_target;
}

SkelMat4 skelBone_HoseRotBoth::GetDirtyTransform( const skelAnimStoreFrameList_c *frames )
{
	SkelMat4 myParentTM;
	SkelMat4 targetTM;

	if( m_parent )
	{
		myParentTM = m_parent->GetTransform( frames );
		targetTM = m_target->GetTransform( frames );
	}

	VectorInverse( targetTM[ 0 ] );
	VectorInverse( targetTM[ 2 ] );
	VectorInverse( myParentTM[ 0 ] );
	VectorInverse( myParentTM[ 2 ] );

	return skelBone_HoseRot::GetDirtyTransform( myParentTM, targetTM );
}

void skelBone_HoseRotBoth::SetBaseValue( const Skeleton::BoneData *boneData )
{
	skelBone_HoseRot::SetBaseValue( boneData );
	m_basePos.x = -m_basePos.x;
	m_basePos.z = -m_basePos.z;
}

SkelMat4 skelBone_HoseRotParent::GetDirtyTransform( const skelAnimStoreFrameList_c *frames )
{
	SkelMat4 myParentTM;
	SkelMat4 targetTM;

	if( m_parent )
	{
		myParentTM = m_parent->GetTransform( frames );
		targetTM = m_target->GetTransform( frames );
	}

	VectorInverse( myParentTM[ 0 ] );
	VectorInverse( myParentTM[ 2 ] );

	return skelBone_HoseRot::GetDirtyTransform( myParentTM, targetTM );
}

void skelBone_HoseRotParent::SetBaseValue( const Skeleton::BoneData *boneData )
{
	skelBone_HoseRot::SetBaseValue( boneData );
	m_basePos.x = -m_basePos.x;
	m_basePos.z = -m_basePos.z;
}

SkelMat4 skelBone_World::GetDirtyTransform( const skelAnimStoreFrameList_c *frames )
{
	m_cachedValue.MakeIdentity();
	m_isDirty = false;
	return m_cachedValue;
}

void skelBone_World::SetBaseValue( const Skeleton::BoneData *boneData )
{
}

size_t skelBone_World::GetChannelIndex( size_t num )
{
	return 0;
}

skelBone_Base *skelBone_World::GetBoneRef( size_t num )
{
	return NULL;
}
