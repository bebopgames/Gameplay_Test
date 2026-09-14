/**
* Copyright (c) 2022 Yigsoft. All Rights Reserved.
*/

#pragma once
#include "Projectile.h"

class Projectile_Primary final : public Projectile
{
public:
	Projectile_Primary( const Vector3& position, const Vector3& velocity );
	virtual ~Projectile_Primary() override;

	void OnUpdate( float deltaTime, FlockManager& flockManager ) override;
	void OnRender( cdp_framework::RenderContextPtr& renderContext ) override;

private:
	void BounceOffWorldBounds();

	PrimitivePtr m_shape;
	float m_energy = 8.0f;
};
