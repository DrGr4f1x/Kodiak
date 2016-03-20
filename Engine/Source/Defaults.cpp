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

shared_ptr<Effect> GetDefaultBaseEffect() { return s_defaultBaseEffect; }
shared_ptr<Effect> GetDefaultDepthEffect() { return s_defaultDepthEffect; }

void SetDefaultBaseEffect(shared_ptr<Effect> effect) { s_defaultBaseEffect = effect; }
void SetDefaultDepthEffect(shared_ptr<Effect> effect) { s_defaultDepthEffect = effect; }


// Default render passes
static shared_ptr<RenderPass> s_defaultBasePass;
static shared_ptr<RenderPass> s_defaultDepthPass;

shared_ptr<RenderPass> GetDefaultBasePass() { return s_defaultBasePass; }
shared_ptr<RenderPass> GetDefaultDepthPass() { return s_defaultDepthPass; }

void SetDefaultBasePass(shared_ptr<RenderPass> pass) { s_defaultBasePass = pass; }
void SetDefaultDepthPass(shared_ptr<RenderPass> pass) { s_defaultDepthPass = pass; }

} // namespace Kodiak