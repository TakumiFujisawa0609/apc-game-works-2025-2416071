#include "Grid.h"

Grid::Grid(int size, int count, int color)
	:cellSize(size), halfCount(count), lineColor(color) 
{

}

void Grid::Draw() const
{
	for (int i = -halfCount; i <= halfCount; i++) {
		//XŽ²‚É‘Î‚µ‚Ä
		DrawLine3D(
			VGet((float)i * cellSize, 0, -halfCount * cellSize),
			VGet((float)i * cellSize, 0, halfCount * cellSize),
			lineColor
		);

		//Z•ûŒü‚Ìü
		DrawLine3D(
			VGet(-halfCount * cellSize, 0, (float)i * cellSize),
			VGet(halfCount * cellSize, 0, (float)i * cellSize),
			lineColor
		);
	}
}

void Grid::SetCellSize(int size) { cellSize = size; }
void Grid::SetHalfCount(int count) { halfCount = count; }
void Grid::SetColor(int color) { lineColor = color; }
