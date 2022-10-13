/*=============================================================================*/
// Copyright 2017-2019 Elite Engine
// Authors: Matthieu Delaere
/*=============================================================================*/
// ERenderer.h: class that holds the surface to render too + DirectX initialization.
/*=============================================================================*/
#ifndef ELITE_RAYTRACING_RENDERER
#define	ELITE_RAYTRACING_RENDERER

#include <vector>

#include "Effect.h"
#include "FireEffect.h"
#include "Primitive.h"
#include "AlphaPrimitive.h"
#include "Texture.h"

struct SDL_Window;
struct SDL_Surface;
struct Vertex_Input;
class Primitive;
class BasePrimitive;
class AlphaPrimitive;

namespace Elite
{
	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void RenderHardware();
		void RenderSoftware();
		RGBColor PixelShader(const Vertex_Input& v, const FPoint3& viewMatrix, const FPoint3& pixel);
		void UpdateRotation(float elapsedSec);
		void ChangeTechnique();
		void ChangeCullMode();
		void ChangeRenderMode();
		RenderMode GetRenderMode();
		void SetIsRotating();
		void ChangeRenderFireEffect();
		void RegisterKeyPress(SDL_Scancode key);
		void ToggleVSync();

		//static FMatrix4 m_WorldMatrix;

	private:
		SDL_Window* m_pWindow;
		uint32_t m_Width;
		uint32_t m_Height;
		ID3D11Device* m_pDevice;
		ID3D11DeviceContext* m_pDeviceContext;
		IDXGIFactory* m_pDXGIFactory;
		IDXGISwapChain* m_pSwapChain;
		ID3D11Texture2D* m_pDepthStencilBuffer;
		ID3D11DepthStencilView* m_pDepthStencilView;
		ID3D11Resource* m_pRenderTargetBuffer;
		ID3D11RenderTargetView* m_pRenderTargetView;
		ID3D11Debug* m_pDebug;
		SDL_Surface* m_pFrontBuffer = nullptr;
		SDL_Surface* m_pBackBuffer = nullptr;
		uint32_t* m_pBackBufferPixels = nullptr;

		bool m_IsInitialized{ false };
		bool m_IsRotating{ true };
		bool m_IsRenderFireEffect{ true };
		bool m_DisplayEasterEgg{ false };
		bool m_IsEnabledVSync{ true };
		int m_Iter{};
		RenderMode m_RenderMode{ RenderMode::HARDWARE };
		FVector3 m_LightDirection{ 0.577f, -0.577f, 0.577f };
		D3D11_CULL_MODE m_CullMode{ D3D11_CULL_BACK };
		//FMatrix4 m_RotationMatrix;
		Effect* m_pEffect;
		FireEffect* m_pFireEffect;
		std::vector<Texture*> m_pTextures;
		Texture* m_pFireTexture;
		Texture* m_pRastaRizing;
		std::vector<BasePrimitive*> m_pPrimitives;
		std::vector<Vertex_Input> m_Vertices;
		std::vector<uint32_t> m_Indices;

		HRESULT InitializeDirectX();
		void LoadPrimitives();
		bool CullModeTest(float v0Cross, float v1Cross, float v2Cross);
	};
}

#endif