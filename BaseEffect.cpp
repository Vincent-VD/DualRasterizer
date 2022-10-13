#include "pch.h"
#include "BaseEffect.h"

BaseEffect::BaseEffect(ID3D11Device* pDevice, const std::wstring& assetfile)
{
	m_pEffect = LoadEffect(pDevice, assetfile);
	if (!m_pEffect->IsValid())
	{
		std::wcout << L"Effect nog valid\n";
	}
	m_pEffectMatrixVariable = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
	if (!m_pEffectMatrixVariable->IsValid())
	{
		std::wcout << L"Matrix not valid\n";
	}
}

BaseEffect::~BaseEffect()
{
	if (m_pEffectMatrixVariable)
	{
		m_pEffectMatrixVariable->Release();
		m_pEffectMatrixVariable = nullptr;
	}
	if (m_pPointTechnique)
	{
		m_pPointTechnique->Release();
		m_pPointTechnique = nullptr;
	}
	if (m_pLinearTechnique)
	{
		m_pLinearTechnique->Release();
		m_pLinearTechnique = nullptr;
	}
	if (m_pAnisotropicTechnique)
	{
		m_pAnisotropicTechnique->Release();
		m_pAnisotropicTechnique = nullptr;
	}
	if (m_pRasterizerState)
	{
		m_pRasterizerState->Release();
		m_pRasterizerState = nullptr;
	}
	if (m_pEffect)
	{
		m_pEffect->Release();
		m_pEffect = nullptr;
	}
}

ID3DX11Effect* BaseEffect::GetEffect()
{
	return m_pEffect;
}

ID3DX11EffectTechnique* BaseEffect::GetTechnique()
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
		break;
	}
}

void BaseEffect::ChangeTechnique()
{
	m_CurrTechnique = (m_CurrTechnique + 1) % 3;
}

void BaseEffect::SetWorldViewProjMatrix(Elite::FMatrix4 worldViewProjMatrix)
{
	m_pEffectMatrixVariable->SetMatrix(reinterpret_cast<float*>(&worldViewProjMatrix));
}

void BaseEffect::SetRasterizerState(ID3D11DeviceContext* pDeviceContext)
{
	pDeviceContext->RSSetState(m_pRasterizerState);
}

ID3DX11Effect* BaseEffect::LoadEffect(ID3D11Device* pDevice, const std::wstring& assetfile)
{
	HRESULT result{ S_OK };
	ID3D10Blob* pErrorBlob{ nullptr };
	ID3DX11Effect* pEffect;

	DWORD shaderFlags{};
#if defined(DEBUG) | defined(_DEBUG)
	{
		shaderFlags |= D3DCOMPILE_DEBUG;
		shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
	}
#endif

	result = D3DX11CompileEffectFromFile(assetfile.c_str(),
										 nullptr,
										 nullptr,
										 shaderFlags,
										 0,
										 pDevice,
										 &pEffect,
										 &pErrorBlob);

	if (FAILED(result))
	{
		if (pErrorBlob != nullptr)
		{
			char* pErrors{ (char*)pErrorBlob->GetBufferPointer() };
			std::wstringstream ss;
			for (unsigned int i = 0; i < pErrorBlob->GetBufferSize(); ++i)
			{
				ss << pErrors[i];
			}

			OutputDebugStringW(ss.str().c_str());
			pErrorBlob->Release();
			pErrorBlob = nullptr;

			std::wcout << ss.str() << std::endl;
		}
		else
		{
			std::wstringstream ss;
			ss << "EffectLoader:  Failed to CreateEffectFromFile!\nPath: " << assetfile;
			std::wcout << ss.str() << std::endl;
			return nullptr;
		}
	}
	return pEffect;
}