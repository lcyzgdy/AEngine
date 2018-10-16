#include "DeferredRenderPipeline.h"
#include "SceneManager.h"
#include "Renderer.h"
using namespace std;
using namespace AnEngine::Game;

namespace AnEngine::RenderCore
{
	void DeferredRenderPipeline::OnRender()
	{
		GBuffer();
		DepthPreLight();
	}

	void DeferredRenderPipeline::GBuffer()
	{
		var scene = Game::SceneManager::ActiveScene();
	}

	void DeferredRenderPipeline::DepthPreLight()
	{
	}
}