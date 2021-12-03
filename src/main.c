#include "main.h"

/* 
    NOTE: You can change global definitions in main.h. Currently you can change
    file to read and VERBOSE mode.
*/

Matrix *getData(char *fileName);
char *getOutputName();

int main(int argc, char **argv)
{
    Matrix *matrix = NULL;

    int globalRank, nProc, matrixSize;
    
    double timeStart, timeFinish;

    /* Initialize global communicator and read the matrix data */
    MPI_Init(&argc, &argv); 
    MPI_Comm_size(MPI_COMM_WORLD, &nProc);
    MPI_Comm_rank(MPI_COMM_WORLD, &globalRank);

    if (globalRank == 0)
    {
        matrix = getData(FILENAME);
        matrixSize = matrix->nCol;

		float Q = sqrt(nProc);
		float fracPart = Q - (int)Q;

		if( (fracPart != 0) || (matrixSize % (int)Q != 0) ){
			printf("ERROR: Invalid configuration!\n");
			return -1;
		}
		
        int *pos;
        for (int i = 0; i < matrixSize; i++)
        {
            for (int x = 0; x < matrixSize; x++)
            {
                pos = getMatrixPos(matrix, i, x);
                if ((i != x) && (*pos == 0))
                    setMatrixPos(matrix, i, x, INT_MAX); //Change value to infinite
            }
        }
	}

#if VERBOSE
    printf("World %d: starting\n", globalRank);
#endif
    // Wait
    MPI_Barrier(MPI_COMM_WORLD);

    /* Construct grid communicator */
    GridInfoType *grid = newGrid();
    // Wait
    
    MPI_Barrier(MPI_COMM_WORLD); /* Start timer */
	timeStart = MPI_Wtime();

    MPI_Bcast(&matrixSize, 1, MPI_INT, 0, grid->comm);

    // Scatter the data. Each process has a localMatrix that is a submatrix
    int localMatrixSize = matrixSize / grid->q;
    Matrix *local_A = scatterData(grid, matrix, newMatrix(localMatrixSize, localMatrixSize, 0));
    Matrix *local_B = newMatrix(localMatrixSize, localMatrixSize, 0);
    copyMatrix(local_A, local_B);

    Matrix *local_C = newMatrix(localMatrixSize, localMatrixSize, INT_MAX);

    for (int x = 1; x < matrixSize - 1; x++) //Fox algorithm
    {
        Fox(matrixSize, grid, local_A, local_B, local_C);
        copyMatrix(local_C, local_A);
        copyMatrix(local_C, local_B);
    }

    Matrix *result = gatherData(local_C, grid);

	MPI_Barrier(MPI_COMM_WORLD);
	timeFinish = MPI_Wtime();    /* Finish timer */

    if (grid->myRank == 0)
    {

# if VERBOSE
		char *outputFile = getOutputName();
        
        Matrix *mOutput = getData(outputFile);

        if (!equalsMatrix(result, mOutput))
        {   
            printf("NOT EQUALS\n");
            
            printf("Result:\n");
            printMatrix(result);
            printf("Output:\n");
            printMatrix(mOutput);
        }else{
            printf("EQUALS\n");
        }
        
        free(outputFile);
        freeMatrix(mOutput);
#endif    

		printMatrix(result);
		printf("\nExecution time: %f\n", timeFinish - timeStart);
        freeMatrix(result);
    }
	
    freeGrid(grid);
    freeMatrix(matrix);
    freeMatrix(local_A);
    freeMatrix(local_B);
    freeMatrix(local_C);

    MPI_Finalize();
}

Matrix *getData(char *fileName)
{
    Matrix *matrix;

    if (fileName == NULL)
        matrix = readData(FILENAME);

    else
        matrix = readData(fileName);

#if VERBOSE
    printf("Matrix Read:\n");
    printMatrix(matrix);
#endif

    return matrix;
}


char *getOutputName(){
	
	char *inputName = FILENAME;
	const char *number = inputName + 5; //Word "input" have 5 letters

	int sizeOfOutput = strlen("output") + strlen(number);
	char *outputName = (char *)malloc(sizeof(char) * (sizeOfOutput+1));

	checkAlloc(outputName, "getOutputName", "outputName");

	strcpy(outputName, "output");
	strcat(outputName, number);
		
	return outputName;
}
