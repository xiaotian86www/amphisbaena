#pragma once

template <typename ObjectType>
class Object
{
public:
    Object(ObjectType &obj)
        : obj_(obj) {}

public:
    template <typename ValueType_>
    ValueType_ get_value(const char *name, ValueType_ default_value) const;

    template <typename ValueType_>
    void set_value(const char *name, ValueType_ value);

    template <typename GroupType_>
    GroupType_ &get_group(const char *name);

    template <typename GroupType_>
    const GroupType_ &get_group(const char *name) const;

private:
    ObjectType &obj_;
};

template <typename GroupType_>
class Group
{
public:
    Group(GroupType_ & group)
        : group_(group) {}

public:
    template <typename ObjectType_>
    ObjectType_ &at(std::size_t index);

    template <typename ObjectType_>
    const ObjectType_ &at(std::size_t index) const;

    std::size_t get_size() const;

    template <typename IterType_>
    IterType_ begin();

    template <typename IterType_>
    IterType_ end();

    template <typename ConstIterType_>
    ConstIterType_ begin() const;

    template <typename ConstIterType_>
    ConstIterType_ end() const;

private:
    GroupType_ &group_;
};

