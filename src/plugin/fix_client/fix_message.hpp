#pragma once

#include <exception>
#include <map>
#include <memory>
#include <stdexcept>
#include <string_view>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#pragma GCC diagnostic ignored "-Wdeprecated-copy"
#pragma GCC diagnostic ignored "-Wdeprecated-copy-with-user-provided-copy"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#include <quickfix/DataDictionary.h>
#include <quickfix/Field.h>
#include <quickfix/FieldMap.h>
#include <quickfix/FieldTypes.h>
#include <quickfix/FixFields.h>
#include <quickfix/Message.h>
#pragma GCC diagnostic pop

#include "message.hpp"

namespace amphisbaena {

class FixObject : public Object
{
public:
  class ConstIterator : public Object::ConstIterator
  {
  public:
    ConstIterator() = default;

    ConstIterator(FIX::FieldMap::const_iterator it);

  public:
    std::string_view get_name() override;

    FieldType get_type() override;

    int32_t get_int() override;

    std::string_view get_string() override;

    double get_double() override;

    bool operator!=(const Object::ConstIterator& right) override;

    Object::ConstIterator& operator++() override;

  private:
    FIX::FieldMap::const_iterator it_;
  };

public:
  FixObject(FIX::FieldMap& fields);

public:
  int32_t get_value(std::string_view name,
                    int32_t default_value) const override;

  std::string_view get_value(std::string_view name,
                             std::string_view default_value) const override;

  double get_value(std::string_view name, double default_value) const override;

  int32_t get_int(std::string_view name) const override;

  std::string_view get_string(std::string_view name) const override;

  double get_double(std::string_view name) const override;

  Object* set_value(std::string_view name, int32_t value) override;

  Object* set_value(std::string_view name, std::string_view value) override;

  Object* set_value(std::string_view name, double value) override;

  std::size_t count() const override;

  ConstIteratorWrap begin() const override;

  ConstIteratorWrap end() const override;

  ObjectPtr get_object(std::string_view name) override;

  ConstObjectPtr get_object(std::string_view name) const override;

  ObjectPtr get_or_set_object(std::string_view name) override;

  GroupPtr get_group(std::string_view name) override;

  const GroupPtr get_group(std::string_view name) const override;

public:
  void from_string(std::string_view str) override;

  std::string to_string() const override;

private:
  template<typename Field_, typename Value_>
  Value_ get_value(std::string_view name, Value_ default_value) const;

  template<typename Field_, typename Value_>
  Value_ get_value(std::string_view name) const;

  template<typename Field_, typename Value_>
  Object* set_value(std::string_view name, Value_ value);

private:
  FIX::FieldMap& fields_;
};

class FixMessage : public Message
{
public:
  FixMessage();

  FixMessage(const FIX::Message& message);

  FixMessage(const FixMessage& right);

  MessagePtr clone() const override;

public:
  ObjectPtr get_head() override;

  ConstObjectPtr get_head() const override;

  ObjectPtr get_body() override;

  ConstObjectPtr get_body() const override;

  ObjectPtr get_tail() override;

  ConstObjectPtr get_tail() const override;

  void clear() override;

public:
  FIX::Message fix_message;
};

typedef std::shared_ptr<FixMessage> FixMessagePtr;

class FixMessageFactory : public MessageFactory
{
public:
  MessagePtr create() override;

  std::string_view name() override;
};
}