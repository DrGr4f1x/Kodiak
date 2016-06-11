// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "Defaults.h"

#include "Effect.h"
#include "RenderPass.h"

using namespace Kodiak;
using namespace std;


namespace Kodiak
{

// Default effects
static shared_ptr<Effect> s_defaultBaseEffect;
static shared_ptr<Effect> s_defaultDepthEffect;
static shared_ptr<Effect> s_defaultShadowEffect;

shared_ptr<Effect> GetDefaultBaseEffect() { return s_defaultBaseEffect; }
shared_ptr<Effect> GetDefaultDepthEffect() { return s_defaultDepthEffect; }
shared_ptr<Effect> GetDefaultShadowEffect() { return s_defaultShadowEffect; }

void SetDefaultBaseEffect(shared_ptr<Effect> effect) { s_defaultBaseEffect = effect; }
void SetDefaultDepthEffect(shared_ptr<Effect> effect) { s_defaultDepthEffect = effect; }
void SetDefaultShadowEffect(shared_ptr<Effect> effect) { s_defaultShadowEffect = effect; }


// Default render passes
static shared_ptr<RenderPass> s_defaultBasePass;
static shared_ptr<RenderPass> s_defaultDepthPass;
static shared_ptr<RenderPass> s_defaultShadowPass;

shared_ptr<RenderPass> GetDefaultBasePass() { return s_defaultBasePass; }
shared_ptr<RenderPass> GetDefaultDepthPass() { return s_defaultDepthPass; }
shared_ptr<RenderPass> GetDefaultShadowPass() { return s_defaultShadowPass; }

void SetDefaultBasePass(shared_ptr<RenderPass> pass) { s_defaultBasePass = pass; }
void SetDefaultDepthPass(shared_ptr<RenderPass> pass) { s_defaultDepthPass = pass; }
void SetDefaultShadowPass(shared_ptr<RenderPass> pass) { s_defaultShadowPass = pass; }

} // namespace Kodiak