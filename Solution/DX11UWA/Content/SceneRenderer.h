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
		void Render( void );

		// Helper functions for keyboard and mouse input
		void SetKeyboardButtons( const char* list );
		void SetMousePosition( const Windows::UI::Input::PointerPoint^ pos );
		void SetInputDeviceData( const char* kb, const Windows::UI::Input::PointerPoint^ pos );


	private:
		struct IndexTriangle
		{
			unsigned int p[ 3 ];
			unsigned int t[ 3 ];
			unsigned int n[ 3 ];
		};
		void AnimateMesh( DX::StepTimer const& radians );
		void UpdateCamera( DX::StepTimer const& timer, float const moveSpd, float const rotSpd );
		static void ObjMesh_ToBuffer( Vertex*&, unsigned int*&, unsigned int&, unsigned int&,
									  const std::vector<DirectX::XMFLOAT3>&, const std::vector<DirectX::XMFLOAT2>&,
									  const std::vector<DirectX::XMFLOAT3>&, const std::vector<IndexTriangle>& );
		static void ObjMesh_LoadMesh( const char* const, Vertex*&, unsigned int*&, unsigned int&, unsigned int& );
		static void ObjMesh_Unload( Vertex*&, unsigned int*& );

	private:
		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		// Direct3D resources for cube geometry.
		Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_inputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_vertexBuffer2;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_indexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_indexBuffer2;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_pixelShader2;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_constantBuffer;

		ModelViewProjectionConstantBuffer	m_constantBufferData;
		uint32	m_indexCount;

		ID3D11Texture2D* texture;
		ID3D11ShaderResourceView* srv;
		ID3D11Texture2D* texture2;
		ID3D11ShaderResourceView* srv2;

		// Variables used with the rendering loop.
		bool	m_loadingComplete;
		float	m_degreesPerSecond;

		// Data members for keyboard and mouse input
		char	m_kbuttons[ 256 ];
		Windows::UI::Input::PointerPoint^ m_currMousePos;
		Windows::UI::Input::PointerPoint^ m_prevMousePos;

		// Matrix data member for the camera
		DirectX::XMFLOAT4X4 m_camera;
	};
}