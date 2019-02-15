#include "stdio.h"
#include "stdlib.h"
#include "assert.h"

#include "resume.h"

typedef struct __glist_node {
    int vnumber;
    struct __glist_node *next;
} glist_node;

typedef struct __graph_node {
    glist_node *head, *tail;
    int vnumber;
    int visit;
    // other properties
} graph_node;

typedef struct {
    int V;
    int E;
    graph_node *nodes;
} graph_t;

void graph_create(graph_t *g, int V) {
    g->V = V;
    g->nodes = malloc(V * sizeof(graph_node));
    assert(g->nodes != NULL);

    graph_node *v;
    for (int i=0; i < V; i++) {
        v = &g->nodes[i];
        v->head = NULL;
        v->tail = NULL;
        v->vnumber = -1;
        v->visit = 0;
    }
}

void graph_add_edge(graph_t *g, int x, int y) {
    assert(x >= 0 && x < g->V);
    assert(y >= 0 && y < g->V);

    graph_node *start = &g->nodes[x];
    if (start->vnumber == -1) {
        start->vnumber = x;
    }

    graph_node *end = &g->nodes[y];
    if (end->vnumber == -1) {
        end->vnumber = y;
    }

    glist_node *newv = malloc(sizeof(glist_node));
    newv->vnumber = y;
    newv->next = NULL;

    if (start->head == NULL) {
        start->head = newv;
        start->tail = newv;
    } else {
        start->tail->next = newv;
        start->tail = newv;
    }
}

void graph_clear_visit(graph_t *g) {
    graph_node *v;
    for (int i=0; i < g->V; i++) {
        v = &g->nodes[i];
        v->visit = 0;
    }
}

void graph_dfs_impl(graph_t *g, int x) {
    printf("dfs %d\n", x);

    graph_node *v = &g->nodes[x];
    v->visit = 1;

    glist_node *adj = v->head;
    while (adj != NULL) {
        if (!g->nodes[adj->vnumber].visit) {
            graph_dfs_impl(g, adj->vnumber);
        }
        adj = adj->next;
    }
}

void graph_dfs(graph_t *g, int x) {
    assert(x >= 0 && x < g->V);

    graph_node *start = &g->nodes[x];
    if (start->vnumber == -1) {
        printf("node %d not found\n", x);
        assert(0);
    }

    graph_clear_visit(g);
    graph_dfs_impl(g, x);
}

void test_graph() {
    graph_t g;
    graph_create(&g, 100);

    graph_add_edge(&g, 0, 1);
    graph_add_edge(&g, 0, 3);

    graph_add_edge(&g, 1, 2);
    graph_add_edge(&g, 1, 3);
    graph_add_edge(&g, 3, 4);


    graph_dfs(&g, 0);
}
