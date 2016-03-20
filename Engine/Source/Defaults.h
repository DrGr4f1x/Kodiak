// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

namespace Kodiak
{

// Forward declarations
class Effect;
class RenderPass;


// Default effects
std::shared_ptr<Effect> GetDefaultBaseEffect();
std::shared_ptr<Effect> GetDefaultDepthEffect();

void SetDefaultBaseEffect(std::shared_ptr<Effect> effect);
void SetDefaultDepthEffect(std::shared_ptr<Effect> effect);


// Default render passes
std::shared_ptr<RenderPass> GetDefaultBasePass();
std::shared_ptr<RenderPass> GetDefaultDepthPass();

void SetDefaultBasePass(std::shared_ptr<RenderPass> pass);
void SetDefaultDepthPass(std::shared_ptr<RenderPass> pass);

} // namespace Kodiak