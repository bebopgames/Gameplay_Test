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

	PrimitivePtr m_shape;
};
