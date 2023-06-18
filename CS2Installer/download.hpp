#pragma once

namespace Downloader {
	bool needsUpdate();
	void UpdateInstaller();

	void PrepareDownload();
	void DownloadCS2();
	void DownloadMods();
}