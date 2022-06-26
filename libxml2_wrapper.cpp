/*
 * Summary: libxml2 wrapper
 * Description: extracts contents encased between html tags
 * Libraries used: libiconv.1.14.0.11, libxml2.2.7.8.7
 * 
 * Author: strmbld wlfhrtd
 * 
 * All credit to libxml2 authors
 */

#include <iostream>
#include <libxml/HTMLparser.h>
#include <fstream>
#include <vector>
#include <algorithm>
#include <filesystem>

bool isTag(xmlNode* cur_node, const std::vector<std::string>& tags)
{
    if (cur_node->parent->type == XML_ELEMENT_NODE)
    {
        std::string elemName = reinterpret_cast<const char*>(cur_node->parent->name);
        return std::any_of(tags.cbegin(), tags.cend(), [elemName](std::string s) {return elemName == s; });
    }
    return false;
}

void traverse_dom_trees(xmlNode* a_node, std::string& output, const std::vector<std::string>& tags)
{
    xmlNode* cur_node = NULL;

    if (a_node == NULL) return;

    for (cur_node = a_node; cur_node; cur_node = cur_node->next)
    {
        if (cur_node->type == XML_TEXT_NODE && isTag(cur_node, tags))
        {
            output += reinterpret_cast<const char*>(cur_node->content);
            output += "\n";
        }
        traverse_dom_trees(cur_node->children, output, tags);
    }
}

void parseFile(const char* filename, std::string& output, const std::vector<std::string>& tags)
{
    htmlDocPtr document;
    xmlNode* root_element = NULL;

    document = htmlReadFile(filename, "UTF-8", HTML_PARSE_NOBLANKS | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING | HTML_PARSE_NONET);

    if (document == NULL)
    {
        std::cerr << "\nFailed to parse document\n";
        exit(-1);
    }

    root_element = xmlDocGetRootElement(document);

    if (root_element == NULL)
    {
        std::cerr << "\nUnable to get RootElement or Document is empty\n";
        xmlFreeDoc(document);
        exit(-1);
    }

    traverse_dom_trees(root_element, output, tags);

    xmlFreeDoc(document);
    xmlCleanupParser();
}

void readTagsFromCin(std::vector<std::string>& tags)
{
    while (true)
    {
        std::string s;
        std::cin >> s;
        if (s == "quit")
        {
            break;
        }
        tags.emplace_back(s);
    }
}

void readToFile(const char* filename, std::string& output)
{
    std::ofstream out;
    out.open(filename);
    if (!out.is_open())
    {
        std::cerr << "Unable to read into file. Filename: " << filename << std::endl;
        exit(-1);
    }
    out << output;
    out.close();
}

static inline void getCurrDirFilenames(std::vector<std::filesystem::path>& paths)
{
    for (const auto& entry : std::filesystem::directory_iterator(std::filesystem::current_path())) paths.emplace_back(entry.path());
}

void doBatch(std::string& output, const std::vector<std::string>& tags, const std::string& suffix, const std::string& excludeAppName)
{
    std::vector<std::filesystem::path> paths{};
    getCurrDirFilenames(paths);
    const char* pInputFilename;
    const char* pOutputFilename;

    for (const auto& path : paths)
    {
        std::string inputFilename = path.filename().string();
        if (inputFilename == excludeAppName)
        {
            continue;
        }
        pInputFilename = inputFilename.c_str();
        parseFile(pInputFilename, output, tags);

        std::string newOutputFilename = path.stem().string() + suffix + path.extension().string();
        pOutputFilename = newOutputFilename.c_str();
        readToFile(pOutputFilename, output);

        output = "";
    }
}

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        std::cerr << "\nInvalid argument\n"
            << "Enter input/output filenames (e.g. input.html, result.txt)\n"
            << "For batch operation in current directory start with -b parameter extended by name suffix for processed files\n"
            << "e.g. libxml2_wrapper.exe -b __result\n"
            << "so new files will have names like %original_filename%suffix.txt\n";
        return -1;
    }

    // macro to check api, should match to used dll
    LIBXML_TEST_VERSION

    std::cout << "Enter tags (separated by space) for data that should be extracted.\n"
              << "(e.g. li h2 b i a)"
              << "Type 'quit' (without quotes) to stop input.\n";

    std::vector<std::string> tags{};
    readTagsFromCin(tags);
    
    std::string output = "";

    if (std::basic_string_view(argv[1]) == "-b")
    {
        std::string suffix = std::string(argv[2]);
        std::string excludeAppName = std::string(argv[0]);
        doBatch(output, tags, suffix, excludeAppName);

        return 0;
    }

    parseFile(argv[1], output, tags);

    readToFile(argv[2], output);

    return 0;
}