#include "pch.h"
#include "BRDF.h"

RGBColor BRDF::Lambert(const float diffRefl, const RGBColor diffColor) {

	return diffColor * diffRefl / (float)E_PI;
}

RGBColor BRDF::Lambert(const RGBColor diffRefl, const RGBColor diffColor) {

	return diffColor * diffRefl / (float)E_PI;
}

RGBColor BRDF::Phong(const RGBColor specColor, const float exp, const FVector3& w0, const FVector3& w1, const FVector3& normal) {

	FVector3 reflect{ Reflect(-w0, normal) };
	Normalize(reflect);

	float cosAlpha{ Dot(reflect, w1) };
	cosAlpha = Clamp(cosAlpha, 0.f, 1.f);
	RGBColor res{ specColor * powf(cosAlpha, exp) };

	//RGBColor res{ temp, temp, temp };

	return res;
}

RGBColor BRDF::CookTerrance(const float d, const RGBColor f, const float g, const FVector3& l, const FVector3& v, const FVector3 normal) {

	return (f * d * g) / (4 * (Dot(v, normal) * Dot(l, normal)));
}