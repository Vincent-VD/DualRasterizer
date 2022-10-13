#pragma once

#include "BaseEffect.h"

class Effect final : public BaseEffect 
{
public:
	Effect(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, const std::wstring& assetfile);
	~Effect();

	Effect(const Effect& other) = delete;
	Effect& operator=(const Effect& other) = delete;
	Effect(Effect&& other) = delete;
	Effect& operator=(Effect&& other) = delete;

	//ID3DX11Effect* GetEffect();
	ID3DX11EffectTechnique* GetTechnique();

	void ChangeTechnique();
	void ChangeRasterizerState(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, D3D11_CULL_MODE cullMode);

	void SetLightDirection(Elite::FVector3 lightDirection);
	void SetPhongSettings(Elite::FVector3 phongSettings);
	void SetWorldMatrix(Elite::FMatrix4 worldMatrix);
	void SetViewInverseMatrix(Elite::FMatrix4 viewInverseMatrix);
	void SetDiffuseMap(ID3D11ShaderResourceView* pShaderResourceView);
	void SetNormalMap(ID3D11ShaderResourceView* pShaderResourceView);
	void SetSpecularMap(ID3D11ShaderResourceView* pShaderResourceView);
	void SetGlossMap(ID3D11ShaderResourceView* pShaderResourceView);

private:
	int m_CullMode{ 3 };
	//int m_CurrTechnique{};
	ID3DX11EffectVectorVariable* m_pLightDirection;
	ID3DX11EffectVectorVariable* m_pPhongSettings;
	ID3DX11EffectMatrixVariable* m_pWorldMatrixVariable;
	ID3DX11EffectMatrixVariable* m_pViewInverseMatrixVariable;
	ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable;
	ID3DX11EffectShaderResourceVariable* m_pNormalMapVariable;
	ID3DX11EffectShaderResourceVariable* m_pSpecMapVariable;
	ID3DX11EffectShaderResourceVariable* m_pGlossMapVariable;
};

