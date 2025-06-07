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
        std::cout << all_interfaces.dump(4) << std::endl << std::endl;

        std::cout << "Switch to namespace 'sample'!" << std::endl;
        ::os::network::InformerNetlink::switch_to_namespace("sample");
        std::cout << "All interfaces namespace 'sample':\n";
        auto const connection_sample = ::os::network::InformerNetlink::create();
        auto const all_interfaces_sample = connection_sample->get_all_interfaces();
        std::cout << all_interfaces_sample.dump(4) << std::endl << std::endl;

        std::cout << "Show info for 'eth0' in namespace 'sample'" << std::endl;
        auto const answer = connection_sample->get_interface_info("eth0");
        std::cout << answer.dump(4) << std::endl << std::endl;

        std::cout << "Disable interface 'eth0' in namespace 'sample'" << std::endl << std::endl;

        connection_sample->disable_interface("eth0");

        std::cout << "Show info for 'eth0' in namespace 'sample'" << std::endl;

        auto const new_answer_after_down = connection_sample->get_interface_info("eth0");
        std::cout << new_answer_after_down.dump(4) << std::endl;

        return 0;
    } catch (std::exception const &ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }
}