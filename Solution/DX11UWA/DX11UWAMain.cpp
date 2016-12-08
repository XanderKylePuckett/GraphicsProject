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
	if ( 0u == m_timer.GetFrameCount() ) return false;

	auto context = m_deviceResources->GetD3DDeviceContext();

	auto viewport = m_deviceResources->GetScreenViewport();
	context->RSSetViewports( 1u, &viewport );

	m_sceneRenderer->DrawRTTScene();
	ID3D11Texture2D* drawSurface;
	m_sceneRenderer->CreateDrawSurface( &drawSurface );
	if ( m_sceneRenderer->Render() )
		m_sceneRenderer->DrawSurfaceToScreen( drawSurface );
	m_fpsTextRenderer->Render();

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