/**
* Copyright (c) 2022 Yigsoft. All Rights Reserved.
*/

#include "pch.h"
#include "Pickup_Missiles.h"
#include "Engine.h"
#include "FlockManager.h"

namespace
{
	constexpr float PICKUP_HEIGHT = 1.4f;
	constexpr float PICKUP_DIAMETER = 0.8f;
	constexpr float ROTATION_SPEED_X = 3.5f;
	constexpr float TILT_Z = M_PI / 4.0f;
}

Pickup_Missiles::Pickup_Missiles( const Vector3& position, float lifetime ) :
	Pickup( position, PICKUP_HEIGHT * 0.5f ),
	m_lifetime( lifetime )
{
	m_shape = GetEngine().CreateCylinderPrimitive( PICKUP_HEIGHT, PICKUP_DIAMETER );
}

Pickup_Missiles::~Pickup_Missiles() = default;

void Pickup_Missiles::OnUpdate( float deltaTime, FlockManager& flockManager )
{
	UNREFERENCED_PARAMETER( flockManager );
	const float timeStep = std::min( deltaTime, 0.05f );
	if ( timeStep <= 0.0f || !m_isActive ) return;

	m_lifetime -= timeStep;
	m_rotationX += ROTATION_SPEED_X * timeStep;
	if ( m_lifetime <= 0.0f ) m_isActive = false;
}

void Pickup_Missiles::OnRender( cdp_framework::RenderContextPtr& renderContext )
{
	renderContext->RenderPrimitive( m_shape, Vector3::One, m_position, Vector3( m_rotationX, 0.0f, TILT_Z ), Colors::Brown );
}

void Pickup_Missiles::OnActivated( FlockManager& flockManager )
{
	UNREFERENCED_PARAMETER( flockManager );
	m_isActive = false;
}
