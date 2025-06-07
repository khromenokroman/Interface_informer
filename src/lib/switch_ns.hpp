#pragma once
#include <dirent.h>
#include <fcntl.h>
#include <fmt/format.h>
#include <sched.h>
#include <unistd.h>

#include <stdexcept>
#include <vector>

namespace os::ns {
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

/**
 * @class NetNamespaceHandler
 * @brief Класс для работы с сетевыми пространствами имен Linux
 *
 * Предоставляет методы для перечисления доступных сетевых пространств имен,
 * переключения между ними и возврата в исходное пространство имен.
 */
class NetNamespaceHandler {
   public:
    /**
     * @brief Конструктор сохраняет текущее сетевое пространство имен
     * @throw exceptions::OpenNamespace если не удалось открыть текущее пространство имен
     */
    NetNamespaceHandler();
    /**
     * @brief Деструктор возвращается в исходное пространство имен
     */
    ~NetNamespaceHandler();

    NetNamespaceHandler(NetNamespaceHandler const &) = delete;
    NetNamespaceHandler(NetNamespaceHandler &&) = delete;
    NetNamespaceHandler &operator=(NetNamespaceHandler const &) = delete;
    NetNamespaceHandler &operator=(NetNamespaceHandler &&) = delete;

    /**
     * @brief Получает список доступных сетевых пространств имен
     * @return Вектор имен доступных пространств имен
     */
    static std::vector<std::string> getNetworkNamespaces();
    /**
     * @brief Переключается в указанное сетевое пространство имен
     * @param name Имя пространства имен для переключения
     * @throw exceptions::OpenNamespace если не удалось открыть указанное пространство имен
     * @throw exceptions::SwitchNamespace если не удалось переключиться в указанное пространство имен
     */
    static void switchToNamespace(const std::string &name);
    /**
     * @brief Возвращается в исходное пространство имен
     * @throw exceptions::SwitchNamespace если не удалось вернуться в исходное пространство имен
     */
    void switchBackToOriginal() const;

   private:
    int m_original_ns_fd{}; // 4
};
} // namespace os::ns
