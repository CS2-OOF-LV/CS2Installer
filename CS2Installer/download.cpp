#include "download.hpp"
#include "globals.hpp"

#include <Windows.h>
#include <filesystem>
#include <fstream>

#include <wininet.h>
#pragma comment(lib, "wininet.lib")

bool DownloadFile(const char* url, const char* outputFile) { /* currently not using it as bool because i dont need any checks for it at the moment */
	HINTERNET hInternet = InternetOpenA("FileDownloader", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	if (!hInternet) {
		printf("failed to initialize wininet.\n");
		return false;
	}

	HINTERNET hUrl = InternetOpenUrlA(hInternet, url, NULL, 0, INTERNET_FLAG_RELOAD, 0);
	if (!hUrl) {
		printf("failed to access url.\n");
		return false;
	}

	//printf("path -> %s\n", outputFile);

	HANDLE hFile = CreateFileA(outputFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		printf("failed to download file -> %s\n", outputFile);
		return false;
	}

	BYTE buffer[4096];
	DWORD bytesRead;
	while (InternetReadFile(hUrl, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
		DWORD bytesWritten;
		WriteFile(hFile, buffer, bytesRead, &bytesWritten, NULL);
	}

	CloseHandle(hFile);
	//printf("downloaded file to %s\n", outputFile);

	InternetCloseHandle(hUrl);
	InternetCloseHandle(hInternet);
}

std::string ReadOnlineString(const char* url) {
	HINTERNET hInternet = InternetOpenA("URLReader", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	if (!hInternet) {
		printf("failed to initialize wininet.\n");
		return "";
	}

	HINTERNET hUrl = InternetOpenUrlA(hInternet, url, NULL, 0, INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 0);
	if (!hUrl) {
		printf("failed to access url.\n");
		return "";
	}

	std::string result;
	constexpr DWORD bufferSize = 4096;
	char buffer[bufferSize];
	DWORD bytesRead;
	while (InternetReadFile(hUrl, buffer, bufferSize, &bytesRead) && bytesRead > 0) {
		result.append(buffer, bytesRead);
	}

	InternetCloseHandle(hUrl);
	InternetCloseHandle(hInternet);

	//printf("read from string -> %s\n", result.c_str());
	return result;
}

std::filesystem::path GetLocalAppData() {
	char* localAppData;
	size_t bufferSize = 0;
	errno_t result = _dupenv_s(&localAppData, &bufferSize, "LOCALAPPDATA");
	if (result != NULL || !localAppData) {
		printf("failed to get local appdata directory.\n");
		system("pause");
		exit(0);
	}

	std::filesystem::path localAppDataPath(static_cast<const char*>(localAppData));

	return localAppDataPath;
}

bool Downloader::needsUpdate() {
	std::string versionString = ReadOnlineString("https://raw.githubusercontent.com/CS2-OOF-LV/CS2Installer-Dependencies/main/version.txt");
	if (!versionString.empty() && versionString.find(Globals::currentVersion) == std::string::npos) /* check if the string isnt empty and isnt current version */
		return true;

	return false;
}

void Downloader::UpdateInstaller() {
	std::string currentAppPath;
	std::string updatedAppPath;

	/* get the application name */
	char buffer[MAX_PATH];
	GetModuleFileNameA(NULL, buffer, MAX_PATH);
	currentAppPath = buffer;
	updatedAppPath = currentAppPath + ".temp";

	/* download the update */
	if (!DownloadFile("https://github.com/CS2-OOF-LV/CS2Installer/raw/main/build/CS2Installer.exe", updatedAppPath.c_str())) {
		printf("failed to download update.\n");
		system("pause");
		exit(0);
	}

	/* extract file names from paths */
	std::string currentAppName = currentAppPath.substr(currentAppPath.find_last_of("\\/") + 1);
	std::string updatedAppName = updatedAppPath.substr(updatedAppPath.find_last_of("\\/") + 1);

	/* schedule deletion of the current installer after we exited our program */
	std::string deleteCommand = "del \"" + currentAppName + "\" & ren \"" + updatedAppName + "\" \"" + currentAppName + "\" & del \"%~f0\"";
	std::string batchFilePath = currentAppPath + ".bat";
	std::ofstream batchFile(batchFilePath);
	if (batchFile.is_open()) {
		batchFile << "@echo off\n";
		batchFile << deleteCommand;
		batchFile.close();
	}
	else {
		printf("failed to create the deletion script.\n");
		system("pause");
		exit(0);
	}

	/* create a seperate process for deletion */
	STARTUPINFOA startupInfo;
	PROCESS_INFORMATION processInfo;
	ZeroMemory(&startupInfo, sizeof(startupInfo));
	ZeroMemory(&processInfo, sizeof(processInfo));
	startupInfo.cb = sizeof(startupInfo);
	if (CreateProcessA(NULL, (LPSTR)batchFilePath.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo)) {
		/* close the handles so that the process can execute independently */
		CloseHandle(processInfo.hProcess);
		CloseHandle(processInfo.hThread);
	}
	else {
		printf("failed to create the deletion process.\n");
		system("pause");
		exit(0);
	}

	/* exit our prograam so that it can delete itself */
	exit(0);
}

void Downloader::PrepareDownload() {
	/* General Data we need for Preparing */
	std::filesystem::path currentPath = std::filesystem::current_path();
	std::filesystem::path localAppData = GetLocalAppData();
	const char* manifestNames[3] = { "730_2347770", "730_2347771", "730_2347779" };
	const char* depotKeys = "depot_keys.json";

	if (!Globals::usesNoManifests)
		std::filesystem::remove_all("manifestFiles"); // delete the manifestFiles folder everytime we download so that we dont accidentally cause the game to not update

	std::string stringPath = (currentPath / "manifestFiles").string();

	/* create manifest folder and download manifest files */
	std::filesystem::create_directory(currentPath / "manifestFiles");

	//printf("tried creating manifest directory at -> %s\n", stringPath.c_str());
	if (!Globals::usesNoManifests) {
		int maxIndex = sizeof(manifestNames) / sizeof(manifestNames[0]);
		for (int downloadIndex = 0; downloadIndex < maxIndex; ++downloadIndex) { /* download our manifest files */
			std::string downloadLink = "https://github.com/CS2-OOF-LV/CS2Installer-Dependencies/raw/main/";
			downloadLink += manifestNames[downloadIndex];

			std::string downloadPath = "manifestFiles/";
			downloadPath += manifestNames[downloadIndex];

			DownloadFile(downloadLink.c_str(), downloadPath.c_str());

			/*HRESULT downloadedManifest = URLDownloadToFileA(NULL, downloadLink.c_str(), downloadPath.c_str(), NULL, NULL);
			if (downloadedManifest != S_OK) {
				printf("failed to download manifest file.\n");
				printf("please report this to Nebel: %i\n", downloadedManifest);
				system("pause");
				exit(0);
			}*/
		}
	}

	/* Create steamctl folders and import depot keys */
	std::filesystem::path steamctlDirectory = localAppData / "steamctl";
	std::filesystem::create_directory(steamctlDirectory);
	std::filesystem::create_directory(steamctlDirectory / "steamctl");

	if (!Globals::usesNoManifests) {
		std::string downloadLink = "https://github.com/CS2-OOF-LV/CS2Installer-Dependencies/raw/main/";
		downloadLink += depotKeys;

		std::string downloadPath = localAppData.string();
		downloadPath += "\\steamctl\\steamctl\\";
		downloadPath += depotKeys;

		DownloadFile(downloadLink.c_str(), downloadPath.c_str());

		/*HRESULT downloadedKeys = URLDownloadToFileA(NULL, downloadLink.c_str(), downloadPath.c_str(), NULL, NULL);
		if (downloadedKeys != S_OK) {
			printf("failed to download depot keys.\n");
			printf("please report this to Nebel: %i\n", downloadedKeys);
			system("pause");
			exit(0);
		}*/
	}
}

void Downloader::DownloadCS2() {
	/* General Data we need for Downloading */
	std::filesystem::path currentPath = std::filesystem::current_path();
	const char* manifestNames[3] = { "730_2347770", "730_2347771", "730_2347779" };

	std::string stringPath = currentPath.string();
	std::filesystem::current_path(stringPath.c_str());

	/* Download CS2 using steams python library named steamctl */
	int maxIndex = sizeof(manifestNames) / sizeof(manifestNames[0]);
	for (int downloadIndex = 0; downloadIndex < (maxIndex); ++downloadIndex) {
		std::string manifestPath = "manifestFiles\\";
		manifestPath += manifestNames[downloadIndex];

		/* execute steamctl command using our manifests */
		std::string executeDownload = "\"" + currentPath.string() + "\\";
		executeDownload += "python-3.11.4-embed-amd64\\python.exe";
		executeDownload += "\"";
		executeDownload += " -m steamctl depot download -f ";
		executeDownload += manifestPath;
		executeDownload += " --skip-licenses --skip-login";
		//printf("command executed -> %s\n", executeDownload.c_str());
		system(executeDownload.c_str());
	}
}

void Downloader::DownloadMods() {
	std::filesystem::path currentPath = std::filesystem::current_path();
	const char* githubPaths[13] = {
		"https://github.com/CS2-OOF-LV/CS2-Client/raw/main/Mod%20Loading%20Files/game/csgo_mods/pak01_000.vpk",
		"https://github.com/CS2-OOF-LV/CS2-Client/raw/main/Mod%20Loading%20Files/game/csgo_mods/pak01_001.vpk",
		"https://github.com/CS2-OOF-LV/CS2-Client/raw/main/Mod%20Loading%20Files/game/csgo_mods/pak01_002.vpk",
		"https://github.com/CS2-OOF-LV/CS2-Client/raw/main/Mod%20Loading%20Files/game/csgo_mods/pak01_003.vpk",
		"https://github.com/CS2-OOF-LV/CS2-Client/raw/main/Mod%20Loading%20Files/game/csgo_mods/pak01_dir.vpk",
		"https://github.com/CS2-OOF-LV/CS2-Client/raw/main/Mod%20Loading%20Files/game/bin/win64/vscript.dll",
		"https://raw.githubusercontent.com/CS2-OOF-LV/CS2-Client/main/Mod%20Loading%20Files/game/csgo/gameinfo.gi",
		"https://raw.githubusercontent.com/CS2-OOF-LV/CS2-Client/main/Mod%20Loading%20Files/game/csgo/scripts/vscripts/banList.lua",
		"https://github.com/CS2-OOF-LV/CS2-Client/raw/main/Mod%20Loading%20Files/Start%20Game%20(English)%20-%20DEBUG.bat",
		"https://github.com/CS2-OOF-LV/CS2-Client/raw/main/Mod%20Loading%20Files/Start%20Game%20(English).bat",
		"https://github.com/CS2-OOF-LV/CS2-Client/raw/main/Mod%20Loading%20Files/Start%20Server.bat",
		"https://github.com/CS2-OOF-LV/CS2-Client/raw/main/Mod%20Loading%20Files/Workshop%20Tools%20-%20RAYTRACING.bat",
		"https://github.com/CS2-OOF-LV/CS2-Client/raw/main/Mod%20Loading%20Files/Workshop%20Tools.bat" };

	const char* filePaths[13] = {
		"game\\csgo_mods\\pak01_000.vpk",
		"game\\csgo_mods\\pak01_001.vpk",
		"game\\csgo_mods\\pak01_002.vpk",
		"game\\csgo_mods\\pak01_003.vpk",
		"game\\csgo_mods\\pak01_dir.vpk",
		"game\\bin\\win64\\vscript.dll"
		"game\\csgo\\gameinfo.gi",
		"game\\csgo\\scripts\\vscripts\\banList.lua",
		"Start Game (English) - DEBUG.bat",
		"Start Game (English).bat",
		"Start Server.bat",
		"Workshop Tools - RAYTRACING.bat",
		"Workshop Tools.bat" };

	/* Create needed folders */
	std::filesystem::create_directory(currentPath / "game" / "csgo_mods");
	std::filesystem::create_directory(currentPath / "game" / "csgo" / "scripts");
	std::filesystem::create_directory(currentPath / "game" / "csgo" / "scripts" / "vscripts");
	std::filesystem::create_directory(currentPath / "game" / "bin" / "win64");

	/* download files to specific directories */
	int maxIndex = sizeof(filePaths) / sizeof(filePaths[0]);
	for (int downloadIndex = 0; downloadIndex < maxIndex; ++downloadIndex) {
		std::filesystem::path filePath = filePaths[downloadIndex];
		if (std::filesystem::exists(filePath)) {
			std::filesystem::remove(filePath);
		}

		std::filesystem::path downloadFilePath = currentPath / filePaths[downloadIndex];
		std::string downloadPath = downloadFilePath.string();
		//printf("%s\n", downloadPath.c_str());

		DownloadFile(githubPaths[downloadIndex], downloadPath.c_str());

		/*HRESULT downloadedKeys = URLDownloadToFileA(NULL, githubPaths[downloadIndex], downloadPath.c_str(), NULL, NULL);
		if (downloadedKeys != S_OK) {
			printf("failed to download depot keys.\n");
			printf("please report this to Nebel: %i\n", downloadedKeys);
			system("pause");
			exit(0);
		}*/
	}
}