/**
 * VST3 Lister - Utility for listing VST3 plugins
 * 
 * This program scans a VST3 directory and creates CSV reports of all VST plugins found,
 * organized by manufacturer.
 * 
 * Converted from vst3_list.py Python script by: salva
 * v1.0
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <filesystem>
#include <algorithm>
#include <stdexcept>
#include <sstream>

namespace fs = std::filesystem;

/**
 * @class VST3Lister
 * @brief Class that handles the scanning and reporting of VST3 plugins
 */
class VST3Lister {
public:
    /**
     * @brief Constructor that initializes constants
     */
    VST3Lister() {
        VST3_EXTENSIONS = {".vst3", ".dll"};
        EXCLUDED_EXTENSIONS = {".ini"};
        NOT_RECOGNIZED_LABEL = "_Not recognized";
        UNKNOWN_LABEL = "_Unknown";
    }

    /**
     * @brief Process a VST3 folder and generate CSV reports
     * @param pathVst3 The path to the VST3 folder to scan
     * @param pathReport The path where to save the CSV reports
     * @throws std::runtime_error if there are issues accessing directories or files
     */
    void process(const std::string& pathVst3, const std::string& pathReport) {
        if (!fs::exists(pathVst3)) {
            throw std::runtime_error("VST3 folder does not exist: " + pathVst3);
        }
        
        if (!fs::exists(pathReport)) {
            throw std::runtime_error("Report folder does not exist: " + pathReport);
        }
        
        // Process the VST3 folder
        auto vstData = processVST3Folder(pathVst3);
        
        // Save data to CSV files
        saveToCSV(vstData, pathReport);
    }

private:
    // Constants
    std::vector<std::string> VST3_EXTENSIONS;
    std::vector<std::string> EXCLUDED_EXTENSIONS;
    std::string NOT_RECOGNIZED_LABEL;
    std::string UNKNOWN_LABEL;
    
    /**
     * @struct FilePathInfo
     * @brief Contains information about a file path
     */
    struct FilePathInfo {
        std::string baseName;    // File name with extension
        std::string extension;   // File extension (lowercase)
        std::string pathPrefix;  // Common path prefix
        std::string pathSuffix;  // Relative path from prefix to parent directory
    };
    
    /**
     * @brief Extracts file information such as basename, extension, common prefix, and relative path
     * @param currPath The current file path to analyze
     * @param basePath The base path for relative path calculation
     * @return FilePathInfo structure with path information
     */
    FilePathInfo extractFilePathInfo(const std::string& currPath, const std::string& basePath) {
        FilePathInfo info;
        fs::path path(currPath); // to create an object called path, from the std::filesystem::path class (fs::path)
        
        // Extract basename (filename with extension)
        info.baseName = path.filename().string();
        
        // Extract and normalize extension to lowercase
        info.extension = path.extension().string();
        std::transform(info.extension.begin(), info.extension.end(), 
                       info.extension.begin(), ::tolower);
        
        // Find common path prefix
        fs::path commonPath = findCommonPath(currPath, basePath);
        info.pathPrefix = commonPath.string();
        
        // Calculate relative path (directory only)
        fs::path dirPath = path.parent_path();
        info.pathSuffix = fs::relative(dirPath, commonPath).string();
        if (info.pathSuffix.empty()) {
            info.pathSuffix = ".";
        }
        
        return info;
    }
    
    /**
     * @brief Find the common path between two paths
     * @param path1 First path
     * @param path2 Second path
     * @return Common path prefix
     */
    fs::path findCommonPath(const std::string& path1, const std::string& path2) {
        fs::path p1 = fs::absolute(path1);
        fs::path p2 = fs::absolute(path2);
        
        auto it1 = p1.begin();
        auto it2 = p2.begin();
        
        fs::path result;
        
        // Compare path components
        while (it1 != p1.end() && it2 != p2.end() && *it1 == *it2) {
            result /= *it1;
            ++it1;
            ++it2;
        }
        
        return result;
    }
    
    /**
     * @brief Check if a path is a subpath of any path in a list
     * @param parentPaths List of potential parent paths
     * @param childPath Path to check
     * @return true if childPath is a subpath of any path in parentPaths
     */
    bool isSubPath(const std::vector<std::string>& parentPaths, const std::string& childPath) {
        for (const auto& parent : parentPaths) {
            if (parent.size() <= childPath.size() && childPath.compare(0, parent.size(), parent) == 0) {
                return true;
            }
        }
        return false;
    }
    
    /**
     * @brief Check if a file has a specific extension
     * @param filePath Path to the file
     * @param extensions List of extensions to check
     * @return true if the file has one of the extensions
     */
    bool hasExtension(const std::string& filePath, const std::vector<std::string>& extensions) {
        fs::path path(filePath); // to create an object called path, from the std::filesystem::path class (fs::path)
        std::string ext = path.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        
        return std::find(extensions.begin(), extensions.end(), ext) != extensions.end();
    }
    
    /**
     * @brief List all files and directories in a path, applying filters
     * @param pathStart Starting path for the search
     * @return Pair of vectors: first with file paths, second with directory paths
     */
    std::pair<std::vector<std::string>, std::vector<std::string>> listFilesAndDirs(const std::string& pathStart) {
        std::vector<std::string> listSubPaths, listFiles;
        
        try {
            for (const auto& entry : fs::recursive_directory_iterator(pathStart)) {
                std::string entryPath = entry.path().string();
                
                if (entry.is_directory()) {
                    if (hasExtension(entryPath, VST3_EXTENSIONS)) {
                        listFiles.push_back(entryPath);
                    } else {
                        listSubPaths.push_back(entryPath);
                    }
                } else if (entry.is_regular_file()) {
                    if (!hasExtension(entryPath, EXCLUDED_EXTENSIONS)) {
                        // Check if the file is not inside any .vst3 folder
                        if (!isSubPath(listFiles, entryPath)) {
                            listFiles.push_back(entryPath);
                        }
                    }
                }
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Warning: " << e.what() << std::endl;
            // Continue with partial results
        }
        
        return {listFiles, listSubPaths};
    }
    
    /**
     * @brief Determine the name of the VST plugin and its manufacturer
     * @param currPath Current file path
     * @param startPath Starting path of the search
     * @return Tuple with VST name, generic filename, and manufacturer
     */
    std::tuple<std::string, std::string, std::string> nameOfVST(const std::string& currPath, const std::string& startPath) {
        FilePathInfo info = extractFilePathInfo(currPath, startPath);
        
        // Determine manufacturer based on path
        std::string manufacturer = (info.pathSuffix == ".") ? UNKNOWN_LABEL : info.pathSuffix;
        
        // Create generic filename with path
        std::string filenameGen;
        if (info.pathSuffix == ".") {
            filenameGen = info.baseName;
        } else {
            // Use platform-specific path separator
            filenameGen = (info.pathSuffix + std::string(1, fs::path::preferred_separator) + info.baseName);
        }
        
        // If it's a VST file, return the VST name
        if (hasExtension(currPath, VST3_EXTENSIONS)) {
            return {info.baseName, filenameGen, manufacturer};
        }
        
        // Otherwise, mark as not recognized
        return {"", filenameGen, NOT_RECOGNIZED_LABEL};
    }
    
    /**
     * @brief Process the VST3 folder and categorize plugins by manufacturer
     * @param pathVst3 Path to the VST3 folder
     * @return Map with manufacturer as key and vector of plugin names as value
     */
    std::map<std::string, std::vector<std::string>> processVST3Folder(const std::string& pathVst3) {
        // List all files in the directory
        auto [allFiles, _] = listFilesAndDirs(pathVst3);
        
        // Initialize result map with special categories
        std::map<std::string, std::vector<std::string>> vstDict;
        vstDict[UNKNOWN_LABEL] = {};
        vstDict[NOT_RECOGNIZED_LABEL] = {};
        
        // Process each file
        for (const auto& currPath : allFiles) {
            auto [currVstFile, currGenFile, currVstDev] = nameOfVST(currPath, pathVst3);
            
            // Ensure the category exists in the map
            if (vstDict.find(currVstDev) == vstDict.end()) {
                vstDict[currVstDev] = {};
            }
            
            // Add to appropriate category
            if (!currVstFile.empty()) {
                vstDict[currVstDev].push_back(currVstFile);
            } else {
                vstDict[currVstDev].push_back(currGenFile);
            }
        }
        
        return vstDict;
    }
    
    /**
     * @brief Escape a string for CSV output
     * @param str String to escape
     * @return Escaped string with quotes if needed
     */
    std::string escapeCSV(const std::string& str) {
        if (str.find(',') != std::string::npos || str.find('"') != std::string::npos) {
            std::string result = str;
            // Replace all " with ""
            size_t pos = 0;
            while ((pos = result.find('"', pos)) != std::string::npos) {
                result.replace(pos, 1, "\"\"");
                pos += 2;
            }
            return "\"" + result + "\"";
        }
        return str;
    }
    
    /**
     * @brief Save the VST3 data to CSV files
     * @param vst3Data Map with manufacturer as key and vector of plugin names as value
     * @param pathReport Path where to save the CSV files
     */
    void saveToCSV(const std::map<std::string, std::vector<std::string>>& vst3Data, const std::string& pathReport) {
        fs::path reportPath(pathReport);
        fs::path pathSprd = reportPath / "VST3_List.csv";
        fs::path pathChck = reportPath / "VST3_2Check.csv";
        
        // Create the first CSV: all manufacturers except unknown and not recognized
        try {
            std::ofstream spreadsheet(pathSprd);
            if (!spreadsheet) {
                throw std::runtime_error("Cannot create file: " + pathSprd.string());
            }
            
            // Collect regular manufacturers (excluding special categories)
            std::vector<std::string> manufacturers;
            for (const auto& [key, _] : vst3Data) {
                if (key != UNKNOWN_LABEL && key != NOT_RECOGNIZED_LABEL) {
                    manufacturers.push_back(key);
                }
            }
            
            // Write header
            bool firstCol = true;
            for (const auto& manufacturer : manufacturers) {
                if (!firstCol) {
                    spreadsheet << ",";
                }
                spreadsheet << escapeCSV(manufacturer);
                firstCol = false;
            }
            spreadsheet << std::endl;
            
            // Find the maximum number of entries in any category
            size_t maxEntries = 0;
            for (const auto& manufacturer : manufacturers) {
                maxEntries = std::max(maxEntries, vst3Data.at(manufacturer).size());
            }
            
            // Write data rows
            for (size_t i = 0; i < maxEntries; ++i) {
                firstCol = true;
                for (const auto& manufacturer : manufacturers) {
                    if (!firstCol) {
                        spreadsheet << ",";
                    }
                    if (i < vst3Data.at(manufacturer).size()) {
                        spreadsheet << escapeCSV(vst3Data.at(manufacturer)[i]);
                    }
                    firstCol = false;
                }
                spreadsheet << std::endl;
            }
            spreadsheet.close();
        } catch (const std::exception& e) {
            throw std::runtime_error("Error writing to VST3_List.csv: " + std::string(e.what()));
        }
        
        // Create the second CSV: only unknown and not recognized
        try {
            std::ofstream checklist(pathChck);
            if (!checklist) {
                throw std::runtime_error("Cannot create file: " + pathChck.string());
            }
            
            // Write header
            checklist << escapeCSV(UNKNOWN_LABEL) << "," << escapeCSV(NOT_RECOGNIZED_LABEL) << std::endl;
            
            // Find the maximum number of entries
            size_t maxUnknown = vst3Data.at(UNKNOWN_LABEL).size();
            size_t maxNotRecognized = vst3Data.at(NOT_RECOGNIZED_LABEL).size();
            size_t maxEntries = std::max(maxUnknown, maxNotRecognized);
            
            // Write data rows
            for (size_t i = 0; i < maxEntries; ++i) {
                if (i < maxUnknown) {
                    checklist << escapeCSV(vst3Data.at(UNKNOWN_LABEL)[i]);
                }
                checklist << ",";
                if (i < maxNotRecognized) {
                    checklist << escapeCSV(vst3Data.at(NOT_RECOGNIZED_LABEL)[i]);
                }
                checklist << std::endl;
            }
            checklist.close();
        } catch (const std::exception& e) {
            throw std::runtime_error("Error writing to VST3_2Check.csv: " + std::string(e.what()));
        }
    }
};

/**
 * Main function
 */
int main() {
    try {
        std::string pathVst3, pathRprt;
        
        // Get VST3 folder path from user or use default
        std::cout << "VST3 folder ([C:\\Program Files\\Common Files\\VST3]): ";
        std::getline(std::cin, pathVst3);
        if (pathVst3.empty()) {
            pathVst3 = "C:\\Program Files\\Common Files\\VST3";
        }
        
        // Get report folder path from user or use current directory
        std::cout << "Excel folder ([" << fs::current_path().string() << "]): ";
        std::getline(std::cin, pathRprt);
        if (pathRprt.empty()) {
            pathRprt = fs::current_path().string();
        }
        
        // Create VST3Lister object and process the folders
        VST3Lister lister;
        lister.process(pathVst3, pathRprt);
        
        std::cout << "VST3 list generated successfully!" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}