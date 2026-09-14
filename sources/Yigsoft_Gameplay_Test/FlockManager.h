/**
* Copyright (c) 2022 Yigsoft. All Rights Reserved.
*/

#pragma once
#include "IRenderContext.h"

class City;

class FlockManager final
{
public:
	FlockManager( const City& city );
	~FlockManager();

	void OnInitialize();
	void OnUpdate( float deltaTime );
	void OnRender( cdp_framework::RenderContextPtr& renderContext );
	void OnShutdown();
	bool FindClosestYellowBall( const Vector3& position, Vector3& outPosition ) const;
	bool ConsumeYellowBall( const Vector3& position, float consumeDistance );
	void ScatterYellowBalls( const Vector3& threatPosition, float scatterDistance );
	void AddToNearestFlock( const Vector3& position, const Vector3& velocity );
	void BounceProjectileOffBuildings( Vector3& position, Vector3& velocity, float radius ) const;
	bool IntersectsBuilding( const Vector3& position, float radius ) const;
	bool HasBoidWithin( const Vector3& position, float radius ) const;
	void FreezeBoids( const Vector3& position, float radius, float duration );

	// Tunable flock parameters. Each flock receives a random integer size in
	// the inclusive [minimumFlockSize, maximumFlockSize] range on initialization.
	size_t flockNumber = 4;
	int minimumFlockSize = 5;
	int maximumFlockSize = 12;
	float cohesion = 0.15f;
	float alignment = 0.55f;
	float closeRangeSeparation = 3.5f;

private:
	struct Boid final
	{
		Vector3 position;
		Vector3 velocity;
		float cruiseSpeed;
		float wanderPhase;
		float freezeRemaining = 0.0f;
		size_t flockIndex;
	};

	Vector3 CalculateObstacleAvoidance( const Boid& boid ) const;
	void ResolveBuildingCollision( Boid& boid ) const;
	void ResolveBoidCollisions();
	void KeepBoidInFlightArea( Boid& boid ) const;

	const City& m_city;
	std::vector< Boid > m_boids;
	PrimitivePtr m_boidShape;
};
