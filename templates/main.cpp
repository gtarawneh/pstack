#include <stdio.h>
#include <queue>

#define DEVICE_COUNT 10

{% for device in graph_type['device_types'] %}
{% set CLASS_NAME = device['id'] + "_t" %}
class {{ CLASS_NAME }} {

public:

    {% for scalar in device['state']['scalars'] %}
    {{- scalar['type'] }} {{ scalar['name'] }};
    {% endfor %}

    {{ CLASS_NAME }} (){
        // Default constructor
        {% for scalar in device['state']['scalars'] %}
        {{- scalar['name'] }} = 0;
        {% endfor %}
    }

    void print() {
        {% for scalar in device['state']['scalars'] %}
        printf("{{ scalar['name']}} = %d\n", this->{{ scalar['name']}});
        {% endfor %}
    }
};
{% endfor %}

class msg_t {

public:

    int content;

    void print() {
        printf("content = %d\n", this->content);
    }
};


void receive(node_t *state, msg_t *msg) {
    printf("I received a message\n");
    (*msg).print();
    // state->counter += msg->content;
}

msg_t get_trigger() {
    msg_t result;
    result.content = 1;
    return result;
}

int main() {

    node_t states[DEVICE_COUNT];

    msg_t trigger = get_trigger();

    receive(&states[0], &trigger);

    states[0].print();

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