/**
* Copyright (c) 2022 Yigsoft. All Rights Reserved.
*/

#include "pch.h"
#include "Pickup_Speed.h"
#include "Engine.h"
#include "FlockManager.h"

namespace
{
	constexpr float PICKUP_SIZE = 1.0f;
	constexpr float ROTATION_SPEED_Y = 3.5f;
}

Pickup_Speed::Pickup_Speed( const Vector3& position ) :
	Pickup( position, PICKUP_SIZE * 0.5f )
{
	m_shape = GetEngine().CreateTetrahedronPrimitive( PICKUP_SIZE );
}

Pickup_Speed::~Pickup_Speed() = default;

void Pickup_Speed::OnUpdate( float deltaTime, FlockManager& flockManager )
{
	UNREFERENCED_PARAMETER( flockManager );
	const float timeStep = std::min( deltaTime, 0.05f );
	if ( timeStep <= 0.0f || !m_isActive ) return;

	m_lifetime -= timeStep;
	m_rotationY += ROTATION_SPEED_Y * timeStep;
	if ( m_lifetime <= 0.0f ) m_isActive = false;
}

void Pickup_Speed::OnRender( cdp_framework::RenderContextPtr& renderContext )
{
	renderContext->RenderPrimitive( m_shape, Vector3::One, m_position, Vector3( 0.0f, m_rotationY, 0.0f ), Colors::Green );
}

void Pickup_Speed::OnActivated( FlockManager& flockManager )
{
	UNREFERENCED_PARAMETER( flockManager );
	m_isActive = false;
}
