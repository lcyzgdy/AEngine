#include "Engine.h"
#include "RenderCore.h"
#include "Screen.h"
#include "Input.h"
#include "ThreadPool.hpp"
#include "DTimer.h"
#include "SceneManager.h"
#include "IParallel.h"
#include "ISerial.h"
#include "GpuContext.h"
#include "GraphicsCard.h"

using namespace std;
using namespace AnEngine::Game;

namespace AnEngine
{
	void Engine::UpdateBottom()
	{
		BaseInput::Instance()->ZeroInputState();
		DTimer::Instance()->Tick(nullptr);
		BaseInput::Instance()->Update();
	}

	void Engine::UpdateSystem()
	{
		var scene = SceneManager::ActiveScene();

		// 执行 Behaviour 的 BeforeUpdate，这是在 System 更新之前调用的
		for (var obj : scene->GetAllGameObjects())
		{
			for (var b : obj->GetAllBehaviours())
			{
				b->BeforeUpdate();
			}
		}

		for (var sys : scene->GetAllSystems())
		{
			if (is_base_of<IParallel, decltype(*sys)>::value)
			{
				var parallel = (IParallel*)sys;
				condition_variable cv;
				unique_lock<mutex> lock(m_parallelMutex);
				atomic_int32_t count = 0;
				for (int i = 0; i < parallel->Length; i++)
				{
					Utility::ThreadPool::Commit(bind(&IParallel::Execute, parallel, i), [&]() { count++; });
				}
				cv.wait(lock, [&]()->bool { return parallel->Length == count; });
			}
			else if (is_base_of<ISerial, decltype(*sys)>::value)
			{
				var serial = (ISerial*)sys;
				for (int i = 0; i < serial->Length; i++)
				{
					serial->Execute(i);
				}
			}
		}
	}

	void Engine::UpdateBehaviour()
	{
		var scene = Game::SceneManager::ActiveScene();
		for (var obj : scene->GetAllGameObjects())
		{
			for (var b : obj->GetAllBehaviours())
			{
				b->OnUpdate();
			}
		}
		for (var obj : scene->GetAllGameObjects())
		{
			for (var b : obj->GetAllBehaviours())
			{
				b->AfterUpdate();
			}
		}
	}

	void Engine::Initialize(const Win32App& win32App)
	{
		if (m_initialized) return;
		m_initialized = true;
		BaseInput::Instance()->Initialize(win32App.m_hwnd, win32App.m_hInstance);
		Screen::Instance()->Initialize(win32App.m_windowWidth, win32App.m_windowHeight);
	}

	void Engine::Release()
	{
		lock_guard<mutex> lock(m_sceneResMutex);
		m_initialized = false;
		m_running = false;
		SceneManager::ActiveScene()->onUnload();
		BaseInput::Instance()->Release();
	}

	const RenderCore::GraphicsCard& Engine::InitializeRender(IDXGIFactory6* dxgiFactory)
	{
		RenderCore::GpuContext::Instance()->Initialize(dxgiFactory);
		return RenderCore::GpuContext::Instance()->Default();
	}

	void Engine::AttachSwapChain(const Microsoft::WRL::ComPtr<IDXGISwapChain4>& swapChain, uint32_t bufferCount)
	{
		RenderCore::GpuContext::Instance()->AttachSwapChain(swapChain, bufferCount);
	}

	void Engine::StartScene()
	{
		m_running = true;
		var scene = SceneManager::ActiveScene();
		scene->onLoad();
		Utility::ThreadPool::Commit([this]()->void
			{
				double deltaTime = 0;
				double totalTime = 0;
				while (this->m_running)
				{
					deltaTime = DTimer::Instance()->GetTotalSeconds() - totalTime;
					if (deltaTime > 1.0 / 60.0)
					{
						// 限制最高帧率为60FPS，未来这里应该修改为可配置。
						totalTime = DTimer::Instance()->GetTotalSeconds();
						// 使用大锁的情况下不需要放另一个线程进行等待。
						//Utility::ThreadPool::Commit([this]()->void
						//{
						/* 使用配置的渲染管线进行渲染。渲染时会锁定游戏逻辑。
						 * 这里使用大锁，假如CPU时间远小于GPU时间，则两帧更新之间游戏逻辑会执行多次。
						 * 使用小锁会带来逻辑错误。
						 * 下一步这里应修改为同步、异步两种模式。
						 */
						 //lock_guard<mutex> lock(this->m_sceneResMutex);
						this->m_rp->OnRender(this->m_sceneResMutex);
						//});
					}
					//lock_guard<mutex> lock(m_sceneResMutex);
					this->UpdateBottom();
					this->UpdateSystem();
					this->UpdateBehaviour();
				}
			});
	}
}
