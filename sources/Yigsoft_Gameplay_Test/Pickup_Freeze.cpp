/**
* Copyright (c) 2022 Yigsoft. All Rights Reserved.
*/

#include "pch.h"
#include "Pickup_Freeze.h"
#include "Engine.h"
#include "FlockManager.h"

namespace
{
	constexpr float PICKUP_HALF_EXTENT = 0.45f;
	constexpr float ROTATION_SPEED_X = 4.0f;
	constexpr float ROTATION_SPEED_Y = 5.5f;
	constexpr float FREEZE_RADIUS = 12.0f;
	constexpr float FREEZE_DURATION = 5.0f;
	constexpr float PULSE_DURATION = 0.35f;
}

Pickup_Freeze::Pickup_Freeze( const Vector3& position, float lifetime ) :
	Pickup( position, PICKUP_HALF_EXTENT ),
	m_lifetime( lifetime )
{
	m_cubeShape = GetEngine().CreateBoxPrimitive( Vector3::One * ( PICKUP_HALF_EXTENT * 2.0f ) );
	m_pulseShape = GetEngine().CreateSpherePrimitive( 1.0f );
}

Pickup_Freeze::~Pickup_Freeze() = default;

void Pickup_Freeze::OnUpdate( float deltaTime, FlockManager& flockManager )
{
	UNREFERENCED_PARAMETER( flockManager );
	const float timeStep = std::min( deltaTime, 0.05f );
	if ( timeStep <= 0.0f || !m_isActive ) return;

	if ( m_hasActivated )
	{
		m_pulseElapsed += timeStep;
		if ( m_pulseElapsed >= PULSE_DURATION ) m_isActive = false;
		return;
	}

	m_lifetime -= timeStep;
	m_rotation.x += ROTATION_SPEED_X * timeStep;
	m_rotation.y += ROTATION_SPEED_Y * timeStep;
	if ( m_lifetime <= 0.0f ) m_isActive = false;
}

void Pickup_Freeze::OnRender( cdp_framework::RenderContextPtr& renderContext )
{
	if ( !m_hasActivated )
	{
		renderContext->RenderPrimitive( m_cubeShape, Vector3::One, m_position, m_rotation, Colors::Blue );
		return;
	}

	const float progress = std::min( m_pulseElapsed / PULSE_DURATION, 1.0f );
	renderContext->RenderPrimitive( m_pulseShape, Vector3::One * ( FREEZE_RADIUS * progress ), m_position, Vector3::Zero, Colors::LightBlue );
}

void Pickup_Freeze::OnActivated( FlockManager& flockManager )
{
	m_hasActivated = true;
	flockManager.FreezeBoids( m_position, FREEZE_RADIUS, FREEZE_DURATION );
}
