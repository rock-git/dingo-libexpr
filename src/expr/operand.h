// Copyright (c) 2023 dingodb.com, Inc. All Rights Reserved
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef _EXPR_OPERAND_H_
#define _EXPR_OPERAND_H_

#include <any>
#include <cstdint>
#include <memory>
#include <optional>
#include <unordered_map>
#include <variant>
#include <vector>

#include "expr/expr_string.h"
#include "types.h"

namespace dingodb::expr {

class Operand {
 public:
  template <typename T>
  Operand(T v) : m_data(v) {
  }

  Operand(String::ValueType v) : m_data(String(v)) {
  }

  Operand([[maybe_unused]] std::nullptr_t v) {
  }

  Operand() = default;

  bool operator==(const Operand &v) const {
    return m_data == v.m_data;
  }

  bool operator==([[maybe_unused]] std::nullptr_t v) const {
    return std::holds_alternative<std::monostate>(m_data);
  }

  bool operator!=(const Operand &v) const {
    return m_data != v.m_data;
  }

  bool operator!=([[maybe_unused]] std::nullptr_t v) const {
    return !std::holds_alternative<std::monostate>(m_data);
  }

  template <typename T>
  inline T GetValue() const {
    return std::get<T>(m_data);
  }

 private:
  std::variant<
      std::monostate,
      int32_t,
      int64_t,
      bool,
      float,
      double,
      String,
      std::shared_ptr<std::vector<int32_t>>,
      std::shared_ptr<std::vector<int64_t>>,
      std::shared_ptr<std::vector<bool>>,
      std::shared_ptr<std::vector<float>>,
      std::shared_ptr<std::vector<double>>,
      std::shared_ptr<std::vector<std::string>>>
      m_data;

  friend class std::hash<::dingodb::expr::Operand>;

  friend std::ostream &operator<<(std::ostream &os, const Operand &v);
};

using Tuple = std::vector<Operand>;

namespace any_optional_data_adaptor {

template <typename T>
Operand ToOperand(const std::any &v) {
  if (!v.has_value()) {
    return nullptr;
  }

  const auto &origin_value = std::any_cast<const T &>(v);
  if constexpr (std::is_same_v<T, std::vector<int32_t>> || std::is_same_v<T, std::vector<int64_t>> || std::is_same_v<T, std::vector<bool>> || std::is_same_v<T, std::vector<float>> || std::is_same_v<T, std::vector<double>> || std::is_same_v<T, std::vector<std::string>>) {
    return std::make_shared<T>(origin_value);
  } else {
    return origin_value;
  }
}

template <typename T>
std::any FromOperand(const Operand &v) {
  if constexpr (std::is_same_v<T, std::vector<int32_t>> || std::is_same_v<T, std::vector<int64_t>> || std::is_same_v<T, std::vector<bool>> || std::is_same_v<T, std::vector<float>> || std::is_same_v<T, std::vector<double>> || std::is_same_v<T, std::vector<std::string>>) {
    return v != nullptr ? std::make_any<T>(*v.GetValue<std::shared_ptr<T>>()) : std::any();
  } else if constexpr (std::is_same_v<T, std::string>) {
    return v != nullptr ? std::make_any<std::string>(*v.GetValue<String>().GetPtr()) : std::any();
  } else {
    return v != nullptr ? std::make_any<T>(v.GetValue<T>()) : std::any();
  }
}

// GCC does not allow template specialization in class, so we need this.
template <>
std::any FromOperand<String::ValueType>(const Operand &v);

}  // namespace any_optional_data_adaptor

}  // namespace dingodb::expr

namespace std {

template <>
struct hash<::dingodb::expr::Operand> {
  size_t operator()(const ::dingodb::expr::Operand &val) const noexcept {
    return hash<decltype(val.m_data)>()(val.m_data);
  }
};

template <>
struct hash<::dingodb::expr::Tuple> {
  size_t operator()(const ::dingodb::expr::Tuple &val) const noexcept {
    size_t h = 0LL;
    for (const auto &v : val) {
      h *= 31LL;
      h += hash<::dingodb::expr::Operand>()(v);
    }
    return h;
  }
};

}  // namespace std

#endif /* _EXPR_OPERAND_H_ */
