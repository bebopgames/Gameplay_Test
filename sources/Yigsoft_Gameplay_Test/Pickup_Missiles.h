/**
* Copyright (c) 2022 Yigsoft. All Rights Reserved.
*/

#pragma once
#include "Pickup.h"

class Pickup_Missiles final : public Pickup
{
public:
	Pickup_Missiles( const Vector3& position, float lifetime );
	virtual ~Pickup_Missiles() override;

	void OnUpdate( float deltaTime, FlockManager& flockManager ) override;
	void OnRender( cdp_framework::RenderContextPtr& renderContext ) override;

protected:
	void OnActivated( FlockManager& flockManager ) override;
	PickupEffect GetEffect() const override { return PickupEffect::Missiles; }

private:
	PrimitivePtr m_shape;
	float m_lifetime = 6.0f;
	float m_rotationX = 0.0f;
};
