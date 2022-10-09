test-dd :
	gcc src/test_dd.c src/dd_*c -g -Wall -Wextra -o bin/test_dd
	valgrind --leak-check=yes ./bin/test_dd
	rm ./bin/test_dd

test-te :
	gcc src/test_te.c src/te_*c src/dd_*c -g -Wall -Wextra -o bin/test_te
	valgrind --leak-check=yes ./bin/test_te
	rm ./bin/test_te

fe-scratch :
	gcc src/fe_*c src/te_*c src/dd_*c \
		-g -Wall -Wextra -o bin/fe_scratch
