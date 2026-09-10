/**
* Copyright (c) 2022 Yigsoft. All Rights Reserved.
*/

#pragma once
#include "IRenderContext.h"

struct Skyscraper final
{
	std::unique_ptr< DirectX::GeometricPrimitive > shape;
	Vector3 position;
	float width;
	float length;
	float height;
};

struct Boid final
{
	Vector3 position;
	Vector3 velocity;
	float cruiseSpeed;
	float wanderPhase;
	size_t flockIndex;
};

class City
{

public:
	City();
	~City();

	void OnInitialize();
	void OnUpdate( float deltaTime );
	void OnRender( cdp_framework::RenderContextPtr& renderContext );
	void OnShutdown();

private:
    void Load();
	Vector3 CalculateObstacleAvoidance( const Boid& boid ) const;
	void ResolveBuildingCollision( Boid& boid ) const;
	void KeepBoidInFlightArea( Boid& boid ) const;

	std::vector< Skyscraper > m_skyscrapers;
	std::vector< Boid > m_boids;
	PrimitivePtr m_boidShape;
};

