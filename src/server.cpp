// #include "server.hpp"

// namespace translator {
// void
// ServerPool::add(std::string_view name, std::shared_ptr<Server> server)
// {
//   servers_[name] = server;
// }

// Server*
// ServerPool::get(std::string_view name) const
// {
//   auto it = servers_.find(name);
//   if (it != servers_.end())
//     return it->second.get();

//   std::stringstream ss;
//   ss << "cannot find server '" << name << "'";

//   throw new std::runtime_error(ss.str());
// }

// } // namespace translator
