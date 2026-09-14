/**
* Copyright (c) 2022 Yigsoft. All Rights Reserved.
*/

#pragma once
#include "IRenderContext.h"

namespace cdp_framework
{
	class IGame
	{
	public:
		virtual ~IGame() = default;
		virtual void OnInitialize() = 0;
		virtual void OnUpdate( float deltaTime, DirectX::Keyboard& keyboard, DirectX::Mouse& mouse, DirectX::GamePad& gamepad ) = 0;
		virtual void OnRender( RenderContextPtr& renderContext ) = 0;
		virtual void OnShutdown() = 0;
	};
}