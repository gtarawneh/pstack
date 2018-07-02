// vim: set ft=cpp:

#include <stdio.h>
#include <queue>

void handler_log(int level, const char *msg, ...) {
    // do nothing
}

void handler_exit(int exitCode) {
    // do nothing
}

{% include 'messages.cpp' %}

{% include 'devices.cpp' %}

{{ graph_type['shared_code'] }}

{% include 'handlers.cpp' %}

int main() {

    {% include 'init.cpp' %}

    for (int i=0; i<5; i++){
        printf("Device %d:\n", i);
        deviceStates_node[i].print();
    }

    for (int i=0; i<5; i++){
        int rts = get_rts_node(deviceStates_node + i, deviceProperties_node + i);
        printf("rts[%d]: 0x%x\n", i, rts);
    }

    // print_state(&states[0]);

    // queue <state> q;
    // state *s1 = states;
    // s1->x = 5;
    // printf("queue size = %d\n", q.size());
    // q.push(*s1);
    // state t = q.front();
    // printf("queue size = %d\n", q.size());
    // q.pop();
    // printf("queue size = %d\n", q.size());
    // printf("hello %d\n", t.x);

    return 0;
}