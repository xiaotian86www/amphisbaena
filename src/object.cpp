#include "object.hpp"

#include <sstream>

namespace translator
{
    void ObjectFactory::registe(std::string_view name, ctor_func_type &&func)
    {
        ctors_[name] = std::move(func);
    }

    ObjectPtr ObjectFactory::produce(Environment *env, std::string_view name)
    {
        auto it = ctors_.find(name);
        if (it != ctors_.end())
        {
            return it->second(env);
        }

        std::stringstream ss;
        ss << "cannot find object '" << name << "'";

        throw new std::runtime_error(ss.str());
    }
} // namespace translator
