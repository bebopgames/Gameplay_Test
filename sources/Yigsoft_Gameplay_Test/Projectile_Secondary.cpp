/**
* Copyright (c) 2022 Yigsoft. All Rights Reserved.
*/

#include "pch.h"
#include "Projectile_Secondary.h"
#include "Engine.h"
#include "FlockManager.h"

namespace
{
	constexpr float PROJECTILE_HALF_EXTENT = 0.45f;
	constexpr float INITIAL_SPEED = 28.0f;
	constexpr float ROTATION_SPEED_X = 4.0f;
	constexpr float ROTATION_SPEED_Y = 5.5f;
	constexpr float FREEZE_RADIUS = 12.0f;
	constexpr float FREEZE_DURATION = 5.0f;
	constexpr float PULSE_DURATION = 0.35f;
	constexpr float MIN_XZ = -21.0f;
	constexpr float MAX_XZ = 21.0f;
	constexpr float MIN_Y = 0.0f;
	constexpr float MAX_Y = 25.0f;
}

Projectile_Secondary::Projectile_Secondary( const Vector3& position, const Vector3& direction ) :
	Projectile( position, direction * INITIAL_SPEED )
{
	m_cubeShape = GetEngine().CreateBoxPrimitive( Vector3::One * ( PROJECTILE_HALF_EXTENT * 2.0f ) );
	m_pulseShape = GetEngine().CreateSpherePrimitive( 1.0f );
}

Projectile_Secondary::~Projectile_Secondary() = default;

int Projectile_Secondary::OnUpdate( float deltaTime, FlockManager& flockManager, Pickup* pickup )
{
	UNREFERENCED_PARAMETER( pickup );
	const float timeStep = std::min( deltaTime, 0.05f );
	if ( timeStep <= 0.0f || !m_isActive ) return 0;

	if ( m_hasImpacted )
	{
		m_pulseElapsed += timeStep;
		if ( m_pulseElapsed >= PULSE_DURATION ) m_isActive = false;
		return 0;
	}

	m_rotation.x += ROTATION_SPEED_X * timeStep;
	m_rotation.y += ROTATION_SPEED_Y * timeStep;
	m_position += m_velocity * timeStep;
	if ( HasImpactedWorld( flockManager ) ) BeginImpact( flockManager );
	return 0;
}

void Projectile_Secondary::OnRender( cdp_framework::RenderContextPtr& renderContext )
{
	if ( !m_hasImpacted )
	{
		renderContext->RenderPrimitive( m_cubeShape, Vector3::One, m_position, m_rotation, Colors::Blue );
		return;
	}

	const float pulseProgress = std::min( m_pulseElapsed / PULSE_DURATION, 1.0f );
	const float pulseRadius = FREEZE_RADIUS * pulseProgress;
	renderContext->RenderPrimitive( m_pulseShape, Vector3::One * pulseRadius, m_impactPosition, Vector3::Zero, Colors::LightBlue );
}

bool Projectile_Secondary::HasImpactedWorld( const FlockManager& flockManager ) const
{
	if ( m_position.x - PROJECTILE_HALF_EXTENT < MIN_XZ || m_position.x + PROJECTILE_HALF_EXTENT > MAX_XZ ||
		m_position.y - PROJECTILE_HALF_EXTENT < MIN_Y || m_position.y + PROJECTILE_HALF_EXTENT > MAX_Y ||
		m_position.z - PROJECTILE_HALF_EXTENT < MIN_XZ || m_position.z + PROJECTILE_HALF_EXTENT > MAX_XZ ) return true;

	return flockManager.IntersectsBuilding( m_position, PROJECTILE_HALF_EXTENT ) || flockManager.HasBoidWithin( m_position, PROJECTILE_HALF_EXTENT );
}

void Projectile_Secondary::BeginImpact( FlockManager& flockManager )
{
	m_hasImpacted = true;
	m_impactPosition = m_position;
	m_velocity = Vector3::Zero;
	flockManager.FreezeBoids( m_impactPosition, FREEZE_RADIUS, FREEZE_DURATION );
}
