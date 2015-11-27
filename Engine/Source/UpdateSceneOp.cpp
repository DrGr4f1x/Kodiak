// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "UpdateSceneOp.h"

#include "CommandList.h"
#include "Scene.h"


using namespace Kodiak;
using namespace std;


UpdateSceneOperation::UpdateSceneOperation(shared_ptr<Scene> scene) : m_scene(scene) {}


void UpdateSceneOperation::PopulateCommandList(GraphicsCommandList& commandList)
{
	m_scene->Update(commandList);
}