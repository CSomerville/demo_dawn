#include <stdio.h>
#include "dd_data.h"
#include "dd_utils.h"
#include "fe_neo_lib_land.h"

static void add_log_item(FENLLWorld *world, DDString *str) {
	FENLLogItem item;
	item.turn = world->turn;
	item.description = *str;
	DD_ADD_ARRAY(&world->event_log, item);
}

static bool int_array_includes(DDArrInt *arr, int n) {
	int i;
	for (i = 0; i < arr->size; i++) {
		if (arr->elems[i] == n)
			return true;
	}
	return false;
}

static void get_relations_by_id(DDArrFENLLRelation *found, 
		DDArrFENLLRelation *relations, int id) {
	int i;
	for (i = 0; i < relations->size; i++) {
		if (relations->elems[i].debtor_id == id ||
				relations->elems[i].creditor_id == id)
			DD_ADD_ARRAY(found, relations->elems[i]);
	}
}

static void get_joint_relations(DDArrFENLLRelation *found,
		DDArrFENLLRelation *relations, int id_a, int id_b) {
	int i;
	for (i = 0; i < relations->size; i++) {
		if ((relations->elems[i].debtor_id == id_a &&
				relations->elems[i].creditor_id == id_b) ||
				(relations->elems[i].debtor_id == id_b &&
				relations->elems[i].creditor_id == id_a))
			DD_ADD_ARRAY(found, relations->elems[i]);
	}
}

static int get_manhattan_distance(FENLLPoint *a, FENLLPoint *b) {
	return abs(a->x - b->x) + abs(a->y - b->y);
}

static FENLLPoint invert_point(FENLLPoint point) {
	point.x = -point.x;
	point.y = -point.y;
	return point;
}

static int point_to_tile_index(FENLLWorld *world, int x, int y) {
	return y * world->width + x;
}

static FENLLMapTile *map_tile_at(FENLLWorld *world, int x, int y) {
	int index = point_to_tile_index(world, x, y);
	return &world->map.elems[index];
}

static bool point_on_map(FENLLWorld *world, int x, int y) {
	return x >= 0 && x < world->width && y >= 0 && y < world->height;
}

static bool point_available(FENLLWorld *world, FENLLPoint *point) {
	int i;
	for (i = 0; i < world->populace.size; i++) {
		if (point->x == world->populace.elems[i].loc.x &&
				point->y == world->populace.elems[i].loc.y)
			return false;
	}
	return true;
}

/* map isn't set yet so we have to iterate through landers
 * to determine whether a map tile is already occupied
 */
static void get_lander_start_loc(FENLLander *lander, 
		FENLLConfigureWorld *conf, FENLLWorld *world) {
	FENLLPoint point;
	lander->loc.x = -1;
	lander->loc.y = -1;
	while (lander->loc.x < 0 && lander->loc.y < 0) {
		point.y = rand() % conf->height;
		point.x = rand() % conf->width;
		if (point_available(world, &point)) {
			lander->loc = point;
		}
	}
}

static void get_lander_start_pocket(FENLLander *lander,
		FENLLConfigureWorld *conf) {
	double tmp;
	tmp = rand_double() * (conf->pocket_ceil - conf->pocket_floor)
		+ conf->pocket_floor;
	lander->pocket.fuse_coin = tmp;
	tmp = rand_double() * (conf->pocket_ceil - conf->pocket_floor)
		+ conf->pocket_floor;
	lander->pocket.frux_coin = tmp;
}

static void init_lander(FENLLander *lander) {
	init_dd_string(&lander->name);
}

static void free_lander(FENLLander *lander) {
	free_dd_chars(&lander->name);
}

static void create_lander(FENLLander *lander, int id, DDString *name,
	FENLLConfigureWorld *conf, FENLLWorld *world) {
	lander->id = id;
	dd_copy_dd_string(&lander->name, name);
	get_lander_start_loc(lander, conf, world);
	get_lander_start_pocket(lander, conf);
	lander->goal.goal_type = FE_NLL_GOAL_TYPE_NULL;
	lander->goal.lander = NULL;
	lander->last_move = -1;
	lander->sight = 5;
	lander->alive = true;
}

static void lander_look(DDArrInt *tile_indices, FENLLWorld *world,
		FENLLander *lander) {
	int i, j, d, index;
	FENLLMapTile *tile;
	FENLLPoint pt;
	for (i = -lander->sight; i <= lander->sight; i++) {
		d = lander->sight - abs(i);
		for (j = -d; j <= d; j++) {
			pt.x = lander->loc.x + j;
			pt.y = lander->loc.y + i;
			if (point_on_map(world, pt.x, pt.y)) {
				tile = map_tile_at(world, pt.x, pt.y);
				if (tile->inhabitant) {
					index = point_to_tile_index(world, pt.x, pt.y);
					DD_ADD_ARRAY(tile_indices, index);
				}
			}
		}
	}
}

static void lander_sees_lander_description(DDString *target,
		FENLLander *seer, FENLLander *seen) {
	DDString tmp_str;
	init_dd_string(&tmp_str);
	give_to_dd_string(&tmp_str, " sees ", 6);
	dd_string_concat_mutate(target, &seer->name);
	dd_string_concat_mutate(target, &tmp_str);
	dd_string_concat_mutate(target, &seen->name);
	free_dd_chars(&tmp_str);
}

static void log_new_goal(FENLLWorld *world, FENLLander *lander) {
	DDString tmp_str;
	DDString target;
	init_dd_string(&tmp_str);
	init_dd_string(&target);

	dd_string_concat_mutate(&target, &lander->name);
	give_to_dd_string(&tmp_str, " has new goal: ", 15);
	dd_string_concat_mutate(&target, &tmp_str);
	free_dd_chars(&tmp_str);
	init_dd_string(&tmp_str);
	if (lander->goal.goal_type == FE_NLL_GOAL_TYPE_FLEE) {
		give_to_dd_string(&tmp_str, "flee ", 5);
		dd_string_concat_mutate(&target, &tmp_str);
		dd_string_concat_mutate(&target, &lander->goal.lander->name);
	} else if (lander->goal.goal_type == FE_NLL_GOAL_TYPE_CHARGE) {
		give_to_dd_string(&tmp_str, "charge ", 7);
		dd_string_concat_mutate(&target, &tmp_str);
		dd_string_concat_mutate(&target, &lander->goal.lander->name);
	} else {
		give_to_dd_string(&tmp_str, "none", 4);
		dd_string_concat_mutate(&target, &tmp_str);
	}
	add_log_item(world, &target);
	free_dd_chars(&tmp_str);
}

static void set_goal(DDArrFENLLRelation *relations, FENLLander *lander,
		FENLLander *tile_inhabitant) {
	if (relations->elems[0].debtor_id == lander->id) {
		lander->goal.goal_type = FE_NLL_GOAL_TYPE_FLEE;
		lander->goal.lander = tile_inhabitant;
		lander->goal.invisible_ctr = 0;
	} else if (relations->elems[0].creditor_id == lander->id) {
		lander->goal.goal_type = FE_NLL_GOAL_TYPE_CHARGE;
		lander->goal.lander = tile_inhabitant;
		lander->goal.invisible_ctr = 0;
	}
}

static void evaluate_one_tile(int tile_index, FENLLWorld *world,
		FENLLander *lander) {
	FENLLMapTile *tile;
	DDArrFENLLRelation relations;
	int dist_a;
	int dist_b;

	DD_INIT_ARRAY(&relations);

	tile = &world->map.elems[tile_index];

	if (tile->inhabitant != lander && 
			tile->inhabitant != lander->goal.lander) {
		get_joint_relations(&relations, &world->relations, lander->id,
				tile->inhabitant->id);

		if (relations.size) {
			if (lander->goal.goal_type == FE_NLL_GOAL_TYPE_NULL) {
				set_goal(&relations, lander, tile->inhabitant);
				log_new_goal(world, lander);
			} else {
				dist_a = get_manhattan_distance(&lander->loc,
						&lander->goal.lander->loc);
				dist_b = get_manhattan_distance(&lander->loc,
						&tile->inhabitant->loc);
				if (dist_a / dist_b >= 2) {
					set_goal(&relations, lander, tile->inhabitant);
					log_new_goal(world, lander);
				}
			}
		}
	}
}

static void lander_pick_goal(DDArrInt *tile_indices, FENLLWorld *world,
		FENLLander *lander) {
	int i;
	if (lander->goal.invisible_ctr > 2) {
		lander->goal.goal_type = FE_NLL_GOAL_TYPE_NULL;
		lander->goal.lander = NULL;
		lander->goal.invisible_ctr = 0;
	}
	for (i = 0; i < tile_indices->size; i++) {
		evaluate_one_tile(tile_indices->elems[i], world, lander);
	}
}

static void lander_see(DDArrInt *tile_indices, FENLLWorld *world,
		FENLLander *lander) {
	FENLLMapTile *tile;
	int i;
	DDString tmp_str;
	bool goal_seen;
	goal_seen = false;

	for (i = 0; i < tile_indices->size; i++) {
		tile = &world->map.elems[tile_indices->elems[i]];
		if (tile->inhabitant && tile->inhabitant != lander) {
			if (lander->goal.goal_type != FE_NLL_GOAL_TYPE_NULL &&
					tile->inhabitant == lander->goal.lander) {
				goal_seen = true;
			}
			init_dd_string(&tmp_str);
			lander_sees_lander_description(&tmp_str, lander,
					tile->inhabitant);
			add_log_item(world, &tmp_str);
		}
	}

	if (lander->goal.goal_type != FE_NLL_GOAL_TYPE_NULL &&
		   goal_seen == false) {
		lander->goal.invisible_ctr++;
	}
}

static void move_lander(FENLLWorld *world, FENLLPoint point,
		FENLLander *lander) {
	FENLLMapTile *tile;
	tile = map_tile_at(world, lander->loc.x, lander->loc.y);
	tile->inhabitant = NULL;
	lander->loc = point;
	tile = map_tile_at(world, lander->loc.x, lander->loc.y);
	tile->inhabitant = lander;
}

static FENLLPoint point_in_direction(int direction,
		FENLLPoint point) {
	switch (direction) {
		case -1:
			return point;
		case 0:
			point.y--;
			return point;
		case 1:
			point.x++;
			return point;
		case 2:
			point.y++;
			return point;
		case 3:
			point.x--;
			return point;
	}
}

static void find_available_dirs(DDArrInt *available_dirs,
		FENLLWorld *world, FENLLander *lander) {
	FENLLMapTile *tile;
	FENLLPoint point;
	int i;

	for (i = 0; i < 4; i++) {
		point = point_in_direction(i, lander->loc);
		if (point_on_map(world, point.x, point.y)) {
			tile = map_tile_at(world, point.x, point.y);
			if (tile->inhabitant == NULL)
				DD_ADD_ARRAY(available_dirs, i);
		}
	}
}

static int determine_dir_by_goal(DDArrInt *available_dirs,
		FENLLander *lander) {
	FENLLPoint goal_point;
	if (lander->goal.goal_type == FE_NLL_GOAL_TYPE_CHARGE) {
		goal_point = lander->goal.lander->loc;
	} else {
		goal_point = invert_point(lander->goal.lander->loc);
	}

	if (goal_point.x == lander->loc.x) {
		if (goal_point.y > lander->loc.y) {
			if (int_array_includes(available_dirs, 2))
				return 2;
			else if (int_array_includes(available_dirs, 1))
				return 1;
			else if (int_array_includes(available_dirs, 3))
				return 3;
		} else if (goal_point.y < lander->loc.y) {
			if (int_array_includes(available_dirs, 0))
				return 0;
			else if (int_array_includes(available_dirs, 3))
				return 3;
			else if (int_array_includes(available_dirs, 1))
				return 1;
		}
	} else if (goal_point.x > lander->loc.x) {
		if (goal_point.y == lander->loc.y) {
			if (int_array_includes(available_dirs, 1))
				return 1;
			else if (int_array_includes(available_dirs, 0))
				return 0;
			else if (int_array_includes(available_dirs, 2))
				return 2;
		} else if (goal_point.y > lander->loc.y) {
			if (int_array_includes(available_dirs, 1))
				return 1;
			else if (int_array_includes(available_dirs, 2))
				return 2;
		} else if (goal_point.y < lander->loc.y) {
			if (int_array_includes(available_dirs, 0))
				return 0;
			else if (int_array_includes(available_dirs, 1))
				return 1;
		}
	} else if (goal_point.x < lander->loc.x) {
		if (goal_point.y == lander->loc.y) {
			if (int_array_includes(available_dirs, 3))
				return 3;
			else if (int_array_includes(available_dirs, 2))
				return 2;
			else if (int_array_includes(available_dirs, 0))
				return 0;
		} else if (goal_point.y > lander->loc.y) {
			if (int_array_includes(available_dirs, 2))
				return 2;
			else if (int_array_includes(available_dirs, 3))
				return 3;
		} else if (goal_point.y < lander->loc.y) {
			if (int_array_includes(available_dirs, 3))
				return 3;
			else if (int_array_includes(available_dirs, 0))
				return 0;
		}
	}
	return available_dirs->elems[0];
}

static int pick_direction_2(FENLLWorld *world, FENLLander *lander) {
	DDArrInt available_dirs;
	DD_INIT_ARRAY(&available_dirs);
	
	find_available_dirs(&available_dirs, world, lander);

	if (available_dirs.size == 0)
		return -1;

	if (lander->goal.goal_type == FE_NLL_GOAL_TYPE_NULL)
		return available_dirs.elems[rand() % available_dirs.size];

	return determine_dir_by_goal(&available_dirs, lander);
}

static FENLLPoint pick_direction(FENLLWorld *world, FENLLander *lander) {
	FENLLPoint point;
	FENLLMapTile *tile;
	int i;
	DDArrFENLLPoint possible_dirs;
	DD_INIT_ARRAY(&possible_dirs);
	for (i = 0; i < 4; i++) {
		point = point_in_direction(i, lander->loc);
		if (point_on_map(world, point.x, point.y)) {
			tile = map_tile_at(world, point.x, point.y);
			if (tile->inhabitant == NULL)
				DD_ADD_ARRAY(&possible_dirs, point);
		}
	}
	if (possible_dirs.size == 0)
		return lander->loc;
	return possible_dirs.elems[rand() % possible_dirs.size];
}

static void lander_recoup(FENLLWorld *world, FENLLander *lander) {
	DDString tmp;
	DDString target;
	init_dd_string(&tmp);
	init_dd_string(&target);
	dd_string_concat_mutate(&target, &lander->name);
	give_to_dd_string(&tmp, " attempts recoupment from ", 26);
	dd_string_concat_mutate(&target, &tmp);
	free_dd_chars(&tmp);
	dd_string_concat_mutate(&target, &lander->goal.lander->name);
	add_log_item(world, &target);
}

static void lander_take_turn(FENLLWorld *world, FENLLander *lander) {
	int dir;
	DDArrInt tile_indices;
	DD_INIT_ARRAY(&tile_indices);

	lander_look(&tile_indices, world, lander);
	lander_see(&tile_indices, world, lander);

	if (lander->goal.goal_type == FE_NLL_GOAL_TYPE_CHARGE &&
		(get_manhattan_distance(&lander->goal.lander->loc,
					&lander->loc) == 1)) {
		lander_recoup(world, lander);
	} else {
		lander_pick_goal(&tile_indices, world, lander);
		dir = pick_direction_2(world, lander);
		lander->last_move = dir;
		move_lander(world, point_in_direction(dir, lander->loc), lander);
	}
}

// world->populace must be set.
static void set_relations(FENLLWorld *world, FENLLConfigureWorld *conf) {
	int i, j;
	double tmp;
	bool i_is_debtor;
	FENLLRelation relation;
	for (i = 0; i < world->populace.size; i++) {
		for (j = i + 1; j < world->populace.size; j++) {
			i_is_debtor = rand() % 2;
			relation.debtor_id = i_is_debtor ? i : j;
			relation.creditor_id = i_is_debtor ? j : i;
			tmp = rand_double() * (conf->amt_ceil - conf->amt_floor)
				+ conf->amt_floor;
			relation.amt = tmp;
			tmp = rand_double() * 
				(conf->interest_ceil - conf->interest_floor)
				+ conf->interest_floor;
			relation.interest = tmp;
			relation.coin = rand() % 2 ? FE_NLL_COIN_FUSE :
				FE_NLL_COIN_FRUX;
			DD_ADD_ARRAY(&world->relations, relation);
		}
	}
}

static void init_tile(FENLLMapTile *tile) {
	tile->inhabitant = NULL;
}

// world->populace and world->height/width must be set.
static void set_map(FENLLWorld *world) {
	int i;
	FENLLMapTile *tile;

	DD_INIT_ARRAY_SIZE(&world->map, world->height * world->width);
	for (i = 0; i < world->map.size; i++)
		init_tile(&world->map.elems[i]);

	for (i = 0; i < world->populace.size; i++) {
		tile = map_tile_at(world, world->populace.elems[i].loc.x,
				world->populace.elems[i].loc.y);
		tile->inhabitant = &world->populace.elems[i];
	}
}

static void free_log_item(FENLLogItem *log_item) {
	free_dd_chars(&log_item->description);
}

void fe_nll_print_map(FENLLWorld *world) {
	int i,j;
	FENLLMapTile *tile;
	for (i = 0; i < world->height; i++) {
		for (j = 0; j < world->width; j++) {
			tile = map_tile_at(world, j, i);
			if (tile->inhabitant) {
				printf("@");
			} else {
				printf(".");
			}
		}
		printf("\n");
	}
}

void fe_nll_print_event_log(FENLLWorld *world) {
	int i;
	for (i = 0; i < world->event_log.size; i++) {
		if (world->event_log.elems[i].turn == world->turn)
			printf("%s\n", world->event_log.elems[i].description.chars);
	}
}

void fe_nll_print_lander(FENLLander *lander) {
	printf("id: %d\t%s\n", lander->id, lander->name.chars);
	printf("(%d,%d)\n", lander->loc.x, lander->loc.y);
	printf("fuse: %f\n", lander->pocket.fuse_coin);
	printf("frux: %f\n", lander->pocket.frux_coin);
	printf("alive: %c\n", lander->alive ? 'y' : 'n');
}

void fe_nll_init_world(FENLLWorld *world, FENLLConfigureWorld *conf) { 
	int i;
	DDString tmp_name;
	FENLLander lander;
	world->height = conf->height;
	world->width = conf->width;
	DD_INIT_ARRAY(&world->populace);
	DD_INIT_ARRAY(&world->relations);
	DD_INIT_ARRAY(&world->map);
	DD_INIT_ARRAY(&world->event_log);

	world->turn = 0;
	for (i = 0; i < conf->num_population; i++) {
		init_dd_string(&tmp_name);
		init_lander(&lander);
		dd_repeat_dd_string(&tmp_name, 
			&conf->names.elems[i%conf->names.size], 
			(i / conf->names.size) + 1);
		create_lander(&lander, i, &tmp_name, conf, world);
		DD_ADD_ARRAY(&world->populace, lander);

		free_dd_chars(&tmp_name);
	}
	set_relations(world, conf);
	set_map(world);
	world->mode = conf->mode;
}

void fe_nll_tick(FENLLWorld *world) {
	int i;
	for (i = 0; i < world->populace.size; i++) {
		lander_take_turn(world, &world->populace.elems[i]);
	}

	if (world->mode == FE_NLL_MODE_WATCH) {
		fe_nll_print_map(world);
		fe_nll_print_event_log(world);
		getchar();
	}

	world->turn++;
}

void fe_nll_free_world(FENLLWorld *world) {
	int i;
	for (i = 0; i < world->populace.size; i++)
		free_lander(&world->populace.elems[i]);
	DD_FREE_ARRAY(&world->populace);
	DD_FREE_ARRAY(&world->relations);
	for (i = 0; i < world->event_log.size; i++)
		free_log_item(&world->event_log.elems[i]);
	DD_FREE_ARRAY(&world->event_log);
}
