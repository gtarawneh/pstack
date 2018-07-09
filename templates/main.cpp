// vim: set ft=cpp:

#include <set>
#include <queue>
#include <vector>
#include <string>
#include <stdio.h>
#include <unordered_map>

@ include 'types.cpp'
@ include 'globals.cpp'
@ include 'messages.cpp'
@ include 'devices.cpp'
@ include 'shared.cpp'
@ include 'handlers.cpp'
@ include 'init.cpp'
@ include 'connections.cpp'

typedef std::set<device_t*> rts_set_t;

void print_device_rts(device_t* dev) {

    int rts = (*dev).get_rts();
    int ports = (*dev).getOutputPortCount();

    for (int j=0; j<ports; j++) {

        if (rts & (1 << j)) {

            printf("Device <%s> requested to send on output port <%s>\n",
                dev->name.c_str(),
                (*dev).getOutputPortName(j)
            );

        }

    }

}

void print_rts_set(rts_set_t rts_set) {

    for (auto itr = rts_set.begin(); itr != rts_set.end(); ++itr)
        print_device_rts(*itr);

}

device_t* select_rts_device(rts_set_t rts_set) {

    return *(rts_set.begin());
}


int select_rts_port(device_t* dev) {

    int port_count = (*dev).getOutputPortCount();

    int rts = (*dev).get_rts();

    for (int j=0; j<port_count; j++) {

        if (rts & (1 << j)) return j;

    }

}

int main() {

    std::vector<device_t*> devices;
    std::vector<delivery_t> dlist; // delivery list

    @ set device_types = unique(graph_instance['devices'] | map(attribute='type'))
    @ set init_funcs = pymap(get_init_function_name, device_types)
    @ set init_calls = mformat("%s(devices);", init_funcs) | join("\n")

    {{ init_calls }}

    add_edges(devices);

    printf("----\n");

    // ---- BEGIN RTS SCAN ----

    rts_set_t rts_set;

    for (int i=0; i<devices.size(); i++){

        device_t* dev = devices[i];

        int rts = (*dev).get_rts();

        if (rts) rts_set.insert(dev);

    }

    print_rts_set(rts_set);

    printf("----\n");

    // ---- END RTS SCAN ----

    // ---- BEGIN DELIVERY LIST UPDATE ----

    device_t* dev = select_rts_device(rts_set);

    int port = select_rts_port(dev);

    dst_list_t *dests = (*dev).getPortDestinations(port);

    // Create message by calling send handler

    msg_t* msg = (*dev).send(port);

    // Create delivery object

    delivery_t new_dv = delivery_t(msg, *dests);

    new_dv.print();

    dlist.push_back(new_dv);

    printf("Delivery list size: %d item(s)\n", dlist.size());

    // ---- END DELIVERY LIST UPDATE ----

    // ---- BEGIN DELIVERY ----

    printf("Making the delivery:\n");

    delivery_t dv = dlist.at(0);  // Always choose first delivery, for now (TODO)

    dv.print();

    printf("Chosen destination:\n");

    destination_t dst = dv.dst.at(0);  // Always choose first destination, for now (TODO)

    dst.print();

    printf("Calling receive handler ...\n");

    device_t* dst_dev = (device_t*) dst.device;

    (*dst_dev).receive(dst.port, dv.msg);

    printf("Device <%s>: ", dst_dev->name.c_str());

    (*dst_dev).print();

    // ---- END DELIVERY ----

    return 0;
}