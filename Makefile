.PHONY: all
all: test_assign1_1

test_assign1_1: test_assign1_1.c storage_mgr_com.c dberror.c
	gcc -o test1 test_assign1_1.c storage_mgr_com.c dberror.c -g

.PHONY: clean
clean:
	rm test_assign1_1