/**
* Copyright (c) 2022 Yigsoft. All Rights Reserved.
*/

#pragma once
#include "Projectile.h"

class Projectile_Secondary final : public Projectile
{
public:
	Projectile_Secondary( const Vector3& position, const Vector3& direction );
	virtual ~Projectile_Secondary() override;

	int OnUpdate( float deltaTime, FlockManager& flockManager, Pickup* pickup ) override;
	void OnRender( cdp_framework::RenderContextPtr& renderContext ) override;

private:
	bool HasImpactedWorld( const FlockManager& flockManager ) const;
	void BeginImpact( FlockManager& flockManager );

	PrimitivePtr m_cubeShape;
	PrimitivePtr m_pulseShape;
	Vector3 m_rotation = Vector3::Zero;
	Vector3 m_impactPosition = Vector3::Zero;
	float m_pulseElapsed = 0.0f;
	bool m_hasImpacted = false;
};
