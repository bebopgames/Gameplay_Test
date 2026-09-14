/**
* Copyright (c) 2022 Yigsoft. All Rights Reserved.
*/

#include "pch.h"
#include "Projectile_Primary.h"
#include "Engine.h"
#include "FlockManager.h"

namespace
{
	constexpr float PROJECTILE_RADIUS = 0.45f;
	constexpr float INITIAL_SPEED = 22.0f;
	constexpr float PURSUIT_ACCELERATION = 3.0f;
	constexpr float ENERGY_DRAIN_PER_SECOND = 1.5f;
	constexpr float ENERGY_PER_CONSUMED_BOID = 1.0f;
	constexpr float CONSUME_DISTANCE = PROJECTILE_RADIUS + 0.35f;
	constexpr float MIN_XZ = -21.0f;
	constexpr float MAX_XZ = 21.0f;
	constexpr float MIN_Y = 0.45f;
	constexpr float MAX_Y = 25.0f;
}

Projectile_Primary::Projectile_Primary( const Vector3& position, const Vector3& direction ) :
	Projectile( position, direction * INITIAL_SPEED )
{
	m_shape = GetEngine().CreateSpherePrimitive( PROJECTILE_RADIUS );
}

Projectile_Primary::~Projectile_Primary() = default;

void Projectile_Primary::OnUpdate( float deltaTime, FlockManager& flockManager )
{
	const float timeStep = std::min( deltaTime, 0.05f );
	if ( timeStep <= 0.0f || !m_isActive ) return;

	Vector3 targetPosition;
	if ( flockManager.FindClosestYellowBall( m_position, targetPosition ) )
	{
		Vector3 targetDirection = targetPosition - m_position;
		if ( targetDirection.LengthSquared() > 0.0001f )
		{
			targetDirection.Normalize();
			// Initial shot speed remains dominant; pursuit is intentionally gentle.
			m_velocity += targetDirection * PURSUIT_ACCELERATION * timeStep;
		}
		flockManager.ScatterYellowBalls( m_position, 9.0f );
	}

	m_energy -= ENERGY_DRAIN_PER_SECOND * timeStep;
	m_position += m_velocity * timeStep;
	BounceOffWorldBounds();
	flockManager.BounceProjectileOffBuildings( m_position, m_velocity, PROJECTILE_RADIUS );

	if ( flockManager.ConsumeYellowBall( m_position, CONSUME_DISTANCE ) )
	{
		m_energy += ENERGY_PER_CONSUMED_BOID;
	}

	if ( m_energy <= 0.0f )
	{
		flockManager.AddToNearestFlock( m_position, m_velocity );
		m_isActive = false;
	}
}

void Projectile_Primary::OnRender( cdp_framework::RenderContextPtr& renderContext )
{
	renderContext->RenderPrimitive( m_shape, Vector3::One, m_position, Vector3::Zero, Colors::Red );
}

void Projectile_Primary::BounceOffWorldBounds()
{
	if ( m_position.x < MIN_XZ || m_position.x > MAX_XZ )
	{
		m_position.x = std::max( MIN_XZ, std::min( m_position.x, MAX_XZ ) );
		m_velocity.x = -m_velocity.x;
	}
	if ( m_position.y < MIN_Y || m_position.y > MAX_Y )
	{
		m_position.y = std::max( MIN_Y, std::min( m_position.y, MAX_Y ) );
		m_velocity.y = -m_velocity.y;
	}
	if ( m_position.z < MIN_XZ || m_position.z > MAX_XZ )
	{
		m_position.z = std::max( MIN_XZ, std::min( m_position.z, MAX_XZ ) );
		m_velocity.z = -m_velocity.z;
	}
}
