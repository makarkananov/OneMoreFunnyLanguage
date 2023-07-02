#include "parser.h"

namespace omfl {

Section GetValue(const std::string& line, size_t& now_index) {
  Section Temp;
  std::string to_add = "";
  size_t str_len = line.length();
  while(isspace(line[str_len-1])) str_len--;
  if (now_index < str_len && line[now_index] == ',') now_index++;
  while (now_index < str_len && isspace(line[now_index])) {
    now_index++;
  }
  if (now_index < str_len && line[now_index] == '[') {
    Temp.value_type = Section::vval_type;
    std::vector<Section> vec_to_add;
    now_index++;
    bool array_is_closed = false;
    while (now_index < str_len) {
      if(line[now_index] == ']'){
        array_is_closed = true;
        break;
      }
      if(line[now_index] == ','){
        now_index++;
      }
      while(now_index < str_len && isspace(line[now_index])) {
        now_index++;
      }
      Section TempValue = GetValue(line, now_index);
      if(TempValue.value_type == Section::no_type){
        Temp.value_type = Section::no_type;
        return Temp;
      }
      vec_to_add.push_back(TempValue);
      now_index++;
    }
    if(!array_is_closed){
      Temp.value_type = Section::no_type;
      return Temp;
    }
    Temp.vval = vec_to_add;
  } else {
    bool is_in_string = false;
    bool in_comment = false;
    bool in_spaces = false;
    while (now_index < str_len) {
      if (line[now_index] == '"') is_in_string = !is_in_string;
      if (line[now_index] == '#' && !is_in_string) in_comment = true;
      if (now_index < str_len && isspace(line[now_index]) && !is_in_string) {
        if (!(now_index + 1 == str_len
            || ((line[now_index + 1] == ',' || line[now_index + 1] == ']' || in_comment)))) {
          in_spaces = true;
          now_index++;
          continue;
        }
      }
      if(!in_comment && !isspace(line[now_index])) {
        to_add += line[now_index];
        if(in_spaces) {
          Temp.value_type = Section::no_type;
          return Temp;
        }
      }
      if ((now_index + 1 == str_len
          || ((line[now_index + 1] == ',' || line[now_index + 1] == ']' || in_comment) && !is_in_string))) {
        if (to_add == "true") { //trying to parse as bool true
          Temp.value_type = Section::bval_type;
          Temp.bval = true;
        } else if (to_add == "false") { //trying to parse as bool false
          Temp.value_type = Section::bval_type;
          Temp.bval = false;
        } else if (to_add[0] == '"' && to_add[to_add.length() - 1] == '"') { //trying to parse as string
          size_t to_add_length = to_add.length();
          for(int i = 1; i < to_add_length - 1; ++i){
            if(to_add[i] == '"'){
              Temp.value_type = Section::no_type;
              return Temp;
            }
          }
          Temp.value_type = Section::sval_type;
          Temp.sval = to_add.substr(1, to_add.length() - 2);
        } else if (to_add.find(".") != std::string::npos) { //trying to parse as float
          size_t to_add_length = to_add.length();
          for(int i = 0; i < to_add_length; ++i){
            if(!('0' <= to_add[i] && to_add[i] <= '9' ||  to_add[i] == '.') && !(i == 0 && (to_add[i] == '-' ||  to_add[i] == '+')) || (to_add[i] == '.' && (i + 1 >= to_add_length || i - 1 < 0 || to_add[i - 1] > '9' || to_add[i - 1] < '0'))){
              Temp.value_type = Section::no_type;
              return Temp;
            }
          }
          Temp.value_type = Section::fval_type;
          Temp.fval = std::stof(to_add);
        } else { //trying to parse as int
          size_t to_add_length = to_add.length();
          for(int i = 0; i < to_add_length; ++i){
            if(!('0' <= to_add[i] && to_add[i] <= '9' || to_add[i] == '-' || to_add[i] == '+') || ((i != 0 || to_add_length==1) && (to_add[i] == '-' || to_add[i] == '+'))){
              Temp.value_type = Section::no_type;
              return Temp;
            }
          }
          Temp.value_type = Section::ival_type;
          Temp.ival = std::stoi(to_add);
        }
        break;
      }
      now_index++;
    }
  }
  return Temp;
}

Section parse(const std::string& str) {
  Section result;
  std::stringstream stream;
  stream << str;
  const std::string valid_key_symbols = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
  Section* NowSection = &result;
  std::string line;
  while (std::getline(stream, line)) {
    size_t now_index = 0;
    size_t str_len = line.length();
    while (now_index < str_len && isspace(line[now_index])) {
      now_index++;
    }
    if (now_index == str_len) continue; //blank line
    if (line[now_index] == '#') continue; // comment only line
    if (line.find("=") != std::string::npos) { //assignment
      std::string key;
      while (now_index < str_len && line[now_index] != '=' && !isspace(line[now_index])) {
        if (valid_key_symbols.find_first_of(line[now_index]) != std::string::npos) key += line[now_index];
        else {
          result.parsing_status = false;
          return result;
        }
        now_index++;
      }
      now_index++;
      while (now_index < str_len && (isspace(line[now_index]) || line[now_index] == '=')) {
        now_index++;
      }
        Section Value = GetValue(line, now_index);
        if (NowSection->values.count(key) > 0 || Value.value_type == Section::no_type || key.length() == 0) {
          result.parsing_status = false;
          return result;
        }
        NowSection->values[key] = Value;
    } else if (line[now_index] == '[') { //section declaration
      NowSection = &result;
      std::string section_name = "";
      for (size_t i = now_index + 1; i < str_len; ++i) {
        if (isspace(line[i])) continue;
        if (line[i] == '.') {
          if(section_name.length() == 0) result.parsing_status = false;
          NowSection = &NowSection->Get(section_name);
          section_name = "";
        } else if (line[i] == ']') {
          if(section_name.length() == 0) result.parsing_status = false;
          if (NowSection->values.count(section_name) == 0) {
            NowSection->values[section_name] = Section();
          }
          NowSection = &NowSection->Get(section_name);
        } else if (valid_key_symbols.find_first_of(line[i]) != std::string::npos) {
          section_name += line[i];
        } else {
          result.parsing_status = false;
        }
      }
    } else {
      result.parsing_status = false;
    }
  }
  return result;
}
Section parse(const std::filesystem::path& path){
  std::ifstream f(path);
  f.seekg(0, std::ios::end);
  size_t size = f.tellg();
  std::string s(size, ' ');
  f.seekg(0);
  f.read(&s[0], size);
  return parse(s);
}


}



