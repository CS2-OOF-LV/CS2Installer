#include <Windows.h>
#include <iostream>
#include <filesystem>

#include "globals.hpp"

#include "download.hpp"
#include "patcher.hpp"

#pragma comment(lib, "Urlmon.lib")

int main() {
	std::string currentPath = std::filesystem::current_path().string();
	for (size_t i = 2; i < currentPath.length(); i++) {
		if (i > 1) {
			currentPath.erase(i, 1);
			i--;
		}
	}
	Globals::currentPath = currentPath + " ; " + std::filesystem::current_path().string();
	printf("preparing download...\n");
	Downloader::PrepareDownload();
	printf("prepared download.\n");
	Sleep(1000);
	Patcher::CleanPatchFiles();
	Sleep(1000);
	printf("starting download...\n");
	Downloader::DownloadCS2();
	printf("finished download.\n");
	Sleep(1500);
	printf("starting patches...\n");
	Patcher::PatchClient();
	Patcher::PatchServer();
	Patcher::PatchVScript();
	printf("finished patches.\n");
	Sleep(1500);
	printf("starting mod patches...\n");
	Downloader::DownloadMods();
	printf("finished mod patches.\n");
	Sleep(1000);
	printf("cleaning up...\n");
	std::filesystem::remove_all("manifestFiles");
	printf("cleaned up.\n");
	system("pause");
	return 0;
}