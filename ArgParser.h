//
//  ArgParser.h
//
//  Created by Zhengyi Luo on 2021-5-03.
//  Copyright (c) 2021 Zhengyi Luo. All rights reserved.
//

#ifndef __ARGPARSER_H__
#define __ARGPARSER_H__

#include <map>
#include <string>
#include <vector>
#include <functional>
#include <iostream>
#include <list>
#include <regex>
#include <algorithm>
#include <iomanip>
class ArgParser
{
public:

  enum class OPTTP {
    BOOL,
    STRING,
    CHOICE,
    FILE,
    PATH
  };

  typedef struct argval {
    OPTTP tp;

    bool bval; /* only for OPTTP::BOOL */

    std::vector<std::string> vval; /* vval[0] is the default value */

    argval():bval(false), tp(OPTTP::STRING){}
    bool get_bval() { return bval; }
    std::string get_vval() { return vval[0]; }
    argval& clear() { tp = OPTTP::STRING; bval = false; vval.clear(); return *this; }
    argval& operator =(argval arg) { this->tp = arg.tp; this->bval = arg.bval; this->vval = arg.vval; return *this; }
  } argval;

public:
  typedef struct option {
    argval val;
    std::string help;
    bool required;
    bool multiple;
    bool equal;  /* like --file="./file.txt" */
    bool append; /* like -j4 */
    std::string name; /* option name */
    bool is_alias; /* we consider --enable|-en|-e as option and --enable, -en, -e as alias */
    bool is_set;   /* whether this option is set */
  public:
    option():required(false), multiple(false), equal(false), append(false), is_set(false), is_alias(false) {};
  public:
    class Builder {
    private:
      argval val;
      std::string help;
      bool required;
      bool multiple;
      bool equal;
      bool append;
      option* opt;
    public:
      Builder():required(false), multiple(false), equal(false), append(false), opt(new option()) {};
      virtual ~Builder() { delete opt; }
      Builder& set_help(std::string _help) { help = _help; return *this; }
      Builder& set_opt_type(OPTTP _type) { val.tp = _type; return *this; }
      Builder& set_default_bvalue(bool _bval) { val.bval = _bval; return *this; } /* function overload bool has high prior level */
      Builder& set_default_vvalue(std::vector<std::string> _vval) { val.vval = _vval; return *this; }
      Builder& set_required(bool _required) { required = _required; return *this; }
      Builder& set_multiple(bool _multiple) { multiple = _multiple; return *this; }
      Builder& set_equal(bool _equal) { equal = _equal; return *this; }
      Builder& set_append(bool _append) { equal = _append; return *this; }
      Builder& clear() { val.clear(); help.clear(); required = false; multiple = false; equal = false; append = false; return *this; }
      option* build() {
        opt->val = val;
        opt->help = help;
        opt->required = required;
        opt->multiple = multiple;
        opt->equal = equal;
        opt->append = append;
        return opt;
      }
    };
  } option;
public:
  ArgParser() {

  }

  ArgParser(ArgParser &paser) {

  }

  ArgParser(int nargs) {

  }


  virtual ~ArgParser() {

  }

public:

  ArgParser& addOpt(std::string name, option tp) {
    std::regex invalid_ptn("[^a-zA-Z_| \\-]");
    if (std::regex_search(name, invalid_ptn)) {
      std::cout << "invalid option...\n";
      return *this;
    }


    tp.name = name;
    m_opts[name] = tp;

    if (name.find('|') != std::string::npos) {
      std::smatch m;
      std::regex e("\\-{1,2}[a-zA-Z]+(?=\\||$)");   /* --file|-f  -> --file -f */
      while (std::regex_search(name, m, e)) {
        for (auto const alias : m) {
          // std::cout << x << " ";
          tp.is_alias = true;
          // std::cout << alias << std::endl;
          m_opts[alias] = tp;
        }
        // std::cout << std::endl;
        name = m.suffix().str();
      }
    }
    return *this;
  }

  ArgParser& setf(std::function<bool(std::map<std::string, option>&)> f) {
    m_f = f;
    return *this;
  }



private:
  enum class STATUS {
    INIT,
    BLANK,
    CHAR,
    ESCAPE,  // escape character, only works in double quote
    SQSTART, // single quote start
    SQEND,   // single quote end
    DQSTART, // double quote start
    DQEND    // double quote end
  };
private:
  /*
   * status machine
   */
  bool _lex(std::string text) {
    int len = 0;
    // std::string item(m_arg_maxlen);
    STATUS status = STATUS::INIT;
    for (int i = 0; i <= text.size(); ++i) {
      if (i == text.size()) {
        if (status == STATUS::SQSTART || status == STATUS::DQSTART || status == STATUS::ESCAPE) {
          std::cout << "missing single/double quote." << std::endl;
        }
        else {
          if (len > 0) {
            m_args.push_back(text.substr(i - len, len));
            len = 0;
          }
        }
        break;
      }

      if (text[i] == '\'') {
        if (status == STATUS::BLANK || status == STATUS::INIT) {
          status = STATUS::SQSTART;
          len = 0;
        }
        else if (status == STATUS::SQSTART) {
          status = STATUS::SQEND;
          m_args.push_back(text.substr(i - len, len));
          len = 0;
        }
        else if (status == STATUS::ESCAPE) {
          status = STATUS::DQSTART;
          len += 1;
        }
        else {
          len += 1;
        }

      }
      else if (text[i] == '\"') {
        if (status == STATUS::BLANK || status == STATUS::INIT) {
          status = STATUS::DQSTART;
          len = 0;
        }
        else if (status == STATUS::ESCAPE) {
          status = STATUS::DQSTART;
          len += 1;
        }
        else if (status == STATUS::DQSTART) {
          status = STATUS::DQEND;
          m_args.push_back(text.substr(i - len, len));
          len = 0;
        }
        else {
          len += 1;
        }
      }
      else if (text[i] == '\\') {
        if (status == STATUS::DQSTART) {
          status = STATUS::ESCAPE;
        }
        else if (status == STATUS::ESCAPE) {
          status = STATUS::DQSTART;
        }
        else {

        }
        len += 1;
      }
      else if (text[i] == ' ') {
        if (status == STATUS::SQSTART || status == STATUS::DQSTART) {
          len += 1;
        }
        else {
          status = STATUS::BLANK;
          if (len > 0) {
            m_args.push_back(text.substr(i - len, len));
            len = 0;
          }
        }
      }
      else {
        if (status == STATUS::INIT || status == STATUS::BLANK)
          status = STATUS::CHAR;
        else if (status == STATUS::ESCAPE) {
          status = STATUS::DQSTART;
        }
        else if (status == STATUS::SQEND || status == STATUS::DQEND) {
          std::cout << "only blank is allowed to follow the sing/double quote end." << std::endl;
          return false;
        }
        else {

        }
        len += 1;
      }
    }
    return true;
  }
  bool _parse() {
    option* opt = nullptr;
    std::vector < std::string>::iterator it_arg = m_args.begin();
    ++it_arg; /* skip the command */
    std::regex opt_ptn("^\\-.*");
    for (; it_arg != m_args.end();) {
      if (!std::regex_search(*it_arg, opt_ptn)) {
        std::cout << *it_arg << ":\t" << "unkonwn input." << std::endl;
        return false;
      }
      else {
        // known option
        if (m_opts.find(*it_arg) != m_opts.end()) {

          opt = &m_opts[*it_arg];
          if (opt->is_alias) {
            opt = &m_opts[opt->name];
          }

          opt->is_set = true;

          switch (opt->val.tp) {
          case OPTTP::BOOL:
            opt->val.bval = !opt->val.bval;
            ++it_arg;
            break;
          case OPTTP::CHOICE:
            ++it_arg;
            if (std::find(opt->val.vval.begin(), opt->val.vval.end(), *it_arg) == opt->val.vval.end()) {
              std::cout << *it_arg << ":\t" << "this value is not avalid." << std::endl;
              std::cout << "The value must be one of the below:" << std::endl;
              for (auto& item : opt->val.vval) {
                std::cout << item << std::endl;
              }
              return false;
            }
            opt->val.vval[0] = *it_arg;
            ++it_arg;
            break;
          case OPTTP::STRING:
          case OPTTP::FILE:
          case OPTTP::PATH:
          default:
            ++it_arg;
            opt->val.vval.clear();
            opt->val.vval.push_back(*it_arg);
            ++it_arg;
            break;
          }
        }
        else {
          // TODO ...
          ++it_arg;
        }
      }

    }
    return _check_required();
  };
  bool _check_required() {
    for (auto& item : m_opts) {
      if (!item.second.is_alias && item.second.required && !item.second.is_set) {
        return false;
      }
    }
    return true;
  }

public:

  bool parse(std::string text) {
    if (_lex(text)) {
      if (_parse()) {
        return m_f(m_opts);
      }
    }
    return false;
  }

  bool parse(int nargs, char* argv[]) {
    for (int i = 0; i < nargs; ++i) {
      m_args.push_back(argv[i]);
    }
    if (_parse()) {
      return m_f(m_opts);
    }
    return false;
  }

public:
  void print_args() {
    for (auto &it : m_args) {
      std::cout << it << std::endl;
    }
  }
  void print_opts() {
    std::cout.flags(std::ios::left);
    std::cout << std::setw(32) <<"OPTION" << std::setw(16) <<"TYPE" << std::setw(32) <<"DEFAULT_VALUE" << std::setw(64) <<"HELP" << std::endl;
    for (auto &it : m_opts) {
      if(!it.second.is_alias)
        std::cout << std::setw(32) << it.first << std::setw(32) << it.second.val.bval << std::setw(64) <<it.second.help << std::endl;
    }
  }

protected:
  /*
   * check whether the arg name is legal
   */
  bool check(std::string arg) {

  }

private:
  // save all args
  std::map<std::string, option> m_opts;
  // save fun that handle args
  std::function<bool(std::map<std::string, option>&)> m_f;
  // save input
  std::vector<std::string> m_args;
private:
  int m_arg_maxlen = 265;

};

#endif // __ARGPARSER_H__
