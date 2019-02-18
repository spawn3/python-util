#include "stdio.h"
#include "stdlib.h"
#include "assert.h"

#include "resume.h"

enum {
    COLOR_WHITE = 0,
    COLOR_GRAY = 1,
    COLOR_BLACK = 2,
} ThreeColor;

typedef struct __glist_node {
    int vnumber;
    struct __glist_node *next;
} glist_node;

typedef struct __graph_node {
    glist_node *head, *tail;
    int vnumber;
    int visit;
    int weight;
    int prev;
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
        v->visit = COLOR_WHITE;
        v->weight = 0;
        v->prev = -1;
    }
}

void graph_dfs_impl(graph_t *g, int x) {
    printf("dfs %d\n", x);

    graph_node *v = &g->nodes[x];
    assert(v->vnumber != -1);
    v->visit = COLOR_GRAY;

    glist_node *adj = v->head;
    while (adj != NULL) {
        if (g->nodes[adj->vnumber].visit == COLOR_WHITE) {
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

void graph_bfs_impl(graph_t *g, int x) {
    queue_t q;
    queue_init(&q);

    graph_node *v = &g->nodes[x];
    queue_push(&q, v);
    v->weight = 0;
    v->prev = -1;
    v->visit = COLOR_GRAY;

    graph_node *adjv;
    while (!queue_empty(&q)) {
        v = queue_pop(&q);
        assert(v->vnumber != -1);
        // !!!required
        // if (v->visit)
        //     continue;

        printf("bfs %d prev %d weight %d\n", v->vnumber, v->prev, v->weight);

        glist_node *adj = v->head;
        while (adj != NULL) {
            adjv = &g->nodes[adj->vnumber];
            if (adjv->visit == COLOR_WHITE) {
                printf("-- %d %d\n", v->vnumber, adj->vnumber);
                adjv->prev = v->vnumber;
                adjv->weight = v->weight + 1;
                adjv->visit = COLOR_GRAY;
                queue_push(&q, adjv);
            }
            adj = adj->next;
        }

        v->visit = COLOR_BLACK;
    }
}

void graph_bfs(graph_t *g, int x) {
    assert(x >= 0 && x < g->V);

    graph_node *start = &g->nodes[x];
    if (start->vnumber == -1) {
        printf("node %d not found\n", x);
        assert(0);
    }

    graph_clear_visit(g);
    graph_bfs_impl(g, x);
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
    graph_bfs(&g, 0);
}
