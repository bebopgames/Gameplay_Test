/**
* Copyright (c) 2022 Yigsoft. All Rights Reserved.
*/

#pragma once

class City;

class Camera
{

public:
	Camera( const City& city );
	~Camera();
	void OnUpdate( float deltaTime, DirectX::Keyboard& keyboard, DirectX::Mouse& mouse, DirectX::GamePad& gamepad );
	Vector3 GetPosition() const;
	Vector3 GetForward() const;
	void SetMovementSpeedMultiplier( float multiplier );

private:
	void RotationInput( DirectX::Mouse& mouse, DirectX::GamePad& gamepad );
	Vector3 MovementInput( DirectX::Keyboard& keyboard, DirectX::GamePad& gamepad );
	void ResolveCollisions();

	const City& m_city;
	Vector3 m_cameraPos;
	Vector3 m_localMovementVelocity;
	float m_pitch;
	float m_yaw;
	float m_movementSpeedMultiplier = 1.0f;

};

