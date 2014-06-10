#include <time.h>   /* only refer: (time)*/
#include <stdlib.h> /* only refer: (malloc, free) */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

int32_t randNum(int32_t minNum, int32_t maxNum) {
  int32_t result = 0, lowNum = 0, hiNum = 0;
  if (minNum < maxNum) {
    lowNum = minNum;
    hiNum = maxNum + 1;
  } else {
    lowNum = maxNum + 1;
    hiNum = minNum;
  }
  srand((unsigned)time(NULL));
  result = (rand() % (hiNum - lowNum)) + lowNum;
  return result;
}

typedef struct Point {
  int32_t x;
  int32_t y;
} Point;

typedef struct Grid {
  Point position;
  bool isBeWalkable;
  bool hasBeing;
} Grid;

Grid *newGrid(Point pos, bool isBeWalkable) {
  Grid *grid;
  grid = (Grid *)malloc(sizeof(Grid));
  grid->position = pos;
  grid->isBeWalkable = isBeWalkable;
  grid->hasBeing = false;
  return grid;
}

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
  void (*printMap)(struct Map *map);
  bool (*inMapP)(struct Map *map, Point pos);
  Grid *(*gridth)(struct Map *map, Point pos);
  GridList *(*nearbyGrids)(struct Map *map, Point pos);
  PathList *(*astarFindPath)(struct Map *map, Point srcPos, Point destPos);
} Map;

typedef struct AstarNode {
  Point pos;
  int32_t f;
  int32_t g;
  struct AstarNode *parenNode;
} AstarNode;

AstarNode *newAstarNode(Point pos, int32_t f, int32_t g, AstarNode *parenNode) {
  AstarNode *node;
  node = (AstarNode *)malloc(sizeof(AstarNode));
  node->pos = pos;
  node->f = f;
  node->g = g;
  node->parenNode = parenNode;
  return node;
}

typedef struct AstarList {
  AstarNode *node;
  struct AstarList *next;
} AstarList;

AstarList *newAstarList(AstarNode *node) {
  AstarList *alist;
  alist = (AstarList *)malloc(sizeof(AstarList));
  alist->node = node;
  alist->next = NULL;
  return alist;
}

int32_t astarG(Point srcPos, AstarNode *minNode) {
  bool isDiagonal = (srcPos.x != minNode->pos.x || srcPos.y != minNode->pos.y);
  int32_t g = isDiagonal ? 14 : 10;
  return g + minNode->g;
}

int32_t astarH(Point srcPos, Point destPos) {
  // Manhattan way
  return abs(srcPos.x - destPos.x) + abs(srcPos.y - destPos.y);
}

AstarList *appendAstarNode(AstarList *alist, AstarNode *anode) {
  if (alist == NULL) {
    alist = newAstarList(anode);
    return alist;
  } else if (alist->node == NULL) {
    alist->node = anode;
    return alist;
  } else if (alist->next == NULL) {
    alist->next = newAstarList(anode);
    return alist;
  } else {
    AstarList *tmp = alist;
    AstarList *prev;
    while (tmp) {
      prev = tmp;
      tmp = tmp->next;
    }
    prev->next = newAstarList(anode);
    return alist;
  }
  return alist;
}

AstarList *unshiftAstarNode(AstarList *alist, AstarNode *anode) {
  if (alist == NULL) {
    alist = newAstarList(anode);
    return alist;
  } else if (alist->node == NULL) {
    alist->node = anode;
    return alist;
  } else if (alist->next == NULL) {
    alist->next = newAstarList(anode);
    return alist;
  } else {
    AstarList *newAlist = newAstarList(anode);
    newAlist->next = alist;
    return newAlist;
  }
}

AstarNode *minFvalAstarNode(AstarList *alist) {
  if (alist == NULL || alist->node == NULL) {
    return NULL;
  }
  AstarList *tmpList;
  tmpList = alist;
  AstarNode *minNode;
  minNode = alist->node;
  int32_t minF = 0;
  int32_t tmpF = 0;
  while (tmpList && tmpList->node) {
    minF = minNode->f;
    tmpF = tmpList->node->f;
    if (minF > tmpF) {
      minF = tmpF;
      minNode = tmpList->node;
    }
    tmpList = tmpList->next;
  }
  return minNode;
}

bool equalPoint(Point a, Point b) { return (a.x == b.x && a.y == b.y); }

AstarNode *findNodeByPos(AstarList *alist, Point pos) {
  if (alist->node == NULL) {
    return NULL;
  }
  AstarList *tmp;
  tmp = alist;
  while (tmp) {
    if (equalPoint(tmp->node->pos, pos)) {
      return tmp->node;
    }
    tmp = tmp->next;
  }
  return NULL;
}

void *freeAstarList(AstarList *alist) {
  AstarList *tmp;
  while (tmp) {
    tmp = alist;
    alist = alist->next;
    free(tmp->node);
    free(tmp);
  }
  return NULL;
}

PathList *reversePathList(PathList *plist) {
  PathList *prev = NULL;
  while (plist) {
    PathList *tmp = plist;
    plist = plist->next;
    tmp->next = prev;
    prev = tmp;
  }
  return prev;
}

PathList *astarFindPath(Map *map, Point srcPos, Point destPos) {
  if (equalPoint(srcPos, destPos) || map->gridth(map, destPos) == NULL ||
      map->gridth(map, srcPos) == NULL ||
      map->gridth(map, srcPos)->isBeWalkable == false ||
      map->gridth(map, destPos)->isBeWalkable == false) {
    return NULL;
  }
  AstarList *openList, *closeList;
  AstarNode *srcNode = newAstarNode(srcPos, 0, 0, NULL);
  AstarNode *minNode;
  int32_t f, g, h;
  Point pos;
  AstarNode *foundNode;
  openList = newAstarList(srcNode);
  closeList = (AstarList *)malloc(sizeof(AstarList));
  closeList->next = NULL;
  closeList->node = NULL;
  minNode = NULL;
  f = 0;
  g = 0;
  h = 0;
  do {
    minNode = minFvalAstarNode(openList);
    closeList = unshiftAstarNode(closeList, minNode);
    openList->node = NULL;
    openList->next = NULL;
    GridList *nbyGrids, *newNbyGrids, *tmpGlist, *tmpNewGlist, *lastGlist;
    nbyGrids = map->nearbyGrids(map, minNode->pos);
    tmpGlist = nbyGrids;
    newNbyGrids = NULL;
    tmpNewGlist = NULL;
    lastGlist = NULL;
    while (tmpGlist != NULL) {  // select neccessary grid
      if (tmpGlist->grid->isBeWalkable &&
          findNodeByPos(closeList, tmpGlist->grid->position) == false) {
        if (tmpNewGlist == NULL) {
          newNbyGrids = tmpGlist;
          tmpNewGlist = tmpGlist;
        } else {
          lastGlist = tmpNewGlist;
          tmpNewGlist->next = tmpGlist;
          tmpNewGlist = tmpGlist;
        }
      }
      tmpGlist = tmpGlist->next;
    }
    if (lastGlist->next != NULL) {
      lastGlist->next = NULL;
    }
    if (newNbyGrids == NULL || newNbyGrids->grid == NULL) {
      break;
    }
    tmpGlist = newNbyGrids;
    while (tmpGlist != NULL) {  // each nearbyGrids
      pos = tmpGlist->grid->position;
      g = astarG(pos, minNode);
      h = astarH(pos, destPos);
      f = g + h;
      foundNode = findNodeByPos(openList, pos);
      if (foundNode) {
        if (g < foundNode->g) {
          foundNode->parenNode = minNode;
          foundNode->g = g;
          foundNode->f = g + h;
        }
      } else {
        openList = unshiftAstarNode(openList, newAstarNode(pos, f, g, minNode));
      }
      // next nearbyGrid
      tmpGlist = tmpGlist->next;
    }
  } while (!(equalPoint(minNode->pos, destPos) || openList->node == NULL));
  AstarNode *tmpNode = minNode;
  PathList *path = malloc(sizeof(PathList));
  PathList *tmpPath;
  tmpPath = path;
  while (tmpNode != srcNode) {
    tmpPath->pos = tmpNode->pos;
    tmpPath->next = (PathList *)malloc(sizeof(PathList));
    tmpPath = tmpPath->next;
    tmpNode = tmpNode->parenNode;
  };
  path = reversePathList(path);
  path->pos = srcPos;
  return path;
}

Grid *gridth(Map *map, Point pos) {
  if (map->inMapP(map, pos)) {
    return &map->grids[pos.y][pos.x];
  } else {
    return NULL;
  }
}

void printMap(Map *map) {
  int32_t y, x;
  for (y = 0; y < map->height; y++) {
    if (y == 0) {
      for (x = 0; x < map->width; x++) {
        printf(" %d ", x);
      }
      printf("\n");
    }
    for (x = 0; x < map->width; x++) {
      if (map->gridth(map, (Point) {x, y})->hasBeing) {
        printf(" @ ");
      } else if (map->gridth(map, (Point) {x, y})->isBeWalkable) {
        printf(" . ");
      } else {
        printf(" # ");
      }
      if (x == (map->width - 1)) {
        printf(" %d ", y);
      }
    }
    printf("\n");
  }
}

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
  return (Map) {width,  height, grids,       printMap,
                inMapP, gridth, nearbyGrids, astarFindPath};
};

typedef struct Guard {
  char *name;
  Map *map;
  Point position;
  PathList *(*move)(struct Guard *self, Point pos);
  Map *(*joinMap)(struct Guard *self, Map *map);
} Guard;

Map *joinMap(Guard *guard, Map *map) {
  guard->map = map;
  map->gridth(map, guard->position)->hasBeing = true;
  return map;
}

PathList *move(Guard *guard, Point destPos) {
  Map *curMap = guard->map;
  if (guard->map == NULL) {
    return false;
  }
  PathList *path, *tmpPath;
  Point curPos;
  path = curMap->astarFindPath(curMap, guard->position, destPos);
  if (path != NULL) {
    tmpPath = path;
    while (tmpPath) {
      curPos = tmpPath->pos;
      curMap->gridth(curMap, guard->position)->hasBeing = false;
      guard->position = curPos;
      curMap->gridth(curMap, curPos)->hasBeing = true;
      printf("%s move to ", guard->name);
      printf("(%d, %d)\n", curPos.x, curPos.y);
      curMap->printMap(curMap);
      tmpPath = tmpPath->next;
    }
    return path;
  } else {
    printf("%s can't move to %d, %d\n", guard->name, destPos.x, destPos.y);
    free(path);
    return NULL;
  }
  return NULL;
}

Guard newGuard(char *name, Point position) {
  return (Guard) {name, NULL, position, move, joinMap};
}

typedef struct RectWall {
  // fixed 4 point
  // lazy use dynamic array for dynamic size
  Point area[4];
} RectWall;

int main() {
  Point path[] = {{0, 0}, {10, 10}, {5, 5}, {15, 15}, {19, 19}};
  int32_t pathSize = sizeof(path)/sizeof(Point);
  int32_t mapWidth = 20;
  int32_t mapHeight = 20;
  Map map = newMap(mapWidth, mapHeight);
  Guard guard = newGuard("wiwi", (Point) {0, 0});
  guard.joinMap(&guard, &map);

  int i, j, k = 0;
  int32_t rectWallCount = randNum(3, 5);
  printf("rectWallCount: %d\n", rectWallCount);
  RectWall *rectWalls = malloc(sizeof(RectWall) * rectWallCount);
  for (i = 0; i < rectWallCount; i++) { // each rectWalls
    rectWalls[i].area[0].x = randNum(i, mapWidth - 1);
    rectWalls[i].area[0].y = randNum(i, mapHeight - 1);
    rectWalls[i].area[1].x = rectWalls[i].area[0].x + 1;
    rectWalls[i].area[1].y = rectWalls[i].area[0].y;
    rectWalls[i].area[2].x = rectWalls[i].area[0].x + 1;
    rectWalls[i].area[2].y = rectWalls[i].area[0].y + 1;
    rectWalls[i].area[3].x = rectWalls[i].area[0].x;
    rectWalls[i].area[3].y = rectWalls[i].area[0].y + 1;
    for (j = 0; j < 4; j++) { // each area
      for (k = 0; k < pathSize; k++) { // each path
        while (equalPoint(path[k], rectWalls[i].area[j]) ||
               map.inMapP(&map, rectWalls[i].area[j]) == false) {
          rectWalls[i].area[0].x = randNum(i + randNum(0, k), mapWidth - 1);
          rectWalls[i].area[0].y = randNum(i + randNum(0, j), mapHeight - 1);
          rectWalls[i].area[1].x = rectWalls[i].area[0].x + 1;
          rectWalls[i].area[1].y = rectWalls[i].area[0].y;
          rectWalls[i].area[2].x = rectWalls[i].area[0].x + 1;
          rectWalls[i].area[2].y = rectWalls[i].area[0].y + 1;
          rectWalls[i].area[3].x = rectWalls[i].area[0].x;
          rectWalls[i].area[3].y = rectWalls[i].area[0].y + 1;
        }
      }
    }
    for (j = 0; j < 4; j++) { // each area
      if (map.gridth(&map, rectWalls[i].area[j]) != NULL) {
        printf("rect wall: %d, %d\n",
               map.gridth(&map, rectWalls[i].area[j])->position.x,
               map.gridth(&map, rectWalls[i].area[j])->position.y);
        map.gridth(&map, rectWalls[i].area[j])->isBeWalkable = false;
      }
    }
  }
  free(rectWalls);
  map.printMap(&map);

  printf("%s start moving\n",guard.name);
  map.gridth(&map, path[2])->isBeWalkable = false;
  guard.move(&guard, path[1]);
  map.gridth(&map, path[2])->isBeWalkable = true;
  guard.move(&guard, path[2]);
  map.gridth(&map, path[1])->isBeWalkable = false;
  guard.move(&guard, path[3]);
  map.gridth(&map, path[1])->isBeWalkable = true;
  guard.move(&guard, path[4]);
  return 0;
}
