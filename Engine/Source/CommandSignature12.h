// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from CommandSignature.h in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

#pragma once

namespace Kodiak
{

class RootSignature;

class IndirectParameter
{
	friend class CommandSignature;
public:

	IndirectParameter()
	{
		m_indirectParam.Type = (D3D12_INDIRECT_ARGUMENT_TYPE)0xFFFFFFFF;
	}

	void Draw()
	{
		m_indirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;
	}

	void DrawIndexed()
	{
		m_indirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
	}

	void Dispatch()
	{
		m_indirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;
	}

	void VertexBufferView(UINT Slot)
	{
		m_indirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_VERTEX_BUFFER_VIEW;
		m_indirectParam.VertexBuffer.Slot = Slot;
	}

	void IndexBufferView()
	{
		m_indirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_INDEX_BUFFER_VIEW;
	}

	void Constant(UINT rootParameterIndex, UINT destOffsetIn32BitValues, UINT num32BitValuesToSet)
	{
		m_indirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
		m_indirectParam.Constant.RootParameterIndex = rootParameterIndex;
		m_indirectParam.Constant.DestOffsetIn32BitValues = destOffsetIn32BitValues;
		m_indirectParam.Constant.Num32BitValuesToSet = num32BitValuesToSet;
	}

	void ConstantBufferView(UINT rootParameterIndex)
	{
		m_indirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW;
		m_indirectParam.ConstantBufferView.RootParameterIndex = rootParameterIndex;
	}

	void ShaderResourceView(UINT rootParameterIndex)
	{
		m_indirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_SHADER_RESOURCE_VIEW;
		m_indirectParam.ShaderResourceView.RootParameterIndex = rootParameterIndex;
	}

	void UnorderedAccessView(UINT rootParameterIndex)
	{
		m_indirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_UNORDERED_ACCESS_VIEW;
		m_indirectParam.UnorderedAccessView.RootParameterIndex = rootParameterIndex;
	}

	D3D12_INDIRECT_ARGUMENT_TYPE GetType(void) const { return m_indirectParam.Type; }

protected:
	D3D12_INDIRECT_ARGUMENT_DESC m_indirectParam;
};


class CommandSignature
{
public:

	CommandSignature(UINT numParams = 0) : m_finalized(FALSE), m_numParameters(numParams)
	{
		Reset(numParams);
	}

	void Destroy()
	{
		m_signature = nullptr;
		m_paramArray = nullptr;
	}

	void Reset(UINT numParams)
	{
		if (numParams > 0)
		{
			m_paramArray.reset(new IndirectParameter[numParams]);
		}
		else
		{
			m_paramArray = nullptr;
		}

		m_numParameters = numParams;
	}

	IndirectParameter& operator[] (size_t entryIndex)
	{
		assert(entryIndex < m_numParameters);
		return m_paramArray.get()[entryIndex];
	}

	const IndirectParameter& operator[] (size_t entryIndex) const
	{
		assert(entryIndex < m_numParameters);
		return m_paramArray.get()[entryIndex];
	}

	void Finalize(const RootSignature* rootSignature = nullptr);

	ID3D12CommandSignature* GetSignature() const { return m_signature.Get(); }

protected:

	BOOL m_finalized;
	UINT m_numParameters;
	std::unique_ptr<IndirectParameter[]> m_paramArray;
	Microsoft::WRL::ComPtr<ID3D12CommandSignature> m_signature;
};


extern CommandSignature DispatchIndirectCommandSignature;
extern CommandSignature DrawIndirectCommandSignature;


} // namespace Kodiak