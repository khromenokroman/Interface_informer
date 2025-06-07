#include <informer/interface_informer.hpp>
#include <iostream>

int main() {
    try {

        std::cout << "All namespaces:\n";
        auto const all_ns = ::os::network::InformerNetlink::get_network_namespaces();
        std::cout << all_ns.dump(4) << std::endl << std::endl;

        auto const connection = ::os::network::InformerNetlink::create();

        std::cout << "All interfaces (main):\n";
        auto const all_interfaces = connection->get_all_interfaces();
        std::cout << all_interfaces.dump(4) << std::endl<< std::endl;

        std::cout << "Switch to namespace 'sample'!" << std::endl;
        ::os::network::InformerNetlink::switch_to_namespace("sample");
        std::cout << "All interfaces namespace 'sample':\n";
        auto const connection_sample = ::os::network::InformerNetlink::create();
        auto const all_interfaces_sample = connection_sample->get_all_interfaces();
        std::cout << all_interfaces_sample.dump(4) << std::endl << std::endl;

        std::cout << "Show info for 'eth0' in namespace 'sample'" << std::endl;
        auto const answer = connection_sample->get_interface_info("eth0");
        std::cout << answer.dump(4) << std::endl;

        return 0;
    } catch (std::exception const &ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }
}

// namespace po = boost::program_options;
//
// /**
//  * @brief Парсинг аргументов командной строки
//  * @param argc Количество аргументов
//  * @param argv Массив аргументов
//  * @return Результат парсинга аргументов
//  */
// po::variables_map parseCommandLine(int argc, char* argv[]) {
//     po::variables_map vm;
//
//     try {
//         po::options_description desc("Допустимые опции");
//         desc.add_options()("help,h", "Показать справку")("all,a", "Показать информацию обо всех сетевых пространствах имен")(
//             "current,c", "Показать информацию о текущем пространстве имен (по умолчанию)")(
//             "no-current,n", "Не показывать информацию о текущем пространстве имен")("interface,i", po::value<std::string>(),
//                                                                                     "Показать информацию только о конкретном интерфейсе")(
//             "namespace,s", po::value<std::vector<std::string>>(), "Конкретные пространства имен для отображения");
//
//         po::positional_options_description p;
//         p.add("namespace", -1);
//
//         po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
//         po::notify(vm);
//
//         if (vm.count("help")) {
//             std::cout << "Использование: " << argv[0] << " [опции] [namespace1 namespace2 ...]" << std::endl;
//             std::cout << desc << std::endl;
//             std::cout << "Примеры:" << std::endl;
//             std::cout << "  " << argv[0] << " -a                   # Показать все доступные пространства имен" << std::endl;
//             std::cout << "  " << argv[0] << " -c                   # Показать только текущее пространство имен" << std::endl;
//             std::cout << "  " << argv[0] << " -n netns1 netns2     # Показать указанные пространства имен" << std::endl;
//             std::cout << "  " << argv[0] << " -i eth0              # Показать информацию только по интерфейсу eth0" << std::endl;
//             std::cout << "  " << argv[0] << " -i eth0 -s netns1    # Показать интерфейс eth0 в пространстве имен netns1" << std::endl;
//         }
//     } catch (std::exception& e) {
//         std::cerr << "Ошибка при разборе аргументов: " << e.what() << std::endl;
//         std::cerr << "Используйте --help для получения списка допустимых опций." << std::endl;
//     }
//
//     return vm;
// }
//
// /**
//  * @brief Проверка существования namespace
//  * @param name Имя пространства имен
//  * @param availableNamespaces Список доступных пространств имен
//  * @return true, если namespace существует
//  */
// bool namespaceExists(const std::string& name, const std::vector<std::string>& availableNamespaces) {
//     return std::find(availableNamespaces.begin(), availableNamespaces.end(), name) != availableNamespaces.end();
// }
//
// int main(int argc, char* argv[]) {
//     po::variables_map vm = parseCommandLine(argc, argv);
//
//     if (vm.count("help")) {
//         return 0;
//     }
//
//     std::cout << "=== ИНФОРМАЦИЯ О СЕТЕВЫХ ИНТЕРФЕЙСАХ LINUX ===" << std::endl;
//
//     auto const availableNamespaces = ::os::ns::NetNamespaceHandler::getNetworkNamespaces();
//
//     if (!availableNamespaces.empty()) {
//         std::cout << "Доступные пространства имен: ";
//         for (size_t i = 0; i < availableNamespaces.size(); ++i) {
//             std::cout << availableNamespaces[i];
//             if (i < availableNamespaces.size() - 1) {
//                 std::cout << ", ";
//             }
//         }
//         std::cout << std::endl;
//     } else {
//         std::cout << "Доступных пространств имен не найдено" << std::endl;
//     }
//
//     std::string interfaceFilter;
//     if (vm.count("interface")) {
//         interfaceFilter = vm["interface"].as<std::string>();
//         std::cout << "Фильтр: только интерфейс '" << interfaceFilter << "'" << std::endl;
//     }
//
//     std::vector<std::string> namespacesToShow;
//
//     if (vm.count("namespace")) {
//         auto namespaces = vm["namespace"].as<std::vector<std::string>>();
//
//         for (const auto& ns : namespaces) {
//             if (namespaceExists(ns, availableNamespaces)) {
//                 namespacesToShow.push_back(ns);
//             } else {
//                 std::cerr << "Предупреждение: Пространство имен '" << ns << "' не найдено и будет пропущено" << std::endl;
//                 return -1;
//             }
//         }
//     } else if (vm.count("all")) {
//         namespacesToShow = availableNamespaces;
//     }
//
//     bool showCurrentNamespace = true;
//
//     if (vm.count("no-current")) {
//         showCurrentNamespace = false;
//     } else if (vm.count("current")) {
//         showCurrentNamespace = true;
//     } else if (!namespacesToShow.empty()) {
//         showCurrentNamespace = false;
//     }
//
//     if (showCurrentNamespace) {
//         std::cout << "\n=== ИНТЕРФЕЙСЫ В ТЕКУЩЕМ NAMESPACE ===" << std::endl;
//         try {
//             os::network::ShowInfoInterface infoInterface;
//
//             if (!interfaceFilter.empty()) {
//                 try {
//                     infoInterface.showInterface(interfaceFilter);
//                 } catch (const os::network::exceptions::InterfaceNotFound& e) {
//                     std::cerr << "Ошибка: " << e.what() << " в текущем пространстве имен" << std::endl;
//                 }
//             } else {
//                 infoInterface.show();
//             }
//         } catch (const std::exception& e) {
//             std::cerr << "Ошибка при отображении текущего namespace: " << e.what() << std::endl;
//         }
//     }
//
//     if (namespacesToShow.empty() && !showCurrentNamespace) {
//         std::cout << "\nНет пространств имен для отображения. Используйте --help для получения справки." << std::endl;
//     }
//
//     ::os::ns::NetNamespaceHandler const nsHandler;
//
//     for (const auto& ns : namespacesToShow) {
//         try {
//             std::cout << "\n\n=== ИНТЕРФЕЙСЫ В NAMESPACE: " << ns << " ===" << std::endl;
//
//             ::os::ns::NetNamespaceHandler::switchToNamespace(ns);
//
//             os::network::ShowInfoInterface infoInterface;
//
//             if (!interfaceFilter.empty()) {
//                 try {
//                     infoInterface.showInterface(interfaceFilter);
//                 } catch (const os::network::exceptions::InterfaceNotFound& e) {
//                     std::cerr << "Ошибка: " << e.what() << " в пространстве имен '" << ns << "'" << std::endl;
//                 }
//             } else {
//                 infoInterface.show();
//             }
//
//             nsHandler.switchBackToOriginal();
//         } catch (const os::ns::exceptions::SwitchNamespace& e) {
//             std::cerr << "Ошибка при переключении в пространство имен '" << ns << "': " << e.what() << std::endl;
//         } catch (const os::ns::exceptions::OpenNamespace& e) {
//             std::cerr << "Ошибка при открытии пространства имен '" << ns << "': " << e.what() << std::endl;
//         } catch (const std::exception& e) {
//             std::cerr << "Ошибка при работе с пространством имен '" << ns << "': " << e.what() << std::endl;
//
//             try {
//                 nsHandler.switchBackToOriginal();
//             } catch (...) {
//                 std::cerr << "Критическая ошибка: не удалось вернуться в исходный namespace" << std::endl;
//             }
//         }
//     }
//
//     return 0;
// }