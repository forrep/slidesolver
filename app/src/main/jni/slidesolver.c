#include <jni.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "slidesolver.h"

SolverConfiguration configurations[] = {
		{FieldOperation_comparatorDefault, 140, 1700, SCORING_TYPE_DEFAULT, 40},
		{FieldOperation_comparatorBack27, 140, 1400, SCORING_TYPE_DEFAULT, 0},
		{FieldOperation_comparatorDefault, 180, 1600, SCORING_TYPE_POW, 0},
};
JNIEXPORT jstring JNICALL Java_jp_ne_raccoon_slidesolver_Solver_solveNative
(JNIEnv *env, jclass clazz, jint width, jint height, jstring fieldString) {
	char buf[300 * 2 + 1];
	buf[0] = '\0';
	const char *fieldChar = (*env)->GetStringUTFChars(env, fieldString, 0);
	FieldCondition *condition = NULL;
	FieldOperation *initialField = NULL;
	FieldHistory *fieldHistoryAsc = FieldHistory_alloc(HISTORY_SIZE);
	FieldHistory *fieldHistoryDsc = FieldHistory_alloc(HISTORY_SIZE);
	if (fieldChar == NULL || fieldHistoryAsc == NULL || fieldHistoryDsc == NULL) {
		goto release;
	}
	int i;
	for (i = 0; i < sizeof(configurations) / sizeof(SolverConfiguration); ++i) {
		condition = FieldCondition_alloc(width, height, fieldChar, configurations[i].scoringType);
		initialField = FieldOperation_alloc(condition);
		FieldOperation *solved = FieldOperation_copy(initialField);
		if (condition == NULL || initialField == NULL || solved == NULL) {
			if (solved != NULL) { FieldOperation_release(solved); }
			goto release;
		}
		solve(&configurations[i], initialField, fieldHistoryAsc, fieldHistoryDsc, solved);
		if (FieldOperation_isSolved(solved)) {
			FieldOperation_getOperations(solved->operations, solved->currentOperation, solved->operationCount, solved->reverseMode, buf);
			FieldOperation_release(solved);
			break;
		}
		FieldOperation_release(solved);
	}

	release:
	if (fieldHistoryAsc != NULL) { FieldHistory_release(fieldHistoryAsc); }
	if (fieldHistoryDsc != NULL) { FieldHistory_release(fieldHistoryDsc); }
	if (initialField != NULL) { FieldOperation_release(initialField); }
	if (condition != NULL) { FieldCondition_release(condition); }
	if (fieldChar != NULL) { (*env)->ReleaseStringUTFChars(env, fieldString, fieldChar); }
	return (*env)->NewStringUTF(env, buf);
}

struct problem_t {
	int width;
	int height;
	char *fieldString;
};
int main() {
	struct problem_t problems[] = {
			{4,5,"5G3486F7092C1ADJEHBI"},
			{4,6,"1924D5387EB=H6M0IKNFLAJG"},
			{5,5,"358092=BDA6HL=4GI1JFCMNOK"},
			{3,5,"41=9506BA2C8=7E"},
			{6,3,"==E93D470F=A8GHC65"},
			{6,5,"EF123=KD==4587GSNTQP=BH0JRMIOC"}
	};
//	mtrace();
	char buf[300 * 2 + 1];
	int count = sizeof(problems) / sizeof(struct problem_t);
	int i, j;
	int solvedCount = 0;
	for (i = 0; i < count; ++i) {
		int width = problems[i].width;
		int height = problems[i].height;
		char *fieldChar = problems[i].fieldString;

		FieldCondition *condition = NULL;
		FieldOperation *initialField = NULL;
		FieldOperation *solved = NULL;
		FieldHistory *fieldHistoryAsc = FieldHistory_alloc(HISTORY_SIZE);
		FieldHistory *fieldHistoryDsc = FieldHistory_alloc(HISTORY_SIZE);
		if (fieldChar == NULL || fieldHistoryAsc == NULL || fieldHistoryDsc == NULL) {
			goto release;
		}
		for (j = 0; j < sizeof(configurations) / sizeof(SolverConfiguration); ++j) {
			if (condition != NULL) { FieldCondition_release(condition); }
			if (initialField != NULL) { FieldOperation_release(initialField); }
			if (solved != NULL) { FieldOperation_release(solved); }

			condition = FieldCondition_alloc(width, height, fieldChar, configurations[j].scoringType);
			initialField = FieldOperation_alloc(condition);
			solved = FieldOperation_copy(initialField);
			if (condition == NULL || initialField == NULL || solved == NULL) {
				goto release;
			}
			solve(&configurations[j], initialField, fieldHistoryAsc, fieldHistoryDsc, solved);
			if (FieldOperation_isSolved(solved)) {
				FieldOperation_getOperations(solved->operations, solved->currentOperation, solved->operationCount, solved->reverseMode, buf);
				break;
			}
		}

		printf("#%d\n", i + 1);
		printf("Problem: %d,%d,%s\n", problems[i].width, problems[i].height, problems[i].fieldString);
		printf("IniDst: %d\n", initialField->manhattanDistance);
		if (FieldOperation_isSolved(solved)) {
			FieldOperation_getOperations(solved->operations, solved->currentOperation, solved->operationCount, solved->reverseMode, buf);
			printf("Ope: %s\n", buf);
			printf("Cnt: %d\n", solved->operationCount);
			printf("Dst: %d\n", solved->manhattanDistance);
			++solvedCount;
		}
		else {
			printf("Ope: \n");
			printf("Cnt: 0\n");
			printf("Dst: %d\n", initialField->manhattanDistance);
		}
		printf("\n");

		release:
		if (fieldHistoryAsc != NULL) { FieldHistory_release(fieldHistoryAsc); }
		if (fieldHistoryDsc != NULL) { FieldHistory_release(fieldHistoryDsc); }
		if (initialField != NULL) { FieldOperation_release(initialField); }
		if (solved != NULL) { FieldOperation_release(solved); }
		if (condition != NULL) { FieldCondition_release(condition); }
	}
	printf("Num: %d\n", count);
	printf("Scn: %d\n", solvedCount);
	printf("\n");
//	muntrace();
	return 0;
}

void solve(SolverConfiguration *configuration, FieldOperation *initialField, FieldHistory *fieldHistoryAsc, FieldHistory *fieldHistoryDsc, FieldOperation *solvedField) {
	int i, j;
	Direction direction;
	FieldOperation *field;
	FieldOperation *newField;
	unsigned char solved;

	FieldSet *setAsc = FieldSet_alloc(1, configuration->comparator);
	FieldSet *setDsc = FieldSet_alloc(1, configuration->comparator);
	FieldOperation *reverseField = FieldOperation_copyAsReverse(initialField);
	if (setAsc == NULL || setDsc == NULL || reverseField == NULL) {
		if (reverseField != NULL) { FieldOperation_release(reverseField); }
		goto release;
	}
	FieldSet_add(setAsc, initialField);
	FieldSet_add(setDsc, reverseField);
	FieldOperation_release(reverseField);

	FieldSet *nextSetAsc;
	FieldSet *nextSetDsc;

	Field *foundAnswerAsc = NULL;
	Field *foundAnswerDsc = NULL;
	Field *fieldTemp;
	for (i = 0; i < configuration->moveLimit; ++i) {
		int candidate = configuration->candidateSize * (configuration->moveLimit - i * configuration->decreaseFactor / 100) / configuration->moveLimit;
		nextSetAsc = FieldSet_alloc(candidate, configuration->comparator);
		if (nextSetAsc == NULL) { break; }
		FieldSet_organize(setAsc);
		for (j = 0; j < setAsc->size; ++j) {
			field = setAsc->list[j];
			for (direction = LEFT; direction <= DOWN; ++direction) {
				if (Direction_reverse(direction) != field->lastOperation && FieldOperation_canMove(field, direction)) {
					newField = FieldOperation_copy(field);
					if (newField == NULL) { break; }
					FieldOperation_move(newField, direction, 0);
					fieldTemp = Field_alloc(newField);
					if (fieldTemp == NULL) {
						FieldOperation_release(newField);
						break;
					}
					if (!FieldHistory_add(fieldHistoryAsc, fieldTemp)) {
						FieldOperation_release(newField);
						Field_release(fieldTemp);
						break;
					}
					FieldSet_add(nextSetAsc, newField);
					solved = FieldOperation_isSolved(newField);
					FieldOperation_release(newField);
					if (solved) {
						foundAnswerAsc = fieldTemp;
						break;
					}
					if (FieldHistory_contains(fieldHistoryDsc, fieldTemp, 0)) {
						foundAnswerAsc = fieldTemp;
						foundAnswerDsc = FieldHistory_get(fieldHistoryDsc, fieldTemp, 0);
						Field_retain(foundAnswerDsc);
						break;
					}
					Field_release(fieldTemp);
				}
			}
			if (foundAnswerAsc != NULL) { break; }
		}
		FieldSet_release(setAsc);
		setAsc = nextSetAsc;
		if (foundAnswerAsc != NULL) {
			break;
		}

		nextSetDsc = FieldSet_alloc(candidate, configuration->comparator);
		if (nextSetDsc == NULL) { break; }
		FieldSet_organize(setDsc);
		for (j = 0; j < setDsc->size; ++j) {
			field = setDsc->list[j];
			for (direction = LEFT; direction <= DOWN; ++direction) {
				if (Direction_reverse(direction) != field->lastOperation && FieldOperation_canMove(field, direction)) {
					newField = FieldOperation_copy(field);
					if (newField == NULL) { break; }
					FieldOperation_move(newField, direction, 0);
					fieldTemp = Field_alloc(newField);
					if (fieldTemp == NULL) {
						FieldOperation_release(newField);
						break;
					}
					if (!FieldHistory_add(fieldHistoryDsc, fieldTemp)) {
						FieldOperation_release(newField);
						Field_release(fieldTemp);
						break;
					}
					FieldSet_add(nextSetDsc, newField);
					solved = FieldOperation_isSolved(newField);
					FieldOperation_release(newField);
					if (solved) {
						foundAnswerDsc = fieldTemp;
						break;
					}
					if (FieldHistory_contains(fieldHistoryAsc, fieldTemp, 0)) {
						foundAnswerDsc = fieldTemp;
						foundAnswerAsc = FieldHistory_get(fieldHistoryAsc, fieldTemp, 0);
						Field_retain(foundAnswerAsc);
						break;
					}
					Field_release(fieldTemp);
				}
			}
			if (foundAnswerDsc != NULL) { break; }
		}
		FieldSet_release(setDsc);
		setDsc = nextSetDsc;
		if (foundAnswerDsc != NULL) {
			break;
		}
	}

	if (foundAnswerAsc != NULL || foundAnswerDsc != NULL) {
		char operationsBuffer[configuration->moveLimit + 1];
		if (foundAnswerAsc != NULL) {
			Field_getOperations(foundAnswerAsc, 0, operationsBuffer);
			FieldOperation_moves(solvedField, operationsBuffer);
			Field_release(foundAnswerAsc);
		}
		if (foundAnswerDsc != NULL) {
			Field_getOperations(foundAnswerDsc, 1, operationsBuffer);
			FieldOperation_moves(solvedField, operationsBuffer);
			Field_release(foundAnswerDsc);
		}
	}

	release:
	FieldHistory_clear(fieldHistoryAsc);
	FieldHistory_clear(fieldHistoryDsc);
	FieldSet_release(setAsc);
	FieldSet_release(setDsc);
}


FieldCondition *FieldCondition_alloc(int width, int height, const char *fieldString, ScoringType scoringType) {
	FieldCondition *this = calloc(1, sizeof(FieldCondition));
	int *startField = malloc(sizeof(int) * width * height);
	int *finalField = malloc(sizeof(int) * width * height);
	if (this == NULL || startField == NULL || finalField == NULL) {
		if (this != NULL) { free(this); }
		if (startField != NULL) { free(startField); }
		if (finalField != NULL) { free(finalField); }
		return NULL;
	}

	int i;
	this->scoringType = scoringType;
	this->width = width;
	this->height = height;
	this->size = width * height;
	// 1セルあたりに必要なbit数を計算、size種類（0～size-1）の数値/記号と壁で+1 が必要, size-1+1 が必要とするbit数を算出
	this->bitPerCell = Uint32_numberOfTrailingZeros(Uint32_highestOneBit(this->size)) + 1;
	// 1セル分を取り出すためのMask bitPerCell分だけbitが立つ
	this->cellMask = (UINT32_C(1) << this->bitPerCell) - UINT32_C(1);
	this->cellsPerInt = (int) (INT_SIZE / this->bitPerCell);
	for (i = 0; i < this->size; ++i) {
		char cellString = fieldString[i];
		if (cellString == '=') {
			// = の場合、cellMaskと同値を壁と扱う
			this->wall[i] = 1;
			startField[i + 1] = i;
		}
		else if ('0' <= cellString && cellString <= '9') {
			// 0-9 の場合、そのままの数値を利用
			startField[cellString - '0'] = i;
		}
		else if ('A' <= cellString && cellString <= 'Z') {
			// A-Z の場合、A=10,B=11,,,Z=35
			startField[cellString - 'A' + 10] = i;
		}
		else {
			FieldCondition_release(this);
			return NULL;
		}
	}
	finalField[0] = this->size - 1;
	for (i = 1; i < this->size; ++i) {
		finalField[i] = i - 1;
	}
	this->startField = startField;
	this->finalField = finalField;

	for (i = 0; i < this->size; ++i) {
		this->fieldSlotMap[i] = i / this->cellsPerInt;
		this->fieldInSlotPositionMap[i] = (i % this->cellsPerInt) * this->bitPerCell;
	}

	// 各ポジションから移動可能な位置を表すcanMoveMapを構築
	for (i = 0; i < this->size; ++i) {
		this->canMoveMap[i] = 0;
		Direction direction;
		for (direction = LEFT; direction <= DOWN; ++direction) {
			int targetPosition;
			switch (direction) {
			case LEFT:
				if (i % this->width <= 0) {
					continue;
				}
				targetPosition = i - 1;
				break;
			case RIGHT:
				if (i % this->width >= this->width - 1) {
					continue;
				}
				targetPosition = i + 1;
				break;
			case UP:
				if (i / this->width <= 0) {
					continue;
				}
				targetPosition = i - this->width;
				break;
			case DOWN:
				if (i / this->width >= this->height - 1) {
					continue;
				}
				targetPosition = i + this->width;
				break;
			}

			// 移動先が壁でないことを確認
			if (!this->wall[targetPosition]) {
				this->canMoveMap[i] |= 1 << direction;
			}
		}
	}

	// positionからx,yに変換を行うpositionXMap/positionYMapの構築
	for (i = 0; i < this->size; ++i) {
		this->positionXMap[i] = i % this->width;
		this->positionYMap[i] = i / this->width;
	}

	return this;
}
void FieldCondition_release(FieldCondition *this) {
	free(this->finalField);
	free(this->startField);
	free(this);
}


// -- public static class Field --
Field *Field_alloc(FieldOperation *fieldOperation) {
	Field *this = malloc(sizeof(Field));
	if (this == NULL) {
		return NULL;
	}
	this->reference = 1;
	this->field = fieldOperation->field;
	Uint32Array_retain(this->field);
	this->operations = fieldOperation->operations;
	Uint8Array_retain(this->operations);
	this->currentOperation = fieldOperation->currentOperation;
	this->operationCount = fieldOperation->operationCount;

	int i;
	this->hashCode = 0;
	for (i = 0; i < this->field->length; ++i) {
		this->hashCode ^= this->field->list[i];
		this->hashCode *= 13;
	}

	return this;
}
int Field_equals(Field *this, Field *another) {
	if (another == NULL) {
		return 0;
	}
	if (this->hashCode != another->hashCode
			|| Uint32Array_equals(this->field, another->field) == 0) {
		return 0;
	}
	return 1;
}
void Field_retain(Field *this) {
	this->reference++;
}
void Field_release(Field *this) {
	if (--this->reference <= 0) {
		Uint32Array_release(this->field);
		Uint8Array_release(this->operations);
		free(this);
	}
}
void Field_getOperations(Field *this, unsigned char reverseMode, char *operationsString) {
	FieldOperation_getOperations(this->operations, this->currentOperation, this->operationCount, reverseMode, operationsString);
}

// FieldOperation
FieldOperation *FieldOperation_alloc(FieldCondition *condition) {
	FieldOperation *this = malloc(sizeof(FieldOperation));
	Uint32Array *field = Uint32Array_alloc(condition->size / condition->cellsPerInt + (condition->size % condition->cellsPerInt > 0 ? 1 : 0));
	Uint8Array *operations = Uint8Array_alloc(0);
	if (this == NULL || field == NULL || operations == NULL) {
		if (this != NULL) { free(this); }
		if (field != NULL) { Uint32Array_release(field); }
		if (operations != NULL) { Uint8Array_release(operations); }
		return NULL;
	}

	int i;
	this->reference = 1;
	this->instanceId = condition->instanceId++;
	this->condition = condition;
	this->field = field;

	for (i = 0; i < condition->size; ++i) {
		if ((condition->wall[condition->startField[i]])) {
			// 壁の場合
			FieldOperation_setCell(this, condition->startField[i], condition->cellMask);
		}
		else if (i == 0) {
			// 0 の場合、そのままの数値を利用、positionの初期化
			FieldOperation_setCell(this, condition->startField[i], 0);
			this->position = condition->startField[i];
		}
		else {
			FieldOperation_setCell(this, condition->startField[i], i);
		}
	}
	this->operationCount = UINT16_C(0);
	this->currentOperation = UINT8_C(0);
	this->operations = operations;
	this->lastOperation = -1;
	this->reverseMode = 0;
	FieldOperation_updateManhattanDistance(this);

	return this;
}
FieldOperation *FieldOperation_copy(FieldOperation *this) {
	FieldOperation *copied = malloc(sizeof(FieldOperation));
	Uint32Array *field = Uint32Array_copy(this->field);
	if (this == NULL || field == NULL) {
		if (this != NULL) { free(this); }
		if (field != NULL) { Uint32Array_release(field); }
		return NULL;
	}
	*copied = *this;
	copied->reference = 1;
	copied->field = field;
	copied->instanceId = this->condition->instanceId++;
	// copied->operationsはリードオンリーなので同一オブジェクトを共有、参照保持のためretain()
	Uint8Array_retain(copied->operations);
	return copied;
}
FieldOperation *FieldOperation_copyAsReverse(FieldOperation *this) {
	if (this->reverseMode) {
		return NULL;
	}
	FieldOperation *copied = FieldOperation_alloc(this->condition);
	if (copied == NULL) {
		return NULL;
	}
	int i;
	for (i = 0; i < this->condition->size; ++i) {
		if ((this->condition->wall[i])) {
			// 壁の場合
			FieldOperation_setCell(copied, i, this->condition->cellMask);
		}
		else if (i == this->condition->size - 1) {
			// 0 の場合、そのままの数値を利用、positionの初期化
			FieldOperation_setCell(copied, i, 0);
			copied->position = i;
		}
		else {
			FieldOperation_setCell(copied, i, i + 1);
		}
	}
	copied->reverseMode = 1;
	FieldOperation_updateManhattanDistance(copied);

	return copied;
}
void FieldOperation_retain(FieldOperation *this) {
	this->reference++;
}
void FieldOperation_release(FieldOperation *this) {
	if (--this->reference <= 0) {
		Uint32Array_release(this->field);
		Uint8Array_release(this->operations);
		free(this);
	}
}
int FieldOperation_getCell(FieldOperation *this, int i) {
	return (this->field->list[this->condition->fieldSlotMap[i]] >> this->condition->fieldInSlotPositionMap[i]) & this->condition->cellMask;
}
void FieldOperation_setCell(FieldOperation *this, int i, int cell) {
	int intPos = this->condition->fieldInSlotPositionMap[i];
	int slot = this->condition->fieldSlotMap[i];
	// 指定場所のbitを置き換え
	this->field->list[slot] = (this->field->list[slot] ^ (this->field->list[slot] & (this->condition->cellMask << intPos))) | (cell << intPos);
}
unsigned char FieldOperation_canMove(FieldOperation *this, Direction direction) {
	// 移動先が壁でないことを確認
	return (this->condition->canMoveMap[this->position] & 1 << direction) != 0 ? 1 : 0;
}
void FieldOperation_moves(FieldOperation *this, const char *operations) {
	while (*operations != '\0') {
		FieldOperation_move(this, Direction_valueOf(*operations), 1);
		++operations;
	}
}
void FieldOperation_move(FieldOperation *this, Direction direction, unsigned char check) {
	if (check && FieldOperation_canMove(this, direction) == 0) {
		return;
	}
	int targetPosition;
	switch (direction) {
	case LEFT:
		targetPosition = this->position - 1;
		break;
	case RIGHT:
		targetPosition = this->position + 1;
		break;
	case UP:
		targetPosition = this->position - this->condition->width;
		break;
	case DOWN:
		targetPosition = this->position + this->condition->width;
		break;
	}

	// 入れ替え処理
	int targetCell = FieldOperation_getCell(this, targetPosition);
	this->manhattanDistance -= FieldOperation_getManhattanDistance(this, targetCell, targetPosition);
	this->manhattanDistance += FieldOperation_getManhattanDistance(this, targetCell, this->position);
	FieldOperation_setCell(this, this->position, targetCell);
	FieldOperation_setCell(this, targetPosition, 0);
	this->position = targetPosition;
	this->lastOperation = direction;

	this->currentOperation |= (uint8_t) direction << BITS_PER_OPERATION * (this->operationCount % OPERATIONS_PER_VARIABLE);
	++this->operationCount;

	// OPERATIONS_PER_VARIABLE毎にcurrentOperationをoperations配列に入れて初期化
	if (this->operationCount % OPERATIONS_PER_VARIABLE == 0) {
		Uint8Array *operations = this->operations;
		this->operations = Uint8Array_copyWithLength(operations, this->operationCount / OPERATIONS_PER_VARIABLE);
		this->operations->list[this->operations->length - 1] = this->currentOperation;
		this->currentOperation = UINT8_C(0);
		Uint8Array_release(operations);
	}
}
void FieldOperation_getOperations(Uint8Array *operations, uint8_t currentOperation, uint16_t operationCount, unsigned char reverseMode, char *operationsString) {
	int i, j;
	*(operationsString + operationCount) = '\0';
	if (operationCount <= 0) { return; }
	if (reverseMode) {
		operationsString += operationCount - 1;
	}
	for (i = 0; i < operations->length; ++i) {
		for (j = 0; j < OPERATIONS_PER_VARIABLE; ++j) {
			if (reverseMode) {
				*(operationsString--) = Direction_getValue(Direction_reverse((operations->list[i] >> j * BITS_PER_OPERATION) & OPERATION_MASK));
			}
			else {
				*(operationsString++) = Direction_getValue((operations->list[i] >> j * BITS_PER_OPERATION) & OPERATION_MASK);
			}
		}
	}
	for (j = 0; j < operationCount % OPERATIONS_PER_VARIABLE; ++j) {
		if (reverseMode) {
			*(operationsString--) = Direction_getValue(Direction_reverse((currentOperation >> j * BITS_PER_OPERATION) & OPERATION_MASK));
		}
		else {
			*(operationsString++) = Direction_getValue((currentOperation >> j * BITS_PER_OPERATION) & OPERATION_MASK);
		}
	}
}
void FieldOperation_updateManhattanDistance(FieldOperation *this) {
	int i;
	this->manhattanDistance = 0;
	for (i = 0; i < this->condition->size; ++i) {
		this->manhattanDistance += FieldOperation_getManhattanDistance(this, FieldOperation_getCell(this, i), i);
	}
}
int FieldOperation_getManhattanDistance(FieldOperation *this, int cell, int position) {
	if (cell == this->condition->cellMask || cell == 0) {
		return 0;
	}
	return FieldOperation_getDistance(this, position, this->reverseMode == 0 ? this->condition->finalField[cell] : this->condition->startField[cell]);
}
int FieldOperation_getDistance(FieldOperation *this, int start, int end) {
	int distance = ((distance = this->condition->positionXMap[end] - this->condition->positionXMap[start]) < 0 ? - distance : distance)
					+ ((distance = this->condition->positionYMap[end] - this->condition->positionYMap[start]) < 0 ? - distance : distance);
	if (this->condition->scoringType == SCORING_TYPE_POW) {
		distance *= distance;
	}
	if (this->condition->scoringType == SCORING_TYPE_LEFT) {
		distance *= ((this->condition->width - this->condition->positionXMap[end] - 1) * this->condition->width
				+ (this->condition->height - this->condition->positionYMap[end])) * 30;
	}
	if (this->condition->scoringType == SCORING_TYPE_TOP) {
		distance *= (this->condition->size - end) * 30;
	}
	return distance;
}
unsigned char FieldOperation_isSolved(FieldOperation *this) {
	return this->manhattanDistance == 0 ? 1 : 0;
}
// Id降順/方向なし
int FieldOperation_comparatorDefault(const void *a, const void *b) {
	FieldOperation *o1 = *(FieldOperation **) a;
	FieldOperation *o2 = *(FieldOperation **) b;
	if (o1 == NULL && o2 == NULL) { return 0; }
	if (o1 == NULL) { return 1; }
	if (o2 == NULL) { return -1; }
	if (o1->manhattanDistance != o2->manhattanDistance) {
		return o1->manhattanDistance - o2->manhattanDistance;
	}
	if (o1->instanceId != o2->instanceId) {
		return o1->instanceId > o2->instanceId ? -1 : 1;
	}
	return 0;
}
// Id降順/方向なし/27手まで逆順
int FieldOperation_comparatorBack27(const void *a, const void *b) {
	FieldOperation *o1 = *(FieldOperation **) a;
	FieldOperation *o2 = *(FieldOperation **) b;
	if (o1 == NULL && o2 == NULL) { return 0; }
	if (o1 == NULL) { return 1; }
	if (o2 == NULL) { return -1; }
	int reverseStartup = o1->operationCount <= 27 ? -1 : 1;
	if (o1->manhattanDistance != o2->manhattanDistance) {
		return (o1->manhattanDistance - o2->manhattanDistance) * reverseStartup;
	}
	if (o1->instanceId != o2->instanceId) {
		return (o1->instanceId > o2->instanceId ? -1 : 1) * reverseStartup;
	}
	return 0;
}


// FieldSet
FieldSet *FieldSet_alloc(int capacity, int (*comparator)(const void*, const void*)) {
	FieldSet *this = malloc(sizeof(FieldSet));
	int internalCapacity = (int) ((float) capacity * 1.2f);
	if (internalCapacity <= capacity) { internalCapacity = capacity + 1; }
	FieldOperation **list = calloc(internalCapacity, sizeof(FieldOperation *));
	if (this == NULL || list == NULL) {
		if (this != NULL) { free(this); }
		if (list != NULL) { free(list); }
		return NULL;
	}
	this->comparator = comparator;
	this->list = list;
	this->capacity = capacity;
	this->internalCapacity = internalCapacity;
	this->size = 0;
	this->list = list;
	this->minThresould = NULL;
	this->organized = 1;
	return this;
}
void FieldSet_release(FieldSet *this) {
	int i;
	for (i = 0; i < this->size; ++i) {
		if (this->list[i] != NULL) {
			FieldOperation_release(this->list[i]);
		}
	}
	free(this->list);
	free(this);
}
void FieldSet_addSet(FieldSet *this, FieldSet *field) {
	int i;
	FieldSet_organize(field);
	for (i = 0; i < field->size; ++i) {
		FieldSet_add(this, field->list[i]);
	}
}
void FieldSet_add(FieldSet *this, FieldOperation *field) {
	if (this->minThresould != NULL && this->comparator != NULL && this->comparator(&field, &this->minThresould) > 0) {
		return;
	}
	if (this->size >= this->internalCapacity) {
		FieldSet_organize(this);
	}
	this->list[(this->size)++] = field;
	FieldOperation_retain(field);
	this->organized = 0;
}
void FieldSet_organize(FieldSet *this) {
	if (this->organized) {
		return;
	}
	if (this->comparator != NULL) {
		qsort(this->list, this->size, sizeof(FieldOperation *), this->comparator);
	}
	if (this->size >= this->capacity) {
		// minThresouldに入るオブジェクトは必ずlistに入っているのでretainしない
		this->minThresould = this->list[this->capacity - 1];
		for (; this->size > this->capacity;) {
			FieldOperation_release(this->list[--this->size]);
			this->list[this->size] = NULL;
		}
	}
	this->organized = 1;
}
FieldOperation *FieldSet_first(FieldSet *this) {
	FieldSet_organize(this);
	return this->size > 0 ? this->list[0] : NULL;
}


// FieldHistoryEntry
FieldHistoryEntry *FieldHistoryEntry_alloc() {
	FieldHistoryEntry *this = calloc(1, sizeof(FieldHistoryEntry));
	if (this == NULL) {
		return NULL;
	}
	return this;
}
void FieldHistoryEntry_release(FieldHistoryEntry *this) {
	free(this);
}


// FieldHistory
FieldHistory *FieldHistory_alloc(int capacity) {
	FieldHistory *this = malloc(sizeof(FieldHistory));
	int tableLength = Uint32_highestOneBit((uint32_t) ((float) capacity * 1.5f)) << 1;
	FieldHistoryEntry **table = calloc(tableLength, sizeof(FieldHistoryEntry *));
	FieldHistoryEntry **list = calloc(capacity, sizeof(FieldHistoryEntry *));
	if (this == NULL || table == NULL || list == NULL) {
		if (this != NULL) { free(this); }
		if (table != NULL) { free(table); }
		if (list != NULL) { free(list); }
		return NULL;
	}
	this->capacity = capacity;
	this->tableLength = tableLength;
	this->list = list;
	this->table = table;
	this->index = 0;
	this->waterMark = 0;
	this->version = 0;
	return this;
}
void FieldHistory_release(FieldHistory *this) {
	int i, bucket;
	FieldHistoryEntry *entry;
	FieldHistoryEntry *entryTemp;
	for (i = 0; i < this->waterMark; ++i) {
		if (this->list[i] != NULL) {
			Field_release(this->list[i]->entry);
			FieldHistoryEntry_release(this->list[i]);
			this->list[i] = NULL;
		}
	}
	free(this->list);
	free(this->table);
	free(this);
}
int FieldHistory_indexFor(FieldHistory *this, int hashCode) {
	return (hashCode ^ (hashCode >> Uint32_numberOfTrailingZeros((uint32_t) this->tableLength))) & (this->tableLength - 1);
}
unsigned char FieldHistory_add(FieldHistory *this, Field *field) {
	FieldHistoryEntry *entry = FieldHistory_getEntry(this, field, 0);
	if (entry != NULL) {
		if (entry->version == this->version) {
			return 0;
		}
		else {
			// tableから削除、listには参照が残る
			FieldHistory_remove(this, entry);
		}
	}
	FieldHistoryEntry *recycle = NULL;
	if (this->list[this->index] != NULL) {
		FieldHistory_remove(this, this->list[this->index]);
		Field_release(this->list[this->index]->entry);
		recycle = this->list[this->index];
	}
	if (recycle == NULL) {
		// 失敗した場合はsegfaultで死ぬ
		recycle = FieldHistoryEntry_alloc();
	}
	int bucket = FieldHistory_indexFor(this, field->hashCode);
	recycle->entry = field;
	Field_retain(field);
	recycle->next = this->table[bucket];
	recycle->version = this->version;
	this->table[bucket] = recycle;
	this->list[this->index] = recycle;
	// ARM向け最適化、剰余演算は行わない
	// 元コード this->index = (this->index + 1) % this->capacity;
	++this->index;
	if (this->index >= this->capacity) {
		this->index = this->index - this->capacity;
	}
	if (this->waterMark < this->capacity) { this->waterMark++; }
	return 1;
}
Field *FieldHistory_get(FieldHistory *this, Field *field, unsigned char current) {
	FieldHistoryEntry *entry = FieldHistory_getEntry(this, field, current);
	return entry != NULL ? entry->entry : NULL;
}
FieldHistoryEntry *FieldHistory_getEntry(FieldHistory *this, Field *field, unsigned char current) {
	FieldHistoryEntry *candidate;
	for (candidate = this->table[FieldHistory_indexFor(this, field->hashCode)]; candidate != NULL; candidate = candidate->next) {
		if (Field_equals(candidate->entry, field)) {
			return !current || candidate->version == this->version ? candidate : NULL;
		}
	}
	return NULL;
}
unsigned char FieldHistory_contains(FieldHistory *this, Field *field, unsigned char current) {
	return FieldHistory_get(this, field, current) != NULL ? 1 : 0;
}
/**
 * すべてのデータを論理削除
 * get() contains() の第2引数にfalseを指定すれば、clear()されたけどバッファに残っているデータを取得可能
 */
void FieldHistory_clear(FieldHistory *this) {
	++this->version;
	if (this->version == 0) {
		// uint32_tがオーバーフローした場合はすべてのEntryのバージョンを0にリセットしてversionを1にする
		int i;
		FieldHistoryEntry *entry;
		for (i = 0; i < this->waterMark; ++i) {
			if (this->list[i] != NULL) {
				this->list[i]->version = 0;
			}
		}
		++this->version;
	}
}
void FieldHistory_remove(FieldHistory *this, FieldHistoryEntry *field) {
	int bucket = FieldHistory_indexFor(this, field->entry->hashCode);
	FieldHistoryEntry *candidate = this->table[bucket];
	FieldHistoryEntry *prevCandidate = NULL;
	while (candidate != NULL) {
		if (candidate == field) {
			if (prevCandidate == NULL) {
				this->table[bucket] = candidate->next;
			}
			else {
				prevCandidate->next = candidate->next;
			}
			return;
		}
		prevCandidate = candidate;
		candidate = candidate->next;
	}
	return;
}


// Uint8Array
Uint8Array *Uint8Array_alloc(int length) {
	Uint8Array *this = malloc(sizeof(Uint8Array));
	// 配列要素は初期化処理をしないため0埋め必須
	uint8_t *list = length == 0 ? NULL : calloc(length, sizeof(uint8_t));
	if (this == NULL || (length != 0 && list == NULL)) {
		if (this != NULL) { free(this); }
		if (list != NULL) { free(list); }
		return NULL;
	}
	this->length = length;
	this->list = list;
	this->reference = 1;
	return this;
}
Uint8Array *Uint8Array_copy(Uint8Array *this) {
	Uint8Array *copied = malloc(sizeof(Uint8Array));
	// memcpyで上書きされるためallocと違い初期化が不要
	uint8_t *list = this->length == 0 ? NULL : malloc(sizeof(uint8_t) * this->length);
	if (this == NULL || (this->length != 0 && list == NULL)) {
		if (this != NULL) { free(this); }
		if (list != NULL) { free(list); }
		return NULL;
	}
	memcpy(list, this->list, sizeof(uint8_t) * this->length);
	copied->length = this->length;
	copied->list = list;
	copied->reference = 1;
	return copied;
}
Uint8Array *Uint8Array_copyWithLength(Uint8Array *this, int length) {
	Uint8Array *copied = Uint8Array_alloc(length);
	if (copied == NULL) {
		return NULL;
	}
	memcpy(copied->list, this->list, sizeof(uint8_t) * (this->length < length ? this->length : length));
	return copied;
}
int Uint8Array_equals(Uint8Array *this, Uint8Array *another) {
	if (this == another) { return 1; }
	if (another == NULL) { return 0; }
	int length = this->length;
	if (length != another->length) { return 0; }
	int i;
	for (i = 0; i < length; ++i) {
		if (this->list[i] != another->list[i]) { return 0; }
	}
	return 1;
}
void Uint8Array_retain(Uint8Array *this) {
	this->reference++;
}
void Uint8Array_release(Uint8Array *this) {
	if (--this->reference <= 0) {
		if (this->length > 0) { free(this->list); }
		free(this);
	}
}


// Uint32Array
Uint32Array *Uint32Array_alloc(int length) {
	Uint32Array *this = malloc(sizeof(Uint32Array));
	// 配列要素は初期化処理をしないため0埋め必須
	uint32_t *list = length == 0 ? NULL : calloc(length, sizeof(uint32_t));
	if (this == NULL || (length != 0 && list == NULL)) {
		if (this != NULL) { free(this); }
		if (list != NULL) { free(list); }
		return NULL;
	}
	this->length = length;
	this->list = list;
	this->reference = 1;
	return this;
}
Uint32Array *Uint32Array_copy(Uint32Array *this) {
	Uint32Array *copied = malloc(sizeof(Uint32Array));
	// memcpyで上書きされるためallocと違い初期化が不要
	uint32_t *list = this->length == 0 ? NULL : malloc(sizeof(uint32_t) * this->length);
	if (this == NULL || (this->length != 0 && list == NULL)) {
		if (this != NULL) { free(this); }
		if (list != NULL) { free(list); }
		return NULL;
	}
	memcpy(list, this->list, sizeof(uint32_t) * this->length);
	copied->length = this->length;
	copied->list = list;
	copied->reference = 1;
	return copied;
}
Uint32Array *Uint32Array_copyWithLength(Uint32Array *this, int length) {
	Uint32Array *copied = Uint32Array_alloc(length);
	if (copied == NULL) {
		return NULL;
	}
	memcpy(copied->list, this->list, sizeof(uint32_t) * (this->length < length ? this->length : length));
	return copied;
}
int Uint32Array_equals(Uint32Array *this, Uint32Array *another) {
	if (this == another) { return 1; }
	if (another == NULL) { return 0; }
	int length = this->length;
	if (length != another->length) { return 0; }
	int i;
	for (i = 0; i < length; ++i) {
		if (this->list[i] != another->list[i]) { return 0; }
	}
	return 1;
}
void Uint32Array_retain(Uint32Array *this) {
	this->reference++;
}
void Uint32Array_release(Uint32Array *this) {
	if (--this->reference <= 0) {
		if (this->length > 0) { free(this->list); }
		free(this);
	}
}


// Direction
char Direction_getValue(Direction this) {
	static char directions[] = {'L', 'R', 'U', 'D'};
	return directions[this];
}
Direction Direction_valueOf(char value) {
	switch (value) {
	case 'L':
		return LEFT;
	case 'R':
		return RIGHT;
	case 'U':
		return UP;
	case 'D':
		return DOWN;
	}
}
Direction Direction_reverse(Direction this) {
	return this ^ 1;
}
Direction Direction_clockwise(Direction this) {
	static Direction directions[] = {UP, DOWN, RIGHT, LEFT};
	return directions[this];
}
Direction Direction_counterclockwise(Direction this) {
	static Direction directions[] = {DOWN, UP, LEFT, RIGHT};
	return directions[this];
}


// Uint32
uint32_t Uint32_highestOneBit(uint32_t value) {
	value |= (value >>  1);
	value |= (value >>  2);
	value |= (value >>  4);
	value |= (value >>  8);
	value |= (value >> 16);
	return value - (value >> 1);
}
int Uint32_numberOfTrailingZeros(uint32_t value) {
	int y;
	if (value == UINT32_C(0)) { return 32; }
    int n = 31;
    y = value << 16; if (y != 0) { n -= 16; value = y; }
    y = value <<  8; if (y != 0) { n -=  8; value = y; }
    y = value <<  4; if (y != 0) { n -=  4; value = y; }
    y = value <<  2; if (y != 0) { n -=  2; value = y; }
    return n - ((value << 1) >> 31);
}
