/**
* Copyright (c) 2022 Yigsoft. All Rights Reserved.
*/

#include "pch.h"
#include "City.h"
#include "Engine.h"
#include "StepTimer.h"
#include "DeviceResources.h"

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

	// Physical flight limits are +/-21 in X/Z and 0-25 in Y. Place a thin,
	// solid visual shell just outside those limits.
	const Vector3 boundarySizes[] =
	{
		Vector3( 44.0f, 0.2f, 44.0f ),
		Vector3( 44.0f, 0.2f, 44.0f ),
		Vector3( 0.2f, 26.0f, 44.0f ),
		Vector3( 0.2f, 26.0f, 44.0f ),
		Vector3( 44.0f, 26.0f, 0.2f ),
		Vector3( 44.0f, 26.0f, 0.2f )
	};
	const Vector3 boundaryPositions[] =
	{
		Vector3( 0.0f, -0.1f, 0.0f ), Vector3( 0.0f, 26.1f, 0.0f ),
		Vector3( -22.1f, 13.0f, 0.0f ), Vector3( 22.1f, 13.0f, 0.0f ),
		Vector3( 0.0f, 13.0f, -22.1f ), Vector3( 0.0f, 13.0f, 22.1f )
	};
	for ( size_t i = 0; i < sizeof( boundarySizes ) / sizeof( boundarySizes[ 0 ] ); ++i )
	{
		BoundarySurface surface;
		surface.shape = GetEngine().CreateBoxPrimitive( boundarySizes[ i ] );
		surface.position = boundaryPositions[ i ];
		m_boundarySurfaces.push_back( std::move( surface ) );
	}

}

void City::OnUpdate( float deltaTime )
{
	UNREFERENCED_PARAMETER( deltaTime );
}

void City::OnRender( cdp_framework::RenderContextPtr& renderContext )
{
	for ( const BoundarySurface& surface : m_boundarySurfaces )
	{
		renderContext->RenderPrimitive( surface.shape, Vector3::One, surface.position, Vector3::Zero, Colors::DarkSlateGray );
	}

	for ( Skyscraper& skyscraper : m_skyscrapers )
	{
		renderContext->RenderPrimitive( skyscraper.shape, Vector3::One, skyscraper.position, Vector3::Zero, Colors::BlueViolet );
	}

}

void City::OnShutdown()
{
	for ( BoundarySurface& surface : m_boundarySurfaces )
	{
		surface.shape.reset();
	}
	m_boundarySurfaces.clear();

	for ( Skyscraper& skyscraper : m_skyscrapers )
	{
		skyscraper.shape.reset();
	}

	m_skyscrapers.clear();
}

const std::vector< Skyscraper >& City::GetSkyscrapers() const
{
	return m_skyscrapers;
}
