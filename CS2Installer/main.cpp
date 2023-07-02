#include <string>
#include <thread>
#include <chrono>
#include <filesystem>

#include "globals.hpp"
#include "download.hpp"
#include "patcher.hpp"

int main(int argc, char* argv[]) {
	if (Downloader::needsUpdate()) {
		puts("update required, please press any key to download the update.");
		waitforinput();
		Downloader::UpdateInstaller();
		//return 1; /* doeesnt reach */
	}
	if (argc > 0) {
		for (int i = 1; i < argc; ++i) {
			if (strcmp(argv[i], "disablemanifest") == 0) {
				Globals::usesNoManifests = true;
				break;
			}
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
	if (tolower(_getch()) == 'y') {
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
	waitforinput();
	return 0;
}