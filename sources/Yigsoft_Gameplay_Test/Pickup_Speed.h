/**
* Copyright (c) 2022 Yigsoft. All Rights Reserved.
*/

#pragma once
#include "Pickup.h"

class Pickup_Speed final : public Pickup
{
public:
	Pickup_Speed( const Vector3& position );
	virtual ~Pickup_Speed() override;

	void OnUpdate( float deltaTime, FlockManager& flockManager ) override;
	void OnRender( cdp_framework::RenderContextPtr& renderContext ) override;

protected:
	void OnActivated( FlockManager& flockManager ) override;
	PickupEffect GetEffect() const override { return PickupEffect::Speed; }

private:
	PrimitivePtr m_shape;
	float m_lifetime = 6.0f;
	float m_rotationY = 0.0f;
};
