/**
* Copyright (c) 2022 Yigsoft. All Rights Reserved.
*/

#pragma once
#include "Projectile.h"

class Projectile_Primary final : public Projectile
{
public:
	Projectile_Primary( const Vector3& position, const Vector3& velocity, float speedMultiplier );
	virtual ~Projectile_Primary() override;

	int OnUpdate( float deltaTime, FlockManager& flockManager, Pickup* pickup ) override;
	void OnRender( cdp_framework::RenderContextPtr& renderContext ) override;
	void SetSpeedMultiplier( float multiplier );

private:
	void BounceOffWorldBounds();

	PrimitivePtr m_shape;
	float m_energy = 8.0f;
	float m_speedMultiplier = 1.0f;
};
