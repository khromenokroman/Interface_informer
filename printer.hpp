#pragma once

#include <netlink/cache.h>
#include <netlink/netlink.h>
#include <netlink/route/addr.h>
#include <netlink/route/link.h>
#include <netlink/socket.h>

#include <iomanip>
#include <memory>
#include <string>

namespace os::network {
namespace exceptions {
struct NetlinkEx : std::runtime_error {
    using std::runtime_error::runtime_error;
};
struct AllocateSocket final : NetlinkEx {
    using NetlinkEx::NetlinkEx;
};
struct ConnectNetlinkRoute final : NetlinkEx {
    using NetlinkEx::NetlinkEx;
};
struct GetDataLinks final : NetlinkEx {
    using NetlinkEx::NetlinkEx;
};
struct GetDataAddr final : NetlinkEx {
    using NetlinkEx::NetlinkEx;
};
struct GetDataRoute final : NetlinkEx {
    using NetlinkEx::NetlinkEx;
};
struct GetDataNeigh final : NetlinkEx {
    using NetlinkEx::NetlinkEx;
};
struct InterfaceNotFound final : NetlinkEx {
    using NetlinkEx::NetlinkEx;
};
} // namespace exceptions

class ShowInfoInterface {
   public:
    ShowInfoInterface();
    ~ShowInfoInterface() = default;

    ShowInfoInterface(ShowInfoInterface const &) = delete;
    ShowInfoInterface(ShowInfoInterface &&) = delete;
    ShowInfoInterface &operator=(ShowInfoInterface const &) = delete;
    ShowInfoInterface &operator=(ShowInfoInterface &&) = delete;

    static std::string arp_hrd_type_to_string(unsigned int type);
    static std::string format_size(uint64_t bytes);
    static void print_interface_details(rtnl_link *link);
    static void print_address_info(rtnl_addr *addr);
    void print_neighbour_info(int ifindex) const;
    void print_routes_for_interface(int ifindex) const;
    void show() const;

    /**
     * @brief Показывает информацию только для указанного интерфейса
     * @param interface_name Имя интерфейса
     * @throw exceptions::InterfaceNotFound если интерфейс не найден
     */
    void showInterface(const std::string& interface_name) const;

    /**
     * @brief Получает индекс интерфейса по его имени
     * @param interface_name Имя интерфейса
     * @return Индекс интерфейса
     * @throw exceptions::InterfaceNotFound если интерфейс не найден
     */
    [[nodiscard]] int getInterfaceIndex(const std::string& interface_name) const;

    /**
     * @brief Показать детальную информацию о конкретном интерфейсе
     * @param ifindex Индекс интерфейса
     */
    void showInterfaceByIndex(int ifindex) const;

   private:
    std::unique_ptr<nl_sock, decltype(&nl_socket_free)> m_netlink_socket; // 8
    std::unique_ptr<nl_cache, decltype(&nl_cache_free)> m_link_data;      // 8
    std::unique_ptr<nl_cache, decltype(&nl_cache_free)> m_addr_data;      // 8
    std::unique_ptr<nl_cache, decltype(&nl_cache_free)> m_route_data;     // 8
    std::unique_ptr<nl_cache, decltype(&nl_cache_free)> m_neigh_data;     // 8
};
} // namespace os::network
