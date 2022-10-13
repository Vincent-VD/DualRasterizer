#pragma once
#include "BaseEffect.h"
class FireEffect final: public BaseEffect
{
public:
	FireEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, const std::wstring& assetfile);
	~FireEffect();

	FireEffect(const FireEffect& other) = delete;
	FireEffect& operator=(const FireEffect& other) = delete;
	FireEffect(FireEffect&& other) = delete;
	FireEffect& operator=(FireEffect&& other) = delete;

	virtual ID3DX11EffectTechnique* GetTechnique() override;

	void SetDiffuseMap(ID3D11ShaderResourceView* pShaderResourceView);

private:
	ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable;

};

