test-dd :
	gcc src/test_dd.c src/dd_data.c src/dd_graph.c \
	   src/dd_algo.c src/dd_twine.c -g -Wall -Wextra -o bin/test_dd
	valgrind --leak-check=yes ./bin/test_dd
	rm ./bin/test_dd

test-te :
	gcc src/test_te.c src/te_*c src/dd_*c -g -Wall -Wextra -o bin/test_te
	valgrind --leak-check=yes ./bin/test_te
	rm ./bin/test_te

test-di :
	gcc src/test_di.c src/di_*c src/dd_*c -g -Wall -Wextra -o bin/test_di
	./bin/test_di && valgrind --leak-check=full --show-leak-kinds=all ./bin/test_di
	rm ./bin/test_di

di-exec :
	gcc src/exec_di.c src/di_*c src/dd_*c -g -Wall -Wextra -o bin/exec_di

scratch-fe :
	gcc src/fe_*c src/te_*c src/dd_*c src/scratch_fe.c \
		-g -Wall -Wextra -o bin/scratch_fe

scratch-fe-nll :
	gcc src/fe_neo_lib_land.c src/fe_nll_narrator.c \
		src/te_*c src/dd_*c src/scratch_fe_nll.c \
		-g -Wall -Wextra -o bin/scratch_nll

monstre-scratch :
	bison -d -b fe_monstre src/fe_monstre.y
	flex --header-file=fe_monstre.lex.h -o fe_monstre.lex.c \
		src/fe_monstre.l
	mv fe_monstre* src/
	gcc src/fe_monstre.tab.c -ly -c
	gcc src/fe_monstre.lex.c -lfl -c
	gcc src/dd_data.c \
		src/dd_twine.c \
		src/di_lib.c \
		src/li_lineate.c \
		src/fe_monstre.tab.o src/fe_monstre.lex.o \
		src/fe_monstre_lib.c \
		src/monstre_scratch.c -o bin/monstre_scratch

dd-twine-ball :
	bison -d -b dd_twine_ball src/dd_twine_ball.y
	flex --header-file=dd_twine_ball.lex.h -o \
		dd_twine_ball.lex.c src/dd_twine_ball.l
	mv dd_twine_ball.* src
	gcc src/dd_twine_ball.tab.c -ly -c
	gcc src/dd_twine_ball.lex.c -lfl -c
	gcc src/dd_data.c \
		src/dd_twine.c \
		src/dd_twine_ball.tab.o src/dd_twine_ball.lex.o \
		src/dd_twine_ball_scratch.c -o bin/dd_twine_ball_scratch
