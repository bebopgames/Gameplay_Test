/**
* Copyright (c) 2022 Yigsoft. All Rights Reserved.
*/

#pragma once
#include "Pickup.h"

class Pickup_Freeze final : public Pickup
{
public:
	Pickup_Freeze( const Vector3& position, float lifetime );
	virtual ~Pickup_Freeze() override;

	void OnUpdate( float deltaTime, FlockManager& flockManager ) override;
	void OnRender( cdp_framework::RenderContextPtr& renderContext ) override;

protected:
	void OnActivated( FlockManager& flockManager ) override;

private:
	PrimitivePtr m_cubeShape;
	PrimitivePtr m_pulseShape;
	Vector3 m_rotation = Vector3::Zero;
	float m_lifetime = 6.0f;
	float m_pulseElapsed = 0.0f;
	bool m_hasActivated = false;
};
