/**
* Copyright (c) 2022 Yigsoft. All Rights Reserved.
*/

#pragma once
#include "Pickup.h"

class FlockManager;

class Projectile
{
public:
	Projectile( const Vector3& position, const Vector3& velocity );
	virtual ~Projectile();

	virtual int OnUpdate( float deltaTime, FlockManager& flockManager, Pickup* pickup ) = 0;
	virtual void OnRender( cdp_framework::RenderContextPtr& renderContext ) = 0;
	bool IsActive() const;
	PickupEffect TakePickupEffect();

protected:
	Vector3 m_position;
	Vector3 m_velocity;
	bool m_isActive = true;
	PickupEffect m_pendingPickupEffect = PickupEffect::None;
};
