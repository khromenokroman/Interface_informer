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

/**
 * @struct Neigh
 * @brief Структура для хранения информации о соседях (ARP/NDP таблица)
 */
struct Neigh {
    std::string ip{};                /**< IP-адрес соседа */
    std::string mac{};               /**< MAC-адрес соседа */
    std::vector<std::string> type{}; /**< Типы соседей (PERMANENT, REACHABLE и т.д.) */
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Neigh, ip, mac, type);

/**
 * @struct Routes
 * @brief Структура для хранения информации о маршрутах
 */
struct Routes {
    std::string destination{}; /**< Адрес назначения маршрута */
    std::string gateway{};     /**< Шлюз для маршрута */
    uint32_t metric{};         /**< Метрика маршрута */
    uint32_t table{};          /**< Таблица маршрутизации */
    std::string type{};        /**< Тип маршрута (UNICAST, LOCAL, BROADCAST и т.д.) */
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Routes, destination, gateway, metric, table, type);

/**
 * @struct Ip
 * @brief Структура для хранения информации об IP-адресах интерфейса
 */
struct Ip {
    std::string type{};               /**< Тип IP-адреса (IPv4 или IPv6) */
    std::string ip{};                 /**< IP-адрес с маской подсети */
    int masc{};                       /**< Маска подсети в формате CIDR */
    std::vector<std::string> flags{}; /**< Флаги IP-адреса (PERMANENT, SECONDARY и т.д.) */
    uint32_t valid_lft{};             /**< Срок действия адреса (valid lifetime) */
    uint32_t pref_lft{};              /**< Предпочтительный срок действия (preferred lifetime) */
    std::string broadcast{};          /**< Широковещательный адрес (для IPv4) */
    std::string peer{};               /**< Адрес пира (для point-to-point интерфейсов) */
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Ip, type, ip, masc, flags, valid_lft, pref_lft, broadcast, peer);

/**
 * @struct Protocols
 * @brief Структура для хранения информации о поддерживаемых протоколах
 */
struct Protocols {
    bool routing_ipv4{}; /**< Поддерживается ли IPv4 маршрутизация */
    bool multicast{};    /**< Поддерживается ли многоадресная рассылка */
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Protocols, routing_ipv4, multicast);

/**
 * @struct OperationalStatus
 * @brief Структура для хранения информации об операционном статусе интерфейса
 */
struct OperationalStatus {
    std::string oper_state{}; /**< Операционное состояние интерфейса */
    std::string link_mode{};  /**< Режим соединения */
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(OperationalStatus, oper_state, link_mode);

/**
 * @struct HW
 * @brief Структура для хранения аппаратной информации об интерфейсе
 */
struct HW {
    std::string type{};             /**< Тип аппаратного интерфейса (Ethernet, Loopback и т.д.) */
    std::vector<std::string> mac{}; /**< MAC-адрес(а) интерфейса */
    uint64_t mtu{};                 /**< Максимальный размер передаваемого блока (MTU) */
    uint64_t size_queue{};          /**< Размер очереди передачи (txqlen) */
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(HW, type, mac, mtu, size_queue);

/**
 * @struct General
 * @brief Структура для хранения общей информации об интерфейсе
 */
struct General {
    int index{};                      /**< Индекс интерфейса */
    std::string state{};              /**< Состояние интерфейса (UP/DOWN) */
    std::string type{};               /**< Тип интерфейса (BROADCAST, LOOPBACK и т.д.) */
    std::vector<std::string> flags{}; /**< Флаги интерфейса */
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(General, index, state, type, flags);

/**
 * @struct Json
 * @brief Структура для формирования полного JSON-ответа с информацией об интерфейсе
 */
struct Json {
    std::string interface{};                /**< Имя интерфейса */
    General general{};                      /**< Общая информация об интерфейсе */
    HW hw{};                                /**< Аппаратная информация */
    OperationalStatus operational_status{}; /**< Операционный статус */
    Protocols protocols{};                  /**< Поддерживаемые протоколы */
    std::vector<Ip> ip{};                   /**< Список IP-адресов */
    std::vector<Routes> routes{};           /**< Список маршрутов */
    std::vector<Neigh> neigh{};             /**< Список соседей (ARP/NDP) */
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Json, interface, general, hw, operational_status, protocols, ip, routes, neigh);

namespace exceptions {
/**
 * @struct NetlinkEx
 * @brief Базовое исключение для ошибок Netlink
 */
struct NetlinkEx : std::runtime_error {
    using std::runtime_error::runtime_error;
};

/**
 * @struct AllocateSocket
 * @brief Исключение при невозможности выделить сокет Netlink
 */
struct AllocateSocket final : NetlinkEx {
    using NetlinkEx::NetlinkEx;
};

/**
 * @struct ConnectNetlinkRoute
 * @brief Исключение при ошибке подключения к NETLINK_ROUTE
 */
struct ConnectNetlinkRoute final : NetlinkEx {
    using NetlinkEx::NetlinkEx;
};

/**
 * @struct GetDataLinks
 * @brief Исключение при ошибке получения данных о сетевых интерфейсах
 */
struct GetDataLinks final : NetlinkEx {
    using NetlinkEx::NetlinkEx;
};

/**
 * @struct GetDataAddr
 * @brief Исключение при ошибке получения данных об IP-адресах
 */
struct GetDataAddr final : NetlinkEx {
    using NetlinkEx::NetlinkEx;
};

/**
 * @struct GetDataRoute
 * @brief Исключение при ошибке получения данных о маршрутах
 */
struct GetDataRoute final : NetlinkEx {
    using NetlinkEx::NetlinkEx;
};

/**
 * @struct GetDataNeigh
 * @brief Исключение при ошибке получения данных о соседях
 */
struct GetDataNeigh final : NetlinkEx {
    using NetlinkEx::NetlinkEx;
};

/**
 * @struct InterfaceNotFound
 * @brief Исключение, когда запрошенный интерфейс не найден
 */
struct InterfaceNotFound final : NetlinkEx {
    using NetlinkEx::NetlinkEx;
};
} // namespace exceptions

/**
 * @class ShowInfoInterface
 * @brief Класс для получения детальной информации о сетевых интерфейсах
 *
 * Этот класс является реализацией интерфейса InformerNetlink и предоставляет
 * методы для сбора и отображения подробной информации о сетевых интерфейсах
 * в текущем пространстве имен.
 */
class ShowInfoInterface final : public InformerNetlink {
   public:
    /**
     * @brief Конструктор, инициализирующий соединение с Netlink и загружающий кэши данных
     * @throw exceptions::AllocateSocket если не удалось выделить сокет Netlink
     * @throw exceptions::ConnectNetlinkRoute если не удалось подключиться к NETLINK_ROUTE
     * @throw exceptions::GetDataLinks если не удалось получить данные о сетевых интерфейсах
     * @throw exceptions::GetDataAddr если не удалось получить данные об IP-адресах
     * @throw exceptions::GetDataRoute если не удалось получить данные о маршрутах
     * @throw exceptions::GetDataNeigh если не удалось получить данные о соседях
     */
    ShowInfoInterface();
    ~ShowInfoInterface() override = default;

    ShowInfoInterface(ShowInfoInterface const &) = delete;
    ShowInfoInterface(ShowInfoInterface &&) = delete;
    ShowInfoInterface &operator=(ShowInfoInterface const &) = delete;
    ShowInfoInterface &operator=(ShowInfoInterface &&) = delete;

    /**
     * @brief Получает информацию об указанном интерфейсе
     * @param interface_name Имя интерфейса
     * @return JSON с информацией об интерфейсе или сообщением об ошибке
     */
    ::nlohmann::json get_interface_info(std::string const &interface_name) override;
    /**
     * @brief Получает список всех доступных интерфейсов
     * @return JSON со списком имен интерфейсов
     */
    ::nlohmann::json get_all_interfaces() override;

   private:
    /**
     * @brief Преобразует числовой код типа оборудования в читаемую строку
     * @param type Код типа оборудования (из linux/if_arp.h)
     * @return Строковое представление типа оборудования
     */
    static std::string arp_hrd_type_to_string(unsigned int type);
    /**
     * @brief Извлекает и сохраняет основную информацию об интерфейсе
     * @param link Указатель на структуру интерфейса Netlink
     */
    void print_interface_details(rtnl_link *link);
    /**
     * @brief Извлекает и сохраняет информацию об IP-адресе
     * @param addr Указатель на структуру адреса Netlink
     */
    void print_address_info(rtnl_addr *addr);
    /**
     * @brief Извлекает и сохраняет информацию о соседях (ARP/NDP)
     * @param ifindex Индекс интерфейса
     */
    void print_neighbour_info(int ifindex);
    /**
     * @brief Извлекает и сохраняет информацию о маршрутах для интерфейса
     * @param ifindex Индекс интерфейса
     */
    void print_routes_for_interface(int ifindex);

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
     * @brief Показывает детальную информацию о конкретном интерфейсе по его индексу
     * @param ifindex Индекс интерфейса
     * @throw exceptions::InterfaceNotFound если интерфейс не найден
     */
    void showInterfaceByIndex(int ifindex);

    Json m_json{}; /**< Структура JSON для хранения информации об интерфейсе */
    std::unique_ptr<nl_sock, decltype(&nl_socket_free)> m_netlink_socket{nullptr, nl_socket_free}; /**< Сокет Netlink */
    std::unique_ptr<nl_cache, decltype(&nl_cache_free)> m_link_data{nullptr, nl_cache_free};       /**< Кэш данных об интерфейсах */
    std::unique_ptr<nl_cache, decltype(&nl_cache_free)> m_addr_data{nullptr, nl_cache_free};       /**< Кэш данных об IP-адресах */
    std::unique_ptr<nl_cache, decltype(&nl_cache_free)> m_route_data{nullptr, nl_cache_free};      /**< Кэш данных о маршрутах */
    std::unique_ptr<nl_cache, decltype(&nl_cache_free)> m_neigh_data{nullptr, nl_cache_free};      /**< Кэш данных о соседях */
};

} // namespace os::network
