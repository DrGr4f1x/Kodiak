// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

#include "IRenderOperation.h"

namespace Kodiak
{

// Forward declarations
class Scene;

class UpdateSceneOperation : public IRenderOperation
{
public:
	UpdateSceneOperation(std::shared_ptr<Scene> scene);

	void PopulateCommandList(GraphicsCommandList& commandList) override;

private:
	const std::shared_ptr<Scene>		m_scene;
};

} // namespace Kodiak