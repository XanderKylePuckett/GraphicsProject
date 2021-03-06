﻿#include "pch.h"
#include "SceneRenderer.h"
#include "..\\Common\\DirectXHelper.h"
#include "..\\Common\\DDSTextureLoader.h"
#include "Assets\\star.h"
#include <fstream>
using namespace DX11UWA;

const unsigned int numInstances = 32u;

void SceneRenderer::CreateDrawSurface( ID3D11Texture2D** pTexture )
{
	ID3D11Texture2D*& texture = *pTexture;
	ID3D11RenderTargetView* rtv;
	D3D11_TEXTURE2D_DESC drawSurfaceDesc;
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	ZEROSTRUCT( drawSurfaceDesc );
	ZEROSTRUCT( rtvDesc );
	drawSurfaceDesc.Width = ( UINT )m_deviceResources->GetOutputSize().Width;
	drawSurfaceDesc.Height = ( UINT )m_deviceResources->GetOutputSize().Height;
	drawSurfaceDesc.MipLevels = 1u;
	drawSurfaceDesc.ArraySize = 1u;
	drawSurfaceDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	drawSurfaceDesc.SampleDesc.Count = 1u;
	drawSurfaceDesc.SampleDesc.Quality = 0u;
	drawSurfaceDesc.Usage = D3D11_USAGE_DEFAULT;
	drawSurfaceDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	drawSurfaceDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
	drawSurfaceDesc.MiscFlags = 0u;
	rtvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0u;
	m_deviceResources->GetD3DDevice()->CreateTexture2D( &drawSurfaceDesc, nullptr, &texture );
	m_deviceResources->GetD3DDevice()->CreateRenderTargetView( texture, &rtvDesc, &rtv );
	m_deviceResources->GetD3DDeviceContext()->OMSetRenderTargets( 1u, &rtv, m_deviceResources->GetDepthStencilView() );
	m_deviceResources->GetD3DDeviceContext()->ClearRenderTargetView( rtv, DirectX::Colors::Black );
	m_deviceResources->GetD3DDeviceContext()->ClearDepthStencilView( m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0ui8 );
	rtv->Release();
}

void SceneRenderer::DrawSurfaceToScreen( ID3D11Texture2D*& _texture )
{
	if ( !m_loadingComplete ) return;

	ID3D11DeviceContext3* const context = m_deviceResources->GetD3DDeviceContext();
	ID3D11RenderTargetView* const backBuffer = m_deviceResources->GetBackBufferRenderTargetView();
	context->OMSetRenderTargets( 1u, &backBuffer, m_deviceResources->GetDepthStencilView() );
	context->ClearRenderTargetView( m_deviceResources->GetBackBufferRenderTargetView(), DirectX::Colors::Black );
	context->ClearDepthStencilView( m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0ui8 );

	ID3D11RasterizerState* rsState;
	D3D11_RASTERIZER_DESC rsDesc;
	context->RSGetState( &rsState );
	rsState->GetDesc( &rsDesc );
	rsState->Release();
	D3D11_FILL_MODE currFillMode = rsDesc.FillMode;
	if ( D3D11_FILL_SOLID != currFillMode )
	{
		rsDesc.FillMode = D3D11_FILL_SOLID;
		m_deviceResources->GetD3DDevice()->CreateRasterizerState( &rsDesc, &rsState );
		context->RSSetState( rsState );
		rsState->Release();
	}

	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;
	ID3D11ShaderResourceView* surfaceSrv;
	ID3D11Device3* const device = m_deviceResources->GetD3DDevice();
	UINT stride = sizeof( Vertex );
	UINT offset = 0u;
	D3D11_SUBRESOURCE_DATA vertexBufferData;
	D3D11_SUBRESOURCE_DATA indexBufferData;
	ZEROSTRUCT( vertexBufferData );
	ZEROSTRUCT( indexBufferData );
	CD3D11_BUFFER_DESC vertexBufferDesc( sizeof( Vertex ) * 4u, D3D11_BIND_VERTEX_BUFFER );
	CD3D11_BUFFER_DESC indexBufferDesc( sizeof( unsigned int ) * 6u, D3D11_BIND_INDEX_BUFFER );
	static float ttt = 1.0f;
	if ( m_kbuttons[ 'N' ] ) ttt -= 0.1f;
	if ( m_kbuttons[ 'M' ] ) ttt += 0.1f;
	Vertex vertices[ 4 ] =
	{
		{ DirectX::XMFLOAT4( -1.0f, 1.0f, 0.0f, 1.0f ), DirectX::XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f ), DirectX::XMFLOAT4( 0.0f, 1.0f, 0.0f, 1.0f ) },
		{ DirectX::XMFLOAT4( 1.0f, 1.0f, 0.0f, 1.0f ), DirectX::XMFLOAT4( ttt, 0.0f, 0.0f, 0.0f ), DirectX::XMFLOAT4( 0.0f, 1.0f, 0.0f, 1.0f ) },
		{ DirectX::XMFLOAT4( 1.0f, -1.0f, 0.0f, 1.0f ), DirectX::XMFLOAT4( ttt, ttt, 0.0f, 0.0f ), DirectX::XMFLOAT4( 0.0f, 1.0f, 0.0f, 1.0f ) },
		{ DirectX::XMFLOAT4( -1.0f, -1.0f, 0.0f, 1.0f ), DirectX::XMFLOAT4( 0.0f, ttt, 0.0f, 0.0f ), DirectX::XMFLOAT4( 0.0f, 1.0f, 0.0f, 1.0f ) },
	};
	static const unsigned int indices[ 6 ] = { 0u, 1u, 2u, 0u, 2u, 3u };
	vertexBufferData.pSysMem = vertices;
	indexBufferData.pSysMem = indices;

	device->CreateBuffer( &vertexBufferDesc, &vertexBufferData, &vertexBuffer );
	device->CreateBuffer( &indexBufferDesc, &indexBufferData, &indexBuffer );

	context->IASetVertexBuffers( 0u, 1u, &vertexBuffer, &stride, &offset );
	context->IASetIndexBuffer( indexBuffer, DXGI_FORMAT_R32_UINT, 0u );
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZEROSTRUCT( srvDesc );
	srvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1u;
	srvDesc.Texture2D.MostDetailedMip = 0u;

	device->CreateShaderResourceView( _texture, &srvDesc, &surfaceSrv );
	context->VSSetShader( m_postVertexShader.Get(), nullptr, 0u );
	context->PSSetShader( m_postPS[ m_currPPPS ].Get(), nullptr, 0u );
	context->PSSetShaderResources( 0u, 1u, &surfaceSrv );

	context->DrawIndexed( 6u, 0u, 0 );

	vertexBuffer->Release();
	indexBuffer->Release();
	surfaceSrv->Release();

	if ( D3D11_FILL_SOLID != currFillMode )
	{
		rsDesc.FillMode = currFillMode;
		m_deviceResources->GetD3DDevice()->CreateRasterizerState( &rsDesc, &rsState );
		context->RSSetState( rsState );
		rsState->Release();
	}
}

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
	rsDesc.FillMode = D3D11_FILL_WIREFRAME == rsDesc.FillMode ? D3D11_FILL_SOLID : D3D11_FILL_WIREFRAME;
	m_deviceResources->GetD3DDevice()->CreateRasterizerState( &rsDesc, &rsState );
	m_deviceResources->GetD3DDeviceContext()->RSSetState( rsState );
	rsState->Release();
}

// Loads vertex and pixel shaders from files and instantiates the cube geometry.
SceneRenderer::SceneRenderer( const std::shared_ptr<DX::DeviceResources>& deviceResources ) :
	m_loadingComplete( false ),
	m_drawPlane( true ),
	m_degreesPerSecond( 45.0f ),
	m_talonIndexCount( 0u ),
	m_deviceResources( deviceResources ),
	m_currPPPS( 0u ),
	m_renderCube( true )
{
	memset( m_kbuttons, 0, sizeof( m_kbuttons ) );
	m_currMousePos = nullptr;
	m_prevMousePos = nullptr;
	memset( &m_camera, 0, sizeof( DirectX::XMFLOAT4X4 ) );

	m_lightingBufferData.dLightDirection = DirectX::XMFLOAT4( -1.0f, -1.0f, 1.0f, 1.0f );
	m_lightingBufferData.lightState = DirectX::XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f );
	m_lightingBufferData.pLightPos[ 0 ] = DirectX::XMFLOAT4( 0.0f, -0.5f, 2.0f, 1.0f );
	m_lightingBufferData.pLightPos[ 1 ] = DirectX::XMFLOAT4( 1.7320508f, -0.5f, -1.0f, 1.0f );
	m_lightingBufferData.pLightPos[ 2 ] = DirectX::XMFLOAT4( -1.7320508f, -0.5f, -1.0f, 1.0f );
	m_lightingBufferData.pLightColorRadius[ 0 ] = DirectX::XMFLOAT4( 1.0f, 0.0f, 0.0f, 10.0f );
	m_lightingBufferData.pLightColorRadius[ 1 ] = DirectX::XMFLOAT4( 0.0f, 1.0f, 0.0f, 10.0f );
	m_lightingBufferData.pLightColorRadius[ 2 ] = DirectX::XMFLOAT4( 0.0f, 0.0f, 1.0f, 10.0f );

	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();

	static const float fovAngleY = DirectX::XMConvertToRadians( 70.0f );
	DirectX::XMMATRIX perspectiveMatrix = DirectX::XMMatrixPerspectiveFovLH( fovAngleY, 1.0f, 0.01f, 100.0f );
	DirectX::XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();
	DirectX::XMMATRIX orientationMatrix = XMLoadFloat4x4( &orientation );
	static const DirectX::XMVECTORF32 eye = { 0.0f, 0.0f, -2.0f, 0.0f };
	static const DirectX::XMVECTORF32 at = { 0.0f, 0.0f, 0.0f, 0.0f };
	static const DirectX::XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };
	for ( unsigned int i = 0u; i < NUM_RTT_TRIS; ++i )
	{
		XMStoreFloat4x4( &m_rttConstantBufferDatas[ i ].projection, XMMatrixTranspose( perspectiveMatrix * orientationMatrix ) );
		XMStoreFloat4x4( &m_rttConstantBufferDatas[ i ].model, DirectX::XMMatrixIdentity() );
		XMStoreFloat4x4( &m_rttConstantBufferDatas[ i ].view, XMMatrixTranspose( XMMatrixLookAtLH( eye, at, up ) ) );
	}

	ID3D11RasterizerState* rsState;
	D3D11_RASTERIZER_DESC rsDesc;
	ZEROSTRUCT( rsDesc );
	rsDesc.FillMode = D3D11_FILL_SOLID;
	rsDesc.CullMode = D3D11_CULL_BACK;
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

	D3D11_BLEND_DESC bsDesc;
	ZEROSTRUCT( bsDesc );
	ID3D11BlendState* blendState = nullptr;
	UINT sampleMask = 0xffffffffu;
	m_deviceResources->GetD3DDeviceContext()->OMGetBlendState( &blendState, nullptr, &sampleMask );
	if ( blendState )
	{
		blendState->GetDesc( &bsDesc );
		blendState->Release();
	}
	bsDesc.RenderTarget[ 0 ].BlendEnable = TRUE;
	bsDesc.RenderTarget[ 0 ].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	bsDesc.RenderTarget[ 0 ].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	bsDesc.RenderTarget[ 0 ].BlendOp = D3D11_BLEND_OP_ADD;
	bsDesc.RenderTarget[ 0 ].SrcBlendAlpha = D3D11_BLEND_ONE;
	bsDesc.RenderTarget[ 0 ].DestBlendAlpha = D3D11_BLEND_ZERO;
	bsDesc.RenderTarget[ 0 ].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	bsDesc.RenderTarget[ 0 ].RenderTargetWriteMask = 0x0f;
	m_deviceResources->GetD3DDevice()->CreateBlendState( &bsDesc, &blendState );
	m_deviceResources->GetD3DDeviceContext()->OMSetBlendState( blendState, nullptr, sampleMask );
	blendState->Release();
}

// Initializes view parameters when the window size changes.
void SceneRenderer::CreateWindowSizeDependentResources( void )
{
	Windows::Foundation::Size outputSize = m_deviceResources->GetOutputSize();
	float aspectRatio = outputSize.Width / outputSize.Height;
	static const float fovAngleY = DirectX::XMConvertToRadians( 70.0f );

	DirectX::XMMATRIX perspectiveMatrix = DirectX::XMMatrixPerspectiveFovLH( fovAngleY, aspectRatio, 0.01f, 100.0f );

	DirectX::XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

	DirectX::XMMATRIX orientationMatrix = XMLoadFloat4x4( &orientation );

	XMStoreFloat4x4( &m_constantBufferData.projection, XMMatrixTranspose( perspectiveMatrix * orientationMatrix ) );

	static const DirectX::XMVECTORF32 eye = { 0.0f, 1.7f, -2.5f, 0.0f };
	static const DirectX::XMVECTORF32 at = { 0.0f, 0.0f, 0.0f, 0.0f };
	static const DirectX::XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

	XMStoreFloat4x4( &m_camera, XMMatrixInverse( nullptr, XMMatrixLookAtLH( eye, at, up ) ) );
	XMStoreFloat4x4( &m_constantBufferData.view, XMMatrixTranspose( XMMatrixLookAtLH( eye, at, up ) ) );
}

void SceneRenderer::UpdateLights( DX::StepTimer const& timer )
{
	static bool lightAnim = true;
	if ( KeyHit( 'T' ) ) lightAnim = !lightAnim;
	if ( KeyHit( '1' ) ) m_lightingBufferData.lightState.x = 0.5f < m_lightingBufferData.lightState.x ? 0.0f : 1.0f;
	if ( KeyHit( '2' ) ) m_lightingBufferData.lightState.y = 0.5f < m_lightingBufferData.lightState.y ? 0.0f : 1.0f;

	if ( lightAnim )
	{
		static double animTime = 0.0;
		animTime += timer.GetElapsedSeconds();
		static const DirectX::XMFLOAT4 dir( -1.0f, -1.0f, 1.0f, 1.0f );
		DirectX::XMVECTOR v = DirectX::XMLoadFloat4( &dir );
		const DirectX::XMMATRIX m = DirectX::XMMatrixRotationY( ( float )( -2.0 * animTime ) );
		m_lightingBufferData.pLightPos[ 0 ] = DirectX::XMFLOAT4( 0.0f, -0.5f, 2.0f, 1.0f );
		m_lightingBufferData.pLightPos[ 1 ] = DirectX::XMFLOAT4( 1.7320508f, -0.5f, -1.0f, 1.0f );
		m_lightingBufferData.pLightPos[ 2 ] = DirectX::XMFLOAT4( -1.7320508f, -0.5f, -1.0f, 1.0f );
		for ( unsigned int i = 0u; i < 3u; ++i )
			DirectX::XMStoreFloat4( &m_lightingBufferData.pLightPos[ i ], DirectX::XMVector3Transform(
			DirectX::XMLoadFloat4( &m_lightingBufferData.pLightPos[ i ] ), m ) );
		v = DirectX::XMVector3Transform( v, m );
		DirectX::XMStoreFloat4( &m_lightingBufferData.dLightDirection, v );
	}
}

// Called once per frame, rotates the cube and calculates the model and view matrices.
void SceneRenderer::Update( DX::StepTimer const& timer )
{
	if ( m_loadingComplete )
	{
		if ( KeyHit( 'K' ) )
			ToggleWireframe();
		UpdateRTTScene( timer );
	}

	AnimateMesh( timer );
	UpdateLights( timer );

	if ( KeyHit( '4' ) ) m_currPPPS = 0u;
	if ( KeyHit( '5' ) ) m_currPPPS = 1u;
	if ( KeyHit( '6' ) ) m_currPPPS = 2u;
	if ( KeyHit( '7' ) ) m_currPPPS = 3u;

	// Update or move camera here
	UpdateCamera( timer, 1.5f, 0.75f );

	if ( KeyHit( 'R' ) ) m_renderCube = !m_renderCube;
	if ( KeyHit( 'P' ) ) m_drawPlane = !m_drawPlane;

	Windows::Foundation::Size outputSize = m_deviceResources->GetOutputSize();
	const float aspectRatio = outputSize.Width / outputSize.Height;
	static float fov = 70.0f;
	if ( m_kbuttons[ '8' ] ) fov -= ( float )timer.GetElapsedSeconds() * 30.0f;
	if ( m_kbuttons[ '9' ] ) fov += ( float )timer.GetElapsedSeconds() * 30.0f;
	const float fovAngleY = DirectX::XMConvertToRadians( fov );
	const DirectX::XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();
	static const float nearZ = 0.01f;
	static const float farZ = 100.0f;
	XMStoreFloat4x4( &m_constantBufferData.projection, XMMatrixTranspose(
		DirectX::XMMatrixPerspectiveFovLH( fovAngleY, aspectRatio, nearZ, farZ ) *
		XMLoadFloat4x4( &orientation ) ) );
}

void SceneRenderer::AnimateMesh( DX::StepTimer const& timer )
{
	XMStoreFloat4x4( &m_constantBufferData.model,
					 XMMatrixTranspose(
					 DirectX::XMMatrixRotationY(
					 fmodf( ( float )timer.GetTotalSeconds() *
					 DirectX::XMConvertToRadians( m_degreesPerSecond ),
					 DirectX::XM_2PI ) ) ) );
}

void SceneRenderer::UpdateCamera( DX::StepTimer const& timer, float const moveSpd, float const rotSpd )
{
	const float dt = ( float )timer.GetElapsedSeconds();
	const float dCom = dt * moveSpd;
	DirectX::XMFLOAT3 dPos( 0.0f, 0.0f, 0.0f );
	if ( m_kbuttons[ 'W' ] ) dPos.z += dCom;
	if ( m_kbuttons[ 'S' ] ) dPos.z -= dCom;
	if ( m_kbuttons[ 'A' ] ) dPos.x -= dCom;
	if ( m_kbuttons[ 'D' ] ) dPos.x += dCom;
	if ( m_kbuttons[ VK_SHIFT ] ) dPos.y -= dCom;
	if ( m_kbuttons[ VK_SPACE ] ) dPos.y += dCom;
	XMStoreFloat4x4( &m_camera, XMMatrixMultiply( DirectX::XMMatrixTranslation( dPos.x, dPos.y, dPos.z ), XMLoadFloat4x4( &m_camera ) ) );

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

			DirectX::XMMATRIX rotX = DirectX::XMMatrixRotationX( dy * rotSpd * dt );
			DirectX::XMMATRIX rotY = DirectX::XMMatrixRotationY( dx * rotSpd * dt );
			XMStoreFloat4x4( &m_camera, XMMatrixMultiply( XMMatrixMultiply( rotX, XMLoadFloat4x4( &m_camera ) ), rotY ) );

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
	ID3D11DeviceContext3* const context = m_deviceResources->GetD3DDeviceContext();
	ID3D11Device3* const device = m_deviceResources->GetD3DDevice();

	UINT stride = sizeof( Vertex );
	UINT offset = 0u;
	context->IASetVertexBuffers( 0u, 1u, &m_planeVertexBuffer, &stride, &offset );
	context->IASetIndexBuffer( m_planeIndexBuffer, DXGI_FORMAT_R32_UINT, 0u );
	context->VSSetShader( m_vertexShader.Get(), nullptr, 0u );
	context->PSSetShaderResources( 0u, 1u, &m_planeSrv );

	ModelViewProjectionConstantBuffer mvpCbufferData;
	XMStoreFloat4x4( &mvpCbufferData.model, DirectX::XMMatrixIdentity() );
	mvpCbufferData.projection = m_constantBufferData.projection;
	mvpCbufferData.view = m_constantBufferData.view;
	context->UpdateSubresource1( m_planeConstantBuffer, 0u, nullptr, &mvpCbufferData, 0u, 0u, 0u );
	context->VSSetConstantBuffers1( 0u, 1u, &m_planeConstantBuffer, nullptr, nullptr );

	context->DrawIndexed( 6u, 0u, 0 );
}

void SceneRenderer::UpdateRTTScene( DX::StepTimer const& timer )
{
	float radians = ( float )timer.GetTotalSeconds() * DirectX::XMConvertToRadians( m_degreesPerSecond );
	for ( unsigned int i = 0u; i < NUM_RTT_TRIS; ++i )
		XMStoreFloat4x4( &m_rttConstantBufferDatas[ i ].model,
						 XMMatrixTranspose( DirectX::XMMatrixRotationZ(
						 radians * -3.065f * ( 1.0f - i / ( float )NUM_RTT_TRIS ) ) ) );
}

void SceneRenderer::DrawRTTScene( void )
{
	if ( !m_loadingComplete ) return;

	auto context = m_deviceResources->GetD3DDeviceContext();
	m_deviceResources->GetD3DDeviceContext()->OMSetRenderTargets( 1u, &m_rttRtv, m_rttDsv );
	m_deviceResources->GetD3DDeviceContext()->ClearRenderTargetView( m_rttRtv, DirectX::Colors::White );
	m_deviceResources->GetD3DDeviceContext()->ClearDepthStencilView( m_rttDsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0ui8 );
	context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	context->IASetInputLayout( m_inputLayout.Get() );
	context->VSSetShader( m_rttVertexShader.Get(), nullptr, 0u );
	context->PSSetShader( m_rttPixelShader.Get(), nullptr, 0u );
	UINT stride = sizeof( Vertex );
	UINT offset = 0u;
	D3D11_VIEWPORT screenViewport = m_deviceResources->GetScreenViewport();
	D3D11_VIEWPORT rttViewport = CD3D11_VIEWPORT( 0.0f, 0.0f, ( float )m_rttSize, ( float )m_rttSize );
	context->RSSetViewports( 1u, &rttViewport );

	for ( unsigned int i = 0u; i < NUM_RTT_TRIS; ++i )
	{
		context->UpdateSubresource1( m_rttConstantBuffers[ i ].Get(), 0u, nullptr, &m_rttConstantBufferDatas[ i ], 0u, 0u, 0u );
		context->VSSetConstantBuffers1( 0u, 1u, m_rttConstantBuffers[ i ].GetAddressOf(), nullptr, nullptr );
		context->IASetVertexBuffers( 0u, 1u, m_rttVertexBuffers[ i ].GetAddressOf(), &stride, &offset );
		context->IASetIndexBuffer( m_rttIndexBuffers[ i ].Get(), DXGI_FORMAT_R32_UINT, 0u );
		context->DrawIndexed( m_rttIndexCounts[ i ], 0u, 0 );
	}

	context->RSSetViewports( 1u, &screenViewport );
}

// Renders one frame using the vertex and pixel shaders.
bool SceneRenderer::Render( void )
{
	if ( !m_loadingComplete ) return false;

	auto context = m_deviceResources->GetD3DDeviceContext();

	XMStoreFloat4x4( &m_constantBufferData.view, XMMatrixTranspose( XMMatrixInverse( nullptr, XMLoadFloat4x4( &m_camera ) ) ) );

	UINT stride = sizeof( Vertex );
	UINT offset = 0u;

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
	context->VSSetShader( m_skyVertexShader.Get(), nullptr, 0u );
	context->PSSetShader( m_skyPixelShader.Get(), nullptr, 0u );
	context->PSSetShaderResources( 0u, 1u, &m_skySrv );

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
	dsDesc.DepthEnable = TRUE;
	m_deviceResources->GetD3DDevice()->CreateDepthStencilState( &dsDesc, &dsState );
	context->OMSetDepthStencilState( dsState, 1u );
	dsState->Release();
	context->VSSetShader( m_vertexShaderInst.Get(), nullptr, 0u );
	context->PSSetShader( m_pixelShader.Get(), nullptr, 0u );
	context->UpdateSubresource1( m_lightingBuffer.Get(), 0u, nullptr, &m_lightingBufferData, 0u, 0u, 0u );
	context->PSSetConstantBuffers1( 0u, 1u, m_lightingBuffer.GetAddressOf(), nullptr, nullptr );
	static const UINT strideInst[ 2u ] = { sizeof( Vertex ), sizeof( InstanceData ) };
	static const UINT offsetInst[ 2u ] = { 0u, 0u };
	ID3D11Buffer* vsbuffers[ 2u ];
	vsbuffers[ 1u ] = m_instanceBuffer;
	context->IASetInputLayout( m_inputLayoutInst.Get() );
	if ( m_renderCube )
	{
		vsbuffers[ 0u ] = m_cubeVertexBuffer.Get();
		context->IASetVertexBuffers( 0u, 2u, vsbuffers, strideInst, offsetInst );
		context->IASetIndexBuffer( m_cubeIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0u );
		context->PSSetShaderResources( 0u, 1u, &m_rttSrv );
		context->DrawIndexedInstanced( m_cubeIndexCount, numInstances, 0u, 0, 0u );
	}
	else
	{
		vsbuffers[ 0u ] = m_talonVertexBuffer.Get();
		context->IASetVertexBuffers( 0u, 2u, vsbuffers, strideInst, offsetInst );
		context->IASetIndexBuffer( m_talonIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0u );
		context->PSSetShaderResources( 0u, 1u, &m_talonTexSrv );
		context->DrawIndexedInstanced( m_talonIndexCount, numInstances, 0u, 0, 0u );
	}
	context->IASetInputLayout( m_inputLayout.Get() );
	if ( m_drawPlane ) DrawPlane();
	return true;
}

bool operator==( const Vertex& lhs, const Vertex& rhs )
{
	return
		lhs.position.x == rhs.position.x &&
		lhs.position.y == rhs.position.y &&
		lhs.position.z == rhs.position.z &&
		lhs.texcoord.x == rhs.texcoord.x &&
		lhs.texcoord.y == rhs.texcoord.y &&
		lhs.normal.x == rhs.normal.x &&
		lhs.normal.y == rhs.normal.y &&
		lhs.normal.z == rhs.normal.z;
}
void SceneRenderer::ObjMesh_ToBuffer( Vertex*& outVertices, unsigned int*& outIndices,
									  unsigned int& outNumVertices, unsigned int& outNumIndices,
									  const std::vector<DirectX::XMFLOAT3>& positions,
									  const std::vector<DirectX::XMFLOAT2>& texcoords,
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
	for ( const IndexTriangle& triangle : triangles ) for ( int triVertIdx = 0; triVertIdx < 3; ++triVertIdx )
	{
		tempVert.position.x = positions[ triangle.position[ triVertIdx ] - 1u ].x;
		tempVert.position.y = positions[ triangle.position[ triVertIdx ] - 1u ].y;
		tempVert.position.z = positions[ triangle.position[ triVertIdx ] - 1u ].z;
		tempVert.texcoord.x = texcoords[ triangle.texcoord[ triVertIdx ] - 1u ].x;
		tempVert.texcoord.y = texcoords[ triangle.texcoord[ triVertIdx ] - 1u ].y;
		tempVert.normal.x = normals[ triangle.normal[ triVertIdx ] - 1u ].x;
		tempVert.normal.y = normals[ triangle.normal[ triVertIdx ] - 1u ].y;
		tempVert.normal.z = normals[ triangle.normal[ triVertIdx ] - 1u ].z;
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
	file.open( filepath, std::ios_base::in );
	if ( file.is_open() )
	{
		DirectX::XMFLOAT2 tempf2( 0.0f, 0.0f );
		DirectX::XMFLOAT3 tempf3( 0.0f, 0.0f, 0.0f );
		IndexTriangle tempTriangle;
		ZEROSTRUCT( tempTriangle );
		std::vector<DirectX::XMFLOAT3> positions;
		std::vector<DirectX::XMFLOAT2> texcoords;
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
					texcoords.push_back( tempf2 );
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
					file >> tempTriangle.position[ 0 ] >> temp >> tempTriangle.texcoord[ 0 ] >> temp >> tempTriangle.normal[ 0 ];
					file >> tempTriangle.position[ 1 ] >> temp >> tempTriangle.texcoord[ 1 ] >> temp >> tempTriangle.normal[ 1 ];
					file >> tempTriangle.position[ 2 ] >> temp >> tempTriangle.texcoord[ 2 ] >> temp >> tempTriangle.normal[ 2 ];
					triangles.push_back( tempTriangle );
				}
			}
			while ( '\n' != temp )
				file.get( temp );
		}
		file.close();
		ObjMesh_ToBuffer( outVertices, outIndices,
						  outNumVertices, outNumIndices,
						  positions, texcoords, normals, triangles );
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
			{ DirectX::XMFLOAT4( 0.5f, 0.5f, 0.5f, 1.0f ), DirectX::XMFLOAT4( 1.0f, 0.0f, 0.0f, 1.0f ), DirectX::XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f ) }
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
	auto loadPS = DX::ReadDataAsync( L"PixelShader.cso" ).then( [ this ] ( const std::vector<byte>& fData )
	{
		DX::ThrowIfFailed( m_deviceResources->GetD3DDevice()->CreatePixelShader( &fData[ 0 ], fData.size(), nullptr, &m_pixelShader ) );

		CD3D11_BUFFER_DESC lightingBufferDesc( sizeof( LightingBuffer ), D3D11_BIND_CONSTANT_BUFFER );
		DX::ThrowIfFailed( m_deviceResources->GetD3DDevice()->CreateBuffer( &lightingBufferDesc, nullptr, &m_lightingBuffer ) );
	} );
	auto loadSkyPS = DX::ReadDataAsync( L"PixelShaderSky.cso" ).then( [ this ] ( const std::vector<byte>& fData )
	{
		DX::ThrowIfFailed( m_deviceResources->GetD3DDevice()->CreatePixelShader( &fData[ 0 ], fData.size(), nullptr, &m_skyPixelShader ) );
	} );
	auto loadPPPS0 = DX::ReadDataAsync( L"PostPS0.cso" ).then( [ this ] ( const std::vector<byte>& fData )
	{
		DX::ThrowIfFailed( m_deviceResources->GetD3DDevice()->CreatePixelShader( &fData[ 0 ], fData.size(), nullptr, &m_postPS[ 0 ] ) );
	} );
	auto loadPPPS1 = DX::ReadDataAsync( L"PostPS1.cso" ).then( [ this ] ( const std::vector<byte>& fData )
	{
		DX::ThrowIfFailed( m_deviceResources->GetD3DDevice()->CreatePixelShader( &fData[ 0 ], fData.size(), nullptr, &m_postPS[ 1 ] ) );
	} );
	auto loadPPPS2 = DX::ReadDataAsync( L"PostPS2.cso" ).then( [ this ] ( const std::vector<byte>& fData )
	{
		DX::ThrowIfFailed( m_deviceResources->GetD3DDevice()->CreatePixelShader( &fData[ 0 ], fData.size(), nullptr, &m_postPS[ 2 ] ) );
	} );
	auto loadPPPS3 = DX::ReadDataAsync( L"PostPS3.cso" ).then( [ this ] ( const std::vector<byte>& fData )
	{
		DX::ThrowIfFailed( m_deviceResources->GetD3DDevice()->CreatePixelShader( &fData[ 0 ], fData.size(), nullptr, &m_postPS[ 3 ] ) );
	} );
	auto loadVS = DX::ReadDataAsync( L"VertexShader.cso" ).then( [ this ] ( const std::vector<byte>& fData )
	{
		DX::ThrowIfFailed( m_deviceResources->GetD3DDevice()->CreateVertexShader( &fData[ 0 ], fData.size(), nullptr, &m_vertexShader ) );

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[ ] =
		{
			{ "POSITION", 0u, DXGI_FORMAT_R32G32B32A32_FLOAT, 0u, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0u },
			{ "TEXCOORD", 0u, DXGI_FORMAT_R32G32B32A32_FLOAT, 0u, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0u },
			{ "NORMAL", 0u, DXGI_FORMAT_R32G32B32A32_FLOAT, 0u, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0u }
		};
		DX::ThrowIfFailed( m_deviceResources->GetD3DDevice()->CreateInputLayout( vertexDesc, ARRAYSIZE( vertexDesc ), &fData[ 0 ], fData.size(), &m_inputLayout ) );

		CD3D11_BUFFER_DESC constantBufferDesc( sizeof( ModelViewProjectionConstantBuffer ), D3D11_BIND_CONSTANT_BUFFER );
		DX::ThrowIfFailed( m_deviceResources->GetD3DDevice()->CreateBuffer( &constantBufferDesc, nullptr, &m_constantBuffer ) );
	} );
	auto loadVSI = DX::ReadDataAsync( L"VertexShaderInst.cso" ).then( [ this ] ( const std::vector<byte>& fData )
	{
		DX::ThrowIfFailed( m_deviceResources->GetD3DDevice()->CreateVertexShader( &fData[ 0 ], fData.size(), nullptr, &m_vertexShaderInst ) );
		static const D3D11_INPUT_ELEMENT_DESC vertexDescInst[ ] =
		{
			{ "POSITION", 0u, DXGI_FORMAT_R32G32B32A32_FLOAT, 0u, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0u },
			{ "TEXCOORD", 0u, DXGI_FORMAT_R32G32B32A32_FLOAT, 0u, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0u },
			{ "NORMAL", 0u, DXGI_FORMAT_R32G32B32A32_FLOAT, 0u, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0u },
			{ "INSTANCE", 0u, DXGI_FORMAT_R32G32B32A32_FLOAT, 1u, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1u }
		};
		DX::ThrowIfFailed( m_deviceResources->GetD3DDevice()->CreateInputLayout( vertexDescInst, ARRAYSIZE( vertexDescInst ), &fData[ 0 ], fData.size(), &m_inputLayoutInst ) );
	} );
	auto loadSkyVS = DX::ReadDataAsync( L"VertexShaderSky.cso" ).then( [ this ] ( const std::vector<byte>& fData )
	{
		DX::ThrowIfFailed( m_deviceResources->GetD3DDevice()->CreateVertexShader( &fData[ 0 ], fData.size(), nullptr, &m_skyVertexShader ) );
	} );
	auto loadPostVS = DX::ReadDataAsync( L"VertexShaderPost.cso" ).then( [ this ] ( const std::vector<byte>& fData )
	{
		DX::ThrowIfFailed( m_deviceResources->GetD3DDevice()->CreateVertexShader( &fData[ 0 ], fData.size(), nullptr, &m_postVertexShader ) );
	} );
	auto loadRttPS = DX::ReadDataAsync( L"PixelShaderRtt.cso" ).then( [ this ] ( const std::vector<byte>& fData )
	{
		DX::ThrowIfFailed( m_deviceResources->GetD3DDevice()->CreatePixelShader( &fData[ 0 ], fData.size(), nullptr, &m_rttPixelShader ) );
	} );
	auto loadRttVS = DX::ReadDataAsync( L"VertexShaderRtt.cso" ).then( [ this ] ( const std::vector<byte>& fData )
	{
		DX::ThrowIfFailed( m_deviceResources->GetD3DDevice()->CreateVertexShader( &fData[ 0 ], fData.size(), nullptr, &m_rttVertexShader ) );

		CD3D11_BUFFER_DESC rttConstantBufferDesc( sizeof( ModelViewProjectionConstantBuffer ), D3D11_BIND_CONSTANT_BUFFER );
		for ( unsigned int i = 0u; i < NUM_RTT_TRIS; ++i )
			m_deviceResources->GetD3DDevice()->CreateBuffer( &rttConstantBufferDesc, nullptr, &m_rttConstantBuffers[ i ] );
	} );

	auto createTextures = ( loadSkyPS && loadPS ).then( [ this ] ()
	{
		ID3D11Device* const dev = m_deviceResources->GetD3DDevice();

		CreateDDSTextureFromFile( dev, L"Assets\\Skybox.dds", ( ID3D11Resource** )&m_skyTexture, &m_skySrv );
		CreateDDSTextureFromFile( dev, L"Assets\\Plane.dds", ( ID3D11Resource** )&m_planeTexture, &m_planeSrv );

		D3D11_SUBRESOURCE_DATA cubeTexSubresourceData[ star_numlevels ];
		D3D11_TEXTURE2D_DESC cubeTextureDesc;
		D3D11_SHADER_RESOURCE_VIEW_DESC cubeSrvDesc;
		ZEROSTRUCT( cubeTextureDesc );
		ZEROSTRUCT( cubeSrvDesc );
		unsigned int* pixels = new unsigned int[ star_numpixels ];
		unsigned int i = 0u;

		cubeTextureDesc.ArraySize = 1u;
		cubeTextureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
		cubeTextureDesc.SampleDesc.Count = 1u;
		cubeTextureDesc.SampleDesc.Quality = 0u;
		cubeTextureDesc.Usage = D3D11_USAGE_IMMUTABLE;
		cubeTextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		cubeTextureDesc.CPUAccessFlags = 0u;
		cubeTextureDesc.MiscFlags = 0u;
		cubeTextureDesc.Width = star_width;
		cubeTextureDesc.Height = star_height;
		cubeTextureDesc.MipLevels = star_numlevels;
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
			ZEROSTRUCT( cubeTexSubresourceData[ i ] );
			cubeTexSubresourceData[ i ].pSysMem = &pixels[ star_leveloffsets[ i ] ];
			cubeTexSubresourceData[ i ].SysMemPitch = ( star_width >> i ) * sizeof( unsigned int );
		}
		dev->CreateTexture2D( &cubeTextureDesc, cubeTexSubresourceData, &m_cubeTexture );
		delete[ ] pixels;
		cubeSrvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
		cubeSrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		cubeSrvDesc.Texture2D.MostDetailedMip = 0u;
		cubeSrvDesc.Texture2D.MipLevels = star_numlevels;
		dev->CreateShaderResourceView( m_cubeTexture, &cubeSrvDesc, &m_cubeTexSrv );
		CreateDDSTextureFromFile( dev, L"Assets\\Talon.dds", ( ID3D11Resource** )( &m_talonTexture ), &m_talonTexSrv );

		ID3D11SamplerState* samplerState;
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
		dev->CreateSamplerState( &samplerDesc, &samplerState );
		m_deviceResources->GetD3DDeviceContext()->PSSetSamplers( 0u, 1u, &samplerState );
		samplerState->Release();
	} );
	auto createTalonMesh = loadVS.then( [ this ] ()
	{
		Vertex* vertices = nullptr;
		unsigned int* indices = nullptr;
		unsigned int numVertices = 0u, numIndices = 0u;

		ObjMesh_LoadMesh( "Assets\\Talon.mobj", vertices, indices, numVertices, numIndices );

		D3D11_SUBRESOURCE_DATA vertexBufferData;
		ZEROSTRUCT( vertexBufferData );
		vertexBufferData.pSysMem = vertices;
		CD3D11_BUFFER_DESC vertexBufferDesc( sizeof( Vertex ) * numVertices, D3D11_BIND_VERTEX_BUFFER );
		DX::ThrowIfFailed( m_deviceResources->GetD3DDevice()->CreateBuffer( &vertexBufferDesc, &vertexBufferData, &m_talonVertexBuffer ) );

		m_talonIndexCount = numIndices;

		D3D11_SUBRESOURCE_DATA indexBufferData;
		ZEROSTRUCT( indexBufferData );
		indexBufferData.pSysMem = indices;
		CD3D11_BUFFER_DESC indexBufferDesc( sizeof( unsigned int ) * numIndices, D3D11_BIND_INDEX_BUFFER );
		DX::ThrowIfFailed( m_deviceResources->GetD3DDevice()->CreateBuffer( &indexBufferDesc, &indexBufferData, &m_talonIndexBuffer ) );

		ObjMesh_Unload( vertices, indices );
	} );
	auto createCubeMesh = loadVS.then( [ this ] ()
	{
		Vertex* vertices = nullptr;
		unsigned int* indices = nullptr;
		unsigned int numVertices = 0u, numIndices = 0u;

		ObjMesh_LoadMesh( "NULL", vertices, indices, numVertices, numIndices );

		D3D11_SUBRESOURCE_DATA vertexBufferData;
		ZEROSTRUCT( vertexBufferData );
		vertexBufferData.pSysMem = vertices;
		CD3D11_BUFFER_DESC vertexBufferDesc( sizeof( Vertex ) * numVertices, D3D11_BIND_VERTEX_BUFFER );
		DX::ThrowIfFailed( m_deviceResources->GetD3DDevice()->CreateBuffer( &vertexBufferDesc, &vertexBufferData, &m_cubeVertexBuffer ) );

		m_cubeIndexCount = numIndices;

		D3D11_SUBRESOURCE_DATA indexBufferData;
		ZEROSTRUCT( indexBufferData );
		indexBufferData.pSysMem = indices;
		CD3D11_BUFFER_DESC indexBufferDesc( sizeof( unsigned int ) * numIndices, D3D11_BIND_INDEX_BUFFER );
		DX::ThrowIfFailed( m_deviceResources->GetD3DDevice()->CreateBuffer( &indexBufferDesc, &indexBufferData, &m_cubeIndexBuffer ) );

		ObjMesh_Unload( vertices, indices );
	} );
	auto createPlaneMesh = loadVS.then( [ this ] ()
	{
		CD3D11_BUFFER_DESC constantBufferDesc( sizeof( ModelViewProjectionConstantBuffer ), D3D11_BIND_CONSTANT_BUFFER );
		CD3D11_BUFFER_DESC vertexBufferDesc( sizeof( Vertex ) * 4u, D3D11_BIND_VERTEX_BUFFER );
		CD3D11_BUFFER_DESC indexBufferDesc( sizeof( unsigned int ) * 6u, D3D11_BIND_INDEX_BUFFER );

		D3D11_SUBRESOURCE_DATA vertexBufferData;
		D3D11_SUBRESOURCE_DATA indexBufferData;
		ZEROSTRUCT( vertexBufferData );
		ZEROSTRUCT( indexBufferData );
		static const Vertex vertices[ 4u ] =
		{
			{ DirectX::XMFLOAT4( -10.0f, -1.0f, -10.0f, 1.0f ), DirectX::XMFLOAT4( -8.0f, -8.0f, 0.0f, 0.0f ), DirectX::XMFLOAT4( 0.0f, 1.0f, 0.0f, 1.0f ) },
			{ DirectX::XMFLOAT4( -10.0f, -1.0f, 10.0f, 1.0f ), DirectX::XMFLOAT4( -8.0f, 8.0f, 0.0f, 0.0f ), DirectX::XMFLOAT4( 0.0f, 1.0f, 0.0f, 1.0f ) },
			{ DirectX::XMFLOAT4( 10.0f, -1.0f, 10.0f, 1.0f ), DirectX::XMFLOAT4( 8.0f, 8.0f, 0.0f, 0.0f ), DirectX::XMFLOAT4( 0.0f, 1.0f, 0.0f, 1.0f ) },
			{ DirectX::XMFLOAT4( 10.0f, -1.0f, -10.0f, 1.0f ), DirectX::XMFLOAT4( 8.0f, -8.0f, 0.0f, 0.0f ), DirectX::XMFLOAT4( 0.0f, 1.0f, 0.0f, 1.0f ) },
		};
		static const unsigned int indices[ 6u ] = { 0u, 1u, 2u, 0u, 2u, 3u };
		vertexBufferData.pSysMem = vertices;
		indexBufferData.pSysMem = indices;

		m_deviceResources->GetD3DDevice()->CreateBuffer( &constantBufferDesc, nullptr, &m_planeConstantBuffer );
		m_deviceResources->GetD3DDevice()->CreateBuffer( &vertexBufferDesc, &vertexBufferData, &m_planeVertexBuffer );
		m_deviceResources->GetD3DDevice()->CreateBuffer( &indexBufferDesc, &indexBufferData, &m_planeIndexBuffer );
	} );
	auto createSkyMesh = loadSkyVS.then( [ this ] ()
	{
		static const Vertex vertices[ 8u ] =
		{
			{ DirectX::XMFLOAT4( -2.0f, 2.0f, 2.0f, 1.0f ), DirectX::XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f ), DirectX::XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f ) },
			{ DirectX::XMFLOAT4( 2.0f, 2.0f, 2.0f, 1.0f ), DirectX::XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f ), DirectX::XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f ) },
			{ DirectX::XMFLOAT4( 2.0f, -2.0f, 2.0f, 1.0f ), DirectX::XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f ), DirectX::XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f ) },
			{ DirectX::XMFLOAT4( -2.0f, -2.0f, 2.0f, 1.0f ), DirectX::XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f ), DirectX::XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f ) },
			{ DirectX::XMFLOAT4( -2.0f, 2.0f, -2.0f, 1.0f ), DirectX::XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f ), DirectX::XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f ) },
			{ DirectX::XMFLOAT4( 2.0f, 2.0f, -2.0f, 1.0f ), DirectX::XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f ), DirectX::XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f ) },
			{ DirectX::XMFLOAT4( 2.0f, -2.0f, -2.0f, 1.0f ), DirectX::XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f ), DirectX::XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f ) },
			{ DirectX::XMFLOAT4( -2.0f, -2.0f, -2.0f, 1.0f ), DirectX::XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f ), DirectX::XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f ) }
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
	auto createRttTexture = ( loadPPPS0 && loadPPPS1 && loadPPPS2 && loadPPPS3 && loadPostVS ).then( [ this ] ()
	{
		D3D11_TEXTURE2D_DESC rttTexDesc;
		D3D11_TEXTURE2D_DESC dsTexDesc;
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;

		ZEROSTRUCT( rttTexDesc );
		ZEROSTRUCT( dsTexDesc );
		ZEROSTRUCT( rtvDesc );
		ZEROSTRUCT( srvDesc );
		ZEROSTRUCT( dsvDesc );

		rttTexDesc.Width = m_rttSize;
		rttTexDesc.Height = m_rttSize;
		rttTexDesc.MipLevels = 1u;
		rttTexDesc.ArraySize = 1u;
		rttTexDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		rttTexDesc.SampleDesc.Count = 1u;
		rttTexDesc.SampleDesc.Quality = 0u;
		rttTexDesc.Usage = D3D11_USAGE_DEFAULT;
		rttTexDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		rttTexDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
		rttTexDesc.MiscFlags = 0u;
		m_deviceResources->GetD3DDevice()->CreateTexture2D( &rttTexDesc, nullptr, &m_rttTex );

		rtvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = 0u;
		m_deviceResources->GetD3DDevice()->CreateRenderTargetView( m_rttTex, &rtvDesc, &m_rttRtv );

		srvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1u;
		srvDesc.Texture2D.MostDetailedMip = 0u;
		m_deviceResources->GetD3DDevice()->CreateShaderResourceView( m_rttTex, &srvDesc, &m_rttSrv );

		dsTexDesc.Width = m_rttSize;
		dsTexDesc.Height = m_rttSize;
		dsTexDesc.MipLevels = 1u;
		dsTexDesc.ArraySize = 1u;
		dsTexDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsTexDesc.SampleDesc.Count = 1u;
		dsTexDesc.SampleDesc.Quality = 0u;
		dsTexDesc.Usage = D3D11_USAGE_DEFAULT;
		dsTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		dsTexDesc.CPUAccessFlags = 0u;
		dsTexDesc.MiscFlags = 0u;
		m_deviceResources->GetD3DDevice()->CreateTexture2D( &dsTexDesc, nullptr, &m_rttDsTex );

		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Flags = 0u;
		dsvDesc.Texture2D.MipSlice = 0u;
		m_deviceResources->GetD3DDevice()->CreateDepthStencilView( m_rttDsTex, &dsvDesc, &m_rttDsv );

	} );
	auto createRttTriangles = ( loadRttPS && loadRttVS ).then( [ this ] ()
	{
		static const Vertex triangleVertices[ ] =
		{
			{ DirectX::XMFLOAT4( 0.0f, 1.25f, 0.0f, 1.0f ), DirectX::XMFLOAT4( 1.0f, 0.0f, 0.0f, 1.0f ), DirectX::XMFLOAT4() },
			{ DirectX::XMFLOAT4( 1.08253175f, -0.625f, 0.0f, 1.0f ), DirectX::XMFLOAT4( 0.0f, 1.0f, 0.0f, 1.0f ), DirectX::XMFLOAT4() },
			{ DirectX::XMFLOAT4( -1.08253175f, -0.625f, 0.0f, 1.0f ), DirectX::XMFLOAT4( 0.0f, 0.0f, 1.0f, 1.0f ), DirectX::XMFLOAT4() }
		};
		static const unsigned int triangleIndices[ ] = { 0u, 1u, 2u };
		D3D11_SUBRESOURCE_DATA vertexBufferData;
		D3D11_SUBRESOURCE_DATA indexBufferData;
		ZEROSTRUCT( vertexBufferData );
		ZEROSTRUCT( indexBufferData );
		vertexBufferData.pSysMem = triangleVertices;
		indexBufferData.pSysMem = triangleIndices;
		CD3D11_BUFFER_DESC vertexBufferDesc( sizeof( Vertex ) * 3u, D3D11_BIND_VERTEX_BUFFER );
		CD3D11_BUFFER_DESC indexBufferDesc( sizeof( unsigned int ) * 3u, D3D11_BIND_INDEX_BUFFER );
		for ( unsigned int i = 0u; i < NUM_RTT_TRIS; ++i )
		{
			m_rttIndexCounts[ i ] = 3u;
			m_deviceResources->GetD3DDevice()->CreateBuffer( &vertexBufferDesc, &vertexBufferData, &m_rttVertexBuffers[ i ] );
			m_deviceResources->GetD3DDevice()->CreateBuffer( &indexBufferDesc, &indexBufferData, &m_rttIndexBuffers[ i ] );
		}
	} );
	auto createInstanceBuffer = loadVSI.then( [ this ] ()
	{
		static InstanceData instances[ numInstances ];
		for ( unsigned int i = 0u; i < numInstances; ++i )
			instances[ i ].positionOffset = DirectX::XMFLOAT4( i * 1.0f, i * 0.5f, i * 2.0f, 1.0f );
		D3D11_SUBRESOURCE_DATA instanceBufferData;
		ZEROSTRUCT( instanceBufferData );
		instanceBufferData.pSysMem = instances;
		CD3D11_BUFFER_DESC instanceBufferDesc( sizeof( InstanceData ) * numInstances, D3D11_BIND_VERTEX_BUFFER );
		m_deviceResources->GetD3DDevice()->CreateBuffer( &instanceBufferDesc, &instanceBufferData, &m_instanceBuffer );
	} );

	( createTextures &&
	  createTalonMesh &&
	  createCubeMesh &&
	  createPlaneMesh &&
	  createSkyMesh &&
	  createRttTexture &&
	  createRttTriangles &&
	  createInstanceBuffer ).then( [ this ] ()
	{
		m_loadingComplete = true;
	} );
}

void SceneRenderer::ReleaseDeviceDependentResources( void )
{
	m_loadingComplete = false;
	m_vertexShader.Reset();
	m_vertexShaderInst.Reset();
	m_skyVertexShader.Reset();
	m_rttVertexShader.Reset();
	m_postVertexShader.Reset();
	m_inputLayout.Reset();
	m_inputLayoutInst.Reset();
	m_pixelShader.Reset();
	m_skyPixelShader.Reset();
	m_rttPixelShader.Reset();
	m_postPS[ 0 ].Reset();
	m_postPS[ 1 ].Reset();
	m_postPS[ 2 ].Reset();
	m_postPS[ 3 ].Reset();
	m_constantBuffer.Reset();
	m_talonVertexBuffer.Reset();
	m_talonIndexBuffer.Reset();
	m_cubeVertexBuffer.Reset();
	m_cubeIndexBuffer.Reset();
	m_skyVertexBuffer.Reset();
	m_skyIndexBuffer.Reset();
	m_lightingBuffer.Reset();
	m_skyTexture->Release();
	m_skySrv->Release();
	m_talonTexture->Release();
	m_talonTexSrv->Release();
	m_cubeTexture->Release();
	m_cubeTexSrv->Release();
	m_rttTex->Release();
	m_rttDsTex->Release();
	m_rttRtv->Release();
	m_rttSrv->Release();
	m_rttDsv->Release();
	m_planeTexture->Release();
	m_planeSrv->Release();
	m_planeVertexBuffer->Release();
	m_planeIndexBuffer->Release();
	m_planeConstantBuffer->Release();
	for ( unsigned int i = 0u; i < NUM_RTT_TRIS; ++i )
	{
		m_rttConstantBuffers[ i ].Reset();
		m_rttVertexBuffers[ i ].Reset();
		m_rttIndexBuffers[ i ].Reset();
	}
	m_instanceBuffer->Release();
}