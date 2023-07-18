#include "patcher.hpp"

#include <Windows.h>
#include <iostream>
#include <filesystem>

bool Patcher::PatchClient() {
    const char* steamCheckBytes[2] = { "\x75\x70\xFF\x15", "\xEB\x70\xFF\x15" };
    const char* versionCheckBytes[4] = { "\x6C\x69\x6D\x69\x74\x65\x64\x62\x65\x74\x61", "\x66\x75\x6C\x6C\x76\x65\x72\x73\x69\x6F\x6E", /* there are two checks */ "\x6C\x69\x6D\x69\x74\x65\x64\x62\x65\x74\x61", "\x66\x75\x6C\x6C\x76\x65\x72\x73\x69\x6F\x6E"};
    const char* voiceScaleBytes[2] = { "\x80\x3F\x0F\x11\x44\x24\x60\xE8\x2B\x33", "\xA0\x40\x0F\x11\x44\x24\x60\xE8\x2B\x33" }; /* valve keeps patching this signature ;< need a better signature */

    if (!ReplaceBytes("game/csgo/bin/win64/client.dll", steamCheckBytes[0], steamCheckBytes[1])) {
        puts("failed to patch steam check");
        return false;
    }

    //printf("patched steam check: %s -> %s\n", steamCheckBytes[0], steamCheckBytes[1]);
    if (!ReplaceBytes("game/csgo/bin/win64/client.dll", versionCheckBytes[0], versionCheckBytes[1]) || !ReplaceBytes("game/csgo/bin/win64/client.dll", versionCheckBytes[2], versionCheckBytes[3])) {
        puts("failed to patch version check");
        return false;
    }

    //printf("patched version check: %s -> %s\n", versionCheckBytes[0], versionCheckBytes[1]);
    if (!ReplaceBytes("game/csgo/bin/win64/client.dll", voiceScaleBytes[0], voiceScaleBytes[1])) {
        puts("failed to patch voice_scale");
        return false;
    }

    //printf("patched voice_scale: %s -> %s\n", voiceScaleBytes[0], voiceScaleBytes[1]);
    return true;
}

bool Patcher::PatchServer() {
    const char* clampCheckBytes[2] = { "\x76\x59\xF2\x0F\x10\x4F\x3C", "\xEB\x59\xF2\x0F\x10\x4F\x3C" };

    if (!ReplaceBytes("game/csgo/bin/win64/server.dll", clampCheckBytes[0], clampCheckBytes[1])) {
        puts("failed to patch movement clamp");
        return false;
    }

    //printf("patched movement clamp: %s -> %s\n", clampCheckBytes[0], clampCheckBytes[1]);
    return true;
}

void RemoveExistingPatchFiles(const char* path) {
    std::filesystem::path filePath = path;
    if (std::filesystem::exists(filePath)) {
        std::filesystem::remove(filePath);
    }
}

void Patcher::CleanPatchFiles() {
    RemoveExistingPatchFiles("game/csgo/bin/win64/client.dll");
    RemoveExistingPatchFiles("game/csgo/bin/win64/server.dll");
}

bool Patcher::ReplaceBytes(const char* filename, const char* searchPattern, const char* replaceBytes) {
    size_t replaceLength = strlen(replaceBytes);

    FILE* file = fopen(filename, "rb+");
    if (!file) {
        printf("failed to open: %s\n", filename);
        return false;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);

    unsigned char* buffer = (unsigned char*)malloc(fileSize);
    if (!buffer) {
        printf("failed to allocate memory.\n");
        fclose(file);
        return false;
    }

    fread(buffer, 1, fileSize, file);

    unsigned char* position = buffer;
    unsigned char* endPosition = buffer + fileSize - replaceLength;

    while (position <= endPosition) {
        if (memcmp(position, searchPattern, replaceLength) == 0) {
            fseek(file, position - buffer, SEEK_SET);
            fwrite(replaceBytes, 1, replaceLength, file);
            break;
        }
        position++;
    }

    if (position > endPosition) {
        printf("patcher out of date, report this to nebel: %s | %i\n", filename, (int)replaceLength);
        return false;
    }

    free(buffer);
    fclose(file);
    return true;
}