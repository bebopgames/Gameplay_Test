/**
* Copyright (c) 2022 Yigsoft. All Rights Reserved.
*/

#include "pch.h"
#include "Camera.h"
#include "City.h"
#include "IEngine.h"

namespace
{
	const Vector3 START_POSITION = { 0.0f, 6.0f, -10.5f };
	constexpr float ROTATION_GAIN = 0.05f;
	// The old frame-based gain was 0.05 units at 60 Hz (3 units/second).
	constexpr float MAX_MOVEMENT_SPEED = 6.0f;
	constexpr float MOVEMENT_ACCELERATION = 18.0f;
	constexpr float MOVEMENT_DECELERATION = 22.0f;
	constexpr float CAMERA_RADIUS = 0.4f;
	constexpr float MIN_XZ = -21.0f + CAMERA_RADIUS;
	constexpr float MAX_XZ = 21.0f - CAMERA_RADIUS;
	constexpr float MIN_Y = CAMERA_RADIUS;
	constexpr float MAX_Y = 25.0f - CAMERA_RADIUS;
}

Camera::Camera( const City& city ) :
	m_city( city ),
	m_pitch( 0 ),
	m_yaw( 0 ),
	m_cameraPos( START_POSITION ),
	m_localMovementVelocity( Vector3::Zero )
{
}

Camera::~Camera() = default;

Vector3 Camera::GetPosition() const
{
	return m_cameraPos;
}

Vector3 Camera::GetForward() const
{
	const float r = cosf( m_pitch );
	return Vector3( r * sinf( m_yaw ), sinf( m_pitch ), r * cosf( m_yaw ) );
}

void Camera::SetMovementSpeedMultiplier( float multiplier )
{
	m_movementSpeedMultiplier = std::max( 0.0f, multiplier );
}

void Camera::OnUpdate( float deltaTime, DirectX::Keyboard& keyboard, DirectX::Mouse& mouse, DirectX::GamePad& gamepad )
{
	cdp_framework::IEngine& engine = GetEngine();

    mouse.SetMode( Mouse::MODE_RELATIVE );

	RotationInput( mouse, gamepad );
	Vector3 movementInput = MovementInput( keyboard, gamepad );
	if ( movementInput.LengthSquared() > 1.0f )
	{
		movementInput.Normalize();
	}

	const Vector3 targetVelocity = movementInput * ( MAX_MOVEMENT_SPEED * m_movementSpeedMultiplier );
	const float response = movementInput.LengthSquared() > 0.0f ? MOVEMENT_ACCELERATION : MOVEMENT_DECELERATION;
	const float blend = std::min( response * deltaTime, 1.0f );
	m_localMovementVelocity = Vector3::Lerp( m_localMovementVelocity, targetVelocity, blend );

	const float limit = M_PI / 2.0f - 0.01f;
	m_pitch = std::max( -limit, std::min( +limit, m_pitch ) );

	if ( m_yaw > M_PI )
	{
		m_yaw -= M_PI * 2.f;
	}
	else if ( m_yaw < -M_PI )
	{
		m_yaw += M_PI * 2.f;
	}

	Quaternion q = Quaternion::CreateFromYawPitchRoll( m_yaw, m_pitch, 0.f );
	Vector3 move = Vector3::Transform( m_localMovementVelocity, q );
	move.y *= -1.0f;
	move *= deltaTime;

	m_cameraPos += move;
	ResolveCollisions();

	float y = sinf( m_pitch );
	float r = cosf( m_pitch );
	float z = r * cosf( m_yaw );
	float x = r * sinf( m_yaw );

	Vector3 lookAt = m_cameraPos + Vector3( x, y, z );
	engine.LookAt( m_cameraPos, lookAt );
}

void Camera::ResolveCollisions()
{
	// Keep the camera inside the same room occupied by the flock and projectiles.
	m_cameraPos.x = std::max( MIN_XZ, std::min( m_cameraPos.x, MAX_XZ ) );
	m_cameraPos.y = std::max( MIN_Y, std::min( m_cameraPos.y, MAX_Y ) );
	m_cameraPos.z = std::max( MIN_XZ, std::min( m_cameraPos.z, MAX_XZ ) );

	for ( const Skyscraper& skyscraper : m_city.GetSkyscrapers() )
	{
		const Vector3 halfSize( skyscraper.width * 0.5f + CAMERA_RADIUS, skyscraper.height * 0.5f + CAMERA_RADIUS, skyscraper.length * 0.5f + CAMERA_RADIUS );
		const Vector3 minimum = skyscraper.position - halfSize;
		const Vector3 maximum = skyscraper.position + halfSize;
		if ( m_cameraPos.x < minimum.x || m_cameraPos.x > maximum.x || m_cameraPos.y < minimum.y || m_cameraPos.y > maximum.y || m_cameraPos.z < minimum.z || m_cameraPos.z > maximum.z ) continue;

		const float distances[] =
		{
			m_cameraPos.x - minimum.x, maximum.x - m_cameraPos.x,
			m_cameraPos.y - minimum.y, maximum.y - m_cameraPos.y,
			m_cameraPos.z - minimum.z, maximum.z - m_cameraPos.z
		};
		switch ( static_cast< size_t >( std::min_element( distances, distances + 6 ) - distances ) )
		{
		case 0: m_cameraPos.x = minimum.x; break;
		case 1: m_cameraPos.x = maximum.x; break;
		case 2: m_cameraPos.y = minimum.y; break;
		case 3: m_cameraPos.y = maximum.y; break;
		case 4: m_cameraPos.z = minimum.z; break;
		default: m_cameraPos.z = maximum.z; break;
		}
	}
}

void Camera::RotationInput( DirectX::Mouse& mouse, DirectX::GamePad& gamepad )
{
	// GamePad
	auto padState = gamepad.GetState( 0 );
	if ( padState.IsConnected() )
	{

		if ( padState.IsRightStickPressed() )
		{
			m_yaw = m_pitch = 0.f;
		}
		else
		{
			m_yaw += -padState.thumbSticks.rightX * ROTATION_GAIN;
			m_pitch += padState.thumbSticks.rightY * ROTATION_GAIN;
		}
	}

	// Mouse
	auto mouseState = mouse.GetState();
	if ( mouseState.positionMode == DirectX::Mouse::MODE_RELATIVE )
	{
		Vector3 delta = Vector3( float( mouseState.x ), float( mouseState.y ), 0.f )
			* ROTATION_GAIN;

		m_pitch -= delta.y * ROTATION_GAIN;
		m_yaw -= delta.x * ROTATION_GAIN;
	}
}

Vector3 Camera::MovementInput( DirectX::Keyboard& keyboard, DirectX::GamePad& gamepad )
{
	Vector3 move = Vector3::Zero;

	// GamePad
	auto padState = gamepad.GetState( 0 );
	if ( padState.IsConnected() )
	{
		move.x += -padState.thumbSticks.leftX;
		move.z += padState.thumbSticks.leftY;
	}

	// Keyboard
    auto kb = keyboard.GetState();
	if ( kb.Up || kb.W )
		move.z += 1.f;

	if ( kb.Down || kb.S )
		move.z -= 1.f;

	if ( kb.Left || kb.A )
		move.x += 1.f;

	if ( kb.Right || kb.D )
		move.x -= 1.f;

	if ( kb.PageUp || kb.Space )
		move.y += 1.f;

	if ( kb.PageDown || kb.X )
		move.y -= 1.f;

	return move;
}
