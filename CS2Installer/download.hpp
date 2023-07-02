#pragma once
#include <conio.h>
#include <Windows.h>
/* very bad spot for this but who cares ehh */
inline void waitforinput() {
	int result = _getch();
	if (!result) {
		exit(GetLastError());
	}
}


namespace Downloader {
	bool needsUpdate();
	void UpdateInstaller();

	void PrepareDownload();
	void DownloadCS2();
	void DownloadMods();
}