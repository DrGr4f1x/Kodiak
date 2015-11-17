// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "RenderSceneOp.h"

#include "CommandList.h"
#include "Scene.h"


using namespace Kodiak;
using namespace std;


RenderSceneOperation::RenderSceneOperation(shared_ptr<Scene> scene) : m_scene(scene) {}


void RenderSceneOperation::PopulateCommandList(GraphicsCommandList& commandList)
{
	m_scene->Render(commandList);
}