#ifndef SHADER_HPP
#define SHADER_HPP

#include <string>

using namespace std;

GLuint LoadShaders(string vertex_file_path, string fragment_file_path, string prefix = "");

#endif
