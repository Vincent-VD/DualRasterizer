#pragma once
#include "ERGBColor.h"
#include "EMath.h"
#include "SDL_image.h"
#include <algorithm>

class Texture final
{
public:
	Texture(ID3D11Device* pDevice, const char* filepath)
	{
		Initialize(pDevice, filepath);
	}
	~Texture()
	{
		if (m_pSurface)
		{
			SDL_FreeSurface(m_pSurface);
			m_pSurface = nullptr;
		}
		if (m_pTexture)
		{
			m_pTexture->Release();
			m_pTexture = nullptr;
		}
		if (m_pShaderResourceView)
		{
			m_pShaderResourceView->Release();
			m_pShaderResourceView = nullptr;
		}
	}

	Texture(const Texture& other) = delete;
	Texture& operator=(const Texture& other) = delete;
	Texture(Texture&& other) = delete;
	Texture& operator=(Texture&& other) = delete;

	ID3D11ShaderResourceView* GetResourceView() const
	{
		return m_pShaderResourceView;
	}

	void ChangeTechnique()
	{
		std::cout << "Software: ";
		m_CurrTechnique = (m_CurrTechnique + 1) % 2;
		switch (m_CurrTechnique)
		{
		case 0:
			std::cout << "Point sample\n";
			break;
		case 1:
			std::cout << "Linear\n";
			break;
		}
	}

	Elite::RGBColor Sample(const Elite::FVector2& uv)
	{
		switch (m_CurrTechnique)
		{
		case 0:
			return PointSample(uv);
			break;
		case 1:
			return LinearSample(uv);
			break;
		default:
			return PointSample(uv);
		}
	}

	Elite::RGBColor PointSample(const Elite::FVector2& uv)
	{
		Uint8 r, g, b;
		const int width{ m_pSurface->w };
		const int height{ m_pSurface->h };
		const Uint32 newU{ Uint32(uv.x * width) };
		const Uint32 newV{ Uint32(uv.y * height) };
		Uint32 pixel{ newU + (newV * width) };
		pixel = Elite::Clamp(pixel, Uint32(0), Uint32(width * height));
		SDL_GetRGB(static_cast<Uint32*>(m_pSurface->pixels)[pixel], m_pSurface->format, &r, &g, &b);
		Elite::RGBColor res{ float(r) / 255.f, float(g) / 255.f, float(b) / 255.f };
		return res;
	}

	Elite::RGBColor LinearSample(const Elite::FVector2& uv)
	{
		Uint8 r, g, b;
		const int width{ m_pSurface->w };
		const int height{ m_pSurface->h };
		const Uint32 newU{ Uint32(std::min(int(floor(uv.x * width)), width - 1)) };
		const Uint32 newV{ Uint32(std::min(int(floor(uv.y * height)), height - 1)) };
		Uint32 pixel{ newU + (newV * width) };
		pixel = Elite::Clamp(pixel, Uint32(0), Uint32(width * height));
		SDL_GetRGB(static_cast<Uint32*>(m_pSurface->pixels)[pixel], m_pSurface->format, &r, &g, &b);
		Elite::RGBColor res{ float(r) / 255.f, float(g) / 255.f, float(b) / 255.f };
		return res;
	}

	HRESULT Initialize(ID3D11Device* pDevice, const char* filepath)
	{
		m_pSurface = IMG_Load(filepath);
		if (!m_pSurface)
		{
			std::wcout << IMG_GetError() << std::endl;
			return S_FALSE;
		}

		D3D11_TEXTURE2D_DESC desc;
		desc.Width = m_pSurface->w;
		desc.Height = m_pSurface->h;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA initData{};
		initData.pSysMem = m_pSurface->pixels;
		initData.SysMemPitch = static_cast<UINT>(m_pSurface->pitch);
		initData.SysMemSlicePitch = static_cast<UINT>(m_pSurface->h * m_pSurface->pitch);

		HRESULT res = pDevice->CreateTexture2D(&desc, &initData, &m_pTexture);
		if (FAILED(res))
			return res;

		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
		SRVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = 1;

		res = pDevice->CreateShaderResourceView(m_pTexture, &SRVDesc, &m_pShaderResourceView);
		if (FAILED(res))
			return res;

		//SDL_FreeSurface(pSurface);
		//pSurface = nullptr;

		return res;
	}

private:
	int m_CurrTechnique{};
	SDL_Surface* m_pSurface;
	ID3D11Texture2D* m_pTexture;
	ID3D11ShaderResourceView* m_pShaderResourceView;
};
