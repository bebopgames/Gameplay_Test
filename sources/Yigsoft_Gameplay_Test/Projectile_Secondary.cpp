/**
* Copyright (c) 2022 Yigsoft. All Rights Reserved.
*/

#include "pch.h"
#include "Projectile_Secondary.h"
#include "Engine.h"
#include "FlockManager.h"

namespace
{
	constexpr float PROJECTILE_HEIGHT = 1.4f;
	constexpr float PROJECTILE_DIAMETER = 0.8f;
	constexpr float PROJECTILE_RADIUS = PROJECTILE_HEIGHT * 0.5f;
	constexpr float INITIAL_SPEED = 28.0f;
	constexpr float IMPACT_RADIUS = 5.0f;
	constexpr int SCORE_PER_CONSUMED_BOID = 10;
	constexpr float MIN_XZ = -21.0f;
	constexpr float MAX_XZ = 21.0f;
	constexpr float MIN_Y = 0.0f;
	constexpr float MAX_Y = 25.0f;
}

Projectile_Secondary::Projectile_Secondary( const Vector3& position, const Vector3& direction ) :
	Projectile( position, direction * INITIAL_SPEED )
{
	m_shape = GetEngine().CreateCylinderPrimitive( PROJECTILE_HEIGHT, PROJECTILE_DIAMETER );
}

Projectile_Secondary::~Projectile_Secondary() = default;

int Projectile_Secondary::OnUpdate( float deltaTime, FlockManager& flockManager, Pickup* pickup )
{
	UNREFERENCED_PARAMETER( pickup );
	const float timeStep = std::min( deltaTime, 0.05f );
	if ( timeStep <= 0.0f || !m_isActive ) return 0;

	m_position += m_velocity * timeStep;
	if ( !HasImpactedWorld( flockManager ) ) return 0;

	const int consumedBoids = flockManager.ConsumeYellowBallsWithin( m_position, IMPACT_RADIUS );
	m_isActive = false;
	return consumedBoids * SCORE_PER_CONSUMED_BOID;
}

void Projectile_Secondary::OnRender( cdp_framework::RenderContextPtr& renderContext )
{
	Vector3 direction = m_velocity;
	if ( direction.LengthSquared() <= 0.0001f ) direction = Vector3::UnitZ;
	else direction.Normalize();
	const float yaw = atan2f( direction.x, direction.z );
	const float pitch = acosf( std::max( -1.0f, std::min( 1.0f, direction.y ) ) );
	renderContext->RenderPrimitive( m_shape, Vector3::One, m_position, Vector3( yaw, pitch, 0.0f ), Colors::Brown );
}

bool Projectile_Secondary::HasImpactedWorld( const FlockManager& flockManager ) const
{
	if ( m_position.x - PROJECTILE_RADIUS < MIN_XZ || m_position.x + PROJECTILE_RADIUS > MAX_XZ ||
		m_position.y - PROJECTILE_RADIUS < MIN_Y || m_position.y + PROJECTILE_RADIUS > MAX_Y ||
		m_position.z - PROJECTILE_RADIUS < MIN_XZ || m_position.z + PROJECTILE_RADIUS > MAX_XZ ) return true;

	return flockManager.IntersectsBuilding( m_position, PROJECTILE_RADIUS ) || flockManager.HasBoidWithin( m_position, PROJECTILE_RADIUS );
}
