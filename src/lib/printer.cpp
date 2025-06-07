#include "printer.hpp"

#include <arpa/inet.h>
#include <dirent.h>
#include <fcntl.h>
#include <fmt/format.h>
#include <linux/if_arp.h>
#include <netlink/route/neighbour.h>
#include <netlink/route/route.h>

#include <iostream>

namespace os::network {

InformerNetlink *InformerNetlink::create(char *error_message) noexcept {
    try {
        auto *new_object = new ShowInfoInterface();
        return new_object;
    } catch (std::exception const &ex) {
        ::snprintf(error_message, M_MAX_BUFFER_SIZE, "Cannot create new object %s", ex.what());
        return nullptr;
    }
}

ShowInfoInterface::ShowInfoInterface()
    : m_netlink_socket{nl_socket_alloc(), nl_socket_free},
      m_link_data{nullptr, nl_cache_free},
      m_addr_data{nullptr, nl_cache_free},
      m_route_data{nullptr, nl_cache_free},
      m_neigh_data{nullptr, nl_cache_free} {
    if (!m_netlink_socket) {
        throw exceptions::AllocateSocket("Allocate netlink socket");
    }

    if (nl_connect(m_netlink_socket.get(), NETLINK_ROUTE) < 0) {
        throw exceptions::ConnectNetlinkRoute("Connect to NETLINK_ROUTE");
    }

    auto tmp_link_data = m_link_data.release();
    if (rtnl_link_alloc_cache(m_netlink_socket.get(), AF_UNSPEC, &tmp_link_data) < 0) {
        throw exceptions::GetDataLinks("Allocate link cache");
    }
    m_link_data.reset(tmp_link_data);

    auto tmp_addr_data = m_addr_data.release();
    if (rtnl_addr_alloc_cache(m_netlink_socket.get(), &tmp_addr_data) < 0) {
        throw exceptions::GetDataAddr("Allocate address cache");
    }
    m_addr_data.reset(tmp_addr_data);

    auto tmp_route_data = m_route_data.release();
    if (rtnl_route_alloc_cache(m_netlink_socket.get(), AF_UNSPEC, 0, &tmp_route_data) < 0) {
        throw exceptions::GetDataRoute("Allocate route cache");
    }
    m_route_data.reset(tmp_route_data);

    auto tmp_neigh_data = m_neigh_data.release();
    if (rtnl_neigh_alloc_cache(m_netlink_socket.get(), &tmp_neigh_data) < 0) {
        throw exceptions::GetDataNeigh("Allocate neighbour cache");
    }
    m_neigh_data.reset(tmp_neigh_data);
}
nlohmann::json ShowInfoInterface::get_interface_info(std::string const &interface_name) {
    try {
        showInterface(interface_name);

        return m_json;
    } catch (std::exception const &ex) {
        return {{"error", ex.what()}};
    }
}
nlohmann::json ShowInfoInterface::get_all_interfaces() {
    nlohmann::json json{};
    nlohmann::json interfaces = nlohmann::json::array();
    for (auto obj = nl_cache_get_first(m_link_data.get()); obj; obj = nl_cache_get_next(obj)) {
        auto const link = reinterpret_cast<struct rtnl_link *>(obj);
        std::string interface(rtnl_link_get_name(link));
        interfaces.emplace_back(std::move(interface));
    }
    json["interfaces"] = interfaces;
    return json;
}
std::string ShowInfoInterface::arp_hrd_type_to_string(unsigned int const type) {
    switch (type) {
        case ARPHRD_ETHER:
            return "Ethernet";
        case ARPHRD_LOOPBACK:
            return "Loopback";
        case ARPHRD_PPP:
            return "PPP";
        case ARPHRD_SLIP:
            return "SLIP";
        case ARPHRD_INFINIBAND:
            return "InfiniBand";
        case ARPHRD_TUNNEL:
            return "IPIP Tunnel";
        case ARPHRD_TUNNEL6:
            return "IPv6 Tunnel";
        case ARPHRD_IEEE80211:
            return "IEEE 802.11";
        case ARPHRD_IEEE1394:
            return "IEEE 1394";
        default:
            return "Неизвестный (" + std::to_string(type) + ")";
    }
}
std::string ShowInfoInterface::format_size(uint64_t const bytes) {
    const char *suffixes[] = {"B", "KB", "MB", "GB", "TB", "PB"};
    auto size = static_cast<double>(bytes);
    int suffix_index = 0;

    while (size >= 1024 && suffix_index < 5) {
        size /= 1024;
        suffix_index++;
    }

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(suffix_index > 0 ? 2 : 0) << size << " " << suffixes[suffix_index];
    return oss.str();
}
void ShowInfoInterface::print_interface_details(rtnl_link *link) {
    char if_name[IFNAMSIZ];
    snprintf(if_name, IFNAMSIZ, "%s", rtnl_link_get_name(link));

    m_json.interface = if_name;

    m_json.general.index = rtnl_link_get_ifindex(link);

    unsigned int const flags = rtnl_link_get_flags(link);
    m_json.general.state = ((flags & IFF_UP) ? "UP" : "DOWN");

    if (flags & IFF_LOOPBACK) {
        m_json.general.type = "LOOPBACK";
    } else if (flags & IFF_BROADCAST) {
        m_json.general.type = "BROADCAST";
    } else if (flags & IFF_POINTOPOINT) {
        m_json.general.type = "POINT-TO-POINT";
    } else {
        m_json.general.type = "UNKNOWN";
    }

    if (flags & IFF_UP) {
        m_json.general.flags.emplace_back("UP");
    }
    if (flags & IFF_BROADCAST) {
        m_json.general.flags.emplace_back("BROADCAST");
    }
    if (flags & IFF_DEBUG) {
        m_json.general.flags.emplace_back("DEBUG");
    }
    if (flags & IFF_LOOPBACK) {
        m_json.general.flags.emplace_back("LOOPBACK");
    }
    if (flags & IFF_POINTOPOINT) {
        m_json.general.flags.emplace_back("POINTOPOINT");
    }
    if (flags & IFF_RUNNING) {
        m_json.general.flags.emplace_back("RUNNING");
    }
    if (flags & IFF_NOARP) {
        m_json.general.flags.emplace_back("NOARP");
    }
    if (flags & IFF_PROMISC) {
        m_json.general.flags.emplace_back("PROMISC");
    }
    if (flags & IFF_ALLMULTI) {
        m_json.general.flags.emplace_back("ALLMULTI");
    }
    if (flags & IFF_MASTER) {
        m_json.general.flags.emplace_back("MASTER");
    }
    if (flags & IFF_SLAVE) {
        m_json.general.flags.emplace_back("SLAVE");
    }
    if (flags & IFF_MULTICAST) {
        m_json.general.flags.emplace_back("MULTICAST");
    }
    if (flags & IFF_PORTSEL) {
        m_json.general.flags.emplace_back("PORTSEL");
    }
    if (flags & IFF_AUTOMEDIA) {
        m_json.general.flags.emplace_back("AUTOMEDIA");
    }
    if (flags & IFF_DYNAMIC) {
        m_json.general.flags.emplace_back("DYNAMIC");
    }

    unsigned int const arp_type = rtnl_link_get_arptype(link);
    m_json.hw.type = arp_hrd_type_to_string(arp_type);

    if (auto const hw_addr = rtnl_link_get_addr(link)) {
        char mac_str[20];
        nl_addr2str(hw_addr, mac_str, sizeof(mac_str));
        m_json.hw.mac.emplace_back(mac_str);
    }

    m_json.hw.mtu = rtnl_link_get_mtu(link);

    if (unsigned int const txq_len = rtnl_link_get_txqlen(link); txq_len >= 0) {
        m_json.hw.size_queue = txq_len;
    }

    unsigned int const oper_state = rtnl_link_get_operstate(link);
    switch (oper_state) {
        case IF_OPER_UNKNOWN:
            m_json.operational_status.oper_state = "UNKNOWN";
            break;
        case IF_OPER_NOTPRESENT:
            m_json.operational_status.oper_state = "NOT PRESENT";
            break;
        case IF_OPER_DOWN:
            m_json.operational_status.oper_state = "DOWN";
            break;
        case IF_OPER_LOWERLAYERDOWN:
            m_json.operational_status.oper_state = "LOWER LAYER DOWN";
            break;
        case IF_OPER_TESTING:
            m_json.operational_status.oper_state = "TESTING";
            break;
        case IF_OPER_DORMANT:
            m_json.operational_status.oper_state = "DORMANT";
            break;
        case IF_OPER_UP:
            m_json.operational_status.oper_state = "UP";
            break;
        default:
            m_json.operational_status.oper_state = "UNDEFINED";
    }

    unsigned int const link_mode = rtnl_link_get_linkmode(link);
    switch (link_mode) {
        case IF_LINK_MODE_DEFAULT:
            m_json.operational_status.link_mode = "DEFAULT";
            break;
        case IF_LINK_MODE_DORMANT:
            m_json.operational_status.link_mode = "DORMANT";
            break;
        default:
            m_json.operational_status.link_mode = "UNKNOWN";
    }

    // uint64_t const rx_bytes = rtnl_link_get_stat(link, RTNL_LINK_RX_BYTES);
    // uint64_t const tx_bytes = rtnl_link_get_stat(link, RTNL_LINK_TX_BYTES);

    m_json.protocols.routing_ipv4 = (flags & IFF_NOARP) ? false : true;
    m_json.protocols.multicast = (flags & IFF_MULTICAST) ? true : false;
}
void ShowInfoInterface::print_address_info(rtnl_addr *addr) {
    auto const local = rtnl_addr_get_local(addr);
    if (!local) {
        return;
    }

    Ip ip;

    int const family = nl_addr_get_family(local);
    char ip_str[100];
    nl_addr2str(local, ip_str, sizeof(ip_str));

    std::string family_str;
    if (family == AF_INET) {
        family_str = "IPv4";
        ip.type = "IPv4";
    } else if (family == AF_INET6) {
        family_str = "IPv6";
        ip.type = "IPv6";
    } else {
        family_str = "Неизвестный (" + std::to_string(family) + ")";
    }

    ip.ip = ip_str;

    int const prefix_len = rtnl_addr_get_prefixlen(addr);
    ip.masc = prefix_len;

    unsigned int const flags = rtnl_addr_get_flags(addr);
    if (flags & IFA_F_PERMANENT) {
        ip.flags.emplace_back("PERMANENT");
    }
    if (flags & IFA_F_SECONDARY) {
        ip.flags.emplace_back("SECONDARY");
    }
    if (flags & IFA_F_TENTATIVE) {
        ip.flags.emplace_back("TENTATIVE");
    }
    if (flags & IFA_F_DEPRECATED) {
        ip.flags.emplace_back("DEPRECATED");
    }
    if (flags & IFA_F_HOMEADDRESS) {
        ip.flags.emplace_back("HOME");
    }
    if (flags & IFA_F_NODAD) {
        ip.flags.emplace_back("NODAD");
    }
    if (flags & IFA_F_OPTIMISTIC) {
        ip.flags.emplace_back("OPTIMISTIC");
    }
    if (flags & IFA_F_TEMPORARY) {
        ip.flags.emplace_back("TEMPORARY");
    }

    uint32_t const valid_lft = rtnl_addr_get_valid_lifetime(addr);
    uint32_t const pref_lft = rtnl_addr_get_preferred_lifetime(addr);

    if (valid_lft != 0xFFFFFFFF) {
        ip.valid_lft = valid_lft;
    }

    if (pref_lft != 0xFFFFFFFF) {
        ip.pref_lft = pref_lft;
    }

    if (family == AF_INET) {
        if (auto const broadcast = rtnl_addr_get_broadcast(addr)) {
            nl_addr2str(broadcast, ip_str, sizeof(ip_str));
            ip.broadcast = ip_str;
        }
    }

    if (auto const peer = rtnl_addr_get_peer(addr)) {
        nl_addr2str(peer, ip_str, sizeof(ip_str));
        ip.peer = ip_str;
    }
    m_json.ip.emplace_back(ip);
}
void ShowInfoInterface::print_neighbour_info(int const ifindex) {
    bool has_neighbours = false;

    Neigh neigh_json;

    for (auto obj = nl_cache_get_first(m_neigh_data.get()); obj; obj = nl_cache_get_next(obj)) {
        auto const neigh = reinterpret_cast<struct rtnl_neigh *>(obj);

        if (rtnl_neigh_get_ifindex(neigh) != ifindex) {
            continue;
        }

        if (!has_neighbours) {
            has_neighbours = true;
        }

        auto const dst = rtnl_neigh_get_dst(neigh);
        auto const lladdr = rtnl_neigh_get_lladdr(neigh);

        if (!dst) continue;

        char ip_str[100];
        nl_addr2str(dst, ip_str, sizeof(ip_str));

        neigh_json.ip = ip_str;

        if (lladdr) {
            char mac_str[20];
            nl_addr2str(lladdr, mac_str, sizeof(mac_str));
            neigh_json.mac = mac_str;
        } else {
        }

        int const state = rtnl_neigh_get_state(neigh);
        if (state & NUD_INCOMPLETE) {
            neigh_json.type.emplace_back("INCOMPLETE");
        }
        if (state & NUD_REACHABLE) {
            neigh_json.type.emplace_back("REACHABLE");
        }
        if (state & NUD_STALE) {
            neigh_json.type.emplace_back("STALE");
        }
        if (state & NUD_DELAY) {
            neigh_json.type.emplace_back("DELAY");
        }
        if (state & NUD_PROBE) {
            neigh_json.type.emplace_back("PROBE");
        }
        if (state & NUD_FAILED) {
            neigh_json.type.emplace_back("FAILED");
        }
        if (state & NUD_NOARP) {
            neigh_json.type.emplace_back("NOARP");
        }
        if (state & NUD_PERMANENT) {
            neigh_json.type.emplace_back("PERMANENT");
        }
        m_json.neigh.emplace_back(neigh_json);
    }

    if (!has_neighbours) {
    }
}
void ShowInfoInterface::print_routes_for_interface(int const ifindex) {
    bool has_routes = false;

    Routes routes;

    for (auto obj = nl_cache_get_first(m_route_data.get()); obj; obj = nl_cache_get_next(obj)) {
        auto const route = reinterpret_cast<struct rtnl_route *>(obj);

        bool route_for_interface = false;

        int const next_hops = rtnl_route_get_nnexthops(route);
        for (int i = 0; i < next_hops; i++) {
            if (auto const nh = rtnl_route_nexthop_n(route, i); rtnl_route_nh_get_ifindex(nh) == ifindex) {
                route_for_interface = true;
                break;
            }
        }

        if (!route_for_interface) {
            continue;
        }

        if (!has_routes) {
            has_routes = true;
        }

        auto const dst = rtnl_route_get_dst(route);
        char dst_str[100] = "(default)";
        if (dst && !nl_addr_iszero(dst)) {
            nl_addr2str(dst, dst_str, sizeof(dst_str));
        }

        uint32_t const table = rtnl_route_get_table(route);
        uint32_t const priority = rtnl_route_get_priority(route);

        routes.destination = dst_str;

        for (int i = 0; i < next_hops; i++) {
            auto const nh = rtnl_route_nexthop_n(route, i);
            auto const gateway = rtnl_route_nh_get_gateway(nh);

            if (gateway) {
                char gw_str[100];
                nl_addr2str(gateway, gw_str, sizeof(gw_str));
                routes.gateway = gw_str;
            } else {
                routes.gateway = "direct";
            }
        }

        routes.metric = priority;
        routes.table = table;

        switch (rtnl_route_get_type(route)) {
            case RTN_UNICAST:
                routes.type = "UNICAST";
                break;
            case RTN_LOCAL:
                routes.type = "LOCAL";
                break;
            case RTN_BROADCAST:
                routes.type = "BROADCAST";
                break;
            case RTN_ANYCAST:
                routes.type = "ANYCAST";
                break;
            case RTN_MULTICAST:
                routes.type = "MULTICAST";
                break;
            case RTN_BLACKHOLE:
                routes.type = "BLACKHOLE";
                break;
            case RTN_UNREACHABLE:
                routes.type = "UNREACHABLE";
                break;
            case RTN_PROHIBIT:
                routes.type = "PROHIBIT";
                break;
            case RTN_THROW:
                routes.type = "THROW";
                break;
            case RTN_NAT:
                routes.type = "NAT";
                break;
            case RTN_XRESOLVE:
                routes.type = "XRESOLVE";
                break;
            default:
                routes.type = "UNKNOWN";
                break;
        }

        m_json.routes.emplace_back(routes);
    }

    if (!has_routes) {
    }
}
void ShowInfoInterface::show() {
    for (auto obj = nl_cache_get_first(m_link_data.get()); obj; obj = nl_cache_get_next(obj)) {
        auto const link = reinterpret_cast<struct rtnl_link *>(obj);

        print_interface_details(link);

        int const ifindex = rtnl_link_get_ifindex(link);

        bool has_addresses = false;

        for (auto addr_obj = nl_cache_get_first(m_addr_data.get()); addr_obj; addr_obj = nl_cache_get_next(addr_obj)) {
            if (auto const addr = reinterpret_cast<struct rtnl_addr *>(addr_obj); rtnl_addr_get_ifindex(addr) == ifindex) {
                print_address_info(addr);
                has_addresses = true;
            }
        }

        if (!has_addresses) {
        }

        print_neighbour_info(ifindex);

        print_routes_for_interface(ifindex);

        std::cout << "\n";
    }
}
void ShowInfoInterface::showInterface(const std::string &interface_name) {
    int ifindex = getInterfaceIndex(interface_name);

    showInterfaceByIndex(ifindex);
}

int ShowInfoInterface::getInterfaceIndex(const std::string &interface_name) {
    for (auto obj = nl_cache_get_first(m_link_data.get()); obj; obj = nl_cache_get_next(obj)) {
        auto const link = reinterpret_cast<struct rtnl_link *>(obj);
        const char *if_name = rtnl_link_get_name(link);

        if (if_name && interface_name == if_name) {
            return rtnl_link_get_ifindex(link);
        }
    }

    throw exceptions::InterfaceNotFound(fmt::format("Интерфейс '{}' не найден", interface_name));
}

void ShowInfoInterface::showInterfaceByIndex(int ifindex) {
    rtnl_link *link = nullptr;
    for (auto obj = nl_cache_get_first(m_link_data.get()); obj; obj = nl_cache_get_next(obj)) {
        auto current_link = reinterpret_cast<struct rtnl_link *>(obj);
        if (rtnl_link_get_ifindex(current_link) == ifindex) {
            link = current_link;
            break;
        }
    }

    if (!link) {
        throw exceptions::InterfaceNotFound(fmt::format("Интерфейс с индексом {} не найден", ifindex));
    }

    print_interface_details(link);

    bool has_addresses = false;

    for (auto addr_obj = nl_cache_get_first(m_addr_data.get()); addr_obj; addr_obj = nl_cache_get_next(addr_obj)) {
        if (auto const addr = reinterpret_cast<struct rtnl_addr *>(addr_obj); rtnl_addr_get_ifindex(addr) == ifindex) {
            print_address_info(addr);
            has_addresses = true;
        }
    }

    print_neighbour_info(ifindex);
    print_routes_for_interface(ifindex);
}

} // namespace os::network
