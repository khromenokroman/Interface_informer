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

Библиотека `interface_informer` предоставляет программный интерфейс (API) для получения подробной информации о сетевых 
интерфейсах в операционной системе Linux. Библиотека использует технологию Netlink для сбора 
детальных данных о конфигурации сетевых интерфейсов, их адресах, маршрутах и соседях в 
форматированном JSON представлении. Поддерживается работа как с интерфейсами в основном пространстве 
имен сети, так и с интерфейсами в других доступных сетевых пространствах имен (network namespaces). 



### Возможности

- **Получение структурированной информации о сетевых интерфейсах**:
  - Базовые характеристики (идентификатор, статус, тип интерфейса, флаги)
  - Аппаратные свойства (тип физического интерфейса, MAC-адрес, MTU, размер очереди передачи)
  - Оперативный статус и режим работы соединения
  - IP-адреса (поддержка IPv4 и IPv6) с дополнительной информацией (флаги, срок действия, широковещательные адреса)
  - Таблица соседей (кэш ARP/NDP) с информацией о состоянии связей
  - Таблица маршрутизации с детализацией по интерфейсам

- **Работа с сетевыми пространствами имен**:
  - Перечисление доступных сетевых пространств имен системы
  - Переключение между пространствами имен для сбора информации
  - Возможность последовательной работы с несколькими пространствами имен

- **Программный интерфейс**:
  - Получение всех доступных интерфейсов системы или указанного пространства имен
  - Получение полной детализированной информации о конкретном интерфейсе
  - Данные представлены в удобном для обработки формате JSON
  - Исключения для обработки ошибок с информативными сообщениями
  - Реализация с использованием современных возможностей C++20

### Зависимости

- C++20
- libnl-3
- libnl-route-3
- fmt

### Опции сборки

Проект предоставляет следующие опции сборки через CMake:

- **BUILD_SHARED_LIBS** - выбор типа библиотеки для сборки:
  - `ON` (по умолчанию) - сборка динамической библиотеки (`.so`)
  - `OFF` - сборка статической библиотеки (`.a`)

- **BUILD_EXAMPLE** - включение/отключение сборки примера:
  - `ON` (по умолчанию) - собирать пример использования библиотеки
  - `OFF` - не собирать пример

### Сборка

Проект использует CMake для сборки:

````bash
mkdir build
cd build
cmake ..
make
````

#### Сборка только статической библиотеки
```bash
mkdir build
cd build
cmake -DBUILD_SHARED_LIBS=OFF ..
make
```

#### Сборка без примера
```bash
mkdir build
cd build
cmake -DBUILD_EXAMPLE=OFF ..
make
```

#### Сборка только статической библиотеки без примера
```bash
mkdir build
cd build
cmake -DBUILD_SHARED_LIBS=OFF -DBUILD_EXAMPLE=OFF ..
make
```

#### Для создания пакета DEB:

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

При сборке динамической библиотеки создаются два пакета:
- `libinterface_informer0` - содержит саму динамическую библиотеку
- `libinterface_informer0-dev` - содержит заголовочные файлы и символьные ссылки для разработки

При сборке статической библиотеки создается только один пакет:
- `libinterface_informer0-dev` - содержит статическую библиотеку и заголовочные файлы


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

Для того чтобы не запускать программу от **root** можно сделать так
````bash
setcap cap_net_admin,cap_sys_admin+ep ./src/example/example
````
### Примеры работы

````
f-15% ./src/example/example
All namespaces:
{
    "namespaces": [
        "sample"
    ]
}

All interfaces (main):
{
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
All interfaces namespace 'sample':
{
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