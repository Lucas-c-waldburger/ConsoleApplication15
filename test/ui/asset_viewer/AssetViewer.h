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

private:
	AssetViewerIcons icons_;
	DirectoryViewerChild directoryViewer_;
	AssetGridViewerChild assetGridViewer_;
	AssetPreviewViewerChild assetPreviewViewer_;
};


} // ui

#endif