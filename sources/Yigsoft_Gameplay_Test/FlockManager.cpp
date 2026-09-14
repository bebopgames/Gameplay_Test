/**
* Copyright (c) 2022 Yigsoft. All Rights Reserved.
*/

#include "pch.h"
#include "FlockManager.h"
#include "City.h"
#include "Engine.h"

namespace
{
	constexpr float BOID_RADIUS = 0.35f;
	constexpr float NEIGHBOUR_RADIUS = 8.0f;
	constexpr float SEPARATION_RADIUS = 2.25f;
	constexpr float MIN_SPEED = 3.0f;
	constexpr float MAX_SPEED = 6.5f;
	constexpr float MIN_XZ = -21.0f;
	constexpr float MAX_XZ = 21.0f;
	constexpr float MIN_Y = 2.0f;
	constexpr float MAX_Y = 25.0f;

	uint32_t s_randomState = 0xC0FFEEu;

	float RandomRange( const float minimum, const float maximum )
	{
		s_randomState = s_randomState * 1664525u + 1013904223u;
		const float unit = static_cast< float >( s_randomState >> 8 ) / static_cast< float >( 0x01000000u );
		return minimum + ( maximum - minimum ) * unit;
	}

	int RandomRange( const int minimum, const int maximum )
	{
		return minimum + static_cast< int >( RandomRange( 0.0f, 1.0f ) * static_cast< float >( maximum - minimum + 1 ) );
	}

	float Clamp( const float value, const float minimum, const float maximum )
	{
		return std::max( minimum, std::min( value, maximum ) );
	}

	Vector3 LimitMagnitude( const Vector3& value, const float maximum )
	{
		const float lengthSquared = value.LengthSquared();
		return lengthSquared > maximum * maximum ? value * ( maximum / sqrtf( lengthSquared ) ) : value;
	}
}

FlockManager::FlockManager( const City& city ) :
	m_city( city )
{
}

FlockManager::~FlockManager() = default;

void FlockManager::OnInitialize()
{
	OnShutdown();
	m_boidShape = GetEngine().CreateSpherePrimitive( BOID_RADIUS );

	const float flockX[] = { -12.0f, 12.0f, -4.0f };
	const float flockZ[] = { -12.0f, -4.0f, 12.0f };
	const float flockY[] = { 10.0f, 16.0f, 19.0f };
	const size_t spawnPointCount = sizeof( flockX ) / sizeof( flockX[ 0 ] );
	const int minimumSize = std::max( 1, std::min( minimumFlockSize, maximumFlockSize ) );
	const int maximumSize = std::max( minimumSize, maximumFlockSize );

	for ( size_t flockIndex = 0; flockIndex < flockNumber; ++flockIndex )
	{
		const int flockSize = RandomRange( minimumSize, maximumSize );
		for ( int boidIndex = 0; boidIndex < flockSize; ++boidIndex )
		{
			Boid boid;
			const size_t spawnIndex = flockIndex % spawnPointCount;
			boid.flockIndex = flockIndex;
			boid.position = Vector3( flockX[ spawnIndex ] + RandomRange( -2.0f, 2.0f ), flockY[ spawnIndex ] + RandomRange( -2.0f, 2.0f ), flockZ[ spawnIndex ] + RandomRange( -2.0f, 2.0f ) );
			boid.velocity = Vector3( RandomRange( -1.0f, 1.0f ), RandomRange( -0.25f, 0.25f ), RandomRange( -1.0f, 1.0f ) );
			boid.velocity.Normalize();
			boid.cruiseSpeed = RandomRange( 4.25f, 5.5f );
			boid.wanderPhase = static_cast< float >( boidIndex ) * 0.63f + static_cast< float >( flockIndex ) * 1.97f;
			boid.velocity *= boid.cruiseSpeed;
			m_boids.push_back( boid );
		}
	}
}

void FlockManager::OnUpdate( float deltaTime )
{
	const float timeStep = std::min( deltaTime, 0.05f );
	if ( timeStep <= 0.0f ) return;

	std::vector< Vector3 > accelerations( m_boids.size(), Vector3::Zero );
	const float neighbourRadiusSquared = NEIGHBOUR_RADIUS * NEIGHBOUR_RADIUS;
	const float separationRadiusSquared = SEPARATION_RADIUS * SEPARATION_RADIUS;
	for ( size_t i = 0; i < m_boids.size(); ++i )
	{
		const Boid& boid = m_boids[ i ];
		Vector3 averagePosition = Vector3::Zero;
		Vector3 averageVelocity = Vector3::Zero;
		Vector3 separation = Vector3::Zero;
		size_t neighbourCount = 0;
		for ( size_t j = 0; j < m_boids.size(); ++j )
		{
			if ( i == j ) continue;
			const Vector3 offset = m_boids[ j ].position - boid.position;
			const float distanceSquared = offset.LengthSquared();
			if ( distanceSquared < separationRadiusSquared && distanceSquared > 0.0001f ) separation -= offset / distanceSquared;
			if ( m_boids[ j ].flockIndex != boid.flockIndex || distanceSquared >= neighbourRadiusSquared ) continue;
			averagePosition += m_boids[ j ].position;
			averageVelocity += m_boids[ j ].velocity;
			++neighbourCount;
		}
		if ( neighbourCount > 0 )
		{
			const float inverseCount = 1.0f / static_cast< float >( neighbourCount );
			accelerations[ i ] += ( averagePosition * inverseCount - boid.position ) * cohesion;
			accelerations[ i ] += ( averageVelocity * inverseCount - boid.velocity ) * alignment;
		}
		accelerations[ i ] += separation * closeRangeSeparation;
		accelerations[ i ] += Vector3( sinf( boid.wanderPhase ), sinf( boid.wanderPhase * 0.73f ), cosf( boid.wanderPhase * 0.91f ) ) * 0.75f;
		accelerations[ i ] += CalculateObstacleAvoidance( boid );
	}

	for ( size_t i = 0; i < m_boids.size(); ++i )
	{
		Boid& boid = m_boids[ i ];
		boid.velocity += LimitMagnitude( accelerations[ i ], 12.0f ) * timeStep;
		boid.wanderPhase += timeStep * ( 0.65f + static_cast< float >( boid.flockIndex ) * 0.08f );
		const float speed = boid.velocity.Length();
		const float desiredSpeed = boid.cruiseSpeed + sinf( boid.wanderPhase );
		if ( speed < MIN_SPEED ) boid.velocity = speed > 0.0001f ? boid.velocity * ( MIN_SPEED / speed ) : Vector3::UnitZ * MIN_SPEED;
		else if ( speed > MAX_SPEED ) boid.velocity *= MAX_SPEED / speed;
		else boid.velocity *= ( speed + ( desiredSpeed - speed ) * timeStep * 0.8f ) / speed;
		boid.position += boid.velocity * timeStep;
		ResolveBuildingCollision( boid );
		KeepBoidInFlightArea( boid );
	}
	ResolveBoidCollisions();
	for ( Boid& boid : m_boids )
	{
		// Pair separation can move a ball into a nearby wall, so finish with one
		// static-world correction pass.
		ResolveBuildingCollision( boid );
		KeepBoidInFlightArea( boid );
	}
}

void FlockManager::OnRender( cdp_framework::RenderContextPtr& renderContext )
{
	for ( const Boid& boid : m_boids ) renderContext->RenderPrimitive( m_boidShape, Vector3::One, boid.position, Vector3::Zero, Colors::Yellow );
}

void FlockManager::OnShutdown()
{
	m_boidShape.reset();
	m_boids.clear();
}

bool FlockManager::FindClosestYellowBall( const Vector3& position, Vector3& outPosition ) const
{
	if ( m_boids.empty() ) return false;

	const Boid* closestBoid = &m_boids.front();
	float closestDistanceSquared = ( closestBoid->position - position ).LengthSquared();
	for ( const Boid& boid : m_boids )
	{
		const float distanceSquared = ( boid.position - position ).LengthSquared();
		if ( distanceSquared < closestDistanceSquared )
		{
			closestDistanceSquared = distanceSquared;
			closestBoid = &boid;
		}
	}
	outPosition = closestBoid->position;
	return true;
}

bool FlockManager::ConsumeYellowBall( const Vector3& position, float consumeDistance )
{
	const float consumeDistanceSquared = consumeDistance * consumeDistance;
	for ( auto it = m_boids.begin(); it != m_boids.end(); ++it )
	{
		if ( ( it->position - position ).LengthSquared() <= consumeDistanceSquared )
		{
			m_boids.erase( it );
			return true;
		}
	}
	return false;
}

void FlockManager::ScatterYellowBalls( const Vector3& threatPosition, float scatterDistance )
{
	const float scatterDistanceSquared = scatterDistance * scatterDistance;
	for ( Boid& boid : m_boids )
	{
		Vector3 away = boid.position - threatPosition;
		const float distanceSquared = away.LengthSquared();
		if ( distanceSquared > 0.0001f && distanceSquared < scatterDistanceSquared )
		{
			away.Normalize();
			boid.velocity += away * ( 1.0f - sqrtf( distanceSquared ) / scatterDistance ) * 7.0f;
		}
	}
}

void FlockManager::AddToNearestFlock( const Vector3& position, const Vector3& velocity )
{
	Boid yellowBall;
	yellowBall.position = position;
	yellowBall.velocity = velocity;
	if ( yellowBall.velocity.LengthSquared() > 0.0001f ) yellowBall.velocity.Normalize();
	else yellowBall.velocity = Vector3::UnitZ;
	yellowBall.cruiseSpeed = 4.75f;
	yellowBall.velocity *= yellowBall.cruiseSpeed;
	yellowBall.wanderPhase = RandomRange( 0.0f, 6.28f );

	if ( m_boids.empty() )
	{
		yellowBall.flockIndex = 0;
		m_boids.push_back( yellowBall );
		return;
	}

	// The closest existing member provides a stable nearest-flock assignment.
	const Boid* closestBoid = &m_boids.front();
	float closestDistanceSquared = ( closestBoid->position - position ).LengthSquared();
	for ( const Boid& boid : m_boids )
	{
		const float distanceSquared = ( boid.position - position ).LengthSquared();
		if ( distanceSquared < closestDistanceSquared )
		{
			closestDistanceSquared = distanceSquared;
			closestBoid = &boid;
		}
	}
	yellowBall.flockIndex = closestBoid->flockIndex;
	m_boids.push_back( yellowBall );
}

void FlockManager::BounceProjectileOffBuildings( Vector3& position, Vector3& velocity, float radius ) const
{
	constexpr float restitution = 0.82f;
	for ( const Skyscraper& skyscraper : m_city.GetSkyscrapers() )
	{
		const Vector3 halfSize( skyscraper.width * 0.5f + radius, skyscraper.height * 0.5f + radius, skyscraper.length * 0.5f + radius );
		const Vector3 minimum = skyscraper.position - halfSize;
		const Vector3 maximum = skyscraper.position + halfSize;
		if ( position.x < minimum.x || position.x > maximum.x || position.y < minimum.y || position.y > maximum.y || position.z < minimum.z || position.z > maximum.z ) continue;

		const float distances[] = { position.x - minimum.x, maximum.x - position.x, position.y - minimum.y, maximum.y - position.y, position.z - minimum.z, maximum.z - position.z };
		const size_t closestFace = static_cast< size_t >( std::min_element( distances, distances + 6 ) - distances );
		Vector3 collisionNormal = Vector3::Zero;
		switch ( closestFace )
		{
		case 0: position.x = minimum.x; collisionNormal = -Vector3::UnitX; break;
		case 1: position.x = maximum.x; collisionNormal = Vector3::UnitX; break;
		case 2: position.y = minimum.y; collisionNormal = -Vector3::UnitY; break;
		case 3: position.y = maximum.y; collisionNormal = Vector3::UnitY; break;
		case 4: position.z = minimum.z; collisionNormal = -Vector3::UnitZ; break;
		default: position.z = maximum.z; collisionNormal = Vector3::UnitZ; break;
		}

		// Buildings are static/infinite-mass bodies. Reflect only the velocity
		// directed into the surface; restitution models the kinetic energy lost.
		const float normalVelocity = velocity.x * collisionNormal.x + velocity.y * collisionNormal.y + velocity.z * collisionNormal.z;
		if ( normalVelocity < 0.0f ) velocity -= collisionNormal * ( ( 1.0f + restitution ) * normalVelocity );
	}
}

Vector3 FlockManager::CalculateObstacleAvoidance( const Boid& boid ) const
{
	Vector3 avoidance = Vector3::Zero;
	Vector3 direction = boid.velocity;
	if ( direction.LengthSquared() > 0.0001f ) direction.Normalize();
	const Vector3 futurePosition = boid.position + direction * 3.0f;
	const float avoidanceDistance = 2.0f + BOID_RADIUS;
	for ( const Skyscraper& skyscraper : m_city.GetSkyscrapers() )
	{
		const Vector3 halfSize( skyscraper.width * 0.5f, skyscraper.height * 0.5f, skyscraper.length * 0.5f );
		const Vector3 minimum = skyscraper.position - halfSize;
		const Vector3 maximum = skyscraper.position + halfSize;
		const Vector3 closest( Clamp( futurePosition.x, minimum.x, maximum.x ), Clamp( futurePosition.y, minimum.y, maximum.y ), Clamp( futurePosition.z, minimum.z, maximum.z ) );
		const Vector3 away = futurePosition - closest;
		const float distanceSquared = away.LengthSquared();
		if ( distanceSquared > 0.0001f && distanceSquared < avoidanceDistance * avoidanceDistance ) avoidance += away * ( ( avoidanceDistance - sqrtf( distanceSquared ) ) / sqrtf( distanceSquared ) ) * 8.0f;
	}
	const float boundaryMargin = 2.0f;
	if ( futurePosition.y < MIN_Y + boundaryMargin ) avoidance.y += ( MIN_Y + boundaryMargin - futurePosition.y ) * 5.0f;
	else if ( futurePosition.y > MAX_Y - boundaryMargin ) avoidance.y -= ( futurePosition.y - ( MAX_Y - boundaryMargin ) ) * 5.0f;
	if ( futurePosition.x < MIN_XZ + boundaryMargin ) avoidance.x += ( MIN_XZ + boundaryMargin - futurePosition.x ) * 3.0f;
	else if ( futurePosition.x > MAX_XZ - boundaryMargin ) avoidance.x -= ( futurePosition.x - ( MAX_XZ - boundaryMargin ) ) * 3.0f;
	if ( futurePosition.z < MIN_XZ + boundaryMargin ) avoidance.z += ( MIN_XZ + boundaryMargin - futurePosition.z ) * 3.0f;
	else if ( futurePosition.z > MAX_XZ - boundaryMargin ) avoidance.z -= ( futurePosition.z - ( MAX_XZ - boundaryMargin ) ) * 3.0f;
	return LimitMagnitude( avoidance, 16.0f );
}

void FlockManager::ResolveBuildingCollision( Boid& boid ) const
{
	for ( const Skyscraper& skyscraper : m_city.GetSkyscrapers() )
	{
		const Vector3 halfSize( skyscraper.width * 0.5f + BOID_RADIUS, skyscraper.height * 0.5f + BOID_RADIUS, skyscraper.length * 0.5f + BOID_RADIUS );
		const Vector3 minimum = skyscraper.position - halfSize;
		const Vector3 maximum = skyscraper.position + halfSize;
		if ( boid.position.x < minimum.x || boid.position.x > maximum.x || boid.position.y < minimum.y || boid.position.y > maximum.y || boid.position.z < minimum.z || boid.position.z > maximum.z ) continue;
		const float distances[] = { boid.position.x - minimum.x, maximum.x - boid.position.x, boid.position.y - minimum.y, maximum.y - boid.position.y, boid.position.z - minimum.z, maximum.z - boid.position.z };
		Vector3 collisionNormal = Vector3::Zero;
		switch ( static_cast< size_t >( std::min_element( distances, distances + 6 ) - distances ) )
		{
		case 0: boid.position.x = minimum.x; collisionNormal = -Vector3::UnitX; break;
		case 1: boid.position.x = maximum.x; collisionNormal = Vector3::UnitX; break;
		case 2: boid.position.y = minimum.y; collisionNormal = -Vector3::UnitY; break;
		case 3: boid.position.y = maximum.y; collisionNormal = Vector3::UnitY; break;
		case 4: boid.position.z = minimum.z; collisionNormal = -Vector3::UnitZ; break;
		default: boid.position.z = maximum.z; collisionNormal = Vector3::UnitZ; break;
		}
		const float normalVelocity = boid.velocity.x * collisionNormal.x + boid.velocity.y * collisionNormal.y + boid.velocity.z * collisionNormal.z;
		if ( normalVelocity < 0.0f ) boid.velocity -= collisionNormal * ( 1.55f * normalVelocity );
	}
}

void FlockManager::ResolveBoidCollisions()
{
	constexpr float contactDistance = BOID_RADIUS * 2.0f;
	constexpr float restitution = 0.35f;
	for ( size_t first = 0; first < m_boids.size(); ++first )
	{
		for ( size_t second = first + 1; second < m_boids.size(); ++second )
		{
			Boid& firstBoid = m_boids[ first ];
			Boid& secondBoid = m_boids[ second ];
			Vector3 collisionNormal = secondBoid.position - firstBoid.position;
			const float distanceSquared = collisionNormal.LengthSquared();
			if ( distanceSquared >= contactDistance * contactDistance ) continue;

			const float distance = sqrtf( distanceSquared );
			if ( distance > 0.0001f ) collisionNormal /= distance;
			else collisionNormal = Vector3::UnitX;

			// Equal-mass balls share positional correction and collision impulse.
			const float penetration = contactDistance - distance;
			firstBoid.position -= collisionNormal * ( penetration * 0.5f );
			secondBoid.position += collisionNormal * ( penetration * 0.5f );
			const Vector3 relativeVelocity = secondBoid.velocity - firstBoid.velocity;
			const float separatingVelocity = relativeVelocity.x * collisionNormal.x + relativeVelocity.y * collisionNormal.y + relativeVelocity.z * collisionNormal.z;
			if ( separatingVelocity < 0.0f )
			{
				const float impulse = -( 1.0f + restitution ) * separatingVelocity * 0.5f;
				firstBoid.velocity -= collisionNormal * impulse;
				secondBoid.velocity += collisionNormal * impulse;
			}
		}
	}
}

void FlockManager::KeepBoidInFlightArea( Boid& boid ) const
{
	if ( boid.position.x < MIN_XZ || boid.position.x > MAX_XZ ) { boid.position.x = Clamp( boid.position.x, MIN_XZ, MAX_XZ ); boid.velocity.x = -boid.velocity.x; }
	if ( boid.position.z < MIN_XZ || boid.position.z > MAX_XZ ) { boid.position.z = Clamp( boid.position.z, MIN_XZ, MAX_XZ ); boid.velocity.z = -boid.velocity.z; }
	if ( boid.position.y < MIN_Y || boid.position.y > MAX_Y ) { boid.position.y = Clamp( boid.position.y, MIN_Y, MAX_Y ); boid.velocity.y = -boid.velocity.y; }
}
