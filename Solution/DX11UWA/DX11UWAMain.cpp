#include "pch.h"
#include "DX11UWAMain.h"
#include "Common\\DirectXHelper.h"

using namespace DX11UWA;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Concurrency;

// Loads and initializes application assets when the application is loaded.
DX11UWAMain::DX11UWAMain( const std::shared_ptr<DX::DeviceResources>& deviceResources ) :
	m_deviceResources( deviceResources )
{
	// Register to be notified if the Device is lost or recreated
	m_deviceResources->RegisterDeviceNotify( this );

	// TODO: Replace this with your app's content initialization.
	m_sceneRenderer = std::unique_ptr<SceneRenderer>( new SceneRenderer( m_deviceResources ) );

	m_fpsTextRenderer = std::unique_ptr<FpsTextRenderer>( new FpsTextRenderer( m_deviceResources ) );

	// TODO: Change the timer settings if you want something other than the default variable timestep mode.
	// e.g. for 60 FPS fixed timestep update logic, call:

	//m_timer.SetFixedTimeStep(true);
	//m_timer.SetTargetElapsedSeconds(1.0 / 60);

}

DX11UWAMain::~DX11UWAMain( void )
{
	// Deregister device notification
	m_deviceResources->RegisterDeviceNotify( nullptr );
}

// Updates application state when the window size changes (e.g. device orientation change)
void DX11UWAMain::CreateWindowSizeDependentResources( void )
{
	// TODO: Replace this with the size-dependent initialization of your app's content.
	m_sceneRenderer->CreateWindowSizeDependentResources();
}

// Updates the application state once per frame.
void DX11UWAMain::Update( void )
{
	// Update scene objects.
	m_timer.Tick( [ & ]()
	{
		// TODO: Replace this with your app's content update functions.
		m_sceneRenderer->Update( m_timer );
		m_sceneRenderer->SetInputDeviceData( main_kbuttons, main_currentpos );
		m_fpsTextRenderer->Update( m_timer );
	} );
}

// Renders the current frame according to the current application state.
// Returns true if the frame was rendered and is ready to be displayed.
bool DX11UWAMain::Render( void )
{
	// Don't try to render anything before the first Update.
	if ( m_timer.GetFrameCount() == 0 )
	{
		return false;
	}

	auto context = m_deviceResources->GetD3DDeviceContext();

	//// // //
	////
	//////
	//D3D11_TEXTURE2D_DESC rttTexDesc;
	//ZEROSTRUCT( rttTexDesc );
	//rttTexDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	//rttTexDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	//rttTexDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	//////
	////
	//// // //

	// Reset the viewport to target the whole screen.
	auto viewport = m_deviceResources->GetScreenViewport();
	context->RSSetViewports( 1, &viewport );

	// Reset render targets to the screen.
	ID3D11Texture2D* drawSurface;
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
	m_deviceResources->GetD3DDevice()->CreateTexture2D( &drawSurfaceDesc, nullptr, &drawSurface );
	m_deviceResources->GetD3DDevice()->CreateRenderTargetView( drawSurface, &rtvDesc, &rtv );
	context->OMSetRenderTargets( 1u, &rtv, m_deviceResources->GetDepthStencilView() );

	// Clear the back buffer and depth stencil view.
	context->ClearRenderTargetView( rtv, DirectX::Colors::CornflowerBlue );
	context->ClearDepthStencilView( m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0ui8 );

	// Render the scene objects.
	// TODO: Replace this with your app's content rendering functions.
	bool x = m_sceneRenderer->Render();
	m_fpsTextRenderer->Render();

	ID3D11RenderTargetView *const targets[ 1u ] = { m_deviceResources->GetBackBufferRenderTargetView() };
	context->OMSetRenderTargets( 1u, targets, m_deviceResources->GetDepthStencilView() );
	context->ClearRenderTargetView( m_deviceResources->GetBackBufferRenderTargetView(), DirectX::Colors::CornflowerBlue );
	context->ClearDepthStencilView( m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0ui8 );

	if ( x ) m_sceneRenderer->Draw( drawSurface );

	rtv->Release();
	drawSurface->Release();
	return true;
}

// Notifies renderers that device resources need to be released.
void DX11UWAMain::OnDeviceLost( void )
{
	m_sceneRenderer->ReleaseDeviceDependentResources();
	m_fpsTextRenderer->ReleaseDeviceDependentResources();
}

// Notifies renderers that device resources may now be recreated.
void DX11UWAMain::OnDeviceRestored( void )
{
	m_sceneRenderer->CreateDeviceDependentResources();
	m_fpsTextRenderer->CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}

void DX11UWAMain::GetKeyboardButtons( const char* buttons )
{
	memcpy_s( main_kbuttons, sizeof( main_kbuttons ), buttons, sizeof( main_kbuttons ) );
}

void DX11UWAMain::GetMousePos( const Windows::UI::Input::PointerPoint^ pos )
{
	main_currentpos = const_cast< Windows::UI::Input::PointerPoint^ >( pos );
}