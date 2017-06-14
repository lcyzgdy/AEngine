#include"RenderCore.h"

// �����Ƿ���HDR�������
#define CONDITIONALLY_ENABLE_HDR_OUTPUT 1


namespace RenderCore
{
	void GraphicCard::CreateDevice()
	{
		UINT dxgiFactoryFlags = 0;

		// ����Debugģʽ
#if defined(DEBUG) || defined(_DEBUG)
		ComPtr<ID3D12Debug> d3dDebugController;
		if (D3D12GetDebugInterface(IID_PPV_ARGS(&d3dDebugController)))
		{
			d3dDebugController->EnableDebugLayer();
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
#endif

		//ComPtr<IDXGIFactory4> r_dxgiFactory;
		CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(Private::r_dxgiFactory.GetAddressOf()));

		ComPtr<IDXGIAdapter1> hardwareAdapter;
		GetHardwareAdapter(Private::r_dxgiFactory.Get(), &hardwareAdapter);
		D3D12CreateDevice(hardwareAdapter.Get(), RenderCore::MinD3DFeatureLevel, IID_PPV_ARGS(&m_device));

		if (m_device.Get() == nullptr)
		{
			ComPtr<IDXGIAdapter> warpAdapter;
			Private::r_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));
			D3D12CreateDevice(warpAdapter.Get(), RenderCore::MinD3DFeatureLevel, IID_PPV_ARGS(&m_device));
		}

		m_device->SetStablePowerState(stableFlag);

#if defined(DEBUG) || defined(_DEBUG)
		ID3D12InfoQueue* compInfoQueue;
		m_device->QueryInterface(IID_PPV_ARGS(&compInfoQueue));
		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };

		// ͨ��ID�����������Ϣ��
		D3D12_MESSAGE_ID denyMessageIds[] =
		{
			// ��������������δ��ʼ����������ʱ����ʹ��ɫ�������ʣ�Ҳ�ᷢ������������������������л���ɫ�������ж����Ǹı�̫��Ĵ��롢���°�����Դ��
			D3D12_MESSAGE_ID_INVALID_DESCRIPTOR_HANDLE,
			// ����ɫ���������ȾĿ���������ɫ�������������RGBд��R10G10B10A2������ʱ������Alphaʱ������
			D3D12_MESSAGE_ID_CREATEGRAPHICSPIPELINESTATE_PS_OUTPUT_RT_OUTPUT_MISMATCH,
			// ��ʹ����ɫ��������ȱ�ٵ���������Ҳ������������δ��ʱ��������ͬ����ɫ��֮�乲��ĸ�ǩ����������Ҫ��ͬ���͵���Դ��
			D3D12_MESSAGE_ID_COMMAND_LIST_DESCRIPTOR_TABLE_NOT_SET,

			(D3D12_MESSAGE_ID)1008,
		};
		D3D12_INFO_QUEUE_FILTER newFilter = {};
		newFilter.DenyList.NumSeverities = _countof(severities);
		newFilter.DenyList.pSeverityList = severities;
		newFilter.DenyList.NumIDs = _countof(denyMessageIds);
		newFilter.DenyList.pIDList = denyMessageIds;

		compInfoQueue->PushStorageFilter(&newFilter);
		compInfoQueue->Release();
#endif
		if (SUCCEEDED(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &m_featureDataOptions, sizeof(m_featureDataOptions))))
		{
			if (m_featureDataOptions.TypedUAVLoadAdditionalFormats)
			{
				D3D12_FEATURE_DATA_FORMAT_SUPPORT formatSupport =
				{
					DXGI_FORMAT_R11G11B10_FLOAT,
					D3D12_FORMAT_SUPPORT1_NONE,
					D3D12_FORMAT_SUPPORT2_NONE
				};
				if (SUCCEEDED(m_device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(formatSupport))) && (formatSupport.Support2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_LOAD) != 0)
				{
					m_isTypedUAVLoadSupport_R11G11B10_FLOAT = true;
				}
				formatSupport.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
				if (SUCCEEDED(m_device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(formatSupport))) && (formatSupport.Support2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_LOAD) != 0)
				{
					m_isTypedUAVLoadSupport_R16G16B16A16_FLOAT = true;
				}
			}
		}
	}

	void GraphicCard::CreateCommandQueue(D3D12_COMMAND_LIST_TYPE type)
	{
		switch (type)
		{
		case D3D12_COMMAND_LIST_TYPE_DIRECT:
		{
			m_renderCommandQueue.Initialize(m_device.Get());
			break;
		}
		case D3D12_COMMAND_LIST_TYPE_BUNDLE:
		{
			break;
		}
		case D3D12_COMMAND_LIST_TYPE_COMPUTE:
		{
			//D3D12_COMMAND_QUEUE_DESC queueDesc = {};
			//queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
			//queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
			//m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_computeCommandQueue.GetAddressOf()));
			m_computeCommandQueue.Initialize(m_device.Get(), D3D12_COMMAND_LIST_TYPE_COMPUTE);
			break;
		}
		case D3D12_COMMAND_LIST_TYPE_COPY:
		{
			m_copyCommandQueue.Initialize(m_device.Get(), D3D12_COMMAND_LIST_TYPE_COPY);
			break;
		}
		default:
			break;
		}
	}

	const ID3D12Device2* GraphicCard::GetDevice() const
	{
		return m_device.Get();
	}

	void GraphicCard::IsStable(bool isStable)
	{
		stableFlag = isStable;
	}

	const ID3D12CommandQueue* GraphicCard::GetCommandQueue(D3D12_COMMAND_LIST_TYPE type) const
	{
		switch (type)
		{
		case D3D12_COMMAND_LIST_TYPE_DIRECT:
		{
			return m_renderCommandQueue.GetCommandQueue();
			break;
		}
		case D3D12_COMMAND_LIST_TYPE_BUNDLE:
		{
			break;
		}
		case D3D12_COMMAND_LIST_TYPE_COMPUTE:
		{
			return m_computeCommandQueue.GetCommandQueue();
			break;
		}
		case D3D12_COMMAND_LIST_TYPE_COPY:
		{
			return m_copyCommandQueue.GetCommandQueue();
			break;
		}
		default:
			break;
		}
		return nullptr;
	}

	GraphicCard::GraphicCard() :
		stableFlag(false)
	{
	}

	GraphicCard::GraphicCard(const GraphicCard & graphicCard)
	{
	}

	void GraphicCard::Initialize(bool compute, bool copy)
	{
		CreateDevice();
		CreateCommandQueue();
		if (compute) CreateCommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE);
		if (copy) CreateCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);
	}
}

namespace RenderCore
{
	void CommandQueue::Initialize(ID3D12Device2* device, D3D12_COMMAND_LIST_TYPE type)
	{
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
		device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_commandQueue.GetAddressOf()));
	}

	void CommandQueue::Release()
	{
	}

	const ID3D12CommandQueue* CommandQueue::GetCommandQueue() const
	{
		return nullptr;
	}

	D3D12_COMMAND_LIST_TYPE CommandQueue::GetType()
	{
		return D3D12_COMMAND_LIST_TYPE();
	}
}

namespace RenderCore
{
	vector<GraphicCard> r_renderCore;
	ComPtr<IDXGISwapChain1> r_swapChain = nullptr;

	bool r_enableHDROutput = false;

	namespace Private
	{
		ComPtr<IDXGIFactory4> r_dxgiFactory;
	}

	void InitializeRender(int graphicCardCount, bool isStable)
	{
		while (graphicCardCount--)
		{
			GraphicCard aRender;
			aRender.IsStable(isStable);
			aRender.Initialize();
			r_renderCore.push_back(aRender);
		}
	}

	void InitializeSwapChain(int width, int height, HWND hwnd, DXGI_FORMAT dxgiFormat)
	{
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.BufferCount = SwapChainBufferCount;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		swapChainDesc.Width = width;
		swapChainDesc.Height = height;
		swapChainDesc.Format = dxgiFormat;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.Scaling = DXGI_SCALING_NONE;

		ComPtr<IDXGISwapChain1> swapChain1;
		ThrowIfFailed(Private::r_dxgiFactory->CreateSwapChainForHwnd
		(
			const_cast<ID3D12CommandQueue*>(r_renderCore[0].GetCommandQueue()), 
			hwnd, &swapChainDesc, nullptr, nullptr, swapChain1.GetAddressOf()
		));
		swapChain1.As(&r_swapChain);
#if CONDITIONALLY_ENABLE_HDR_OUTPUT && defined(NTDDI_WIN10_RS2) && (NTDDI_VERSION >= NTDDI_WIN10_RS2)
		{
			IDXGISwapChain4* swapChain = static_cast<IDXGISwapChain4*>(r_swapChain.Get());
			ComPtr<IDXGIOutput> output;
			ComPtr<IDXGIOutput6> output6;
			DXGI_OUTPUT_DESC1 outputDesc;
			UINT colorSpaceSupport;

			if (SUCCEEDED(swapChain->GetContainingOutput(&output)) &&
				SUCCEEDED(output.As(&output6)) &&
				SUCCEEDED(output6->GetDesc1(&outputDesc)) &&
				outputDesc.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020 &&
				SUCCEEDED(swapChain->CheckColorSpaceSupport(DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020, &colorSpaceSupport)) &&
				(colorSpaceSupport & DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT) &&
				SUCCEEDED(swapChain->SetColorSpace1(DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020)))
			{
				r_enableHDROutput = true;
			}
		}
#endif
		for (UINT i = 0; i < SwapChainBufferCount; ++i)
		{
			ComPtr<ID3D12Resource> DisplayPlane;
			ThrowIfFailed(r_swapChain->GetBuffer(i, IID_PPV_ARGS(&DisplayPlane)));
			//r_displayPlane[i].CreateFromSwapChain(L"Primary SwapChain Buffer", DisplayPlane.Detach());
		}
	}
}