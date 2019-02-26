#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "list.h"

#include "common.h"

static uint16_t values[256];

static void list_merge(struct list_head *left, struct list_head *right)
{
    struct list_head sorted;
    struct list_head *lnode = NULL;
    struct list_head *rnode = right->next;
    struct list_head *safe = NULL;
    int cmpRes = 0;
    INIT_LIST_HEAD(&sorted);

#ifdef DEBUG
    puts("##### Now merging #####");
    struct listitem *item = NULL;
    list_for_each_entry (item, right, list) {
        printf("->%d\n", item->i);
    }
    puts("----- and -----");
    list_for_each_entry (item, left, list) {
        printf("->%d\n", item->i);
    }
    puts("##### End merging #####");
#endif

    list_for_each_safe (lnode, safe, left) {
        if (rnode == right)
            break;
        cmpRes = cmpint(&list_entry(lnode, struct listitem, list)->i,
                        &list_entry(rnode, struct listitem, list)->i);
        if (cmpRes < 0) {
            list_del(lnode);
            list_add_tail(lnode, &sorted);
        } else {
            safe = rnode->next;
            list_del(rnode);
            list_add_tail(rnode, &sorted);
            rnode = safe;
            safe = lnode;  // rollback lnode since the lnode pointer does not go
                           // forward
        }
    }

    // Handle the remaining right part
    while (rnode != right) {
        safe = rnode->next;
        list_del(rnode);
        list_add_tail(rnode, &sorted);
        rnode = safe;
    }

    // Handle the remaining left part
    while (lnode != left) {
        safe = lnode->next;
        list_del(lnode);
        list_add_tail(lnode, &sorted);
        lnode = safe;
    }


    INIT_LIST_HEAD(right);
    list_splice(&sorted, right);
}

static void list_split(struct list_head *head)
{
    if (list_is_singular(head)) {
        return;
    }

    struct list_head *search_for = head->next;
    struct list_head *search_bak = head->prev;

    // The right/left part of the splitted list
    struct list_head *right = NULL;
    struct list_head *left =
        (struct list_head *) malloc(sizeof(struct list_head));

    // find the middle element
    while (1) {
        // two conditions
        if (search_for == search_bak || search_for->next == search_bak)
            break;
        search_for = search_for->next;
        search_bak = search_bak->prev;
    }

    // split to two parts
    list_cut_position(left, head, search_for);
    right = head;

    list_split(left);
    list_split(right);
    list_merge(left, right);

    free(left);

#ifdef DEBUG
    puts("##### Merge result #####");
    struct listitem *item = NULL;
    list_for_each_entry (item, right, list) {
        printf("->%d\n", item->i);
    }
    puts("########################");
#endif
}

static void list_mergesort(struct list_head *head)
{
    list_split(head);
}

int main(void)
{
    struct list_head testlist;
    struct listitem *item = NULL, *is = NULL;
    size_t i;

    random_shuffle_array(values, (uint16_t) ARRAY_SIZE(values));

    INIT_LIST_HEAD(&testlist);

    assert(list_empty(&testlist));

    for (i = 0; i < ARRAY_SIZE(values); i++) {
        item = (struct listitem *) malloc(sizeof(*item));
        assert(item);
        item->i = values[i];
        list_add_tail(&item->list, &testlist);
    }

    assert(!list_empty(&testlist));

    qsort(values, ARRAY_SIZE(values), sizeof(values[0]), cmpint);
    list_mergesort(&testlist);

    i = 0;
    list_for_each_entry_safe (item, is, &testlist, list) {
        assert(item->i == values[i]);
        list_del(&item->list);
        free(item);
        i++;
    }

    assert(i == ARRAY_SIZE(values));
    assert(list_empty(&testlist));

    return 0;
}
