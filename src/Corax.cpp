
#include <iostream>
#include <sstream>
#include <filesystem>

#include "CLI.hpp"
#include "precompiler.h"
#include "compiler.h"
#include "error.h"

using std::filesystem::path;

CompilerListener comp;
Error err;

int main(int argc, const char* argv[])
{

  CLI::App app{"Compiler for the Corvus flavor of C"};

  std::string filename;
  app.add_option("file", filename, "File name")
    ->required()
    ->check(CLI::ExistingFile);

  CLI11_PARSE(app, argc, argv);

  path infile = filename;

  ProcessedCode code;

  Precompiler pre(&code, infile, &err);

  pre.Process();

  std::cout << code.code;



  // std::cout << prepped;

  // std::ifstream stream2;
  // std::stringstream buffer;
  // stream2.open(filename);
  // buffer << stream2.rdbuf();

  // std::cout << buffer.str();

  // stream2.open(filename);
  // ANTLRInputStream postinput(stream2);
  // comp.Process(&postinput);

  return 0;
}