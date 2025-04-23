#pragma once

constexpr unsigned int MAX_POINT_LIGHTS = 25;
constexpr unsigned int MAX_SPOT_LIGHTS = 25;

struct UniformDirectionalLight
{
	unsigned int Color{};
	unsigned int AmbientIntensity{};
	unsigned int DiffuseIntensity{};

	unsigned int Direction{};
};

struct UniformPointLight
{
	unsigned int Color{};
	unsigned int AmbientIntensity{};
	unsigned int DiffuseIntensity{};

	unsigned int Position{};
	unsigned int Constant{};
	unsigned int Linear{};
	unsigned int Exponent{};
};

struct UniformSpotLight
{
	unsigned int Color{};
	unsigned int AmbientIntensity{};
	unsigned int DiffuseIntensity{};

	unsigned int Position{};
	unsigned int Constant{};
	unsigned int Linear{};
	unsigned int Exponent{};

	unsigned int Direction{};
	unsigned int Edge{};
};
