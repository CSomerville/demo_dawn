LINK_BISON="-L/usr/local/opt/bison/lib"
BISON_PATH="/usr/local/opt/bison/bin/bison"

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

di-exec : src/exec_di.c di_lib
	gcc bin/di_lib.o bin/dd_data.o bin/dd_twine.o \
		src/exec_di.c -g -Wall -Wextra -o bin/exec_di

scratch-fe :
	gcc src/fe_*c src/te_*c src/dd_*c src/scratch_fe.c \
		-g -Wall -Wextra -o bin/scratch_fe

te_scanner : src/te_scanner.c src/te_scanner.h bin/dd_data.o
	gcc src/te_scanner.c -g -Wall -Wextra -c -o bin/te_scanner.o

te_tendril : src/te_tendril.c src/te_tendril.h te_scanner \
				dd_data dd_algo dd_graph
	gcc src/te_tendril.c \
		-g -Wall -Wextra -c -o bin/te_tendril.o

dd_utils : src/dd_utils.c src/dd_utils.h
	gcc src/dd_utils.c \
		-g -Wall -Wextra -c -o bin/dd_utils.o

dd_data : src/dd_data.c src/dd_data.h
	gcc src/dd_data.c \
		-g -Wall -Wextra -c -o bin/dd_data.o

dd_algo : src/dd_algo.c src/dd_algo.h dd_data
	gcc src/dd_algo.c \
		-g -Wall -Wextra -c -o bin/dd_algo.o

dd_graph : src/dd_graph.c src/dd_graph.h dd_data
	gcc src/dd_graph.c \
		-g -Wall -Wextra -c -o bin/dd_graph.o

dd_twine : src/dd_twine.c src/dd_twine.h dd_data
	gcc src/dd_twine.c \
		-g -Wall -Wextra -c -o bin/dd_twine.o

dd_twine_ball : src/dd_twine_ball.y src/dd_twine_ball.l dd_data \
				dd_twine src/dd_twine_ball_lib.c				\
				src/dd_twine_ball_lib.h
	$(BISON_PATH) -d -b dd_twine_ball --header=src/dd_twine_ball.tab.h \
		-o src/dd_twine_ball.tab.c src/dd_twine_ball.y
	flex --header-file=src/dd_twine_ball.lex.h -o \
		src/dd_twine_ball.lex.c src/dd_twine_ball.l
	gcc src/dd_twine_ball.tab.c \
		-g -Wall -Wextra -c -o bin/dd_twine_ball.tab.o
	gcc src/dd_twine_ball.lex.c \
		-g -Wall -Wextra -c -o bin/dd_twine_ball.lex.o
	gcc src/dd_twine_ball_lib.c \
		-g -Wall -Wextra -c -o bin/dd_twine_ball_lib.o

di_lib : src/di_lib.h src/di_lib.c dd_data dd_twine
	gcc src/di_lib.c \
		-g -Wall -Wextra -c -o bin/di_lib.o

fe_neo_lib_land : src/fe_neo_lib_land.c src/fe_neo_lib_land.h \
					dd_data dd_utils
	gcc src/fe_neo_lib_land.c \
		-g -Wall -Wextra -c -o bin/fe_neo_lib_land.o

fe_nll_narrator_2 : src/fe_nll_narrator.c src/fe_nll_narrator.h \
					dd_data dd_twine dd_twine_ball
	gcc src/fe_nll_narrator_2.c \
		-g -Wall -Wextra -c -o bin/fe_nll_narrator_2.o

scratch_fe_nll_2 : src/scratch_fe_nll_2.c fe_neo_lib_land \
					dd_twine fe_nll_narrator_2
	gcc bin/dd_data.o bin/dd_utils.o bin/fe_neo_lib_land.o \
		bin/fe_nll_narrator_2.o src/scratch_fe_nll_2.c	\
		bin/dd_twine.o									\
		bin/dd_twine_ball.tab.o bin/dd_twine_ball.lex.o \
		bin/dd_twine_ball_lib.o							\
		-g -Wall -Wextra -o bin/scratch_fe_nll_2

li_lineate : src/li_lineate.c src/li_lineate.h dd_data \
					dd_twine di_lib
	gcc src/li_lineate.c \
		-g -Wall -Wextra -c -o bin/li_lineate.o

fe_lib : src/fe_lib.h src/fe_lib.c dd_data dd_twine te_tendril \
			dd_utils di_lib li_lineate 
	gcc src/fe_lib.c \
		-g -Wall -Wextra -c -o bin/fe_lib.o

fe : src/fe.c fe_lib
	gcc bin/te_scanner.o bin/te_tendril.o bin/dd_data.o \
		bin/dd_graph.o bin/dd_algo.o bin/dd_twine.o bin/di_lib.o bin/fe_lib.o \
		bin/dd_utils.o bin/li_lineate.o src/fe.c \
		-g -Wall -Wextra -o bin/fe

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
