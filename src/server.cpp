#include "server.hpp"

namespace translator
{
    void ServerPool::add(std::string_view name, std::shared_ptr<Server> server)
    {
        pool[name] = server;
    }

    Server *ServerPool::get(std::string_view name)
    {
        auto it = pool.find(name);
        if (it != pool.end())
            return it->second.get();

        std::stringstream ss;
        ss << "cannot find server '" << name << "'";

        throw new std::runtime_error(ss.str());
    }

} // namespace translator
