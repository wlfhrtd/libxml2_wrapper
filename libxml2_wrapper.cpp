#include <iostream>
#include <libxml/HTMLparser.h>
#include <fstream>

std::string output = "";

void traverse_dom_trees(xmlNode* a_node)
{
    xmlNode* cur_node = NULL;

    if (a_node == NULL)
    {
        // std::cerr << "Invalid argument a_node " << reinterpret_cast<const char*>(a_node->name) << "\n"; // always triggered and process always stops here somehow
        // but fine with commented; interesting
        return;
    }

    for (cur_node = a_node; cur_node; cur_node = cur_node->next)
    {
        if (cur_node->type == XML_ELEMENT_NODE)
        {
            // check if current node should be excluded
            output += "Node type: Text, name: ";
            output += reinterpret_cast<const char*>(cur_node->name);
            output += "\n";
        }
        else if (cur_node->type == XML_TEXT_NODE)
        {
            // process text node
            output += "Node type: Text, node content: ";
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
        std::cerr << "\nInvalid argument\n";
        return -1;
    }

    // macro to check api, should match to used dll
    LIBXML_TEST_VERSION

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
        return 0;
    }

    output += "Root Node is ";
    output += reinterpret_cast<const char*>(root_element->name);
    output += "\n";
    
    traverse_dom_trees(root_element);

    std::ofstream out;
    out.open(argv[2]);
    out << output;
    out.close();

    xmlFreeDoc(document); // free document
    xmlCleanupParser(); // free globals
    return 0;
}