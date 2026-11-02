/*
 * Campus Route Finder Using Graphs
 * Name   : Arnav
 * Course : B.Tech CSE (CGC University, Mohali)
 *
 * Campus locations = vertices, paths = edges (undirected, unweighted).
 * Features: adjacency list, adjacency matrix, BFS shortest path (min edges),
 * DFS traversal, reachability, disconnected locations, test cases.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_V    32
#define NAME_LEN 32

/* Adjacency list node: one entry per neighbour */
typedef struct AdjNode {
    int v;
    struct AdjNode *next;
} AdjNode;

typedef struct {
    int      n;                       /* number of vertices */
    char     names[MAX_V][NAME_LEN];  /* index -> location name */
    AdjNode *head[MAX_V];             /* adjacency list heads */
    AdjNode *tail[MAX_V];             /* tails keep insertion order */
    int      matrix[MAX_V][MAX_V];    /* adjacency matrix */
} CampusGraph;

/* ---------- Graph construction ---------- */
static int find_location(const CampusGraph *g, const char *name) {
    for (int i = 0; i < g->n; i++)
        if (strcmp(g->names[i], name) == 0) return i;
    return -1;
}

static int add_location(CampusGraph *g, const char *name) {
    int i = find_location(g, name);
    if (i >= 0) return i;
    if (g->n >= MAX_V) { fprintf(stderr, "Too many locations\n"); exit(1); }
    strncpy(g->names[g->n], name, NAME_LEN - 1);
    g->names[g->n][NAME_LEN - 1] = '\0';
    g->head[g->n] = g->tail[g->n] = NULL;
    return g->n++;
}

static int has_edge_list(const CampusGraph *g, int a, int b) {
    for (AdjNode *p = g->head[a]; p; p = p->next)
        if (p->v == b) return 1;
    return 0;
}

static void append_neighbour(CampusGraph *g, int a, int b) {
    AdjNode *node = (AdjNode *)malloc(sizeof(AdjNode));
    if (!node) { fprintf(stderr, "Out of memory\n"); exit(1); }
    node->v = b; node->next = NULL;
    if (g->tail[a]) g->tail[a]->next = node; else g->head[a] = node;
    g->tail[a] = node;
}

/* Add an undirected edge between two locations. */
static void add_path(CampusGraph *g, const char *a, const char *b) {
    int u = add_location(g, a), v = add_location(g, b);
    if (!has_edge_list(g, u, v)) {
        append_neighbour(g, u, v);
        append_neighbour(g, v, u);
    }
}

/* Build adjacency matrix from the adjacency list. */
static void build_matrix(CampusGraph *g) {
    memset(g->matrix, 0, sizeof(g->matrix));
    for (int a = 0; a < g->n; a++)
        for (AdjNode *p = g->head[a]; p; p = p->next)
            g->matrix[a][p->v] = 1;
}

static void free_graph(CampusGraph *g) {
    for (int i = 0; i < g->n; i++) {
        AdjNode *p = g->head[i];
        while (p) { AdjNode *t = p->next; free(p); p = t; }
        g->head[i] = g->tail[i] = NULL;
    }
}

/* ---------- Display ---------- */
static void show_adj_list(const CampusGraph *g) {
    printf("\nADJACENCY LIST\n");
    for (int i = 0; i < 60; i++) putchar('-');
    putchar('\n');
    for (int i = 0; i < g->n; i++) {
        printf("%-14s -> ", g->names[i]);
        for (AdjNode *p = g->head[i]; p; p = p->next)
            printf("%s%s", g->names[p->v], p->next ? ", " : "");
        putchar('\n');
    }
}

static void show_adj_matrix(CampusGraph *g) {
    build_matrix(g);
    printf("\nADJACENCY MATRIX  (1 = path exists, 0 = no path)\n");
    for (int i = 0; i < 60; i++) putchar('-');
    putchar('\n');
    for (int i = 0; i < g->n; i++) printf("  [%d] %s\n", i, g->names[i]);
    printf("\n    ");
    for (int i = 0; i < g->n; i++) printf(" %-3d", i);
    putchar('\n');
    for (int i = 0; i < g->n; i++) {
        printf("[%d]  ", i);
        for (int j = 0; j < g->n; j++) printf("%-3d%s", g->matrix[i][j], j + 1 < g->n ? " " : "");
        putchar('\n');
    }
}

/* ---------- BFS : shortest route (min edges) ----------
 * Fills path[] (source ... dest) and returns the number of edges,
 * or -1 if either location is invalid or dest is unreachable. */
static int bfs_route(const CampusGraph *g, int src, int dest, int *path, int *plen) {
    if (src < 0 || dest < 0 || src >= g->n || dest >= g->n) { *plen = 0; return -1; }
    int visited[MAX_V] = {0}, parent[MAX_V], queue[MAX_V];
    int qh = 0, qt = 0;
    for (int i = 0; i < g->n; i++) parent[i] = -2;   /* -2 = never discovered */
    visited[src] = 1; parent[src] = -1;              /* -1 = no parent (source) */
    queue[qt++] = src;
    while (qh < qt) {
        int node = queue[qh++];
        if (node == dest) break;
        for (AdjNode *p = g->head[node]; p; p = p->next) {
            if (!visited[p->v]) {
                visited[p->v] = 1;
                parent[p->v] = node;
                queue[qt++] = p->v;
            }
        }
    }
    if (parent[dest] == -2) { *plen = 0; return -1; }
    /* path reconstruction using parent / predecessor */
    int tmp[MAX_V], k = 0;
    for (int cur = dest; cur != -1; cur = parent[cur]) tmp[k++] = cur;
    for (int i = 0; i < k; i++) path[i] = tmp[k - 1 - i];   /* reverse */
    *plen = k;
    return k - 1;
}

/* ---------- DFS ---------- */
static void dfs_visit(const CampusGraph *g, int node, int *visited, int *order, int *cnt) {
    visited[node] = 1;
    order[(*cnt)++] = node;
    for (AdjNode *p = g->head[node]; p; p = p->next)
        if (!visited[p->v]) dfs_visit(g, p->v, visited, order, cnt);
}

/* Recursive DFS: fills order[] with all locations reachable from start. */
static int dfs_reachable(const CampusGraph *g, int start, int *order) {
    int visited[MAX_V] = {0}, cnt = 0;
    if (start >= 0 && start < g->n) dfs_visit(g, start, visited, order, &cnt);
    return cnt;
}

/* Iterative DFS (explicit stack): is dest reachable from src? */
static int dfs_is_reachable(const CampusGraph *g, int src, int dest) {
    if (src < 0 || dest < 0 || src >= g->n || dest >= g->n) return 0;
    int visited[MAX_V] = {0}, stack[MAX_V * MAX_V], top = 0;
    stack[top++] = src;
    while (top > 0) {
        int node = stack[--top];
        if (node == dest) return 1;
        if (!visited[node]) {
            visited[node] = 1;
            int nb[MAX_V], c = 0;
            for (AdjNode *p = g->head[node]; p; p = p->next) nb[c++] = p->v;
            for (int i = c - 1; i >= 0; i--)          /* reversed: keeps list order */
                if (!visited[nb[i]]) stack[top++] = nb[i];
        }
    }
    return 0;
}

static void print_names(const CampusGraph *g, const int *idx, int n, const char *sep) {
    for (int i = 0; i < n; i++) printf("%s%s", g->names[idx[i]], i + 1 < n ? sep : "");
}

/* Locations that cannot be reached from start. */
static int disconnected_from(const CampusGraph *g, int start, int *out) {
    int order[MAX_V], reach[MAX_V] = {0}, c = 0;
    int cnt = dfs_reachable(g, start, order);
    for (int i = 0; i < cnt; i++) reach[order[i]] = 1;
    for (int i = 0; i < g->n; i++) if (!reach[i]) out[c++] = i;
    return c;
}

static void print_components(const CampusGraph *g) {
    int seen[MAX_V] = {0}, order[MAX_V], k = 0;
    for (int i = 0; i < g->n; i++) {
        if (seen[i]) continue;
        int cnt = dfs_reachable(g, i, order);
        for (int j = 0; j < cnt; j++) seen[order[j]] = 1;
        printf("  %d) ", ++k);
        print_names(g, order, cnt, ", ");
        putchar('\n');
    }
}

static void print_comp_count(const CampusGraph *g) {
    int seen[MAX_V] = {0}, order[MAX_V], k = 0;
    for (int i = 0; i < g->n; i++) {
        if (seen[i]) continue;
        int cnt = dfs_reachable(g, i, order);
        for (int j = 0; j < cnt; j++) seen[order[j]] = 1;
        k++;
    }
    printf("  Total connected components: %d\n", k);
}

/* ---------- Campus data ---------- */
static void build_campus(CampusGraph *g) {
    static const char *edges[][2] = {
        {"Main Gate", "Parking"}, {"Main Gate", "Library"},
        {"Main Gate", "Block 1"}, {"Library", "Block 1"},
        {"Library", "Cafeteria"}, {"Block 1", "Block 2"},
        {"Block 2", "Lab"},       {"Cafeteria", "Auditorium"},
        {"Auditorium", "Lab"},
        /* separate (disconnected) part of the campus */
        {"Hostel", "Sports Complex"},
    };
    memset(g, 0, sizeof(*g));
    for (size_t i = 0; i < sizeof(edges) / sizeof(edges[0]); i++)
        add_path(g, edges[i][0], edges[i][1]);
}

static void show_route(const CampusGraph *g, int s, int d) {
    int path[MAX_V], plen;
    int steps = bfs_route(g, s, d, path, &plen);
    if (steps < 0) {
        printf("No route found from %s to %s (unreachable / invalid).\n", g->names[s], g->names[d]);
    } else {
        printf("BFS Route : ");
        print_names(g, path, plen, " -> ");
        printf("\nMinimum edges : %d\n", steps);
    }
}

/* ---------- Test cases ---------- */
static void run_one_test(const CampusGraph *g, const char *label, const char *s, const char *d,
                         const char **expected, int exp_len) {
    int path[MAX_V], plen;
    int si = find_location(g, s), di = find_location(g, d);
    int steps = bfs_route(g, si, di, path, &plen);
    int pass;
    printf("\n%s: %s -> %s\n", label, s, d);

    printf("  Expected : ");
    if (exp_len == 0) printf("No route (NULL)");
    for (int i = 0; i < exp_len; i++) printf("%s%s", expected[i], i + 1 < exp_len ? " -> " : "");
    printf("\n  Actual   : ");
    if (steps < 0) printf("No route (NULL)");
    else print_names(g, path, plen, " -> ");
    putchar('\n');

    if (exp_len == 0) pass = (steps < 0);
    else {
        pass = (steps >= 0 && plen == exp_len);
        for (int i = 0; pass && i < exp_len; i++)
            if (strcmp(g->names[path[i]], expected[i]) != 0) pass = 0;
    }
    printf("  Result   : %s\n", pass ? "PASS" : "FAIL");
}

static void run_test_cases(const CampusGraph *g) {
    printf("\n============================================================\n");
    printf("TEST CASES\n");
    printf("============================================================\n");
    const char *e1[] = {"Main Gate", "Library"};
    const char *e2[] = {"Main Gate", "Block 1", "Block 2", "Lab"};
    const char *e4[] = {"Library"};
    run_one_test(g, "TC1 Direct route",               "Main Gate", "Library", e1, 2);
    run_one_test(g, "TC2 Multi-stop route",           "Main Gate", "Lab",     e2, 4);
    run_one_test(g, "TC3 Unreachable destination",    "Main Gate", "Hostel",  NULL, 0);
    run_one_test(g, "TC4 Same source & destination",  "Library",   "Library", e4, 1);
}

/* ---------- Input ---------- */
static int pick(const CampusGraph *g, const char *prompt) {
    char buf[64];
    printf("%s", prompt);
    if (!fgets(buf, sizeof(buf), stdin)) return -1;
    buf[strcspn(buf, "\r\n")] = '\0';
    /* trim leading/trailing spaces */
    char *s = buf; while (*s == ' ') s++;
    size_t len = strlen(s); while (len && s[len - 1] == ' ') s[--len] = '\0';
    for (int i = 0; i < g->n; i++) {                 /* case-insensitive match */
        const char *a = g->names[i], *b = s;
        while (*a && *b && tolower((unsigned char)*a) == tolower((unsigned char)*b)) { a++; b++; }
        if (*a == '\0' && *b == '\0') return i;
    }
    return -1;
}

/* ---------- Menu ---------- */
int main(void) {
    CampusGraph g;
    build_campus(&g);
    char choice[16];

    for (;;) {
        printf("\n============================================================\n");
        printf("        CAMPUS ROUTE FINDER (Graph - BFS / DFS)\n");
        printf("============================================================\n");
        printf("1. Show all locations (Adjacency List)\n");
        printf("2. Show Adjacency Matrix\n");
        printf("3. Find route between two places (BFS)\n");
        printf("4. Show reachable locations from a start (DFS)\n");
        printf("5. Check if destination is reachable (DFS)\n");
        printf("6. Show disconnected locations / components\n");
        printf("7. Run test cases\n");
        printf("0. Exit\n");
        printf("Enter choice: ");
        if (!fgets(choice, sizeof(choice), stdin)) break;
        choice[strcspn(choice, "\r\n")] = '\0';
        int ch = atoi(choice);
        if (strcmp(choice, "0") != 0 && ch == 0) ch = -1;

        int s, d, order[MAX_V], other[MAX_V];
        switch (ch) {
        case 1: show_adj_list(&g); break;
        case 2: show_adj_matrix(&g); break;
        case 3:
            printf("Locations: ");
            for (int i = 0; i < g.n; i++)
                printf("%s%s", g.names[i], i + 1 < g.n ? ", " : "");
            putchar('\n');
            s = pick(&g, "Source: "); d = pick(&g, "Destination: ");
            if (s >= 0 && d >= 0) show_route(&g, s, d);
            else printf("Invalid location name!\n");
            break;
        case 4:
            s = pick(&g, "Start location: ");
            if (s >= 0) {
                int cnt = dfs_reachable(&g, s, order);
                printf("DFS reachable : ");
                print_names(&g, order, cnt, " -> ");
                putchar('\n');
            } else printf("Invalid location name!\n");
            break;
        case 5:
            s = pick(&g, "Source: "); d = pick(&g, "Destination: ");
            if (s >= 0 && d >= 0)
                printf("%s is %s from %s\n", g.names[d],
                       dfs_is_reachable(&g, s, d) ? "REACHABLE" : "NOT reachable", g.names[s]);
            else printf("Invalid location name!\n");
            break;
        case 6:
            s = pick(&g, "Start location: ");
            if (s >= 0) {
                int cnt = dfs_reachable(&g, s, order);
                int oc = disconnected_from(&g, s, other);
                printf("Reachable    : ");
                print_names(&g, order, cnt, ", ");
                printf("\nDisconnected : ");
                if (oc == 0) printf("None");
                else print_names(&g, other, oc, ", ");
                printf("\nComponents   :\n");
                print_components(&g);
                print_comp_count(&g);
            } else printf("Invalid location name!\n");
            break;
        case 7: run_test_cases(&g); break;
        case 0:
            printf("Thank you!\n");
            free_graph(&g);
            return 0;
        default:
            printf("Wrong choice, try again.\n");
        }
    }
    free_graph(&g);
    return 0;
}
