#pragma once
#include <fcntl.h>
#include <fmt/format.h>

#include <nlohmann/json.hpp>

namespace os::network {
namespace exceptions {
struct NetNamespaceHandlerEx : std::runtime_error {
    using std::runtime_error::runtime_error;
};
struct OpenNamespace final : NetNamespaceHandlerEx {
    using NetNamespaceHandlerEx::NetNamespaceHandlerEx;
};
struct SwitchNamespace final : NetNamespaceHandlerEx {
    using NetNamespaceHandlerEx::NetNamespaceHandlerEx;
};
} // namespace exceptions
class InformerNetlink {
   public:
    InformerNetlink() = default;
    virtual ~InformerNetlink() = default;

    InformerNetlink(InformerNetlink const &) = delete;
    InformerNetlink(InformerNetlink &&) = delete;
    InformerNetlink &operator=(InformerNetlink const &) = delete;
    InformerNetlink &operator=(InformerNetlink &&) = delete;

    virtual ::nlohmann::json get_interface_info(std::string const &interface_name, std::string const &ns) = 0;

    static void switch_to_namespace(const std::string &name);

    static std::unique_ptr<InformerNetlink> create();

   private:
    static InformerNetlink *create(char *error_message) noexcept;
    static constexpr std::size_t M_MAX_BUFFER_SIZE = 1024;
};

inline std::unique_ptr<InformerNetlink> InformerNetlink::create() {
    auto const message_error = std::make_unique<char[]>(M_MAX_BUFFER_SIZE);

    InformerNetlink *new_object = create(message_error.get());
    if (new_object == nullptr) {
        throw std::runtime_error(message_error.get());
    }

    return std::unique_ptr<InformerNetlink>{new_object};
}
inline void InformerNetlink::switch_to_namespace(const std::string &name) {
    std::string const nsPath = "/var/run/netns/" + name;
    int const fd = open(nsPath.c_str(), O_RDONLY);

    if (fd < 0) {
        throw exceptions::OpenNamespace(::fmt::format("Open namespace {}", name));
    }

    bool const result = (setns(fd, CLONE_NEWNET) == 0);
    close(fd);

    if (!result) {
        throw exceptions::SwitchNamespace(::fmt::format("Switch namespace {}", name));
    }
}
} // namespace os::network