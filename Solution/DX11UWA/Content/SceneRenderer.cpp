#include "pch.h"
#include "SceneRenderer.h"
#include "..\\Common\\DirectXHelper.h"
#include "..\\Common\\DDSTextureLoader.h"
#include "Assets\\Talon.h"
#include <fstream>
using namespace DX11UWA;
bool SceneRenderer::renderFileMesh = true;

// Loads vertex and pixel shaders from files and instantiates the cube geometry.
SceneRenderer::SceneRenderer( const std::shared_ptr<DX::DeviceResources>& deviceResources ) :
	m_loadingComplete( false ),
	m_degreesPerSecond( 45 ),
	m_indexCount( 0 ),
	m_tracking( false ),
	m_deviceResources( deviceResources )
{
	memset( m_kbuttons, 0, sizeof( m_kbuttons ) );
	m_currMousePos = nullptr;
	m_prevMousePos = nullptr;
	memset( &m_camera, 0, sizeof( DirectX::XMFLOAT4X4 ) );

	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}

// Initializes view parameters when the window size changes.
void SceneRenderer::CreateWindowSizeDependentResources( void )
{
	Windows::Foundation::Size outputSize = m_deviceResources->GetOutputSize();
	float aspectRatio = outputSize.Width / outputSize.Height;
	float fovAngleY = 70.0f * DirectX::XM_PI / 180.0f;

	// This is a simple example of change that can be made when the app is in
	// portrait or snapped view.
	if ( aspectRatio < 1.0f )
	{
		fovAngleY *= 2.0f;
	}

	// Note that the OrientationTransform3D matrix is post-multiplied here
	// in order to correctly orient the scene to match the display orientation.
	// This post-multiplication step is required for any draw calls that are
	// made to the swap chain render target. For draw calls to other targets,
	// this transform should not be applied.

	// This sample makes use of a right-handed coordinate system using row-major matrices.
	DirectX::XMMATRIX perspectiveMatrix = DirectX::XMMatrixPerspectiveFovLH( fovAngleY, aspectRatio, 0.01f, 100.0f );

	DirectX::XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

	DirectX::XMMATRIX orientationMatrix = XMLoadFloat4x4( &orientation );

	XMStoreFloat4x4( &m_constantBufferData.projection, XMMatrixTranspose( perspectiveMatrix * orientationMatrix ) );

	// Eye is at (0,0.7,1.5), looking at point (0,-0.1,0) with the up-vector along the y-axis.
	static const DirectX::XMVECTORF32 eye = { 0.0f, 0.7f, -1.5f, 0.0f };
	static const DirectX::XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
	static const DirectX::XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

	XMStoreFloat4x4( &m_camera, XMMatrixInverse( nullptr, XMMatrixLookAtLH( eye, at, up ) ) );
	XMStoreFloat4x4( &m_constantBufferData.view, XMMatrixTranspose( XMMatrixLookAtLH( eye, at, up ) ) );
}

// Called once per frame, rotates the cube and calculates the model and view matrices.
void SceneRenderer::Update( DX::StepTimer const& timer )
{
	if ( !m_tracking )
	{
		Rotate( ( float )fmod(
			timer.GetTotalSeconds() *
			DirectX::XMConvertToRadians( m_degreesPerSecond ),
			DirectX::XM_2PI ) );
	}


	// Update or move camera here
	UpdateCamera( timer, 1.5f, 0.75f );
	static bool rButtonDown = false;
	if ( m_kbuttons[ 'R' ] )
	{
		if ( !rButtonDown )
		{
			rButtonDown = true;
			ReleaseDeviceDependentResources();
			renderFileMesh = !renderFileMesh;
			CreateDeviceDependentResources();
		}
	}
	else
		rButtonDown = false;
}

// Rotate the 3D cube model a set amount of radians.
void SceneRenderer::Rotate( float radians )
{
	// Prepare to pass the updated model matrix to the shader
	XMStoreFloat4x4( &m_constantBufferData.model, XMMatrixTranspose( DirectX::XMMatrixRotationY( radians ) ) );
}

void SceneRenderer::UpdateCamera( DX::StepTimer const& timer, float const moveSpd, float const rotSpd )
{
	const float delta_time = ( float )timer.GetElapsedSeconds();

	if ( m_kbuttons[ 'W' ] )
	{
		DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation( 0.0f, 0.0f, moveSpd * delta_time );
		DirectX::XMMATRIX temp_camera = XMLoadFloat4x4( &m_camera );
		DirectX::XMMATRIX result = XMMatrixMultiply( translation, temp_camera );
		XMStoreFloat4x4( &m_camera, result );
	}
	if ( m_kbuttons[ 'S' ] )
	{
		DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation( 0.0f, 0.0f, -moveSpd * delta_time );
		DirectX::XMMATRIX temp_camera = XMLoadFloat4x4( &m_camera );
		DirectX::XMMATRIX result = XMMatrixMultiply( translation, temp_camera );
		XMStoreFloat4x4( &m_camera, result );
	}
	if ( m_kbuttons[ 'A' ] )
	{
		DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation( -moveSpd * delta_time, 0.0f, 0.0f );
		DirectX::XMMATRIX temp_camera = XMLoadFloat4x4( &m_camera );
		DirectX::XMMATRIX result = XMMatrixMultiply( translation, temp_camera );
		XMStoreFloat4x4( &m_camera, result );
	}
	if ( m_kbuttons[ 'D' ] )
	{
		DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation( moveSpd * delta_time, 0.0f, 0.0f );
		DirectX::XMMATRIX temp_camera = XMLoadFloat4x4( &m_camera );
		DirectX::XMMATRIX result = XMMatrixMultiply( translation, temp_camera );
		XMStoreFloat4x4( &m_camera, result );
	}
	if ( m_kbuttons[ VK_SHIFT ] )
	{
		DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation( 0.0f, -moveSpd * delta_time, 0.0f );
		DirectX::XMMATRIX temp_camera = XMLoadFloat4x4( &m_camera );
		DirectX::XMMATRIX result = XMMatrixMultiply( translation, temp_camera );
		XMStoreFloat4x4( &m_camera, result );
	}
	if ( m_kbuttons[ VK_SPACE ] )
	{
		DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation( 0.0f, moveSpd * delta_time, 0.0f );
		DirectX::XMMATRIX temp_camera = XMLoadFloat4x4( &m_camera );
		DirectX::XMMATRIX result = XMMatrixMultiply( translation, temp_camera );
		XMStoreFloat4x4( &m_camera, result );
	}

	if ( m_currMousePos )
	{
		if ( m_currMousePos->Properties->IsLeftButtonPressed && m_prevMousePos )
		{
			float dx = m_currMousePos->Position.X - m_prevMousePos->Position.X;
			float dy = m_currMousePos->Position.Y - m_prevMousePos->Position.Y;

			DirectX::XMFLOAT4 pos( m_camera._41, m_camera._42, m_camera._43, m_camera._44 );

			m_camera._41 = 0.0f;
			m_camera._42 = 0.0f;
			m_camera._43 = 0.0f;

			DirectX::XMMATRIX rotX = DirectX::XMMatrixRotationX( dy * rotSpd * delta_time );
			DirectX::XMMATRIX rotY = DirectX::XMMatrixRotationY( dx * rotSpd * delta_time );

			DirectX::XMMATRIX temp_camera = XMLoadFloat4x4( &m_camera );
			temp_camera = XMMatrixMultiply( rotX, temp_camera );
			temp_camera = XMMatrixMultiply( temp_camera, rotY );

			XMStoreFloat4x4( &m_camera, temp_camera );

			m_camera._41 = pos.x;
			m_camera._42 = pos.y;
			m_camera._43 = pos.z;
		}
		m_prevMousePos = m_currMousePos;
	}


}

void SceneRenderer::SetKeyboardButtons( const char* list )
{
	memcpy_s( m_kbuttons, sizeof( m_kbuttons ), list, sizeof( m_kbuttons ) );
}

void SceneRenderer::SetMousePosition( const Windows::UI::Input::PointerPoint^ pos )
{
	m_currMousePos = const_cast< Windows::UI::Input::PointerPoint^ >( pos );
}

void SceneRenderer::SetInputDeviceData( const char* kb, const Windows::UI::Input::PointerPoint^ pos )
{
	SetKeyboardButtons( kb );
	SetMousePosition( pos );
}

void DX11UWA::SceneRenderer::StartTracking( void )
{
	m_tracking = true;
}

// When tracking, the 3D cube can be rotated around its Y axis by tracking pointer position relative to the output screen width.
void SceneRenderer::TrackingUpdate( float positionX )
{
	if ( m_tracking )
	{
		float radians = DirectX::XM_2PI * 2.0f * positionX / m_deviceResources->GetOutputSize().Width;
		Rotate( radians );
	}
}

void SceneRenderer::StopTracking( void )
{
	m_tracking = false;
}

// Renders one frame using the vertex and pixel shaders.
void SceneRenderer::Render( void )
{
	// Loading is asynchronous. Only draw geometry after it's loaded.
	if ( !m_loadingComplete ) return;

	auto context = m_deviceResources->GetD3DDeviceContext();

	XMStoreFloat4x4( &m_constantBufferData.view, XMMatrixTranspose( XMMatrixInverse( nullptr, XMLoadFloat4x4( &m_camera ) ) ) );


	// Prepare the constant buffer to send it to the graphics device.
	context->UpdateSubresource1( m_constantBuffer.Get(), 0u, nullptr, &m_constantBufferData, 0u, 0u, 0u );
	// Each vertex is one instance of the Vertex struct.
	UINT stride = sizeof( Vertex );
	UINT offset = 0u;
	context->IASetVertexBuffers( 0u, 1u, m_vertexBuffer.GetAddressOf(), &stride, &offset );
	// Each index is one 32-bit unsigned integer.
	context->IASetIndexBuffer( m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0u );
	context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	context->IASetInputLayout( m_inputLayout.Get() );
	// Attach our vertex shader.
	context->VSSetShader( m_vertexShader.Get(), nullptr, 0u );
	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1( 0u, 1u, m_constantBuffer.GetAddressOf(), nullptr, nullptr );
	// Attach our pixel shader.
	context->PSSetShader( m_pixelShader.Get(), nullptr, 0u );
	// Draw the objects.
	context->DrawIndexed( m_indexCount, 0u, 0 );
}

bool operator==( const Vertex& lhs, const Vertex& rhs )
{
	return
		lhs.pos.x == rhs.pos.x &&
		lhs.pos.y == rhs.pos.y &&
		lhs.pos.z == rhs.pos.z &&
		lhs.uv.x == rhs.uv.x &&
		lhs.uv.y == rhs.uv.y &&
		lhs.normal.x == rhs.normal.x &&
		lhs.normal.y == rhs.normal.y &&
		lhs.normal.z == rhs.normal.z;
}
void SceneRenderer::ObjMesh_ToBuffer( Vertex*& outVertices, unsigned int*& outIndices,
									  unsigned int& outNumVertices, unsigned int& outNumIndices,
									  const std::vector<DirectX::XMFLOAT3>& positions,
									  const std::vector<DirectX::XMFLOAT2>& uvs,
									  const std::vector<DirectX::XMFLOAT3>& normals,
									  const std::vector<SceneRenderer::IndexTriangle>& triangles )
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	Vertex tempVert =
	{
		DirectX::XMFLOAT4( 0.0f, 0.0f, 0.0f, 1.0f ),
		DirectX::XMFLOAT4( 0.0f, 0.0f, 1.0f, 1.0f ),
		DirectX::XMFLOAT4( 0.0f, 0.0f, 1.0f, 1.0f )
	};
	unsigned int i;
	for ( const IndexTriangle& triangle : triangles ) for ( int v = 0; v < 3; ++v )
	{
		tempVert.pos.x = positions[ triangle.p[ v ] - 1u ].x;
		tempVert.pos.y = positions[ triangle.p[ v ] - 1u ].y;
		tempVert.pos.z = positions[ triangle.p[ v ] - 1u ].z;
		tempVert.uv.x = uvs[ triangle.t[ v ] - 1u ].x;
		tempVert.uv.y = uvs[ triangle.t[ v ] - 1u ].y;
		tempVert.normal.x = normals[ triangle.n[ v ] - 1u ].x;
		tempVert.normal.y = normals[ triangle.n[ v ] - 1u ].y;
		tempVert.normal.z = normals[ triangle.n[ v ] - 1u ].z;
		for ( i = 0u; i < vertices.size(); ++i )
			if ( vertices[ i ] == tempVert )
				break;
		if ( i == vertices.size() )
			vertices.push_back( tempVert );
		indices.push_back( i );
	}
	outNumVertices = ( unsigned int )vertices.size();
	outNumIndices = ( unsigned int )indices.size();
	outVertices = new Vertex[ outNumVertices ];
	outIndices = new unsigned int[ outNumIndices ];
	for ( i = 0u; i < outNumVertices; ++i )
		outVertices[ i ] = vertices[ i ];
	for ( i = 0u; i < outNumIndices; ++i )
		outIndices[ i ] = indices[ i ];
}
void SceneRenderer::ObjMesh_LoadMesh(
	const char* const filepath,
	Vertex*& outVertices,
	unsigned int*& outIndices,
	unsigned int& outNumVertices,
	unsigned int& outNumIndices )
{
	std::ifstream file;
	if ( renderFileMesh )
		file.open( filepath, std::ios_base::in );
	if ( file.is_open() )
	{
		DirectX::XMFLOAT2 tempf2( 0.0f, 0.0f );
		DirectX::XMFLOAT3 tempf3( 0.0f, 0.0f, 0.0f );
		IndexTriangle tempTriangle;
		ZEROSTRUCT( tempTriangle );
		std::vector<DirectX::XMFLOAT3> positions;
		std::vector<DirectX::XMFLOAT2> uvs;
		std::vector<DirectX::XMFLOAT3> normals;
		std::vector<IndexTriangle> triangles;
		char temp;
		for ( ;; )
		{
			file.get( temp );
			if ( file.eof() )
				break;
			if ( 'v' == temp )
			{
				file.get( temp );
				if ( ' ' == temp )
				{
					file >> tempf3.x >> tempf3.y >> tempf3.z;
					positions.push_back( tempf3 );
				}
				else if ( 't' == temp )
				{
					file >> tempf2.x >> tempf2.y;
					tempf2.y = 1.0f - tempf2.y;
					uvs.push_back( tempf2 );
				}
				else if ( 'n' == temp )
				{
					file >> tempf3.x >> tempf3.y >> tempf3.z;
					normals.push_back( tempf3 );
				}
			}
			else if ( 'f' == temp )
			{
				file.get( temp );
				if ( ' ' == temp )
				{
					file >> tempTriangle.p[ 0 ] >> temp >> tempTriangle.t[ 0 ] >> temp >> tempTriangle.n[ 0 ];
					file >> tempTriangle.p[ 1 ] >> temp >> tempTriangle.t[ 1 ] >> temp >> tempTriangle.n[ 1 ];
					file >> tempTriangle.p[ 2 ] >> temp >> tempTriangle.t[ 2 ] >> temp >> tempTriangle.n[ 2 ];
					triangles.push_back( tempTriangle );
				}
			}
			while ( '\n' != temp )
				file.get( temp );
		}
		file.close();
		ObjMesh_ToBuffer( outVertices, outIndices,
						  outNumVertices, outNumIndices,
						  positions, uvs, normals, triangles );
	}
	else
	{
		static const Vertex tempVertices[ ] =
		{
			{ DirectX::XMFLOAT4( -0.5f, -0.5f, -0.5f, 1.0f ), DirectX::XMFLOAT4( 0.0f, 0.0f, 0.0f, 1.0f ), DirectX::XMFLOAT4( 0.0f, 0.0f, 1.0f, 1.0f ) },
			{ DirectX::XMFLOAT4( -0.5f, -0.5f, 0.5f, 1.0f ), DirectX::XMFLOAT4( 0.0f, 0.0f, 1.0f, 1.0f ), DirectX::XMFLOAT4( 0.0f, 0.0f, 1.0f, 1.0f ) },
			{ DirectX::XMFLOAT4( -0.5f, 0.5f, -0.5f, 1.0f ), DirectX::XMFLOAT4( 0.0f, 1.0f, 0.0f, 1.0f ), DirectX::XMFLOAT4( 0.0f, 0.0f, 1.0f, 1.0f ) },
			{ DirectX::XMFLOAT4( -0.5f, 0.5f, 0.5f, 1.0f ), DirectX::XMFLOAT4( 0.0f, 1.0f, 1.0f, 1.0f ), DirectX::XMFLOAT4( 0.0f, 0.0f, 1.0f, 1.0f ) },
			{ DirectX::XMFLOAT4( 0.5f, -0.5f, -0.5f, 1.0f ), DirectX::XMFLOAT4( 1.0f, 0.0f, 0.0f, 1.0f ), DirectX::XMFLOAT4( 0.0f, 0.0f, 1.0f, 1.0f ) },
			{ DirectX::XMFLOAT4( 0.5f, -0.5f, 0.5f, 1.0f ), DirectX::XMFLOAT4( 1.0f, 0.0f, 1.0f, 1.0f ), DirectX::XMFLOAT4( 0.0f, 0.0f, 1.0f, 1.0f ) },
			{ DirectX::XMFLOAT4( 0.5f, 0.5f, -0.5f, 1.0f ), DirectX::XMFLOAT4( 1.0f, 1.0f, 0.0f, 1.0f ), DirectX::XMFLOAT4( 0.0f, 0.0f, 1.0f, 1.0f ) },
			{ DirectX::XMFLOAT4( 0.5f, 0.5f, 0.5f, 1.0f ), DirectX::XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f ) , DirectX::XMFLOAT4( 0.0f, 0.0f, 1.0f, 1.0f ) }
		};
		static const unsigned int tempIndices[ ] =
		{
			0u, 1u, 2u,
			1u, 3u, 2u,

			4u, 6u, 5u,
			5u, 6u, 7u,

			0u, 5u, 1u,
			0u, 4u, 5u,

			2u, 7u, 6u,
			2u, 3u, 7u,

			0u, 6u, 4u,
			0u, 2u, 6u,

			1u, 7u, 3u,
			1u, 5u, 7u
		};
		outNumVertices = 8u;
		outNumIndices = 36u;
		outVertices = new Vertex[ outNumVertices ];
		outIndices = new unsigned int[ outNumIndices ];
		memcpy_s( outVertices, sizeof( Vertex ) * outNumVertices, tempVertices, sizeof( tempVertices ) );
		memcpy_s( outIndices, sizeof( unsigned int ) * outNumIndices, tempIndices, sizeof( tempIndices ) );
	}
}
void SceneRenderer::ObjMesh_Unload( Vertex*& vertices, unsigned int*& indices )
{
	delete[ ] vertices;
	delete[ ] indices;
	vertices = nullptr;
	indices = nullptr;
}

unsigned int getColor( const unsigned int& inCol )
{
	return
		( ( ( inCol & 0xff000000u ) >> 24 ) | ( ( inCol & 0xff0000u ) >> 8 ) |
		( ( inCol & 0xff00u ) << 8 ) | ( ( inCol & 0xffu ) << 24 ) );
}

void SceneRenderer::CreateDeviceDependentResources( void )
{
	// Load shaders asynchronously.
	auto loadVSTask = DX::ReadDataAsync( L"VertexShader.cso" );
	auto loadPSTask = DX::ReadDataAsync( L"PixelShader.cso" );
	auto createVSTask = loadVSTask.then( [ this ]( const std::vector<byte>& fileData )
	{
		DX::ThrowIfFailed( m_deviceResources->GetD3DDevice()->CreateVertexShader( &fileData[ 0 ], fileData.size(), nullptr, &m_vertexShader ) );

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[ ] =
		{
			{ "POSITION", 0u, DXGI_FORMAT_R32G32B32A32_FLOAT, 0u, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0u },
			{ "TEXCOORD", 0u, DXGI_FORMAT_R32G32B32A32_FLOAT, 0u, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0u },
			{ "NORMAL", 0u, DXGI_FORMAT_R32G32B32A32_FLOAT, 0u, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0u },
		};

		DX::ThrowIfFailed( m_deviceResources->GetD3DDevice()->CreateInputLayout( vertexDesc, ARRAYSIZE( vertexDesc ), &fileData[ 0 ], fileData.size(), &m_inputLayout ) );
	} );
	auto createPSTask = loadPSTask.then( [ this ]( const std::vector<byte>& fileData )
	{
		DX::ThrowIfFailed( m_deviceResources->GetD3DDevice()->CreatePixelShader( &fileData[ 0 ], fileData.size(), nullptr, &m_pixelShader ) );

		CD3D11_BUFFER_DESC constantBufferDesc( sizeof( ModelViewProjectionConstantBuffer ), D3D11_BIND_CONSTANT_BUFFER );
		DX::ThrowIfFailed( m_deviceResources->GetD3DDevice()->CreateBuffer( &constantBufferDesc, nullptr, &m_constantBuffer ) );
	} );
	auto createMeshTask = createVSTask.then( [ this ]()
	{
		Vertex* vertices = nullptr;
		unsigned int* indices = nullptr;
		unsigned int numVertices = 0u, numIndices = 0u;

		ObjMesh_LoadMesh( "Assets\\Talon.mobj", vertices, indices, numVertices, numIndices );

		D3D11_SUBRESOURCE_DATA vertexBufferData;
		ZEROSTRUCT( vertexBufferData );
		vertexBufferData.pSysMem = vertices;
		CD3D11_BUFFER_DESC vertexBufferDesc( sizeof( Vertex ) * numVertices, D3D11_BIND_VERTEX_BUFFER );
		DX::ThrowIfFailed( m_deviceResources->GetD3DDevice()->CreateBuffer( &vertexBufferDesc, &vertexBufferData, &m_vertexBuffer ) );

		m_indexCount = numIndices;

		D3D11_SUBRESOURCE_DATA indexBufferData;
		ZEROSTRUCT( indexBufferData );
		indexBufferData.pSysMem = indices;
		CD3D11_BUFFER_DESC indexBufferDesc( sizeof( unsigned int ) * numIndices, D3D11_BIND_INDEX_BUFFER );
		DX::ThrowIfFailed( m_deviceResources->GetD3DDevice()->CreateBuffer( &indexBufferDesc, &indexBufferData, &m_indexBuffer ) );

		ObjMesh_Unload( vertices, indices );
	} );
	auto createTextureTask = createPSTask.then( [ this ]()
	{
		D3D11_TEXTURE2D_DESC textureDesc;
		D3D11_SUBRESOURCE_DATA textureSubresourceData[ Talon_numlevels ];
		ZEROSTRUCT( textureDesc );
		textureDesc.Width = Talon_width;
		textureDesc.Height = Talon_height;
		textureDesc.MipLevels = Talon_numlevels;
		textureDesc.ArraySize = 1u;
		textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
		textureDesc.SampleDesc.Count = 1u;
		textureDesc.SampleDesc.Quality = 0u;
		textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0u;
		textureDesc.MiscFlags = 0u;
		unsigned int* pixels = new unsigned int[ Talon_numpixels ];
		for ( unsigned int i = 0u; i < Talon_numpixels; ++i )
			pixels[ i ] = getColor( Talon_pixels[ i ] );
		for ( unsigned int i = 0u; i < Talon_numlevels; ++i )
		{
			ZEROSTRUCT( textureSubresourceData[ i ] );
			textureSubresourceData[ i ].pSysMem = &pixels[ Talon_leveloffsets[ i ] ];
			textureSubresourceData[ i ].SysMemPitch = ( Talon_width >> i ) * sizeof( unsigned int );
		}
		ID3D11Texture2D* texture;
		m_deviceResources->GetD3DDevice()->CreateTexture2D( &textureDesc, textureSubresourceData, &texture );
		delete[ ] pixels;

		D3D11_SAMPLER_DESC samplerDesc;
		ZEROSTRUCT( samplerDesc );
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MaxAnisotropy = 1u;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		samplerDesc.BorderColor[ 0 ] = 1.0f;
		samplerDesc.BorderColor[ 1 ] = 1.0f;
		samplerDesc.BorderColor[ 2 ] = 1.0f;
		samplerDesc.BorderColor[ 3 ] = 1.0f;
		samplerDesc.MinLOD = -FLT_MAX;
		samplerDesc.MaxLOD = FLT_MAX;
		ID3D11SamplerState* samplerState;
		m_deviceResources->GetD3DDevice()->CreateSamplerState( &samplerDesc, &samplerState );

		m_deviceResources->GetD3DDeviceContext()->PSSetSamplers( 0u, 1u, &samplerState );

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZEROSTRUCT( srvDesc );
		srvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0u;
		srvDesc.Texture2D.MipLevels = Talon_numlevels;
		ID3D11ShaderResourceView* srv;
		m_deviceResources->GetD3DDevice()->CreateShaderResourceView( texture, &srvDesc, &srv );

		m_deviceResources->GetD3DDeviceContext()->PSSetShaderResources( 0u, 1u, &srv );

		srv->Release();
		samplerState->Release();
		texture->Release();
	} );
	( createMeshTask && createTextureTask ).then( [ this ]() { m_loadingComplete = true; } );
}

void SceneRenderer::ReleaseDeviceDependentResources( void )
{
	m_loadingComplete = false;
	m_vertexShader.Reset();
	m_inputLayout.Reset();
	m_pixelShader.Reset();
	m_constantBuffer.Reset();
	m_vertexBuffer.Reset();
	m_indexBuffer.Reset();
}