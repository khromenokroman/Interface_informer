#include <iomanip>
#include <iostream>

#include "printer.hpp"
#include "switch_ns.hpp"

int main() {
    std::cout << "=== ИНФОРМАЦИЯ О СЕТЕВЫХ ИНТЕРФЕЙСАХ LINUX ===" << std::endl;
    auto const namespaces = ::os::ns::NetNamespaceHandler::getNetworkNamespaces();

    std::cout << "\n=== ИНТЕРФЕЙСЫ В ТЕКУЩЕМ NAMESPACE ===" << std::endl;
    os::network::ShowInfoInterface().show();

    for (const auto &ns : namespaces) {
        ::os::ns::NetNamespaceHandler const nsHandler;
        std::cout << "\n\n=== ИНТЕРФЕЙСЫ В NAMESPACE: " << ns << " ===" << std::endl;

        ::os::ns::NetNamespaceHandler::switchToNamespace(ns);
        os::network::ShowInfoInterface().show();

        nsHandler.switchBackToOriginal();
    }

    return 0;
}
