#include <Windows.h>
#include <iostream>
#include <filesystem>
#include <conio.h>
#include <thread>
#include <chrono>

#include "globals.hpp"

#include "download.hpp"
#include "patcher.hpp"

int main(int argc, char* argv[]) {
	if (Downloader::needsUpdate()) {
		printf("update required, please press enter to download the update.\n");
		_getch();
		Downloader::UpdateInstaller();
		_getch();
		return 0;
	}

	for (int i = 1; i < argc; ++i) {
		if (std::strcmp(argv[i], "disablemanifest") == 0) {
			Globals::usesNoManifests = true;
			break;
		}
	}

	printf("preparing download...\n");
	Downloader::PrepareDownload();
	printf("prepared download.\n");
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	Patcher::CleanPatchFiles();
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	printf("starting download...\n");
	Downloader::DownloadCS2();
	printf("finished download.\n");
	std::this_thread::sleep_for(std::chrono::milliseconds(1500));
	printf("starting patches...\n");
	Patcher::PatchClient();
	Patcher::PatchServer();
	printf("finished patches.\n");
	std::this_thread::sleep_for(std::chrono::milliseconds(1500));
	printf("starting mod patches(this can take a long time)...\n");
	Downloader::DownloadMods();
	printf("finished mod patches.\n");
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	printf("cleaning up...\n");
	if (!Globals::usesNoManifests) {
		std::filesystem::remove_all("manifestFiles");
	}
	printf("cleaned up.\n");
	_getch();
	return 0;
}