#pragma once

#include <filesystem>
#include <fstream>
#include <istream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <variant>

namespace omfl {
class Section {
 public:
  Section operator[](int i) {
    if(i >= std::get<std::vector <Section>>(vval).size()){
      Section Temp;
      Temp.value_type = no_type;
      return Temp;
    }
    return std::get<std::vector <Section>>(vval)[i];
  }
  bool IsArray() {
    return value_type == vval_type;
  }
  bool IsInt() {
    return value_type == ival_type;
  }
  bool IsBool() {
    return value_type == bval_type;
  }
  bool IsString() {
    return value_type == sval_type;
  }
  bool IsFloat() {
    return value_type == fval_type;
  }
  int32_t AsInt() {
    return std::get<int32_t>(ival);
  }
  bool AsBool() {
    return std::get<bool>(bval);
  }
  std::string AsString() {
    return std::get<std::string>(sval);
  }
  float AsFloat() {
    return std::get<float>(fval);
  }
  int32_t AsIntOrDefault(int32_t default_value) {
    if (value_type == ival_type) return std::get<int32_t>(ival);
    else return default_value;
  }
  bool AsBoolOrDefault(bool default_value) {
    if (value_type == bval_type) return std::get<bool>(bval);
    else return default_value;
  }
  std::string AsStringOrDefault(std::string default_value) {
    if (value_type == sval_type) return std::get<std::string>(sval);
    else return default_value;
  }
  float AsFloatOrDefault(float default_value) {
    if (value_type == fval_type) return std::get<float>(fval);
    else return default_value;
  }
  bool valid() {
    return parsing_status;
  }
  Section& Get(const std::string& path, int ind=0){
    size_t str_len = path.length();
    std::string key;
    while(ind < str_len) {
      if(path[ind] == '.'){
        return values[key].Get(path, ind+1);
        key = "";
      } else {
        key += path[ind];
      }
      ind++;
    }
    return values[key];
  }
  friend Section parse(const std::filesystem::path& path);
  friend Section parse(const std::string& str);
  friend Section GetValue(const std::string& line, size_t& now_index);
 private:
  enum Value_Type {
    ival_type,
    fval_type,
    sval_type,
    bval_type,
    vval_type,
    no_type
  };
  Value_Type value_type = no_type;
  std::variant<int32_t, float, std::string, bool, std::vector<Section>> ival,
      fval, sval, bval, vval;
  std::unordered_map<std::string, Section> values;
  bool parsing_status = true;
};
}// namespace omfl