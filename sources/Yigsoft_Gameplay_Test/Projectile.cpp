/**
* Copyright (c) 2022 Yigsoft. All Rights Reserved.
*/

#include "pch.h"
#include "Projectile.h"

Projectile::Projectile( const Vector3& position, const Vector3& velocity ) :
	m_position( position ),
	m_velocity( velocity )
{
}

Projectile::~Projectile() = default;

bool Projectile::IsActive() const
{
	return m_isActive;
}

PickupEffect Projectile::TakePickupEffect()
{
	const PickupEffect effect = m_pendingPickupEffect;
	m_pendingPickupEffect = PickupEffect::None;
	return effect;
}
