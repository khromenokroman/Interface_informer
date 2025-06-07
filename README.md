# Printer Interface

````bash
                _       _              _       _             __               
               (_)     | |            (_)     | |           / _|              
  _ __   _ __   _ _ __ | |_ ___ _ __   _ _ __ | |_ ___ _ __| |_ __ _  ___ ___ 
 | '_ \ | '__| | | '_ \| __/ _ \ '__| | | '_ \| __/ _ \ '__|  _/ _` |/ __/ _ \
 | |_) || |    | | | | | ||  __/ |    | | | | | ||  __/ |  | || (_| | (_|  __/
 | .__/ |_|    |_|_| |_|\__\___|_|    |_|_| |_|\__\___|_|  |_| \__,_|\___\___|
 | |                                                                         
 |_|                                                                         
 ````

### Описание

Утилита командной строки для отображения информации о сетевых интерфейсах в Linux.
Программа позволяет просматривать подробную информацию о сетевых интерфейсах как
в текущем пространстве имен (namespace), так и в других доступных пространствах
имен сети.

### Возможности

- Отображение детальной информации о сетевых интерфейсах:
    - Основные характеристики (индекс, состояние, тип, флаги)
    - Аппаратная информация (тип адреса, MAC-адрес, MTU)
    - Операционное состояние и режим соединения
    - Статистика передачи данных (полученные/отправленные байты и пакеты)
    - IP-адреса (IPv4 и IPv6) с подробностями
    - Таблица соседей (ARP/NDP)
    - Таблица маршрутизации для интерфейса

- Фильтрация вывода:
    - По конкретному интерфейсу
    - По конкретному пространству имен сети
    - Возможность отображать/скрывать информацию о текущем пространстве имен

### Зависимости

- C++20
- libnl-3
- libnl-route-3
- fmt
- Boost Program Options

### Сборка

Проект использует CMake для сборки:

````bash
mkdir build
cd build
cmake ..
make
````

Для создания пакета DEB:

````bash
cd build
make package
````

### Установка

После сборки можно установить программу:

````bash
sudo make install
````

Или установить созданный DEB-пакет:

````bash
sudo dpkg -i libinterface_informer0*.deb
````

Добавим тестовый namespace **'sample''** и интерфейс **eth0**

````bash
ip netns add sample
ip link add veth0 type veth peer name veth1
ip link set veth1 netns sample
ip netns exec sample ip link set veth1 name eth0
ip netns exec sample ip link set lo up
ip netns exec sample ip link set eth0 up
ip netns exec sample ip addr add 192.168.100.2/24 dev eth0
ip netns exec sample ip route add default via 192.168.100.1
````

Пример кода
````c++
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
````

### Примеры работы

````json
All namespaces:
{
"namespaces": [
"sample"
]
}

All interfaces (main): {
"interfaces": [
"lo",
"wlp0s20f3",
"br-cc1332e4f8fd",
"docker0",
"veth0",
"tun0"
]
}

Switch to namespace 'sample'!
All interfaces namespace 'sample': {
"interfaces": [
"lo",
"eth0"
]
}

Show info for 'eth0' in namespace 'sample'
{
"general": {
"flags": [
"UP",
"BROADCAST",
"MULTICAST"
],
"index": 493,
"state": "UP",
"type": "BROADCAST"
},
"hw": {
"mac": [
"ba:2a:bf:ab:6c:2f"
],
"mtu": 1500,
"size_queue": 1000,
"type": "Ethernet"
},
"interface": "eth0",
"ip": [
{
"broadcast": "",
"flags": [
"PERMANENT"
],
"ip": "192.168.100.2/24",
"masc": 24,
"peer": "",
"pref_lft": 0,
"type": "IPv4",
"valid_lft": 0
}
],
"neigh": [],
"operational_status": {
"link_mode": "DEFAULT",
"oper_state": "LOWER LAYER DOWN"
},
"protocols": {
"multicast": true,
"routing_ipv4": true
},
"routes": [
{
"destination": "192.168.100.0/24",
"gateway": "direct",
"metric": 0,
"table": 254,
"type": "UNICAST"
},
{
"destination": "192.168.100.2",
"gateway": "direct",
"metric": 0,
"table": 255,
"type": "LOCAL"
},
{
"destination": "192.168.100.255",
"gateway": "direct",
"metric": 0,
"table": 255,
"type": "BROADCAST"
}
]
}
````