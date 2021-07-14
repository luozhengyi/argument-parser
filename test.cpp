#include <iostream>
#include <string>
#include "ArgParser.h"
using namespace std;

int main(int argc, char* argv[])
{
  ArgParser obj;
  ArgParser::option::Builder builder;

  ArgParser::option* prop = builder.set_opt_type(ArgParser::OPTTP::FILE).set_default_vvalue({"tmp.txt"}).build();
  obj.addOpt("--file|-f", *prop);

  prop = builder.set_opt_type(ArgParser::OPTTP::STRING).set_help("this is invalid option name.").build();
  obj.addOpt("--str123", *prop);

  prop = builder.clear().set_opt_type(ArgParser::OPTTP::BOOL).set_default_bvalue(false).set_help("Whether rebuild the test file. Default Valuse: FALSE.").build();
  obj.addOpt("--rebuild", *prop);

  prop = builder.clear().set_opt_type(ArgParser::OPTTP::CHOICE).set_default_vvalue({"2021.9","2021.6","2021.3","2020.12"}).set_help("Specify the version.").build();
  obj.addOpt("--version|-v", *prop);

  obj.print_opts();
  std::cout << std::endl;

  std::string file;
  bool rb;
  obj.setf([&](std::map<std::string, ArgParser::option>& opts) -> bool {
    file = opts["--file"].val.get_vval();
    rb = opts["--rebuild"].val.get_bval();
    return true;
  });
  std::string text = "  xxx.sh --file text.txt -t --rebuild  -k 10  ";
  // obj.parse(text);
  // obj.print_args();
  obj.parse(argc, argv);
  std::cout << file << std::endl;

  return 0;
}
