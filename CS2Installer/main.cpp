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
	std::string wantsMovementPatch;

	if (Downloader::needsUpdate()) {
		puts("update required, please press enter to download the update.");
		_getch();
		Downloader::UpdateInstaller();
		_getch();
		return 1;
	}

	for (int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "disablemanifest") == 0) {
			Globals::usesNoManifests = true;
			break;
		}
	}
	puts("preparing download...");
	Downloader::PrepareDownload();
	puts("prepared download.");
	Patcher::CleanPatchFiles();
	puts("starting download...");
	Downloader::DownloadCS2();
	puts("finished download.");
	puts("starting patches...");
	Patcher::PatchClient();
	puts("movement patch(better for surf or bhop servers)(y/n)");
	std::cin >> wantsMovementPatch;
	if (wantsMovementPatch.find("y") != std::string::npos || wantsMovementPatch.find("y") != std::string::npos) {
		Patcher::PatchServer();
	}
	puts("finished patches.");
	puts("starting mod patches(this can take a long time)...");
	Downloader::DownloadMods();
	puts("finished mod patches.");
	puts("cleaning up...");
	if (!Globals::usesNoManifests) {
		std::filesystem::remove_all("manifestFiles");
	}
	puts("cleaned up.");
	_getch();
	return 0;
}