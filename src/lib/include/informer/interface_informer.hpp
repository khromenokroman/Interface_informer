/**
 * @file interface_informer.hpp
 * @brief Интерфейс для получения информации о сетевых интерфейсах Linux с поддержкой сетевых пространств имен.
 * @author Roman Khromenko
 * @copyright 2025
 */

#pragma once
#include <dirent.h>
#include <fcntl.h>
#include <fmt/format.h>

#include <nlohmann/json.hpp>

namespace os::network {
namespace exceptions {

/**
 * @struct NetNamespaceHandlerEx
 * @brief Базовый класс исключений для операций с сетевыми пространствами имен.
 */
struct NetNamespaceHandlerEx : std::runtime_error {
    using std::runtime_error::runtime_error;
};

/**
 * @struct OpenNamespace
 * @brief Исключение, возникающее при невозможности открыть сетевое пространство имен.
 */
struct OpenNamespace final : NetNamespaceHandlerEx {
    using NetNamespaceHandlerEx::NetNamespaceHandlerEx;
};

/**
 * @struct SwitchNamespace
 * @brief Исключение, возникающее при невозможности переключиться в сетевое пространство имен.
 */
struct SwitchNamespace final : NetNamespaceHandlerEx {
    using NetNamespaceHandlerEx::NetNamespaceHandlerEx;
};
} // namespace exceptions

/**
 * @class InformerNetlink
 * @brief Абстрактный класс для получения информации о сетевых интерфейсах через Netlink.
 *
 * Этот класс предоставляет интерфейс для получения подробной информации
 * о сетевых интерфейсах в системе Linux с использованием Netlink API.
 * Он поддерживает работу с сетевыми пространствами имен и предоставляет
 * информацию в формате JSON.
 */
class InformerNetlink {
   public:
    InformerNetlink() = default;
    virtual ~InformerNetlink() = default;

    InformerNetlink(InformerNetlink const &) = delete;
    InformerNetlink(InformerNetlink &&) = delete;
    InformerNetlink &operator=(InformerNetlink const &) = delete;
    InformerNetlink &operator=(InformerNetlink &&) = delete;

    /**
     * @brief Получает подробную информацию о конкретном сетевом интерфейсе.
     * @param interface_name Имя интерфейса, о котором нужно получить информацию.
     * @return JSON-объект с детальной информацией об интерфейсе.
     */
    virtual ::nlohmann::json get_interface_info(std::string const &interface_name) = 0;
    /**
     * @brief Получает список всех доступных сетевых интерфейсов в текущем пространстве имен.
     * @return JSON-объект со списком интерфейсов.
     */
    virtual ::nlohmann::json get_all_interfaces() = 0;
    /**
     * @brief Переключается в указанное сетевое пространство имен.
     * @param name Имя сетевого пространства имен.
     * @throw exceptions::OpenNamespace Если не удалось открыть пространство имен.
     * @throw exceptions::SwitchNamespace Если не удалось переключиться в пространство имен.
     */
    static void switch_to_namespace(const std::string &name);
    /**
     * @brief Получает список всех доступных сетевых пространств имен.
     * @return JSON-объект со списком доступных пространств имен.
     */
    static ::nlohmann::json get_network_namespaces();
    /**
     * @brief Создает экземпляр класса-наследника InformerNetlink.
     * @return Умный указатель на созданный объект.
     * @throw std::runtime_error Если не удалось создать объект.
     */
    static std::unique_ptr<InformerNetlink> create();

   private:
    /**
     * @brief Внутренний метод для создания экземпляра класса-наследника InformerNetlink.
     * @param error_message Буфер для сообщения об ошибке в случае неудачи.
     * @return Указатель на созданный объект или nullptr в случае ошибки.
     */
    static InformerNetlink *create(char *error_message) noexcept;
    /**
     * @brief Максимальный размер буфера для сообщений об ошибках.
     */
    static constexpr std::size_t M_MAX_BUFFER_SIZE = 1024;
};

/**
 * @brief Создает экземпляр класса-наследника InformerNetlink.
 * @return Умный указатель на созданный объект.
 * @throw std::runtime_error Если не удалось создать объект.
 */
inline std::unique_ptr<InformerNetlink> InformerNetlink::create() {
    auto const message_error = std::make_unique<char[]>(M_MAX_BUFFER_SIZE);

    InformerNetlink *new_object = create(message_error.get());
    if (new_object == nullptr) {
        throw std::runtime_error(message_error.get());
    }

    return std::unique_ptr<InformerNetlink>{new_object};
}

/**
 * @brief Переключается в указанное сетевое пространство имен.
 * @param name Имя сетевого пространства имен.
 * @throw exceptions::OpenNamespace Если не удалось открыть пространство имен.
 * @throw exceptions::SwitchNamespace Если не удалось переключиться в пространство имен.
 */
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

/**
 * @brief Получает список всех доступных сетевых пространств имен.
 * @return JSON-объект со списком доступных пространств имен.
 */
inline ::nlohmann::json InformerNetlink::get_network_namespaces() {
    nlohmann::json json{};
    nlohmann::json namespaces = nlohmann::json::array();
    if (auto const dir = opendir("/var/run/netns")) {
        dirent *entry = nullptr;
        while ((entry = readdir(dir)) != nullptr) {
            if (entry->d_name[0] != '.') {
                namespaces.emplace_back(entry->d_name);
            }
        }
        closedir(dir);
    }

    json["namespaces"] = namespaces;

    return json;
}
} // namespace os::network