#include "GameView.h"

namespace vmc
{
	GameView::GameView(Application& application) :
		View(application)
	{
	}

	GameView::~GameView()
	{
	}

	void GameView::update(float timeDelta)
	{
	}

	void GameView::render(RenderContext& renderContext)
	{
		auto commandBuffer = renderContext.startFrame({ 0.8f, 0.9f, 1.0f, 1.0f });
		renderContext.endFrame();
	}
}

