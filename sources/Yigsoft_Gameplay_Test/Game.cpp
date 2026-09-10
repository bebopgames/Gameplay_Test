/**
* Copyright (c) 2022 Yigsoft. All Rights Reserved.
*/

#include "pch.h"
#include "Game.h"
#include "Engine.h"

Game::Game()
{
    m_camera = std::make_unique< Camera >();
    m_city = std::make_unique< City >();
    m_crosshair = std::make_unique< Crosshair >();

    // Your code here


}

Game::~Game() = default;

void Game::OnInitialize()
{
    m_city->OnInitialize();
    m_crosshair->OnInitialize();

	// Your code here


}

void Game::OnUpdate( float deltaTime, DirectX::Keyboard& keyboard, DirectX::Mouse& mouse, DirectX::GamePad& gamepad )
{
	m_camera->OnUpdate( deltaTime, keyboard, mouse, gamepad );
	m_city->OnUpdate( deltaTime );
    m_crosshair->OnUpdate( deltaTime, mouse, gamepad );

	// Your code here


}

void Game::OnRender( cdp_framework::RenderContextPtr& renderContext )
{
	m_city->OnRender( renderContext );
    m_crosshair->OnRender( renderContext );
    Vector2 windowSize = GetEngine().GetWindowSize();
	renderContext->RenderText( "CD Projekt RED Gameplay Test", Vector2( 10, ( windowSize.y - 40 ) ), 1, Colors::DarkRed );

	// Your code here


}

void Game::OnShutdown()
{
	m_city->OnShutdown();
    m_crosshair->OnShutdown();

	// Your code here


}
