/*
 * Student Record Search System Using Binary Search Tree (BST)
 * Name   : Arnav
 * Course : B.Tech CSE (CGC University, Mohali)
 *
 * Each record: enrolment ID (BST key, unique), name, department.
 * Features: insert (duplicates rejected), search, delete (leaf / one child /
 * two children), inorder / preorder / postorder, height, comparison counting
 * for balanced vs sorted insertion order, menu-driven interface.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Student {
    int  enrol_id;          /* BST key (unique) */
    char name[32];
    char dept[16];
} Student;

typedef struct Node {
    Student data;
    struct Node *left;
    struct Node *right;
} Node;

typedef struct {
    Node *root;
    long  comparisons;      /* counts key comparisons (for analysis) */
} StudentBST;

/* ---------- Helpers ---------- */
static Student make_student(int id, const char *name, const char *dept) {
    Student s;
    s.enrol_id = id;
    strncpy(s.name, name, sizeof(s.name) - 1);  s.name[sizeof(s.name) - 1] = '\0';
    strncpy(s.dept, dept, sizeof(s.dept) - 1);  s.dept[sizeof(s.dept) - 1] = '\0';
    return s;
}

static void print_student(const Student *s) {
    printf("  ID: %d | Name: %-10s | Dept: %s\n", s->enrol_id, s->name, s->dept);
}

static void tree_init(StudentBST *t) { t->root = NULL; t->comparisons = 0; }

static void free_nodes(Node *n) {
    if (n) { free_nodes(n->left); free_nodes(n->right); free(n); }
}

static int count_nodes(Node *n) {
    return n ? 1 + count_nodes(n->left) + count_nodes(n->right) : 0;
}

/* ---------- Search ---------- */
static Node *search(StudentBST *t, int id, int count) {
    Node *cur = t->root;
    while (cur) {
        if (count) t->comparisons++;
        if (id == cur->data.enrol_id) return cur;
        cur = (id < cur->data.enrol_id) ? cur->left : cur->right;
    }
    return NULL;
}

/* ---------- Insert ---------- */
static Node *insert_node(StudentBST *t, Node *node, Student s) {
    if (node == NULL) {
        Node *n = (Node *)malloc(sizeof(Node));
        if (!n) { fprintf(stderr, "Out of memory\n"); exit(1); }
        n->data = s; n->left = n->right = NULL;
        return n;
    }
    t->comparisons++;
    if (s.enrol_id < node->data.enrol_id)
        node->left = insert_node(t, node->left, s);
    else
        node->right = insert_node(t, node->right, s);
    return node;
}

/* Returns 0 for a duplicate enrolment ID, 1 on success. */
static int insert(StudentBST *t, Student s) {
    if (search(t, s.enrol_id, 0)) {
        printf("  [!] Duplicate ID %d: record rejected.\n", s.enrol_id);
        return 0;
    }
    t->root = insert_node(t, t->root, s);
    return 1;
}

/* ---------- Delete ---------- */
static Node *delete_node(Node *node, int key, int quiet) {
    if (node == NULL) return NULL;
    if (key < node->data.enrol_id) {
        node->left = delete_node(node->left, key, quiet);
    } else if (key > node->data.enrol_id) {
        node->right = delete_node(node->right, key, quiet);
    } else {
        /* Case 1: leaf node */
        if (node->left == NULL && node->right == NULL) {
            if (!quiet) printf("  (Case 1: leaf node deleted)\n");
            free(node);
            return NULL;
        }
        /* Case 2: one child */
        if (node->left == NULL) {
            Node *child = node->right;
            if (!quiet) printf("  (Case 2: node with one (right) child deleted)\n");
            free(node);
            return child;
        }
        if (node->right == NULL) {
            Node *child = node->left;
            if (!quiet) printf("  (Case 2: node with one (left) child deleted)\n");
            free(node);
            return child;
        }
        /* Case 3: two children -> replace with inorder successor */
        Node *succ = node->right;
        while (succ->left) succ = succ->left;
        if (!quiet)
            printf("  (Case 3: two children; replaced by inorder successor %d)\n",
                   succ->data.enrol_id);
        node->data = succ->data;
        node->right = delete_node(node->right, succ->data.enrol_id, 1);
    }
    return node;
}

/* Returns 1 if deleted, 0 if the ID was not found. */
static int delete_id(StudentBST *t, int id) {
    if (!search(t, id, 0)) {
        printf("  [!] ID %d not found. Nothing deleted.\n", id);
        return 0;
    }
    t->root = delete_node(t->root, id, 0);
    return 1;
}

/* ---------- Traversals (collect pointers into out[]) ---------- */
static void inorder(Node *n, Student **out, int *k) {
    if (n) { inorder(n->left, out, k); out[(*k)++] = &n->data; inorder(n->right, out, k); }
}
static void preorder(Node *n, Student **out, int *k) {
    if (n) { out[(*k)++] = &n->data; preorder(n->left, out, k); preorder(n->right, out, k); }
}
static void postorder(Node *n, Student **out, int *k) {
    if (n) { postorder(n->left, out, k); postorder(n->right, out, k); out[(*k)++] = &n->data; }
}

static int height(Node *n) {
    if (n == NULL) return 0;
    int l = height(n->left), r = height(n->right);
    return 1 + (l > r ? l : r);
}

typedef void (*TraverseFn)(Node *, Student **, int *);

static void show_traversal(StudentBST *t, const char *title, TraverseFn fn) {
    int n = count_nodes(t->root), k = 0;
    printf("%s\n", title);
    if (n == 0) { printf("  (tree is empty)\n"); return; }
    Student **out = (Student **)malloc(sizeof(Student *) * n);
    fn(t->root, out, &k);
    for (int i = 0; i < k; i++) print_student(out[i]);
    free(out);
}

static void keys_of(StudentBST *t, int *keys, int *n) {
    int cnt = count_nodes(t->root), k = 0;
    Student **out = (Student **)malloc(sizeof(Student *) * (cnt ? cnt : 1));
    inorder(t->root, out, &k);
    for (int i = 0; i < k; i++) keys[i] = out[i]->enrol_id;
    *n = k;
    free(out);
}

/* ---------- Sample data ---------- */
static void load_samples(StudentBST *t) {
    Student s[10] = {
        {2556447, "Dhruv",  "CSE"}, {2556420, "Aman",   "CSE"},
        {2556480, "Simran", "IT"},  {2556410, "Riya",   "ECE"},
        {2556430, "Karan",  "CSE"}, {2556460, "Neha",   "IT"},
        {2556490, "Rohan",  "ME"},  {2556405, "Pooja",  "CSE"},
        {2556425, "Vikas",  "ECE"}, {2556470, "Anjali", "CSE"},
    };
    for (int i = 0; i < 10; i++) insert(t, s[i]);
}

/* ---------- Balanced vs sorted experiment ---------- */
static int cmp_int(const void *a, const void *b) {
    int x = *(const int *)a, y = *(const int *)b;
    return (x > y) - (x < y);
}

static void comparison_experiment(void) {
    int ids[15] = {50, 30, 70, 20, 40, 60, 80, 10, 25, 35, 45, 55, 65, 75, 90};
    int sorted_ids[15];
    memcpy(sorted_ids, ids, sizeof(ids));
    qsort(sorted_ids, 15, sizeof(int), cmp_int);

    const char *labels[2] = {"Balanced-looking order", "Sorted order"};
    int *orders[2] = {ids, sorted_ids};
    long ins[2], srch[2]; int h[2];

    for (int r = 0; r < 2; r++) {
        StudentBST t; tree_init(&t);
        for (int i = 0; i < 15; i++) insert(&t, make_student(orders[r][i], "S", "X"));
        ins[r] = t.comparisons;
        t.comparisons = 0;
        for (int i = 0; i < 15; i++) search(&t, ids[i], 1);   /* search every key once */
        srch[r] = t.comparisons;
        h[r] = height(t.root);
        free_nodes(t.root);
    }
    printf("\nCOMPARISON: Balanced vs Sorted insertion (15 keys)\n");
    for (int i = 0; i < 66; i++) putchar('-');
    printf("\n%-24s%-14s%-14s%s\n", "Insertion order", "Insert cmp", "Search cmp", "Height");
    for (int r = 0; r < 2; r++)
        printf("%-24s%-14ld%-14ld%d\n", labels[r], ins[r], srch[r], h[r]);
    printf("Balanced tree height ~ log2(n); sorted input -> skewed tree, height = n.\n");
}

/* ---------- Test cases ---------- */
static const int TEST_IDS[8] = {50, 30, 70, 20, 40, 60, 80, 35};

static void fresh_tree(StudentBST *t) {
    tree_init(t);
    for (int i = 0; i < 8; i++) insert(t, make_student(TEST_IDS[i], "S", "X"));
}

static void print_int_list(const int *a, int n) {
    printf("[");
    for (int i = 0; i < n; i++) printf("%s%d", i ? ", " : "", a[i]);
    printf("]");
}

static int same_list(const int *a, int na, const int *b, int nb) {
    if (na != nb) return 0;
    for (int i = 0; i < na; i++) if (a[i] != b[i]) return 0;
    return 1;
}

static void run_test_cases(void) {
    printf("\n==============================================================\n"
           "TEST CASES\n"
           "==============================================================\n");
    int act[64], n;
    StudentBST t;

    /* TC1 */
    int e1[] = {30, 35, 40, 50, 60, 70, 80};
    fresh_tree(&t);
    printf("\nTC1 Delete LEAF node (20)\n");
    t.root = delete_node(t.root, 20, 0);
    keys_of(&t, act, &n);
    printf("  Expected inorder: "); print_int_list(e1, 7); printf("\n");
    printf("  Actual   inorder: "); print_int_list(act, n);
    printf("  -> %s\n", same_list(act, n, e1, 7) ? "PASS" : "FAIL");
    free_nodes(t.root);

    /* TC2 */
    int e2[] = {20, 30, 35, 50, 60, 70, 80};
    fresh_tree(&t);
    printf("\nTC2 Delete node with ONE child (40, child 35)\n");
    t.root = delete_node(t.root, 40, 0);
    keys_of(&t, act, &n);
    printf("  Expected inorder: "); print_int_list(e2, 7); printf("\n");
    printf("  Actual   inorder: "); print_int_list(act, n);
    printf("  -> %s\n", same_list(act, n, e2, 7) ? "PASS" : "FAIL");
    free_nodes(t.root);

    /* TC3 */
    int e3[] = {20, 30, 35, 40, 60, 70, 80};
    fresh_tree(&t);
    printf("\nTC3 Delete node with TWO children (50 = root)\n");
    t.root = delete_node(t.root, 50, 0);
    keys_of(&t, act, &n);
    printf("  Expected inorder: "); print_int_list(e3, 7); printf("   (new root = 60)\n");
    printf("  Actual   inorder: "); print_int_list(act, n);
    printf("  (new root = %d)  -> %s\n", t.root->data.enrol_id,
           (same_list(act, n, e3, 7) && t.root->data.enrol_id == 60) ? "PASS" : "FAIL");
    free_nodes(t.root);

    /* TC4 */
    fresh_tree(&t);
    printf("\nTC4 Duplicate insertion (30)\n");
    int r = insert(&t, make_student(30, "Dup", "X"));
    printf("  Expected: rejected (0) | Actual: %d -> %s\n", r, r == 0 ? "PASS" : "FAIL");

    /* TC5 */
    printf("\nTC5 Search unsuccessful (99)\n");
    Node *f = search(&t, 99, 1);
    printf("  Expected: NULL | Actual: %s -> %s\n", f ? "found" : "NULL", f == NULL ? "PASS" : "FAIL");
    free_nodes(t.root);
}

/* ---------- Input helper ---------- */
static int read_int(const char *prompt, int *out) {
    char buf[64], *end;
    printf("%s", prompt);
    if (!fgets(buf, sizeof(buf), stdin)) return 0;
    long v = strtol(buf, &end, 10);
    if (end == buf || (*end != '\n' && *end != '\0')) {
        printf("  [!] Enter numeric enrolment ID only.\n");
        return 0;
    }
    *out = (int)v;
    return 1;
}

static void read_line(const char *prompt, char *dst, size_t size) {
    printf("%s", prompt);
    if (!fgets(dst, (int)size, stdin)) { dst[0] = '\0'; return; }
    dst[strcspn(dst, "\r\n")] = '\0';
}

/* ---------- Menu ---------- */
int main(void) {
    StudentBST tree; tree_init(&tree);
    char choice[16];

    for (;;) {
        printf("\n==============================================================\n");
        printf("     STUDENT RECORD SEARCH SYSTEM (Binary Search Tree)\n");
        printf("==============================================================\n");
        printf("1. Insert student record\n");
        printf("2. Load 10 sample records\n");
        printf("3. Search by enrolment ID\n");
        printf("4. Delete by enrolment ID\n");
        printf("5. Inorder traversal (sorted order)\n");
        printf("6. Preorder traversal\n");
        printf("7. Postorder traversal\n");
        printf("8. Tree height\n");
        printf("9. Compare balanced vs sorted insertion\n");
        printf("10. Run test cases\n");
        printf("0. Exit\n");
        printf("Enter choice: ");
        if (!fgets(choice, sizeof(choice), stdin)) break;
        choice[strcspn(choice, "\r\n")] = '\0';
        int ch = atoi(choice), id;
        if (strcmp(choice, "0") != 0 && ch == 0) ch = -1;

        switch (ch) {
        case 1:
            if (read_int("Enrolment ID: ", &id)) {
                char name[32], dept[16];
                read_line("Name: ", name, sizeof(name));
                read_line("Department: ", dept, sizeof(dept));
                if (insert(&tree, make_student(id, name, dept)))
                    printf("  Record inserted successfully.\n");
            }
            break;
        case 2:
            load_samples(&tree);
            printf("  10 sample records loaded.\n");
            break;
        case 3:
            if (read_int("Enrolment ID to search: ", &id)) {
                Node *n = search(&tree, id, 1);
                if (n) { printf("  FOUND ->"); printf(" ID: %d | Name: %-10s | Dept: %s\n",
                                  n->data.enrol_id, n->data.name, n->data.dept); }
                else printf("  Record with ID %d NOT found.\n", id);
            }
            break;
        case 4:
            if (read_int("Enrolment ID to delete: ", &id) && delete_id(&tree, id))
                printf("  Record %d deleted.\n", id);
            break;
        case 5: show_traversal(&tree, "INORDER (sorted by ID):", inorder);   break;
        case 6: show_traversal(&tree, "PREORDER:", preorder);               break;
        case 7: show_traversal(&tree, "POSTORDER:", postorder);             break;
        case 8: printf("  Height of tree = %d\n", height(tree.root));        break;
        case 9: comparison_experiment();                                    break;
        case 10: run_test_cases();                                          break;
        case 0:
            printf("Thank you!\n");
            free_nodes(tree.root);
            return 0;
        default:
            printf("Wrong choice, try again.\n");
        }
    }
    free_nodes(tree.root);
    return 0;
}
