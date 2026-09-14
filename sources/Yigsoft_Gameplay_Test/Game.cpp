/**
* Copyright (c) 2022 Yigsoft. All Rights Reserved.
*/

#include "pch.h"
#include "Game.h"
#include "Engine.h"

namespace
{
	uint32_t s_pickupRandomState = 0xB01DFACEu;

	float RandomRange( const float minimum, const float maximum )
	{
		s_pickupRandomState = s_pickupRandomState * 1664525u + 1013904223u;
		const float unit = static_cast< float >( s_pickupRandomState >> 8 ) / static_cast< float >( 0x01000000u );
		return minimum + ( maximum - minimum ) * unit;
	}

	constexpr float PICKUP_COLLISION_RADIUS = 0.45f;

	std::string FormatCountdown( float remainingSeconds )
	{
		const int totalSeconds = std::max( 0, static_cast< int >( remainingSeconds + 0.999f ) );
		const int minutes = totalSeconds / 60;
		const int seconds = totalSeconds % 60;
		return "Time: " + std::to_string( minutes ) + ":" + ( seconds < 10 ? "0" : "" ) + std::to_string( seconds );
	}
}

Game::Game()
{
    m_city = std::make_unique< City >();
    m_camera = std::make_unique< Camera >( *m_city );
    m_crosshair = std::make_unique< Crosshair >();
	m_flockManager = std::make_unique< FlockManager >( *m_city );
}

Game::~Game() = default;

void Game::OnInitialize()
{
	m_score = 0;
	m_pickup.reset();
	m_pickupSpawnTimer = 0.0f;
	m_pickupSpawnDelay = RandomRange( 10.0f, 20.0f );
	m_primaryProjectileSpeedMultiplier = 1.0f;
	m_camera->SetMovementSpeedMultiplier( 1.0f );
	m_speedBoostRemaining = 0.0f;
	m_gameTimeRemaining = 120.0f;
	m_isGameOver = false;
    m_city->OnInitialize();
	m_flockManager->OnInitialize();
    m_crosshair->OnInitialize();

	// Your code here


}

void Game::OnUpdate( float deltaTime, DirectX::Keyboard& keyboard, DirectX::Mouse& mouse, DirectX::GamePad& gamepad )
{
	if ( m_isGameOver )
	{
		if ( keyboard.GetState().Enter ) RestartGame();
		return;
	}

	const float timeStep = std::max( 0.0f, deltaTime );
	m_gameTimeRemaining -= timeStep;
	if ( m_gameTimeRemaining <= 0.0f )
	{
		m_gameTimeRemaining = 0.0f;
		m_isGameOver = true;
		return;
	}

	if ( m_speedBoostRemaining > 0.0f )
	{
		m_speedBoostRemaining -= timeStep;
		if ( m_speedBoostRemaining <= 0.0f )
		{
			m_speedBoostRemaining = 0.0f;
			m_camera->SetMovementSpeedMultiplier( 1.0f );
			SetPrimaryProjectileSpeedMultiplier( 1.0f );
		}
	}

	m_camera->OnUpdate( deltaTime, keyboard, mouse, gamepad );
	m_city->OnUpdate( deltaTime );
	m_flockManager->OnUpdate( deltaTime );
	UpdatePickup( deltaTime );

	const auto mouseState = mouse.GetState();
	if ( mouseState.leftButton && !m_leftMouseWasDown )
	{
		const Vector3 direction = m_camera->GetForward();
		m_projectiles.push_back( std::make_unique< Projectile_Primary >( m_camera->GetPosition() + direction, direction, m_primaryProjectileSpeedMultiplier ) );
	}
	m_leftMouseWasDown = mouseState.leftButton;

	for ( const auto& projectile : m_projectiles )
	{
		m_score += projectile->OnUpdate( deltaTime, *m_flockManager, m_pickup.get() );
		if ( projectile->TakePickupEffect() == PickupEffect::Speed )
		{
			m_camera->SetMovementSpeedMultiplier( 1.2f );
			SetPrimaryProjectileSpeedMultiplier( 1.5f );
			m_speedBoostRemaining = 10.0f;
		}
	}
	m_projectiles.erase( std::remove_if( m_projectiles.begin(), m_projectiles.end(), []( const std::unique_ptr< Projectile >& projectile ) { return !projectile->IsActive(); } ), m_projectiles.end() );
    m_crosshair->OnUpdate( deltaTime, mouse, gamepad );

	// Your code here


}

void Game::OnRender( cdp_framework::RenderContextPtr& renderContext )
{
	m_city->OnRender( renderContext );
	m_flockManager->OnRender( renderContext );
	if ( m_pickup ) m_pickup->OnRender( renderContext );
	for ( const auto& projectile : m_projectiles )
	{
		projectile->OnRender( renderContext );
	}
    m_crosshair->OnRender( renderContext );
    Vector2 windowSize = GetEngine().GetWindowSize();
	renderContext->RenderTextCentered( FormatCountdown( m_gameTimeRemaining ), Vector2( windowSize.x * 0.5f, 10.0f ), 1.0f, Colors::White );
	renderContext->RenderTextRightAligned( "Score: " + std::to_string( m_score ), Vector2( windowSize.x - 10.0f, 10.0f ), 1.0f, Colors::White );
	renderContext->RenderText( "CD Projekt RED Gameplay Test", Vector2( 10, ( windowSize.y - 40 ) ), 1, Colors::DarkRed );
	if ( m_isGameOver )
	{
		renderContext->RenderTextCentered( "Game over", Vector2( windowSize.x * 0.5f, windowSize.y * 0.5f - 25.0f ), 1.5f, Colors::White );
		renderContext->RenderTextCentered( "Score: " + std::to_string( m_score ), Vector2( windowSize.x * 0.5f, windowSize.y * 0.5f + 15.0f ), 1.0f, Colors::White );
		renderContext->RenderTextCentered( "Hit Enter To Restart", Vector2( windowSize.x * 0.5f, windowSize.y * 0.5f + 45.0f ), 1.0f, Colors::White );
	}

	// Your code here


}

void Game::OnShutdown()
{
	m_city->OnShutdown();
	m_flockManager->OnShutdown();
	m_projectiles.clear();
	m_pickup.reset();
    m_crosshair->OnShutdown();

	// Your code here


}

void Game::UpdatePickup( float deltaTime )
{
	const float timeStep = std::max( 0.0f, deltaTime );
	if ( m_pickup )
	{
		m_pickup->OnUpdate( timeStep, *m_flockManager );
		if ( m_pickup->IsActive() ) return;

		m_pickup.reset();
		m_pickupSpawnTimer = 0.0f;
		m_pickupSpawnDelay = RandomRange( 10.0f, 20.0f );
		return;
	}

	m_pickupSpawnTimer += timeStep;
	if ( m_pickupSpawnTimer < m_pickupSpawnDelay ) return;

	for ( int attempt = 0; attempt < 32; ++attempt )
	{
		const Vector3 position( RandomRange( -18.0f, 18.0f ), RandomRange( 4.0f, 22.0f ), RandomRange( -18.0f, 18.0f ) );
		if ( m_flockManager->IntersectsBuilding( position, PICKUP_COLLISION_RADIUS ) ) continue;

		if ( RandomRange( 0.0f, 1.0f ) < 0.5f ) m_pickup = std::make_unique< Pickup_Freeze >( position );
		else m_pickup = std::make_unique< Pickup_Speed >( position );
		return;
	}
}

void Game::SetPrimaryProjectileSpeedMultiplier( float multiplier )
{
	m_primaryProjectileSpeedMultiplier = multiplier;
	for ( const auto& projectile : m_projectiles )
	{
		if ( Projectile_Primary* primaryProjectile = dynamic_cast< Projectile_Primary* >( projectile.get() ) ) primaryProjectile->SetSpeedMultiplier( m_primaryProjectileSpeedMultiplier );
	}
}

void Game::RestartGame()
{
	OnShutdown();
	OnInitialize();
	m_leftMouseWasDown = false;
}
