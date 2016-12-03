#include "pch.h"
#include "SceneRenderer.h"
#include "..\\Common\\DirectXHelper.h"
#include "..\\Common\\DDSTextureLoader.h"
#include "Assets\\star.h"
#include <fstream>
using namespace DX11UWA;
bool renderCube = true;

bool SceneRenderer::KeyHit( char key )
{
	static bool keys[ 256 ] = { false };
	if ( m_kbuttons[ key ] )
	{
		if ( !keys[ key ] )
		{
			keys[ key ] = true;
			return true;
		}
	}
	else keys[ key ] = false;
	return false;
}

void SceneRenderer::ToggleWireframe( void )
{
	ID3D11RasterizerState* rsState;
	D3D11_RASTERIZER_DESC rsDesc;
	m_deviceResources->GetD3DDeviceContext()->RSGetState( &rsState );
	rsState->GetDesc( &rsDesc );
	rsState->Release();
	rsDesc.FillMode = ( ( rsDesc.FillMode ) == ( D3D11_FILL_SOLID ) ? ( D3D11_FILL_WIREFRAME ) : ( D3D11_FILL_SOLID ) );
	m_deviceResources->GetD3DDevice()->CreateRasterizerState( &rsDesc, &rsState );
	m_deviceResources->GetD3DDeviceContext()->RSSetState( rsState );
	rsState->Release();
}

// Loads vertex and pixel shaders from files and instantiates the cube geometry.
SceneRenderer::SceneRenderer( const std::shared_ptr<DX::DeviceResources>& deviceResources ) :
	m_loadingComplete( false ),
	drawPlane( true ),
	m_degreesPerSecond( 45 ),
	m_indexCount( 0 ),
	m_deviceResources( deviceResources )
{
	memset( m_kbuttons, 0, sizeof( m_kbuttons ) );
	m_currMousePos = nullptr;
	m_prevMousePos = nullptr;
	memset( &m_camera, 0, sizeof( DirectX::XMFLOAT4X4 ) );

	m_lightingCBufferData.dLightDirection = DirectX::XMFLOAT4( -1.0f, -1.0f, 1.0f, 1.0f );
	m_lightingCBufferData.lightState = DirectX::XMFLOAT4( 0.0f, 1.0f, 0.0f, 0.0f );
	m_lightingCBufferData.pLightPos0 = DirectX::XMFLOAT4( 0.0f, -0.5f, 2.0f, 1.0f );
	m_lightingCBufferData.pLightPos1 = DirectX::XMFLOAT4( 1.7320508f, -0.5f, -1.0f, 1.0f );
	m_lightingCBufferData.pLightPos2 = DirectX::XMFLOAT4( -1.7320508f, -0.5f, -1.0f, 1.0f );
	m_lightingCBufferData.pLightColorRadius0 = DirectX::XMFLOAT4( 1.0f, 0.0f, 0.0f, 10.0f );
	m_lightingCBufferData.pLightColorRadius1 = DirectX::XMFLOAT4( 0.0f, 1.0f, 0.0f, 10.0f );
	m_lightingCBufferData.pLightColorRadius2 = DirectX::XMFLOAT4( 0.0f, 0.0f, 1.0f, 10.0f );

	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();

	ID3D11RasterizerState* rsState;
	D3D11_RASTERIZER_DESC rsDesc;
	ZEROSTRUCT( rsDesc );
	rsDesc.FillMode = D3D11_FILL_SOLID;
	rsDesc.CullMode = D3D11_CULL_NONE;
	rsDesc.FrontCounterClockwise = FALSE;
	rsDesc.DepthBias = 0;
	rsDesc.SlopeScaledDepthBias = 0.0f;
	rsDesc.DepthBiasClamp = 0.0f;
	rsDesc.DepthClipEnable = TRUE;
	rsDesc.ScissorEnable = FALSE;
	rsDesc.MultisampleEnable = FALSE;
	rsDesc.AntialiasedLineEnable = FALSE;
	m_deviceResources->GetD3DDevice()->CreateRasterizerState( &rsDesc, &rsState );
	m_deviceResources->GetD3DDeviceContext()->RSSetState( rsState );
	rsState->Release();
}

// Initializes view parameters when the window size changes.
void SceneRenderer::CreateWindowSizeDependentResources( void )
{
	Windows::Foundation::Size outputSize = m_deviceResources->GetOutputSize();
	static float aspectRatio = outputSize.Width / outputSize.Height;
	static const float fovAngleY = DirectX::XMConvertToRadians( 70.0f );

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

void SceneRenderer::UpdateLights( DX::StepTimer const& timer )
{
	static bool lightAnim = true;
	if ( KeyHit( 'T' ) ) lightAnim = !lightAnim;
	if ( KeyHit( '1' ) ) m_lightingCBufferData.lightState.x = m_lightingCBufferData.lightState.x > 0.5f ? 0.0f : 1.0f;
	if ( KeyHit( '2' ) ) m_lightingCBufferData.lightState.y = m_lightingCBufferData.lightState.y > 0.5f ? 0.0f : 1.0f;
	if ( KeyHit( '3' ) ) m_lightingCBufferData.lightState.w = 0.0f;
	if ( KeyHit( '4' ) ) m_lightingCBufferData.lightState.w = 1.0f;
	if ( KeyHit( '5' ) ) m_lightingCBufferData.lightState.w = 2.0f;
	if ( KeyHit( '6' ) ) m_lightingCBufferData.lightState.w = 3.0f;
	if ( KeyHit( '7' ) ) m_lightingCBufferData.lightState.w = 4.0f;

	if ( lightAnim )
	{
		static double animTime = 0.0;
		animTime += timer.GetElapsedSeconds();
		DirectX::XMFLOAT4 dir( -1.0f, -1.0f, 1.0f, 1.0f );
		DirectX::XMVECTOR v = DirectX::XMLoadFloat4( &dir );
		DirectX::XMMATRIX m = DirectX::XMMatrixRotationY( ( float )( -2.0 * animTime ) );
		static const float pLightDist = 2.0f;
		static const float sq3div2 = 0.8660254f;
		m_lightingCBufferData.pLightPos0 = DirectX::XMFLOAT4( 0.0f, -0.5f, pLightDist, 1.0f );
		m_lightingCBufferData.pLightPos1 = DirectX::XMFLOAT4( pLightDist * sq3div2, -0.5f, -0.5f * pLightDist, 1.0f );
		m_lightingCBufferData.pLightPos2 = DirectX::XMFLOAT4( -pLightDist * sq3div2, -0.5f, -0.5f * pLightDist, 1.0f );
		DirectX::XMStoreFloat4( &m_lightingCBufferData.pLightPos0, DirectX::XMVector3Transform(
			DirectX::XMLoadFloat4( &m_lightingCBufferData.pLightPos0 ), m ) );
		DirectX::XMStoreFloat4( &m_lightingCBufferData.pLightPos1, DirectX::XMVector3Transform(
			DirectX::XMLoadFloat4( &m_lightingCBufferData.pLightPos1 ), m ) );
		DirectX::XMStoreFloat4( &m_lightingCBufferData.pLightPos2, DirectX::XMVector3Transform(
			DirectX::XMLoadFloat4( &m_lightingCBufferData.pLightPos2 ), m ) );
		v = DirectX::XMVector3Transform( v, m );
		DirectX::XMStoreFloat4( &m_lightingCBufferData.dLightDirection, v );
	}
}

// Called once per frame, rotates the cube and calculates the model and view matrices.
void SceneRenderer::Update( DX::StepTimer const& timer )
{
	AnimateMesh( timer );
	UpdateLights( timer );

	if ( m_loadingComplete ) if ( KeyHit( 'K' ) ) ToggleWireframe();

// Update or move camera here
	UpdateCamera( timer, 1.5f, 0.75f );

	if ( KeyHit( 'R' ) )
	{
		ReleaseDeviceDependentResources();
		renderCube = !renderCube;
		CreateDeviceDependentResources();
	}
	if ( KeyHit( 'P' ) ) drawPlane = !drawPlane;
}

void SceneRenderer::AnimateMesh( DX::StepTimer const& timer )
{
	// Prepare to pass the updated model matrix to the shader
	float radians = ( float )fmod(
		timer.GetTotalSeconds() *
		DirectX::XMConvertToRadians( m_degreesPerSecond ),
		DirectX::XM_2PI );
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
		bool mouseButtonPressed =
			m_currMousePos->Properties->IsLeftButtonPressed ||
			m_currMousePos->Properties->IsMiddleButtonPressed ||
			m_currMousePos->Properties->IsRightButtonPressed;
		if ( m_prevMousePos && mouseButtonPressed )
		{
			float dx = m_currMousePos->Position.X - m_prevMousePos->Position.X;
			float dy = m_currMousePos->Position.Y - m_prevMousePos->Position.Y;

			DirectX::XMFLOAT3 pos( m_camera._41, m_camera._42, m_camera._43 );

			m_camera._41 = 0.0f;
			m_camera._42 = 0.0f;
			m_camera._43 = 0.0f;

			DirectX::XMMATRIX rotX = DirectX::XMMatrixRotationX( dy * rotSpd * delta_time );
			DirectX::XMMATRIX rotY = DirectX::XMMatrixRotationY( dx * rotSpd * delta_time );

			DirectX::XMMATRIX temp_camera = XMLoadFloat4x4( &m_camera );
			temp_camera = XMMatrixMultiply( XMMatrixMultiply( rotX, temp_camera ), rotY );
			XMStoreFloat4x4( &m_camera, temp_camera );

			m_camera._41 = pos.x;
			m_camera._42 = pos.y;
			m_camera._43 = pos.z;
		}
		m_prevMousePos = m_currMousePos;
	}
}

void SceneRenderer::SetInputDeviceData( const char* kb, const Windows::UI::Input::PointerPoint^ pos )
{
	memcpy_s( m_kbuttons, sizeof( m_kbuttons ), kb, sizeof( m_kbuttons ) );
	m_currMousePos = const_cast< Windows::UI::Input::PointerPoint^ >( pos );
}

void SceneRenderer::DrawPlane( void )
{
	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;
	ID3D11Buffer* constantBuffer;
	ID3D11Texture2D* planeTexture;
	ID3D11ShaderResourceView* planeSrv;
	ID3D11DeviceContext3* const context = m_deviceResources->GetD3DDeviceContext();
	ID3D11Device3* const device = m_deviceResources->GetD3DDevice();
	ModelViewProjectionConstantBuffer mvpCbufferData;
	UINT stride = sizeof( Vertex );
	UINT offset = 0u;
	CD3D11_BUFFER_DESC constantBufferDesc( sizeof( ModelViewProjectionConstantBuffer ), D3D11_BIND_CONSTANT_BUFFER );
	D3D11_SUBRESOURCE_DATA vertexBufferData;
	D3D11_SUBRESOURCE_DATA indexBufferData;
	ZEROSTRUCT( vertexBufferData );
	ZEROSTRUCT( indexBufferData );
	CD3D11_BUFFER_DESC vertexBufferDesc( sizeof( Vertex ) * 4u, D3D11_BIND_VERTEX_BUFFER );
	CD3D11_BUFFER_DESC indexBufferDesc( sizeof( unsigned int ) * 6u, D3D11_BIND_INDEX_BUFFER );
	static const Vertex vertices[ 4 ] =
	{
		{ DirectX::XMFLOAT4( -10.0f, -1.0f, -10.0f, 1.0f ), DirectX::XMFLOAT4( -8.0f, -8.0f, 0.0f, 0.0f ), DirectX::XMFLOAT4( 0.0f, 1.0f, 0.0f, 1.0f ) },
		{ DirectX::XMFLOAT4( -10.0f, -1.0f, 10.0f, 1.0f ), DirectX::XMFLOAT4( -8.0f, 8.0f, 0.0f, 0.0f ), DirectX::XMFLOAT4( 0.0f, 1.0f, 0.0f, 1.0f ) },
		{ DirectX::XMFLOAT4( 10.0f, -1.0f, 10.0f, 1.0f ), DirectX::XMFLOAT4( 8.0f, 8.0f, 0.0f, 0.0f ), DirectX::XMFLOAT4( 0.0f, 1.0f, 0.0f, 1.0f ) },
		{ DirectX::XMFLOAT4( 10.0f, -1.0f, -10.0f, 1.0f ), DirectX::XMFLOAT4( 8.0f, -8.0f, 0.0f, 0.0f ), DirectX::XMFLOAT4( 0.0f, 1.0f, 0.0f, 1.0f ) },
	};
	static const unsigned int indices[ 6 ] = { 0u, 1u, 2u, 0u, 2u, 3u };
	vertexBufferData.pSysMem = vertices;
	indexBufferData.pSysMem = indices;
	XMStoreFloat4x4( &mvpCbufferData.model, DirectX::XMMatrixIdentity() );
	mvpCbufferData.projection = m_constantBufferData.projection;
	mvpCbufferData.view = m_constantBufferData.view;

	CreateDDSTextureFromFile( device, L"Assets\\Plane.dds", ( ID3D11Resource** )&planeTexture, &planeSrv );
	device->CreateBuffer( &constantBufferDesc, nullptr, &constantBuffer );
	device->CreateBuffer( &vertexBufferDesc, &vertexBufferData, &vertexBuffer );
	device->CreateBuffer( &indexBufferDesc, &indexBufferData, &indexBuffer );

	context->UpdateSubresource1( constantBuffer, 0u, nullptr, &mvpCbufferData, 0u, 0u, 0u );
	context->VSSetConstantBuffers1( 0u, 1u, &constantBuffer, nullptr, nullptr );
	context->IASetVertexBuffers( 0u, 1u, &vertexBuffer, &stride, &offset );
	context->IASetIndexBuffer( indexBuffer, DXGI_FORMAT_R32_UINT, 0u );
	context->PSSetShaderResources( 0u, 1u, &planeSrv );
	context->DrawIndexed( 6u, 0u, 0 );

	vertexBuffer->Release();
	indexBuffer->Release();
	constantBuffer->Release();
	planeTexture->Release();
	planeSrv->Release();
}

// Renders one frame using the vertex and pixel shaders.
void SceneRenderer::Render( void )
{
	if ( !m_loadingComplete ) return;

	auto context = m_deviceResources->GetD3DDeviceContext();

	XMStoreFloat4x4( &m_constantBufferData.view, XMMatrixTranspose( XMMatrixInverse( nullptr, XMLoadFloat4x4( &m_camera ) ) ) );

	UINT stride = sizeof( Vertex );
	UINT offset = 0u;

	context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	context->IASetInputLayout( m_inputLayout.Get() );

	context->VSSetShader( m_vertexShader.Get(), nullptr, 0u );

	//////
	ID3D11DepthStencilState* dsState;
	D3D11_DEPTH_STENCIL_DESC dsDesc;
	ZEROSTRUCT( dsDesc );
	dsDesc.DepthEnable = FALSE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	dsDesc.StencilEnable = FALSE;
	dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	dsDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	m_deviceResources->GetD3DDevice()->CreateDepthStencilState( &dsDesc, &dsState );
	context->OMSetDepthStencilState( dsState, 1u );
	dsState->Release();
	context->PSSetShader( m_skyPixelShader.Get(), nullptr, 0u );
	context->PSSetShaderResources( 0u, 1u, &skySrv );

	DirectX::XMMATRIX camPosMat = DirectX::XMMatrixTranslation( m_camera._41, m_camera._42, m_camera._43 );
	DirectX::XMFLOAT4X4 model = m_constantBufferData.model;
	XMStoreFloat4x4( &m_constantBufferData.model, XMMatrixTranspose( camPosMat ) );
	context->UpdateSubresource1( m_constantBuffer.Get(), 0u, nullptr, &m_constantBufferData, 0u, 0u, 0u );
	context->VSSetConstantBuffers1( 0u, 1u, m_constantBuffer.GetAddressOf(), nullptr, nullptr );
	context->IASetVertexBuffers( 0u, 1u, m_skyVertexBuffer.GetAddressOf(), &stride, &offset );
	context->IASetIndexBuffer( m_skyIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0u );

	context->DrawIndexed( 36u, 0u, 0 );

	m_constantBufferData.model = model;
	context->UpdateSubresource1( m_constantBuffer.Get(), 0u, nullptr, &m_constantBufferData, 0u, 0u, 0u );
	context->VSSetConstantBuffers1( 0u, 1u, m_constantBuffer.GetAddressOf(), nullptr, nullptr );
	context->IASetVertexBuffers( 0u, 1u, m_vertexBuffer.GetAddressOf(), &stride, &offset );
	context->IASetIndexBuffer( m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0u );
	dsDesc.DepthEnable = TRUE;
	m_deviceResources->GetD3DDevice()->CreateDepthStencilState( &dsDesc, &dsState );
	context->OMSetDepthStencilState( dsState, 1u );
	dsState->Release();
	//////

	context->PSSetShader( m_pixelShader.Get(), nullptr, 0u );
	context->PSSetShaderResources( 0u, 1u, &srv );
	context->UpdateSubresource1( m_lightingCBuffer.Get(), 0u, nullptr, &m_lightingCBufferData, 0u, 0u, 0u );
	context->PSSetConstantBuffers1( 0u, 1u, m_lightingCBuffer.GetAddressOf(), nullptr, nullptr );

	context->DrawIndexed( m_indexCount, 0u, 0 );

	if ( drawPlane ) DrawPlane();
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
	if ( !renderCube )
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
		static const Vertex cubeVertices[ ] =
		{
			{ DirectX::XMFLOAT4( -0.5f, -0.5f, -0.5f, 1.0f ), DirectX::XMFLOAT4( 1.0f, 1.0f, 0.0f, 1.0f ), DirectX::XMFLOAT4( -1.0f, -1.0f, -1.0f, 1.0f ) },
			{ DirectX::XMFLOAT4( -0.5f, -0.5f, 0.5f, 1.0f ), DirectX::XMFLOAT4( 0.0f, 1.0f, 0.0f, 1.0f ), DirectX::XMFLOAT4( -1.0f, -1.0f, 1.0f, 1.0f ) },
			{ DirectX::XMFLOAT4( -0.5f, 0.5f, -0.5f, 1.0f ), DirectX::XMFLOAT4( 1.0f, 0.0f, 0.0f, 1.0f ), DirectX::XMFLOAT4( -1.0f, 1.0f, -1.0f, 1.0f ) },
			{ DirectX::XMFLOAT4( -0.5f, 0.5f, 0.5f, 1.0f ), DirectX::XMFLOAT4( 0.0f, 0.0f, 0.0f, 1.0f ), DirectX::XMFLOAT4( -1.0f, 1.0f, 1.0f, 1.0f ) },
			{ DirectX::XMFLOAT4( 0.5f, -0.5f, -0.5f, 1.0f ), DirectX::XMFLOAT4( 0.0f, 1.0f, 0.0f, 1.0f ), DirectX::XMFLOAT4( 1.0f, -1.0f, -1.0f, 1.0f ) },
			{ DirectX::XMFLOAT4( 0.5f, -0.5f, 0.5f, 1.0f ), DirectX::XMFLOAT4( 1.0f, 1.0f, 0.0f, 1.0f ), DirectX::XMFLOAT4( 1.0f, -1.0f, 1.0f, 1.0f ) },
			{ DirectX::XMFLOAT4( 0.5f, 0.5f, -0.5f, 1.0f ), DirectX::XMFLOAT4( 0.0f, 0.0f, 0.0f, 1.0f ), DirectX::XMFLOAT4( 1.0f, 1.0f, -1.0f, 1.0f ) },
			{ DirectX::XMFLOAT4( 0.5f, 0.5f, 0.5f, 1.0f ), DirectX::XMFLOAT4( 1.0f, 0.0f, 0.0f, 1.0f ) , DirectX::XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f ) }
		};
		static const unsigned int cubeIndices[ ] =
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
		memcpy_s( outVertices, sizeof( Vertex ) * outNumVertices, cubeVertices, sizeof( cubeVertices ) );
		memcpy_s( outIndices, sizeof( unsigned int ) * outNumIndices, cubeIndices, sizeof( cubeIndices ) );
	}
}
void SceneRenderer::ObjMesh_Unload( Vertex*& vertices, unsigned int*& indices )
{
	delete[ ] vertices;
	delete[ ] indices;
	vertices = nullptr;
	indices = nullptr;
}

void SceneRenderer::CreateDeviceDependentResources( void )
{
	auto loadPSTask = DX::ReadDataAsync( L"PixelShader.cso" );
	auto loadSkyPSTask = DX::ReadDataAsync( L"PixelShaderSky.cso" );
	auto loadVSTask = DX::ReadDataAsync( L"VertexShader.cso" );
	auto createPSTask = loadPSTask.then( [ this ]( const std::vector<byte>& fileData )
	{
		DX::ThrowIfFailed( m_deviceResources->GetD3DDevice()->CreatePixelShader( &fileData[ 0 ], fileData.size(), nullptr, &m_pixelShader ) );

		CD3D11_BUFFER_DESC lightingCBufferDesc( sizeof( LightingConstantBuffer ), D3D11_BIND_CONSTANT_BUFFER );
		DX::ThrowIfFailed( m_deviceResources->GetD3DDevice()->CreateBuffer( &lightingCBufferDesc, nullptr, &m_lightingCBuffer ) );
	} );
	auto createSkyPSTask = loadSkyPSTask.then( [ this ]( const std::vector<byte>& fileData )
	{
		DX::ThrowIfFailed( m_deviceResources->GetD3DDevice()->CreatePixelShader( &fileData[ 0 ], fileData.size(), nullptr, &m_skyPixelShader ) );
	} );
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

		CD3D11_BUFFER_DESC constantBufferDesc( sizeof( ModelViewProjectionConstantBuffer ), D3D11_BIND_CONSTANT_BUFFER );
		DX::ThrowIfFailed( m_deviceResources->GetD3DDevice()->CreateBuffer( &constantBufferDesc, nullptr, &m_constantBuffer ) );
	} );
	auto createTextureTask = createPSTask.then( [ this ]()
	{
		ID3D11Device* const dev = m_deviceResources->GetD3DDevice();
		ID3D11SamplerState* samplerState;
		D3D11_SAMPLER_DESC samplerDesc;
		ZEROSTRUCT( samplerDesc );

		if ( renderCube )
		{
			D3D11_SUBRESOURCE_DATA textureSubresourceData[ star_numlevels ];
			D3D11_TEXTURE2D_DESC textureDesc;
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
			ZEROSTRUCT( textureDesc );
			ZEROSTRUCT( srvDesc );
			unsigned int* pixels = new unsigned int[ star_numpixels ];
			unsigned int i = 0u;

			textureDesc.ArraySize = 1u;
			textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
			textureDesc.SampleDesc.Count = 1u;
			textureDesc.SampleDesc.Quality = 0u;
			textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
			textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			textureDesc.CPUAccessFlags = 0u;
			textureDesc.MiscFlags = 0u;
			textureDesc.Width = star_width;
			textureDesc.Height = star_height;
			textureDesc.MipLevels = star_numlevels;
			for ( ; i < star_numpixels; ++i )
			{
				const unsigned int& x = star_pixels[ i ];
				pixels[ i ] =
					( ( x & 0xff000000u ) >> 24 ) |
					( ( x & 0x00ff0000u ) >> 8 ) |
					( ( x & 0x0000ff00u ) << 8 ) |
					( ( x & 0x000000ffu ) << 24 );
			}
			for ( i = 0u; i < star_numlevels; ++i )
			{
				ZEROSTRUCT( textureSubresourceData[ i ] );
				textureSubresourceData[ i ].pSysMem = &pixels[ star_leveloffsets[ i ] ];
				textureSubresourceData[ i ].SysMemPitch = ( star_width >> i ) * sizeof( unsigned int );
			}
			dev->CreateTexture2D( &textureDesc, textureSubresourceData, &texture );
			delete[ ] pixels;
			srvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MostDetailedMip = 0u;
			srvDesc.Texture2D.MipLevels = star_numlevels;
			dev->CreateShaderResourceView( texture, &srvDesc, &srv );
		}
		else CreateDDSTextureFromFile( dev, L"Assets\\Talon.dds", ( ID3D11Resource** )( &texture ), &srv );

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
		dev->CreateSamplerState( &samplerDesc, &samplerState );
		m_deviceResources->GetD3DDeviceContext()->PSSetSamplers( 0u, 1u, &samplerState );
		samplerState->Release();
	} );
	auto createSkyTextureTask = createSkyPSTask.then( [ this ]()
	{
		CreateDDSTextureFromFile( m_deviceResources->GetD3DDevice(), L"Assets\\Skybox.dds", ( ID3D11Resource** )&skyTexture, &skySrv );
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
	auto createSkyMeshTask = createMeshTask.then( [ this ]()
	{
		static const Vertex vertices[ 8u ] =
		{
			{ DirectX::XMFLOAT4( -0.5f, 0.5f, 0.5f, 1.0f ), DirectX::XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f ), DirectX::XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f ) },
			{ DirectX::XMFLOAT4( 0.5f, 0.5f, 0.5f, 1.0f ), DirectX::XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f ), DirectX::XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f ) },
			{ DirectX::XMFLOAT4( 0.5f, -0.5f, 0.5f, 1.0f ), DirectX::XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f ), DirectX::XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f ) },
			{ DirectX::XMFLOAT4( -0.5f, -0.5f, 0.5f, 1.0f ), DirectX::XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f ), DirectX::XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f ) },
			{ DirectX::XMFLOAT4( -0.5f, 0.5f, -0.5f, 1.0f ), DirectX::XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f ), DirectX::XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f ) },
			{ DirectX::XMFLOAT4( 0.5f, 0.5f, -0.5f, 1.0f ), DirectX::XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f ), DirectX::XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f ) },
			{ DirectX::XMFLOAT4( 0.5f, -0.5f, -0.5f, 1.0f ), DirectX::XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f ), DirectX::XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f ) },
			{ DirectX::XMFLOAT4( -0.5f, -0.5f, -0.5f, 1.0f ), DirectX::XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f ) , DirectX::XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f ) }
		};
		static const unsigned int indices[ 36u ] =
		{
			0u, 1u, 2u, 0u, 2u, 3u,
			0u, 3u, 4u, 0u, 4u, 1u,
			1u, 4u, 5u, 1u, 5u, 2u,
			2u, 5u, 6u, 2u, 6u, 3u,
			3u, 6u, 7u, 3u, 7u, 4u,
			4u, 6u, 5u, 4u, 7u, 6u
		};

		D3D11_SUBRESOURCE_DATA vertexBufferData;
		ZEROSTRUCT( vertexBufferData );
		vertexBufferData.pSysMem = vertices;
		CD3D11_BUFFER_DESC vertexBufferDesc( sizeof( Vertex ) * 8u, D3D11_BIND_VERTEX_BUFFER );
		DX::ThrowIfFailed( m_deviceResources->GetD3DDevice()->CreateBuffer( &vertexBufferDesc, &vertexBufferData, &m_skyVertexBuffer ) );

		D3D11_SUBRESOURCE_DATA indexBufferData;
		ZEROSTRUCT( indexBufferData );
		indexBufferData.pSysMem = indices;
		CD3D11_BUFFER_DESC indexBufferDesc( sizeof( unsigned int ) * 36u, D3D11_BIND_INDEX_BUFFER );
		DX::ThrowIfFailed( m_deviceResources->GetD3DDevice()->CreateBuffer( &indexBufferDesc, &indexBufferData, &m_skyIndexBuffer ) );
	} );
	( createSkyMeshTask && createTextureTask && createSkyTextureTask ).then( [ this ]()
	{

	} ).then( [ this ]() { m_loadingComplete = true; } );
}

void SceneRenderer::ReleaseDeviceDependentResources( void )
{
	m_loadingComplete = false;
	m_vertexShader.Reset();
	m_inputLayout.Reset();
	m_pixelShader.Reset();
	m_skyPixelShader.Reset();
	m_constantBuffer.Reset();
	m_vertexBuffer.Reset();
	m_skyVertexBuffer.Reset();
	m_indexBuffer.Reset();
	m_skyIndexBuffer.Reset();
	m_lightingCBuffer.Reset();
	skyTexture->Release();
	skySrv->Release();
	texture->Release();
	srv->Release();
}