#include <Windows.h>
#include <iostream>
#include <filesystem>
#include <thread>
#include <chrono>

#include "globals.hpp"

#include "download.hpp"
#include "patcher.hpp"

int main(int argc, char* argv[]) {
	std::string wantsMovementPatch;

	if (Downloader::needsUpdate()) {
		puts("update required, please press enter to download the update.");
		waitforinput();
		Downloader::UpdateInstaller();
		//return 1; /* doeesnt reach */
	}

	for (int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "disablemanifest") == 0) {
			Globals::usesNoManifests = true;
			break;
		}
	}
	puts("Preparing Download...");
	Downloader::PrepareDownload();
	puts("Prepared Download!");
	Patcher::CleanPatchFiles();
	puts("Starting Download...");
	Downloader::DownloadCS2();
	puts("Finished Download!.");
	puts("Starting Patches...");
	Patcher::PatchClient();
	puts("Do you want to install the Movement Patch?(This is recommended for bhop/surf servers for better movement)");
	puts("Press 'Y' for 'Yes' or 'N' for 'No' on your keyboard");
	std::cin >> wantsMovementPatch;

	for (char& c : wantsMovementPatch) { /* make the anwser lowercase */
		c = std::tolower(c);
	}

	if (wantsMovementPatch.find("y") != std::string::npos) {
		Patcher::PatchServer();
	}

	puts("Finished Patches!");
	puts("Starting Client-Mod Patches...");
	puts("This can take a long time...");
	Downloader::DownloadMods();
	puts("Finished Client-Mod Patches!");
	puts("Cleaning up downloader files...");
	if (!Globals::usesNoManifests) {
		std::filesystem::remove_all("manifestFiles");
	}
	puts("Cleaned up!");
	puts("Download complete!");
	puts("Press 'Enter' to close the installer!");
	waitforinput();
	return 0;
}
