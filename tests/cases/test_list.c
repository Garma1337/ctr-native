/* Build a list of `n` items (items[0..n-1]) using AddBack; assert the spine. */
static void list_build(struct LinkedList *L, struct Item *items, int n)
{
	LIST_Clear(L);
	for (int i = 0; i < n; i++)
	{
		LIST_AddBack(L, &items[i]);
	}
}

UTEST(list, clear_zeroes_everything)
{
	struct LinkedList L = {.first = (struct Item *)0x1, .last = (struct Item *)0x2, .count = 99};
	LIST_Clear(&L);
	ASSERT_TRUE(L.first == NULL);
	ASSERT_TRUE(L.last == NULL);
	ASSERT_EQ(0, L.count);
}

UTEST(list, add_front_links_and_orders)
{
	struct LinkedList L;
	struct Item a, b, c;
	LIST_Clear(&L);

	LIST_AddFront(&L, &a); /* [a] */
	LIST_AddFront(&L, &b); /* [b, a] */
	LIST_AddFront(&L, &c); /* [c, b, a] */

	ASSERT_EQ(3, L.count);
	ASSERT_TRUE(L.first == &c);
	ASSERT_TRUE(L.last == &a);
	/* forward spine c -> b -> a */
	ASSERT_TRUE(c.next == &b);
	ASSERT_TRUE(b.next == &a);
	ASSERT_TRUE(a.next == NULL);
	/* back spine a -> b -> c */
	ASSERT_TRUE(a.prev == &b);
	ASSERT_TRUE(b.prev == &c);
	ASSERT_TRUE(c.prev == NULL);
}

UTEST(list, add_back_links_and_orders)
{
	struct LinkedList L;
	struct Item a, b, c;
	LIST_Clear(&L);
	LIST_AddBack(&L, &a);
	LIST_AddBack(&L, &b);
	LIST_AddBack(&L, &c); /* [a, b, c] */

	ASSERT_EQ(3, L.count);
	ASSERT_TRUE(L.first == &a);
	ASSERT_TRUE(L.last == &c);
	ASSERT_TRUE(a.next == &b);
	ASSERT_TRUE(b.next == &c);
	ASSERT_TRUE(c.next == NULL);
	ASSERT_TRUE(a.prev == NULL);
}

UTEST(list, add_null_is_noop)
{
	struct LinkedList L;
	struct Item a;
	LIST_Clear(&L);
	LIST_AddBack(&L, &a);

	LIST_AddFront(&L, NULL);
	LIST_AddBack(&L, NULL);

	ASSERT_EQ(1, L.count);
	ASSERT_TRUE(L.first == &a);
	ASSERT_TRUE(L.last == &a);
}

UTEST(list, get_first_and_next_traverse)
{
	struct LinkedList L;
	struct Item items[3];
	list_build(&L, items, 3);

	struct Item *it = (struct Item *)LIST_GetFirstItem(&L);
	ASSERT_TRUE(it == &items[0]);
	it = (struct Item *)LIST_GetNextItem(it);
	ASSERT_TRUE(it == &items[1]);
	it = (struct Item *)LIST_GetNextItem(it);
	ASSERT_TRUE(it == &items[2]);
	ASSERT_TRUE(LIST_GetNextItem(it) == NULL);
}

UTEST(list, remove_member_middle)
{
	struct LinkedList L;
	struct Item items[3];
	list_build(&L, items, 3); /* [0,1,2] */

	struct Item *removed = LIST_RemoveMember(&L, &items[1]);
	ASSERT_TRUE(removed == &items[1]);
	ASSERT_EQ(2, L.count);
	/* spine relinked: 0 -> 2 */
	ASSERT_TRUE(items[0].next == &items[2]);
	ASSERT_TRUE(items[2].prev == &items[0]);
	/* removed node is detached */
	ASSERT_TRUE(items[1].next == NULL);
	ASSERT_TRUE(items[1].prev == NULL);
}

UTEST(list, remove_member_head_and_tail)
{
	struct LinkedList L;
	struct Item items[3];
	list_build(&L, items, 3);

	LIST_RemoveMember(&L, &items[0]); /* head */
	ASSERT_TRUE(L.first == &items[1]);
	ASSERT_TRUE(items[1].prev == NULL);

	LIST_RemoveMember(&L, &items[2]); /* tail */
	ASSERT_TRUE(L.last == &items[1]);
	ASSERT_TRUE(items[1].next == NULL);
	ASSERT_EQ(1, L.count);
}

UTEST(list, remove_front_and_back)
{
	struct LinkedList L;
	struct Item items[3];
	list_build(&L, items, 3);

	ASSERT_TRUE(LIST_RemoveFront(&L) == &items[0]);
	ASSERT_TRUE(LIST_RemoveBack(&L) == &items[2]);
	ASSERT_EQ(1, L.count);
	ASSERT_TRUE(L.first == &items[1]);
	ASSERT_TRUE(L.last == &items[1]);
}

UTEST(list, remove_from_empty_returns_null)
{
	struct LinkedList L;
	LIST_Clear(&L);
	ASSERT_TRUE(LIST_RemoveFront(&L) == NULL);
	ASSERT_TRUE(LIST_RemoveBack(&L) == NULL);
	ASSERT_TRUE(LIST_RemoveMember(&L, NULL) == NULL);
}

UTEST(list, init_builds_list_from_array)
{
	struct LinkedList L;
	struct Item items[4];
	LIST_Clear(&L);

	LIST_Init(&L, items, (int)sizeof(struct Item), 4);

	ASSERT_EQ(4, L.count);
	ASSERT_TRUE(L.first == &items[0]);
	ASSERT_TRUE(L.last == &items[3]);
	ASSERT_TRUE(items[0].next == &items[1]);
	ASSERT_TRUE(items[3].next == NULL);
}
