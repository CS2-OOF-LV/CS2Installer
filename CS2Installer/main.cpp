#include <Windows.h>
#include <iostream>
#include <filesystem>
#include <thread>
#include <chrono>

#include "globals.hpp"

#include "download.hpp"
#include "patcher.hpp"

int main(int argc, char* argv[]) {
	puts("下载之前请自行完成代理（或VPN，加速器等）的配置。");
	std::string wantsNextPatch;
	if (Downloader::needsUpdate()) {
		puts("需要更新，请按 Enter 键下载更新");
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
	puts("正在准备下载...");
	Downloader::PrepareDownload();
	puts("下载准备完毕！");
	Patcher::CleanPatchFiles();
	puts("开始下载...");
	Downloader::DownloadCS2();
	puts("下载完毕！");
	puts("Starting Patches...");
	Patcher::PatchClient();

	puts("是否要安装移动补丁？（建议bhop/surf服务器安装此补丁，以获得更好的移动效果）");
	puts("在键盘上按“Y”表示“是”或按“N”表示“否”");
	std::cin >> wantsNextPatch;
	for (char& c : wantsNextPatch) { /* make the anwser lowercase */
		c = std::tolower(c);
	}
	if (wantsNextPatch.find("y") != std::string::npos) {
		Patcher::PatchServer();
		puts("移动补丁安装完成！");
	}

	puts("是否安装来自oof.lv的游戏模组？（详见https://github.com/CS2-OOF-LV/CS2-Client）");
	puts("在键盘上按“Y”表示“是”或按“N”表示“否”");
	std::cin >> wantsNextPatch;
	for (char& c : wantsNextPatch) { /* make the anwser lowercase */
		c = std::tolower(c);
	}
	if (wantsNextPatch.find("y") != std::string::npos) {
	puts("开始下载客户端 Mod 补丁...");
	puts("这可能需要一会时间...");
	Downloader::DownloadMods();
	puts("已完成客户端模组补丁下载！");
	}

	puts("清理下载器文件...");
	if (!Globals::usesNoManifests) {
		std::filesystem::remove_all("manifestFiles");
	}
	puts("清理完毕！");
	puts("“神一般的起源2引擎——源神，启动！”");
	puts("下载完毕！按“Enter”关闭下载程序");
	waitforinput();
	return 0;
}
