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
	LoadSettings();
	m_score = 0;
	m_secondaryProjectileAmmo = 0;
	m_pickup.reset();
	m_pickupSpawnTimer = 0.0f;
	m_pickupSpawnDelay = RandomRange( m_pickupSpawnIntervalMinimum, m_pickupSpawnIntervalMaximum );
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
	if ( mouseState.rightButton && !m_rightMouseWasDown && m_secondaryProjectileAmmo > 0 )
	{
		const Vector3 direction = m_camera->GetForward();
		m_projectiles.push_back( std::make_unique< Projectile_Secondary >( m_camera->GetPosition() + direction, direction ) );
		--m_secondaryProjectileAmmo;
	}
	m_leftMouseWasDown = mouseState.leftButton;
	m_rightMouseWasDown = mouseState.rightButton;

	for ( const auto& projectile : m_projectiles )
	{
		m_score += projectile->OnUpdate( deltaTime, *m_flockManager, m_pickup.get() );
		const PickupEffect pickupEffect = projectile->TakePickupEffect();
		if ( pickupEffect == PickupEffect::Speed )
		{
			m_camera->SetMovementSpeedMultiplier( 1.2f );
			SetPrimaryProjectileSpeedMultiplier( 1.5f );
			m_speedBoostRemaining = 10.0f;
		}
		else if ( pickupEffect == PickupEffect::Missiles )
		{
			m_secondaryProjectileAmmo += 3;
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
	renderContext->RenderTextRightAligned( "Missiles: " + std::to_string( m_secondaryProjectileAmmo ), Vector2( windowSize.x - 10.0f, 40.0f ), 1.0f, Colors::White );
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
		m_pickupSpawnDelay = RandomRange( m_pickupSpawnIntervalMinimum, m_pickupSpawnIntervalMaximum );
		return;
	}

	m_pickupSpawnTimer += timeStep;
	if ( m_pickupSpawnTimer < m_pickupSpawnDelay ) return;

	for ( int attempt = 0; attempt < 32; ++attempt )
	{
		const Vector3 position( RandomRange( -18.0f, 18.0f ), RandomRange( 4.0f, 22.0f ), RandomRange( -18.0f, 18.0f ) );
		if ( m_flockManager->IntersectsBuilding( position, PICKUP_COLLISION_RADIUS ) ) continue;

		const float pickupChoice = RandomRange( 0.0f, 1.0f );
		if ( pickupChoice < 1.0f / 3.0f ) m_pickup = std::make_unique< Pickup_Freeze >( position, m_pickupLifetime );
		else if ( pickupChoice < 2.0f / 3.0f ) m_pickup = std::make_unique< Pickup_Speed >( position, m_pickupLifetime );
		else m_pickup = std::make_unique< Pickup_Missiles >( position, m_pickupLifetime );
		return;
	}
}

void Game::LoadSettings()
{
	// Reset defaults first so an edited settings file is re-read cleanly on every restart.
	m_pickupSpawnIntervalMinimum = 10.0f;
	m_pickupSpawnIntervalMaximum = 20.0f;
	m_pickupLifetime = 6.0f;
	m_flockManager->flockNumber = 4;
	m_flockManager->minimumFlockSize = 5;
	m_flockManager->maximumFlockSize = 12;
	m_flockManager->boidSize = 0.70f;
	m_flockManager->boidVelocity = 4.875f;
	m_flockManager->cohesion = 0.15f;
	m_flockManager->alignment = 0.55f;
	m_flockManager->closeRangeSeparation = 3.5f;

	std::ifstream settingsFile( "../../data/Setting.json" );
	if ( !settingsFile ) return;

	std::stringstream settingsContents;
	settingsContents << settingsFile.rdbuf();
	Json::Document settings;
	settings.Parse< Json::kParseCommentsFlag >( settingsContents.str().c_str() );
	if ( settings.HasParseError() || !settings.IsObject() ) return;

	auto readFloat = []( const Json::Value& object, const char* name, float& value )
	{
		if ( object.HasMember( name ) && object[ name ].IsNumber() ) value = object[ name ].GetFloat();
	};
	auto readInteger = []( const Json::Value& object, const char* name, int& value )
	{
		if ( object.HasMember( name ) && object[ name ].IsInt() ) value = object[ name ].GetInt();
	};

	if ( settings.HasMember( "pickup" ) && settings[ "pickup" ].IsObject() )
	{
		const Json::Value& pickup = settings[ "pickup" ];
		readFloat( pickup, "spawnIntervalMinSeconds", m_pickupSpawnIntervalMinimum );
		readFloat( pickup, "spawnIntervalMaxSeconds", m_pickupSpawnIntervalMaximum );
		readFloat( pickup, "lifetimeSeconds", m_pickupLifetime );
	}
	if ( settings.HasMember( "flock" ) && settings[ "flock" ].IsObject() )
	{
		const Json::Value& flock = settings[ "flock" ];
		int flockNumber = static_cast< int >( m_flockManager->flockNumber );
		readInteger( flock, "count", flockNumber );
		m_flockManager->flockNumber = static_cast< size_t >( std::max( 1, flockNumber ) );
		readInteger( flock, "minimumSize", m_flockManager->minimumFlockSize );
		readInteger( flock, "maximumSize", m_flockManager->maximumFlockSize );
		readFloat( flock, "boidSize", m_flockManager->boidSize );
		readFloat( flock, "boidVelocity", m_flockManager->boidVelocity );
		readFloat( flock, "cohesion", m_flockManager->cohesion );
		readFloat( flock, "alignment", m_flockManager->alignment );
		readFloat( flock, "separation", m_flockManager->closeRangeSeparation );
	}

	m_pickupSpawnIntervalMinimum = std::max( 0.1f, m_pickupSpawnIntervalMinimum );
	m_pickupSpawnIntervalMaximum = std::max( m_pickupSpawnIntervalMinimum, m_pickupSpawnIntervalMaximum );
	m_pickupLifetime = std::max( 0.1f, m_pickupLifetime );
	m_flockManager->minimumFlockSize = std::max( 1, m_flockManager->minimumFlockSize );
	m_flockManager->maximumFlockSize = std::max( m_flockManager->minimumFlockSize, m_flockManager->maximumFlockSize );
	m_flockManager->boidSize = std::max( 0.05f, m_flockManager->boidSize );
	m_flockManager->boidVelocity = std::max( 0.1f, m_flockManager->boidVelocity );
	m_flockManager->cohesion = std::max( 0.0f, m_flockManager->cohesion );
	m_flockManager->alignment = std::max( 0.0f, m_flockManager->alignment );
	m_flockManager->closeRangeSeparation = std::max( 0.0f, m_flockManager->closeRangeSeparation );
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
	m_rightMouseWasDown = false;
}
