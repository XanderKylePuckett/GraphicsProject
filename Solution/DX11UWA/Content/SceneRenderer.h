#pragma once

#include "..\\Common\\DeviceResources.h"
#include "ShaderStructures.h"
#include "..\\Common\\StepTimer.h"
#include <vector>

namespace DX11UWA
{
	// This sample renderer instantiates a basic rendering pipeline.
	class SceneRenderer
	{
	public:
		SceneRenderer( const std::shared_ptr<DX::DeviceResources>& deviceResources );
		void CreateDeviceDependentResources( void );
		void CreateWindowSizeDependentResources( void );
		void ReleaseDeviceDependentResources( void );
		void Update( DX::StepTimer const& timer );
		bool Render( void );
		void Draw( ID3D11Texture2D*& surface );

		// Helper functions for keyboard and mouse input
		void SetInputDeviceData( const char* kb, const Windows::UI::Input::PointerPoint^ pos );


	private:
		struct IndexTriangle
		{
			unsigned int position[ 3 ];
			unsigned int texcoord[ 3 ];
			unsigned int normal[ 3 ];
		};
		void UpdateLights( DX::StepTimer const& );
		void AnimateMesh( DX::StepTimer const& );
		void UpdateCamera( DX::StepTimer const&, float const, float const );
		static void ObjMesh_ToBuffer( Vertex*&, unsigned int*&, unsigned int&, unsigned int&,
									  const std::vector<DirectX::XMFLOAT3>&, const std::vector<DirectX::XMFLOAT2>&,
									  const std::vector<DirectX::XMFLOAT3>&, const std::vector<IndexTriangle>& );
		static void ObjMesh_LoadMesh( const char* const, Vertex*&, unsigned int*&, unsigned int&, unsigned int& );
		static void ObjMesh_Unload( Vertex*&, unsigned int*& );

		void DrawPlane( void );
		void ToggleWireframe( void );

		bool SceneRenderer::KeyHit( char );

	private:
		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_inputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_talonVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_talonIndexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_cubeVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_cubeIndexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_skyVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_skyIndexBuffer;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_skyVertexShader;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_postVertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_skyPixelShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_postPS[ 4 ];
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_constantBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_lightingBuffer;

		ModelViewProjectionConstantBuffer	m_constantBufferData;
		LightingBuffer	m_lightingBufferData;
		uint32	m_talonIndexCount;
		uint32	m_cubeIndexCount;

		ID3D11Texture2D* m_talonTexture;
		ID3D11ShaderResourceView* m_talonTexSrv;
		ID3D11Texture2D* m_cubeTexture;
		ID3D11ShaderResourceView* m_cubeTexSrv;
		ID3D11Texture2D* m_skyTexture;
		ID3D11ShaderResourceView* m_skySrv;

		// Variables used with the rendering loop.
		bool	m_loadingComplete;
		float	m_degreesPerSecond;
		bool m_drawPlane;
		unsigned int m_currPPPS;
		bool m_renderCube;

		// Data members for keyboard and mouse input
		char	m_kbuttons[ 256 ];
		Windows::UI::Input::PointerPoint^ m_currMousePos;
		Windows::UI::Input::PointerPoint^ m_prevMousePos;

		// Matrix data member for the camera
		DirectX::XMFLOAT4X4 m_camera;
	};
}