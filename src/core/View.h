#pragma once

#include <rendering/RenderContext.h>

namespace vmc
{
	class View
	{
	public:
		View(Application& application);

		View(const View&) = delete;

		View(View&& other) = delete;

		virtual ~View() = default;

		View& operator=(const View&) = delete;

		View& operator=(View&&) = delete;

		virtual void update(float timeDelta) = 0;

		virtual void render(RenderContext& renderContext) = 0;

	protected:
		Application& application;
	};
}