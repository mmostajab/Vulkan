#ifndef __HELPER_H__
#define __HELPER_H__

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include "glm/glm.hpp"


static std::string convertFileToString(const std::string& filename);

// read the file content and generate a string from it.
static std::string convertFileToString(const std::string& filename) {
    std::ifstream ifile(filename);
    if (!ifile){
        return std::string("");
    }

    return std::string(std::istreambuf_iterator<char>(ifile), (std::istreambuf_iterator<char>()));

}

#endif