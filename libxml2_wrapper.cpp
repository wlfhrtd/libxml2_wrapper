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

std::string output = "";
std::vector<std::string> tags = {};

bool isTag(xmlNode* cur_node)
{
    if (cur_node->parent->type == XML_ELEMENT_NODE)
    {
        std::string elemName = reinterpret_cast<const char*>(cur_node->parent->name);
        return std::any_of(tags.cbegin(), tags.cend(), [elemName](std::string s) {return elemName == s; });
    }
    return false;
}

void traverse_dom_trees(xmlNode* a_node)
{
    xmlNode* cur_node = NULL;

    if (a_node == NULL) return;

    for (cur_node = a_node; cur_node; cur_node = cur_node->next)
    {
        if (cur_node->type == XML_TEXT_NODE && isTag(cur_node))
        {
            output += reinterpret_cast<const char*>(cur_node->content);
            output += "\n";
        }
        traverse_dom_trees(cur_node->children);
    }
}

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        std::cerr << "\nInvalid argument\n"
                  << "Enter input/output filenames (e.g. input.html, result.txt)\n";
        return -1;
    }

    // macro to check api, should match to used dll
    LIBXML_TEST_VERSION

    std::cout << "Enter tags (separated by space) for data that should be extracted.\n"
              << "(e.g. li h2 b i a)"
              << "Type 'quit' (without quotes) to stop input.\n";

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
   
    htmlDocPtr document;
    xmlNode* root_element = NULL;

    document = htmlReadFile(argv[1], "UTF-8", HTML_PARSE_NOBLANKS | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING | HTML_PARSE_NONET);

    if (document == NULL)
    {
        std::cerr << "\nFailed to parse document\n";
        return -1;
    }
    
    root_element = xmlDocGetRootElement(document);

    if (root_element == NULL)
    {
        std::cerr << "\nUnable to get RootElement or Document is empty\n";
        xmlFreeDoc(document);
        return -1;
    }

    traverse_dom_trees(root_element);

    std::ofstream out;
    out.open(argv[2]);
    out << output;
    out.close();

    xmlFreeDoc(document);
    xmlCleanupParser();
    return 0;
}