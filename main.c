#include "pmi.h"

int main(int argc, char ** argv )
{
	if( PMI_Init() != PMI_SUCCESS )
	{
		fprintf(stderr, "Could not init PMI\n");
		return 1;
	}

	int rank = 0;
        int size = 1;
	if( PMI_Get_size(&size) != PMI_SUCCESS )
	{
		fprintf(stderr, "Could not get PMI size\n");
		return 1;
	}
	fprintf(stdout,"size : %d\t",size);

	if( PMI_Get_rank(&rank) != PMI_SUCCESS )
	{
		fprintf(stderr, "Could not get PMI rank\n");
		return 1;
	}
	fprintf(stdout,"rank : %d\t\n",rank);

	if( PMI_Finalize() != PMI_SUCCESS )
	{
		fprintf(stderr, "Could not finalize PMI\n");
		return 1;
	}

	exit(0);
}
