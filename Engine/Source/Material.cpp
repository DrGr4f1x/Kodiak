// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "Material.h"

using namespace Kodiak;
using namespace std;


// Make sure these values stay in sync with the shaders!
static const uint32_t	s_perViewConstantsSlot = 0;
static const uint32_t	s_perObjectConstantsSlot = 1;
static const uint32_t	s_perMaterialConstantsSlot = 2;
static const string		s_perViewConstantsName{ "PerViewConstants" };
static const string		s_perObjectConstantsName{ "PerObjectConstants" };
static const string		s_perMaterialConstantsName{ "PerMaterialConstants" };

namespace Kodiak
{

uint32_t GetPerViewConstantsSlot() { return s_perViewConstantsSlot; }
uint32_t GetPerObjectConstantsSlot() { return s_perObjectConstantsSlot; }
uint32_t GetPerMaterialConstantsSlot() { return s_perMaterialConstantsSlot; }
const string& GetPerViewConstantsName() { return s_perViewConstantsName; }
const string& GetPerObjectConstantsName() {	return s_perObjectConstantsName; }
const string& GetPerMaterialConstantsName() { return s_perMaterialConstantsName; }

} // namespace Kodiak