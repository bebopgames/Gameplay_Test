/**
* Copyright (c) 2022 Yigsoft. All Rights Reserved.
*/

#include "pch.h"
#include "Pickup.h"
#include "FlockManager.h"

Pickup::Pickup( const Vector3& position, float collisionRadius ) :
	m_position( position ),
	m_collisionRadius( collisionRadius )
{
}

Pickup::~Pickup() = default;

PickupEffect Pickup::TryActivate( const Vector3& projectilePosition, float projectileRadius, FlockManager& flockManager )
{
	if ( !m_isActive || !m_isCollectible ) return PickupEffect::None;
	const float collisionDistance = m_collisionRadius + projectileRadius;
	if ( ( projectilePosition - m_position ).LengthSquared() > collisionDistance * collisionDistance ) return PickupEffect::None;

	m_isCollectible = false;
	OnActivated( flockManager );
	return GetEffect();
}

bool Pickup::IsActive() const
{
	return m_isActive;
}
