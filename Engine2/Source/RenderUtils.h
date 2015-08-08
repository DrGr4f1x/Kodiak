#pragma once

namespace Kodiak
{

inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw;
	}
}

} // namespace Kodiak