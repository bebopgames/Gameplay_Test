/**
* Copyright (c) 2022 Yigsoft. All Rights Reserved.
*/

#pragma once
#include "IRenderContext.h"

class FlockManager;

class Projectile
{
public:
	Projectile( const Vector3& position, const Vector3& velocity );
	virtual ~Projectile();

	virtual void OnUpdate( float deltaTime, FlockManager& flockManager ) = 0;
	virtual void OnRender( cdp_framework::RenderContextPtr& renderContext ) = 0;
	bool IsActive() const;

protected:
	Vector3 m_position;
	Vector3 m_velocity;
	bool m_isActive = true;
};
