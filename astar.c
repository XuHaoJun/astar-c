#include <stdlib.h> /* malloc, free */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef struct Point {
  int32_t x;
  int32_t y;
} Point;

typedef struct Grid {
  Point position;
  bool isBeWalkable;
} Grid;

Grid *newGrid(Point pos, bool isBeWalkable) {
  Grid *grid;
  grid = (Grid *)malloc(sizeof(Grid));
  grid->position = pos;
  grid->isBeWalkable = isBeWalkable;
  return grid;
}

typedef struct GridArray {
  int32_t size;
  Grid *grids;
} GridArray;

typedef struct GridList {
  Grid *grid;
  struct GridList *next;
} GridList;

typedef struct PathList {
  Point pos;
  struct PathList *next;
} PathList;

typedef struct Map {
  int32_t width;
  int32_t height;
  Grid **grids;
  bool (*inMapP)(struct Map *map, Point pos);
  Grid *(*gridth)(struct Map *map, Point pos);
  GridList *(*nearbyGrids)(struct Map *map, Point pos);
  PathList *(*astarFindPath)(Point srcPos, Point destPos);
} Map;

typedef struct AstarNode {
  Point pos;
  int32_t f;
  int32_t g;
  struct AstarNode *parenNode;
} AstarNode;

typedef struct AstarList {
  AstarNode node;
  struct AstarList *next;
} AstarList;

int32_t astarG(Point srcPos, AstarNode *minNode) {
  bool isDiagonal = (srcPos.x != minNode->pos.x || srcPos.y != minNode->pos.y);
  int32_t g = isDiagonal ? 14 : 10;
  return g + minNode->g;
}

int32_t astarH(Point srcPos, Point destPos) {
  // Manhattan way
  return abs(srcPos.x - destPos.x) + abs(srcPos.y - destPos.y);
}

bool equalPoint(Point a, Point b) { return (a.x == b.x && a.y == b.y); }

PathList *astarFindPath(Point srcPos, Point destPos) {
  if (equalPoint(srcPos, destPos)) {
    return NULL;
  }
  return NULL;
}

Grid *gridth(Map *map, Point pos) { return &map->grids[pos.y][pos.x]; }

bool inMapP(Map *map, Point pos) {
  int32_t x, y;
  x = pos.x;
  y = pos.y;
  if (x >= map->width || y >= map->height || x < 0 || y < 0) {
    return false;
  }
  return true;
}

GridList *newGridList(Grid *grid) {
  GridList *glist;
  glist = (GridList *)malloc(sizeof(GridList));
  glist->grid = grid;
  glist->next = NULL;
  return glist;
}

GridList *appendGrid(GridList *glist, Grid *grid) {
  GridList *newGlist = newGridList(grid);
  if (glist == NULL) {
    glist = newGlist;
  } else {
    GridList *tmp = glist;
    while (tmp->next) {
      tmp = tmp->next;
    }
    tmp->next = newGlist;
  }
  return glist;
}

GridList *nearbyGrids(Map *map, Point pos) {
  int32_t directions[8][2] = {
      {0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}};
  if (!map->inMapP(map, pos)) {
    return NULL;
  }
  GridList *glist, *tmp, *cur;
  glist = NULL;
  Point curPos;
  int i;
  for (i = 0; i < 8; i++) {
    curPos.x = pos.x + directions[i][0];
    curPos.y = pos.y + directions[i][1];
    if (map->inMapP(map, curPos)) {
      cur = newGridList(map->gridth(map, curPos));
      if (glist == NULL) {
        glist = cur;
        tmp = glist;
        continue;
      } else {
        tmp->next = cur;
        tmp = cur;
      }
    }
  }
  return glist;
}

Map newMap(int32_t width, int32_t height) {
  Grid **grids = malloc(sizeof(Grid *) * height);
  Grid *temp = malloc(sizeof(Grid) * width * height);
  int i;
  for (i = 0; i < width; i++) {
    grids[i] = temp + (i * width);
  }
  int ix, iy;
  for (ix = 0; ix < width; ix++) {
    for (iy = 0; iy < height; iy++) {
      grids[iy][ix].position = (Point) {ix, iy};
      grids[iy][ix].isBeWalkable = true;
    }
  }
  return (Map) {width,  height,      grids,        inMapP,
                gridth, nearbyGrids, astarFindPath};
};

typedef struct Guard {
  char *name;
  Map *map;
  Point position;
  bool (*move)(struct Guard *self, Point pos);
  /* Grid* (*move)(struct Guard *self, Point pos); */
  Map *(*joinMap)(struct Guard *self, Map *map);
} Guard;

Map *joinMap(Guard *self, Map *map) {
  self->map = map;
  return map;
}

bool move(Guard *self, Point pos) {
  if (self->map == NULL) {
    return false;
  }
  printf("%s move to ", self->name);
  printf("x: %d, y: %d\n", pos.x, pos.y);
  return true;
}

Guard newGuard(char *name, Point position) {
  return (Guard) {name, NULL, position, move, joinMap};
}

int main() {
  Point path[] = {{10, 10}, {5, 5}, {15, 15}};
  Map map = newMap(20, 20);
  Guard guard = newGuard("wiwi", (Point) {0, 0});
  guard.joinMap(&guard, &map);
  int32_t pathSize = (sizeof(path) / sizeof(Point));
  int i;
  for (i = 0; i < pathSize; i++) {
    guard.move(&guard, path[i]);
  }
  printf("path size %d\n", pathSize);
  printf("guard name: %s\n", guard.name);
  printf("x: %d, y: %d\n", map.grids[2][3].position.x,
         map.grids[2][3].position.y);
  GridList *grids = map.nearbyGrids(&map, (Point) {0, 0});
  GridList *cur = grids;
  int size = 0;
  while (cur != NULL) {
    printf("nearbyGrids %d, %d\n", cur->grid->position.x,
           cur->grid->position.y);
    size++;
    cur = cur->next;
  }
  printf("size %d\n", size);
  return 0;
}
