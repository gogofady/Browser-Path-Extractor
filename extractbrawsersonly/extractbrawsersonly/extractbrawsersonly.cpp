#include <iostream>
#include <windows.h>
#include <string>
#include <vector>
#include <fstream>

struct Browser {
    std::string name;
    std::string registryPath;
    HKEY rootKey;
    std::string installPath;
};

bool getRegistryValue(HKEY hKeyRoot, const std::string& subKey, const std::string& valueName, std::string& value) {
    HKEY hKey;
    if (RegOpenKeyExA(hKeyRoot, subKey.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        return false;
    }
    
    char valueBuffer[512];
    DWORD bufferSize = sizeof(valueBuffer);
    if (RegQueryValueExA(hKey, valueName.c_str(), nullptr, nullptr, (LPBYTE)valueBuffer, &bufferSize) == ERROR_SUCCESS) {
        value = valueBuffer;
        RegCloseKey(hKey);
        return true;
    }
    
    RegCloseKey(hKey);
    return false;
}

std::vector<Browser> getInstalledBrowsers() {
    std::vector<Browser> browsers = {
        // Google Chrome
        {"Google Chrome", R"(SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Google Chrome)", HKEY_LOCAL_MACHINE, ""},
        {"Google Chrome (WOW64)", R"(SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall\Google Chrome)", HKEY_LOCAL_MACHINE, ""},

        // Mozilla Firefox
        {"Mozilla Firefox", R"(SOFTWARE\Mozilla\Mozilla Firefox)", HKEY_LOCAL_MACHINE, ""},
        {"Mozilla Firefox (Current User)", R"(SOFTWARE\Mozilla\Mozilla Firefox)", HKEY_CURRENT_USER, ""},
        {"Mozilla Firefox (Install Directory)", R"(SOFTWARE\Mozilla\Mozilla Firefox\CurrentVersion\Main)", HKEY_LOCAL_MACHINE, ""},

        // Microsoft Edge
        {"Microsoft Edge", R"(SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\msedge.exe)", HKEY_LOCAL_MACHINE, ""},

        // Opera
        {"Opera", R"(SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Opera Stable)", HKEY_LOCAL_MACHINE, ""},
        {"Opera (WOW64)", R"(SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall\Opera Stable)", HKEY_LOCAL_MACHINE, ""},

        // Brave
        {"Brave", R"(SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\BraveSoftware Brave-Browser)", HKEY_LOCAL_MACHINE, ""},
        {"Brave (WOW64)", R"(SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall\BraveSoftware Brave-Browser)", HKEY_LOCAL_MACHINE, ""}
    };

    for (auto& browser : browsers) {
        // Try to get "InstallLocation" or "Path" for the browser location
        if (!getRegistryValue(browser.rootKey, browser.registryPath, "InstallLocation", browser.installPath)) {
            getRegistryValue(browser.rootKey, browser.registryPath, "Path", browser.installPath);
        }

        // For Firefox, check additional "Install Directory" value
        if (browser.name.find("Mozilla Firefox") != std::string::npos && browser.installPath.empty()) {
            getRegistryValue(browser.rootKey, browser.registryPath, "Install Directory", browser.installPath);
        }

        // If DisplayIcon path is found, it may include an executable path, so remove it
        if (browser.installPath.empty()) {
            getRegistryValue(browser.rootKey, browser.registryPath, "DisplayIcon", browser.installPath);
        }
        
        size_t exePos = browser.installPath.find_last_of("\\");
        if (exePos != std::string::npos) {
            browser.installPath = browser.installPath.substr(0, exePos);
        }
    }
    
    return browsers;
}

void saveBrowserPathsToFile(const std::vector<Browser>& browsers) {
    std::ofstream file("browsers_paths.txt");
    if (file.is_open()) {
        for (const auto& browser : browsers) {
            if (!browser.installPath.empty()) {
                file << browser.name << " - " << browser.installPath << std::endl;
                std::cout << browser.name << " found at: " << browser.installPath << std::endl;
            }
        }
        file.close();
        std::cout << "Browser paths have been saved to browsers_paths.txt" << std::endl;
    } else {
        std::cerr << "Error: Unable to open file for writing." << std::endl;
    }
}

int main() {
    std::vector<Browser> browsers = getInstalledBrowsers();
    if (browsers.empty()) {
        std::cout << "No known browsers found in the registry." << std::endl;
    } else {
        std::cout << "Installed browsers found in the registry:" << std::endl;
        saveBrowserPathsToFile(browsers);
    }
    return 0;
}
