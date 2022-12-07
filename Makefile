test-dd :
	gcc src/test_dd.c src/dd_*c -g -Wall -Wextra -o bin/test_dd
	valgrind --leak-check=yes ./bin/test_dd
	rm ./bin/test_dd

test-te :
	gcc src/test_te.c src/te_*c src/dd_*c -g -Wall -Wextra -o bin/test_te
	valgrind --leak-check=yes ./bin/test_te
	rm ./bin/test_te

test-di :
	gcc src/test_di.c src/di_*c src/dd_*c -g -Wall -Wextra -o bin/test_di
	valgrind --leak-check=full --show-leak-kinds=all ./bin/test_di
	rm ./bin/test_di

scratch-fe :
	gcc src/fe_*c src/te_*c src/dd_*c src/scratch_fe.c \
		-g -Wall -Wextra -o bin/scratch_fe

scratch-fe-nll :
	gcc src/fe_*c src/te_*c src/dd_*c src/scratch_fe_nll.c \
		-g -Wall -Wextra -o bin/scratch_nll
