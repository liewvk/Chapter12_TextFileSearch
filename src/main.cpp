#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <limits>
#include <filesystem>

namespace fs = std::filesystem;

struct SearchResult
{
    int lineNumber;
    std::string lineText;
};

void displayMenu()
{
    std::cout << std::endl;
    std::cout << "====================================" << std::endl;
    std::cout << "        Text File Search Tool" << std::endl;
    std::cout << "====================================" << std::endl;
    std::cout << "1. Search File" << std::endl;
    std::cout << "2. View Last Search Result" << std::endl;
    std::cout << "3. Exit" << std::endl;
    std::cout << "Please choose an option: ";
}

std::string toLowerCase(const std::string& text)
{
    std::string result = text;

    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char ch)
        {
            return static_cast<char>(std::tolower(ch));
        });

    return result;
}

bool lineContainsKeyword(const std::string& line, const std::string& keyword, bool caseSensitive)
{
    if (caseSensitive)
    {
        return line.find(keyword) != std::string::npos;
    }

    std::string lowerLine = toLowerCase(line);
    std::string lowerKeyword = toLowerCase(keyword);

    return lowerLine.find(lowerKeyword) != std::string::npos;
}

// Find the project root folder by looking for the data folder
fs::path findProjectRoot()
{
    fs::path currentPath = fs::current_path();

    for (int i = 0; i < 6; i++)
    {
        if (fs::exists(currentPath / "data"))
        {
            return currentPath;
        }

        if (currentPath.has_parent_path())
        {
            currentPath = currentPath.parent_path();
        }
    }

    return fs::current_path();
}

// Convert user input into a usable file path
fs::path getInputFilePath(const std::string& filename)
{
    fs::path userPath(filename);

    // If the user typed a full path or a valid relative path
    if (fs::exists(userPath))
    {
        return userPath;
    }

    fs::path projectRoot = findProjectRoot();

    // Try projectRoot / filename
    if (fs::exists(projectRoot / userPath))
    {
        return projectRoot / userPath;
    }

    // Try projectRoot / data / filename
    if (!userPath.has_parent_path())
    {
        fs::path dataPath = projectRoot / "data" / userPath;

        if (fs::exists(dataPath))
        {
            return dataPath;
        }
    }

    // Return original path if not found
    return userPath;
}

std::vector<SearchResult> searchFile(const std::string& filename, const std::string& keyword, bool caseSensitive)
{
    std::vector<SearchResult> results;

    fs::path inputFilePath = getInputFilePath(filename);

    std::ifstream file(inputFilePath);

    if (!file)
    {
        std::cout << "Error: Could not open file." << std::endl;
        std::cout << "Attempted path: " << inputFilePath << std::endl;
        return results;
    }

    std::string line;
    int lineNumber = 0;

    while (std::getline(file, line))
    {
        lineNumber++;

        if (lineContainsKeyword(line, keyword, caseSensitive))
        {
            SearchResult result;
            result.lineNumber = lineNumber;
            result.lineText = line;
            results.push_back(result);
        }
    }

    file.close();

    return results;
}

void displaySearchResults(const std::vector<SearchResult>& results)
{
    std::cout << std::endl;
    std::cout << "========== Search Results ==========" << std::endl;

    if (results.empty())
    {
        std::cout << "No matching lines found." << std::endl;
        return;
    }

    for (const SearchResult& result : results)
    {
        std::cout << "Line " << result.lineNumber << ": "
            << result.lineText << std::endl;
    }

    std::cout << std::endl;
    std::cout << "Total matching lines: " << results.size() << std::endl;
}

void saveSearchResultsToFile(
    const std::vector<SearchResult>& results,
    const fs::path& outputFilename,
    const std::string& sourceFilename,
    const std::string& keyword)
{
    fs::create_directories(outputFilename.parent_path());

    std::ofstream file(outputFilename);

    if (!file)
    {
        std::cout << "Error: Could not save search results." << std::endl;
        return;
    }

    file << "Search Source File: " << sourceFilename << std::endl;
    file << "Search Keyword: " << keyword << std::endl;
    file << "------------------------------------" << std::endl;

    if (results.empty())
    {
        file << "No matching lines found." << std::endl;
    }
    else
    {
        for (const SearchResult& result : results)
        {
            file << "Line " << result.lineNumber << ": "
                << result.lineText << std::endl;
        }

        file << std::endl;
        file << "Total matching lines: " << results.size() << std::endl;
    }

    file.close();

    std::cout << "Search results saved successfully." << std::endl;
    std::cout << "Saved to: " << outputFilename << std::endl;
}

void viewLastSearchResult(const fs::path& resultFilename)
{
    std::ifstream file(resultFilename);

    if (!file)
    {
        std::cout << "No saved search result found." << std::endl;
        std::cout << "Expected path: " << resultFilename << std::endl;
        return;
    }

    std::string line;

    std::cout << std::endl;
    std::cout << "========== Last Search Result ==========" << std::endl;

    while (std::getline(file, line))
    {
        std::cout << line << std::endl;
    }

    file.close();
}

void performSearch(const fs::path& resultFilename)
{
    std::string filename;
    std::string keyword;
    char caseChoice;
    bool caseSensitive;

    std::cout << "Enter file name: ";
    std::getline(std::cin, filename);

    std::cout << "Enter keyword to search: ";
    std::getline(std::cin, keyword);

    if (filename.empty())
    {
        std::cout << "File name cannot be empty." << std::endl;
        return;
    }

    if (keyword.empty())
    {
        std::cout << "Keyword cannot be empty." << std::endl;
        return;
    }

    std::cout << "Case-sensitive search? (Y/N): ";
    std::cin >> caseChoice;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    caseChoice = static_cast<char>(std::toupper(static_cast<unsigned char>(caseChoice)));
    caseSensitive = (caseChoice == 'Y');

    std::vector<SearchResult> results = searchFile(filename, keyword, caseSensitive);

    displaySearchResults(results);

    saveSearchResultsToFile(results, resultFilename, filename, keyword);
}

int main()
{
    fs::path projectRoot = findProjectRoot();
    fs::path resultFilename = projectRoot / "data" / "search_result.txt";

    int choice;

    while (true)
    {
        displayMenu();

        if (!(std::cin >> choice))
        {
            std::cout << "Invalid input. Please enter a number." << std::endl;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }

        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch (choice)
        {
        case 1:
            performSearch(resultFilename);
            break;

        case 2:
            viewLastSearchResult(resultFilename);
            break;

        case 3:
            std::cout << "Thank you for using the Text File Search Tool." << std::endl;
            return 0;

        default:
            std::cout << "Invalid option. Please choose again." << std::endl;
        }
    }
}