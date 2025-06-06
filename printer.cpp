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

    std::cout << "\n==================================================================" << std::endl;
    std::cout << "ИНТЕРФЕЙС: " << if_name << std::endl;
    std::cout << "==================================================================" << std::endl;

    std::cout << "ОСНОВНАЯ ИНФОРМАЦИЯ:" << std::endl;
    std::cout << "  Индекс: " << rtnl_link_get_ifindex(link) << std::endl;

    unsigned int const flags = rtnl_link_get_flags(link);
    std::cout << "  Состояние: " << ((flags & IFF_UP) ? "АКТИВЕН (UP)" : "НЕАКТИВЕН (DOWN)") << std::endl;

    std::cout << "  Тип интерфейса: ";
    if (flags & IFF_LOOPBACK)
        std::cout << "LOOPBACK";
    else if (flags & IFF_BROADCAST)
        std::cout << "BROADCAST";
    else if (flags & IFF_POINTOPOINT)
        std::cout << "POINT-TO-POINT";
    else
        std::cout << "UNKNOWN";
    std::cout << std::endl;

    std::cout << "  Флаги: ";
    if (flags & IFF_UP) std::cout << "UP ";
    if (flags & IFF_BROADCAST) std::cout << "BROADCAST ";
    if (flags & IFF_DEBUG) std::cout << "DEBUG ";
    if (flags & IFF_LOOPBACK) std::cout << "LOOPBACK ";
    if (flags & IFF_POINTOPOINT) std::cout << "POINTOPOINT ";
    if (flags & IFF_RUNNING) std::cout << "RUNNING ";
    if (flags & IFF_NOARP) std::cout << "NOARP ";
    if (flags & IFF_PROMISC) std::cout << "PROMISC ";
    if (flags & IFF_ALLMULTI) std::cout << "ALLMULTI ";
    if (flags & IFF_MASTER) std::cout << "MASTER ";
    if (flags & IFF_SLAVE) std::cout << "SLAVE ";
    if (flags & IFF_MULTICAST) std::cout << "MULTICAST ";
    if (flags & IFF_PORTSEL) std::cout << "PORTSEL ";
    if (flags & IFF_AUTOMEDIA) std::cout << "AUTOMEDIA ";
    if (flags & IFF_DYNAMIC) std::cout << "DYNAMIC ";
    std::cout << std::endl;

    std::cout << "\nАППАРАТНАЯ ИНФОРМАЦИЯ:" << std::endl;
    unsigned int const arp_type = rtnl_link_get_arptype(link);
    std::cout << "  Тип аппаратного адреса: " << arp_hrd_type_to_string(arp_type) << std::endl;

    if (auto const hw_addr = rtnl_link_get_addr(link)) {
        char mac_str[20];
        nl_addr2str(hw_addr, mac_str, sizeof(mac_str));
        std::cout << "  MAC-адрес: " << mac_str << std::endl;
    }

    std::cout << "  MTU: " << rtnl_link_get_mtu(link) << " байт" << std::endl;

    if (int const txq_len = rtnl_link_get_txqlen(link); txq_len >= 0) {
        std::cout << "  Длина очереди передачи: " << txq_len << std::endl;
    }

    unsigned int const oper_state = rtnl_link_get_operstate(link);
    std::cout << "\nОПЕРАЦИОННОЕ СОСТОЯНИЕ:" << std::endl;
    std::cout << "  Состояние: ";
    switch (oper_state) {
        case IF_OPER_UNKNOWN:
            std::cout << "UNKNOWN";
            break;
        case IF_OPER_NOTPRESENT:
            std::cout << "NOT PRESENT";
            break;
        case IF_OPER_DOWN:
            std::cout << "DOWN";
            break;
        case IF_OPER_LOWERLAYERDOWN:
            std::cout << "LOWER LAYER DOWN";
            break;
        case IF_OPER_TESTING:
            std::cout << "TESTING";
            break;
        case IF_OPER_DORMANT:
            std::cout << "DORMANT";
            break;
        case IF_OPER_UP:
            std::cout << "UP";
            break;
        default:
            std::cout << "UNDEFINED (" << oper_state << ")";
    }
    std::cout << std::endl;

    unsigned int const link_mode = rtnl_link_get_linkmode(link);
    std::cout << "  Режим соединения: ";
    switch (link_mode) {
        case IF_LINK_MODE_DEFAULT:
            std::cout << "DEFAULT";
            break;
        case IF_LINK_MODE_DORMANT:
            std::cout << "DORMANT";
            break;
        default:
            std::cout << "UNKNOWN (" << link_mode << ")";
    }
    std::cout << std::endl;

    std::cout << "\nСТАТИСТИКА ИНТЕРФЕЙСА:" << std::endl;
    std::cout << "  Получено:" << std::endl;
    uint64_t const rx_bytes = rtnl_link_get_stat(link, RTNL_LINK_RX_BYTES);
    std::cout << "    Байт: " << rx_bytes << " (" << format_size(rx_bytes) << ")" << std::endl;
    std::cout << "    Пакетов: " << rtnl_link_get_stat(link, RTNL_LINK_RX_PACKETS) << std::endl;
    std::cout << "    Ошибок: " << rtnl_link_get_stat(link, RTNL_LINK_RX_ERRORS) << std::endl;
    std::cout << "    Отброшенных: " << rtnl_link_get_stat(link, RTNL_LINK_RX_DROPPED) << std::endl;

    std::cout << "  Отправлено:" << std::endl;
    uint64_t const tx_bytes = rtnl_link_get_stat(link, RTNL_LINK_TX_BYTES);
    std::cout << "    Байт: " << tx_bytes << " (" << format_size(tx_bytes) << ")" << std::endl;
    std::cout << "    Пакетов: " << rtnl_link_get_stat(link, RTNL_LINK_TX_PACKETS) << std::endl;
    std::cout << "    Ошибок: " << rtnl_link_get_stat(link, RTNL_LINK_TX_ERRORS) << std::endl;
    std::cout << "    Отброшенных: " << rtnl_link_get_stat(link, RTNL_LINK_TX_DROPPED) << std::endl;

    std::cout << "\nПРОТОКОЛЫ:" << std::endl;
    std::cout << "  Маршрутизация IPv4: " << ((flags & IFF_NOARP) ? "Выключена" : "Включена") << std::endl;
    std::cout << "  Multicast: " << ((flags & IFF_MULTICAST) ? "Включен" : "Выключен") << std::endl;
}
void ShowInfoInterface::print_address_info(rtnl_addr *addr) {
    auto const local = rtnl_addr_get_local(addr);
    if (!local) return;

    int const family = nl_addr_get_family(local);
    char ip_str[100];
    nl_addr2str(local, ip_str, sizeof(ip_str));

    std::string family_str;
    if (family == AF_INET) {
        family_str = "IPv4";
    } else if (family == AF_INET6) {
        family_str = "IPv6";
    } else {
        family_str = "Неизвестный (" + std::to_string(family) + ")";
    }

    std::cout << "  " << family_str << ": " << ip_str;

    int const prefix_len = rtnl_addr_get_prefixlen(addr);
    std::cout << "/" << prefix_len;

    unsigned int const flags = rtnl_addr_get_flags(addr);
    std::cout << " [";
    if (flags & IFA_F_PERMANENT) std::cout << "PERMANENT ";
    if (flags & IFA_F_SECONDARY) std::cout << "SECONDARY ";
    if (flags & IFA_F_TENTATIVE) std::cout << "TENTATIVE ";
    if (flags & IFA_F_DEPRECATED) std::cout << "DEPRECATED ";
    if (flags & IFA_F_HOMEADDRESS) std::cout << "HOME ";
    if (flags & IFA_F_NODAD) std::cout << "NODAD ";
    if (flags & IFA_F_OPTIMISTIC) std::cout << "OPTIMISTIC ";
    if (flags & IFA_F_TEMPORARY) std::cout << "TEMPORARY ";
    std::cout << "]";

    uint32_t const valid_lft = rtnl_addr_get_valid_lifetime(addr);
    uint32_t const pref_lft = rtnl_addr_get_preferred_lifetime(addr);

    if (valid_lft != 0xFFFFFFFF) {
        std::cout << " valid_lft=" << valid_lft << "s";
    }

    if (pref_lft != 0xFFFFFFFF) {
        std::cout << " pref_lft=" << pref_lft << "s";
    }

    std::cout << std::endl;

    if (family == AF_INET) {
        if (auto const broadcast = rtnl_addr_get_broadcast(addr)) {
            nl_addr2str(broadcast, ip_str, sizeof(ip_str));
            std::cout << "    Broadcast: " << ip_str << std::endl;
        }
    }

    if (auto const peer = rtnl_addr_get_peer(addr)) {
        nl_addr2str(peer, ip_str, sizeof(ip_str));
        std::cout << "    Peer: " << ip_str << std::endl;
    }
}
void ShowInfoInterface::print_neighbour_info(int const ifindex) const {
    bool has_neighbours = false;

    for (auto obj = nl_cache_get_first(m_neigh_data.get()); obj; obj = nl_cache_get_next(obj)) {
        auto const neigh = reinterpret_cast<struct rtnl_neigh *>(obj);

        if (rtnl_neigh_get_ifindex(neigh) != ifindex) {
            continue;
        }

        if (!has_neighbours) {
            std::cout << "\nТАБЛИЦА СОСЕДЕЙ (ARP/NDP):" << std::endl;
            has_neighbours = true;
        }

        auto const dst = rtnl_neigh_get_dst(neigh);
        auto const lladdr = rtnl_neigh_get_lladdr(neigh);

        if (!dst) continue;

        char ip_str[100];
        nl_addr2str(dst, ip_str, sizeof(ip_str));

        std::cout << "  " << ip_str << " => ";

        if (lladdr) {
            char mac_str[20];
            nl_addr2str(lladdr, mac_str, sizeof(mac_str));
            std::cout << mac_str;
        } else {
            std::cout << "(неизвестно)";
        }

        int const state = rtnl_neigh_get_state(neigh);
        std::cout << " [";
        if (state & NUD_INCOMPLETE) std::cout << "INCOMPLETE ";
        if (state & NUD_REACHABLE) std::cout << "REACHABLE ";
        if (state & NUD_STALE) std::cout << "STALE ";
        if (state & NUD_DELAY) std::cout << "DELAY ";
        if (state & NUD_PROBE) std::cout << "PROBE ";
        if (state & NUD_FAILED) std::cout << "FAILED ";
        if (state & NUD_NOARP) std::cout << "NOARP ";
        if (state & NUD_PERMANENT) std::cout << "PERMANENT ";
        std::cout << "]" << std::endl;
    }

    if (!has_neighbours) {
        std::cout << "\nТАБЛИЦА СОСЕДЕЙ: Нет записей" << std::endl;
    }
}
void ShowInfoInterface::print_routes_for_interface(int const ifindex) const {
    bool has_routes = false;

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
            std::cout << "\nТАБЛИЦА МАРШРУТИЗАЦИИ:" << std::endl;
            has_routes = true;
        }

        auto const dst = rtnl_route_get_dst(route);
        char dst_str[100] = "(default)";
        if (dst && !nl_addr_iszero(dst)) {
            nl_addr2str(dst, dst_str, sizeof(dst_str));
        }

        uint32_t const table = rtnl_route_get_table(route);
        uint32_t const priority = rtnl_route_get_priority(route);

        std::cout << "  " << dst_str;

        for (int i = 0; i < next_hops; i++) {
            auto const nh = rtnl_route_nexthop_n(route, i);
            auto const gateway = rtnl_route_nh_get_gateway(nh);

            std::cout << " via ";
            if (gateway) {
                char gw_str[100];
                nl_addr2str(gateway, gw_str, sizeof(gw_str));
                std::cout << gw_str;
            } else {
                std::cout << "direct";
            }
        }

        std::cout << " metric " << priority << " table " << table;

        switch (rtnl_route_get_type(route)) {
            case RTN_UNICAST:
                std::cout << " [UNICAST]";
                break;
            case RTN_LOCAL:
                std::cout << " [LOCAL]";
                break;
            case RTN_BROADCAST:
                std::cout << " [BROADCAST]";
                break;
            case RTN_ANYCAST:
                std::cout << " [ANYCAST]";
                break;
            case RTN_MULTICAST:
                std::cout << " [MULTICAST]";
                break;
            case RTN_BLACKHOLE:
                std::cout << " [BLACKHOLE]";
                break;
            case RTN_UNREACHABLE:
                std::cout << " [UNREACHABLE]";
                break;
            case RTN_PROHIBIT:
                std::cout << " [PROHIBIT]";
                break;
            case RTN_THROW:
                std::cout << " [THROW]";
                break;
            case RTN_NAT:
                std::cout << " [NAT]";
                break;
            case RTN_XRESOLVE:
                std::cout << " [XRESOLVE]";
                break;
            default:
                std::cout << " [UNKNOWN]";
        }

        std::cout << std::endl;
    }

    if (!has_routes) {
        std::cout << "\nТАБЛИЦА МАРШРУТИЗАЦИИ: Нет маршрутов" << std::endl;
    }
}
void ShowInfoInterface::show() const {
    for (auto obj = nl_cache_get_first(m_link_data.get()); obj; obj = nl_cache_get_next(obj)) {
        auto const link = reinterpret_cast<struct rtnl_link *>(obj);

        print_interface_details(link);

        int const ifindex = rtnl_link_get_ifindex(link);

        std::cout << "\nIP-АДРЕСА:" << std::endl;
        bool has_addresses = false;

        for (auto addr_obj = nl_cache_get_first(m_addr_data.get()); addr_obj; addr_obj = nl_cache_get_next(addr_obj)) {
            if (auto const addr = reinterpret_cast<struct rtnl_addr *>(addr_obj); rtnl_addr_get_ifindex(addr) == ifindex) {
                print_address_info(addr);
                has_addresses = true;
            }
        }

        if (!has_addresses) {
            std::cout << "  Нет IP-адресов" << std::endl;
        }

        print_neighbour_info(ifindex);

        print_routes_for_interface(ifindex);

        std::cout << "\n";
    }
}
} // namespace os::network
