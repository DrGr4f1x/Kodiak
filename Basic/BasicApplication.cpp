#include "pch.h"

#include "BasicApplication.h"

#include "Engine\Source\DeviceResources.h"
#include "Engine\Source\Effect.h"

BasicApplication::BasicApplication(const std::shared_ptr<Kodiak::DeviceResources>& deviceResources)
    : Kodiak::Application(deviceResources)
{}


BasicApplication::~BasicApplication()
{}


void BasicApplication::Initialize()
{
    Application::Initialize();

    InitializeScene();
    InitializeRenderer();
}


void BasicApplication::Update()
{}


void BasicApplication::InitializeScene()
{
    auto effect = std::make_shared<Kodiak::Effect>();
    effect->SetVertexShaderPath(L"Shaders\\BasicVS.hlsl");
    effect->SetPixelShaderPath(L"Shaders\\BasicPS.hlsl");
}


void BasicApplication::InitializeRenderer()
{}