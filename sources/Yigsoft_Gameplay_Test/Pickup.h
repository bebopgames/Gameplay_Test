/**
* Copyright (c) 2022 Yigsoft. All Rights Reserved.
*/

#pragma once
#include "IRenderContext.h"

class FlockManager;

enum class PickupEffect
{
	None,
	Speed,
	Missiles
};

class Pickup
{
public:
	Pickup( const Vector3& position, float collisionRadius );
	virtual ~Pickup();

	virtual void OnUpdate( float deltaTime, FlockManager& flockManager ) = 0;
	virtual void OnRender( cdp_framework::RenderContextPtr& renderContext ) = 0;
	PickupEffect TryActivate( const Vector3& projectilePosition, float projectileRadius, FlockManager& flockManager );
	bool IsActive() const;

protected:
	virtual void OnActivated( FlockManager& flockManager ) = 0;
	virtual PickupEffect GetEffect() const { return PickupEffect::None; }

	Vector3 m_position;
	float m_collisionRadius;
	bool m_isActive = true;
	bool m_isCollectible = true;
};
