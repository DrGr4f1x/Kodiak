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

class InputLayout
{
public:
	std::vector<D3D12_INPUT_ELEMENT_DESC>	elements;
	std::vector<std::string>				semantics; // Memory lifetime issue with LPCSTR SemanticName field
	                                                   // on D3D12_INPUT_ELEMENT_DESC.  Remember this from Trinity?
};


} // namespace Kodiak