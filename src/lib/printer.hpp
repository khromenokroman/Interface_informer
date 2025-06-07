#pragma once

#include <netlink/cache.h>
#include <netlink/netlink.h>
#include <netlink/route/addr.h>
#include <netlink/route/link.h>
#include <netlink/socket.h>

#include <iomanip>
#include <memory>
#include <string>

#include "informer/interface_informer.hpp"

namespace os::network {

struct Neigh {
    std::string ip{};
    std::string mac{};
    std::vector<std::string> type{};
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Neigh, ip, mac, type);

struct Routes {
    std::string destination{};
    std::string gateway{};
    uint32_t metric{};
    uint32_t table{};
    std::string type{};
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Routes, destination, gateway, metric, table, type);

struct Ip {
    std::string type{};
    std::string ip{};
    int masc{};
    std::vector<std::string> flags{};
    uint32_t valid_lft{};
    uint32_t pref_lft{};
    std::string broadcast{};
    std::string peer{};
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Ip, type, ip, masc, flags, valid_lft, pref_lft, broadcast, peer);

struct Protocols {
    bool routing_ipv4{};
    bool multicast{};
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Protocols, routing_ipv4, multicast);

struct OperationalStatus {
    std::string oper_state{};
    std::string link_mode{};
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(OperationalStatus, oper_state, link_mode);

struct HW {
    std::string type{};
    std::vector<std::string> mac{};
    uint64_t mtu{};
    uint64_t size_queue{};
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(HW, type, mac, mtu, size_queue);

struct General {
    int index{};
    std::string state{};
    std::string type{};
    std::vector<std::string> flags{};
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(General, index, state, type, flags);

struct Json {
    std::string interface{};
    General general{};
    HW hw{};
    OperationalStatus operational_status{};
    Protocols protocols{};
    std::vector<Ip> ip{};
    std::vector<Routes> routes{};
    std::vector<Neigh> neigh{};
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Json, interface, general, hw, operational_status, protocols, ip, routes, neigh);

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

class ShowInfoInterface final : public InformerNetlink {
   public:
    ShowInfoInterface();
    ~ShowInfoInterface() override = default;

    ShowInfoInterface(ShowInfoInterface const &) = delete;
    ShowInfoInterface(ShowInfoInterface &&) = delete;
    ShowInfoInterface &operator=(ShowInfoInterface const &) = delete;
    ShowInfoInterface &operator=(ShowInfoInterface &&) = delete;

    ::nlohmann::json get_interface_info(std::string const &interface_name) override;
    ::nlohmann::json get_all_interfaces() override;

   private:
    static std::string arp_hrd_type_to_string(unsigned int type);
    void print_interface_details(rtnl_link *link);
    void print_address_info(rtnl_addr *addr);
    void print_neighbour_info(int ifindex);
    void print_routes_for_interface(int ifindex);

    void show();

    /**
     * @brief Показывает информацию только для указанного интерфейса
     * @param interface_name Имя интерфейса
     * @throw exceptions::InterfaceNotFound если интерфейс не найден
     */
    void showInterface(const std::string &interface_name);

    /**
     * @brief Получает индекс интерфейса по его имени
     * @param interface_name Имя интерфейса
     * @return Индекс интерфейса
     * @throw exceptions::InterfaceNotFound если интерфейс не найден
     */
    [[nodiscard]] int getInterfaceIndex(const std::string &interface_name) const;

    /**
     * @brief Показать детальную информацию о конкретном интерфейсе
     * @param ifindex Индекс интерфейса
     */
    void showInterfaceByIndex(int ifindex);

    Json m_json{};
    std::unique_ptr<nl_sock, decltype(&nl_socket_free)> m_netlink_socket; // 8
    std::unique_ptr<nl_cache, decltype(&nl_cache_free)> m_link_data;      // 8
    std::unique_ptr<nl_cache, decltype(&nl_cache_free)> m_addr_data;      // 8
    std::unique_ptr<nl_cache, decltype(&nl_cache_free)> m_route_data;     // 8
    std::unique_ptr<nl_cache, decltype(&nl_cache_free)> m_neigh_data;     // 8
};

} // namespace os::network
