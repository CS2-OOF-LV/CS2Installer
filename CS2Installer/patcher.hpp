#pragma once

#define _CRT_SECURE_NO_WARNINGS

namespace Patcher {
	bool ReplaceBytes(const char* filename, const char* searchPattern, const char* replaceBytes);
	bool PatchClient();
	bool PatchServer();
	bool PatchVScript();
	void CleanPatchFiles();
}