#include <boost/asio/io_service.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <boost/algorithm/string.hpp>
#include <string>
#include <vector>
#include <iostream>

#define PRINT(x)  std::cout << #x << " " << x << std::endl

using DbusEventHandlerList =
    std::vector<std::unique_ptr<sdbusplus::bus::match_t>>;

constexpr auto dbusServiceList = {
    "xyz.openbmc_project.GpuMgr",
    "xyz.openbmc_project.GpioStatusHandler",
    "xyz.openbmc_project.Tsm",
};

boost::asio::io_service io;
std::shared_ptr<sdbusplus::asio::dbus_interface> iface  = nullptr;
std::shared_ptr<sdbusplus::asio::connection> systemBus = nullptr;


void dbusEventHandlerCallback(sdbusplus::message::message& msg)
{
    boost::container::flat_map<std::string, std::variant<double>>  propertiesChanged;
    std::string msgInterface;
    msg.read(msgInterface, propertiesChanged);

    std::string objectPath = msg.get_path();
    std::string signalSignature = msg.get_signature();
    std::string interface = msg.get_interface();
    std::string member = msg.get_member();
//    std::string destination = msg.get_destination();
    std::string sender      = msg.get_sender();

    PRINT(objectPath);
    PRINT(interface);
    PRINT(member);
//    PRINT(destination);
    PRINT(sender);
    PRINT(signalSignature);
}


DbusEventHandlerList setHandler(sdbusplus::bus::bus& bus)
{
    auto genericHandler = std::bind(dbusEventHandlerCallback, std::placeholders::_1);

    DbusEventHandlerList handlerList;

    for (auto& service : dbusServiceList)
    {
         std::string setPropertiesChanged =  sdbusplus::bus::match::rules::type::signal() +
                sdbusplus::bus::match::rules::sender(service) +
                sdbusplus::bus::match::rules::member("PropertiesChanged") +
                sdbusplus::bus::match::rules::interface("org.freedesktop.DBus.Properties");

        handlerList.push_back(std::make_unique<sdbusplus::bus::match_t>(bus, setPropertiesChanged, genericHandler));
    }
    return handlerList;
}


int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;
    systemBus = std::make_shared<sdbusplus::asio::connection>(io);
    auto conn = systemBus.get();
    sdbusplus::bus::bus& bus = static_cast<sdbusplus::bus::bus&>(*conn);
    sdbusplus::asio::object_server objServer(systemBus);

    auto handlers = setHandler(bus);
    io.run();
    return 0;
}

