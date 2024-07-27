#include "physics.h"

extern State state;

int randInt(int min, int max) {
  return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}

// Function to add a node to the end of the list
void add_node() {
  Node *newNode = (Node *)malloc(sizeof(Node));
  if (!newNode) {
    printf("Memory allocation failed\n");
    exit(1);
  }

  newNode->object = (Circle){.ycenter = (float)randInt(0, SCREEN_HEIGHT / 2),
                             .xcenter = (float)randInt(0, SCREEN_WIDTH),
                             .radius = 10.0,
                             .yvelocity = 0.0,
                             .dx = 0.0,
                             .dy = 0.0};
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
    // detect collisions
    // calculate new location for object
    calculate_location(temp);
    temp = temp->next;
  }
}

void init_state() {
  for (int i = 0; i < NUM_CIRCLES; i++) {
    add_node();
  }
}
