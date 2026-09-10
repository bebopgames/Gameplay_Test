/**
* Copyright (c) 2022 Yigsoft. All Rights Reserved.
*/

#include "pch.h"
#include "City.h"
#include "Engine.h"
#include "StepTimer.h"
#include "DeviceResources.h"

namespace
{
	constexpr size_t FLOCK_COUNT = 3;
	constexpr size_t BOIDS_PER_FLOCK = 16;
	constexpr size_t BOID_COUNT = FLOCK_COUNT * BOIDS_PER_FLOCK;
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
		const float unit = static_cast< float >( s_randomState >> 8 ) / static_cast< float >( 0x00FFFFFFu );
		return minimum + ( maximum - minimum ) * unit;
	}

	float Clamp( const float value, const float minimum, const float maximum )
	{
		return std::max( minimum, std::min( value, maximum ) );
	}

	Vector3 LimitMagnitude( const Vector3& value, const float maximum )
	{
		const float lengthSquared = value.LengthSquared();
		if ( lengthSquared > maximum * maximum )
		{
			return value * ( maximum / sqrtf( lengthSquared ) );
		}
		return value;
	}
}

City::City() = default;
City::~City() = default;

void City::OnInitialize()
{
    Load();
}

void City::Load()
{
	OnShutdown();

	std::ifstream stream( "../../data/city/city.json" );
	std::string fileData( ( std::istreambuf_iterator< char >( stream ) ), std::istreambuf_iterator< char >() );

	Json::Document document;
	document.Parse( fileData.c_str() );
	assert( !document.HasParseError() );

	assert( document.IsObject() );

	// skyscrapers
	assert( document.HasMember( "skyscrapers" ) );
	const Json::Value& arrayObject = document[ "skyscrapers" ];
	assert( arrayObject.IsArray() );

	for ( Json::SizeType i = 0; i < arrayObject.Size(); i++ )
	{
		Skyscraper newSkyscraper;

		// pos_x
		assert( arrayObject[ i ].HasMember( "pos_x" ) );
		assert( arrayObject[ i ][ "pos_x" ].IsFloat() );
		newSkyscraper.position.x = arrayObject[ i ][ "pos_x" ].GetFloat();

		// pos_z
		assert( arrayObject[ i ].HasMember( "pos_z" ) );
		assert( arrayObject[ i ][ "pos_z" ].IsFloat() );
		newSkyscraper.position.z = arrayObject[ i ][ "pos_z" ].GetFloat();

		// width
		assert( arrayObject[ i ].HasMember( "width" ) );
		assert( arrayObject[ i ][ "width" ].IsFloat() );
		newSkyscraper.width = arrayObject[ i ][ "width" ].GetFloat();

		// length
		assert( arrayObject[ i ].HasMember( "length" ) );
		assert( arrayObject[ i ][ "length" ].IsFloat() );
		newSkyscraper.length = arrayObject[ i ][ "length" ].GetFloat();

		// height
		assert( arrayObject[ i ].HasMember( "height" ) );
		assert( arrayObject[ i ][ "height" ].IsFloat() );
		newSkyscraper.height = arrayObject[ i ][ "height" ].GetFloat();

		newSkyscraper.position.y = newSkyscraper.height * 0.5f;

		m_skyscrapers.push_back( std::move( newSkyscraper ) );
	}

	// create primitives
	for( Skyscraper& skyscraper : m_skyscrapers )
	{
		skyscraper.shape = GetEngine().CreateBoxPrimitive( Vector3( skyscraper.width, skyscraper.height, skyscraper.length ) );
	}

	// A single sphere mesh is shared by every boid. Each flock starts on a
	// different set of streets, which keeps the groups visually distinct.
	m_boidShape = GetEngine().CreateSpherePrimitive( BOID_RADIUS );
	m_boids.reserve( BOID_COUNT );
	for ( size_t i = 0; i < BOID_COUNT; ++i )
	{
		const size_t flockIndex = i / BOIDS_PER_FLOCK;
		const size_t boidIndex = i % BOIDS_PER_FLOCK;
		const float flockX[] = { -12.0f, 12.0f, -4.0f };
		const float flockZ[] = { -12.0f, -4.0f, 12.0f };
		const float flockY[] = { 10.0f, 16.0f, 19.0f };
		Boid boid;
		boid.flockIndex = flockIndex;
		boid.position = Vector3( flockX[ flockIndex ] + RandomRange( -2.0f, 2.0f ), flockY[ flockIndex ] + RandomRange( -2.0f, 2.0f ), flockZ[ flockIndex ] + RandomRange( -2.0f, 2.0f ) );
		boid.velocity = Vector3( RandomRange( -1.0f, 1.0f ), RandomRange( -0.25f, 0.25f ), RandomRange( -1.0f, 1.0f ) );
		boid.velocity.Normalize();
		boid.cruiseSpeed = RandomRange( 4.25f, 5.5f );
		boid.wanderPhase = static_cast< float >( boidIndex ) * 0.63f + static_cast< float >( flockIndex ) * 1.97f;
		boid.velocity *= boid.cruiseSpeed;
		m_boids.push_back( boid );
	}
}

void City::OnUpdate( float deltaTime )
{
	const float timeStep = std::min( deltaTime, 0.05f );
	if ( timeStep <= 0.0f )
	{
		return;
	}

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
			if ( i == j )
			{
				continue;
			}

			const Vector3 offset = m_boids[ j ].position - boid.position;
			const float distanceSquared = offset.LengthSquared();
			if ( distanceSquared < separationRadiusSquared && distanceSquared > 0.0001f )
			{
				// Separation is global: adjacent flocks do not visibly interpenetrate.
				separation -= offset / distanceSquared;
			}

			if ( m_boids[ j ].flockIndex != boid.flockIndex || distanceSquared >= neighbourRadiusSquared )
			{
				continue;
			}

			averagePosition += m_boids[ j ].position;
			averageVelocity += m_boids[ j ].velocity;
			++neighbourCount;

		}

		if ( neighbourCount > 0 )
		{
			const float inverseCount = 1.0f / static_cast< float >( neighbourCount );
			// Cohesion, alignment, and separation are deliberately evaluated from
			// the same frame's state so update order cannot bias the flock.
			accelerations[ i ] += ( averagePosition * inverseCount - boid.position ) * 0.35f;
			accelerations[ i ] += ( averageVelocity * inverseCount - boid.velocity ) * 0.55f;
		}
		accelerations[ i ] += separation * 3.5f;

		// Low-frequency wandering prevents the flock from looking mechanical while
		// preserving its shared flocking response.
		accelerations[ i ] += Vector3( sinf( boid.wanderPhase ), sinf( boid.wanderPhase * 0.73f ), cosf( boid.wanderPhase * 0.91f ) ) * 0.75f;

		accelerations[ i ] += CalculateObstacleAvoidance( boid );
	}

	for ( size_t i = 0; i < m_boids.size(); ++i )
	{
		Boid& boid = m_boids[ i ];
		boid.velocity += LimitMagnitude( accelerations[ i ], 12.0f ) * timeStep;
		boid.wanderPhase += timeStep * ( 0.65f + static_cast< float >( boid.flockIndex ) * 0.08f );

		const float speed = boid.velocity.Length();
		const float desiredSpeed = boid.cruiseSpeed + sinf( boid.wanderPhase ) * 1.0f;
		if ( speed < MIN_SPEED )
		{
			boid.velocity = speed > 0.0001f ? boid.velocity * ( MIN_SPEED / speed ) : Vector3::UnitZ * MIN_SPEED;
		}
		else if ( speed > MAX_SPEED )
		{
			boid.velocity *= MAX_SPEED / speed;
		}
		else if ( speed > 0.0001f )
		{
			// Smoothly change speed instead of moving at a fixed game-like rate.
			const float adjustedSpeed = speed + ( desiredSpeed - speed ) * timeStep * 0.8f;
			boid.velocity *= adjustedSpeed / speed;
		}

		boid.position += boid.velocity * timeStep;
		ResolveBuildingCollision( boid );
		KeepBoidInFlightArea( boid );
	}
}

void City::OnRender( cdp_framework::RenderContextPtr& renderContext )
{
	for ( Skyscraper& skyscraper : m_skyscrapers )
	{
		renderContext->RenderPrimitive( skyscraper.shape, Vector3::One, skyscraper.position, Vector3::Zero, Colors::BlueViolet );
	}

	for ( const Boid& boid : m_boids )
	{
		renderContext->RenderPrimitive( m_boidShape, Vector3::One, boid.position, Vector3::Zero, Colors::Yellow );
	}
}

void City::OnShutdown()
{
	m_boidShape.reset();
	m_boids.clear();

	for ( Skyscraper& skyscraper : m_skyscrapers )
	{
		skyscraper.shape.reset();
	}

	m_skyscrapers.clear();
}

Vector3 City::CalculateObstacleAvoidance( const Boid& boid ) const
{
	Vector3 avoidance = Vector3::Zero;
	Vector3 direction = boid.velocity;
	if ( direction.LengthSquared() > 0.0001f )
	{
		direction.Normalize();
	}
	const Vector3 futurePosition = boid.position + direction * 3.0f;
	const float avoidanceDistance = 2.0f + BOID_RADIUS;

	for ( const Skyscraper& skyscraper : m_skyscrapers )
	{
		const Vector3 halfSize( skyscraper.width * 0.5f, skyscraper.height * 0.5f, skyscraper.length * 0.5f );
		const Vector3 minimum = skyscraper.position - halfSize;
		const Vector3 maximum = skyscraper.position + halfSize;
		const Vector3 closest(
			Clamp( futurePosition.x, minimum.x, maximum.x ),
			Clamp( futurePosition.y, minimum.y, maximum.y ),
			Clamp( futurePosition.z, minimum.z, maximum.z ) );
		const Vector3 away = futurePosition - closest;
		const float distanceSquared = away.LengthSquared();
		if ( distanceSquared > 0.0001f && distanceSquared < avoidanceDistance * avoidanceDistance )
		{
			avoidance += away * ( ( avoidanceDistance - sqrtf( distanceSquared ) ) / sqrtf( distanceSquared ) ) * 8.0f;
		}
	}

	// The floor and flight-area limits are steering obstacles too, not merely
	// post-movement clamps. This makes a low-flying ball curve upward naturally.
	const float boundaryMargin = 2.0f;
	if ( futurePosition.y < MIN_Y + boundaryMargin )
	{
		avoidance.y += ( MIN_Y + boundaryMargin - futurePosition.y ) * 5.0f;
	}
	else if ( futurePosition.y > MAX_Y - boundaryMargin )
	{
		avoidance.y -= ( futurePosition.y - ( MAX_Y - boundaryMargin ) ) * 5.0f;
	}
	if ( futurePosition.x < MIN_XZ + boundaryMargin )
	{
		avoidance.x += ( MIN_XZ + boundaryMargin - futurePosition.x ) * 3.0f;
	}
	else if ( futurePosition.x > MAX_XZ - boundaryMargin )
	{
		avoidance.x -= ( futurePosition.x - ( MAX_XZ - boundaryMargin ) ) * 3.0f;
	}
	if ( futurePosition.z < MIN_XZ + boundaryMargin )
	{
		avoidance.z += ( MIN_XZ + boundaryMargin - futurePosition.z ) * 3.0f;
	}
	else if ( futurePosition.z > MAX_XZ - boundaryMargin )
	{
		avoidance.z -= ( futurePosition.z - ( MAX_XZ - boundaryMargin ) ) * 3.0f;
	}

	return LimitMagnitude( avoidance, 16.0f );
}

void City::ResolveBuildingCollision( Boid& boid ) const
{
	for ( const Skyscraper& skyscraper : m_skyscrapers )
	{
		const Vector3 halfSize( skyscraper.width * 0.5f + BOID_RADIUS, skyscraper.height * 0.5f + BOID_RADIUS, skyscraper.length * 0.5f + BOID_RADIUS );
		const Vector3 minimum = skyscraper.position - halfSize;
		const Vector3 maximum = skyscraper.position + halfSize;
		if ( boid.position.x < minimum.x || boid.position.x > maximum.x || boid.position.y < minimum.y || boid.position.y > maximum.y || boid.position.z < minimum.z || boid.position.z > maximum.z )
		{
			continue;
		}

		const float distances[] =
		{
			boid.position.x - minimum.x, maximum.x - boid.position.x,
			boid.position.y - minimum.y, maximum.y - boid.position.y,
			boid.position.z - minimum.z, maximum.z - boid.position.z
		};
		const size_t closestFace = static_cast< size_t >( std::min_element( distances, distances + 6 ) - distances );
		switch ( closestFace )
		{
		case 0: boid.position.x = minimum.x; boid.velocity.x = -fabsf( boid.velocity.x ); break;
		case 1: boid.position.x = maximum.x; boid.velocity.x = fabsf( boid.velocity.x ); break;
		case 2: boid.position.y = minimum.y; boid.velocity.y = -fabsf( boid.velocity.y ); break;
		case 3: boid.position.y = maximum.y; boid.velocity.y = fabsf( boid.velocity.y ); break;
		case 4: boid.position.z = minimum.z; boid.velocity.z = -fabsf( boid.velocity.z ); break;
		default: boid.position.z = maximum.z; boid.velocity.z = fabsf( boid.velocity.z ); break;
		}
	}
}

void City::KeepBoidInFlightArea( Boid& boid ) const
{
	if ( boid.position.x < MIN_XZ || boid.position.x > MAX_XZ )
	{
		boid.position.x = Clamp( boid.position.x, MIN_XZ, MAX_XZ );
		boid.velocity.x = -boid.velocity.x;
	}
	if ( boid.position.z < MIN_XZ || boid.position.z > MAX_XZ )
	{
		boid.position.z = Clamp( boid.position.z, MIN_XZ, MAX_XZ );
		boid.velocity.z = -boid.velocity.z;
	}
	if ( boid.position.y < MIN_Y || boid.position.y > MAX_Y )
	{
		boid.position.y = Clamp( boid.position.y, MIN_Y, MAX_Y );
		boid.velocity.y = -boid.velocity.y;
	}
}
