#include <iostream>
#include <assimp/Importer.hpp>

int main() {
    Assimp::Importer importer;
    std::string exts;
    importer.GetExtensionList(exts);
    std::cout << "Supported extensions: " << exts << std::endl;
    return 0;
}
