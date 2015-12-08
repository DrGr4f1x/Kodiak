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
class Material;
struct MaterialDesc;


class MaterialManager
{
public:
	static MaterialManager& GetInstance();
	static void Destroy();

	std::shared_ptr<Material> CreateMaterial(const MaterialDesc& desc, bool asyncCreate = true);

private:
	void CreateMaterialAsync(std::shared_ptr<Material> material, const MaterialDesc& desc);
	void CreateMaterialSerial(std::shared_ptr<Material> material, const MaterialDesc& desc);
};


} // namespace Kodiak