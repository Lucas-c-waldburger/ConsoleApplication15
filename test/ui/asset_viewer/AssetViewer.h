#pragma once
#include "AssetPreviewViewerChild.h"
#include "DirectoryViewerChild.h"

#if IMGUI_ENABLED

namespace ui {

class AssetViewer
{
public:
	bool Draw(SceneFixture& fixture);

	Result<Void> Init(SceneFixture& fixture);

	void TearDown();

private:
	struct DirectoryViewerChildInfo
	{
		static constexpr float kCollapsedWidth = 45.0f;
		static constexpr float kInitialExpandedWidth = 180.0f;

		float expandWidth = kInitialExpandedWidth;
		bool expanded = false;
	};

	AssetViewerIcons icons_;
	DirectoryViewerChild directoryViewer_;
	AssetGridViewerChild assetGridViewer_;
	AssetPreviewViewerChild assetPreviewViewer_;
	DirectoryViewerChildInfo dirChildInfo_;
};


} // ui

#endif