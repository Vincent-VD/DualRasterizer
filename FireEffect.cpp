#include "pch.h"
#include "FireEffect.h"

FireEffect::FireEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, const std::wstring& assetfile)
	: BaseEffect{ pDevice, assetfile }
{
	m_pPointTechnique = m_pEffect->GetTechniqueByName("FireTechnique");
	if (!m_pPointTechnique->IsValid())
	{
		std::wcout << L"Technique not valid\n";
	}
	m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
	if (!m_pDiffuseMapVariable->IsValid())
	{
		std::wcout << L"Shader resource diffuse map not valid\n";
	}

	m_RastDesc.FillMode = D3D11_FILL_SOLID;
	m_RastDesc.CullMode = D3D11_CULL_NONE;
	m_RastDesc.FrontCounterClockwise = true;
	m_RastDesc.AntialiasedLineEnable = false;
	m_RastDesc.DepthBias = 0;
	m_RastDesc.DepthBiasClamp = 0;
	m_RastDesc.SlopeScaledDepthBias = 0;
	m_RastDesc.DepthClipEnable = true;
	m_RastDesc.ScissorEnable = false;
	m_RastDesc.MultisampleEnable = false;
	pDevice->CreateRasterizerState(&m_RastDesc, &m_pRasterizerState);
	//pDeviceContext->RSSetState(m_pRasterizerState);
}

FireEffect::~FireEffect()
{
	if (m_pDiffuseMapVariable)
	{
		m_pDiffuseMapVariable->Release();
		m_pDiffuseMapVariable = nullptr;
	}
}

ID3DX11EffectTechnique* FireEffect::GetTechnique()
{
	return m_pPointTechnique;
}

void FireEffect::SetDiffuseMap(ID3D11ShaderResourceView* pShaderResourceView)
{
	if (m_pDiffuseMapVariable->IsValid())
	{
		m_pDiffuseMapVariable->SetResource(pShaderResourceView);
	}
}