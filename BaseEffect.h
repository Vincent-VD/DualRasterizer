#pragma once
#include <dxgi.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <d3dx11effect.h>
#include <sstream>

class BaseEffect
{
public:
	BaseEffect(ID3D11Device* pDevice, const std::wstring& assetfile);
	virtual ~BaseEffect();

	BaseEffect(const BaseEffect& other) = delete;
	BaseEffect& operator=(const BaseEffect& other) = delete;
	BaseEffect(BaseEffect&& other) = delete;
	BaseEffect& operator=(BaseEffect&& other) = delete;

	ID3DX11Effect* GetEffect();
	virtual ID3DX11EffectTechnique* GetTechnique();

	void ChangeTechnique();

	void SetWorldViewProjMatrix(Elite::FMatrix4 worldViewProjMatrix);
	void SetRasterizerState(ID3D11DeviceContext* pDeviceContext);

	static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetfile);

protected:
	int m_CurrTechnique{};
	D3D11_RASTERIZER_DESC m_RastDesc{};
	ID3DX11Effect* m_pEffect;
	ID3DX11EffectTechnique* m_pPointTechnique;
	ID3DX11EffectTechnique* m_pLinearTechnique;
	ID3DX11EffectTechnique* m_pAnisotropicTechnique;
	ID3DX11EffectMatrixVariable* m_pEffectMatrixVariable;
	ID3D11RasterizerState* m_pRasterizerState;
};

