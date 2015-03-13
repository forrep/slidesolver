#ifndef SLIDESOLVER_H
#define SLIDESOLVER_H

#include <jni.h>
#include <stdint.h>

#define HISTORY_SIZE 400000

#define INT_SIZE 32
#define FIELD_SIZE_MAX 36
#define BITS_PER_OPERATION 2  // 1 Operationに必要なbit数
#define OPERATIONS_PER_VARIABLE 4 // 1変数フィールドに入るOperation数
#define OPERATION_MASK 3      // 1 Operationを切り出すためのMask 2bit


typedef enum {
	SCORING_TYPE_DEFAULT,
	SCORING_TYPE_POW,
	SCORING_TYPE_LEFT,
	SCORING_TYPE_TOP
} ScoringType;

typedef struct {
	int (*comparator)(const void*, const void*);
	int moveLimit;
	int candidateSize;
	ScoringType scoringType;
	int decreaseFactor;
} SolverConfiguration;

typedef struct {
	uint32_t cellMask;
	ScoringType scoringType;
	int *startField; // length=size
	int *finalField; // length=size
	int width;
	int height;
	int size;
	int bitPerCell;
	int cellsPerInt;
	int instanceId;
	int fieldSlotMap[FIELD_SIZE_MAX];
	int fieldInSlotPositionMap[FIELD_SIZE_MAX];
	int positionXMap[FIELD_SIZE_MAX];
	int positionYMap[FIELD_SIZE_MAX];
	int canMoveMap[FIELD_SIZE_MAX];
	int wall[FIELD_SIZE_MAX];
} FieldCondition;

typedef struct {
	uint8_t *list;
	int reference;
	int length;
} Uint8Array;

typedef struct {
	uint32_t *list;
	int reference;
	int length;
} Uint32Array;

typedef struct {
	Uint32Array *field;
	Uint8Array *operations;
	int reference;
	int hashCode;
	uint16_t operationCount;
	uint8_t currentOperation;
} Field;

typedef enum {
	LEFT, RIGHT, UP, DOWN
} Direction;

typedef struct {
	FieldCondition *condition;
	Uint32Array *field;
	Uint8Array *operations;
	int reference;
	int instanceId;
	int position; // X軸 + Y軸 * width = position
	int manhattanDistance; // 計算済みのマンハッタン距離
	Direction lastOperation;
	uint16_t operationCount;
	uint8_t currentOperation;
	unsigned char reverseMode;
} FieldOperation;

typedef struct {
	FieldOperation **list;
	FieldOperation *minThresould;
	int (*comparator)(const void*, const void*);
	int capacity;
	int internalCapacity;
	int size;
	int organized;
} FieldSet;

typedef struct FieldHistoryEntry_t {
	Field *entry;
	struct FieldHistoryEntry_t *next;
	uint32_t version;
} FieldHistoryEntry;

typedef struct {
	FieldHistoryEntry **table;
	FieldHistoryEntry **list;
	int tableLength;
	int capacity;
	int index;
	int waterMark;
	uint32_t version;
} FieldHistory;


JNIEXPORT jstring JNICALL Java_jp_ne_raccoon_slidesolver_Solver_solveNative(JNIEnv *env, jclass clazz, jint width, jint height, jstring fieldString);
void solve(SolverConfiguration *configuration, FieldOperation *initialField, FieldHistory *fieldHistoryAsc, FieldHistory *fieldHistoryDsc, FieldOperation *solvedField);


// FieldCondition
FieldCondition *FieldCondition_alloc(int width, int height, const char *fieldString, ScoringType scoringType);
void FieldCondition_release(FieldCondition *this);


// Field
Field *Field_alloc(FieldOperation *fieldOperation);
int Field_equals(Field *this, Field *another);
void Field_retain(Field *this);
void Field_release(Field *this);
void Field_getOperations(Field *this, unsigned char reverseMode, char *operationsString);


// FieldOperation
FieldOperation *FieldOperation_alloc(FieldCondition *condition);
FieldOperation *FieldOperation_copy(FieldOperation *this);
FieldOperation *FieldOperation_copyAsReverse(FieldOperation *this);
void FieldOperation_retain(FieldOperation *this);
void FieldOperation_release(FieldOperation *this);
int FieldOperation_getCell(FieldOperation *this, int i);
void FieldOperation_setCell(FieldOperation *this, int i, int cell);
unsigned char FieldOperation_canMove(FieldOperation *this, Direction direction);
void FieldOperation_moves(FieldOperation *this, const char *operations);
void FieldOperation_move(FieldOperation *this, Direction direction, unsigned char check);
void FieldOperation_getOperations(Uint8Array *operations, uint8_t currentOperation, uint16_t operationCount, unsigned char reverseMode, char *operationsString);
void FieldOperation_updateManhattanDistance(FieldOperation *this);
int FieldOperation_getManhattanDistance(FieldOperation *this, int cell, int position);
int FieldOperation_getDistance(FieldOperation *this, int start, int end);
unsigned char FieldOperation_isSolved(FieldOperation *this);
int FieldOperation_comparatorDefault(const void *a, const void *b);
int FieldOperation_comparatorBack27(const void *a, const void *b);


// FieldSet
FieldSet *FieldSet_alloc(int capacity, int (*comparator)(const void*, const void*));
void FieldSet_release(FieldSet *this);
void FieldSet_addSet(FieldSet *this, FieldSet *field);
void FieldSet_add(FieldSet *this, FieldOperation *field);
void FieldSet_organize(FieldSet *this);
FieldOperation *FieldSet_first(FieldSet *this);


// FieldHistoryEntry
FieldHistoryEntry *FieldHistoryEntry_alloc();
void FieldHistoryEntry_release(FieldHistoryEntry *this);


// FieldHistory
FieldHistory *FieldHistory_alloc(int capacity);
void FieldHistory_release(FieldHistory *this);
int FieldHistory_indexFor(FieldHistory *this, int hashCode);
unsigned char FieldHistory_add(FieldHistory *this, Field *field);
Field *FieldHistory_get(FieldHistory *this, Field *field, unsigned char current);
FieldHistoryEntry *FieldHistory_getEntry(FieldHistory *this, Field *field, unsigned char current);
unsigned char FieldHistory_contains(FieldHistory *this, Field *field, unsigned char current);
void FieldHistory_clear(FieldHistory *this);
void FieldHistory_remove(FieldHistory *this, FieldHistoryEntry *field);

// Uint8Array
Uint8Array *Uint8Array_alloc(int length);
Uint8Array *Uint8Array_copy(Uint8Array *this);
Uint8Array *Uint8Array_copyWithLength(Uint8Array *this, int length);
int Uint8Array_equals(Uint8Array *this, Uint8Array *another);
void Uint8Array_retain(Uint8Array *this);
void Uint8Array_release(Uint8Array *this);

// Uint32Array
Uint32Array *Uint32Array_alloc(int length);
Uint32Array *Uint32Array_copy(Uint32Array *this);
Uint32Array *Uint32Array_copyWithLength(Uint32Array *this, int length);
int Uint32Array_equals(Uint32Array *this, Uint32Array *another);
void Uint32Array_retain(Uint32Array *this);
void Uint32Array_release(Uint32Array *this);

// Direction
char Direction_getValue(Direction this);
Direction Direction_valueOf(char value);
Direction Direction_reverse(Direction this);
Direction Direction_clockwise(Direction this);
Direction Direction_counterclockwise(Direction this);

// Uint32
uint32_t Uint32_highestOneBit(uint32_t value);
int Uint32_numberOfTrailingZeros(uint32_t value);

#endif
