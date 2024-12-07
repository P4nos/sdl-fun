#include "physics.h"
#include "util.h"

extern State state;

// Function to add a node to the end of the list
void add_node(Circle c) {
  Node *newNode = (Node *)malloc(sizeof(Node));
  if (!newNode) {
    printf("Memory allocation failed\n");
    exit(1);
  }

  newNode->object = c;
  newNode->next = NULL;

  if (state.head == NULL) {
    state.head = newNode;
  } else {
    Node *temp = state.head;
    // move at the end of the list and append the new node
    while (temp->next != NULL) {
      temp = temp->next;
    }
    temp->next = newNode;
  }
}

void reset_state() {
  Node *temp;
  while (state.head != NULL) {
    temp = state.head;
    state.head = state.head->next;
    free(temp);
  }
}

void update_state() {
  Node *temp = state.head;
  while (temp != NULL) {
    // calculate new location for object
    calculate_location(temp);
    // detect collisions
    handle_object_collisions(temp);
    handle_border_collisions(temp);
    temp = temp->next;
  }
}

void init_state() {
  for (int i = 0; i < NUM_CIRCLES; i++) {
    Circle c = {.ycenter = (float)rand_int_range(0, SCREEN_HEIGHT / 2),
                .xcenter = (float)rand_int_range(0, SCREEN_WIDTH),
                .radius = 2.0,
                .yvelocity = (float)rand_int_range(-200, 200),
                .xvelocity = (float)rand_int_range(-200, 200),
                .m = 1.0,
                .dx = 0.0,
                .dy = 0.0,
                .color = get_rand_color()};
    add_node(c);
  }
}
