#include "switch_ns.hpp"

namespace os::ns {

NetNamespaceHandler::NetNamespaceHandler() {
    m_original_ns_fd = open("/proc/self/ns/net", O_RDONLY);
    if (m_original_ns_fd < 0) {
        throw exceptions::OpenNamespace("Open /proc/self/ns/net");
    }
}
NetNamespaceHandler::~NetNamespaceHandler() {
    if (m_original_ns_fd >= 0) {
        setns(m_original_ns_fd, CLONE_NEWNET);
        close(m_original_ns_fd);
    }
}
std::vector<std::string> NetNamespaceHandler::getNetworkNamespaces() {
    std::vector<std::string> namespaces;

    if (auto const dir = opendir("/var/run/netns")) {
        dirent *entry = nullptr;
        while ((entry = readdir(dir)) != nullptr) {
            if (entry->d_name[0] != '.') {
                namespaces.emplace_back(entry->d_name);
            }
        }
        closedir(dir);
    }

    return namespaces;
}
void NetNamespaceHandler::switchToNamespace(const std::string &name) {
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
void NetNamespaceHandler::switchBackToOriginal() const {
    if (auto const result = (setns(m_original_ns_fd, CLONE_NEWNET) == 0); !result) {
        throw exceptions::SwitchNamespace("Switch original namespace");
    }
}
} // namespace os::ns
