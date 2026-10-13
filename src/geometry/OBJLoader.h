#pragma once

#include <string>

#include "Mesh.h"

class OBJLoader
{

public:

    static Mesh Load(
        const std::string& filepath
    );

};