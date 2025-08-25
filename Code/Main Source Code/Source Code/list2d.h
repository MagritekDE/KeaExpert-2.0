
#ifndef LIST2D_H
#define LIST2D_H

class List2DData
{
public:
	List2DData(int nRows);
	~List2DData();
	 bool MakeRow(int row, int width);
    bool AddEntry(char *txt, int x, int y);
   char*** strings; // Pointer to array of lists (which is an array of strings)
   int nrRows;      // Number of rows
   int maxCols;     // Maximum number of columns
   int *rowSz;      // Size of each row
};

#endif // define LIST2D_H