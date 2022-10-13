#include "pch.h"
#include "Effect.h"

Effect::Effect(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, const std::wstring& assetfile)
	: BaseEffect{ pDevice, assetfile }
{
	m_pPointTechnique = m_pEffect->GetTechniqueByName("PointTechnique");
	m_pLinearTechnique = m_pEffect->GetTechniqueByName("LinearTechnique");
	m_pAnisotropicTechnique = m_pEffect->GetTechniqueByName("AnisotropicTechnique");
	if (!m_pPointTechnique->IsValid() || !m_pLinearTechnique->IsValid() || !m_pAnisotropicTechnique->IsValid())
	{
		std::wcout << L"Technique not valid\n";
	}
	m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
	if (!m_pDiffuseMapVariable->IsValid())
	{
		std::wcout << L"Shader resource diffuse map not valid\n";
	}
	m_pNormalMapVariable = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();
	if (!m_pNormalMapVariable->IsValid())
	{
		std::wcout << L"Shader resource normal map not valid\n";
	}
	m_pSpecMapVariable = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();
	if (!m_pSpecMapVariable->IsValid())
	{
		std::wcout << L"Shader resource specular map not valid\n";
	}
	m_pGlossMapVariable = m_pEffect->GetVariableByName("gGlossMap")->AsShaderResource();
	if (!m_pGlossMapVariable->IsValid())
	{
		std::wcout << L"Shader resource glossiness map not valid\n";
	}
	m_pLightDirection = m_pEffect->GetVariableByName("gLightDirection")->AsVector();
	if (!m_pLightDirection->IsValid())
	{
		std::wcout << L"Light vector not valid\n";
	}
	m_pPhongSettings = m_pEffect->GetVariableByName("gPhongSettings")->AsVector();
	if (!m_pPhongSettings->IsValid())
	{
		std::wcout << L"Light vector not valid\n";
	}
	m_pWorldMatrixVariable = m_pEffect->GetVariableByName("gWorldMatrix")->AsMatrix();
	if (!m_pWorldMatrixVariable->IsValid())
	{
		std::wcout << L"World matrix not valid\n";
	}
	m_pViewInverseMatrixVariable = m_pEffect->GetVariableByName("gONB")->AsMatrix();
	if (!m_pViewInverseMatrixVariable->IsValid())
	{
		std::wcout << L"View inverse matrix not valid\n";
	}

	m_RastDesc.FillMode = D3D11_FILL_SOLID;
	m_RastDesc.CullMode = D3D11_CULL_MODE(m_CullMode);
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

Effect::~Effect()
{
	if (m_pWorldMatrixVariable)
	{
		m_pWorldMatrixVariable->Release();
		m_pWorldMatrixVariable = nullptr;
	}
	if (m_pViewInverseMatrixVariable)
	{
		m_pViewInverseMatrixVariable->Release();
		m_pViewInverseMatrixVariable = nullptr;
	}
	if (m_pDiffuseMapVariable)
	{
		m_pDiffuseMapVariable->Release();
		m_pDiffuseMapVariable = nullptr;
	}
	if (m_pNormalMapVariable)
	{
		m_pNormalMapVariable->Release();
		m_pNormalMapVariable = nullptr;
	}
	if (m_pSpecMapVariable)
	{
		m_pSpecMapVariable->Release();
		m_pSpecMapVariable = nullptr;
	}
	if (m_pGlossMapVariable)
	{
		m_pGlossMapVariable->Release();
		m_pGlossMapVariable = nullptr;
	}
	if (m_pLightDirection)
	{
		m_pLightDirection->Release();
		m_pLightDirection = nullptr;
	}
	if(m_pPhongSettings)
	{
		m_pPhongSettings->Release();
		m_pPhongSettings = nullptr;
	}
}

ID3DX11EffectTechnique* Effect::GetTechnique()
{
	switch (m_CurrTechnique)
	{
	case 0:
		return m_pPointTechnique;
		break;
	case 1:
		return m_pLinearTechnique;
		break;
	case 2:
		return m_pAnisotropicTechnique;
		break;
	default:
		return m_pPointTechnique;
	}
}

void Effect::ChangeTechnique()
{
	std::cout << "DirectX: ";
	m_CurrTechnique = (m_CurrTechnique + 1) % 3;
	switch (m_CurrTechnique)
	{
	case 0:
		std::cout << "Point Sample\n";
		break;
	case 1:
		std::cout << "Linear\n";
		break;
	case 2:
		std::cout << "Anisotropic\n";
		break;
	default:
		std::cout << "Point Sample\n";
	}
}

void Effect::ChangeRasterizerState(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, D3D11_CULL_MODE cullMode)
{

	if (m_pRasterizerState)
	{
		m_pRasterizerState->Release();
		m_pRasterizerState = nullptr;
	}

	m_RastDesc.FillMode = D3D11_FILL_SOLID;
	m_RastDesc.CullMode = D3D11_CULL_MODE(cullMode);
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

void Effect::SetLightDirection(Elite::FVector3 lightDirection)
{
	m_pLightDirection->SetFloatVector(reinterpret_cast<float*>(&lightDirection));
}

void Effect::SetPhongSettings(Elite::FVector3 phongSettings)
{
	m_pPhongSettings->SetFloatVector(reinterpret_cast<float*>(&phongSettings));
}

void Effect::SetWorldMatrix(Elite::FMatrix4 worldMatrix)
{
	m_pWorldMatrixVariable->SetMatrix(reinterpret_cast<float*>(&worldMatrix));
}

void Effect::SetViewInverseMatrix(Elite::FMatrix4 viewInverseMatrix)
{
	m_pViewInverseMatrixVariable->SetMatrix(reinterpret_cast<float*>(&viewInverseMatrix));
}

void Effect::SetDiffuseMap(ID3D11ShaderResourceView* pShaderResourceView)
{
	if (m_pDiffuseMapVariable->IsValid())
	{
		m_pDiffuseMapVariable->SetResource(pShaderResourceView);
	}
}

void Effect::SetNormalMap(ID3D11ShaderResourceView* pShaderResourceView)
{
	if (m_pNormalMapVariable->IsValid())
	{
		m_pNormalMapVariable->SetResource(pShaderResourceView);
	}
}

void Effect::SetSpecularMap(ID3D11ShaderResourceView* pShaderResourceView)
{
	if (m_pSpecMapVariable->IsValid())
	{
		m_pSpecMapVariable->SetResource(pShaderResourceView);
	}
}

void Effect::SetGlossMap(ID3D11ShaderResourceView* pShaderResourceView)
{
	if (m_pGlossMapVariable->IsValid())
	{
		m_pGlossMapVariable->SetResource(pShaderResourceView);
	}
}